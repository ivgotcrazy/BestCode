#include "clientsocket.h"
#include <deque>
#include <QDebug>

#define WM_SESSIONNOTIFY    WM_USER+100

ClientSocket::ClientSocket()
    :m_pSessionManager(NULL)
    ,m_pFactory(NULL)
    ,m_pNetFileManager(NULL)
    ,m_fcMainThreadCall(NULL)
    ,m_fcWbSink(NULL)
    ,m_dwUserID(0)
    ,m_dwCheckCode(0)
    ,m_wSrvAppID(0)
    ,m_wSessionID(0)
    ,m_pMemoryAllocator(NULL)
    ,m_bRun(FALSE)
    ,m_pResponse(NULL)
    ,m_bFirstWB(FALSE)
    ,m_bWbLogin(FALSE)
    ,m_nWBServerStatus(WB_EVENT_CONNECTING)
{
    memset(&m_Notify,0,sizeof(WBASE_NOTIFY));
    memset(&m_guid,0,sizeof(GUID));
}


/********************************************************************************************************
 * 描述：客户端析构函数
 * 参数：
 * 环境：
 * 依赖：
 * 影响：
 ********************************************************************************************************/
ClientSocket::~ClientSocket()
{
    m_fcWbSink = NULL;
    m_pSessionManager = NULL;
    m_pFactory = NULL;
    m_pNetFileManager = NULL;
    m_fcMainThreadCall = NULL;
    m_pResponse = NULL;
    m_bRun = FALSE;
    this->StopThread();

    m_WBMsgProcessor.Release();
}
/********************************************************************************************************
 * 描述：客户端初始化
 * 参数：
 * 环境：
 * 依赖：白板调用MultiWhiteBoard
 * 影响：
 ********************************************************************************************************/
BOOL ClientSocket::Initialized(ISessionManager2 *pSessionManager,IComponentFactory *pFactory,IFileManager *pNetFileManager,const WBASE_NOTIFY &notify,FUNC_MSGTOMAIN funcMsgToMain)
{
    m_pSessionManager = pSessionManager;
    m_pFactory = pFactory;
    m_pNetFileManager = pNetFileManager;
    m_Notify = notify;
    m_fcMainThreadCall = funcMsgToMain;//任务交给主线程函数处理入口，向主线程发送信号通知，让主线程回调处理

    FS_HRESULT hr = pFactory->QueryInterface(IID_IMemoryAllocator,(void**)&m_pMemoryAllocator);//查询组件是否支持
    if(FAILED(hr))
        return FALSE;
    return TRUE;
}
/********************************************************************************************************
 * 描述：设置白板的Sink
 * 参数：
 * 环境：
 * 依赖：白板调用MultiWhiteBoard
 * 影响：
 ********************************************************************************************************/
void ClientSocket::SetWBSinkSource(FUNC_WBSINK funcWbSink)
{
    m_fcWbSink = funcWbSink;
}
/********************************************************************************************************
 * 描述：创建会话
 * 参数：
 * 环境：对白板开放
 * 依赖：白板调用，MultiWhiteBoard
 * 影响：开启线程,调用私有的连接函数
 ********************************************************************************************************/
BOOL ClientSocket::Create(const GUID *pGuid,DWORD dwUserID,DWORD dwCheckCode,WORD wAppID,const CHAR *lpszSrvAddrLink)
{
    m_guid = *pGuid;
    m_dwUserID = dwUserID;
    m_dwCheckCode = dwCheckCode;
    m_wSrvAppID = wAppID;
    m_strSrvAddrLink = lpszSrvAddrLink;

    m_bRun = TRUE;
    m_bRun = StartThread();

    return this->CreateSession(wAppID,lpszSrvAddrLink);

}
/********************************************************************************************************
 * 描述：设置白板响应
 * 参数：
 * 环境：对白板公开
 * 依赖：白板调用MultiWhiteBoard
 * 影响：
 ********************************************************************************************************/
void ClientSocket::SetNetWorkResponse(INetWorkResponse *pResponse)
{
    this->m_pResponse = pResponse;
}


/********************************************************************************************************
 * 描述：解析路径
 * 参数：
 * 环境：私有，服务自身
 * 依赖：
 * 影响：
 * 日期：2017.07.26
 * 修改：无
 ********************************************************************************************************/
CHAR*	ClientSocket::ParsePath( CHAR* szPath,CHAR** szName,CHAR** szAttributeName,CHAR** szAttributeValue )
{
    if( NULL == szPath || strlen(szPath) == 0 || szName == NULL || szAttributeName == NULL || szAttributeValue == NULL )
        return NULL;

    *szName = szPath;
    *szAttributeName = NULL;
    *szAttributeValue = NULL;

    CHAR *szReturn = strchr( szPath,'/');
    if( szReturn ){
        szReturn[0] = 0;
        ++szReturn;
    }

    CHAR *szFind = strchr( szPath,' ');
    if( szFind ){
        szFind[0] = 0;
        ++szFind;
        while( szFind[0] == ' ' ) ++szFind;
        *szAttributeName = szFind;

        szFind = strchr( szFind,'=');
        if( szFind ){
            szFind[0] = 0;
            ++szFind;
            while( szFind[0] == ' ' ) ++szFind;
            *szAttributeValue = szFind;
        }
    }
    return szReturn;
}

///
///
///
///----------------------- session message process --------------------------------------------------


/********************************************************************************************************
 * 描述：消息派分
 * 参数：
 * 环境：回调函数
 * 依赖：连接白板服务器时，把此函数注册给会话管理器
 * 影响：会话管理器回调此函数，进行消息派分
 ********************************************************************************************************/
BOOL EZSTDCALL OnSessionNotifyDispatch(UINT nMsgID,WPARAM wParam,LPARAM lParam,FS_UINT32 dwReserved,FS_UINT dwUserData)
{
    if(nMsgID == WM_SESSIONNOTIFY)
    {
        ClientSocket *pSocket = (ClientSocket*)dwUserData;
        pSocket->OnSessionNotify(nMsgID,wParam,lParam,dwReserved,dwUserData);
    }
    return TRUE;
}


/********************************************************************************************************
 * 描述：与服务器建立链接
 * 参数：
 * 环境：私有
 * 依赖：由公有的create对外
 * 影响：1、把会话通知接口WBASE_NOTIFY notify注册到会话管理器 2、初始化消息管理类
 * 日期：2017.07.26
 * 修改：无
 ********************************************************************************************************/
