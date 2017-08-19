#include "EZWhiteBoard.h"
#include "EZDoc.h"

using namespace eztalks;

CEZWhiteBoard::CEZWhiteBoard()
	:m_nActiveWBId(-1)
    ,m_pParentView(NULL)
    ,m_nViewWidth(0)
    ,m_nViewHeight(0)
    ,m_pNetworkRequest(NULL)
    ,m_nCurTool(TOOLTYPE_DRAW)
    ,m_AccessMode(WBM_ACCESS_READ)
{
    m_Pen.color.fromARGB(0xff000000);
    m_Pen.nWidth = 2;
    m_Pen.lineStyle = SolidLine;
    m_Pen.bFill = false;
    m_Pen.nFontSize = 12;
}

CEZWhiteBoard::~CEZWhiteBoard()
{
    m_pParentView = NULL;
    m_pDelegate = NULL;
    this->CloseAllWB();
    m_pNetworkRequest = NULL;
}

void CEZWhiteBoard::SetRequestNetWork(INetWorkRequest *pRequest)
{
    this->m_pNetworkRequest = pRequest;
}

IWhiteBoardPlatformDelegate* CEZWhiteBoard::GetPlatformDelegate()
{
    return m_pDelegate;
}

//设置背景色
void CEZWhiteBoard::SetBackgroundColor(Color_t color)
{
	WhiteBoardMap::iterator it;
	for (it = m_kvWhiteBoards.begin(); it != m_kvWhiteBoards.end(); it++){
		IContainerView *pView = it->second;
		pView->SetBackgroundColor(color);
	}
}

//设置白板保存文件夹
void CEZWhiteBoard::SetWBSaveDir(const wchar_t *wszSaveDir)
{
	m_strSaveDir = wszSaveDir;
}

//打开本地白板,同步到远端
int32_t CEZWhiteBoard::OpenLocalWB(const wchar_t *wszFilePath)
{
    if(m_pDelegate)
    {
        int32_t nWBId = m_pDelegate->GenerateWBID();
        while(m_kvWhiteBoards.find(nWBId) != m_kvWhiteBoards.end())
        {
            nWBId = m_pDelegate->GenerateWBID();
        }

        IDoc *pDoc = new CEZDoc();
        pDoc->Initialized(this);
        pDoc->SetWBId(nWBId);
        pDoc->SetTitleName(L"NewWhiteBoard");

        IContainerView *pContainerView = m_pDelegate->CreateContainerView();
        pContainerView->SetDoc(pDoc);
        pContainerView->Initialized(this,m_pParentView,m_nViewWidth,m_nViewHeight);
        pContainerView->CreateView(800,1024);
        pContainerView->SetAccessMode(WBM_ACCESS_ALL);
        pContainerView->Active(true);

        m_kvWhiteBoards[pDoc->GetWBId()] = pContainerView;

        pContainerView->SetDrawTool(m_nCurTool);
        pContainerView->SetLineStyle(m_Pen.lineStyle);
        pContainerView->SetColorRgba(m_Pen.color.r,m_Pen.color.g,m_Pen.color.b,m_Pen.color.a);
        pContainerView->SetLineWidth(m_Pen.nWidth);
        pContainerView->SetFontSize(m_Pen.nFontSize);
        pContainerView->SetFillMode(false);

        if(m_pNetworkRequest)
        {
            m_pNetworkRequest->RequestAddWB(pContainerView);
            m_pNetworkRequest->RequestActiveWB(pDoc->GetWBId());
        }
        m_nActiveWBId = pDoc->GetWBId();
        return pDoc->GetWBId();
    }
    return -1;
}

//打开远端白板，同步到本地
int32_t CEZWhiteBoard::OpenRemoteWB(int32_t nWBId)
{
    m_nActiveWBId = nWBId;
    if(m_pDelegate)
    {
        IDoc *pDoc = new CEZDoc();
        pDoc->Initialized(this);
        pDoc->SetWBId(nWBId);
        pDoc->SetTitleName(L"NewWhiteBoard");

        IContainerView *pContainerView = m_pDelegate->CreateContainerView();
        pContainerView->SetDoc(pDoc);
        pContainerView->Initialized(this,m_pParentView,m_nViewWidth,m_nViewHeight);
        pContainerView->CreateView(800,1024);
        pContainerView->SetAccessMode(m_AccessMode);
        pContainerView->Active(true);

        m_kvWhiteBoards[pDoc->GetWBId()] = pContainerView;

        pContainerView->SetDrawTool(m_nCurTool);
        pContainerView->SetLineStyle(m_Pen.lineStyle);
        pContainerView->SetColorRgba(m_Pen.color.r,m_Pen.color.g,m_Pen.color.b,m_Pen.color.a);
        pContainerView->SetLineWidth(m_Pen.nWidth);
        pContainerView->SetFontSize(m_Pen.nFontSize);
        pContainerView->SetFillMode(false);

        return pDoc->GetWBId();
    }
    return -1;
}

