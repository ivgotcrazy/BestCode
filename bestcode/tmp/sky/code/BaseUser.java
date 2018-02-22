
/**
 * 会议用户类型基类
 * Created by wangy on 2017/1/16.
 */
public class BaseUser {

    private static final int LOCAL_USER_PRIORITY = 100;
    private static final int MAIN_SPEAKER_PRIORITY = 99;
    private static final int TELEPHONE_USER_PRIORITY = -100;

    protected RoomUserInfo userInfo;

    /**
     * 构造器
     * */
    public BaseUser(RoomUserInfo userInfo) {
        this.userInfo = userInfo;
    }

    public void setRoomInfo(RoomUserInfo roomUserInfo) {
        userInfo = roomUserInfo;
    }

    /**
     * 检查是否有广播音频的权限
     */
    public boolean checkAudioPermission() {
        return !isListener();
    }

    /**
     * 检查是否有广播音频的权限
     */
    public boolean checkVideoPermission() {
        return !isListener() && !MeetingModel.getInstance().isAudioMeeting();
    }

    /**
     * 检查是否有申请主讲的权限
     */
    public boolean checkMainSpeakPermission() {
        return !isListener();
    }

    /**
     * 检查音频频权限
     */
    public boolean checkAVPermission() {
        return checkVideoPermission() && checkAudioPermission();
    }

    /**
     * 检查是否具有公聊权限
     */
    public boolean checkPubChatPermission() {
        MeetingModel roomInfoModel = MeetingModel.getInstance();
        return roomInfoModel.chatEnable() && roomInfoModel.pubChatEnable() && isChatEnable();
    }

    /**
     * 检查是否具有私聊权限
     */
    public boolean checkP2pChatPermission() {
        MeetingModel roomInfoModel = MeetingModel.getInstance();
        return roomInfoModel.chatEnable() && roomInfoModel.p2pChatEnable() && isChatEnable();
    }

    /**
     * 检查是否有控制远程用户的权限，如发言，视频广播，其实际就是当前用户是主讲或者主席与{@checkHighestPermission()}方法返回一致
     */
    public boolean checkControlRemotePermission() {
        return checkHighestPermission();
    }

    /**
     * @return 请出会议室权限：只有主席才有权限
     */
    public boolean checkRemoveUserPermission() {
        return userInfo.userRight == RoomUserInfo.USERRIGHT_CHAIR;
    }

    /**
     * 检查是否有共享文档的权限
     *
     * @return 只有非旁听用户才具有共享文档的权限
     */
    public boolean checkSharePermission() {
        return checkMainSpeakPermission();
    }

    /**
     * 用户是否具有最高权限，即是主席或者主讲
     */
    public boolean checkHighestPermission() {
        return isChair() || isMainSpeakerDone();
    }

    public boolean isMainSpeakerDone() {
        return userInfo.dataState == RoomUserInfo.STATE_DONE;
    }

    public boolean isMainSpeakerWait() {
        return userInfo.dataState == RoomUserInfo.STATE_WAITING;
    }

    /**
     * 不在主讲状态，不包含申请主讲中状态
     */
    public boolean isMainSpeakerNone() {
        return userInfo.dataState == RoomUserInfo.STATE_NONE;
    }

    /**
     * 是否在主讲或者申请主讲状态中
     */
    public boolean isMainSpeakerDoneOrWait() {
        return !isMainSpeakerNone();
    }

    /**
     * 是否在共享
     */
    public boolean isVncDone() {
        return userInfo.vncState == RoomUserInfo.STATE_DONE;
    }

    public boolean isMediaShareDone() {
        return (userInfo.audioShareID != 0 || userInfo.videoShareID != 0)
                && userInfo.mediaShareState != RoomUserInfo.MEDIA_SHARE_PAUSE;
    }

    public boolean isMediaSharePause() {
        return userInfo.mediaShareState == RoomUserInfo.MEDIA_SHARE_PAUSE;
    }

    public boolean isMediaShareNone() {
        return (userInfo.audioShareID == 0 && userInfo.videoShareID == 0);
    }