BOOL ClientSocket::CreateSession(WORD wAppID,const CHAR *lpszSrvAddrLink)
{
    if(NULL == m_pSessionManager || NULL == m_pMemoryAllocator)
        return FALSE;

    WBASE_NOTIFY notify;
    notify.nNotifyMode = WBASE_NOTIFYMODE_CALLBACK;
    notify.CallbackMode.pCallback = (void*)OnSessionNotifyDispatch;
    notify.CallbackMode.dwUserData = (FS_UINT)this;
    notify.CallbackMode.nMsgID = WM_SESSIONNOTIFY;

    m_wSessionID = m_pSessionManager->CreateSession(lpszSrvAddrLink,SESSIONTYPE_RELIABLE,wAppID,&notify);

    if(m_wSessionID == 0)
        return FALSE;

    m_WBMsgProcessor.Init(this,m_pSessionManager,m_pMemoryAllocator,m_wSessionID);//初始化白板消息处理类
    return TRUE;
}

/********************************************************************************************************
 * 描述：收到白板服务器相关消息,丢进消息队列
 * 参数：
 * 环境：公有
 * 依赖：由会话管理器回调
 * 影响：消息提取转换WM，push到消息队列
 * 日期：2017.07.26
 * 修改：无
 ********************************************************************************************************/
LRESULT ClientSocket::OnSessionNotify(UINT nMsgID,WPARAM wParam,LPARAM lParam,FS_UINT32 dwReserved,FS_UINT dwUserData)
{
    if(NULL == m_pSessionManager || lParam != m_wSessionID)
        return -1;

    CMsgMpNode *pMsg = m_NetMsgAllocator.Alloc();
    if(pMsg)
    {
        pMsg->SetMessage(WM_SESSIONNOTIFY);
        pMsg->SetWParam(wParam);
        pMsg->SetLParam(lParam);
        pMsg->SetUserData((void*)dwUserData);
        pMsg->SetUserDataEx(dwReserved);
        m_NetMsgQueue.PushMsg(pMsg);
    }

}

/********************************************************************************************************
 * 描述：线程异步从消息队列中取数据进行处理
 * 参数：
 * 环境：线程虚函数，线程的执行主体函数
 * 依赖：在连接服务器时创建线程
 * 影响：emit消息到UI线程，让UI线程回调处理
 * 日期：2017.07.26
 * 修改：无
 ********************************************************************************************************/
FS_UINT32 ClientSocket::ThreadProcEx()
{
    while(m_bRun)
    {
        CMsgMpNode *pMsg = NULL;
        pMsg = m_NetMsgQueue.PopMsg(1000);
        if(pMsg && m_fcMainThreadCall)
        {
            m_fcMainThreadCall(this,pMsg);//取到数据丢到主线程处理
        }
    }
    return 0;
}

/********************************************************************************************************
 * 描述：主线程处理消息
 * 参数：
 * 环境：回调
 * 依赖：由UI线程回调执行
 * 影响：处理消息
 * 日期：2017.07.26
 * 修改：无
 ********************************************************************************************************/
bool ClientSocket::handleMsg(void* pMsg)
{
    if(pMsg == NULL)
        return false;

    CMsgMpNode *p = static_cast<CMsgMpNode*>(pMsg);

    if(p && p->GetMessage() == WM_SESSIONNOTIFY)
    {
        SESSION_EVENT *pEvent = NULL;
        while((pEvent = m_pSessionManager->GetEvent(p->GetLParam())) != NULL)
        {
            this->ProcessSessionEvent(pEvent);//具体消息处理
            m_pSessionManager->FreeEvent(p->GetLParam(),pEvent);
        }
        m_NetMsgAllocator.Free(p);
    }

}

/********************************************************************************************************
 * 描述：消息分类，交由消息执行者处理
 * 参数：
 * 环境：公有
 * 依赖：是和MultiWBMsgProcessor的通道（白板消息执行对象是在连接服务器时初始化）
 * 影响：
 * 日期：2017.07.26
 * 修改：无
 ********************************************************************************************************/
void ClientSocket::ProcessSessionEvent(SESSION_EVENT *pEvent)
{
    switch( pEvent->nEventType ) {
        case SESSION_EVENT_ACCEPT:
            assert( 0 );
            break;
        case SESSION_EVENT_CREATED:
            if( pEvent->nSessionID == m_wSessionID ){
                m_WBMsgProcessor.WriteLoginReq(m_dwUserID,m_dwCheckCode,m_guid);
            }else{
                assert( 0 );
                m_pSessionManager->CloseSession( pEvent->nSessionID );
            }
            break;
        case SESSION_EVENT_CREATEFAIL:
            if( pEvent->nSessionID == m_wSessionID ){
                this->SendNotify(WB_EVENT_CONNECTFAILED);
            }else{
                assert( 0 );
                m_pSessionManager->CloseSession( pEvent->nSessionID );
            }
            break;
        case SESSION_EVENT_DATA:
            m_WBMsgProcessor.ProcessMsg( pEvent->pbData,pEvent->dwDataLen );
            break;
        case SESSION_EVENT_CLOSED:
            if( pEvent->nSessionID == m_wSessionID ){
                this->SendNotify(WB_EVENT_CONNECTIONCLOSED);
            }else{
                assert( 0 );
                m_pSessionManager->CloseSession( pEvent->nSessionID );
            }
            break;
        case SESSION_EVENT_RECONNECTING:
            if( pEvent->nSessionID == m_wSessionID ){
            }else{
                assert( 0 );
                m_pSessionManager->CloseSession( pEvent->nSessionID );
            }
            break;
        case SESSION_EVENT_RECONNECTED:
            if( pEvent->nSessionID == m_wSessionID ){
            }else{
                assert( 0 );
                m_pSessionManager->CloseSession( pEvent->nSessionID );
            }
            break;
        case SESSION_EVENT_SEND:
            if( pEvent->nSessionID == m_wSessionID ){
            }else{
                assert( 0 );
                m_pSessionManager->CloseSession( pEvent->nSessionID );
            }
            break;
        }
}
/********************************************************************************************************
 * 描述：通知
 * 参数：
 * 环境：私有，异常通知，和关闭通知
 * 依赖：
 * 影响：
 * 日期：2017.07.26
 * 修改：无
 ********************************************************************************************************/
BOOL ClientSocket::SendNotify(int nEvent)
{

    qDebug() << "[whiteboard] SendNotify event=" << nEvent;

    m_nWBServerStatus = nEvent;

    BOOL bResult = FALSE;
    switch( m_Notify.nNotifyMode )
    {
        case WBASE_NOTIFYMODE_EVENT:
            //		SetEvent( m_notify.EventMode.event );
            break;
        case WBASE_NOTIFYMODE_THREADMSG:
            //		bResult = PostThreadMessage( m_notify.ThreadMsgMode.nThreadID,m_notify.ThreadMsgMode.nNotifyMsg,nEvent,0 );
            break;
        case WBASE_NOTIFYMODE_HWNDMSG:
            //		bResult = PostMessage( m_notify.HwndMsgMode.hWnd,m_notify.HwndMsgMode.nNotifyMsg,nEvent,0 );
            break;
        case WBASE_NOTIFYMODE_CALLBACK:
            if( m_Notify.CallbackMode.pCallback )
            {
                bResult = ((WBASECALLBACK)m_Notify.CallbackMode.pCallback)( m_Notify.CallbackMode.nMsgID,nEvent,0,0,m_Notify.CallbackMode.dwUserData );
            }
            break;
    }
    return bResult;
}
/********************************************************************************************************
 * 描述：获取白板服务器登录状态
 * 参数：
 * 环境：公有函数，私有属性获取
 * 依赖：打开白板时，调用
 * 影响：
 * 日期：2017.07.26
 * 修改：无
 ********************************************************************************************************/