//关闭白板
void CEZWhiteBoard::CloseWB(int32_t nWBId)
{
    if(m_kvWhiteBoards.find(nWBId) != m_kvWhiteBoards.end())
    {
        IContainerView *pContainerView = m_kvWhiteBoards[nWBId];
        pContainerView->Active(false);

        if(m_pNetworkRequest)
        {
            m_pNetworkRequest->RequestCloseWB(nWBId);
        }

        m_kvWhiteBoards.erase(nWBId);
    }
}

//关闭所有白板
void CEZWhiteBoard::CloseAllWB()
{
//    WhiteBoardMap::iterator it;
//    for(it = m_kvWhiteBoards.begin();it != m_kvWhiteBoards.end();it++)
//    {
//        IContainerView *pContainerView = it->second;
//        pContainerView->Active(false);
//    }
    m_kvWhiteBoards.clear();

//    if(m_pNetworkRequest && m_AccessMode >= WBM_ACCESS_ALL)
//    {
//        m_pNetworkRequest->RequestCloseAllWB();
//    }
}

//保存白板
bool CEZWhiteBoard::SaveWB(int32_t nWBId,const wchar_t *wszSavePath)
{
    if (m_kvWhiteBoards.find(nWBId) != m_kvWhiteBoards.end())
    {
        IDoc *pDoc = m_kvWhiteBoards[m_nActiveWBId]->GetDoc();
        if (pDoc)
        {
            if(wszSavePath){
                return pDoc->Save(wszSavePath);
            }

            std::wstring strFilePath;
            strFilePath += m_strSaveDir;
            strFilePath += L"/";
            strFilePath += pDoc->GetTitleName();
            return pDoc->Save(strFilePath.c_str());
        }
    }
    return false;
}

//设置白板访问权限
void CEZWhiteBoard::SetAccessMode(WBAccessMode mode)
{
    m_AccessMode = mode;
    WhiteBoardMap::iterator it;
    for (it = m_kvWhiteBoards.begin(); it != m_kvWhiteBoards.end(); it++){
        IContainerView *pView = it->second;
        pView->SetAccessMode(mode);
    }
}

//设置缩放
void CEZWhiteBoard::SetZoom(int32_t nWBId,int nZoom)
{
    if(m_kvWhiteBoards.find(nWBId)!= m_kvWhiteBoards.end())
    {
        m_kvWhiteBoards[nWBId]->SetZoom(nZoom);

        if(m_pNetworkRequest)
        {
            m_pNetworkRequest->RequestModifyZoom(nWBId,0,nZoom);
        }
    }
}

//获取缩放
int CEZWhiteBoard::GetZoom(int32_t nWBId)
{
    if(m_kvWhiteBoards.find(nWBId) != m_kvWhiteBoards.end())
    {
        return m_kvWhiteBoards[nWBId]->GetZoom();
    }
    return -1;
}

//设置当前绘图工具
void CEZWhiteBoard::SetDrawTool(int nTool)
{
    m_nCurTool = nTool;
    WhiteBoardMap::iterator it;
    for (it = m_kvWhiteBoards.begin(); it != m_kvWhiteBoards.end(); it++){
        IContainerView *pView = it->second;
        pView->SetDrawTool(nTool);
    }
}

int CEZWhiteBoard::GetDrawTool()
{
    return m_nCurTool;
}

//设置线条宽度
void CEZWhiteBoard::SetLineWidth(int nWidth)
{
    m_Pen.nWidth = nWidth;
    WhiteBoardMap::iterator it;
    for (it = m_kvWhiteBoards.begin(); it != m_kvWhiteBoards.end(); it++){
        IContainerView *pView = it->second;
        pView->SetLineWidth(nWidth);
    }
}

//设置字体大小
void CEZWhiteBoard::SetFontSize(int nFontSize)
{
    m_Pen.nFontSize = nFontSize;
    WhiteBoardMap::iterator it;
    for (it = m_kvWhiteBoards.begin(); it != m_kvWhiteBoards.end(); it++){
        IContainerView *pView = it->second;
        pView->SetFontSize(nFontSize);
    }
}

