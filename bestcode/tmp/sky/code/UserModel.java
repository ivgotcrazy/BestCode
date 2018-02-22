/**
 * Created by wangy on 2017/1/17.
 * UserModel实现了观察者，其返回的数据对象为UserMessage
 */
public class UserModel extends Observable {
    private static final String TAG = "UserModel";
    private static final String LISTENERS_NUMBER = "旁听人数:";

    private ConcurrentHashMap<Long, BaseUser> users = new ConcurrentHashMap<>();
    private BaseUser mainSpeakerUser = null;
    private BaseUser localUser = null;
    private MeetingRoomConfState meetingRoomConfState;
    private static UserModel instance;
    private Handler userHandler = new UserHandler();

    // 会议室总人数
    private int totalUser = 0;

    private CopyOnWriteArrayList<IUserStateChangedListener> listeners = new CopyOnWriteArrayList<>();

    enum UserUpdateType {
        USER_ADD,
        USER_BATCH_ADD, //用户批量添加
        USER_REMOVE,
        USER_UPDATE
    }

    /**
     * 单例获取器
     */
    public static UserModel getInstance() {
        if (instance == null) {
            instance = new UserModel();
        }
        return instance;
    }

    private UserModel() {
        // 先添加本地用户，防止出现外界调用为空指针的问题
        RoomUserInfo localUserInfo = UserManager.getInstance().getLocalUser();
        // 当本地用未被初始化好时，上述方法返回的对象不为null，但是userName一定为null，只有都不为null时才添加
        if (localUserInfo != null && localUserInfo.strUserName != null) {
            // 当应用被回收重新进入会中时，会导致本地用户为空
            localUser = putUserToMap(localUserInfo);
            if (localUser != null) {
                updateUserAndNotify(false, localUser, UserUpdateType.USER_ADD);
            }
        }
    }

    void setMeetingRoomConfState(MeetingRoomConfState state) {
        meetingRoomConfState = state;
    }

    private BaseUser createUser(RoomUserInfo userInfo) {
        return new BaseUser(userInfo);
    }

    /**
     * 添加用户，如果要添加的用户已经存在，则替换原来的
     *
     * @return 返回加入的user
     */
    BaseUser addUser(RoomUserInfo userInfo) {
        return addUserWithNotify(userInfo, true);
    }

    void addLocalUser(RoomUserInfo userInfo) {
        localUser = addUser(userInfo);
    }

    void addUserList(RoomUserInfo[] userInfos) {
        if (userInfos != null) {
            for (RoomUserInfo userInfo : userInfos) {
                addUserWithNotify(userInfo, false);
            }

            // 批量添加用户时，只在添加完成所有用户后才发送用户数目发生变化通知。
            // 注意批量添加用户时将changdeUser设置为null。
            sendUserChangedMsg(null, UserUpdateType.USER_BATCH_ADD);
        }
    }

    void removeUserById(long userID) {
        // 先更新数据在移除，保证用户状态时最新的
        RoomUserInfo removeUserInfo = UserManager.getInstance().getUser(userID);
        BaseUser removedUser = users.get(userID);
        if (removeUserInfo != null && removedUser != null) {
            removedUser.setRoomInfo(removeUserInfo);
        }

        removedUser = users.remove(userID);
        if (removedUser != null) {
            updateUserAndNotify(true, removedUser, UserUpdateType.USER_REMOVE);
        }
    }

    void updateUserById(long userID) {
        updateUserInfo(UserManager.getInstance().getUser(userID));
    }

    void updateUser(RoomUserInfo userInfo) {
        updateUserInfo(userInfo);
    }

    private void updateUserInfo(RoomUserInfo userInfo) {
        BaseUser user = users.get(userInfo.userID);
        if (user != null) {
            user.setRoomInfo(userInfo);
            updateUserAndNotify(true, user, UserUpdateType.USER_UPDATE);
            notifyUserChangedWitchCheckMainThread(user);
        } else {
            // 先从底层的用户列表中根据ID获取用户，如果存在则添加到上层用户列表中
            if (UserManager.getInstance().getUser(userInfo.userID) != null) {
                addUser(userInfo);
            }
        }
    }

    /**
     * 添加用户
     * @param isNeedSendMsg 是否需要发送数据变化广播
     */
    private BaseUser addUserWithNotify(RoomUserInfo userInfo, boolean isNeedSendMsg) {
        BaseUser user = putUserToMap(userInfo);
        if (user != null) {
            updateUserAndNotify(isNeedSendMsg, user, UserUpdateType.USER_ADD);
        }

        return user;
    }

    private BaseUser putUserToMap(RoomUserInfo userInfo) {
        BaseUser user = createUser(userInfo);
        users.put(userInfo.userID, user);
        return user;
    }

    /**
     * 先计算所有的参会人数。
     * 在更新主讲状态。
     * 最后确定是否发送用户发生变化通知
     */
    private void updateUserAndNotify(boolean isNeedSendMsg, BaseUser user, UserUpdateType type) {
        calcTotalUsersAndNotify(user, type);
        updateMainSpeakAfterUpdateUsers(user);

        if (isNeedSendMsg) {
            sendUserChangedMsg(user, type);
        }
    }

