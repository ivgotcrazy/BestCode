#include "session_msg_processor.h"
#include <arpa/inet.h>
#include "multiavprotocol.h"
#include "fsp-gc.pb.h"
#include "fsp-gs.pb.h"
#include "fsp-sc.pb.h"
#include "fsp-ss.pb.h"
#include "global_objects.h"
#include "misc/util.h"
#include "utility.h"
#include "fsp-common.pb.h"
#include "config.h"

#define BIND_SESSION_HANDLER(type, method)                                      \
    {                                                                           \
        type, std::bind(&SessionMsgProcessor::method, this, _1, _2, _3, _4, _5) \
    }

#define BIND_KAFKA_HANDLER(type, method)                        \
    {                                                           \
        type, std::bind(&SessionMsgProcessor::method, this, _1) \
    }

#define BIND_CALLBACK(method, context_handler)                                    \
    [this, context_handler](KafkaMsgMgr::RespErrno err, KafkaMsg &&msg) {         \
        m_thread.PostThreadMsg(std::bind(&SessionMsgProcessor::method, this, err, \
                                         std::move(msg), context_handler));       \
    }
#define BIND_CALLBACK_2(method, stream_id, client_id)                             \
    [this, stream_id, client_id](KafkaMsgMgr::RespErrno err, KafkaMsg &&msg) {    \
        m_thread.PostThreadMsg(std::bind(&SessionMsgProcessor::method, this, err, \
                                         std::move(msg), stream_id, client_id));  \
    }
#define BIND_CALLBACK_3(method, stream_id)                             \
        [this, stream_id](KafkaMsgMgr::RespErrno err, KafkaMsg &&msg) {    \
            m_thread.PostThreadMsg(std::bind(&SessionMsgProcessor::method, this, err, \
                                             std::move(msg), stream_id));  \
        }

#define BIND_CALLBACK_SESSION(method, session_id)                                           \
    [this, session_id]() {                                                                  \
        m_thread.PostThreadMsg(std::bind(&SessionMsgProcessor::method, this, session_id));  \
    }
#define BIND_CALLBACK_CHANNEL(method, channel_id)                                           \
    [this, channel_id]() {                                                                  \
        m_thread.PostThreadMsg(std::bind(&SessionMsgProcessor::method, this, channel_id));  \
    }

#ifdef LOCK
#undef LOCK
#endif
#define LOCK() std::lock_guard<std::mutex> lock(m_mutex);