//设置颜色
void CEZWhiteBoard::SetColorRgba(int r,int g,int b,int a)
{
    m_Pen.color.r = r;
    m_Pen.color.g = g;
    m_Pen.color.b = b;
    m_Pen.color.a = a;
    WhiteBoardMap::iterator it;
    for (it = m_kvWhiteBoards.begin(); it != m_kvWhiteBoards.end(); it++){
        IContainerView *pView = it->second;
        pView->SetColorRgba(r,g,b,a);
    }
}

//设置填充
void CEZWhiteBoard::SetFillMode(bool bFill)
{
    m_Pen.bFill = bFill;
    WhiteBoardMap::iterator it;
    for (it = m_kvWhiteBoards.begin(); it != m_kvWhiteBoards.end(); it++){
        IContainerView *pView = it->second;
        pView->SetFillMode(bFill);
    }
}

//设置线条样式
void CEZWhiteBoard::SetLineStyle(LineStyle lineStyle)
{
    m_Pen.lineStyle = lineStyle;
    WhiteBoardMap::iterator it;
    for (it = m_kvWhiteBoards.begin(); it != m_kvWhiteBoards.end(); it++){
        IContainerView *pView = it->second;
        pView->SetLineStyle(lineStyle);
    }
}

//获取线宽度
int CEZWhiteBoard::GetLineWidth()
{
    return m_Pen.nWidth;
}

//获取字体大小
int CEZWhiteBoard::GetFontSize()
{
    return m_Pen.nFontSize;
}

//获取颜色
int32_t CEZWhiteBoard::GetColorRgba()
{
    return m_Pen.color.toRGBA();
}

//是否填充
bool CEZWhiteBoard::GetFillMode()
{
    return m_Pen.bFill;
}

//获取线条样式
LineStyle CEZWhiteBoard::GetLineStyle()
{
    return m_Pen.lineStyle;
}


//保存激活的白板
bool CEZWhiteBoard::SaveActiveWB()
{
	if (m_kvWhiteBoards.find(m_nActiveWBId) != m_kvWhiteBoards.end())
	{
		IDoc *pDoc = m_kvWhiteBoards[m_nActiveWBId]->GetDoc();
		if (pDoc)
		{
			std::wstring strFilePath;
			strFilePath += m_strSaveDir;
			strFilePath += L"/";
			strFilePath += pDoc->GetTitleName();
            return pDoc->Save(strFilePath.c_str());
		}
	}
	return false;
}

//设置激活哪个白板
 void CEZWhiteBoard::SetActiveWB(int32_t nWBId)
 {
     m_nActiveWBId = nWBId;
	 WhiteBoardMap::iterator it;
     for (it = m_kvWhiteBoards.begin(); it != m_kvWhiteBoards.end();it++)
	 {
		 it->second->Active(nWBId == it->first);

         if(m_pNetworkRequest && nWBId == it->first)
         {
            m_pNetworkRequest->RequestActiveWB(nWBId);
         }
	 }
 }

//获取激活的白板是哪一个
 int32_t CEZWhiteBoard::GetActiveWB()
 {
	 return m_nActiveWBId;
 }

//获取一个白板内有多少页
 int CEZWhiteBoard::GetWBPageCount(int32_t nWBId)
 {
	 if (m_kvWhiteBoards.find(nWBId) != m_kvWhiteBoards.end())
	 {
		 return m_kvWhiteBoards[nWBId]->GetDoc()->GetPageCount();
	 }
	 return 0;
 }

