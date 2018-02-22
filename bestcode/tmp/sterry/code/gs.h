#pragma once

#include <functional>
#include <map>
#include <string>

#include "xmlprotocolparser.h"

#include "component_mgr.h"
#include "kafka_msg_mgr.h"
#include "thread.h"

#include "context_def.h"
#include "group_mgr.h"
#include "media_mgr.h"
#include "timer_mgr.h"
#include <mutex>

#include "logger/printers/trace_printer.h"
#include "fsp-common.pb.h"
#define MAX_SESSION_RETRY_REQ_TIMES 3

namespace fsp
{
namespace sss
{
namespace gs
{
using common::KafkaMsgMgr;
using common::KafkaMsg;
constexpr std::chrono::milliseconds SESSION_REQUEST_TIMEOUT{60 * 1000};


// TODO: send sending_start/sending_stop req when necessary
// TODO: close the sessions to superior when necessary
// (note: OnSessionClosed, OnLogout, OnLoginRecvChannelRsp)
class SessionMsgProcessor final
{
public:
    SessionMsgProcessor(util::Thread &thread, // NOLINT(runtime/references)
                        WBASE_NOTIFY &notify, // NOLINT(runtime/references)
                        util::TracePrinter &trace) // NOLINT(runtime/references)
        : m_thread(thread),
          m_notify(notify),
          m_trace(trace)
    {
        InitMsgHandlers();
        m_media_mgr.SetProcessor(this);
        m_timer_mgr.Start();
    }

public:
    void OnSessionEvent(const SESSION_EVENT &event);
    void OnKafkaMsg(KafkaMsg &&msg);
    void DestroyClientSrc(const MediaTuple &media_tuple, uint16_t session_id,
                               uint32_t channel_id, uint32_t channel_check_code);
    void DestroySSSrc(uint16_t session_id);
    void StatisticsConcurrency();

private:
    using OnSessionResp = std::function<void()>;
    using OnChannelResp = std::function<void()>;
    enum SessionStates
    {
        SESSION_STATES_CREATED,
        SESSION_STATES_LOGGING_IN,
        SESSION_STATES_JOINED_GROUP,
        SESSION_STATES_CLIENT_CONNECTED,
        SESSION_STATES_LOGGED_IN,
    };
    struct SessionData final
    {
        std::string stream_id;
        std::string client_id;
        SessionStates session_state = SESSION_STATES_CREATED;
    };
    struct SignalSessionData final
    {
        uint32_t user_id;
        std::string group_id;
        SessionStates session_state = SESSION_STATES_CREATED;
    };
    struct MediaSessionData final
    {
        SessionStates session_state = SESSION_STATES_CREATED;
    };
    struct StreamInfo final
    {
        MediaTuple media_tuple;
        std::map<std::string /*client_id*/, uint32_t /*channel_id*/> channel_table;
        uint64_t seq;
        uint16_t session_id = 0;
    };
    struct RecvLoginChannelInfo final
    {
        uint16_t channel_session_id;
        ChannelType2  channel_type;
        uint64_t seq = 0;
        std::string src;
    };
    struct SrcChannelInfo final
    {
        uint16_t superior_session_id = 0;
        uint32_t src_channel_id = 0;
    };
    struct ChannelTimerInfo final
    {
        int64_t timer_id;
        OnChannelResp onResp;
    };
    struct SessionTimerInfo final
    {
        int64_t timer_id;
        OnSessionResp onResp;
    };

    using SessionMsgHandler = std::function<void(uint16_t session_id, const uint8_t *data,
                                                 uint32_t len, uint64_t, uint64_t)>;
    using KafkaMsgHandler = std::function<void(KafkaMsg &&msg)>;

private:
    void InitMsgHandlers();

private:
    void OnSessionCreated(const SESSION_EVENT &event);
    void OnSessionCreateFailed(const SESSION_EVENT &event);
    void OnSessionClosed(const SESSION_EVENT &event);
    void OnSessionMsg(const SESSION_EVENT &event);
    void OnSessionReconnecting(const SESSION_EVENT &event);
    void OnSessionReconnected(const SESSION_EVENT &event);