    /**
     * 计算会议中总用户数。
     * 在boss后台设置为：旁听用户的列表显示旁听用户总数 时，所有旁听归纳为一个用户
     * 导致总人数出错（比如旁听人数为5，有一行显示为：“旁听人数:5”，旁听自己算一个人，这一行算一个人）
     * 所以当本地用户也作为旁听时并且设置为“旁听人数:x”时，客户端将每个旁听都作为单独的一个人计算。
     */
    private void calcTotalUsersAndNotify(BaseUser user, UserUpdateType type) {
        BaseUser localUser = getLocalUser();
        if (localUser == null || user == null) {
            return;
        }

        boolean isOneItemListeners = isListenersOnOneItem(user, localUser);

        int oldTotalUser = totalUser;
        switch (type) {
            case USER_ADD:
                calcTotalUserWhenAdd(user, isOneItemListeners);
                break;

            case USER_UPDATE:
                calcTotalUserWhenUpdate(user, isOneItemListeners);
                break;

            case USER_REMOVE:
                totalUser--;
                break;

            default:
                break;
        }

        if (oldTotalUser != totalUser) {
            notifyUserCountChangedWitchCheckMainThread();
        }
    }

    private void calcTotalUserWhenUpdate(BaseUser user, boolean isOneItemListeners) {
        if (isOneItemListeners) {
            int listenerNum = Integer.parseInt(user.getNickName().split(":")[1]);            
            if (listenerNum == 1) { // 旁听人数：1 这种情况要要减去这一个人数
                totalUser = users.size() - listenerNum;
            } else { // -2去除重复，因为本地用户和“旁听人数：x”都已经在参会人列表中作为一个单独的人计算
                totalUser = listenerNum - 2 + users.size();
            }
        }
    }

    private void calcTotalUserWhenAdd(BaseUser user, boolean isOneItemListeners) {
        if (isOneItemListeners) {
            // 修复BUG：14531；在UserModel初始化的时候已经把本地用户计算到totalUser内，
            // “旁听人数：” 冒号后面的数字代表所有的旁听，包括了本地用户自己。为了防止重复添加，所以要减1。
            totalUser += (Integer.parseInt(user.getNickName().split(":")[1]) - 1);
        } else {
            totalUser++;
        }
    }

    /**
     * 判断旁听用户是否全部在作为一个用户处理
     */
    private boolean isListenersOnOneItem(BaseUser user, BaseUser localUser) {
        boolean isOneItemListeners = false;

        // 只有本地用户是旁听时，才对旁听用户做处理
        if (localUser.isListener()) {
            if (user.getNickName().length() > LISTENERS_NUMBER.length()) {
                String subUserName = user.getNickName().substring(0, 5);
                if (subUserName.equals(LISTENERS_NUMBER)) {
                    isOneItemListeners = true;
                }
            }
        }
        return isOneItemListeners;
    }

    private void updateMainSpeakAfterUpdateUsers(BaseUser user) {
        if (user.isMainSpeakerDone()) {
            mainSpeakerUser = user;
        } else if (mainSpeakerUser != null && mainSpeakerUser == user) {
            mainSpeakerUser = null;
        }
    }

    /**
     * 只要有用户状态的改变就会调用该方法
     *
     * @param user 发生改变的用户
     * @param type 变更类型
     */
    private void sendUserChangedMsg(BaseUser user, UserUpdateType type) {
        userHandler.removeMessages(UserHandler.NOTIFY_USERS_CHANGED);
        userHandler.sendEmptyMessage(UserHandler.NOTIFY_USERS_CHANGED);
    }

    /**
     * 获取在线人数
     */
    public int getOnlineUserNumber() {
        return totalUser;
    }

    /**
     * 返回当前会议室参会人列表
     */
    public ArrayList<BaseUser> getUsers() {
        ArrayList<BaseUser> usersList = new ArrayList<>();
        usersList.addAll(users.values());
        return usersList;
    }

    /**
     * 返回指定Id的参会人
     */
    public BaseUser getUserById(long userId) {
        return users.get(userId);
    }

    /**
     * 用户是否在会议室中
     */
    public boolean isUserInMeeting(BaseUser user) {
        return getUserById(user.getUserID()) != null;
    }

    /**
     * 当localuser为null 或者UserManager.getInstance().getLocalUser()返回为空则返回true。
     * 使用该方法用于判断应用是否被系统回收JNI层或者JAVA层
     */
    public boolean isLocalUserDisable() {
        return (getLocalUser() == null || UserManager.getInstance().getLocalUser() == null);
    }

    /**
     * 获取当前本地用户
     */
    public BaseUser getLocalUser() {
        // 如果为空尝试从列表中重新获取一次
        if (localUser == null) {
            RoomUserInfo localUserInfo = UserManager.getInstance().getLocalUser();
            if (localUserInfo != null) {
                // 从保存的map中拿本地用户，如果不存在则add到列表中
                BaseUser tempUser = getUserById(localUserInfo.userID);
                if (tempUser == null) {
                    tempUser = addUser(localUserInfo);
                }

                localUser = tempUser;
            }
        }

        return localUser;
    }

