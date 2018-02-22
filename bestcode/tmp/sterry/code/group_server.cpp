#include "group_server.h"

#include <chrono>
#include <iostream>
#include <thread>

#include "json/json.h"

#include "component_mgr.h"
#include "global_objects.h"
#include "kafka_errcode.h"
#include "zk_errcode.h"

#include "config.h"
#include "winitguid.h"
#include "utility.h"

namespace fsp
{
namespace sss
{
namespace gs
{
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

using common::g_session_mgr;
using common::g_kafka_msg_mgr;
using common::g_logger;
using common::g_printer;
using common::g_zkclient;

using common::RegisterCallback;
using common::UnregisterCallback;

enum MsgClass
{
    SESSION_MSG,
    KAFKA_MSG,
};

static BOOL OnSessionMsg(uint32_t msg_class, FS_INT, FS_UINT param2, uint32_t, FS_UINT user_data)
{
    util::Thread *thread = reinterpret_cast<util::Thread *>(user_data);
    thread->PostThreadMsg(nullptr, msg_class, param2);
    return TRUE;
}

bool GroupServer::Start(const std::string &config)
{
    m_running = true;

    InitConfig(config);
    m_thread.Start(std::bind(&GroupServer::HandleThreadMsg, this, _1, _2, _3));
    InitLogger();
    InitSessionMgr();
    InitZkClient();
    InitKafkaMsgMgr();
    m_session_msg_processor.StatisticsConcurrency();

    while (true)
    {
        if (m_already_register_zk)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    INFO(
        "GroupServer Start succeed. instance_id: %s, kafka_brokers: %s, consumer_group: %s, "
        "sc_topic: %s, gc_topic: %s, broadcast_topic: %s, app_id: %u, listen_port: %u, "
        "ip: %s, zookeeper: %s, zk_root_node: %s",
        g_config.instance_id.c_str(), g_config.kafka_brokers.c_str(),
        g_config.consumer_group.c_str(), g_config.sc_topic.c_str(), g_config.gc_topic.c_str(),
        g_config.broadcast_topic.c_str(), g_config.app_id, g_config.listen_port,
        g_config.ip.c_str(), g_config.zookeeper.c_str(), g_config.zk_root_node.c_str());

    return true;
}

void GroupServer::Stop()
{
    INFO("GroupServer Stopped.");

    g_kafka_msg_mgr.Stop();
    g_session_mgr->UnRegisterApplication(g_config.app_id);
    g_zkclient.Unregister(::fsp::sss::common::GROUP_SERVER, g_config.zk_root_node,
        g_config.instance_id, nullptr);
    g_logger.Stop();
    m_thread.Stop();

    m_running = false;
}

bool GroupServer::Running()
{
    return m_running;
}

void GroupServer::InitConfig(const std::string &config)
{
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(config, root))
    {
        std::cout << "parse config failed" << std::endl;
        throw std::runtime_error("parse config failed");
    }

    g_config.instance_id            = root.get("instance_id", "").asString();
    // take self instance id as consumer group id
    g_config.consumer_group         = g_config.instance_id;
    g_config.kafka_brokers          = root.get("kafka_brokers", "").asString();
    g_config.sc_topic               = root.get("sc_topic", "").asString();
    g_config.gc_topic               = root.get("gc_topic", "").asString();
    g_config.broadcast_topic        = root.get("broadcast_topic", "").asString();
    g_config.app_id                 = 1; // TODO:
    g_config.listen_port            = root.get("listen_port", 0).asUInt();
    g_config.ip                     = root.get("ip", "").asString();
    g_config.zookeeper              = root.get("zookeeper", "").asString();
    g_config.zk_root_node           = root.get("zk_root_node", "").asString();
    g_config.net_address_list       = root.get("net_address_list", "").asString();
    g_config.trace_path             = root.get("trace_path", "").asString();
    if (g_config.instance_id.empty() || g_config.kafka_brokers.empty() ||
        g_config.sc_topic.empty() || g_config.gc_topic.empty() || g_config.app_id == 0 ||
        g_config.listen_port == 0 || g_config.ip.empty() || g_config.zookeeper.empty() ||
        g_config.zk_root_node.empty() || g_config.consumer_group.empty() ||
        g_config.broadcast_topic.empty())
    {
        std::cout << "invalid config item" << std::endl;
        throw std::runtime_error("invalid config item");
    }
}

void GroupServer::InitLogger()
{
    util::FilePrinter::Config log_config;
    log_config.log_module_name = g_config.log_module_name;
    log_config.max_file_size   = g_config.max_file_size;

    g_printer.SetConfig(log_config);

    g_logger.AddPrinter(std::bind(&util::FilePrinter::OnLog, &g_printer, _1, _2));
    g_logger.Start();

    util::TracePrinter::Config trace_config;
    trace_config.trace_path = g_config.trace_path;
    trace_config.device_id = g_config.instance_id;
    m_trace.SetConfig(trace_config);
}

void GroupServer::InitZkClient()
{
    // default timeout : 3000
    CHAR connect_str[MAX_PATH] = {0};
    uint32_t size = MAX_PATH;
    g_session_mgr->GetAddrLink(connect_str, &size, TRUE, TRUE);
    g_config.connect_str = fsp::sss::common::FilterKCP(connect_str);
    int result = g_zkclient.Init(g_config.zookeeper.c_str(), 3000);
    if (result != 0)
    {
        FATAL("ZKClient::Init failed, errcode is %u", result);
        throw std::runtime_error("ZkClient Init failed.");
    }

    RegisterCallback cb = std::bind(&GroupServer::RegisterZkCallback, this, _1);
    result = g_zkclient.Register(::fsp::sss::common::GROUP_SERVER, g_config.zk_root_node,
                                 g_config.instance_id,
                                 g_config.ip, g_config.listen_port, g_config.connect_str, cb);
    if (result != 0)
    {
        FATAL("ZKClient::Register failed, error code is %d", result);
        throw std::runtime_error("ZkClient Init failed.");
    }
}

void GroupServer::RegisterZkCallback(int rc)
{
    if (rc == 0)
    {
        m_already_register_zk = true;
        INFO("zookeeper register succeed.");
    }
    else
    {
        sleep(1);
        WARN("zookeeper register failed, error code is %d. retry...", rc);
        if(rc == fsp::sss::common::ZK_CONNECTION_LOSS)
        {
            InitZkClient();
        }
        else
        {
            RegisterCallback cb = std::bind(&GroupServer::RegisterZkCallback, this, _1);
            int count = 0;
            do
            {
                rc = g_zkclient.Register(::fsp::sss::common::GROUP_SERVER, g_config.zk_root_node,
                                         g_config.instance_id, g_config.ip,
                                         g_config.listen_port, g_config.connect_str, cb);
                usleep(20000);
            }while (rc == fsp::sss::common::ZK_CREATE_ERROR && count++ < 10);

            INFO("zookeeper register retry start, rc = %d.", rc);
        }
    }
}

void GroupServer::InitSessionMgr()
{
    common::ComponentMgr::Instance().SetNetIPList(g_config.net_address_list.c_str());
    g_session_mgr = common::ComponentMgr::Instance().GetSessionMgr();
    AVQosInit(common::ComponentMgr::Instance().GetComponentFactory(), NULL, TRUE);

    InitNotify();
    common::ComponentMgr::InitSessionMgrForServer(g_session_mgr, g_config.app_id,
                                                   g_config.listen_port, &m_notify,
                                                   g_config.session_security_type);
}

void GroupServer::InitKafkaMsgMgr()
{
    auto result = g_kafka_msg_mgr.Start(g_config.kafka_brokers, g_config.instance_id,
                                        g_config.consumer_group,
                                        std::vector<std::string>{g_config.instance_id,
                                        g_config.broadcast_topic},
                                        std::bind(&GroupServer::OnKafkaMsg, this, _1));

    if (result != ::fsp::sss::common::KAFKA_SUCCESS)
    {
        FATAL("GroupServer start failed, instance_id: %s, sc_topic: %s, brokers: %s",
              g_config.instance_id.c_str(), g_config.sc_topic.c_str(),
              g_config.kafka_brokers.c_str());
        throw std::runtime_error("KafkaMsgMgr start failed");
    }

    INFO("KafkaMsgMgr started");
}

void GroupServer::InitNotify() noexcept
{
    m_notify.nNotifyMode             = WBASE_NOTIFYMODE_CALLBACK;
    m_notify.CallbackMode.pCallback  = reinterpret_cast<void *>(OnSessionMsg);
    m_notify.CallbackMode.nMsgID     = SESSION_MSG;
    m_notify.CallbackMode.dwUserData = reinterpret_cast<uint64_t>(&m_thread);
}

void GroupServer::OnKafkaMsg(KafkaMsg &&msg)
{
    m_thread.PostThreadMsg(new KafkaMsg(std::move(msg)), KAFKA_MSG, 0);
}

void GroupServer::HandleThreadMsg(void *data, int64_t msg_class, uint64_t session_id)
{
    switch (msg_class)
    {
    case SESSION_MSG:
    {
        SESSION_EVENT *event = nullptr;
        while ((event = g_session_mgr->GetEvent(static_cast<uint16_t>(session_id))))
        {
            m_session_msg_processor.OnSessionEvent(*event);
            g_session_mgr->FreeEvent(static_cast<uint16_t>(session_id), event);
        }
        break;
    }
    case KAFKA_MSG:
    {
        KafkaMsg *msg = reinterpret_cast<KafkaMsg *>(data);
        assert(msg);
        m_session_msg_processor.OnKafkaMsg(std::move(*msg));
        delete msg;
        break;
    }
    default:
        ERROR("invalid msg_class: %llu", msg_class);
    }
}
}
}
}