BOOL ClientSocket::IsWBLogin()
{
    return m_bWbLogin;
}
/********************************************************************************************************
 * 描述：获取白板服务器状态
 * 参数：
 * 环境：公有函数，私有属性获取
 * 依赖：
 * 影响：
 * 日期：2017.07.26
 * 修改：无
 ********************************************************************************************************/
UINT ClientSocket::GetWBServerStatus()
{
    return m_nWBServerStatus;
}

///
///\brief 接口实现，供执行者MultiWBMsgProcessor回调
///

//----------------------------- IXMLDocMsgReaderCallback Function ---------------------------

/********************************************************************************************************
 * 描述：登录白板服务器响应
 * 参数：
 * 环境：IXMLDocMsgReaderCallback接口实现
 * 依赖：
 * 影响：判断登录成功后，会请求获取服务器上的白板；失败会发送通知
 * 日期：2017.07.26
 * 修改：无
 ********************************************************************************************************/
BOOL	ClientSocket::OnLoginRep( WORD wResult )
{
    qDebug() << "[whiteboard] OnLoginRep result=" << wResult;
    if(wResult != 0)
    {
        m_bWbLogin = FALSE;
        SendNotify(WB_EVENT_LOGINFAILED);
        return FALSE;
    }

    m_nWBServerStatus = WB_EVENT_LOGINSUCCESS;
    m_bWbLogin = TRUE;
    m_bFirstWB = FALSE;
    //请求获取服务器上的白板
    m_WBMsgProcessor.WriteGetDocReq();

    return TRUE;
}
/********************************************************************************************************
 * 描述：响应连接关闭
 * 参数：
 * 环境：IXMLDocMsgReaderCallback接口实现
 * 依赖：
 * 影响：判断登录成功后，会请求获取服务器上的白板；失败会发送通知
 * 日期：2017.07.26
 * 修改：无
 ********************************************************************************************************/
BOOL	ClientSocket::OnBye()
{
    m_bFirstWB = FALSE;
    m_bWbLogin = FALSE;
    return this->SendNotify(WB_EVENT_CONNECTIONCLOSED);
}

/********************************************************************************************************
 * 描述：响应从服务器发来的白板
 * 参数：
 * 环境：IXMLDocMsgReaderCallback接口实现
 * 依赖：
 * 影响：
 * 日期：2017.07.26
 * 修改：无
 ********************************************************************************************************/
BOOL	ClientSocket::OnGetDocRep( CHAR *szDocData,DWORD dwDataLen )
{
    qDebug() << "[whiteboard] ClientSocket::OnGetDocRep() " << QString::fromLatin1(szDocData,dwDataLen);
    if( NULL == szDocData || dwDataLen == 0)
        return FALSE;

    TiXmlElement element( "Doc" );
    element.Parse( szDocData,NULL,TIXML_ENCODING_UTF8 );

    TiXmlElement *pWBElement = element.FirstChildElement( "WBItem" );
    while( pWBElement ){

        LoadWBElement( pWBElement );
        pWBElement = pWBElement->NextSiblingElement( "WBItem" );
    }

    TiXmlElement *pSelElement = element.FirstChildElement( "WBSel" );
    if( NULL == pSelElement )
        return FALSE;
    LoadSelElement( pSelElement );

    return TRUE;
}


/********************************************************************************************************
 * 描述：响应服务器通知，关闭所有白板
 * 参数：
 * 环境：IXMLDocMsgReaderCallback接口实现
 * 依赖：
 * 影响：
 * 日期：2017.07.26
 * 修改：无
 ********************************************************************************************************/
BOOL	ClientSocket::OnClearDoc()
{
    qDebug() << "[whiteboard] ClientSocket::OnClearDoc() ";
    m_bFirstWB = FALSE;
    //关闭所有白板
    if(m_pResponse)
    {
        m_pResponse->ResponseClearAllWB();
    }

    return TRUE;
}


/********************************************************************************************************
 * 描述：响应服务器通知，相关新增操作
 * 参数：
 * 环境：IXMLDocMsgReaderCallback接口实现
 * 依赖：
 * 影响：
 * 日期：2017.07.26
 * 修改：无
 ********************************************************************************************************/
BOOL	ClientSocket::OnInsertDocNode( BYTE bInsertType,BYTE bOnlyOne,CHAR* szParentPath,CHAR* szInsertPosPath,CHAR* szNodeData )
{
    qDebug() << "[whiteboard] ClientSocket::OnInsertDocNode() " << QString::fromLatin1(szNodeData);
    if( NULL == szNodeData )
        return FALSE;
    if( NULL == szParentPath || strlen(szParentPath) == 0 ){
        TiXmlElement element( "" );
        if( !element.Parse( szNodeData,NULL,TIXML_ENCODING_UTF8 ))
            return FALSE;
        if( strcmp( element.Value(),"WBSel") == 0 ){
            LoadSelElement( &element );
        }else if( strcmp( element.Value(),"WBItem") == 0 ){
            LoadWBElement( &element );
        }
        return TRUE;
    }
    CHAR *szName = NULL;
    CHAR *szAttributeName = NULL;
    CHAR *szAttributeValue = NULL;
    CHAR *szPath = szParentPath;

    szPath = ParsePath( szPath,&szName,&szAttributeName,&szAttributeValue );
    if( szName == NULL || NULL == szAttributeName || NULL == szAttributeValue )
        return FALSE;
    if( strcmp( szName,"WBItem") != 0 || strcmp( szAttributeName,"ID" ) != 0 )
        return FALSE;
    DWORD dwWBID = atol( szAttributeValue );

    if( NULL == szPath || strlen(szPath) == 0 ){
        TiXmlElement element("");
        if( !element.Parse( szNodeData,NULL,TIXML_ENCODING_UTF8 ) )
            return FALSE;
        if( strcmp( element.Value(),"FilList" ) == 0 ){
            LoadFileListElement( dwWBID,&element );
        }else if( strcmp( element.Value(),"Tool" ) == 0 ){
            LoadToolElement( dwWBID,&element );
        }
        else if ( strcmp( element.Value(), "Rotate") == 0)
        {
                LoadRotateElement( dwWBID,&element );
        }
        else if ( strcmp(element.Value(), "Rotate") == 0)
        {
            LoadRotateElement(dwWBID, &element );
        }
        else if ( strcmp( element.Value(), "BkColor") == 0)
        {
            LoadBkColorElement( dwWBID,&element );
        }
        else if( strcmp( element.Value(),"Doc" ) == 0 ){
            LoadDocElement( dwWBID,&element );
        }
        return TRUE;
    }

    szPath = ParsePath( szPath,&szName,&szAttributeName,&szAttributeValue );
    if( szName == NULL )
        return FALSE;
    if( strcmp( szName,"Doc" ) == 0 ){
        szPath = ParsePath( szPath,&szName,&szAttributeName,&szAttributeValue );
        if( szName == NULL || NULL == szAttributeName || NULL == szAttributeValue )
            return FALSE;
        if( strcmp( szName,"Page") != 0 || strcmp( szAttributeName,"Id" ) != 0 )
            return FALSE;

        int nPage = atoi(szAttributeValue);
        int nObjId = 0;
        int nObjType = 0;
        const CHAR* szTmpData = szNodeData;

        do{
            TiXmlElement txml(" ");
            szTmpData = txml.Parse(szTmpData,NULL,TIXML_ENCODING_UTF8);
            if(NULL == szTmpData)
                break;
            if(strcmp(txml.Value(),"Obj") == 0)
            {
                if(!txml.Attribute("ID",&nObjId))
                    return FALSE;
                if(!txml.Attribute("Type",&nObjType))
                    return FALSE;

                this->LoadDrawObjectElement(dwWBID,nPage,&txml);
            }

        }while(szTmpData && strlen(szTmpData) > 0);

    }else if( strcmp(szName,"FileList") == 0 ){
//        InsertWBFileNode( dwWBID, bInsertType,szPath,szInsertPosPath,szNodeData );
    }
    return TRUE;
}



