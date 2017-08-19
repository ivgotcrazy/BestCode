// MultiWBMsgProcessor.cpp: implementation of the CMultiWBMsgProcessor class.
//
//////////////////////////////////////////////////////////////////////

#include "MultiWBMsgProcessor.h"
#include "XmlDocProtocol.h"
#include "macros.h"
#include "assert.h"
#include <QDebug>

using namespace WBASELIB;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define		SEND_BUFFER_SIZE		65000
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMultiWBMsgProcessor::CMultiWBMsgProcessor():
m_pSessionManager( NULL ),
m_pMemoryAllocator( NULL ),
m_wSessionID ( 0 ),
m_pReaderCallback( NULL ),
m_pRecvBuffer( NULL ),
m_pSendBuffer( NULL ),
m_bLogined( FALSE )
{

}

CMultiWBMsgProcessor::~CMultiWBMsgProcessor()
{
	SAFE_RELEASE( m_pSendBuffer );
	SAFE_RELEASE( m_pRecvBuffer );
}

/********************************************************************************************************
 * 描述：初始化白板管理器
 * 参数：IXMLDocMsgReaderCallback 回调接口类、  ISessionManager2 可用的会话管理器、IMemoryAllocator 内存管理
 * 环境：初始化白板组件，连接白板服务器成功时，才调用此初始化
 ********************************************************************************************************/
BOOL CMultiWBMsgProcessor::Init( IXMLDocMsgReaderCallback *pReaderCallback,ISessionManager2 *pSessionManager, IMemoryAllocator *pMemoryAllocator,WORD wSessionID)
{
	if( NULL == pSessionManager || NULL == pMemoryAllocator )
		return FALSE;
	if( !m_msgPack.Init( pSessionManager ))
		return FALSE;

    m_pReaderCallback  = pReaderCallback;
	m_pSessionManager = pSessionManager;
	m_pMemoryAllocator = pMemoryAllocator;
	m_wSessionID = wSessionID;
	m_bLogined = FALSE;

	if( FAILED( m_pMemoryAllocator->Alloc( SEND_BUFFER_SIZE,&m_pSendBuffer )))
		return FALSE;
	
	return TRUE;
}

/********************************************************************************************************
 * 描述：处理来自白板服务器的消息
 * 参数：PBYTE pbData 数据、DWORD dwDataLen 数据长度
 * 环境：
 * 依赖：供白板客户端调用，ClientSocket::ProcessSessionEvent（）
 * 影响：需要回调客户端的接口
 ********************************************************************************************************/
BOOL	CMultiWBMsgProcessor::ProcessMsg( PBYTE pbData,DWORD dwDataLen )
{
	if( NULL == pbData || dwDataLen <= 2 || !m_pReaderCallback )
		return FALSE;
	PBYTE pbOut = NULL;
    FS_UINT32 dwOutLen = 0;
	if( !m_msgPack.UnPack( pbData,dwDataLen,pbOut,dwOutLen ))
		return FALSE;

	BOOL bResult = FALSE;
	WORD wCommand = *(WORD*)pbOut;
	switch( wCommand ) {
	case XMLDOC_CMDID_LOGINREP:
		if( dwOutLen == sizeof(XMLDOC_CMD_LOGINREP)){
			XMLDOC_CMD_LOGINREP *pLoginRep = (XMLDOC_CMD_LOGINREP*)pbOut;
			m_bLogined = pLoginRep->wResult == 0;
			bResult = m_pReaderCallback->OnLoginRep( pLoginRep->wResult );
		}
		break;
	case XMLDOC_CMDID_BYE:
		if( dwOutLen == sizeof(XMLDOC_CMD_BYE )){
			XMLDOC_CMD_BYE *pBye = (XMLDOC_CMD_BYE*)pbOut;
			bResult = m_pReaderCallback->OnBye();
		}
		break;
	case XMLDOC_CMDID_CLEARDOC:
		if( dwOutLen == sizeof(XMLDOC_CMD_CLEARDOC)){
			XMLDOC_CMD_CLEARDOC *pClearDoc = (XMLDOC_CMD_CLEARDOC*)pbOut;
			bResult = m_pReaderCallback->OnClearDoc();
		}
		break;
	case XMLDOC_CMDID_GETDOCREP:
		bResult = ProcessGetDocRep( pbOut,dwOutLen );
		break;
	case XMLDOC_CMDID_INSERTDOCNODE:
		bResult = ProcessInsertDocNode( pbOut,dwOutLen );
		break;
	case XMLDOC_CMDID_MODIFYDOCNODE:
		bResult = ProcessModifyDocNode( pbOut,dwOutLen );
		break;
	case XMLDOC_CMDID_DELDOCNODE:
		if( dwOutLen >= sizeof(XMLDOC_CMD_DELDOCNODE)){
			XMLDOC_CMD_DELDOCNODE *pDelNode = (XMLDOC_CMD_DELDOCNODE*)pbOut;
			if( dwOutLen == sizeof(XMLDOC_CMD_DELDOCNODE)+pDelNode->wNodePathLen ){
				CHAR *szNodePath = NULL;
				if( pDelNode->wNodePathLen >0 ){
					szNodePath = pDelNode->szNodePath;
					szNodePath[pDelNode->wNodePathLen-1] = 0;
				}
				bResult = m_pReaderCallback->OnDelDocNode( szNodePath );
			}
		}
		break;
	}
	return bResult;
}
/********************************************************************************************************
 * 描述：登录服务器请求
 * 参数：
 * 环境：
 * 依赖：
 * 影响：调用会话管理，发送请求
 ********************************************************************************************************/
