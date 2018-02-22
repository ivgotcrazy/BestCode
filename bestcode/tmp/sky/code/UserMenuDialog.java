/**
 * 参会人弹出菜单
 */
public class UserMenuDialog extends BaseDialog implements View.OnClickListener {

    private Context context;
    private BaseUser roomUser;
    private BaseUser localUser;
    private UserModel userModel;

    private UserStateChangedListener userStateListener;

    // 用户名或昵称
    @BindView(R2.id.tv_username)
    TextView userNameTextView;
    // 主讲
    @BindView(R2.id.main_speaker_text_view)
    TextView mainTextView;
    @BindView(R2.id.main_speaker_Image_view)
    ImageView mainSpeakerImageView;

    /**
     * 构造函数
     */
    public UserMenuDialog(Context context, @NonNull BaseUser user) {
        super(context);
        this.context = context;
        roomUser = user;
        userModel = UserModel.getInstance();
        localUser = userModel.getLocalUser();
        setContentView(R.layout.main_pop_user);
        ButterKnife.bind(this);

        initViews();
        initValues();
        initListener();
    }

    public BaseUser getRoomUser() {
        return roomUser;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    protected void initViews() {}

    @Override
    protected void initValues() {}

    @Override
    protected void initListener() {
        mainTextView.setOnClickListener(this);
    }

    @Override
    public void show() {
        updateUi();
        super.show();

        userStateListener = new UserStateChangedListener();
        UserModel.getInstance().addUserStateChangedListener(userStateListener);
    }

    @Override
    public void dismiss() {
        super.dismiss();
        if (userStateListener != null) {
            UserModel.getInstance().removeUserStateChangedListener(userStateListener);
            userStateListener = null;
        }
    }

    private void updateUi() {
        setUserName(roomUser);
        setMainSpeaker();
    }

    private void setUserName(BaseUser user) {
        userNameTextView.setText(user.getNickName());
    }

    /**
     * 根据当前用户的主讲状态设置主讲字体和图标的颜色
     */
    private void setMainSpeaker() {
        if (roomUser.isMainSpeakerDone()) {
            mainTextView.setText(R.string.main_video_main_cancel);
            mainTextView.setTextColor(context.getResources().getColor(R.color.item_main_done_color));
            mainSpeakerImageView.setImageResource(R.drawable.menu_speaker_applied);
        }
        if (roomUser.isMainSpeakerWait()) {
            mainTextView.setText(R.string.main_video_main_apply);
            mainTextView.setTextColor(context.getResources().getColor(R.color.item_main_waiting_color));
            mainSpeakerImageView.setImageResource(R.drawable.menu_speaker_applying);
        }
        if (roomUser.isMainSpeakerNone()) {
            mainTextView.setText(R.string.main_video_main_apply);
            mainTextView.setTextColor(context.getResources().getColor(R.color.item_tv_noraml_color));
            mainSpeakerImageView.setImageResource(R.drawable.menu_speaker);
        }
    }

    @Override
    public void onClick(View view) {
        // 在点击按键时用户已经离开会议室，则不响应
        if (userModel.isUserInMeeting(roomUser)) {
            switch (view.getId()) {
                case R.id.main_speaker_text_view:
                    onMainSpeaker();
                    break;

                default:
                    break;
            }
        }

        dismiss();
    }

    private void onMainSpeaker() {
        if (roomUser != null) {
            userModel.dealMainSpeaker(roomUser);
        }
    }

    private class UserStateChangedListener extends SimpleUserStateChangedListener {
        @Override
        public void onUserMainSpeakerChanged(BaseUser user) {
            if (!userModel.getLocalUser().checkHighestPermission()) {
                dismiss();
            }
            if (isCurUserOrLocalUser(user)) {
                setMainSpeaker();
            }
        }

        /**
         * 更新昵称
         */
        @Override
        public void onUserChanged(BaseUser user) {
            if (isShowing() && isCurUser(user)) {
                setUserName(user);
            }
        }

        private boolean isCurUserOrLocalUser(BaseUser user) {
            return isCurUser(user) || user.getUserID() == localUser.getUserID();
        }

        private boolean isCurUser(BaseUser user) {
            return user.getUserID() == roomUser.getUserID();
        }
    }
}