/********************************************************************************************************
 * 描述：响应服务器通知，相关修改操作
 * 参数：
 * 环境：IXMLDocMsgReaderCallback接口实现
 * 依赖：
 * 影响：
 * 日期：2017.07.26
 * 修改：无
 ********************************************************************************************************/
BOOL	ClientSocket::OnModifyDocNode( CHAR *szNodePath,CHAR *szNodeData )
{
    qDebug() << "[whiteboard] ClientSocket::OnModifyDocNode() " << QString::fromLatin1(szNodeData);
    if( NULL == szNodeData )
            return FALSE;
        if( NULL == szNodePath || strlen(szNodePath) == 0 ){
            return FALSE;
        }
        CHAR *szName = NULL;
        CHAR *szAttributeName = NULL;
        CHAR *szAttributeValue = NULL;
        CHAR *szPath = szNodePath;

        szPath = this->ParsePath( szPath,&szName,&szAttributeName,&szAttributeValue );
        if( NULL == szName )
            return FALSE;
        if( strcmp( szName,"WBSel") == 0 ){
            TiXmlElement element( "" );
            if( !element.Parse( szNodeData,NULL,TIXML_ENCODING_UTF8 ))
                return FALSE;
            int nActiveWBID = -1;
            if(!element.Attribute("Sel",&nActiveWBID))
                return FALSE;
            //激活指定白板
            if(m_pResponse)
            {
                m_pResponse->ResponseActiveWB(nActiveWBID);
            }
            return TRUE;
        }
        if( NULL == szAttributeName || NULL == szAttributeValue )
            return FALSE;
        if( strcmp( szName,"WBItem") != 0 || strcmp( szAttributeName,"ID" ) != 0 )
            return FALSE;
        DWORD dwWBID = atol( szAttributeValue );

        szPath = ParsePath( szPath,&szName,&szAttributeName,&szAttributeValue );
        if( szName == NULL )
            return FALSE;

        if( strcmp( szName,"Tool" ) == 0 ){
            TiXmlElement element("");
            if( element.Parse( szNodeData,NULL,TIXML_ENCODING_UTF8 )){
                //缩放白板
                LoadToolElement( dwWBID,&element );
            }
        }
        else if ( strcmp( szName, "Rotate") == 0)
        {
            TiXmlElement element("");
            if( element.Parse( szNodeData,NULL,TIXML_ENCODING_UTF8 )){
                //旋转
                LoadRotateElement( dwWBID,&element );
            }
        }
        else if ( strcmp( szName, "BkColor") == 0)
        {
            TiXmlElement element("");
            if( element.Parse( szNodeData,NULL,TIXML_ENCODING_UTF8 )){
                //设置背景色
                LoadBkColorElement( dwWBID,&element );
            }
        }
        else if( strcmp( szName,"Indicator") == 0 )
        {
            TiXmlElement element("");
            if( element.Parse( szNodeData,NULL,TIXML_ENCODING_UTF8 )){
                //指示器位置更新
                LoadIndicatorElement( dwWBID,&element );
            }
        }
        else if( strcmp(szName,"Doc") == 0 ){
            //修改图元
            szPath = ParsePath( szPath,&szName,&szAttributeName,&szAttributeValue );
            if(NULL == szName || NULL == szAttributeName || NULL == szAttributeValue )
                return FALSE;
            if( strcmp( szName,"Page") != 0 || strcmp( szAttributeName,"Id" ) != 0 )
                return FALSE;
            int nPage = atoi(szAttributeValue);

            szPath = ParsePath( szPath,&szName,&szAttributeName,&szAttributeValue );
            if(NULL == szName || NULL == szAttributeName || NULL == szAttributeValue )
                return FALSE;
            if( strcmp( szName,"Obj") != 0 || strcmp( szAttributeName,"ID" ) != 0 )
                return FALSE;
            DWORD dwObjID = atol(szAttributeValue);

            TiXmlElement txml("Obj");
            txml.Parse(szNodeData,NULL,TIXML_ENCODING_UTF8);
            this->LoadDrawObjectElement(dwWBID,nPage,&txml,TRUE);
        }
        else if( strcmp(szName,"Scroll") == 0 ){
            TiXmlElement element("");
            if( element.Parse( szNodeData,NULL,TIXML_ENCODING_UTF8 )){
                //更新滚动条位置
                LoadScrollElement(dwWBID,&element);
            }
        }else if( strcmp(szName,"Action") == 0 ){
            TiXmlElement element("");
            if( element.Parse( szNodeData,NULL,TIXML_ENCODING_UTF8 )){
                //动作名称
                LoadActionElement( dwWBID,&element );
            }
        }
        return TRUE;
}


/********************************************************************************************************
 * 描述：响应服务器通知，相关删除操作
 * 参数：
 * 环境：IXMLDocMsgReaderCallback接口实现
 * 依赖：
 * 影响：
 * 日期：2017.07.26
 * 修改：无
 ********************************************************************************************************/
