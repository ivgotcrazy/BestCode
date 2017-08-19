// XMLDocMsgProcessor.h: interface for the CXMLDocMsgProcessor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLDOCMSGPROCESSOR_H__5787CEA1_92FD_4AAD_B13C_16283AB51CF5__INCLUDED_)
#define AFX_XMLDOCMSGPROCESSOR_H__5787CEA1_92FD_4AAD_B13C_16283AB51CF5__INCLUDED_

#include "platform/types.h"

#include "ISessionManager.h"
#include "IBaseComponent.h"
#include "XMLDocProtocol.h"
#include "list"
#include "XmlMsgIOPack.h"

interface IXMLDocMsgProcessor
{
	virtual BOOL	ProcessMsg( PBYTE pbData,DWORD dwDataLen ) = 0;
};

interface IXMLDocMsgWriter
{
	virtual BOOL	WriteLoginReq( DWORD dwUserID,DWORD dwCheckCode,const GUID& guid ) = 0;
	virtual BOOL	WriteBye() = 0;
// 	virtual BOOL	WriteSendFileReq( DWORD dwWBID,const GUID& guidFile,BOOL bSaveInServer ) = 0;
	virtual BOOL	WriteGetDocReq() = 0;
	virtual BOOL	WriteClearDoc() = 0;
	virtual BOOL	WriteInsertDocNode( BYTE bInsertType,BYTE bOnlyOne,CHAR* szParentPath,CHAR* szInsertPosPath,CHAR* szNodeData ) = 0;
	virtual BOOL	WriteModifyDocNode( CHAR *szNodePath,CHAR *szNodeData ) = 0;
	virtual BOOL	WriteDelDocNode( CHAR *szNodePath ) = 0;
};

interface IXMLDocMsgReaderCallback
{
	virtual BOOL	OnLoginRep( WORD wResult ) = 0;
	virtual BOOL	OnBye() = 0;
	virtual BOOL	OnGetDocRep( CHAR *szDocData,DWORD dwDataLen ) = 0;
	virtual BOOL	OnClearDoc() = 0;
	virtual BOOL	OnInsertDocNode( BYTE bInsertType,BYTE bOnlyOne,CHAR* szParentPath,CHAR* szInsertPosPath,CHAR* szNodeData ) = 0;
	virtual BOOL	OnModifyDocNode( CHAR *szNodePath,CHAR *szNodeData ) = 0;
	virtual BOOL	OnDelDocNode( CHAR *szNodePath ) = 0;	
};

// CXMLDocMsgProcessor
class CMultiWBMsgProcessor:public IXMLDocMsgProcessor,public IXMLDocMsgWriter
{
public:
	CMultiWBMsgProcessor();
	virtual ~CMultiWBMsgProcessor();
public:
	void			Release();
	BOOL			Init( IXMLDocMsgReaderCallback *pReaderCallback,ISessionManager2* pSessionManager,IMemoryAllocator *pMemoryAllocator,WORD wSessionID );

	virtual BOOL	ProcessMsg( PBYTE pbData,DWORD dwDataLen );
	virtual BOOL	WriteLoginReq( DWORD dwUserID,DWORD dwCheckCode,const GUID& guid );
	virtual BOOL	WriteBye();
// 	virtual BOOL	WriteSendFileReq( DWORD dwWBID,const GUID& guidFile,BOOL bSaveInServer );
	virtual BOOL	WriteGetDocReq();
	virtual BOOL	WriteClearDoc();
	virtual BOOL	WriteInsertDocNode( BYTE bInsertType,BYTE bOnlyOne,CHAR* szParentPath,CHAR* szInsertPosPath,CHAR* szNodeData );
	virtual BOOL	WriteModifyDocNode( CHAR *szNodePath,CHAR *szNodeData );
	virtual BOOL	WriteDelDocNode( CHAR *szNodePath );
protected:	
	BOOL			Send( WORD wSessionID,PBYTE pbData,DWORD dwDataLen );
	BOOL			ProcessModifyDocNode( PBYTE pbData,DWORD dwDataLen );
	BOOL			ProcessInsertDocNode( PBYTE pbData,DWORD dwDataLen );
	BOOL			ProcessGetDocRep( PBYTE pbData,DWORD dwDataLen );

	BOOL				m_bLogined;
	ISessionManager2*	m_pSessionManager;
	IMemoryAllocator*	m_pMemoryAllocator;
	IXMLDocMsgReaderCallback*	m_pReaderCallback;
	IWBuffer*			m_pRecvBuffer;
	IWBuffer*			m_pSendBuffer;
	WORD				m_wSessionID;

	struct FileReqItem{
		DWORD			dwWBID;
		GUID			guidFileReq;
		BOOL            bSaveInServer; //dj 2008-1-3 是否保存在服务器上
	};
	std::list<FileReqItem>	m_lsFileReq;
        CXmlMsgIOPack		m_msgPack;
};

#endif // !defined(AFX_XMLDocMSGPROCESSOR_H__5787CEA1_92FD_4AAD_B13C_16283AB51CF5__INCLUDED_)