namespace fsp
{
namespace sss
{
namespace gs
{
using namespace ::com::fsmeeting::fsp::proto::gc;     // NOLINT(build/namespaces)
using namespace ::com::fsmeeting::fsp::proto::sc;     // NOLINT(build/namespaces)
using namespace ::com::fsmeeting::fsp::proto::ss;     // NOLINT(build/namespaces)
using namespace ::com::fsmeeting::fsp::proto::gs;     // NOLINT(build/namespaces)
using namespace ::com::fsmeeting::fsp::proto::common; // NOLINT(build/namespaces)

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;
using std::placeholders::_5;
using common::UnserializePBMsg;
using common::SendSessionMsg;
using common::SendKafkaMsg;
using common::ReplyKafkaMsg;

using ::com::fsmeeting::fsp::proto::gs::Enum2NotifyStreamSendingStart;
using ::com::fsmeeting::fsp::proto::gs::Enum2NotifyStreamSendingStartRsp;
using ::com::fsmeeting::fsp::proto::gs::Enum2NotifyStreamSendingStop;
using ::com::fsmeeting::fsp::proto::gs::Enum2NotifyStreamSendingStopRsp;
using ::com::fsmeeting::fsp::proto::gs::NotifyStreamSendingStart;
using ::com::fsmeeting::fsp::proto::gs::NotifyStreamSendingStartRsp;
using ::com::fsmeeting::fsp::proto::gs::NotifyStreamSendingStop;
using ::com::fsmeeting::fsp::proto::gs::NotifyStreamSendingStopRsp;

void SessionMsgProcessor::InitMsgHandlers()
{
    m_session_pb_msg_handlers = {
        BIND_SESSION_HANDLER(Enum2LoginReceivingChannel, OnLoginRecvChannelReq),
        BIND_SESSION_HANDLER(Enum2LoginReceivingChannelRsp, OnLoginRecvChannelRsp),
        BIND_SESSION_HANDLER(Enum2Logout, OnLogout),
        BIND_SESSION_HANDLER(Enum2StreamData, OnStreamData),
    };
    m_session_xml_msg_handlers = {
        BIND_SESSION_HANDLER(MULTIAV_CMDID_LOGINREQ, OnLoginReq),
        BIND_SESSION_HANDLER(MULTIAV_CMDID_RECVREQ, OnRecvReq),
        BIND_SESSION_HANDLER(MULTIAV_CMDID_SENDENABLEREP, OnSendEnableRsp),
        BIND_SESSION_HANDLER(MULTIAV_CMDID_PAUSERECVREQ, OnPauseRecvReq),
        BIND_SESSION_HANDLER(MULTIAV_CMDID_BYE, OnBye),
    };
    m_session_raw_msg_handlers = {
        BIND_SESSION_HANDLER(MULTIAV_CMDID_SENDLOGINREQ, OnSendLoginReq),
        BIND_SESSION_HANDLER(MULTIAV_CMDID_RECVLOGINREQ, OnRecvLoginReq),
        BIND_SESSION_HANDLER(MULTIAV_CMDID_SENDBYE, OnSendBye),
        BIND_SESSION_HANDLER(MULTIAV_CMDID_RECVBYE, OnRecvBye),
        BIND_SESSION_HANDLER(MULTIAV_CMDID_AVDATA, OnAVData),
        BIND_SESSION_HANDLER(MULTIAV_CMDID_QOSDATA, OnQOSData),
    };
    m_kafka_msg_handlers = {
        BIND_KAFKA_HANDLER(Enum2NotifyStreamSendingStart, OnNotifyStreamSendingStart),
        BIND_KAFKA_HANDLER(Enum2NotifyStreamSendingStop, OnNotifyStreamSendingStop),
        BIND_KAFKA_HANDLER(::com::fsmeeting::fsp::proto::ss::Enum2NotifyStreamSendingStart,
                           OnNotifyStreamSendingStart),
        BIND_KAFKA_HANDLER(::com::fsmeeting::fsp::proto::ss::Enum2NotifyStreamSendingStop,
                           OnNotifyStreamSendingStop),
        BIND_KAFKA_HANDLER(Enum2NotifyPublishStream, OnNotifyPublishStream),
        BIND_KAFKA_HANDLER(Enum2NotifyStreamDestroied, OnNotifyStreamDestroied),
        BIND_KAFKA_HANDLER(Enum2StreamSourceChanged, OnNotifyStreamSourceChanged),
    };
}

void SessionMsgProcessor::OnSessionEvent(const SESSION_EVENT &event)
{
    switch (event.nEventType)
    {
    case SESSION_EVENT_ACCEPT:
        INFO("session(%u) accepted", event.nSessionID);
        break;
    case SESSION_EVENT_CREATED:
        INFO("session(%u) created", event.nSessionID);
        OnSessionCreated(event);
        break;
    case SESSION_EVENT_CREATEFAIL:
        ERROR("session(%u) create failed", event.nSessionID);
        OnSessionCreateFailed(event);
        SESSION->CloseSession(event.nSessionID);
        break;
    case SESSION_EVENT_DATA:
        OnSessionMsg(event);
        break;
    case SESSION_EVENT_CLOSED:
        INFO("session(%u) closed", event.nSessionID);
        OnSessionClosed(event);
        SESSION->CloseSession(event.nSessionID);
        break;
    case SESSION_EVENT_RECONNECTING:
        WARN("session(%u) reconnecting...", event.nSessionID);
        OnSessionReconnecting(event);
        break;
    case SESSION_EVENT_RECONNECTED:
        WARN("session(%u) reconnected", event.nSessionID);
        OnSessionReconnected(event);
        break;
    case SESSION_EVENT_SEND:
        break;
    }
}

void SessionMsgProcessor::OnKafkaMsg(KafkaMsg &&msg)
{
    uint32_t msg_type = common::GetPBMsgType(msg);
    if (msg_type == UINT32_MAX)
    {
        return;
    }

    INFO("recv kafka msg(from: %s), type: %u, len: %u", msg.Src().c_str(), msg_type, msg.Len());

    auto iter = m_kafka_msg_handlers.find(msg_type);
    if (iter == m_kafka_msg_handlers.end())
    {
        ERROR("no handler for kafka msg(type: %u)", msg_type);
        return;
    }

    iter->second(std::move(msg));
}

void SessionMsgProcessor::OnSessionMsg(const SESSION_EVENT &event)
{
    if (event.dwDataLen < 1)
    {
        ERROR("invalid session msg, session_id: %u", event.nSessionID);
        return;
    }

    int32_t msg_type = -1;
    std::map<int32_t, SessionMsgHandler> *handler_map = nullptr;
    // protobuf msg
    if (event.pbData[0] == 0)
    {
        uint32_t type = common::GetPBMsgType(event.pbData, event.dwDataLen);
        if (type == UINT32_MAX)
        {
            ERROR("GetPBMsgType failed, session_id: %u", event.nSessionID);
            return;
        }
        msg_type    = type;
        handler_map = &m_session_pb_msg_handlers;
    }
    // xml msg
    else if (event.pbData[0] == '<')
    {
        TiXmlElement element("cmd");
        uint16_t command;
        if (!WXmlParser_LoadCommand(element, reinterpret_cast<const char *>(event.pbData),
                                    event.dwDataLen) ||
            !WXmlParser_GetCommand(&element, command))
        {
            ERROR("load command failed, session_id: %u", event.nSessionID);
            return;
        }
        msg_type    = command;
        handler_map = &m_session_xml_msg_handlers;
    }
    // raw binary msg
    else
    {
        msg_type    = event.pbData[0];
        handler_map = &m_session_raw_msg_handlers;
    }

    auto iter = handler_map->find(msg_type);
    if (iter == handler_map->end())
    {
        ERROR("no handler for session msg(type: %d)", msg_type);
        return;
    }

    iter->second(event.nSessionID, event.pbData, event.dwDataLen, event.dwUserData1,
                 event.dwUserData2);
}


void SessionMsgProcessor::OnLoginRecvChannelRsp(uint16_t session_id, const uint8_t *data,
                                                uint32_t len, uint64_t, uint64_t)
{
    auto iter = m_creating_session_context_table.find(session_id);
    if (iter == m_creating_session_context_table.end())
    {
        ERROR("OnLoginRecvChannelRsp, session context not found, session_id: %u", session_id);
        return;
    }

    auto context_handler = std::move(iter->second);
    RecvContext &context = context_handler->ContextData();
    m_creating_session_context_table.erase(iter);
    context.src_session_id = session_id;

    RemoveSessionTimer(session_id);

    LoginReceivingChannelRsp rsp;
    if (!UnserializePBMsg(data, len, &rsp) || rsp.response().response_code() != EnumSuccess)
    {
        ERROR("OnLoginRecvChannelRsp, LoginRecvChannel failed, session_id: %u, rspCode: %u",
              session_id, rsp.response().response_code());
        return;
    }

    uint32_t channel_id = m_media_mgr.OnLoginRecvChannelRsp(context.media_tuple, session_id);

    context.src_channel_id = channel_id;
    context.src_channel_type = SS_SRC_CHANNEL;
    INFO("OnLoginRecvChannelRsp, session_id: %u, channel_id: %u, stream_id: %s",
          session_id, channel_id, context.stream.stream_id().c_str());
    if (channel_id == 0)
    {
        ERROR("OnLoginRecvChannelRsp create channel failed");
        return;
    }

    m_stream_src_channel[context.stream.stream_id()].superior_session_id = session_id;
    m_stream_src_channel[context.stream.stream_id()].src_channel_id = channel_id;

    m_src_channel_stream[channel_id] = context.stream;

    SESSION->SetUserData(session_id, channel_id, SS_SRC_CHANNEL);
    m_logged_in_sessions[session_id].session_state = SESSION_STATES_LOGGED_IN;

    SendSendingStartReq(BIND_CALLBACK(OnSendingStartRsp, context_handler), context_handler);
}

void SessionMsgProcessor::OnLoginReq(uint16_t session_id, const uint8_t *data, uint32_t len,
                                     uint64_t, uint64_t)
{
    TiXmlElement req("cmd");
    WXmlParser_LoadCommand(req, reinterpret_cast<const char *>(data), len);

    GUID group_guid;
    char group_token[64];
    int32_t user_id = -1;
    std::string group_id = "";

    if (!WXmlParser_GetFieldValue(&req, "Guid", group_guid) ||
        !WXmlParser_GetFieldValue(&req, "CheckCode", group_token, sizeof(group_token)) ||
        !WXmlParser_GetFieldValue(&req, "FrontUserID", user_id))
    {
        ERROR("OnLoginReq, GetFieldValue failed. session_id: %u", session_id);
        SendLoginRsp(group_id, user_id, EnumError, session_id);
        return;
    }

    group_id = GUID2String(group_guid);

    TraceLog trace = m_trace.CreateTrace(TraceEnum::GS_USER_LOGIN);
    trace.user_id = user_id;
    trace.group_id = group_id;
    m_login_req_trace[session_id] = trace;

    INFO("OnLoginReq, session_id: %u, group_id: %s, group_token: %s, user_id: %d,trace_id: %s", session_id,
        group_id.c_str(), group_token, user_id, trace.trace_id.c_str());

    SessionStates session_state = m_logged_in_signal_sessions[session_id].session_state;
    if (session_state == SESSION_STATES_LOGGING_IN)
    {
        WARN("session is logging in, session_id: %u", session_id);
        return;
    }
    else if (session_state == SESSION_STATES_LOGGED_IN)
    {
        WARN("session already logged in, session_id: %u", session_id);
        SendLoginRsp(group_id, user_id, EnumSuccess, session_id);
        return;
    }

    if (m_group_mgr.UserLoggedIn(group_id, user_id))//当客户端异常退出后，重新登录到本gs时
    {
        m_group_mgr.Logout(group_id, user_id);
    }

    if (!m_group_mgr.Login(group_id, user_id, session_id))
    {
        ERROR("login failed, session_id: %u", session_id);
        SendLoginRsp(group_id, user_id, EnumError, session_id);
        return;
    }

    m_logged_in_signal_sessions[session_id].session_state = SESSION_STATES_LOGGING_IN;
    m_logged_in_signal_sessions[session_id].user_id = user_id;
    m_logged_in_signal_sessions[session_id].group_id = group_id;

    auto context_handler = MakeUserLoginContext(
        session_id, group_id, group_token, user_id,
        std::bind(&SessionMsgProcessor::OnUserLoginContextDestroy, this, _1));

    context_handler->ContextData().trace = trace;

    SendJoinGroupReq(BIND_CALLBACK(OnJoinGroupRsp, context_handler), context_handler);
    SendClientConnectedReq(BIND_CALLBACK(OnClientConnectedRsp, context_handler),
                           context_handler);
}

// 这个函数处理客户端发送的请求接收（MULTIAV_CMDID_RECVREQ）消息。它的核
// 心功能是分配一个通道的元信息（包括媒体四元组（分组ID，用户ID，媒体ID，
// 媒体类型），请求接收数据的用户ID，通道校验码）。保留的这些信息，留待
// 客户端真正地登录接收通道时使用，那时才真正地创建接收通道，加入源通道
// 的接收者列表中。回复应答给客户端时，带上了创建好的 channel_id，下次客
// 户端在登录接收通道时，就携带这个通道的标识符。
void SessionMsgProcessor::OnRecvReq(uint16_t session_id, const uint8_t *data, uint32_t len,
                                    uint64_t, uint64_t)
{
    MediaTuple media_tuple;
    uint32_t channel_id         = 0;
    uint32_t channel_check_code = 0;
    GUID group_guid;
    int32_t src_user_id = -1;
    int32_t user_id     = -1;
    int32_t media_type  = -1;
    int32_t media_id    = -1;
    int32_t recv        = -1;
    int32_t result = EnumError;

    if (m_logged_in_signal_sessions[session_id].session_state != SESSION_STATES_LOGGED_IN)
    {
        ERROR("OnRecvReq, session did not log in, session_id: %u", session_id);
        SendRecvRsp(media_tuple, user_id, channel_id, channel_check_code, recv, session_id, result);
        return;
    }

    TiXmlElement req("cmd");
    WXmlParser_LoadCommand(req, reinterpret_cast<const char *>(data), len);

    if (!WXmlParser_GetFieldValue(&req, "Guid", group_guid) ||
        !WXmlParser_GetFieldValue(&req, "SrcUserID", src_user_id) ||
        !WXmlParser_GetFieldValue(&req, "FrontUserID", user_id) ||
        !WXmlParser_GetFieldValue(&req, "MediaType", media_type) ||
        !WXmlParser_GetFieldValue(&req, "MediaID", media_id) ||
        !WXmlParser_GetFieldValue(&req, "Recv", recv))
    {
        ERROR("OnRecvReq, GetFieldValue failed. session_id: %u", session_id);
        SendRecvRsp(media_tuple, user_id, channel_id, channel_check_code, recv, session_id, result);
        return;
    }

    media_tuple.group_id   = GUID2String(group_guid);
    media_tuple.user_id    = src_user_id;
    media_tuple.media_type = media_type;
    media_tuple.media_id   = media_id;

    INFO("OnRecvReq, session_id: %u, group_id: %s, src_user_id: %d, user_id: %d, media_type: "
         "%d, media_id: %d, recv: %d",
         session_id, media_tuple.group_id.c_str(), src_user_id, user_id, media_type, media_id, recv);

    if (recv == 1)
    {
        // user_id 是接收数据的目标用户，也就是说，这个用户希望接收源用
        // 户发送的数据
        channel_id = m_media_mgr.ReserveChannel(media_tuple, CC_DST_CHANNEL, user_id, &channel_check_code);
        result = EnumSuccess;
    }
    //停止接收，在此之前所有媒体通道会收到三个recvBye，并且处理了logout请求
    //就算没有收到recvBye,该信令通道在OnSessionClose时也会调用LogoutAll清理上下文
    else if(recv == 0)
    {
        m_media_mgr.GetChannelIdAndCheckCode(media_tuple, user_id, &channel_id, &channel_check_code);
        result = EnumSuccess;
    }
    else
    {
        ERROR("OnRecvReq, recv(%d) is invalidate, user_id: %d, session_id: %u",recv, user_id, session_id);
    }

    TraceLog trace = m_trace.CreateTrace(TraceEnum::GS_RECVREQ);
    trace.user_id = user_id;
    trace.group_id = m_login_req_trace[session_id].group_id; //因为此处client上传的group_id为空
    m_recv_req_trace[channel_id] = trace;

    SendRecvRsp(media_tuple, user_id, channel_id, channel_check_code, recv, session_id, result);
}

// 客户端登录接收通道
void SessionMsgProcessor::OnRecvLoginReq(uint16_t session_id, const uint8_t *data, uint32_t len,
                                         uint64_t, uint64_t)
{
    if (len != sizeof(MULTIAV_CMD_RECVLOGINREQ))
    {
        ERROR("OnRecvLoginReq, invalid msg len, session_id: %u", session_id);
        return;
    }

    MULTIAV_CMD_RECVLOGINREQ req = *reinterpret_cast<const MULTIAV_CMD_RECVLOGINREQ *>(data);
    req.dwChannelID              = ntohl(req.dwChannelID);
    req.dwChannelCheckCode       = ntohl(req.dwChannelCheckCode);

    INFO("OnRecvLoginReq, session_id: %u, channel_id: %u, channel_check_code: %u", session_id,
         req.dwChannelID, req.dwChannelCheckCode);

    if (m_logged_in_media_sessions[session_id].session_state == SESSION_STATES_LOGGED_IN)
    {
        INFO("media session(%u) had been logged in", session_id);
        SendRecvLoginRsp(req.dwChannelID, EnumSuccess, session_id);
        return;
    }
    else if (m_logged_in_media_sessions[session_id].session_state == SESSION_STATES_LOGGING_IN)
    {
        INFO("media session(%u) is logging in", session_id);
        return;
    }

    if (!m_media_mgr.CheckChannelCheckCode(req.dwChannelID, req.dwChannelCheckCode))
    {
        ERROR("invalid check_code, session_id: %u", session_id);
        SendRecvLoginRsp(req.dwChannelID, EnumError, session_id);
        return;
    }

    if (!m_media_mgr.RecvLogin(req.dwChannelID, session_id))
    {
        INFO("RecvLogin failed, session_id: %u, channel_id: %u", session_id, req.dwChannelID);
        SendRecvLoginRsp(req.dwChannelID, EnumError, session_id);
        return;
    }

    m_logged_in_media_sessions[session_id].session_state = SESSION_STATES_LOGGING_IN;

    // channel_type，在 SessionMsgProcessor::OnSessionMsg() 中使用，这样
    // 可以通过 session_id 找到 channel_id 和 channel_type
    SESSION->SetUserData(session_id, req.dwChannelID, CC_DST_CHANNEL);

    // 记录接收数据的客户端会话，以便在转发出口中区分流服务器节点和客户
    // 端节点

    if (m_media_mgr.HasSrcChannel(req.dwChannelID))
    {
        INFO("MediaHasSrcChannel(channel_id: %u)", req.dwChannelID);

        uint32_t src_channel_id = m_media_mgr.GetSrcChannelID(req.dwChannelID);
        if (m_src_channel_stream.find(src_channel_id) != m_src_channel_stream.end())
        {
            m_recv_req_trace[req.dwChannelID].stream_id = m_src_channel_stream[src_channel_id].stream_id();

            if (m_src_channel_stream[src_channel_id].stream_type() == StreamType::EnumAudioStream)
            {
                m_recv_req_trace[req.dwChannelID].stream_type = "AUDIO";
            }
            else if(m_src_channel_stream[src_channel_id].stream_type() == StreamType::EnumVideoStream)
            {
                m_recv_req_trace[req.dwChannelID].stream_type = "VIDEO";
            }
        }

        if (m_media_mgr.AddDstChannel(req.dwChannelID, CC_DST_CHANNEL))
        {
            SendRecvLoginRsp(req.dwChannelID, EnumSuccess, session_id);
            m_logged_in_media_sessions[session_id].session_state = SESSION_STATES_LOGGED_IN;
        }
        else
        {
            SendRecvLoginRsp(req.dwChannelID, EnumError, session_id);
            m_logged_in_media_sessions[session_id].session_state = SESSION_STATES_CREATED;
        }
    }
    else
    {
        PrepareToCreateSrcChannel(req.dwChannelID, session_id);
    }
    StatisticsConcurrency();
}

void SessionMsgProcessor::SendSendingStartReq(KafkaMsgMgr::OnResp &&on_resp,
                                              RecvContextHandler context_handler)
{
    const RecvContext &context = context_handler->ContextData();

    INFO("SendSendingStartReq, stream_id: %s", context.stream.stream_id().c_str());

    StreamSendingStart req;
    req.set_stream_id(context.stream.stream_id());
    req.set_recv_client_id(common::MakeClientID(g_config.instance_id));

    req.mutable_commoninvokeinfo()->set_trace_id(context.trace.trace_id);
    req.mutable_commoninvokeinfo()->set_invoke_order(context.trace.invoke_order+"4");

    if (!SendKafkaMsg(Enum2StreamSendingStart, req, g_config.sc_topic, std::move(on_resp)))
    {
        ERROR("SendKafkaMsg failed, stream_id: %s", context.stream.stream_id().c_str());
        return;
    }
}

// 在登录接收通道之后，但是接收通道还没有源通道的时候，通知源用户登录发
// 送通道发送数据
void SessionMsgProcessor::PrepareToCreateSrcChannel(uint32_t channel_id, /*bool is_src_server,*/
                                                    uint16_t session_id)
{
    MediaTuple media_tuple;
    if (!m_media_mgr.GetMediaTuple(channel_id, &media_tuple))
    {
        ERROR("GetMediaTuple failed, session_id: %u, channel_id: %u", session_id, channel_id);
        m_logged_in_media_sessions[session_id].session_state = SESSION_STATES_CREATED;
        SendRecvLoginRsp(channel_id, EnumError, session_id);
        return;
    }

    INFO("PrepareToCreateSrcChannel: group_id: %s, user_id: %u, media_type: %u, media_id: %u",
         media_tuple.group_id.c_str(), media_tuple.user_id, media_tuple.media_type,
         media_tuple.media_id);

    // 在客户端登录发送通道之后，才回复登录接收通道的客户端。这时，需要
    // 找到这些登录接收通道但是还没有回复的客户端的会话。在一个节点中，
    // 媒体的流向包括许多的通道，每个通道和一个会话相关。
    RecvLoginChannelInfo chn_info;
    chn_info.channel_session_id = session_id;
    chn_info.channel_type = CC_DST_CHANNEL;
    m_recv_login_reqs_table[media_tuple][channel_id] = chn_info;
    if (m_recv_login_reqs_table[media_tuple].size() > 1)
    {
        INFO("PrepareToCreateSrcChannel: parallel processing, do nothing here.");
        return;
    }

    auto context_handler = MakeRecvContext(
        media_tuple, std::bind(&SessionMsgProcessor::OnRecvContextDestroy, this, _1));
    context_handler->ContextData().trace = m_recv_req_trace[channel_id];


    if (m_group_mgr.UserLoggedIn(media_tuple.group_id, media_tuple.user_id))
    {
        uint32_t channel_check_code = 0;
        uint32_t channel_id =
            m_media_mgr.ReserveChannel(media_tuple, CC_SRC_CHANNEL, /*TODO*/ 0, &channel_check_code);

        m_recv_context_table[channel_id] = std::move(context_handler);

        uint16_t signal_session_id =
            m_group_mgr.GetSessionID(media_tuple.group_id, media_tuple.user_id);

        SendSendEnableReq(media_tuple, channel_id, channel_check_code, true, signal_session_id);
    }
    else
    {
        m_logged_in_media_sessions[session_id].session_state = SESSION_STATES_LOGGING_IN;
        SendGetStreamReq(BIND_CALLBACK(OnGetStreamRsp, context_handler), context_handler);
    }
}

void SessionMsgProcessor::SendJoinGroupReq(KafkaMsgMgr::OnResp &&on_resp,
                                           UserLoginContextHandler context_handler)
{
    const UserLoginContext &context = context_handler->ContextData();

    INFO("SendJoinGroupReq, session_id: %u, group_id: %s, group_token: %s, user_id: %d",
         context.session_id, context.group_id.c_str(), context.group_token.c_str(),
         context.user_id);

    JoinGroup req;
    req.set_group_id(context.group_id);
    req.set_group_token(context.group_token);
    req.set_user_id(std::to_string(context.user_id));
    req.set_client_id(common::MakeClientID(context.group_id + ";" +
        std::to_string(context.user_id)));
    req.set_service_instance_id(g_config.instance_id);

    req.mutable_commoninvokeinfo()->set_trace_id(context.trace.trace_id);
    req.mutable_commoninvokeinfo()->set_invoke_order(context.trace.invoke_order + "1");

    if (!SendKafkaMsg(Enum2JoinGroup, req, g_config.gc_topic, std::move(on_resp)))
    {
        ERROR("SendKafkaMsg failed, session_id: %u", context.session_id);
        return;
    }
}

void SessionMsgProcessor::OnJoinGroupRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                                         UserLoginContextHandler context_handler)
{
    UserLoginContext &context = context_handler->ContextData();

    JoinGroupRsp rsp;
    if (err != KafkaMsgMgr::RESP_SUCCESS || !UnserializePBMsg(msg, &rsp) ||
        rsp.response().response_code() != EnumSuccess)
    {
        ERROR("JoinGroup failed. session_id: %u, group_id: %s, group_token: %s, user_id: %d,"
              "kafkaCode: %u, rspCode: %u, seq: %lu, src: %s", context.session_id,
              context.group_id.c_str(), context.group_token.c_str(), context.user_id, err,
              rsp.response().response_code(), msg.Seq(), msg.Src().c_str());
        return;
    }

    INFO("JoinGroup success. session_id: %u, group_id: %s, group_token: %s, user_id: %d, seq: %lu, src: %s",
         context.session_id, context.group_id.c_str(), context.group_token.c_str(),
         context.user_id, msg.Seq(), msg.Src().c_str());

    if(m_logged_in_signal_sessions.find(context.session_id) != m_logged_in_signal_sessions.end())
    {
        if (m_logged_in_signal_sessions[context.session_id].session_state == SESSION_STATES_LOGGED_IN)
        {
            //donothing
        }
        else if(m_logged_in_signal_sessions[context.session_id].session_state == SESSION_STATES_CLIENT_CONNECTED)
        {
            m_logged_in_signal_sessions[context.session_id].session_state = SESSION_STATES_LOGGED_IN;

            context.session_logged_in = true;
            context.err_code          = EnumSuccess;
        }
        else
        {
            m_logged_in_signal_sessions[context.session_id].session_state = SESSION_STATES_JOINED_GROUP;
        }
    }
    else
    {
        ERROR("OnJoinGroupRsp, can't find session_id(%u) in m_logged_in_signal_session", context.session_id);
    }
}

void SessionMsgProcessor::SendClientConnectedReq(KafkaMsgMgr::OnResp &&on_resp,
                                                 UserLoginContextHandler context_handler)
{
    const UserLoginContext &context = context_handler->ContextData();

    INFO("SendClientConnectedReq, session_id: %u", context.session_id);

    ClientConnected req;
    req.set_app_id("1");
    req.set_client_name(g_config.instance_id);
    req.set_client_id(common::MakeClientID(context.group_id + ";" +
        std::to_string(context.user_id)));
    req.set_service_instance_id(g_config.instance_id);

    req.mutable_commoninvokeinfo()->set_trace_id(context.trace.trace_id);
    req.mutable_commoninvokeinfo()->set_invoke_order(context.trace.invoke_order + "2");

    if (!SendKafkaMsg(Enum2ClientConnected, req, g_config.sc_topic, std::move(on_resp)))
    {
        ERROR("SendKafkaMsg failed, session_id: %u", context.session_id);
        return;
    }
}

void SessionMsgProcessor::OnClientConnectedRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                                               UserLoginContextHandler context_handler)
{
    UserLoginContext &context = context_handler->ContextData();

    ClientConnectedRsp rsp;
    if (err != KafkaMsgMgr::RESP_SUCCESS || !UnserializePBMsg(msg, &rsp) ||
        rsp.response().response_code() != EnumSuccess)
    {
        ERROR("ClientConnected failed, session_id: %u, kafkaCode: %u, rspCode: %u, seq: %lu, "
              "src: %s", context.session_id, err, rsp.response().response_code(),
              msg.Seq(), msg.Src().c_str());

        return;
    }

    INFO("ClientConnected success. session_id: %u, seq: %lu, src: %s",
         context.session_id, msg.Seq(), msg.Src().c_str());

    if(m_logged_in_signal_sessions.find(context.session_id) != m_logged_in_signal_sessions.end())
    {
        if (m_logged_in_signal_sessions[context.session_id].session_state == SESSION_STATES_LOGGED_IN)
        {
            //donothing
        }
        else if(m_logged_in_signal_sessions[context.session_id].session_state == SESSION_STATES_JOINED_GROUP)
        {
            m_logged_in_signal_sessions[context.session_id].session_state = SESSION_STATES_LOGGED_IN;

            context.session_logged_in = true;
            context.err_code          = EnumSuccess;
        }
        else
        {
            m_logged_in_signal_sessions[context.session_id].session_state = SESSION_STATES_CLIENT_CONNECTED;
        }
    }
    else
    {
        ERROR("OnClientConnectedRsp, can't find session_id(%u) in m_logged_in_signal_session", context.session_id);
    }
}

void SessionMsgProcessor::SendGetStreamReq(KafkaMsgMgr::OnResp &&on_resp,
                                           RecvContextHandler context_handler)
{
    const RecvContext &context = context_handler->ContextData();

    INFO("GetStream, group_id: %s, user_id: %u, media_type: %u, media_id: %u",
         context.media_tuple.group_id.c_str(), context.media_tuple.user_id,
         context.media_tuple.media_type, context.media_tuple.media_id);

    GetStream req;
    req.set_group_id(context.media_tuple.group_id);
    req.set_user_id(std::to_string(context.media_tuple.user_id));
    req.set_media_id(context.media_tuple.media_id);

    req.mutable_commoninvokeinfo()->set_trace_id(context.trace.trace_id);
    req.mutable_commoninvokeinfo()->set_invoke_order(context.trace.invoke_order+"1");

    if (context.media_tuple.media_type == EnumAudio)
    {
        req.set_media_type(EnumAudio);
    }
    else if (context.media_tuple.media_type == EnumVideo)
    {
        req.set_media_type(EnumVideo);
    }
    else
    {
        ERROR("unknown media type: %u", context.media_tuple.media_type);
        return;
    }

    if (!SendKafkaMsg(Enum2GetStream, req, g_config.gc_topic, std::move(on_resp)))
    {
        ERROR("SendKafkaMsg failed");
        return;
    }
}

void SessionMsgProcessor::OnGetStreamRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                                         RecvContextHandler context_handler)
{
    RecvContext &context = context_handler->ContextData();

    GetStreamRsp rsp;
    if (err != KafkaMsgMgr::RESP_SUCCESS || !UnserializePBMsg(msg, &rsp) ||
        rsp.response().response_code() != EnumSuccess)
    {
        ERROR("GetStream failed, stream_id: %s, kafkaCode: %u, rspCode: %u, seq: %lu, "
              "src: %s", context.stream.stream_id().c_str(), err, rsp.response().response_code(),
              msg.Seq(), msg.Src().c_str());
        return;
    }

    INFO(
        "OnGetStreamRsp, stream_id: %s, stream_type: %u, stream_property: %u, "
        "stream_subscribe_token: %s, seq: %lu, src: %s",
        rsp.stream().stream_id().c_str(), rsp.stream().stream_type(),
        rsp.stream().stream_property(), rsp.stream().stream_subscribe_token().c_str(),
        msg.Seq(), msg.Src().c_str());

    context.stream = rsp.stream();

    SendGetSuperiorReq(BIND_CALLBACK(OnGetSuperiorRsp, context_handler), context_handler);
}