BOOL	CMultiWBMsgProcessor::WriteLoginReq( DWORD dwUserID,DWORD dwCheckCode,const GUID& guid )
{
	XMLDOC_CMD_LOGINREQ LoginReq;
	LoginReq.wCommand = XMLDOC_CMDID_LOGINREQ;
	LoginReq.bVersion = XMLDOC_PROTOCOL_VERSION;
	LoginReq.bReserved = 0;
	LoginReq.dwUserID = dwUserID;
	LoginReq.dwCheckCode = dwCheckCode;
	LoginReq.guidDoc = guid;

    qDebug() << "[whiteboard] CMultiWBMsgProcessor::WriteLoginReq()";

	bool hr = Send( m_wSessionID,(PBYTE)&LoginReq,sizeof(LoginReq));
    return hr;
}
/********************************************************************************************************
 * 描述：
 * 参数：
 * 环境：
 * 依赖：
 * 影响：调用会话管理，发送请求
 ********************************************************************************************************/
BOOL	CMultiWBMsgProcessor::WriteBye()
{
	if( !m_bLogined )
		return FALSE;
	XMLDOC_CMD_BYE bye;
	bye.wCmdID = XMLDOC_CMDID_BYE;
	bye.wReserved = 0;

    qDebug() << "[whiteboard] CMultiWBMsgProcessor::WriteBye()";

	return Send( m_wSessionID,(PBYTE)&bye,sizeof(bye));
}
/********************************************************************************************************
 * 描述：获取文档请求
 * 参数：
 * 环境：
 * 依赖：
 * 影响：调用会话管理，发送请求
 ********************************************************************************************************/
BOOL	CMultiWBMsgProcessor::WriteGetDocReq()
{
	if( !m_bLogined )
		return FALSE;
	XMLDOC_CMD_GETDOCREQ DocReq;
	DocReq.wCmdID = XMLDOC_CMDID_GETDOCREQ;
	DocReq.wReserved = 0;

    qDebug() << "[whiteboard] CMultiWBMsgProcessor::WriteGetDocReq()";

	return Send( m_wSessionID,(PBYTE)&DocReq,sizeof(DocReq));
}
/********************************************************************************************************
 * 描述：清除文档请求
 * 参数：
 * 环境：
 * 依赖：
 * 影响：调用会话管理，发送请求
 ********************************************************************************************************/
BOOL	CMultiWBMsgProcessor::WriteClearDoc()
{
	if( !m_bLogined )
		return FALSE;
	XMLDOC_CMD_CLEARDOC ClearDoc;
	ClearDoc.wCmdID = XMLDOC_CMDID_CLEARDOC;
	ClearDoc.wReserved = 0;

    qDebug() << "[whiteboard] CMultiWBMsgProcessor::WriteClearDoc()";

	return Send( m_wSessionID,(PBYTE)&ClearDoc,sizeof(ClearDoc));
}
/********************************************************************************************************
 * 描述：加入文档节点请求
 * 参数：
 * 环境：
 * 依赖：
 * 影响：调用会话管理，发送请求
 ********************************************************************************************************/
