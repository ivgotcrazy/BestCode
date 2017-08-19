#include "EZNotifyMsgDispatch.h"

CEZNotifyMsgDispatch::CEZNotifyMsgDispatch()
{

}


CEZNotifyMsgDispatch::~CEZNotifyMsgDispatch()
{
    m_stListeners.clear();
    m_lsMessages.clear();
}

//消息分发
void CEZNotifyMsgDispatch::DispatchNotifyMessage(INotifyMessageListener *pSelf)
{
    ListenerSet::iterator it;
    EZMsg_t msg = m_lsMessages.front();
    m_lsMessages.pop_front();

    if(pSelf)
    {
        if(pSelf->OnNotifyMessageRecv(msg.nWBId,msg.nMsgId,msg.nParam1,msg.nParam2)){
            return;
        }
    }

    for(it = m_stListeners.begin();it != m_stListeners.end();it++)
    {
        INotifyMessageListener *pListener = (*it);
        if(pListener->OnNotifyMessageRecv(msg.nWBId,msg.nMsgId,msg.nParam1,msg.nParam2))
        {
            return;
        }
    }
}

//注册消息监听
void CEZNotifyMsgDispatch::RegNotifyMessage(INotifyMessageListener *pListener)
{
    m_stListeners.insert(pListener);
}

//注销消息监听
void CEZNotifyMsgDispatch::UnRegNotifyMessage(INotifyMessageListener *pListener)
{
    m_stListeners.erase(pListener);
}

//投递消息
void CEZNotifyMsgDispatch::PostNotifyMessage(int32_t nWBId,int32_t nMsgId,int64_t nParam1,int64_t nParam2)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    m_lsMessages.push_back({nWBId,nMsgId,nParam1,nParam2});
}
