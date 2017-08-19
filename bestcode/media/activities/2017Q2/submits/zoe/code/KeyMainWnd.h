/*
	ʱ�䣺2016/04/08
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

// �������������
typedef E_RetCode(CKeyMainWnd::*Active_Step_Fun)();

// ������
class CKeyMainWnd : public CWindowImplBaseCtrl
{
protected:
	// DOWN - ���� �� UP - �ϴ� �� AHOST - �輤������ �� KHOST - ��Կ����
	enum Active_Step
	{
		STEP_START = 0,
		// ��Կ��������
		STEP_DOWN_KHOST_CLIREG		= 11,			// ����CliReg�ļ�
		STEP_DOWN_KHOST_PUBKEY		= 12,			// ���ع�Կ�ļ�
		STEP_DOWN_KHOST_KEY			= 13,			// ����Key�ļ�
		STEP_EXEC_KHOST_LICENSE		= 14,			// ִ�нű�������Key�ļ�
		STEP_UP_KHOST_REG			= 15,			// �ϴ�Reg�ļ�
		// ������������
		STEP_DOWN_AHOST_REG			= 21,			// ����Reg�ļ�
		STEP_EXEC_AHOST_CLIREG		= 22,			// ִ�нű�������Reg�ļ�
		STEP_EXEC_AHOST_CLEAR		= 23,			// ������ʱ�ļ�
		STEP_UP_AHOST_CLIREG		= 24,			// �ϴ�CliReg�ļ�
		STEP_UP_AHOST_PUBKEY		= 25,			// �ϴ�CliReg�ļ�
		STEP_UP_AHOST_KEY			= 26,			// �ϴ�Key�ļ�
		STEP_UP_AHOST_VIDEOSTRATEGY = 27,			// �ϴ�videostrategy�ļ�
		STEP_UP_AHOST_SOFTDETECTION = 28,			// �ϴ�softdetection�ļ�
		STEP_WEB_SEND_STREAMS		= 29,			// ��������Ϣ
		// ����
		STEP_END  = 100,
		// ��һ��
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
	bool                        m_bIsExitApp;                       // �Ƿ��˳�App
	Active_Step					m_curStep;							// ��ǰ�����
	bool						m_bSchemeConfigFinish;				// ���÷������
	string						m_szWebStreamsJson;					// ��������Ϣ��ָ��
	string						m_szWebUrl;							// web�ӿڵ�ַ
	// UI����----------------------------------------------------------------
	CVerticalLayoutUI			*m_pActivePage;						// ����ǰҳ��
	CButtonUI                   *m_pActiveBtn;						// ���ť
	CComboUI					*m_pDataSrcCbo;						// ����Դ����
	CComboUI					*m_pProductTypeCbo;					// ��Ʒ����
	CButtonUI					*m_pProductDetailConfigBtn;			// ��Ʒ��ϸ���ð�ť
	CEditUI						*m_pRbsAddress;						// ����������ַ�����
	CEditUI						*m_pRbsUsername;					// ����������¼�û��������
	CEditUI						*m_pRbsPassword;					// ����������¼���������
	CEditUI						*m_pKeyAddress;						// ��Կ������ַ�����
	CEditUI						*m_pKeyUsername;					// ��Կ������¼�û��������
	CEditUI						*m_pKeyPassword;					// ��Կ������¼���������
	CEditUI						*m_pDevModel;						// �豸�ͺ������
	CEditUI						*m_pDevSerial;						// �豸���к������
	CEditUI						*m_pDevVer;							// �豸�汾�����
	CCheckBoxUI					*m_pRememberPwChk;					// ��ס���븴ѡ��
	CButtonUI					*m_pShowOtherBtn;					// ��ʾ���������ť
	CButtonUI					*m_pHideOtherBtn;					// �������������ť
	CVerticalLayoutUI			*m_pActivingPage;					// ������ҳ��
	CLabelUI					*m_pProgressTipLabel;				// �������������ʾ
	CProgressUI                 *m_pActiveProgress;                 // ���������
	CVerticalLayoutUI			*m_pActivedPage;					// �����ҳ��
	CButtonUI                   *m_pActivedOkBtn;					// ����ɹ�ȷ�ϰ�ť
	CLabelUI					*m_pRebootTipLabel;					// ��ʾ������������״̬
	// ���ݶ���--------------------------------------------------------------
	MapActiveStepFun			m_mapActiveStepFun;					// �����ӳ��
	MapActiveStepScheme			m_mapActiveStepScheme;				// �����ӳ��
	uint32_t					m_uCurSchemeIdx = 0;				// ��ǰʹ�÷�������
	// �߼�����--------------------------------------------------------------
	CHttpUd						*m_ptrHttpUd;						// HTTPЭ���ϴ����ؿ��ƶ���
	CBaseHttp					*m_ptrWebHttp;						// ����HTTPЭ���WEBͨ�Ŷ���ָ��
	CSshUDExec					*m_ptrHostSshUdExec;				// ��������Ssh�ϴ�����ִ�п��ƶ���ָ��
	CSshUDExec					*m_ptrKeySshUdExec;					// ��Կ����Ssh�ϴ�����ִ�п��ƶ���ָ��
};

#endif // _KeyMainWnd_H_