BOOL	CMultiWBMsgProcessor::WriteInsertDocNode( BYTE bInsertType,BYTE bOnlyOne,CHAR* szParentPath,CHAR* szInsertPosPath,CHAR* szNodeData )
{
	if( !m_bLogined )
		return FALSE;
	if( NULL == m_pSendBuffer )
		return FALSE;

	int nSize = sizeof(XMLDOC_CMD_INSERTDOCNODE)+4;
	if( szParentPath )
		nSize += strlen( szParentPath )+1;
	if( szInsertPosPath)
		nSize += strlen( szInsertPosPath)+1;
	if( szNodeData )
		nSize += strlen( szNodeData)+1;
	if( nSize > SEND_BUFFER_SIZE )
		return FALSE;
	
	PBYTE pbBuffer;
	m_pSendBuffer->GetBuffer( &pbBuffer );

	nSize = 0;
	XMLDOC_CMD_INSERTDOCNODE *pInsertNode = (XMLDOC_CMD_INSERTDOCNODE*)pbBuffer;
	pInsertNode->wCmdID = XMLDOC_CMDID_INSERTDOCNODE;
	pInsertNode->bInsertType = bInsertType;
	pInsertNode->bOnlyOne = bOnlyOne;
	pInsertNode->wParentPathLen = szParentPath?strlen( szParentPath )+1:0;

	nSize = sizeof(XMLDOC_CMD_INSERTDOCNODE);
	if( szParentPath ){
		strcpy( pInsertNode->szParentPath,szParentPath );
		nSize += pInsertNode->wParentPathLen;
	}

	WORD wLen = szInsertPosPath?strlen( szInsertPosPath)+1:0;
	*(WORD*)(pbBuffer+nSize) = wLen;
	nSize += 2;
	if( szInsertPosPath ){
		strcpy( (char*)(pbBuffer+nSize),szInsertPosPath );
		nSize += wLen;
	}
	
	wLen = szNodeData?strlen( szNodeData)+1:0;
	*(WORD*)(pbBuffer+nSize) = wLen;
	nSize += 2;
	if( szNodeData ){
		strcpy((char*)(pbBuffer+nSize),szNodeData );
		nSize += wLen;
	}

    qDebug() << "[whiteboard] CMultiWBMsgProcessor::WriteInsertDocNode() " << szNodeData;
	
	return Send( m_wSessionID,pbBuffer,nSize ); 
}
/********************************************************************************************************
 * 描述：修改文档节点通知请求
 * 参数：
 * 环境：
 * 依赖：
 * 影响：调用会话管理，发送请求
 ********************************************************************************************************/
BOOL	CMultiWBMsgProcessor::WriteModifyDocNode( CHAR *szNodePath,CHAR *szNodeData )
{
	if( !m_bLogined )
		return FALSE;
	if( NULL == m_pSendBuffer )
		return FALSE;

	int nSize = sizeof(XMLDOC_CMD_MODIFYDOCNODE)+2;
	if( szNodePath )
		nSize += strlen( szNodePath )+1;
	if( szNodeData )
		nSize += strlen( szNodeData)+1;
	if( nSize > SEND_BUFFER_SIZE )
		return FALSE;
	
	PBYTE pbBuffer;
	m_pSendBuffer->GetBuffer( &pbBuffer );
	
	nSize = 0;
	XMLDOC_CMD_MODIFYDOCNODE *pModifyNode = (XMLDOC_CMD_MODIFYDOCNODE*)pbBuffer;
	pModifyNode->wCmdID = XMLDOC_CMDID_MODIFYDOCNODE;
	pModifyNode->wNodePathLen = szNodePath?strlen( szNodePath)+1:0;

	nSize = sizeof(XMLDOC_CMD_MODIFYDOCNODE);
	if( szNodePath ){
		strcpy( pModifyNode->szNodePath,szNodePath );
		nSize += pModifyNode->wNodePathLen;
	}
		
	WORD wLen = szNodeData?strlen( szNodeData)+1:0;
	*(WORD*)(pbBuffer+nSize) = wLen;
	nSize += 2;
	if( szNodeData ){
		strcpy((char*)(pbBuffer+nSize),szNodeData );
		nSize += wLen;
	}
	
	return Send( m_wSessionID,pbBuffer,nSize ); 
}
/********************************************************************************************************
 * 描述：删除文档节点请求
 * 参数：
 * 环境：
 * 依赖：
 * 影响：调用会话管理，发送请求
 ********************************************************************************************************/