    static void releaseObj() {
        if (instance != null) {
            instance.deleteObservers();
            instance.users.clear();
            instance.listeners.clear();
            instance = null;
        }
    }

    /**
     * 通知用户状态发生改变的Handler，此处引入handler的原因是防止大并发时短时间用户的频繁变化导致UI刷新过多的问题 。
     * 同样使用该种方式广播用户，也可能一些变化的用户还没有来的及广播就已经被新的变化用户给覆盖了。
     * 基于以上原因现在通知用户发生变化，只携带当前用户列表参数，不携带用户更新类型。
     */
    private static class UserHandler extends Handler {
        static final int NOTIFY_USERS_CHANGED = 1;

        @Override
        public void handleMessage(Message msg) {
            if (msg.what == NOTIFY_USERS_CHANGED) {
                UserModel userModel = UserModel.getInstance();
                if (userModel != null) {
                    userModel.setChanged();
                    userModel.notifyObservers(userModel.getUsers());
                }
            }
        }
    }

    /**
     * 初始化会议时，当在子线程加入本地用户时，需要切换到主线程推送参会人数变更
     */
    private void notifyUserCountChangedWitchCheckMainThread() {
        if (ScreenUtil.isMainThread()) {
            notifyUserCountChanged();
        } else {
            userHandler.post(new Runnable() {
                @Override
                public void run() {
                    notifyUserCountChanged();
                }
            });
        }
    }

    /**
     * 初始化会议时，当在子线程更新本地用户时，需要切换到主线程推送用户状态发生改变
     */
    private void notifyUserChangedWitchCheckMainThread(final BaseUser user) {
        if (ScreenUtil.isMainThread()) {
            notifyUserChanged(user);
        } else {
            userHandler.post(new Runnable() {
                @Override
                public void run() {
                    notifyUserChanged(user);
                }
            });
        }
    }

    void notifyUserAudioChanged(long userId) {
        BaseUser user = getUserById(userId);
        for (IUserStateChangedListener listener : listeners) {
            if (listener != null) {
                listener.onUserAudioChanged(user);
            }
        }
    }

    void notifyUserVideoChanged(long userId) {
        BaseUser user = getUserById(userId);
        for (IUserStateChangedListener listener : listeners) {
            if (listener != null) {
                listener.onUserVideoChanged(user);
            }
        }
    }

    void notifyUserMainSpeakerChanged(long userId) {
        BaseUser user = getUserById(userId);
        for (IUserStateChangedListener listener : listeners) {
            if (listener != null) {
                listener.onUserMainSpeakerChanged(user);
            }
        }
    }

    void notifyUserCountChanged() {
        for (IUserStateChangedListener listener : listeners) {
            if (listener != null) {
                listener.onMeetingUserNumberChanged(totalUser);
            }
        }
    }

    void notifyUserWBMarkChanged(long userId) {
        BaseUser user = getUserById(userId);
        for (IUserStateChangedListener listener : listeners) {
            if (listener != null) {
                listener.onUserWBMarkChanged(user);
            }
        }
    }

    void notifyUserChanged(BaseUser user) {
        for (IUserStateChangedListener listener : listeners) {
            if (listener != null) {
                listener.onUserChanged(user);
            }
        }
    }

    /**
     * 用户状态接口
     */
    public interface IUserStateChangedListener {
        /**
         * 本地用户音频状态发生改变
         */
        void onUserAudioChanged(BaseUser localUser);

        /**
         * 本地用户视频状态发生改变
         */
        void onUserVideoChanged(BaseUser localUser);

        /**
         * 本地用户主讲状态发生变化
         */
        void onUserMainSpeakerChanged(BaseUser localUser);

        /**
         * 会议室人数发生变化
         */
        void onMeetingUserNumberChanged(int totalUser);

        /**
         * 用户标注白板权限发生变化
         */
        void onUserWBMarkChanged(BaseUser user);

        /**
         * 用户信息发生变化，包括email,手机，性别，昵称。
         */
        void onUserChanged(BaseUser user);
    }

    /**
     * 便捷的本地状态发生改变的监听实现类
     */
    public static class SimpleUserStateChangedListener implements IUserStateChangedListener {

        @Override
        public void onUserAudioChanged(BaseUser user) {}

        @Override
        public void onUserVideoChanged(BaseUser user) {}

        @Override
        public void onUserMainSpeakerChanged(BaseUser user) {}

        @Override
        public void onMeetingUserNumberChanged(int totalUser) {}

        @Override
        public void onUserWBMarkChanged(BaseUser user) {}

        @Override
        public void onUserChanged(BaseUser user) {}
    }

    /**
     * 添加用户状态变化监听器
     */
    public void addUserStateChangedListener(IUserStateChangedListener listener) {
        if (!listeners.contains(listener)) {
            listeners.add(listener);
        }
    }

    /**
     * 移除用户状态变化监听器
     */
    public void removeUserStateChangedListener(IUserStateChangedListener listener) {
        listeners.remove(listener);
    }
}