BOOL	ClientSocket::OnDelDocNode( CHAR *szNodePath )
{
    qDebug() << "[whiteboard] ClientSocket::OnDelDocNode() " << szNodePath;

    if(NULL == szNodePath || strlen(szNodePath) == 0)
        return FALSE;

    CHAR *szName = NULL;
    CHAR *szAttributeName = NULL;
    CHAR *szAttributeValue = NULL;
    CHAR *szPath = szNodePath;
    szPath = this->ParsePath(szPath,&szName,&szAttributeName,&szAttributeValue);
    if(szName == NULL || szAttributeName == NULL || szAttributeValue == NULL)
        return FALSE;
    if(strcmp(szName,"WBItem") != 0 || strcmp(szAttributeName,"ID") != 0)
        return FALSE;

    DWORD dwWBID = atol(szAttributeValue);

    if(NULL == szPath)
    {
        //删除白板
        if(m_pResponse)
        {
            m_pResponse->ResponseCloseWB(dwWBID);
        }

        if(m_fcWbSink)
        {
            m_fcWbSink(WB_SINK_MSG_CLOSE_WB,dwWBID);
        }

        return TRUE;
    }

    szPath = this->ParsePath(szPath,&szName,&szAttributeName,&szAttributeValue);
    if(szName == NULL)
        return FALSE;
    if(strcmp(szName,"FileList") == 0)
    {
        //删除文件
        //暂不实现
    }else if (strcmp(szName,"Doc") == 0)
    {

        if(szName == NULL)
            return FALSE;

        szPath = this->ParsePath(szPath,&szName,&szAttributeName,&szAttributeValue);
        if(szName == NULL || szAttributeName == NULL || szAttributeValue == NULL)
            return FALSE;
        if(strcmp(szName,"Page") != 0 || strcmp(szAttributeName,"Id") != 0)
            return FALSE;
        int nPage = atoi(szAttributeValue);

        szPath = this->ParsePath(szPath,&szName,&szAttributeName,&szAttributeValue);
        if(szName == NULL || szAttributeName == NULL || szAttributeValue == NULL)
            return FALSE;
        if(strcmp(szName,"Obj") !=0 || strcmp(szAttributeName,"ID") != 0)
            return FALSE;
        DWORD dwObjID = atol(szAttributeValue);

        //删除元素
        if(m_pResponse)
        {
            m_pResponse->ResponseDelDrawObj(dwWBID,nPage - 1,dwObjID);
        }
    }
}


///
///\brief 协议解析
///
/********************************************************************************************************
 * 描述：
 * 参数：
 * 环境：私有解析服务函数，XML处理
 * 依赖：
 * 影响：
 * 日期：2017.07.26
 * 修改：无
 ********************************************************************************************************/

void ClientSocket::LoadActionElement(DWORD dwWBID, TiXmlElement *pElement)
{
    if( NULL == pElement )
        return;
    const CHAR* szActionName = pElement->Attribute("Name");
}

//解析白板数据
void ClientSocket::LoadWBElement(TiXmlElement *pElement)
{
    if( NULL == pElement )
            return;
        int nValue;
        if( !pElement->Attribute( "ID",&nValue ))
            return;
        DWORD dwWBID = nValue;

        const CHAR *szValue = pElement->Attribute( "Name" );
        if( !szValue )
            return;


        TiXmlElement *pFileListElement = pElement->FirstChildElement("FileList");
        TiXmlElement *pToolElement = pElement->FirstChildElement( "Tool" );
        TiXmlElement *pRotateElement = pElement->FirstChildElement( "Rotate" );
        TiXmlElement *pBkColorElement = pElement->FirstChildElement( "BkColor" );
        TiXmlElement *pDocElement = pElement->FirstChildElement("Doc");
        TiXmlElement *pScrollElement = pElement->FirstChildElement( "Scroll" );
        TiXmlElement *pActionElement = pElement->FirstChildElement( "Action" );

        if( NULL == pFileListElement || NULL == pToolElement || NULL == pDocElement )
            return;

        int nDocType = DOCTYPE_PIC;
        if(m_pResponse)
        {
            m_pResponse->ResponseAddWB(dwWBID,nDocType,szValue);
        }


        LoadDocElement( dwWBID,pDocElement );
        LoadFileListElement( dwWBID,pFileListElement );
        LoadToolElement( dwWBID,pToolElement );
        if( pScrollElement )
            LoadScrollElement( dwWBID,pScrollElement );
        if( pActionElement )
            LoadActionElement( dwWBID,pActionElement );
        LoadRotateElement(dwWBID, pRotateElement );
        LoadBkColorElement(dwWBID, pBkColorElement);
}

//解析当前激活白板
void ClientSocket::LoadSelElement(TiXmlElement *pElement)
{
    int			nWBID;
    if( !pElement->Attribute( "Sel",&nWBID ))
        return;

    if(m_pResponse)
    {
        m_pResponse->ResponseActiveWB(nWBID);
    }
}

void ClientSocket::LoadFileListElement(DWORD dwWBID, TiXmlElement *pElement)
{
    if( NULL == pElement )
            return;
        TiXmlElement *pFileElement = pElement->FirstChildElement("File");
        while( pFileElement ){
            LoadWBFileElement( dwWBID,pFileElement );
            pFileElement = pFileElement->NextSiblingElement( "File" );
        }
}


void ClientSocket::LoadWBFileElement(DWORD dwWBID, TiXmlElement *pElement)
{
    if( NULL == pElement )
            return;
        int nValue;
        const CHAR* szValue = pElement->Attribute( "Guid" );
        if( NULL == szValue )
            return;
        WCHAR wszGuid[MAX_PATH];
        if( !ConvertUtf8ToUnicode( szValue,wszGuid,MAX_PATH ))
            return;
        GUID guidFile;
        CLSIDFromString( wszGuid,&guidFile );

        const CHAR* szSrvLink = pElement->Attribute( "Link" );
        if( NULL == szSrvLink )
            return;
        if( !pElement->Attribute( "AppID",&nValue ))
            return;
        WORD wAppID = nValue;
        if( !pElement->Attribute( "CheckCode",&nValue ))
            return;
        BOOL bMainFile = FALSE;
        BOOL bLimitTransferSubRange  = TRUE;
//        CWBController *pController = m_pMultiWBController->GetWBController( dwWBID );
//        if( pController ){
//            CWBDoc *pDoc = pController->GetDoc();
//            if( pDoc ){
//                bMainFile = pDoc->GetBgFileGuid() == guidFile;
//                bLimitTransferSubRange  = pDoc->GetDocType() != DOCTYPE_WEB;
//            }
//        }

//        DWORD dwCheckCode = nValue;
//        m_pFileManager->RecvFile( dwWBID,guidFile,bMainFile,bLimitTransferSubRange,szSrvLink,wAppID,dwCheckCode );


}