    // PB msg handlers
private:
    void OnLoginRecvChannelReq(uint16_t session_id, const uint8_t *data, uint32_t len, uint64_t,
                               uint64_t);
    void OnLoginRecvChannelRsp(uint16_t session_id, const uint8_t *data, uint32_t len, uint64_t,
                               uint64_t);
    void OnLogout(uint16_t session_id, const uint8_t *data, uint32_t len, uint64_t channel_id,
                  uint64_t channel_type);
    void OnStreamData(uint16_t session_id, const uint8_t *data, uint32_t len,
                      uint64_t channel_id, uint64_t channel_type);

    // XML msg handlers
private:
    void OnLoginReq(uint16_t session_id, const uint8_t *data, uint32_t len, uint64_t, uint64_t);
    void OnRecvReq(uint16_t session_id, const uint8_t *data, uint32_t len, uint64_t, uint64_t);
    void OnSendEnableRsp(uint16_t session_id, const uint8_t *data, uint32_t len, uint64_t,
                         uint64_t);
    void OnPauseRecvReq(uint16_t session_id, const uint8_t *data, uint32_t len, uint64_t,
                        uint64_t);
    void OnBye(uint16_t session_id, const uint8_t *data, uint32_t len, uint64_t, uint64_t);

    // raw msg handlers
private:
    void OnSendLoginReq(uint16_t session_id, const uint8_t *data, uint32_t len, uint64_t,
                        uint64_t);
    void OnRecvLoginReq(uint16_t session_id, const uint8_t *data, uint32_t len, uint64_t,
                        uint64_t);
    void OnSendBye(uint16_t session_id, const uint8_t *data, uint32_t len, uint64_t channel_id,
                   uint64_t channel_type);
    void OnRecvBye(uint16_t session_id, const uint8_t *data, uint32_t len, uint64_t channel_id,
                   uint64_t channel_type);
    void OnAVData(uint16_t session_id, const uint8_t *data, uint32_t len, uint64_t channel_id,
                  uint64_t channel_type);
    void OnQOSData(uint16_t session_id, const uint8_t *data, uint32_t len, uint64_t channel_id,
                   uint64_t channel_type);

private:
    void OnNotifyStreamSendingStart(KafkaMsg &&msg);
    void OnNotifyStreamSendingStop(KafkaMsg &&msg);
    void OnNotifyPublishStream(KafkaMsg &&msg);
    void OnNotifyStreamDestroied(KafkaMsg &&msg);
    void OnNotifyStreamSourceChanged(KafkaMsg &&msg);

private:
    void SendCheckPublishTokenReq(KafkaMsgMgr::OnResp &&on_resp,
                                  LoginContextHandler context_handler);
    void OnCheckPublishTokenRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                                LoginContextHandler context_handler);
    void SendCheckSubscribeTokenReq(KafkaMsgMgr::OnResp &&on_resp,
                                    LoginContextHandler context_handler);
    void OnCheckSubscribeTokenRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                                  LoginContextHandler context_handler);
    void SendGetStreamTypeReq(KafkaMsgMgr::OnResp &&on_resp,
                              LoginContextHandler context_handler);
    void OnGetStreamTypeRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                            LoginContextHandler context_handler);
    void SendChannelConnectedReq(KafkaMsgMgr::OnResp &&on_resp,
                                 LoginContextHandler context_handler);
    void OnChannelConnectedRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                               LoginContextHandler context_handler);
    void SendChannelDisconnectedReq(const std::string &stream_id, const std::string &client_id,
                                    KafkaMsgMgr::OnResp &&on_resp);
    void OnChannelDisconnectedRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                                  const std::string &stream_id, const std::string &client_id);
    void SendSendingStartReq(KafkaMsgMgr::OnResp &&on_resp, RecvContextHandler context_handler);
    void OnSendingStartRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                           RecvContextHandler context_handler);
    void SendPublishStreamReq(KafkaMsgMgr::OnResp &&on_resp, const std::string &stream_id,
                              const std::string &group_id, const uint32_t user_id);
    void OnPublishStreamRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                            const std::string &stream_id);
    void SendSetStreamSourceServerReq(KafkaMsgMgr::OnResp &&on_resp,
                                      const std::string &stream_id);
    void OnSetStreamSourceServerRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                                    const std::string &stream_id);
    void SendNotifyStreamPublishedReq(         const std::string &stream_id);
    void OnNotifyStreamPublishedRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                                    const std::string &stream_id);

