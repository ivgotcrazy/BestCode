/**
 * [参会人列表]
 * Created by Sky on 2017/1/19.
 */
public class UserFragment extends BaseFragment implements IUserView, OnClickListener {

    @BindView(R2.id.attender_listview)
    ListView userListView;
    @BindView(R2.id.back_imageview)
    ImageView backImageView;
    @BindView(R2.id.right_one_imageview)
    ImageView searchImageView;
    @BindView(R2.id.title_textview)
    TextView titleTextView;
    @BindView(R2.id.user_toolbar)
    Toolbar userToolbar;

    private UserMenuDialog userMenuDialog;
    private UserSearchDialog userSearchDialog;

    private UserListAdapter adapter;
    private IUserPresenter presenter;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container,
            @Nullable Bundle savedInstanceState) {
        View view = super.onCreateView(inflater, container, savedInstanceState);
        initViews(view);
        initValues();
        initListeners();
        return view;
    }

    @Override
    protected View loadLayout(LayoutInflater inflater, @Nullable ViewGroup container) {
        return inflater.inflate(R.layout.fragment_user, null);
    }

    @Override
    protected void initViews(View view) {
        adapter = new UserListAdapter(getContext(), new ArrayList<BaseUser>());
        userListView.setAdapter(adapter);
        titleTextView.setText(R.string.top_title_attendee);
        searchImageView.setVisibility(View.VISIBLE);

        if (ScreenUtils.isPhone()) {
            setToolBarHeight();
        }
    }

    @Override
    protected void initValues() {
        presenter.start();
    }

    @Override
    protected void initListeners() {
        searchImageView.setOnClickListener(this);
        backImageView.setOnClickListener(this);
        userListView.setOnItemClickListener(new UserListItemClickListener());
        userListView.setOnScrollListener(new UserListScrollListener());
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        presenter.onUserViewVisible(!hidden);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        setToolBarHeight();
        if (userSearchDialogShowing()) {
            userSearchDialog.setToolBarHeight();
        }
    }

    @Override
    public void showUserMenu(BaseUser user) {
        showUserMenuDialogIfNotShowing(user);
    }

    @Override
    public void dismissUserMenu() {
        dismissUserMenuDialog();
    }

    @Override
    public void showUserSearchDialog() {
        showUserSearchDialogIfNotShowing();
    }

    @Override
    public boolean userSearchDialogShowing() {
        return userSearchDialog != null && userSearchDialog.isShowing();
    }

    @Override
    public boolean userMenuDialogShowing() {
        return userMenuDialog != null && userMenuDialog.isShowing();
    }

    @Override
    public BaseUser getMenuDialogUser() {
        if (userMenuDialogShowing()) {
            return userMenuDialog.getRoomUser();
        }
        return null;
    }

    @Override
    public void showMatchUserEmptyToast() {
        if (userSearchDialog != null) {
            userSearchDialog.showMatchEmptyToast();
        }
    }

    @Override
    public void refreshUserList(ArrayList<BaseUser> users) {
        adapter.updateDataAndNotifyDataChanged(users);
    }

    @Override
    public void refreshUserSearchList(ArrayList<BaseUser> searchedUsers) {
        if (userSearchDialogShowing()) {
            userSearchDialog.refreshSearchUserList(searchedUsers);
        }
    }

    @Override
    public void setPresenter(IUserPresenter userPresenter) {
        presenter = userPresenter;
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.back_imageview:
                presenter.onBack();
                break;

            case R.id.right_one_imageview:
                presenter.onUserViewSearchClick();
                break;

            default:
                break;
        }
    }

    private void setToolBarHeight() {
        if (ScreenUtils.isPortrait(getContext())) {
            ViewGroup.LayoutParams params = userToolbar.getLayoutParams();
            params.height = (int) getResources().getDimension(R.dimen.toolbarHeight);
            userToolbar.setLayoutParams(params);
        } else {
            ViewGroup.LayoutParams params = userToolbar.getLayoutParams();
            params.height = (int) getResources().getDimension(R.dimen.toolbarHeightLand);
            userToolbar.setLayoutParams(params);
        }
    }

    private class UserListItemClickListener extends NoRepeatItemClickListener {

        @Override
        public void onNoRepeatItemClick(AdapterView<?> parent, View view, int position, long id) {
            presenter.onUserItemClick((BaseUser) parent.getAdapter().getItem(position));
        }
    }

    private class UserSearchClickListener implements ISearchClickListener {
        @Override
        public void onSearchClick(String keyword) {
            presenter.onUserSearchViewSearchClick(keyword);
        }
    }

    private void showUserMenuDialogIfNotShowing(BaseUser user) {
        if (!userMenuDialogShowing()) {
            userMenuDialog = new UserMenuDialog(getContext(), user);
            userMenuDialog.setOnDismissListener(new OnDismissListener() {
                @Override
                public void onDismiss(DialogInterface dialog) {
                    userMenuDialog = null;
                }
            });
            userMenuDialog.show();
        }
    }

    private void dismissUserMenuDialog() {
        if (userMenuDialogShowing()) {
            userMenuDialog.dismiss();
        }
    }

    private void showUserSearchDialogIfNotShowing() {
        if (!userSearchDialogShowing()) {
            userSearchDialog = new UserSearchDialog(getContext());
            userSearchDialog.setSearchUserItemClickListener(new UserListItemClickListener());
            userSearchDialog.setSearchClickListener(new UserSearchClickListener());
            userSearchDialog.setOnDismissListener(new OnDismissListener() {
                @Override
                public void onDismiss(DialogInterface dialog) {
                    presenter.onUserSearchDialogDismissed();
                    if (dialog == userSearchDialog) {
                        userSearchDialog = null;
                    }
                }
            });

            userSearchDialog.setToolBarHeight();
            userSearchDialog.show();
        }
    }

    private class UserListScrollListener implements OnScrollListener {
        @Override
        public void onScrollStateChanged(AbsListView view, int scrollState) {
            presenter.onUserListScroll(scrollState != OnScrollListener.SCROLL_STATE_IDLE);
        }

        @Override
        public void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount, int totalItemCount) {}
    }

    @Override
    public void onDestroyView() {
        presenter.onStop();
        super.onDestroyView();
    }
}
