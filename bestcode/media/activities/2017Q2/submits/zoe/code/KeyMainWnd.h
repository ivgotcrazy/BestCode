/*
	时间：2016/04/08
*/
#ifndef _KeyMainWnd_H_
#define _KeyMainWnd_H_

#include "WindowImplBaseCtrl.h"
#include "BaseCallback.h"
#include "MarcoDefine.h"
#include "RetDefine.h"
#include "KeyAppDefine.h"

class CBaseHttp;
class CHttpUd;
class CSshUDExec;
class CKeyMainWnd;

// 激活动作函数声明
typedef E_RetCode(CKeyMainWnd::*Active_Step_Fun)();

// 主界面
class CKeyMainWnd : public CWindowImplBaseCtrl
{
protected:
	// DOWN - 下载 ， UP - 上传 ， AHOST - 需激活主机 ， KHOST - 密钥主机
	enum Active_Step
	{
		STEP_START = 0,
		// 密钥主机操作
		STEP_DOWN_KHOST_CLIREG		= 11,			// 下载CliReg文件
		STEP_DOWN_KHOST_PUBKEY		= 12,			// 下载公钥文件
		STEP_DOWN_KHOST_KEY			= 13,			// 下载Key文件
		STEP_EXEC_KHOST_LICENSE		= 14,			// 执行脚本，生成Key文件
		STEP_UP_KHOST_REG			= 15,			// 上传Reg文件
		// 激活主机操作
		STEP_DOWN_AHOST_REG			= 21,			// 下载Reg文件
		STEP_EXEC_AHOST_CLIREG		= 22,			// 执行脚本，生成Reg文件
		STEP_EXEC_AHOST_CLEAR		= 23,			// 清理临时文件
		STEP_UP_AHOST_CLIREG		= 24,			// 上传CliReg文件
		STEP_UP_AHOST_PUBKEY		= 25,			// 上传CliReg文件
		STEP_UP_AHOST_KEY			= 26,			// 上传Key文件
		STEP_UP_AHOST_VIDEOSTRATEGY = 27,			// 上传videostrategy文件
		STEP_UP_AHOST_SOFTDETECTION = 28,			// 上传softdetection文件
		STEP_WEB_SEND_STREAMS		= 29,			// 设置流信息
		// 结束
		STEP_END  = 100,
		// 下一步
		SETP_NEXT = 200,
	};// step 11-12-24-25-29-22-21-15-14-13-26-27-28
	typedef std::map<Active_Step, Active_Step_Fun>	MapActiveStepFun;
	typedef MapActiveStepFun::iterator				MapActiveStepFunIt;;
	typedef std::deque<Active_Step>					ActiveStepScheme;
	typedef std::map<uint32_t, ActiveStepScheme>	MapActiveStepScheme;

public:
	CKeyMainWnd();
	virtual ~CKeyMainWnd();

public:
	virtual UINT	GetSkinResourceID();
	virtual void	Init();
	virtual void	Notify(DuiLib::TNotifyUI& msg);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

protected:
	virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);
	virtual void	ShowOtherEdits(bool bShow = true);

protected:
	void			InitStep();
	void			UnInitStep();
	void			LoadConfig();
	void			SaveConfig(bool bSaveToFile = true);
	void            OnActiveBtn();
	void			OnActivedOkBtn();
	void            OnReportError(int nResult);
	void			OnActiveSuccess();
	void			OnProductDetailConfigBtn();
	E_RetCode		Active_ExecCurStep();
	E_RetCode		Active_Ready();
	E_RetCode		Active_Step0();
	E_RetCode		Active_Step1();
	E_RetCode		Active_Step2();
	E_RetCode		Active_Step3();
	E_RetCode		Active_Step4();
	E_RetCode		Active_Step5();
	E_RetCode		Active_Step6();
	E_RetCode		Active_Step7();
	E_RetCode		Active_Step8();
	E_RetCode		Active_Step9();
	E_RetCode		Active_Step10();
	E_RetCode		Active_Step11();
	E_RetCode		Active_Step12();
	E_RetCode		Active_Step13();
	E_RetCode		Active_Step14();
	E_RetCode		RebootRemoteHost();
	void			SetCurActiveStep(Active_Step curActiveStep);
	bool			CheckActiveParam();
	E_RetCode		GenerateSchemeFile();
	void			DeleteTmpFile();
	int				ResultFromJsonMsg(const char *pMsgData, int nMsgDataLen);