private:
    void PrepareToCreateSrcChannel(uint32_t channel_id, /*bool is_src_server,*/
                                   uint16_t session_id);
    void SendJoinGroupReq(KafkaMsgMgr::OnResp &&on_resp,
                          UserLoginContextHandler context_handler);
    void OnJoinGroupRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                        UserLoginContextHandler context_handler);
    void SendClientConnectedReq(KafkaMsgMgr::OnResp &&on_resp,
                                UserLoginContextHandler context_handler);
    void OnClientConnectedRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                              UserLoginContextHandler context_handler);
    void SendGetStreamReq(KafkaMsgMgr::OnResp &&on_resp, RecvContextHandler context_handler);
    void OnGetStreamRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                        RecvContextHandler context_handler);
    void OnCreateMediaSessionTimeOut(uint16_t session_id);
    void SendGetSuperiorReq(KafkaMsgMgr::OnResp &&on_resp, RecvContextHandler context_handler);
    void OnGetSuperiorRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                          RecvContextHandler context_handler);
    void SendStreamDestroiesReq(KafkaMsgMgr::OnResp &&on_resp, const std::string &stream_id);
    void OnSendStreamDestroiesRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                        const std::string &stream_id);
    void SendQuitGroupReq(KafkaMsgMgr::OnResp &&on_resp, uint16_t session_id);
    void OnQuitGroupRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                        uint16_t session_id);
    void SendClientDisConnectedReq(KafkaMsgMgr::OnResp &&on_resp, uint16_t session_id);
    void OnClientDisConnectedRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                        uint16_t session_id);

private:
    void SendLoginRecvChannelReq(uint16_t session_id, const std::string &stream_id);
    void WaitForLoginRecvChannelRspTimeOut(uint16_t session_id);
    void SendLoginSendChannelRsp(ResponseCode err_code, uint16_t session_id);
    void SendLoginRecvChannelRsp(ResponseCode err_code, uint16_t session_id);
    void SendLoginRsp(const std::string &group_id, int32_t user_id, ResponseCode err_code,
                      uint16_t session_id);
    void SendRecvRsp(const MediaTuple &media_tuple, int32_t user_id, uint32_t channel_id,
                     uint32_t channel_check_code, int32_t recv, uint16_t session_id, int32_t result = 0);
    void SendRecvLoginRsp(uint32_t channel_id, ResponseCode err_code, uint16_t session_id);
    void SendSendLoginRsp(uint32_t channel_id, ResponseCode err_code, uint16_t session_id);
    void SendPauseRecvRsp(const MediaTuple &media_tuple, int32_t user_id,
                                    int32_t pause, uint16_t session_id, int32_t result = 0);
private:
    void SendNotifyStreamSendingStartRsp(const std::string &msg_src, uint64_t msg_seq,
                                         ResponseCode err_code);
    void SendNotifyStreamSendingStopRsp(const std::string &msg_src, uint64_t msg_seq,
                                        ResponseCode err_code);
    void SendSendEnableReq(const MediaTuple &media_tuple, uint32_t channel_id,
                           uint32_t channel_check_code, bool send, uint16_t session_id);
    void WaitForSendLoginReqTimeOut(uint32_t channel_id);
    void SendStreamDestroiesRsp(const std::string &msg_src, uint64_t msg_seq,
                                ResponseCode err_code);

private:
    void OnLoginSendContextDestroy(LoginContext &context);     // NOLINT(runtime/references)
    void OnLoginRecvContextDestroy(LoginContext &context);     // NOLINT(runtime/references)
    void OnUserLoginContextDestroy(UserLoginContext &context); // NOLINT(runtime/references)

private:
    TiXmlElement &LoadCommand(const uint8_t *data, uint32_t len);