//解析白板缩放数据
void ClientSocket::LoadToolElement(DWORD dwWBID, TiXmlElement *pElement)
{
    if( NULL == pElement )
        return;
    int nCurPage;
    int nZoom;
    if( !pElement->Attribute("Sel",&nCurPage ))
        return;
    if( !pElement->Attribute("Zoom",&nZoom ))
        return;

    if(m_pResponse)
    {
        m_pResponse->ResponseZoomWB(dwWBID,nCurPage -1,nZoom);
    }
}

//解析白板旋转数据
void ClientSocket::LoadRotateElement(DWORD dwWBID, TiXmlElement *pElement)
{
    if( NULL == pElement )
        return;
    int nAngle = 0;
    if( !pElement->Attribute("Angle", &nAngle ))
        return;

    if(m_pResponse)
    {
        m_pResponse->ResponseAngleWB(dwWBID,nAngle);
    }
}

//解析背景颜色数据
void ClientSocket::LoadBkColorElement(DWORD dwWBID, TiXmlElement *pElement)
{
    if( NULL == pElement )
        return;
    int nColor = 0;
    if( !pElement->Attribute("color", &nColor ))
        return;

    if(m_pResponse)
    {
        int r = nColor >> 16 & 0xff;
        int g = nColor >> 8 & 0xff;
        int b = nColor & 0xff;
        m_pResponse->ResponseWBBackgroundColor(dwWBID,r,g,b);
    }
}

//解析滚动条数据
void ClientSocket::LoadScrollElement(DWORD dwWBID, TiXmlElement *pElement)
{
    if( NULL == pElement )
        return;
    int nPtX = 0;
    int nPtY = 0;
    int nCxRate = 0;
    int nCyRate = 0;

    if( !pElement->Attribute("ptX",&nPtX ))
        return;

    if( !pElement->Attribute("ptY",&nPtY ))

    if (!pElement->Attribute("cxRate",&nCxRate ))
        return;

    if (!pElement->Attribute("cyRate",&nCyRate))
        return;

    if(m_pResponse)
    {
        m_pResponse->ResponseScroll(dwWBID,nPtX,nPtY);
    }
}

//解析白板数据
void ClientSocket::LoadDocElement(DWORD dwWBID, TiXmlElement *pElement)
{
    if( NULL == pElement )
            return;

    if(m_pResponse)
    {
        m_pResponse->ResponseCloseAllPage(dwWBID);
    }

    int     nWidth = 0;
    int     nHeight = 0;
    int     nDocType = 0;
    int		nValue = 0;
    int		nPageCount = 0;
    int     i = 0;

    if( !pElement->Attribute( "Count",&nPageCount ))
        return;
    if( nPageCount < 1 )
        return;
    if( !pElement->Attribute( "Width",&nValue ))
        return;
    nWidth = nValue;
    if( !pElement->Attribute( "Height",&nValue ))
        return;
    nHeight = nValue;
    if( pElement->Attribute( "Type",&nValue ))
        nDocType = nValue;
    else
        nDocType = DOCTYPE_PIC;


    if(m_pResponse){
        for(i=0;i<nPageCount;i++){
            m_pResponse->ResponseAddPage(dwWBID,nWidth,nHeight,nDocType);
        }
    }

    i = 1;
    TiXmlElement *pObjElement = NULL;
    TiXmlElement *pPageElement = pElement->FirstChildElement("Page");
    while( pPageElement ){
        if( !pPageElement->Attribute("Id",&nValue ))
            break;
        if( nValue > nPageCount )
            break;

        pObjElement = pPageElement->FirstChildElement("Obj");
        while(pObjElement)
        {
            this->LoadDrawObjectElement(dwWBID,i,pObjElement,FALSE);
            pObjElement = pObjElement->NextSiblingElement();
        }
        i++;

        pPageElement = pPageElement->NextSiblingElement();
    }

//    m_bSaveBgStreamInPage = TRUE;
//    TiXmlElement *pBgElement = pElement->FirstChildElement("BG");
//    if( pBgElement ){
//        LoadBGElement( pBgElement );
//    }
}

//解析指示器数据
void ClientSocket::LoadIndicatorElement(DWORD dwWBID, TiXmlElement *pElement)
{
    if( NULL == pElement )
        return;
    int nX;
    int nY;
    if( !pElement->Attribute("X",&nX ))
        return;
    if( !pElement->Attribute("Y",&nY ))
        return;

    if(m_pResponse)
    {
        m_pResponse->ResponseIndicatorPos(dwWBID,nX,nY);
    }
}


//解析图元对象
void ClientSocket::LoadDrawObjectElement(DWORD dwWBID,int nPage, TiXmlElement *pElement,BOOL bModifyOrNew)
{
//        WB_OBJTYPE_EMPTY = 0,
//        WB_OBJTYPE_LINE,
//        WB_OBJTYPE_ARROW,
//        WB_OBJTYPE_PENCIL,
//        WB_OBJTYPE_MASKPENCIL,
//        WB_OBJTYPE_RECT,
//        WB_OBJTYPE_ROUNDRECT,
//        WB_OBJTYPE_ELLIPSE,
//        WB_OBJTYPE_TEXT,
//        WB_OBJTYPE_HAND,
//        WB_OBJTYPE_PIC

          int nObjId = 0;
          int nObjType = 0;
          int nLineStyle = 0;
          int nLineWidth = 0;
          int nLineColor = 0;
          std::deque<Point_t>  dqPoints;

          if( !pElement )
              return;

          int nValue;
          if( !pElement->Attribute( "ID",&nValue ))
              return;
          nObjId = nValue;

          if( !pElement->Attribute( "Type",&nValue ))
              return;

          nObjType = nValue;

          //WB_OBJTYPE_PENCIL
          //WB_OBJTYPE_MASKPENCIL
          if(nObjType == 3 || nObjType == 4){
              if( !pElement->Attribute( "LS",&nValue ))
                  return;
              nLineStyle = nValue;

              if( !pElement->Attribute( "LW",&nValue ))
                  return;
              nLineWidth = nValue;

              if( !pElement->Attribute( "LC",&nValue ))
                  return;
              nLineColor = nValue;

              TiXmlElement *pPtElement = pElement->FirstChildElement( "Pt" );
              while( pPtElement ){
                  Point_t pt = {0,0};
                  if( !pPtElement->Attribute("Id",&nValue ))
                      break;
                  if( !pPtElement->Attribute("X",&nValue ))
                      break;
                  pt.nX = nValue;
                  if( !pPtElement->Attribute("Y",&nValue ))
                      break;
                  pt.nY = nValue;

                  dqPoints.push_back(pt);
                  pPtElement = pPtElement->NextSiblingElement();
              }

              if(m_pResponse)
              {
                  Color_t lineColor;
                  lineColor.fromBGR(nLineColor);

                  if(nObjType == 4){
                      nObjType = DRAWOBJ_MARKPATH;
                  }else
                  {
                      nObjType = DRAWOBJ_PATH;
                  }

                  if(bModifyOrNew)
                  {
                    m_pResponse->ResponseModifyDrawObject(dwWBID,nPage - 1,nObjId,nObjType,nLineStyle,nLineWidth,lineColor.toRGBA(),dqPoints);
                  }else
                  {
                    m_pResponse->ResponseAddDrawObject(dwWBID,nPage - 1,nObjId,nObjType,nLineStyle,nLineWidth,lineColor.toRGBA(),dqPoints);
                  }
              }
          }
}