void SessionMsgProcessor::SendGetSuperiorReq(KafkaMsgMgr::OnResp &&on_resp,
                                             RecvContextHandler context_handler)
{
    const RecvContext &context = context_handler->ContextData();

    INFO("SendGetSuperiorReq, stream_id: %s", context.stream.stream_id().c_str());

    GetSuperiorStreamServer req;
    req.set_stream_id(context.stream.stream_id());
    req.set_service_instance_id(g_config.instance_id);

    req.mutable_commoninvokeinfo()->set_trace_id(context.trace.trace_id);
    req.mutable_commoninvokeinfo()->set_invoke_order(context.trace.invoke_order+"2");

    if (!SendKafkaMsg(Enum2GetSuperiorStreamServer, req, g_config.sc_topic, std::move(on_resp)))
    {
        ERROR("SendKafkaMsg failed, stream_id: %s", context.stream.stream_id().c_str());
        return;
    }
}

void SessionMsgProcessor::OnGetSuperiorRsp(KafkaMsgMgr::RespErrno err, const KafkaMsg &msg,
                                           RecvContextHandler context_handler)
{
    RecvContext &context = context_handler->ContextData();
    GetSuperiorStreamServerRsp rsp;
    if (err != KafkaMsgMgr::RESP_SUCCESS || !UnserializePBMsg(msg, &rsp) ||
        rsp.response().response_code() != EnumSuccess)
    {
        ERROR("GetSuperior failed, stream_id: %s, kafkaCode: %u, rspCode: %u, seq: %lu, "
              "src: %s", context.stream.stream_id().c_str(), err, rsp.response().response_code(),
              msg.Seq(), msg.Src().c_str());
        return;
    }

    INFO("OnGetSuperiorRsp, stream_id: %s, seq: %lu, src: %s, ip: %s, port: %s, connect_str: %s",
         context.stream.stream_id().c_str(), msg.Seq(), msg.Src().c_str(), rsp.stream_server().addr().c_str(),
         rsp.stream_server().port().c_str(), rsp.stream_server().connect_str().c_str());

    context.superior_client_id = common::MakeClientID(rsp.stream_server().id());

    std::string addr = rsp.stream_server().connect_str();
    SESSIONTYPE session_type;
    if (context.stream.stream_property() == EnumReliable)
    {
        if(addr.length() == 0)
        {
            addr         = "TCP:" + rsp.stream_server().addr() + ":" + rsp.stream_server().port();
        }
        session_type = SESSIONTYPE_RELIABLE;
    }
    else
    {
        if(addr.length() == 0)
        {
            addr         = "UDP:" + rsp.stream_server().addr() + ":" + rsp.stream_server().port();
        }
        session_type = SESSIONTYPE_UNRELIABLE;
    }

    uint16_t superior_session_id =
        SESSION->CreateSession(addr.c_str(), session_type, g_config.app_id, &m_notify);
    if (superior_session_id == 0)
    {
        ERROR("CreateSession to %s failed", addr.c_str());
        return;
    }
    else
    {
        AddSessionTimer(superior_session_id, BIND_CALLBACK_CHANNEL(SessionMsgProcessor::OnCreateMediaSessionTimeOut,
                                                                   superior_session_id));
    }

    if (m_creating_session_context_table.find(superior_session_id) == m_creating_session_context_table.end())
    {
        m_creating_session_context_table[superior_session_id] = std::move(context_handler);
    }
    INFO("Creating Session(%u) to %s ...", superior_session_id, addr.c_str());
}

