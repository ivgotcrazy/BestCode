#ifndef CEZNOTIFYMESSAGE_H
#define CEZNOTIFYMESSAGE_H

#include "EZWBDef.h"
#include <set>
#include <list>
#include <mutex>

using namespace eztalks;

typedef struct{
    int32_t nWBId;
    int32_t nMsgId;
    int64_t nParam1;
    int64_t nParam2;
}EZMsg_t;

typedef std::set<INotifyMessageListener*> ListenerSet;
typedef std::list<EZMsg_t> NotifyMessageList;

class CEZNotifyMsgDispatch
{
public:
    CEZNotifyMsgDispatch();
    ~CEZNotifyMsgDispatch();

    //消息分发
    virtual void DispatchNotifyMessage(INotifyMessageListener *pListener);
    //注册消息监听
    virtual void RegNotifyMessage(INotifyMessageListener *pListener);
    //注销消息监听
    virtual void UnRegNotifyMessage(INotifyMessageListener *pListener);
    //投递消息
    virtual void PostNotifyMessage(int32_t nWBId,int32_t nMsgId,int64_t nParam1,int64_t nParam2);

protected:
    ListenerSet             m_stListeners;
    NotifyMessageList       m_lsMessages;
    std::mutex              m_Lock;
};

#endif // CEZNOTIFYMESSAGE_H