BOOL	CMultiWBMsgProcessor::WriteDelDocNode( CHAR *szNodePath )
{
	if( !m_bLogined )
		return FALSE;
	if( NULL == m_pSendBuffer )
		return FALSE;
	PBYTE pbBuffer;
	m_pSendBuffer->GetBuffer( &pbBuffer );

	XMLDOC_CMD_DELDOCNODE *pDelNode = (XMLDOC_CMD_DELDOCNODE*)pbBuffer;
	pDelNode->wCmdID = XMLDOC_CMDID_DELDOCNODE;
	pDelNode->wNodePathLen = szNodePath?strlen( szNodePath)+1:0;
	if( szNodePath )
		strcpy( pDelNode->szNodePath,szNodePath );
	
    qDebug() << "[whiteboard] CMultiWBMsgProcessor::WriteInsertDocNode() " << szNodePath;
	return Send( m_wSessionID,pbBuffer,sizeof(XMLDOC_CMD_DELDOCNODE)+pDelNode->wNodePathLen);
}

///
///\brief 以下是响应服务器请求
///

/********************************************************************************************************
 * 描述：处理获取文档请求
 * 参数：
 * 环境：
 * 依赖：回调客户端的相关协议接口
 * 影响：
 ********************************************************************************************************/
BOOL CMultiWBMsgProcessor::ProcessGetDocRep(PBYTE pbData, DWORD dwDataLen)
{
	if( dwDataLen <= sizeof(XMLDOC_CMD_GETDOCREP))
		return FALSE;
	XMLDOC_CMD_GETDOCREP *pDocRep = (XMLDOC_CMD_GETDOCREP*)pbData;
	if( dwDataLen != sizeof(XMLDOC_CMD_GETDOCREP)+pDocRep->wSubLength )
		return FALSE;

	assert( !m_pRecvBuffer || pDocRep->wSubSeqnum > 0 );
	if( m_pRecvBuffer && pDocRep->wSubSeqnum == 0 )
		return FALSE;
	assert( m_pRecvBuffer || pDocRep->wSubSeqnum == 0 );
	if( NULL == m_pRecvBuffer && pDocRep->wSubSeqnum > 0 )
		return FALSE;
	if( m_pRecvBuffer ){
        FS_UINT32 dwBufferSize;
		m_pRecvBuffer->GetBufferSize( &dwBufferSize );
		assert( dwBufferSize >= pDocRep->dwDocSize );
		if( pDocRep->dwDocSize > dwBufferSize )
			return FALSE;

        FS_UINT32 dwRecvedLen;
		m_pRecvBuffer->GetLength( &dwRecvedLen );
		assert( dwRecvedLen+pDocRep->wSubLength <= pDocRep->dwDocSize );
		if( dwRecvedLen+pDocRep->wSubLength > pDocRep->dwDocSize )
			return FALSE;
	}
	if( NULL == m_pRecvBuffer ){
		m_pMemoryAllocator->Alloc( pDocRep->dwDocSize+1,&m_pRecvBuffer );
		assert( m_pRecvBuffer );
		if( NULL == m_pRecvBuffer )
			return FALSE;		
		m_pRecvBuffer->SetLength( 0 );
	}
	m_pRecvBuffer->Append( (PBYTE)pDocRep->szDocData,pDocRep->wSubLength );

    FS_UINT32 dwRecvLen;
	m_pRecvBuffer->GetLength( &dwRecvLen );
	if( dwRecvLen == pDocRep->dwDocSize ){
		PBYTE pbBuffer;
		m_pRecvBuffer->GetBuffer( &pbBuffer );
		pbBuffer[pDocRep->dwDocSize] = 0;
		m_pReaderCallback->OnGetDocRep( (char*)pbBuffer,pDocRep->dwDocSize );
		m_pRecvBuffer->SetLength( 0 );
		SAFE_RELEASE( m_pRecvBuffer );
	}
	return TRUE;
}
/********************************************************************************************************
 * 描述：处理加入文档节点请求
 * 参数：
 * 环境：
 * 依赖：回调客户端的相关协议接口
 * 影响：
 ********************************************************************************************************/
