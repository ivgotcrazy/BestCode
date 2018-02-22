/**
 * 用户列表业务presenter类
 * Created by Sky on 2017/2/16.
 */
public class UserPresenterImpl implements IUserPresenter, Observer {
    private static final String TAG = "UserPresenterImpl";

    private IUserView userView;
    private UserModel userModel;
    private String keyword;

    private boolean backCalled = false;
    // 当正在滑动列表时不刷新用户列表
    private boolean scrolling = false;
    private boolean userUpdatedUnrefreshed = false;


    /**
     * 构造器
     */
    public UserPresenterImpl(IUserView view) {
        userView = view;
        userView.setPresenter(this);

        userModel = UserModel.getInstance();
    }

    @Override
    public void start() {
        EventBus.getDefault().register(this);
    }

    @Override
    public void onStop() {
        EventBus.getDefault().unregister(this);
    }

    @Override
    public void onBack() {
        if (!backCalled) {
            EventBus.getDefault().post(new BaseDto(BaseDto.MEETINGACTIVITY_HIDE_POP_FRAGMENT));
            backCalled = true;
        }
    }

    @Override
    public void onUserViewVisible(boolean visible) {
        // 当界面不可见时不更新列表，减少主线程压力
        if (visible) {
            UmsAgent.onEvent(HstApplication.getContext(), UmsUtils.EVENT_ATTENDEES_LIST_LOAD);

            backCalled = false;
            userModel.addObserver(this);
            refreshUserListAndResetUnrefreshed(userModel.getUsers());
        } else {
            userModel.deleteObserver(this);
        }
    }

    @Override
    public void onUserSearchDialogDismissed() {
        keyword = null;
        refreshUserListAndResetUnrefreshed(userModel.getUsers());
    }

    @Override
    public void onUserListScroll(boolean isScrolling) {
        if (shouldRefreshUserList(isScrolling)) {
            refreshUserListAndResetUnrefreshed(userModel.getUsers());
        }

        scrolling = isScrolling;
    }

    @Override
    public void onUserViewSearchClick() {
        userView.showUserSearchDialog();
    }

    @Override
    public void onUserSearchViewSearchClick(String keyword) {
        // 不区分大小写
        this.keyword = keyword.toLowerCase();
        onUserSearch(userModel.getUsers());
    }

    @Override
    public void onUserItemClick(BaseUser user) {
        // 主讲或者主席点击并且当前用户有音视频权限才可以显示菜单
        if (shouldShowUserMenu(user)) {
            userView.showUserMenu(user);
        }
    }

    /**
     * 处理视频接收状态改变事件
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BaseDto event) {
        if (event.getType() == BaseDto.VIDEO_ENABLE_CHANGED) {
            if (userView.userSearchDialogShowing()) {
                updateKeywordUsers(userModel.getUsers());
            }
            onUserListRefreshByScrollState(userModel.getUsers());
        }
    }

    @SuppressWarnings("unchecked")
    @Override
    public void update(Observable observable, Object data) {
        // 当参会人搜索界面可见时只刷新搜索界面，不刷新参会人界面
        ArrayList<BaseUser> newUsers = (ArrayList<BaseUser>) data;

        if (userView.userSearchDialogShowing()) {
            updateKeywordUsers(newUsers);
        } else {
            onUserListRefreshByScrollState(newUsers);
        }

        if (userView.userMenuDialogShowing()) {
            BaseUser user = userView.getMenuDialogUser();
            if (!newUsers.contains(user)) {
                userView.dismissUserMenu();
            }
        }
    }

    private int updateKeywordUsers(ArrayList<BaseUser> newUsers) {
        ArrayList<BaseUser> keywordUsers = getSortedKeyWordUsers(newUsers);
        userView.refreshUserSearchList(keywordUsers);
        return keywordUsers.size();
    }

    private void onUserListRefreshByScrollState(ArrayList<BaseUser> newUsers) {
        if (!scrolling) {
            refreshUserListAndResetUnrefreshed(newUsers);
        } else {
            userUpdatedUnrefreshed = true;
        }
    }

    /**
     * 刷新用户列表，重置未刷新标志位。
     */
    private void refreshUserListAndResetUnrefreshed(ArrayList<BaseUser> newUsers) {
        userView.refreshUserList(sortUserList(newUsers));
        userUpdatedUnrefreshed = false;
    }

    /**
     * 当前为不滑动状态，并且记录前状态为滑动状态，且未刷新标志位为true时，才返回true。
     */
    private boolean shouldRefreshUserList(boolean isScrolling) {
        return !isScrolling && scrolling && userUpdatedUnrefreshed;
    }

    private void onUserSearch(ArrayList<BaseUser> srcUsers) {
        // 如果没有搜索到用户提示未搜索到用户的Toast
        if (updateKeywordUsers(srcUsers) == 0) {
            userView.showMatchUserEmptyToast();
        }
    }

    private ArrayList<BaseUser> getSortedKeyWordUsers(ArrayList<BaseUser> srcUsers) {
        ArrayList<BaseUser> keywordUsers = new ArrayList<>();
        findKeyWordUsers(srcUsers, keywordUsers);
        sortUserList(keywordUsers);

        return keywordUsers;
    }

    private void findKeyWordUsers(ArrayList<BaseUser> srcUsers, ArrayList<BaseUser> keywordUsers) {
        if (!TextUtils.isEmpty(keyword)) {
            for (BaseUser user : srcUsers) {
                String nickName = user.getNickName().toLowerCase();
                if (nickName.contains(keyword)) {
                    keywordUsers.add(user);
                }
            }
        }
    }

    private ArrayList<BaseUser> sortUserList(ArrayList<BaseUser> srcUsers) {
        Collections.sort(srcUsers, new UserComparator());
        return srcUsers;
    }

    /**
     * 只有当前用户有权限申请主讲（且不是电话用户）并且本地用户有授予主讲权限时，才返回true。
     */
    private boolean shouldShowUserMenu(BaseUser user) {
        return user.checkMainSpeakPermission() 
                            && userModel.getLocalUser().checkHighestPermission() && !user.isTelephoneUser();
    }
}