//--------------------------------- 组装协议 -----------------------------------------------------

void ClientSocket::AppendToolElement(TiXmlElement*pElement,int nPage,int nZoom)
{
    TiXmlElement element( "Tool" );
    element.SetAttribute( "Sel", nPage);
    element.SetAttribute( "Zoom",nZoom );
    pElement->InsertEndChild( element );
}

void ClientSocket::AppendRotateElement(TiXmlElement*pElement,int nAngle)
{
    TiXmlElement element( "Rotate" );
    element.SetAttribute( "Angle", nAngle);
    pElement->InsertEndChild( element );
}

void ClientSocket::AppendBkColorElement(TiXmlElement*pElement,int nColor)
{
    TiXmlElement element( "BkColor" );
    element.SetAttribute( "color",nColor);//bgr);
    pElement->InsertEndChild( element );
}

void ClientSocket::AppendIndicatorElement(TiXmlElement*pElement,int nX,int nY)
{
    TiXmlElement element( "Indicator" );
    element.SetAttribute( "X", nX);
    element.SetAttribute( "Y", nY);
    pElement->InsertEndChild( element );
}

void ClientSocket::AppendDocElement(TiXmlElement*pElement,IDoc *pDoc)
{

    if(pDoc == NULL)
        return;

    TiXmlElement element("Doc");
    TiXmlNode *pInsertNode = pElement->InsertEndChild( element );
    if( pInsertNode ){
        TiXmlElement *pDocElement = pInsertNode->ToElement();
        pDocElement->SetAttribute("Count",pDoc->GetPageCount());
        pDocElement->SetAttribute("Width",pDoc->GetPageWidth(0));
        pDocElement->SetAttribute("Height",pDoc->GetPageHeight(0));
        pDocElement->SetAttribute("Type",DOCTYPE_PIC);

        for(int i = 0;i < pDoc->GetPageCount();i++)
        {
            this->AppendPageElement(pDocElement,i,pDoc->GetPage(i));
        }
    }
}

void ClientSocket::AppendPageElement(TiXmlElement*pElement,int nIdx,IPage *pPage)
{
    if(pPage == NULL)
        return;

    TiXmlElement element("Page");
    element.SetAttribute("Id",nIdx + 1);//加1是因为以前客户端会自己默认减1,并使用ID作为页码下标，如果发送出去不加1会造成对方越界崩溃
    TiXmlNode *pInsertNode = pElement->InsertEndChild(element);
    for(int i=0;i < pPage->GetDrawObjects();i++)
    {
        this->AppendDrawObjectElement(pInsertNode->ToElement(),pPage->GetDrawObjectIndex(i));
    }
}

void ClientSocket::AppendDrawObjectElement(TiXmlElement*pElement,IDrawObject *pDrawObj)
{

    if(pDrawObj == NULL)
        return;

    int nType = 3;
    if(pDrawObj->GetObjectType() == DRAWOBJ_MARKPATH)
    {
        nType = 4;
    }

    TiXmlElement element("Obj");
    element.SetAttribute( "ID",pDrawObj->GetObjectId() );
    element.SetAttribute( "Type", nType);
    element.SetAttribute( "LS",(int)pDrawObj->GetLineStyle() );
    element.SetAttribute( "LW",pDrawObj->GetLineWidth() );
    Color_t cr;
    cr.fromRGBA(pDrawObj->GetColorRgba());
    element.SetAttribute( "LC",cr.toBGR());


    std::deque<Point_t> *pDqData = (std::deque<Point_t>*)pDrawObj->GetData();

    int nCount = pDqData->size();
    for( int i = 0;i<nCount;i++){
        TiXmlElement ptElement("Pt");
        ptElement.SetAttribute( "Id",i);
        ptElement.SetAttribute( "X",pDqData->at(i).nX );
        ptElement.SetAttribute( "Y",pDqData->at(i).nY );
        element.InsertEndChild( ptElement );
    }

    pElement->InsertEndChild(element);
}

void ClientSocket::AppendScrollElement(TiXmlElement*pElement,int nX,int nY,int nCxRate,int nCyRate)
{
    TiXmlElement element( "Scroll" );
    element.SetAttribute( "ptX",nX );
    element.SetAttribute( "ptY",nY );
    element.SetAttribute("cxRate",nCxRate);
    element.SetAttribute("cyRate",nCyRate);
    pElement->InsertEndChild( element );
}

void ClientSocket::AppendActionElement(TiXmlElement*pElement,const char*szName)
{
    TiXmlElement element( "Action" );
    element.SetAttribute( "Name",szName );
    pElement->InsertEndChild( element );
}








///
///\brief --------------------------------- INetWorkRequest接口 -----------------------------------------
///
///




/********************************************************************************************************
 * 描述：添加白板
 * 参数：
 * 环境：网络指令发送接口INetWorkRequest实现
 * 依赖：
 * 影响：
 * 日期：2017.07.26
 * 修改：无
 ********************************************************************************************************/
void ClientSocket::RequestAddWB(IContainerView *pWB)
{
    if(pWB == NULL)
        return;

    CHAR szValue[MAX_PATH];
    TiXmlElement WBElement( "WBItem" );
    WBElement.SetAttribute( "ID",pWB->GetDoc()->GetWBId() );
    const WCHAR *wszName = pWB->GetDoc()->GetTitleName();
    if( wszName == NULL )
        return;

    if( _tcslen( wszName ) <= 0 )
        return;

    //生成白板名称节点
    if( ConvertUnicodeToUtf8( wszName,szValue,MAX_PATH )){
        WBElement.SetAttribute( "Name",szValue );
    }else{
        WBElement.SetAttribute( "Name","");
    }

    //生成子文件列表节点
    //注意: 当前并没有填入具体子文件信息,需要等待文件服务器返馈
    {
        TiXmlElement element("FileList");
        WBElement.InsertEndChild( element );
    }
    //生成工具属性节点
    this->AppendToolElement(&WBElement,1,pWB->GetZoom());

    //旋转
    this->AppendRotateElement(&WBElement,0);//暂不支持旋转

    //bkcolor
    this->AppendBkColorElement(&WBElement,0xffffff);//bgr

    //Indicator
    this->AppendIndicatorElement(&WBElement,0,0);

    //生成白板页面内容节点,并读入页面数据
    this->AppendDocElement(&WBElement,pWB->GetDoc());

    //生成滚动位置节点
    this->AppendScrollElement(&WBElement,0,0,100,100);

    //页面动作
    this->AppendActionElement(&WBElement,"");

    {
        TIXML_OSTREAM stream;
        stream<<WBElement;
        this->m_WBMsgProcessor.WriteInsertDocNode( XMLDOC_INSERT_END,0,NULL,NULL,(CHAR*)stream.c_str() );
    }

    if(!m_bFirstWB)
    {
        m_bFirstWB = TRUE;
        TiXmlElement ele("WBSel");
        ele.SetAttribute("Sel",pWB->GetDoc()->GetWBId());
        TIXML_OSTREAM stream;
        stream<<ele;
        this->m_WBMsgProcessor.WriteInsertDocNode( XMLDOC_INSERT_END,1,NULL,NULL,(CHAR*)stream.c_str() );
    }
}