//获取一共有多少个白板
 int CEZWhiteBoard::GetWBCount()
 {
	 return m_kvWhiteBoards.size();
 }

 void CEZWhiteBoard::Initialized(IWhiteBoardPlatformDelegate *pDelegate,void* pParentView,int nWidth,int nHeight)
 {
    m_pParentView = pParentView;
    m_nViewWidth = nWidth;
    m_nViewHeight = nHeight;
    m_pDelegate = pDelegate;
 }

 //消息分发
 void CEZWhiteBoard::DispatchNotifyMessage()
 {
    m_NotifyDispatch.DispatchNotifyMessage(this);
 }

 //注册消息监听
 void CEZWhiteBoard::RegNotifyMessage(INotifyMessageListener *pListener)
 {
    m_NotifyDispatch.RegNotifyMessage(pListener);
 }

 //注销消息监听
 void CEZWhiteBoard::UnRegNotifyMessage(INotifyMessageListener *pListener)
 {
    m_NotifyDispatch.UnRegNotifyMessage(pListener);
 }

 //投递消息
 void CEZWhiteBoard::PostNotifyMessage(int32_t nWBId,int32_t nMsgId,int64_t nParam1,int64_t nParam2)
 {
    m_NotifyDispatch.PostNotifyMessage(nWBId,nMsgId,nParam1,nParam2);
    this->DispatchNotifyMessage();
 }

 bool CEZWhiteBoard::OnNotifyMessageRecv(int32_t nWBId,int32_t nMsgId,int64_t nParam1,int64_t nParam2)
 {
     if(m_pNetworkRequest)
     {
         switch (nMsgId) {
         case MSGID_ADD_OBJECT:
             m_pNetworkRequest->RequestAddDrawObject(nWBId,nParam1,(IDrawObject*)nParam2);
             return true;
         case MSGID_MODIFY_OBJECT:
             m_pNetworkRequest->RequestModifyDrawObject(nWBId,nParam1,(IDrawObject*)nParam2);
             return true;
         case MSGID_MODIFY_INDICATOR:
             m_pNetworkRequest->RequestModifyIndicator(nWBId,nParam1,nParam2);
             return true;
         case MSGID_MODIFY_SCROLL:
             m_pNetworkRequest->RequestModifyScroll(nWBId,nParam1,nParam2);
             return true;
         case MSGID_DEL_OBJECT:
             m_pNetworkRequest->RequestDelDrawObject(nWBId,nParam1,nParam2);
             return true;
         }
     }

     return false;
 }


 //------------------------------------- 服务器响应 --------------------------------------------------------------

 //激活白板
 void CEZWhiteBoard::ResponseActiveWB(int32_t nWBId)
 {
     m_nActiveWBId = nWBId;

     if(m_kvWhiteBoards.find(nWBId) == m_kvWhiteBoards.end()){
        return;
     }

     WhiteBoardMap::iterator it;
     for (it = m_kvWhiteBoards.begin(); it != m_kvWhiteBoards.end();it++)
     {
         it->second->Active(nWBId == it->first);
     }
 }

 //关闭所有白板
 void CEZWhiteBoard::ResponseClearAllWB()
 {
     WhiteBoardMap::iterator it;
     for(it = m_kvWhiteBoards.begin();it != m_kvWhiteBoards.end();it++)
     {
         IContainerView *pContainerView = it->second;
         pContainerView->Active(false);
     }
     m_kvWhiteBoards.clear();
 }

 //关闭白板
 void CEZWhiteBoard::ResponseCloseWB(int32_t nWBId)
 {
     if(m_kvWhiteBoards.find(nWBId) != m_kvWhiteBoards.end())
     {
         IContainerView *pContainerView = m_kvWhiteBoards[nWBId];
         pContainerView->Active(false);
         m_kvWhiteBoards.erase(nWBId);
     }
 }

 //删除图元
 void CEZWhiteBoard::ResponseDelDrawObj(int32_t nWBId,int nPage,int32_t nObjId)
 {
     if(m_kvWhiteBoards.find(nWBId) != m_kvWhiteBoards.end())
     {
         IContainerView *pContainerView = m_kvWhiteBoards[nWBId];
         pContainerView->GetDoc()->GetPage(nPage)->DelDrawObject(nObjId);
         pContainerView->InValidate();
     }
 }

 //添加白板
 void CEZWhiteBoard::ResponseAddWB(int32_t nWBId,int nDocType,const char *szWBName)
 {
    this->OpenRemoteWB(nWBId);
 }

 //缩放白板
 void CEZWhiteBoard::ResponseZoomWB(int32_t nWBId,int nPage,int nZoom)
 {
     if (m_kvWhiteBoards.find(nWBId) != m_kvWhiteBoards.end() && nZoom != 0)
     {
         IContainerView *pContainer = m_kvWhiteBoards[nWBId];
         pContainer->SetCurView(nPage);
         pContainer->SetZoom(nZoom);
     }
 }

 //旋转白板
 void CEZWhiteBoard::ResponseAngleWB(int32_t nWBId,int nAngle)
 {

 }

 //白板背景色
 void CEZWhiteBoard::ResponseWBBackgroundColor(int32_t nWBId,int r,int g,int b)
 {

 }

 //滚动条同步
 void CEZWhiteBoard::ResponseScroll(int32_t nWBId,int nX,int nY)
 {
     if (m_kvWhiteBoards.find(nWBId) != m_kvWhiteBoards.end())
     {
          m_kvWhiteBoards[nWBId]->OnNetScroll(nX,nY);
     }
 }

 //指示器(光标)同步位置
 void CEZWhiteBoard::ResponseIndicatorPos(int32_t nWBId,int nX,int nY)
 {

 }

 //添加图元
 void CEZWhiteBoard::ResponseAddDrawObject(int32_t nWBId,int nPage,int nObjId,int nObjType,int nLineStyle,int nLineWidth,int32_t nRgbaLineColor,std::deque<Point_t>& dqPoints)
 {
     if(m_kvWhiteBoards.find(nWBId) != m_kvWhiteBoards.end())
     {
         IContainerView *pContainerView = m_kvWhiteBoards[nWBId];
         IPage *pPage = pContainerView->GetDoc()->GetPage(nPage);
         if(pPage)
         {
            IDrawObject *pDrawObj = pPage->NewDrawObject(nObjType);
            if(pDrawObj)
            {
                pPage->AddDrawObject(pDrawObj);
                pDrawObj->SetObjectId(nObjId);
                pDrawObj->SetLineWidth(nLineWidth);
                Color_t cr;
                cr.fromRGBA(nRgbaLineColor);
                pDrawObj->SetColorRgba(cr.r,cr.g,cr.b,cr.a);
                pDrawObj->SetLineStyle((LineStyle)nLineStyle);
                std::deque<Point_t>* pDqData = (std::deque<Point_t>*)pDrawObj->GetData();

                if(pDqData)
                {
                    pDqData->assign(dqPoints.begin(),dqPoints.end());
                }

                pDrawObj->RefreshBodyRect();
                pContainerView->InValidate();
            }

         }
     }
 }

 //修改图元
 void CEZWhiteBoard::ResponseModifyDrawObject(int32_t nWBId,int nPage,int nObjId,int nObjType,int nLineStyle,int nLineWidth,int32_t nRgbaLineColor,std::deque<Point_t>& dqPoints)
 {
     if(m_kvWhiteBoards.find(nWBId) != m_kvWhiteBoards.end())
     {
         IContainerView *pContainerView = m_kvWhiteBoards[nWBId];
         IPage *pPage = pContainerView->GetDoc()->GetPage(nPage);
         if(pPage)
         {
            IDrawObject *pDrawObj = pPage->GetDrawObject(nObjId);
            if(pDrawObj)
            {
                pDrawObj->SetObjectId(nObjId);
                pDrawObj->SetLineWidth(nLineWidth);
                Color_t cr;
                cr.fromRGBA(nRgbaLineColor);
                pDrawObj->SetColorRgba(cr.r,cr.g,cr.b,cr.a);
                pDrawObj->SetLineStyle((LineStyle)nLineStyle);
                std::deque<Point_t>* pDqData = (std::deque<Point_t>*)pDrawObj->GetData();

                if(pDqData)
                {
                    pDqData->clear();
                    pDqData->assign(dqPoints.begin(),dqPoints.end());
                }
                pDrawObj->RefreshBodyRect();
                pContainerView->InValidate();
            }

         }
     }
 }

 //关闭指定白板的所有页
 void CEZWhiteBoard::ResponseCloseAllPage(int32_t nWBId)
 {
     if(m_kvWhiteBoards.find(nWBId) != m_kvWhiteBoards.end())
     {
        IContainerView *pContainerView = m_kvWhiteBoards[nWBId];
        for(int i=0;i<pContainerView->GetViewsCount();i++)
        {
            pContainerView->DelView(i);
        }

        for(int i=0;i<pContainerView->GetDoc()->GetPageCount();i++){
            pContainerView->GetDoc()->DelPage(i);
        }
     }
 }

 //给指定白板增加页
 void CEZWhiteBoard::ResponseAddPage(int32_t nWBId,int nWidth,int nHeight,int nPageType)
 {
     if(m_kvWhiteBoards.find(nWBId) != m_kvWhiteBoards.end())
     {
        IContainerView *pContainerView = m_kvWhiteBoards[nWBId];
        pContainerView->CreateView(nWidth,nHeight);
        pContainerView->Active(true);
        pContainerView->SetAccessMode(m_AccessMode);

        pContainerView->SetDrawTool(m_nCurTool);
        pContainerView->SetLineStyle(m_Pen.lineStyle);
        pContainerView->SetColorRgba(m_Pen.color.r,m_Pen.color.g,m_Pen.color.b,m_Pen.color.a);
        pContainerView->SetLineWidth(m_Pen.nWidth);
        pContainerView->SetFontSize(m_Pen.nFontSize);
        pContainerView->SetFillMode(false);
     }
 }