void SessionMsgProcessor::SendLoginRecvChannelReq(uint16_t session_id,
                                                  const std::string &stream_id)
{
    auto iter = m_creating_session_context_table.find(session_id);
    if (iter == m_creating_session_context_table.end())
    {
        ERROR("session context not found, session_id: %u", session_id);
        return;
    }

    RecvContext &context = iter->second->ContextData();

    INFO("SendLoginRecvChannelReq, session_id: %u, stream_id: %s", session_id,
         stream_id.c_str());

    LoginReceivingChannel req;
    req.set_stream_id(stream_id);
    req.set_client_token(
        common::MakeClientToken(common::MakeClientID(g_config.instance_id), ""));

    req.mutable_commoninvokeinfo()->set_trace_id(context.trace.trace_id);
    req.mutable_commoninvokeinfo()->set_invoke_order(context.trace.invoke_order + "3");

    SendSessionMsg(Enum2LoginReceivingChannel, req, session_id);

    AddSessionTimer(session_id,
                    BIND_CALLBACK_CHANNEL(SessionMsgProcessor::WaitForLoginRecvChannelRspTimeOut,session_id));
}

void SessionMsgProcessor::SendLoginRsp(const std::string &group_id, int32_t user_id,
                                       ResponseCode err_code, uint16_t session_id)
{
    INFO("SendLoginRsp, session_id: %u, group_id: %s, user_id: %d, err_code: %d", session_id,
         group_id.c_str(), user_id, err_code);

    TiXmlElement element("cmd");
    WXmlParser_SetCommand(&element, MULTIAV_CMDID_LOGINREP);

    WXmlParser_AddFieldValue(&element, "Guid", group_id.c_str());
    WXmlParser_AddFieldValue(&element, "FrontUserID", user_id);
    WXmlParser_AddFieldValue(&element, "MixerUserID", 0);
    WXmlParser_AddFieldValue(&element, "Result", err_code);

    SendSessionMsg(element, session_id);


    if (m_login_req_trace.find(session_id) != m_login_req_trace.end())     //m_login_req_trace 要传递groupid,在logout处释放
    {
        TraceLog trace = m_login_req_trace[session_id];
        trace.result = err_code;
        m_trace.PrintTrace(trace);
    }
}

