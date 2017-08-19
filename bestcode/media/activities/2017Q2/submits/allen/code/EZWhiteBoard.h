#ifndef _EZWHITEBOARD_H_H_
#define _EZWHITEBOARD_H_H_

#include "EZWBDef.h"
#include <string>
#include <map>
#include "EZNotifyMsgDispatch.h"

using namespace eztalks;

typedef std::map<int32_t, IContainerView*> WhiteBoardMap;

class CEZWhiteBoard :public IWhiteBoard
        ,public INotifyMessageListener
        ,public INetWorkResponse
{
public:
	CEZWhiteBoard(void);
	~CEZWhiteBoard();

    virtual IWhiteBoardPlatformDelegate*GetPlatformDelegate();

    virtual void Initialized(IWhiteBoardPlatformDelegate *pDelegate,void* pParentView,int nWidth,int nHeight);
    virtual void SetRequestNetWork(INetWorkRequest *pRequest);

	//设置背景色
	virtual void SetBackgroundColor(Color_t color);
	//设置白板保存文件夹
	virtual void SetWBSaveDir(const wchar_t *wszSaveDir);

	//打开本地白板,同步到远端
	virtual int32_t OpenLocalWB(const wchar_t *wszFilePath);
	//打开远端白板，同步到本地
	virtual int32_t OpenRemoteWB(int32_t nWBId);
	//关闭白板
	virtual void CloseWB(int32_t nWBId);
	//关闭所有白板
	virtual void CloseAllWB();
	//保存白板
    virtual bool SaveWB(int32_t nWBId,const wchar_t *wszSavePath);
	//设置白板访问权限
	virtual void SetAccessMode(WBAccessMode mode);
	//设置缩放
	virtual void SetZoom(int32_t nWBId,int nZoom);
	//获取缩放
	virtual int GetZoom(int32_t nWBId);

	//设置当前绘图工具
	virtual void SetDrawTool(int nTool);

    virtual int GetDrawTool();

    //设置线宽度
    virtual void SetLineWidth(int nLineWidth);
    //设置字体大小
    virtual void SetFontSize(int nFontSize);
    //设置颜色
    virtual void SetColorRgba(int r,int g,int b,int a);
    //设置填充
    virtual void SetFillMode(bool bFill);
    //设置线条样式
    virtual void SetLineStyle(LineStyle lineStyle);

    //获取线宽度
    virtual int GetLineWidth();
    //获取字体大小
    virtual int GetFontSize();
    //获取颜色
    virtual int32_t GetColorRgba();
    //是否填充
    virtual bool GetFillMode();
    //获取线条样式
    virtual LineStyle GetLineStyle();

	//保存激活的白板
	virtual bool SaveActiveWB();
	//设置激活哪个白板
	virtual void SetActiveWB(int32_t nWBId);
	//获取激活的白板是哪一个
	virtual int32_t GetActiveWB();

	//获取一个白板内有多少页
	virtual int GetWBPageCount(int32_t nWBId);

	//获取一共有多少个白板
	virtual int GetWBCount();


    //消息分发
    virtual void DispatchNotifyMessage();
    //注册消息监听
    virtual void RegNotifyMessage(INotifyMessageListener *pListener);
    //注销消息监听
    virtual void UnRegNotifyMessage(INotifyMessageListener *pListener);
    //投递消息
    virtual void PostNotifyMessage(int32_t nWBId,int32_t nMsgId,int64_t nParam1,int64_t nParam2);

    virtual bool OnNotifyMessageRecv(int32_t nWBId,int32_t nMsgId,int64_t nParam1,int64_t nParam2);

public:
    //激活白板
    virtual void ResponseActiveWB(int32_t nWBId);
    //关闭所有白板
    virtual void ResponseClearAllWB();
    //关闭白板
    virtual void ResponseCloseWB(int32_t nWBId);
    //删除图元
    virtual void ResponseDelDrawObj(int32_t nWBId,int nPage,int32_t nObjId);
    //添加白板
    virtual void ResponseAddWB(int32_t nWBId,int nDocType,const char *szWBName);
    //缩放白板
    virtual void ResponseZoomWB(int32_t nWBId,int nPage,int nZoom);
    //旋转白板
    virtual void ResponseAngleWB(int32_t nWBId,int nAngle);
    //白板背景色
    virtual void ResponseWBBackgroundColor(int32_t nWBId,int r,int g,int b);
    //滚动条同步
    virtual void ResponseScroll(int32_t nWBId,int nX,int nY);
    //指示器(光标)同步位置
    virtual void ResponseIndicatorPos(int32_t nWBId,int nX,int nY);
    //添加图元
    virtual void ResponseAddDrawObject(int32_t nWBId,int nPage,int nObjId,int nObjType,int nLineStyle,int nLineWidth,int32_t nRgbaLineColor,std::deque<Point_t> &dqPoints);
    //修改图元
    virtual void ResponseModifyDrawObject(int32_t nWBId,int nPage,int nObjId,int nObjType,int nLineStyle,int nLineWidth,int32_t nRgbaLineColor,std::deque<Point_t> &dqPoints);
    //关闭指定白板的所有页
    virtual void ResponseCloseAllPage(int32_t nWBId);
    //给指定白板增加页
    virtual void ResponseAddPage(int32_t nWBId,int nWidth,int nHeight,int nPageType);

private:
    std::wstring                    m_strSaveDir;
    WhiteBoardMap                   m_kvWhiteBoards;
    int32_t                         m_nActiveWBId;
    void*                           m_pParentView;
    int                             m_nViewWidth;
    int                             m_nViewHeight;

    IWhiteBoardPlatformDelegate*    m_pDelegate;
    CEZNotifyMsgDispatch            m_NotifyDispatch;    //消息通知对象

    INetWorkRequest*                m_pNetworkRequest;

    Pen_t                           m_Pen;
    int                             m_nCurTool;
    WBAccessMode                    m_AccessMode;
};

#endif