private:
	BASECALLBACK_SINK(CKeyMainWnd, NextStep);
	void            OnNextStepCallback(int nResult, const char *pData, int nDataLen, void *pExtendData);
	BASECALLBACK_SINK(CKeyMainWnd, SshExecReboot);
	void            OnSshExecRebootCallback(int nResult, const char *pData, int nDataLen, void *pExtendData);

private:
	bool                        m_bIsExitApp;                       // 是否退出App
	Active_Step					m_curStep;							// 当前激活步骤
	bool						m_bSchemeConfigFinish;				// 配置方案完成
	string						m_szWebStreamsJson;					// 设置流信息的指令
	string						m_szWebUrl;							// web接口地址
	// UI对象----------------------------------------------------------------
	CVerticalLayoutUI			*m_pActivePage;						// 激活前页面
	CButtonUI                   *m_pActiveBtn;						// 激活按钮
	CComboUI					*m_pDataSrcCbo;						// 数据源类型
	CComboUI					*m_pProductTypeCbo;					// 产品类型
	CButtonUI					*m_pProductDetailConfigBtn;			// 产品详细配置按钮
	CEditUI						*m_pRbsAddress;						// 激活主机地址输入框
	CEditUI						*m_pRbsUsername;					// 激活主机登录用户名输入框
	CEditUI						*m_pRbsPassword;					// 激活主机登录密码输入框
	CEditUI						*m_pKeyAddress;						// 密钥主机地址输入框
	CEditUI						*m_pKeyUsername;					// 密钥主机登录用户名输入框
	CEditUI						*m_pKeyPassword;					// 密钥主机登录密码输入框
	CEditUI						*m_pDevModel;						// 设备型号输入框
	CEditUI						*m_pDevSerial;						// 设备序列号输入框
	CEditUI						*m_pDevVer;							// 设备版本输入框
	CCheckBoxUI					*m_pRememberPwChk;					// 记住密码复选框
	CButtonUI					*m_pShowOtherBtn;					// 显示其他输入框按钮
	CButtonUI					*m_pHideOtherBtn;					// 隐藏其他输入框按钮
	CVerticalLayoutUI			*m_pActivingPage;					// 激活中页面
	CLabelUI					*m_pProgressTipLabel;				// 激活进度文字提示
	CProgressUI                 *m_pActiveProgress;                 // 激活进度条
	CVerticalLayoutUI			*m_pActivedPage;					// 激活后页面
	CButtonUI                   *m_pActivedOkBtn;					// 激活成功确认按钮
	CLabelUI					*m_pRebootTipLabel;					// 提示主机处在重启状态
	// 数据对象--------------------------------------------------------------
	MapActiveStepFun			m_mapActiveStepFun;					// 激活步骤映射
	MapActiveStepScheme			m_mapActiveStepScheme;				// 激活方案映射
	uint32_t					m_uCurSchemeIdx = 0;				// 当前使用方案索引
	// 逻辑对象--------------------------------------------------------------
	CHttpUd						*m_ptrHttpUd;						// HTTP协议上传下载控制对象
	CBaseHttp					*m_ptrWebHttp;						// 基于HTTP协议的WEB通信对象指针
	CSshUDExec					*m_ptrHostSshUdExec;				// 激活主机Ssh上传下载执行控制对象指针
	CSshUDExec					*m_ptrKeySshUdExec;					// 密钥主机Ssh上传下载执行控制对象指针
};

#endif // _KeyMainWnd_H_