void SessionMsgProcessor::SendRecvRsp(const MediaTuple &media_tuple, int32_t user_id,
                                      uint32_t channel_id, uint32_t channel_check_code,
                                      int32_t recv, uint16_t session_id, int32_t result)
{
    INFO(
        "SendRecvRsp, session_id: %u, group_id: %s, src_user_id: %d, media_type: %u, media_id: "
        "%u, user_id: %d, channel_id: %u, channel_check_code: %u, recv: %d, result: %d",
        session_id, media_tuple.group_id.c_str(), media_tuple.user_id, media_tuple.media_type,
        media_tuple.media_id, user_id, channel_id, channel_check_code, recv, result);

    TiXmlElement element("cmd");
    WXmlParser_SetCommand(&element, MULTIAV_CMDID_RECVREP);

    WXmlParser_AddFieldValue(&element, "Guid", media_tuple.group_id.c_str());
    WXmlParser_AddFieldValue(&element, "Recv", recv);
    WXmlParser_AddFieldValue(&element, "MediaType", media_tuple.media_type);
    WXmlParser_AddFieldValue(&element, "MediaID", media_tuple.media_id);
    WXmlParser_AddFieldValue(&element, "ChannelID", channel_id);
    WXmlParser_AddFieldValue(&element, "ChannelCheckCode", channel_check_code);
    WXmlParser_AddFieldValue(&element, "FrontUserID", user_id);
    WXmlParser_AddFieldValue(&element, "SrcUserID", media_tuple.user_id);
    WXmlParser_AddFieldValue(&element, "Result", result);

    SendSessionMsg(element, session_id);
}

void SessionMsgProcessor::SendRecvLoginRsp(uint32_t channel_id, ResponseCode err_code,
                                           uint16_t session_id)
{
    INFO("SendRecvLoginRsp, session_id: %u, channel_id: %u, err_code: %d", session_id,
         channel_id, err_code);

    MULTIAV_CMD_RECVLOGINREP rsp;

    rsp.bCmd        = MULTIAV_CMDID_RECVLOGINREP;
    rsp.bResult     = err_code;
    rsp.dwChannelID = htonl(channel_id);

    SendSessionMsg(&rsp, sizeof(rsp), session_id);

    if (m_recv_req_trace.find(channel_id) != m_recv_req_trace.end())
    {
        TraceLog trace = m_recv_req_trace[channel_id];
        trace.result = err_code;
        m_trace.PrintTrace(trace);
        m_recv_req_trace.erase(channel_id);
    }
}
}
}
}