BOOL CMultiWBMsgProcessor::ProcessInsertDocNode(PBYTE pbData, DWORD dwDataLen)
{
	if( dwDataLen < sizeof(XMLDOC_CMD_INSERTDOCNODE))
		return FALSE;
		
	XMLDOC_CMD_INSERTDOCNODE *pInsertNode = (XMLDOC_CMD_INSERTDOCNODE*)pbData;
	if( dwDataLen < sizeof(XMLDOC_CMD_INSERTDOCNODE)+pInsertNode->wParentPathLen )
		return FALSE;
		
	CHAR *szParentPath = NULL;
	CHAR *szInsertNodePath = NULL;
	CHAR *szNodeData = NULL;
	if( pInsertNode->wParentPathLen > 0 ){
		pInsertNode->szParentPath[pInsertNode->wParentPathLen-1] = 0;
		szParentPath = pInsertNode->szParentPath;
	}
	int	 nPos = sizeof(XMLDOC_CMD_INSERTDOCNODE)+pInsertNode->wParentPathLen;
	if( dwDataLen < nPos+2 )
		return FALSE;
	WORD wLen = *(WORD*)(pbData+nPos);
	nPos += 2;
	if( dwDataLen <nPos+wLen )
		return FALSE;
	if( wLen > 0 ){
		szInsertNodePath = (char*)(pbData+nPos);
		szInsertNodePath[wLen-1] = 0;
		nPos += wLen;
	}

	if( dwDataLen < nPos+2 )
		return FALSE;
	wLen = *(WORD*)(pbData+nPos);
	nPos += 2;
	if( dwDataLen < nPos+wLen )
		return FALSE;
	if( wLen > 0 ){
		szNodeData = (char*)(pbData+nPos);
		szNodeData[wLen-1] = 0;
		nPos += wLen;
	}
	return m_pReaderCallback->OnInsertDocNode( pInsertNode->bInsertType,pInsertNode->bOnlyOne,szParentPath,szInsertNodePath,szNodeData );
}
/********************************************************************************************************
 * 描述：处理加入文档节点请求
 * 参数：
 * 环境：
 * 依赖：回调客户端的相关协议接口
 * 影响：
 ********************************************************************************************************/
BOOL CMultiWBMsgProcessor::ProcessModifyDocNode(PBYTE pbData, DWORD dwDataLen)
{
	if( dwDataLen < sizeof(XMLDOC_CMD_MODIFYDOCNODE))
		return FALSE;
	
	XMLDOC_CMD_MODIFYDOCNODE *pModifyNode = (XMLDOC_CMD_MODIFYDOCNODE*)pbData;
	if( dwDataLen < sizeof(XMLDOC_CMD_MODIFYDOCNODE)+pModifyNode->wNodePathLen )
		return FALSE;
	
	CHAR *szNodePath = NULL;
	CHAR *szNodeData = NULL;
	if( pModifyNode->wNodePathLen > 0 ){
		szNodePath = pModifyNode->szNodePath;
		szNodePath[pModifyNode->wNodePathLen-1] = 0;
	}
	int	 nPos = sizeof(XMLDOC_CMD_MODIFYDOCNODE)+pModifyNode->wNodePathLen;
	if( dwDataLen < nPos+2 )
		return FALSE;
	WORD wLen = *(WORD*)(pbData+nPos);
	nPos += 2;
	if( dwDataLen <nPos+wLen )
		return FALSE;
	if( wLen > 0 ){
		szNodeData = (char*)(pbData+nPos);
		szNodeData[wLen-1] = 0;
		nPos += wLen;
	}
	return m_pReaderCallback->OnModifyDocNode( szNodePath,szNodeData );
}

/********************************************************************************************************
 * 描述：释放
 * 参数：
 * 环境：
 * 依赖：回调客户端的相关协议接口
 * 影响：
 ********************************************************************************************************/
void CMultiWBMsgProcessor::Release()
{
	m_msgPack.Release();
}
/********************************************************************************************************
 * 描述：发送数据
 * 参数：
 * 环境：
 * 依赖：
 * 影响：调用会话管理器
 ********************************************************************************************************/
BOOL CMultiWBMsgProcessor::Send(WORD wSessionID, PBYTE pbData, DWORD dwDataLen)
{
	PBYTE pbOut = NULL;
    FS_UINT32 dwOutLen = 0;
	if( !m_msgPack.Pack( pbData,dwDataLen,pbOut,dwOutLen ))
		return FALSE;
	return m_pSessionManager->Send( wSessionID,pbOut,dwOutLen ) == S_OK;
}