    /**
     * 是否是电话接入用户
     */
    public boolean isTelephoneUser() {
        return userInfo.terminalType == RoomUserInfo.TERMINAL_TELEPHONE;
    }

    /**
     * 是否有接入的视屏设备
     */
    public boolean isVideoDeviceConnected() {
        if (userInfo.vclmgr == null) {
            return false;
        }
        return userInfo.vclmgr.getChannelCount() > 0;
    }

    /**
     * 是否有音频设备接入
     */
    public boolean isAudioDeviceConnected() {
        return userInfo.audioChannel.hasAudio == RoomUserInfo.STATE_BOOL_TRUE;
    }

    public boolean isVncNone() {
        return userInfo.vncState == RoomUserInfo.STATE_NONE;
    }

    /**
     * 是否是本地用户
     */
    public final boolean isLocalUser() {
        return UserManager.getInstance().getLocalUserID() == userInfo.userID;
    }

    public VideoChannelManager getVideoManager() {
        return userInfo.vclmgr;
    }

    public long getUserID() {
        return this.userInfo.userID;
    }

    /**
     * 获取用户昵称，如果昵称为空，则返回用户名称
     */
    public String getNickName() {
        if (userInfo.strNickName == null) {
            return userInfo.strUserName;
        }

        return userInfo.strNickName;
    }

    /**
     * 获取已经接入的视频设备列表
     */
    public ArrayList<VideoChannel> getVideoChannelList() {
        if (userInfo.vclmgr == null) {
            return new ArrayList<>();
        }

        return userInfo.vclmgr.getChannelList();
    }

    /**
     * 是否是主席用户
     */
    public boolean isChair() {
        return userInfo.userRight == RoomUserInfo.USERRIGHT_CHAIR;
    }

    /**
     * 是否是旁听用户
     */
    public boolean isListener() {
        return userInfo.userRight == RoomUserInfo.USERRIGHT_LISTENER;
    }

    /**
     * 是否是出席用户
     */
    public boolean isAttender() {
        return userInfo.userRight == RoomUserInfo.USERRIGHT_ATTENDER;
    }

    /**
     * 是否正在发言中
     */
    public boolean isSpeechDone() {
        return userInfo.audioChannel.state == AudioChannel.AUDIO_STATE_DONE;
    }

    /**
     * 是否正在申请发言
     */
    public boolean isSpeechWait() {
        return userInfo.audioChannel.state == AudioChannel.AUDIO_STATE_WAITING;
    }

    /**
     * 未发言状态，不包含申请中状态
     */
    public boolean isSpeechNone() {
        return userInfo.audioChannel.state == AudioChannel.AUDIO_STATE_NONE;
    }

    /**
     * 是否在广播视频，只要有一路视频在广播，则返回true
     */
    public boolean isVideoDone() {
        return userInfo.vclmgr != null && userInfo.vclmgr.hasStateDone();

    }

    /**
     * 没有视频在广播
     */
    public boolean isVideoNone() {
        return userInfo.vclmgr == null || !userInfo.vclmgr.hasStateDone();
    }

    /**
     * 是否有白板标注权限
     */
    public boolean isWBMarkDone() {
        return userInfo.wbMarkState == RoomUserInfo.STATE_DONE;
    }

    /**
     * 获取用户优先级Id，
     * 用于排序，本地用户 > 主讲 > 主席 > 出席 > 旁听 > 电话用户
     */
    public int getUserPriority() {
        if (isTelephoneUser()) {
            return TELEPHONE_USER_PRIORITY;
        } else if (isLocalUser()) {
            return LOCAL_USER_PRIORITY;
        } else if (isMainSpeakerDone()) {
            return MAIN_SPEAKER_PRIORITY;
        } else {
            return userInfo.userRight;
        }
    }

    /**
     * 用户本身是否具有聊天的权限
     */
    private boolean isChatEnable() {
        return userInfo.enableChat == RoomInfo.STATE_DONE;
    }

    /**
     * 指定的视频channelId的视频是否在广播
     * */
    public boolean isVideoChannelDone(byte channelId) {
        return userInfo.vclmgr != null && userInfo.vclmgr.getChannelState(channelId) == RoomUserInfo.STATE_DONE;
    }
}
