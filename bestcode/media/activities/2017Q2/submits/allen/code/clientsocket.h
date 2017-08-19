#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include "platform/types.h"

#include "IFileManager.h"
#include "IFileMP2.h"
#include "framework.h"
#include "isessionmanager.h"
#include "wbaseobject.h"
#include "wmsgqueue.h"
#include "wthread.h"
#include "IWMultiWhiteBoard.h"
#include "MultiWBMsgProcessor.h"
#include "kernel/core/EZWBDef.h"
#include "MsgHandler.h"
#include "MsgList.h"

using namespace WBASELIB;
using namespace eztalks;

class ClientSocket : public WThread
        ,public MsgHandler
        ,public IXMLDocMsgReaderCallback
        ,public INetWorkRequest
{
public:
    ClientSocket();
    virtual ~ClientSocket();
    BOOL Initialized(ISessionManager2 *pSessionManager,IComponentFactory *pFactory,IFileManager *pNetFileManager,const WBASE_NOTIFY &notify,FUNC_MSGTOMAIN funcMsgToMain);
    BOOL Create(const GUID *pGuid,DWORD dwUserID,DWORD dwCheckCode,WORD wAppID,const CHAR *lpszSrvAddrLink);
    void SetNetWorkResponse(INetWorkResponse *pResponse);
    void SetWBSinkSource(FUNC_WBSINK funcWbSink);
    BOOL IsWBLogin();
    UINT GetWBServerStatus();
public:
    virtual bool handleMsg(void* pMsg);//UI调用接口
    virtual FS_UINT32	ThreadProcEx();
    virtual LRESULT OnSessionNotify(UINT nMsgID,WPARAM wParam,LPARAM lParam,FS_UINT32 dwReserved,FS_UINT dwUserData);
    void ProcessSessionEvent(SESSION_EVENT *pEvent);

///
///\brief 响应接口，IXMLDocMsgReaderCallback，
///
public:
    virtual BOOL	OnLoginRep( WORD wResult );
    virtual BOOL	OnBye();
    virtual BOOL	OnGetDocRep( CHAR *szDocData,DWORD dwDataLen );
    virtual BOOL	OnClearDoc();
    virtual BOOL	OnInsertDocNode( BYTE bInsertType,BYTE bOnlyOne,CHAR* szParentPath,CHAR* szInsertPosPath,CHAR* szNodeData );
    virtual BOOL	OnModifyDocNode( CHAR *szNodePath,CHAR *szNodeData );
    virtual BOOL	OnDelDocNode( CHAR *szNodePath );


public:
    ///
    ///\brief 网络请求接口INetWorkRequest，供EZWhiteBoard调用。（主讲共享时请求服务器）
    ///
    //添加白板
    virtual void RequestAddWB(IContainerView *pWB);
    //添加图元
    virtual void RequestAddDrawObject(int32_t nWBId,int nPage,IDrawObject *pDrawObj);
    //激活白板
    virtual void RequestActiveWB(int32_t nWBId);
    //修改图元
    virtual void RequestModifyDrawObject(int32_t nWBId,int nPage,IDrawObject *pDrawObj);
    //缩放白板
    virtual void RequestModifyZoom(int32_t nWBId,int nPage,int nZoom);
    //指示器位置
    virtual void RequestModifyIndicator(int32_t nWBId,int nX,int nY);
    //滚动条位置
    virtual void RequestModifyScroll(int32_t nWBId,int nX,int nY);
    //旋转白板
    virtual void RequestRotate(int32_t nWBId,int nAngle);
    //设置背景色
    virtual void RequestBkColor(int32_t nWBId,int32_t nRgbaColor);
    //关闭白板
    virtual void RequestCloseWB(int32_t nWBId);
    //关闭所有白板
    virtual void RequestCloseAllWB();
    //删除图元
    virtual void RequestDelDrawObject(int32_t nWBId,int nPage,int nObjId);


private:
    ///
    ///\brief 对服务器响应，私有，供服务器响应函数调用。INetWorkResponse
    ///
    void LoadActionElement(DWORD dwWBID, TiXmlElement *pElement);
    void LoadWBElement(TiXmlElement *pElement);
    void LoadSelElement(TiXmlElement *pElement);
    void LoadFileListElement(DWORD dwWBID, TiXmlElement *pElement);
    void LoadWBFileElement(DWORD dwWBID, TiXmlElement *pElement);
    void LoadToolElement(DWORD dwWBID, TiXmlElement *pElement);
    void LoadRotateElement(DWORD dwWBID, TiXmlElement *pElement);
    void LoadBkColorElement(DWORD dwWBID, TiXmlElement *pElement);
    void LoadScrollElement(DWORD dwWBID, TiXmlElement *pElement);
    void LoadDocElement(DWORD dwWBID, TiXmlElement *pElement);
    void LoadIndicatorElement(DWORD dwWBID, TiXmlElement *pElement);
    void LoadDrawObjectElement(DWORD dwWBID,int nPage, TiXmlElement *pElement,BOOL bModifyOrNew = FALSE);

private:
    ///
    ///\brief 数据包协议相关
    ///
    void AppendToolElement(TiXmlElement*pElement,int nPage,int nZoom);
    void AppendRotateElement(TiXmlElement*pElement,int nAngle);
    void AppendBkColorElement(TiXmlElement*pElement,int nColor);
    void AppendIndicatorElement(TiXmlElement*pElement,int nX,int nY);
    void AppendDocElement(TiXmlElement*pElement,IDoc *pDoc);
    void AppendPageElement(TiXmlElement*pElement,int nIdx,IPage *pPage);
    void AppendDrawObjectElement(TiXmlElement*pElement,IDrawObject *pDrawObj);
    void AppendScrollElement(TiXmlElement*pElement,int nX,int nY,int nCxRate,int nCyRate);
    void AppendActionElement(TiXmlElement*pElement,const char*szName);

private:
    BOOL CreateSession(WORD wAppID,const CHAR *lpszSrvAddrLink);
    BOOL SendNotify(int nEvent);
    CHAR*ParsePath( CHAR* szPath,CHAR** szName,CHAR** szAttributeName,CHAR** szAttributeValue );
private:
    ISessionManager2            *m_pSessionManager;//白板组件的会控。接受服务器事件
    IComponentFactory           *m_pFactory;
    IFileManager                *m_pNetFileManager;
    WBASE_NOTIFY                 m_Notify;
    FUNC_MSGTOMAIN               m_fcMainThreadCall;
    FUNC_WBSINK                  m_fcWbSink;

    GUID                         m_guid;
    DWORD                        m_dwUserID;
    DWORD                        m_dwCheckCode;
    WORD                         m_wSrvAppID;
    std::string                  m_strSrvAddrLink;

    WORD                         m_wSessionID;
    BOOL                         m_bRun;

    IMemoryAllocator            *m_pMemoryAllocator;
    WElementAllocator<CMsgMpNode>m_NetMsgAllocator;
    WMsgQueue<CMsgMpNode>        m_NetMsgQueue;
    CMultiWBMsgProcessor         m_WBMsgProcessor;


    INetWorkResponse            *m_pResponse;//服务器事件响应器（白板）
    BOOL                         m_bFirstWB;
    BOOL                         m_bWbLogin;
    UINT                         m_nWBServerStatus;
};

#endif // CLIENTSOCKET_H