private:
    LoginContextHandler MakeLoginContext(uint16_t session_id, const std::string &stream_id,
                                         const std::string &stream_token,
                                         const std::string &client_id, ChannelType2 channel_type,
                                         uint32_t channel_id,
                                         Context<LoginContext>::OnDestroy &&on_destroy);
    void SetSessionState(uint16_t session_id, bool session_logged_in);

    UserLoginContextHandler MakeUserLoginContext(
        uint16_t session_id, const std::string &group_id, const std::string &group_token,
        int32_t user_id, Context<UserLoginContext>::OnDestroy &&on_destroy);
    void SetSignalSessionState(uint16_t session_id, bool session_logged_in);

    void OnRecvContextDestroy(RecvContext &context); // NOLINT(runtime/references)
    RecvContextHandler MakeRecvContext(const MediaTuple &media_tuple,
                                       Context<RecvContext>::OnDestroy &&on_destroy);
    void CloseAllClientMediaSession(uint32_t user_id, const std::string &group_id);

private:
    void CleanAllResources(const std::string &stream_id);
    void OnChannelTimer(uint64_t channel_id, int64_t timer_id);
    void OnSessionTimer(uint64_t session_id, int64_t timer_id);
    void RemoveChannelTimer(uint32_t channel_id);
    void RemoveSessionTimer(uint16_t session_id);
    void AddChannelTimer(uint32_t channel_id, OnChannelResp &&resp);
    void AddSessionTimer(uint16_t session_id, OnSessionResp &&resp);
    void UserLogout(uint16_t session_id);

private:
    util::Thread &m_thread;
    WBASE_NOTIFY &m_notify;

    GroupMgr m_group_mgr;
    MediaMgr m_media_mgr;

    std::map<std::string /*stream_id*/, StreamInfo> m_stream_table;

    std::map<int32_t /*msg_type*/, SessionMsgHandler> m_session_pb_msg_handlers;
    std::map<int32_t /*msg_type*/, SessionMsgHandler> m_session_xml_msg_handlers;
    std::map<int32_t /*msg_type*/, SessionMsgHandler> m_session_raw_msg_handlers;
    std::map<int32_t /*msg_type*/, KafkaMsgHandler> m_kafka_msg_handlers;

    std::map<uint16_t /*session_id*/, SessionData> m_logged_in_sessions;
    std::map<uint16_t /*session_id*/, SignalSessionData> m_logged_in_signal_sessions;
    std::map<uint16_t /*session_id*/, MediaSessionData> m_logged_in_media_sessions;
    std::map<uint16_t /*session_id*/, RecvContextHandler>
        m_creating_session_context_table; // TODO: erase when login_recv_channel rsp timeout
    std::map<uint32_t /*channel_id*/, RecvContextHandler>
        m_recv_context_table; // TODO: erase when send_login req timeout
    std::map<MediaTuple, std::map<uint32_t /*channel_id*/, RecvLoginChannelInfo>>
        m_recv_login_reqs_table;
    util::TimerMgr m_timer_mgr;
    std::map<std::string/*stream_id*/, SrcChannelInfo> m_stream_src_channel;
    std::map<uint32_t /*channel_id*/, ChannelTimerInfo> m_channel_resps;
    std::map<uint16_t /*session_id*/, SessionTimerInfo> m_session_resps;
    std::mutex m_mutex;

private:
    //print trace log
    std::map<uint16_t/*session_id*/, TraceLog /*trace*/>m_login_req_trace;
    std::map<uint32_t/*channel_id*/, TraceLog/*trace*/>m_recv_req_trace;
    std::map<uint64_t/*msg_req*/, TraceLog/*trace*/>m_notify_trace;
    std::map<uint16_t/*session_id*/, TraceLog/*trace*/>m_loging_recving_channel;
    std::map<std::string/*stream_id*/, TraceLog/*trace*/>m_notify_publish_stream;
    std::map<uint32_t /*src_channel_id*/, Stream /*stream*/>m_src_channel_stream;
    util::TracePrinter &m_trace;
};
}
}
}
