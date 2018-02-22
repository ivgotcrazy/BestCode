#pragma once

#include <string>

#include "iservice.h"
#include "thread.h"

#include "session_msg_processor.h"

namespace fsp
{
namespace sss
{
namespace gs
{
// TODO: set buffer size of session manager
// TODO: confirm if the performance between AddReceiver/RemoveReceiver and PauseReceiver of
// IAVQosServer differ a lot
class GroupServer final : public common::IService
{
    // impl for IService
public:
    GroupServer(){}
    ~GroupServer() { Stop(); }

public:
    virtual bool Start(const std::string &config) override;
    virtual void Stop() override;
    virtual bool Running() override;

private:
    void InitConfig(const std::string &config);
    void InitLogger();
    void InitZkClient();
    void RegisterZkCallback(int rc);
    void InitSessionMgr();
    void InitKafkaMsgMgr();
    void InitNotify() noexcept;

private:
    void OnKafkaMsg(KafkaMsg &&msg);
    void HandleThreadMsg(void *data, int64_t msg_class, uint64_t session_id);
    void HandleSessionEvent(SESSION_EVENT *event);
    void HandleSessionCreated(uint16_t session_id);
    void HandleSessionData(uint16_t session_id, void *data, uint32_t len, uint64_t param1,
                           uint64_t param2);

private:
    SessionMsgProcessor m_session_msg_processor{ m_thread, m_notify, m_trace};
    util::Thread m_thread;
    WBASE_NOTIFY m_notify;
    util::TracePrinter m_trace;

    std::atomic_bool m_running{false};
    std::atomic_bool m_already_register_zk{false};
};
}
}
}
