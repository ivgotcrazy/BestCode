/**
 * 参会人界面接口契约类
 * Created by Sky on 2017/2/16.
 */
public interface IUserFragmentContract {
    interface IUserPresenter extends IBasePresenter {
        /** 参会人列表的的搜索按键响应 */
        void onUserViewSearchClick();

        /** 用户搜索界面的搜素按键响应 */
        void onUserSearchViewSearchClick(String keyword);

        /** 列表的item点击事件的响应 */
        void onUserItemClick(BaseUser user);

        /** 返回按钮 */
        void onBack();

        /** 处理view可见状态变更 */
        void onUserViewVisible(boolean visible);

        /** 处理搜索参会人界面隐藏 */
        void onUserSearchDialogDismissed();

        /** 处理参会人列表滑动状态变更 */
        void onUserListScroll(boolean scrolling);
    }

    interface IUserView extends IBaseView<IUserPresenter> {
        /** 显示用户菜单 */
        void showUserMenu(BaseUser user);

        /** 关闭用户菜单 */
        void dismissUserMenu();

        /** 显示搜索界面菜单 */
        void showUserSearchDialog();

        /** 返回搜索界面是否在显示 */
        boolean userSearchDialogShowing();

        /** 用户菜单是否在显示 */
        boolean userMenuDialogShowing();

        /** 返回菜单用户 */
        BaseUser getMenuDialogUser();

        /** 显示没有与搜索关键词匹配的用户提示 */
        void showMatchUserEmptyToast();

        /** 刷新参会人列表 */
        void refreshUserList(ArrayList<BaseUser> users);

        /** 刷新搜搜参会人列表 */
        void refreshUserSearchList(ArrayList<BaseUser> searchedUsers);
    }
}