//添加图元
void ClientSocket::RequestAddDrawObject(int32_t nWBId,int nPage,IDrawObject *pDrawObj)
{
    CHAR szParentPath[MAX_PATH];
    sprintf( szParentPath,"WBItem ID=%d/Doc/Page Id=%d",(int)nWBId,nPage + 1 );
    std::string str;

    TiXmlElement element("");
    this->AppendDrawObjectElement(&element,pDrawObj);

    TIXML_OSTREAM stream;
    stream << *element.FirstChildElement();
    str = stream.c_str();

    this->m_WBMsgProcessor.WriteInsertDocNode( XMLDOC_INSERT_END,0,szParentPath,NULL,(CHAR*)str.c_str() );
}

//激活白板
void ClientSocket::RequestActiveWB(int32_t nWBId)
{
    TiXmlElement element("WBSel");
    element.SetAttribute( "Sel",nWBId);
    TIXML_OSTREAM stream;
    stream<<element;
    this->m_WBMsgProcessor.WriteModifyDocNode( (CHAR *)"WBSel",(CHAR*)stream.c_str() );
}

//修改图元
void ClientSocket::RequestModifyDrawObject(int32_t nWBId,int nPage,IDrawObject *pDrawObj)
{

    if(pDrawObj == NULL)
        return;

    int nObjID = pDrawObj->GetObjectId();
    CHAR  szNodePath[126] = {0};
    sprintf( szNodePath,"WBItem ID=%d/Doc/Page Id=%d/Obj ID=%d",(int)nWBId,nPage+1,nObjID );
    std::string str;


    TiXmlElement element("");
    this->AppendDrawObjectElement(&element,pDrawObj);

    TIXML_OSTREAM stream;
    stream << *element.FirstChildElement();
    str = stream.c_str();

    this->m_WBMsgProcessor.WriteModifyDocNode( szNodePath,(CHAR*)str.c_str() );
}

//缩放白板
void ClientSocket::RequestModifyZoom(int32_t nWBId,int nPage,int nZoom)
{
    CHAR  szNodePath[126] = {0};
    sprintf( szNodePath,"WBItem ID=%d/Tool",(int)nWBId );

    TiXmlElement element( "Tool" );
    element.SetAttribute( "Sel",nPage );
    element.SetAttribute( "Zoom",nZoom );
    TIXML_OSTREAM stream;
    stream<<element;

    this->m_WBMsgProcessor.WriteModifyDocNode( szNodePath,(CHAR*)stream.c_str() );
}

//指示器位置
void ClientSocket::RequestModifyIndicator(int32_t nWBId,int nX,int nY)
{
    CHAR  szNodePath[126] = {0};
    sprintf( szNodePath,"WBItem ID=%d/Indicator",(int)nWBId );

    TiXmlElement element( "Indicator" );
    element.SetAttribute( "X",nX );
    element.SetAttribute( "Y",nY );
    TIXML_OSTREAM stream;
    stream<<element;

    this->m_WBMsgProcessor.WriteModifyDocNode( szNodePath,(CHAR*)stream.c_str() );
}

//滚动条位置
void ClientSocket::RequestModifyScroll(int32_t nWBId,int nX,int nY)
{
    CHAR  szNodePath[126] = {0};
    sprintf( szNodePath,"WBItem ID=%d/Scroll",(int)nWBId );

    TiXmlElement element( "Scroll" );
    element.SetAttribute( "ptX",nX );
    element.SetAttribute( "ptY",nY );
    element.SetAttribute( "cxRate",100 );
    element.SetAttribute( "cyRate",100 );
    TIXML_OSTREAM stream;
    stream<<element;

    this->m_WBMsgProcessor.WriteModifyDocNode( szNodePath,(CHAR*)stream.c_str() );
}

//旋转白板
void ClientSocket::RequestRotate(int32_t nWBId,int nAngle)
{
    CHAR  szNodePath[126] = {0};
    sprintf( szNodePath,"WBItem ID=%d/Rotate",(int)nWBId );

    TiXmlElement element( "Rotate" );
    element.SetAttribute( "Angle",nAngle);
    TIXML_OSTREAM stream;
    stream<<element;

    this->m_WBMsgProcessor.WriteModifyDocNode( szNodePath,(CHAR*)stream.c_str() );
}

//设置背景色
void ClientSocket::RequestBkColor(int32_t nWBId,int32_t nRgbaColor)
{
    CHAR  szNodePath[126] = {0};
    sprintf( szNodePath,"WBItem ID=%d/BkColor",(int)nWBId );

    Color_t color;
    color.fromRGBA(nRgbaColor);

    TiXmlElement element( "BkColor" );
    element.SetAttribute( "color",color.toBGR());//以前协议传递的颜色值为bgr
    TIXML_OSTREAM stream;
    stream<<element;

    this->m_WBMsgProcessor.WriteModifyDocNode( szNodePath,(CHAR*)stream.c_str() );
}

//关闭白板
void ClientSocket::RequestCloseWB(int32_t nWBId)
{
    CHAR szNodePath[128] = {0};
    sprintf(szNodePath,"WBItem ID=%d",nWBId);
    this->m_WBMsgProcessor.WriteDelDocNode(szNodePath);
}

//关闭所有白板
void ClientSocket::RequestCloseAllWB()
{
    this->m_WBMsgProcessor.WriteClearDoc();
}

//删除图元
void ClientSocket::RequestDelDrawObject(int32_t nWBId,int nPage,int nObjId)
{
    CHAR  szNodePath[256] = {0};
    sprintf( szNodePath,"WBItem ID=%d/Doc/Page Id=%d/Obj ID=%d",(int)nWBId,nPage+1,nObjId );
    this->m_WBMsgProcessor.WriteDelDocNode( szNodePath );
}
