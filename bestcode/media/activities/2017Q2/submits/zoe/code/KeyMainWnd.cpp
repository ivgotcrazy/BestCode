#include "stdafx.h"
#include "resource.h"
#include "KeyMainWnd.h"
#include "AppExit.h"

#include "BaseLog.h"
#include "KeyConfig.h"
#include "HttpUd.h"
#include "SshUDExec.h"
#include "StringUtil.h"
#include "RegexUtil.h"
#include "TipOkWnd.h"
#include "SchemeConfig.h"
#include "ProductConfigWnd.h"
#include "RbsConfigHelper.h"
#include "InterJsonParse.h"
#include "MarcoDefine.h"

// 控件名称声明
const wchar_t Name_wszActivePage[] = L"ActivePage";
const wchar_t Name_wszActivingPage[] = L"ActivingPage";
const wchar_t Name_wszActivedPage[] = L"ActivedPage";

const wchar_t Name_wszActivedOkBtn[] = L"ActivedOkBtn";
const wchar_t Name_wszRebootTipLabel[] = L"RebootTipLabel";
const wchar_t Name_wszActiveProgress[] = L"ActiveProgressBar";
const wchar_t Name_wszActiveBtn[] = L"ActiveBtn";
const wchar_t Name_wszDataSrcTypeCbo[] = L"DataSrcTypeCombo";
const wchar_t Name_wszProductTypeCbo[] = L"ProductTypeCombo";
const wchar_t Name_wszProductDetailConfigBtn[] = L"ProductDetailConfigBtn";
const wchar_t Name_wszRbsAddressEdit[] = L"RBSAddressEdit";
const wchar_t Name_wszRbsUsernameEdit[] = L"RBSUsernameEdit";
const wchar_t Name_wszRbsPasswordEdit[] = L"RBSPasswordEdit";
const wchar_t Name_wszKeyAddressEdit[] = L"KeyAddressEdit";
const wchar_t Name_wszKeyUsernameEdit[] = L"KeyUsernameEdit";
const wchar_t Name_wszKeyPasswordEdit[] = L"KeyPasswordEdit";
const wchar_t Name_wszDevModelEdit[] = L"DevModelEdit";
const wchar_t Name_wszDevSerialEdit[] = L"DevSerialEdit";
const wchar_t Name_wszDevVersionEdit[] = L"DevVersionEdit";
const wchar_t Name_wszRememberPwChk[] = L"RememberPwChk";
const wchar_t Name_wszShowOthersEditBtn[] = L"ShowOthersBtn";
const wchar_t Name_wszHideOthersEditBtn[] = L"HideOthersBtn";

const wchar_t Name_wszRbsUsernameSH[] = L"RBSUsernameSH";
const wchar_t Name_wszRbsPasswordSH[] = L"RBSPasswordSH";
const wchar_t Name_wszKeyAddressSH[] = L"KeyAddressSH";
const wchar_t Name_wszKeyUsernameSH[] = L"KeyUsernameSH";
const wchar_t Name_wszKeyPasswordSH[] = L"KeyPasswordSH";
const wchar_t Name_wszDevModelSH[] = L"DevModelSH";
const wchar_t Name_wszDevSerialSH[] = L"DevSerialSH";
const wchar_t Name_wszDevVersionSH[] = L"DevVersionSH"; 
const wchar_t Name_wszOptionSetSH[] = L"OptionSetSH";

const wchar_t Name_wszActiveTipLabel[] = L"ActiveTipLabel";

const char Name_szCliReg[] = "./cli_reg";
#ifdef TEST_MYKEYSERVER
const char Name_szLicenseCmd[] = "./exec.sh";
const char Name_szCmdExecDir[] = "/home/yuan/share/";
#else
const char Name_szLicenseCmd[] = "./license.sh";
const char Name_szCmdExecDir[] = "/home/MakeLicense/";
#endif
const char Name_szRebootCmd[] = "reboot"; //"shutdown -r now ";

const char Name_szVideoStrategyFile[] = "VideoStrategy.json";
const char Name_szSoftDetectionFile[] = "SoftDetection.json";

CKeyMainWnd::CKeyMainWnd()
{
	m_bIsExitApp = true;
	m_bSchemeConfigFinish = false;

	m_pActiveProgress = NULL;
	m_pActiveBtn = NULL;

	m_ptrHttpUd = NULL;
	m_ptrWebHttp = NULL;
	m_ptrHostSshUdExec = NULL;
	m_ptrKeySshUdExec = NULL;
}

CKeyMainWnd::~CKeyMainWnd()
{
	UnInitStep();
	SAFE_RELEASE(m_ptrHttpUd);
	SAFE_DELETE(m_ptrWebHttp);
	SAFE_RELEASE(m_ptrHostSshUdExec);
	SAFE_RELEASE(m_ptrKeySshUdExec);
}

UINT CKeyMainWnd::GetSkinResourceID()
{
	return IDR_MAINWND_XML;
}

void CKeyMainWnd::Init()
{
	__super::Init();

	SetProp(m_hWnd, L"KeyApp", (HANDLE)1);

	/* 查找控件 */
	// 激活前页面
	m_pDataSrcCbo = static_cast<CComboUI *>(paint_manager_.FindControl(Name_wszDataSrcTypeCbo));
	assert(m_pDataSrcCbo);
	m_pDataSrcCbo->AddStringItem(L"软压采集卡");
	m_pDataSrcCbo->AddStringItem(L"硬压采集卡");
	m_pDataSrcCbo->AddStringItem(L"网络数据源");
	m_pProductTypeCbo = static_cast<CComboUI *>(paint_manager_.FindControl(Name_wszProductTypeCbo));
	assert(m_pProductTypeCbo);
	VecStrType schemeVec = CSchemeConfig::Instance().GetSchemeList();
	for (VecStrTypeIt it = schemeVec.begin(); it != schemeVec.end(); it++)
	{
		wstring wszItem = CStringUtil::CharToWChar((*it).c_str());
		m_pProductTypeCbo->AddStringItem(wszItem.c_str());
	}
	m_pProductDetailConfigBtn = static_cast<CButtonUI *>(paint_manager_.FindControl(Name_wszProductDetailConfigBtn));
	ASSERT(m_pProductDetailConfigBtn);
	m_pActivePage = static_cast<CVerticalLayoutUI *>(paint_manager_.FindControl(Name_wszActivePage));
	ASSERT(m_pActivePage);
	m_pActivePage->SetVisible(true);
	m_pActiveBtn = static_cast<CButtonUI *>(paint_manager_.FindControl(Name_wszActiveBtn));
	ASSERT(m_pActiveBtn);
	m_pRbsAddress = static_cast<CEditUI *>(paint_manager_.FindControl(Name_wszRbsAddressEdit));
	ASSERT(m_pRbsAddress);
	m_pRbsAddress->SetFocus();
	m_pRbsUsername = static_cast<CEditUI *>(paint_manager_.FindControl(Name_wszRbsUsernameEdit));
	ASSERT(m_pRbsUsername);
	m_pRbsPassword = static_cast<CEditUI *>(paint_manager_.FindControl(Name_wszRbsPasswordEdit));
	ASSERT(m_pRbsPassword);
	m_pKeyAddress = static_cast<CEditUI *>(paint_manager_.FindControl(Name_wszKeyAddressEdit));
	ASSERT(m_pKeyAddress);
	m_pKeyUsername = static_cast<CEditUI *>(paint_manager_.FindControl(Name_wszKeyUsernameEdit));
	ASSERT(m_pKeyUsername);
	m_pKeyPassword = static_cast<CEditUI *>(paint_manager_.FindControl(Name_wszKeyPasswordEdit));
	ASSERT(m_pKeyPassword);
	m_pDevModel = static_cast<CEditUI *>(paint_manager_.FindControl(Name_wszDevModelEdit));
	ASSERT(m_pDevModel);
	m_pDevSerial = static_cast<CEditUI *>(paint_manager_.FindControl(Name_wszDevSerialEdit));
	ASSERT(m_pDevSerial);
	m_pDevVer = static_cast<CEditUI *>(paint_manager_.FindControl(Name_wszDevVersionEdit));
	ASSERT(m_pDevVer);
	m_pRememberPwChk = static_cast<CCheckBoxUI *>(paint_manager_.FindControl(Name_wszRememberPwChk));
	ASSERT(m_pRememberPwChk);
	m_pShowOtherBtn = static_cast<CButtonUI *>(paint_manager_.FindControl(Name_wszShowOthersEditBtn));
	ASSERT(m_pShowOtherBtn);
	m_pHideOtherBtn = static_cast<CButtonUI *>(paint_manager_.FindControl(Name_wszHideOthersEditBtn));
	ASSERT(m_pHideOtherBtn);

	// 激活中页面
	m_pActivingPage = static_cast<CVerticalLayoutUI *>(paint_manager_.FindControl(Name_wszActivingPage));
	ASSERT(m_pActivingPage);
	m_pActivingPage->SetVisible(false);
	m_pProgressTipLabel = static_cast<CLabelUI *>(paint_manager_.FindControl(Name_wszActiveTipLabel));
	ASSERT(m_pProgressTipLabel);
	m_pActiveProgress = static_cast<CProgressUI *>(paint_manager_.FindControl(Name_wszActiveProgress));
	ASSERT(m_pActiveProgress);

	// 激活后页面
	m_pActivedPage = static_cast<CVerticalLayoutUI *>(paint_manager_.FindControl(Name_wszActivedPage));
	ASSERT(m_pActivedPage);
	m_pActivedPage->SetVisible(false);
	m_pActivedOkBtn = static_cast<CButtonUI *>(paint_manager_.FindControl(Name_wszActivedOkBtn));
	ASSERT(m_pActivedOkBtn);
	m_pRebootTipLabel = static_cast<CLabelUI *>(paint_manager_.FindControl(Name_wszRebootTipLabel));
	ASSERT(m_pRebootTipLabel);
	m_pRebootTipLabel->SetVisible(false);

	/* 初始化其他 */
	ShowOtherEdits(!m_pShowOtherBtn->IsVisible());
	LoadConfig();
	CheckActiveParam();
}

void CKeyMainWnd::Notify(DuiLib::TNotifyUI& msg)
{
	if (msg.sType == L"windowinit")
	{
		m_pActiveBtn->SetFocus();
		InitStep();
		SetCurActiveStep(STEP_START);
	}
	else if (msg.sType == L"click")
	{
		if (msg.pSender == m_pActiveBtn)
		{
			OnActiveBtn();
		}
		else if (msg.pSender == m_pActivedOkBtn)
		{
			OnActivedOkBtn();
		}
		else if (msg.pSender == m_pShowOtherBtn)
		{
			m_pShowOtherBtn->SetVisible(false);
			ShowOtherEdits(!m_pShowOtherBtn->IsVisible());
		}
		else if (msg.pSender == m_pHideOtherBtn)
		{
			m_pShowOtherBtn->SetVisible(true);
			ShowOtherEdits(!m_pShowOtherBtn->IsVisible());
		}
		else if (msg.pSender == m_pProductDetailConfigBtn)
		{
			OnProductDetailConfigBtn();
		}
	}
	else if (msg.sType == L"itemselect")
	{
		if (msg.pSender == m_pProductTypeCbo)
		{
			string curSchemeName = CStringUtil::WCharToChar(m_pProductTypeCbo->GetItemAt(m_pProductTypeCbo->GetCurSel())->GetText());
			CSchemeConfig::Instance().SetCurSchemeName(curSchemeName);
			CKeyConfig::Instance().SetConfigInt(Key_szSchemeType, m_pProductTypeCbo->GetCurSel());
			CSchemeDetail* pSchemeDetail = CSchemeConfig::Instance().GetSchemeDetail(CSchemeConfig::Instance().GetCurSchemeName());
			assert(pSchemeDetail);
			m_pDevModel->SetText(CStringUtil::CharToWChar(pSchemeDetail->GetConfig(Key_szSchemeModel).c_str()).c_str());
		}
	}
	__super::Notify(msg);
}

LRESULT CKeyMainWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (uMsg == WM_U_EXEC_NEXT_STEP)
	{
		E_RetCode emRet = Active_ExecCurStep();
		if (emRet != emRC_OK)
		{
			OnReportError((int)emRet);
		}
	}
	else if (uMsg == WM_U_CREATE_TIPOK_WND)
	{
		OnActivedOkBtn();
		bHandled = true;
		return 0;
	}
	return __super::HandleCustomMessage(uMsg, wParam, lParam, bHandled);
}

LRESULT CKeyMainWnd::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// 销毁窗口
	__super::OnDestroy(uMsg, wParam, lParam, bHandled);

	RemoveProp(m_hWnd, L"KeyApp");
	if (m_bIsExitApp)
	{
		CAppExit::Instance().ExitApp();
	}
	return 0;
}

LRESULT CKeyMainWnd::ResponseDefaultKeyEvent(WPARAM wParam)
{
	if (wParam == VK_RETURN)
	{
		if (m_pActivePage->IsVisible())
		{
			OnActiveBtn();
		}
		else if (m_pActivedPage->IsVisible())
		{
			OnActivedOkBtn();
		}

		return 1;
	}

	return __super::ResponseDefaultKeyEvent(wParam);
}

void CKeyMainWnd::ShowOtherEdits(bool bShow/* = true*/)
{
	const wchar_t*	items[] = { Name_wszRbsUsernameSH, Name_wszRbsPasswordSH, 
								Name_wszKeyAddressSH, Name_wszKeyUsernameSH, Name_wszKeyPasswordSH, 
								Name_wszDevModelSH, Name_wszDevSerialSH, Name_wszDevVersionSH, Name_wszOptionSetSH };
	int itemcount = sizeof(items) / sizeof(const wchar_t*);
	CHorizontalLayoutUI *pHUI;
	for (int i = 0; i < itemcount; i++)
	{
		pHUI = static_cast<CHorizontalLayoutUI *>(paint_manager_.FindControl(items[i]));
		ASSERT(pHUI);
		// 隐藏设备版本号输入框
		if (items[i] == Name_wszDevVersionSH)
		{
			continue;
		}
		pHUI->SetVisible(bShow);
	}
	m_pShowOtherBtn->SetVisible(!bShow);
}

void CKeyMainWnd::InitStep()
{
	// 初始化激活步骤映射
	if (m_mapActiveStepFun.empty())
	{
		m_mapActiveStepFun[STEP_DOWN_KHOST_CLIREG]		= &CKeyMainWnd::Active_Step9;
		m_mapActiveStepFun[STEP_DOWN_KHOST_PUBKEY]		= &CKeyMainWnd::Active_Step10;
		m_mapActiveStepFun[STEP_DOWN_KHOST_KEY]			= &CKeyMainWnd::Active_Step4;//
		m_mapActiveStepFun[STEP_EXEC_KHOST_LICENSE]		= &CKeyMainWnd::Active_Step3;//
		m_mapActiveStepFun[STEP_UP_KHOST_REG]			= &CKeyMainWnd::Active_Step2;//
		m_mapActiveStepFun[STEP_DOWN_AHOST_REG]			= &CKeyMainWnd::Active_Step1;//
		m_mapActiveStepFun[STEP_EXEC_AHOST_CLIREG]		= &CKeyMainWnd::Active_Step13;
		m_mapActiveStepFun[STEP_EXEC_AHOST_CLEAR]		= &CKeyMainWnd::Active_Step14;
		m_mapActiveStepFun[STEP_UP_AHOST_CLIREG]		= &CKeyMainWnd::Active_Step11;
		m_mapActiveStepFun[STEP_UP_AHOST_PUBKEY]		= &CKeyMainWnd::Active_Step12;
		m_mapActiveStepFun[STEP_UP_AHOST_KEY]			= &CKeyMainWnd::Active_Step5;//
		m_mapActiveStepFun[STEP_UP_AHOST_VIDEOSTRATEGY] = &CKeyMainWnd::Active_Step6;//
		m_mapActiveStepFun[STEP_UP_AHOST_SOFTDETECTION] = &CKeyMainWnd::Active_Step7;//
		m_mapActiveStepFun[STEP_WEB_SEND_STREAMS]		= &CKeyMainWnd::Active_Step0;//
		m_mapActiveStepFun[STEP_END]					= nullptr;//
	}
	if (m_mapActiveStepScheme.empty())
	{
		uint32_t uIndex = 0;
		ActiveStepScheme activeStepSchemeTmp;
		activeStepSchemeTmp.push_back(STEP_START);
		activeStepSchemeTmp.push_back(STEP_DOWN_KHOST_CLIREG);
		activeStepSchemeTmp.push_back(STEP_DOWN_KHOST_PUBKEY);
		activeStepSchemeTmp.push_back(STEP_UP_AHOST_CLIREG);
		activeStepSchemeTmp.push_back(STEP_UP_AHOST_PUBKEY);
		activeStepSchemeTmp.push_back(STEP_WEB_SEND_STREAMS);
		activeStepSchemeTmp.push_back(STEP_EXEC_AHOST_CLIREG);
		activeStepSchemeTmp.push_back(STEP_DOWN_AHOST_REG);
		activeStepSchemeTmp.push_back(STEP_UP_KHOST_REG);
		activeStepSchemeTmp.push_back(STEP_EXEC_KHOST_LICENSE);
		activeStepSchemeTmp.push_back(STEP_DOWN_KHOST_KEY);
		activeStepSchemeTmp.push_back(STEP_UP_AHOST_KEY);
		activeStepSchemeTmp.push_back(STEP_UP_AHOST_VIDEOSTRATEGY);
		activeStepSchemeTmp.push_back(STEP_UP_AHOST_SOFTDETECTION);
		activeStepSchemeTmp.push_back(STEP_EXEC_AHOST_CLEAR);
		activeStepSchemeTmp.push_back(STEP_END);
		m_mapActiveStepScheme[uIndex] = activeStepSchemeTmp;
		uIndex++;
	}
	assert(m_mapActiveStepScheme.size() >= 1);
}

void CKeyMainWnd::UnInitStep()
{
	m_mapActiveStepFun.clear();
	m_mapActiveStepScheme.clear();
}

void CKeyMainWnd::LoadConfig()
{
	const char*	szKeyItems[] = {	Key_szHostAddress, Key_szHostUsername, Key_szHostPassword, 
									Key_szKeyserverAddress, Key_szKeyserverUsername, Key_szKeyserverPassword,
									Key_szDevModel, Key_szDevSerial, Key_szDevVer };
	CEditUI* ptrEditUIArray[] = {	m_pRbsAddress, m_pRbsUsername, m_pRbsPassword,
									m_pKeyAddress, m_pKeyUsername, m_pKeyPassword,
									m_pDevModel, m_pDevSerial, m_pDevVer };
	int itemCount = sizeof(szKeyItems) / sizeof(const char*);
	for (int i = 0; i < itemCount; i++)
	{
		string szKeyValue = CKeyConfig::Instance().GetConfig(szKeyItems[i]);
		CEditUI* ptrEditUI = ptrEditUIArray[i];
		if (!szKeyValue.empty() && ptrEditUI->IsVisible())
		{
			if (ptrEditUI->IsPasswordMode())
			{
				szKeyValue = CStringUtil::SimpleDecryption(szKeyValue);
			}
			ptrEditUI->SetText(CStringUtil::CharToWChar(szKeyValue.c_str()).c_str());
		}
	}

	// 设备序列号生成
	SYSTEMTIME sys;
	::GetLocalTime(&sys);
	CStdString strDevSerial;
	strDevSerial.Format(L"%04d%02d%02d%02d%02d%02d", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);
	m_pDevSerial->SetText(strDevSerial);

	int nRememberPw = CKeyConfig::Instance().GetConfigInt(Key_nRememberPw);
	m_pRememberPwChk->SetCheck(nRememberPw==0?false:true);
	int nSchemeIndex = CKeyConfig::Instance().GetConfigInt(Key_szSchemeType);
	m_pProductTypeCbo->SelectItem(nSchemeIndex);
	int nDataSrcType = CKeyConfig::Instance().GetConfigInt(Key_szDataSrcType);
	m_pDataSrcCbo->SelectItem(nDataSrcType);
}

void CKeyMainWnd::SaveConfig(bool bSaveToFile/* = true*/)
{
	const char*	szKeyItems[] = {	Key_szHostAddress, Key_szHostUsername, Key_szHostPassword,
									Key_szKeyserverAddress, Key_szKeyserverUsername, Key_szKeyserverPassword,
									Key_szDevModel, Key_szDevSerial, Key_szDevVer };
	CEditUI* ptrEditUIArray[] = {	m_pRbsAddress, m_pRbsUsername, m_pRbsPassword,
									m_pKeyAddress, m_pKeyUsername, m_pKeyPassword,
									m_pDevModel, m_pDevSerial, m_pDevVer };
	int itemCount = sizeof(szKeyItems) / sizeof(const char*);
	for (int i = 0; i < itemCount; i++)
	{
		CEditUI* ptrEditUI = ptrEditUIArray[i];
		string szKeyValue = CStringUtil::WCharToChar(ptrEditUI->GetText());
		if (ptrEditUI->IsVisible() && !szKeyValue.empty())
		{
			if (ptrEditUI->IsPasswordMode())
			{
				szKeyValue = CStringUtil::SimpleEncryption(szKeyValue);
			}
			CKeyConfig::Instance().SetConfig(szKeyItems[i], szKeyValue.c_str());
		}
	}

	string szRememberPw = m_pRememberPwChk->GetCheck() ? "1" : "0";
	CKeyConfig::Instance().SetConfig(Key_nRememberPw, szRememberPw.c_str());
	CKeyConfig::Instance().SetConfigInt(Key_szSchemeType, m_pProductTypeCbo->GetCurSel());
	CKeyConfig::Instance().SetConfigInt(Key_szDataSrcType, m_pDataSrcCbo->GetCurSel());
	CKeyConfig::Instance().SetConfigInt(Key_szSchemeType, 2);
	if (bSaveToFile)
	{
		CKeyConfig::Instance().SaveConfig();
		//CSchemeConfig::Instance(m_pProductTypeCbo->GetCurSel()).SaveConfig();
	}

	// 新设备序列号生成
	SYSTEMTIME sys;
	::GetLocalTime(&sys);
	CStdString strDevSerial;
	strDevSerial.Format(L"%04d%02d%02d%02d%02d%02d", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);
	m_pDevSerial->SetText(strDevSerial);
}

void CKeyMainWnd::OnActiveBtn()
{
	// 开始激活
	E_RetCode emResult = emRC_OK;

	CHECK_RETURN(!CheckActiveParam());
	SaveConfig(false);

	do 
	{
		m_pActivePage->SetVisible(false);
		m_pActivingPage->SetVisible(true);

		emResult = Active_Ready();
		CHECK_BREAK(emResult != emRC_OK);
		SetCurActiveStep(SETP_NEXT);
		emResult = Active_ExecCurStep();
		CHECK_BREAK(emResult != emRC_OK);
	} while (false);

	if (emResult != emRC_OK)
	{
		OnReportError((int)emResult);
	}
}

void CKeyMainWnd::OnActivedOkBtn()
{
	bool bNotReboot = true;
	do 
	{
		string szUsername = CKeyConfig::Instance().GetConfig(Key_szHostUsername);
		if (szUsername.compare("root") != 0)
		{
			break;
		}

		CTipOkWnd* pTipOkWnd = new CTipOkWnd();
		if (pTipOkWnd == NULL)
		{
			break;
		}
		pTipOkWnd->Create(this->m_hWnd, L"", UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
		pTipOkWnd->CenterWindow();
		if (IDYES == pTipOkWnd->ShowModal())
		{
			E_RetCode nResult = RebootRemoteHost();
			bNotReboot = (nResult == emRC_OK) ? false : true;
		}
	} while (false);
	
	if (bNotReboot)
	{
		LOG_ERROR("Do not reboot actived host.");
		m_pActivePage->SetVisible(true);
		m_pActivingPage->SetVisible(false);
		m_pActivedPage->SetVisible(false);
	}
}

void CKeyMainWnd::OnReportError(int nResult)
{
	CAtlString wszMsg(L"");
	wszMsg.Format(L"激活主机失败:[%d].", nResult);
	MessageBoxW(m_hWnd, wszMsg, L"错误", MB_ICONERROR);

	LOG_ERROR("Active Host Fail:[%d].", nResult);

	DeleteTmpFile();

	Sleep(100);

	m_pActivePage->SetVisible(true);
	m_pActivingPage->SetVisible(false);
	SetCurActiveStep(STEP_START);
}

void CKeyMainWnd::OnActiveSuccess()
{
	if (m_pRememberPwChk->GetCheck())
	{
		SaveConfig();
	}

	Sleep(500);

	LOG_INFO("Active Host Success.");

	m_pActivePage->SetVisible(false);
	m_pActivingPage->SetVisible(false);
	m_pActivedOkBtn->SetVisible(true);
	m_pRebootTipLabel->SetVisible(false);
	m_pActivedPage->SetVisible(true);

	Sleep(100);

	SetCurActiveStep(STEP_START);

	DeleteTmpFile();

	PostMessage(WM_U_CREATE_TIPOK_WND);
}

void CKeyMainWnd::OnProductDetailConfigBtn()
{
	do
	{
		int curSchemeIndex = m_pProductTypeCbo->GetCurSel();
		if (curSchemeIndex<0 || curSchemeIndex>m_pProductTypeCbo->GetCount())
		{
			m_pProductTypeCbo->SetFocus();
			break;
		}
		CProductConfigWnd* pProductConfigWnd = new CProductConfigWnd();
		if (pProductConfigWnd == NULL)
		{
			break;
		}
		pProductConfigWnd->Create(this->m_hWnd, L"", UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
		pProductConfigWnd->CenterWindow();
		if (IDYES == pProductConfigWnd->ShowModal())
		{
			if (m_pProductTypeCbo->GetCurSel() != CKeyConfig::Instance().GetConfigInt(Key_szSchemeType))
			{
				m_pProductTypeCbo->SelectItem(CKeyConfig::Instance().GetConfigInt(Key_szSchemeType));
			}
		}
	} while (false);
}

void CKeyMainWnd::SetCurActiveStep(Active_Step curActiveStep)
{
	ActiveStepScheme curActiveStepScheme = m_mapActiveStepScheme[m_uCurSchemeIdx];
	uint32_t curStepIdx = 0;
	for (; curStepIdx < curActiveStepScheme.size(); curStepIdx++)
	{
		CHECK_BREAK(m_curStep == curActiveStepScheme.at(curStepIdx));
	}
	Active_Step nextStep = curStepIdx >= (curActiveStepScheme.size() - 1) ? m_curStep : curActiveStepScheme.at(curStepIdx + 1);
	m_curStep = (curActiveStep == SETP_NEXT) ? nextStep : curActiveStep;
	for (curStepIdx = 0; curStepIdx < curActiveStepScheme.size(); curStepIdx++)
	{
		CHECK_BREAK(m_curStep == curActiveStepScheme.at(curStepIdx));
	}
	int curProgress = (m_pActiveProgress->GetMaxValue() - m_pActiveProgress->GetMinValue()) * curStepIdx / (curActiveStepScheme.size() - 1);
	m_pActiveProgress->SetValue(m_pActiveProgress->GetMinValue() + curProgress);
	CStdString stdStr;
	stdStr.Format(L"%d/%d", curStepIdx, (curActiveStepScheme.size() - 1));
	m_pProgressTipLabel->SetText(stdStr);

	LOG_INFO("Current step is: %d ", m_curStep);
}

bool CKeyMainWnd::CheckActiveParam()
{
	bool bRet = true;

	CEditUI* ptrEditUIArray[] = {	m_pRbsAddress, m_pRbsUsername, m_pRbsPassword,
									m_pKeyAddress, m_pKeyUsername, m_pKeyPassword,
									m_pDevModel, m_pDevSerial, m_pDevVer };
	int itemCount = sizeof(ptrEditUIArray) / sizeof(const char*);
	for (int i = 0; i < itemCount; i++)
	{
		CStdString wszValue = ptrEditUIArray[i]->GetText();
		if (wszValue.IsEmpty())
		{
			ptrEditUIArray[i]->SetFocus();
			if (i > 0)
			{
				ShowOtherEdits();
			}
			bRet = false;
			break;
		}
		else
		{
			ASSERT(itemCount == 9);
			// 检查IP和设备版本
			string szValue = CStringUtil::WCharToChar(wszValue);
			switch (i)
			{
			case 0:;
			case 3: bRet = CRegexUtil::IsIp(szValue.c_str());
				break;
			case 8: bRet = CRegexUtil::IsDevVer(szValue.c_str());
				break;
			default:
				break;
			}
			if (!bRet)
			{
				ptrEditUIArray[i]->SetFocus();
				break;
			}
		}
	}
	/*int customSchemeIndex = m_pProductTypeCbo->GetCount() - 1;
	int curSchemeIndex = m_pProductTypeCbo->GetCurSel();
	if (!(curSchemeIndex >=0 && curSchemeIndex < customSchemeIndex) && !(curSchemeIndex == customSchemeIndex && m_bSchemeConfigFinish))
	{
		m_pProductTypeCbo->SetFocus();
		OnProductDetailConfigBtn();
		bRet = false;
	}*/

	return bRet;
}

E_RetCode CKeyMainWnd::GenerateSchemeFile()
{
	CRbsConfigHelper rbsconfigHelper;
	E_RetCode nResult = emRC_OK;
	VIDEO_STRATEGY_INFO stVideoStrategyInfo;

	do
	{
		int curSchemeIndex = m_pProductTypeCbo->GetCurSel();
		if (curSchemeIndex < 0 || curSchemeIndex >= m_pProductTypeCbo->GetCount())
		{
			nResult = emRC_UNKNOW;
			break;
		}
		nResult = rbsconfigHelper.InitConfigParam();
		if (nResult != emRC_OK)
		{
			break;
		}
		CSchemeDetail* pSchemeDetail = CSchemeConfig::Instance().GetSchemeDetail(CSchemeConfig::Instance().GetCurSchemeName());
		if (pSchemeDetail == NULL)
		{
			nResult = emRC_UNKNOW;
			break;
		}
		VecVideoSrcInfoType allVideoSrcInfo = CSchemeConfig::Instance().GetAllVideoSrc();
		VecStrType videoSrcVec = pSchemeDetail->GetVideoSrc();
		for (VecStrTypeIt it = videoSrcVec.begin(); it != videoSrcVec.end(); it++)
		{
			nResult = rbsconfigHelper.GetVideoResource((*it).c_str(), &stVideoStrategyInfo);
			for (VecVideoSrcInfoType::size_type i = 0; i < allVideoSrcInfo.size(); i++)
			{
				string szKeyPrefix = allVideoSrcInfo.at(i).szVideoSrcPrefix;
				string szKeyname;
				if (*it == allVideoSrcInfo.at(i).szVideoSrcName)
				{
					stVideoStrategyInfo.uVideoID = allVideoSrcInfo.at(i).uVideoID;
					stVideoStrategyInfo.stStrategyParam.uVideoID = stVideoStrategyInfo.uVideoID;
					if (allVideoSrcInfo.at(i).szVideoSettinsNode.size() > 0)
					{
						// 云台地址
						szKeyname = szKeyPrefix + Key_szPTZAddress;
						int nValue = pSchemeDetail->GetConfigInt(szKeyname.c_str());
						if (nValue >= 0)
						{
							stVideoStrategyInfo.uPtzAddress = nValue;
						}
						// 云台序列号
						szKeyname = szKeyPrefix + Key_szPTZSerail;
						nValue = pSchemeDetail->GetConfigInt(szKeyname.c_str());
						if (nValue >= 0)
						{
							stVideoStrategyInfo.uPtzSerail = nValue;
						}
						// 是否检测
						szKeyname = szKeyPrefix + Key_szDetectionSW;
						nValue = pSchemeDetail->GetConfigInt(szKeyname.c_str());
						if (nValue >= 0)
						{
							stVideoStrategyInfo.stStrategyParam.bIsDetection = (nValue!=0);
						}
						// 检测模式
						szKeyname = szKeyPrefix + Key_szDetectMode;
						nValue = pSchemeDetail->GetConfigInt(szKeyname.c_str());
						if (nValue >= 0)
						{
							stVideoStrategyInfo.stStrategyParam.bIsSoftDetection = (nValue == 0);
						}
						// 超时
						szKeyname = szKeyPrefix + Key_szTimeout;
						nValue = pSchemeDetail->GetConfigInt(szKeyname.c_str());
						if (nValue >= 0)
						{
							stVideoStrategyInfo.stStrategyParam.uTimeOut = nValue;
						}
						// 切换至
						szKeyname = szKeyPrefix + Key_szTriggerTo;
						nValue = pSchemeDetail->GetConfigInt(szKeyname.c_str());
						if (nValue >= 0 && (unsigned int)nValue < videoSrcVec.size())
						{
							for (VecVideoSrcInfoType::size_type k = 0; k < allVideoSrcInfo.size(); k++)
							{
								if (allVideoSrcInfo.at(k).szVideoSrcName == videoSrcVec[nValue])
								{
									stVideoStrategyInfo.stStrategyParam.uJoinVideoID = allVideoSrcInfo.at(k).uVideoID;
									break;
								}
							}
						}
						// 切换预置位
						szKeyname = szKeyPrefix + Key_szTriggerPreset;
						nValue = pSchemeDetail->GetConfigInt(szKeyname.c_str());
						if (nValue >= 0)
						{
							stVideoStrategyInfo.stStrategyParam.uJoinVideoPreset = nValue;
						}
						// 优先级
						szKeyname = szKeyPrefix + Key_szPriority;
						nValue = pSchemeDetail->GetConfigInt(szKeyname.c_str());
						if (nValue >= 0)
						{
							stVideoStrategyInfo.stStrategyParam.uDirectorPriority = nValue;
						}
						// 共用摄像头
						szKeyname = szKeyPrefix + Key_szShareVideoSrcIndex;
						nValue = pSchemeDetail->GetConfigInt(szKeyname.c_str());
						if (nValue >= 0 && (unsigned int)nValue < videoSrcVec.size())
						{
							for (VecVideoSrcInfoType::size_type k = 0; k < allVideoSrcInfo.size(); k++)
							{
								if (allVideoSrcInfo.at(k).szVideoSrcName == videoSrcVec[nValue])
								{
									stVideoStrategyInfo.stStrategyParam.uShareVideoID = allVideoSrcInfo.at(k).uVideoID;
									break;
								}
							}
						}
					}
					if (allVideoSrcInfo.at(i).szSoftDetectionSettinsNode.size() > 0)
					{
						// 最小变化阈值宽度
						szKeyname = szKeyPrefix + Key_szMinChangeThresholdWidth;
						int nValue = pSchemeDetail->GetConfigInt(szKeyname.c_str());
						if (nValue >= 0)
						{
							stVideoStrategyInfo.stSoftDetectionParam.uMinTargetsWidth = nValue;
						}
						// 最小变化阈值高度
						szKeyname = szKeyPrefix + Key_szMinChangeThresholdHeight;
						nValue = pSchemeDetail->GetConfigInt(szKeyname.c_str());
						if (nValue >= 0)
						{
							stVideoStrategyInfo.stSoftDetectionParam.uMinTargetsHeight = nValue;
						}
						// 最小对象大小阈值
						szKeyname = szKeyPrefix + Key_szMinTargetSizeThreshold;
						nValue = pSchemeDetail->GetConfigInt(szKeyname.c_str());
						if (nValue >= 0)
						{
							stVideoStrategyInfo.stSoftDetectionParam.uObjectThreshold = nValue;
						}
						// 检测分隔线高度偏移
						szKeyname = szKeyPrefix + Key_szSepLineHeightOffset;
						nValue = pSchemeDetail->GetConfigInt(szKeyname.c_str());
						if (nValue >= 0)
						{
							stVideoStrategyInfo.stSoftDetectionParam.fDetectionLineY = nValue / 100.0f;
						}
						// 检测帧间隔
						szKeyname = szKeyPrefix + Key_szFrameSpace;
						nValue = pSchemeDetail->GetConfigInt(szKeyname.c_str());
						if (nValue >= 0)
						{
							stVideoStrategyInfo.stSoftDetectionParam.uDetectionFrameInterval = nValue;
						}
					}
					break;
				}
			}
			rbsconfigHelper.AddVideoResource(&stVideoStrategyInfo);
		}

		// 各个方案的默认值
		DEFAULT_VALUE stDefaultValue;
		int nValue = pSchemeDetail->GetConfigInt(Key_szDirectorVideoIndex);
		for (VecVideoSrcInfoType::size_type k = 0; k < allVideoSrcInfo.size(); k++)
		{
			if (allVideoSrcInfo.at(k).szVideoSrcName == videoSrcVec[nValue])
			{
				stDefaultValue.uDirectorVideoID = allVideoSrcInfo.at(k).uVideoID;
				break;
			}
		}
		nValue = pSchemeDetail->GetConfigInt(Key_szSubDirectorVideoIndex);
		if (nValue < 0)
		{
			stDefaultValue.uSubDirectorVideoID = 0;
		}
		else
		{
			for (VecVideoSrcInfoType::size_type k = 0; k < allVideoSrcInfo.size(); k++)
			{
				if ((nValue >= 0 && (unsigned int)nValue < videoSrcVec.size()) && allVideoSrcInfo.at(k).szVideoSrcName == videoSrcVec[nValue])
				{
					stDefaultValue.uSubDirectorVideoID = allVideoSrcInfo.at(k).uVideoID;
					break;
				}
			}
		}
		stDefaultValue.uDataSrcType = m_pDataSrcCbo->GetCurSel();
		rbsconfigHelper.AddDefaultValue(&stDefaultValue);

		nResult = rbsconfigHelper.BuildConfigFile(&m_szWebStreamsJson);
		if (nResult != emRC_OK)
		{
			break;
		}
	} while (false);

	return nResult;
}

void CKeyMainWnd::DeleteTmpFile()
{
	string szBinPath = CAppParam::Instance().GetExecDir();
	const char*	szItems1[] = { Key_szCliRegFilename, Key_szPubKeyFilename, Key_szRegFilename, Key_szKeyFilename };
	int itemCount = sizeof(szItems1) / sizeof(const char*);
	for (int i = 0; i < itemCount; i++)
	{
		string szPathTmp = szBinPath + CKeyConfig::Instance().GetConfig(szItems1[i]);
		remove(szPathTmp.c_str());
	}
	string szCurPath = "./";
	const char*	szItems2[] = { Name_szVideoStrategyFile, Name_szSoftDetectionFile, SYSCONFIG_JSON_FILENAME };
	itemCount = sizeof(szItems2) / sizeof(const char*);
	for (int i = 0; i < itemCount; i++)
	{
		string szPathTmp = szCurPath + szItems2[i];
		remove(szPathTmp.c_str());
	}
}

int	CKeyMainWnd::ResultFromJsonMsg(const char *pMsgData, int nMsgDataLen)
{
	int nResult = 0;

	do 
	{
		CInterJsonParse JsonParse;
		nResult = JsonParse.Parse(pMsgData, nMsgDataLen, true);
		CHECK_BREAK(IS_FAILED(nResult));
		nResult = JsonParse.GetResult();
	} while (false);

	return nResult;
}

E_RetCode CKeyMainWnd::Active_ExecCurStep()
{
	E_RetCode emRet = emRC_OK;
	MapActiveStepFunIt it = m_mapActiveStepFun.find(m_curStep);
	if (it->first == STEP_END || it->second == nullptr)
	{
		OnActiveSuccess();
	}
	else
	{
		emRet = (this->*(it->second))();
	}

	return emRet;
}

E_RetCode CKeyMainWnd::Active_Ready()
{
	E_RetCode emResult = emRC_OK;

	do
	{
		// WEB通信对象
		SAFE_DELETE(m_ptrWebHttp);
		m_ptrWebHttp = new CBaseHttp;
		if (m_ptrWebHttp == NULL)
		{
			break;
		}
		m_ptrWebHttp->SetRequestCallback(m_spNextStepCallback);
		// 激活主机SSH通信对象
		SAFE_RELEASE(m_ptrHostSshUdExec);
		emResult = CSshUDExec::Create(&m_ptrHostSshUdExec);
		if (emResult != emRC_OK)
		{
			break;
		}
		string szPassword = CKeyConfig::Instance().GetConfig(Key_szHostPassword);
		emResult = m_ptrHostSshUdExec->SetHostParam(CKeyConfig::Instance().GetConfig(Key_szHostAddress), CKeyConfig::Instance().GetConfigInt(Key_nHostPort),
			CKeyConfig::Instance().GetConfig(Key_szHostUsername), CStringUtil::SimpleDecryption(szPassword));
		if (emResult != emRC_OK)
		{
			break;
		}
		// 密钥主机SSH通信对象
		SAFE_RELEASE(m_ptrKeySshUdExec);
		emResult = CSshUDExec::Create(&m_ptrKeySshUdExec);
		if (emResult != emRC_OK)
		{
			break;
		}
		szPassword = CKeyConfig::Instance().GetConfig(Key_szKeyserverPassword);
		emResult = m_ptrKeySshUdExec->SetHostParam(CKeyConfig::Instance().GetConfig(Key_szKeyserverAddress), CKeyConfig::Instance().GetConfigInt(Key_nKeyserverPort),
			CKeyConfig::Instance().GetConfig(Key_szKeyserverUsername), CStringUtil::SimpleDecryption(szPassword));
		if (emResult != emRC_OK)
		{
			break;
		}
		// 生成配置文件和设置数据源内容
		emResult = GenerateSchemeFile();
		if (emResult != emRC_OK)
		{
			break;
		}
		// 生成WebUrl
		m_szWebUrl = "http://";
		m_szWebUrl.append(CKeyConfig::Instance().GetConfig(Key_szHostAddress));
		m_szWebUrl.append("/index.php/unifiedInterface");
	} while (false);

	return emResult;
}

E_RetCode CKeyMainWnd::Active_Step0()
{
	Sleep(100);
	E_RetCode emResult = m_ptrWebHttp->SendRequest(m_szWebUrl.c_str(), m_szWebStreamsJson.c_str(), m_szWebStreamsJson.size());
	if (emResult != emRC_OK)
	{
		LOG_ERROR("Send Web Request Fail Data:\r\n%s", m_szWebStreamsJson.c_str());
	}
	else
	{
		LOG_INFO("Send Web Request Succeed Data:\r\n%s", m_szWebStreamsJson.c_str());
	}

	return emResult;
}

E_RetCode CKeyMainWnd::Active_Step1()
{
	Sleep(100);
	E_RetCode emResult = m_ptrHostSshUdExec->DownloadFile(CKeyConfig::Instance().GetConfig(Key_szRegFilename),
												CKeyConfig::Instance().GetConfig(Key_szHostDownloadDir),
												CAppParam::Instance().GetExecDir(), m_spNextStepCallback);

	return emResult;
}

E_RetCode CKeyMainWnd::Active_Step2()
{
	Sleep(100);
	E_RetCode emResult = m_ptrKeySshUdExec->UploadFile(CKeyConfig::Instance().GetConfig(Key_szRegFilename), CKeyConfig::Instance().GetConfig(Key_szKeyserverUploadDir),
														CAppParam::Instance().GetExecDir(), m_spNextStepCallback);

	return emResult;
}

E_RetCode CKeyMainWnd::Active_Step3()
{
	E_RetCode emResult = emRC_OK;

	char szMakeNrssetupFileCmd[1024 * 10] = { 0 };
	string szDevModel = CKeyConfig::Instance().GetConfig(Key_szDevModel);
	string szDevSerial = CKeyConfig::Instance().GetConfig(Key_szDevSerial);;
	string szDevVer = CKeyConfig::Instance().GetConfig(Key_szDevVer);;
	sprintf_s(szMakeNrssetupFileCmd, DEVINFO_FILE_FORMAT, szDevModel.c_str(), szDevModel.c_str(), szDevSerial.c_str(), szDevVer.c_str());
	string szCmd = "echo \"";
	szCmd += szMakeNrssetupFileCmd;
	szCmd += "\" > ";
	szCmd += DEVINFO_FILENAME;
	szCmd += " && ";
	szCmd += Name_szLicenseCmd;
	emResult = m_ptrKeySshUdExec->ExecCmd(szCmd, Name_szCmdExecDir, m_spNextStepCallback);

	return emResult;
}

E_RetCode CKeyMainWnd::Active_Step4()
{
	E_RetCode emResult = m_ptrKeySshUdExec->DownloadFile(CKeyConfig::Instance().GetConfig(Key_szKeyFilename), CKeyConfig::Instance().GetConfig(Key_szKeyserverDownloadDir),
														CAppParam::Instance().GetExecDir(), m_spNextStepCallback);

	return emResult;
}

E_RetCode CKeyMainWnd::Active_Step5()
{
	E_RetCode emResult = m_ptrHostSshUdExec->UploadFile(CKeyConfig::Instance().GetConfig(Key_szKeyFilename),
											CKeyConfig::Instance().GetConfig(Key_szHostUpload),
											CAppParam::Instance().GetExecDir(), m_spNextStepCallback);

	return emResult;
}

E_RetCode CKeyMainWnd::Active_Step6()
{
	Sleep(100);
	E_RetCode emResult = m_ptrHostSshUdExec->UploadFile(Name_szVideoStrategyFile,
		CKeyConfig::Instance().GetConfig(Key_szHostUpload),
		CAppParam::Instance().GetExecDir(), m_spNextStepCallback);

	return emResult;
}

E_RetCode CKeyMainWnd::Active_Step7()
{
	Sleep(100);
	E_RetCode emResult = m_ptrHostSshUdExec->UploadFile(Name_szSoftDetectionFile,
		CKeyConfig::Instance().GetConfig(Key_szHostUpload),
		CAppParam::Instance().GetExecDir(), m_spNextStepCallback);

	return emResult;
}

E_RetCode CKeyMainWnd::Active_Step8()
{
	Sleep(100);
	E_RetCode emResult = m_ptrHostSshUdExec->UploadFile(SYSCONFIG_JSON_FILENAME,
			CKeyConfig::Instance().GetConfig(Key_szHostUpload),
			CAppParam::Instance().GetExecDir(), m_spNextStepCallback);

	return emResult;
}

E_RetCode CKeyMainWnd::Active_Step9()
{
	Sleep(100);
	E_RetCode emResult = m_ptrKeySshUdExec->DownloadFile(CKeyConfig::Instance().GetConfig(Key_szCliRegFilename), CKeyConfig::Instance().GetConfig(Key_szKeyserverDownloadDir),
														CAppParam::Instance().GetExecDir(), m_spNextStepCallback);

	return emResult;
}

E_RetCode CKeyMainWnd::Active_Step10()
{
	Sleep(100);
	E_RetCode emResult = m_ptrKeySshUdExec->DownloadFile(CKeyConfig::Instance().GetConfig(Key_szPubKeyFilename), CKeyConfig::Instance().GetConfig(Key_szKeyserverDownloadDir),
														CAppParam::Instance().GetExecDir(), m_spNextStepCallback);

	return emResult;
}

E_RetCode CKeyMainWnd::Active_Step11()
{
	Sleep(100);
	E_RetCode emResult = m_ptrHostSshUdExec->UploadFile(CKeyConfig::Instance().GetConfig(Key_szCliRegFilename), CKeyConfig::Instance().GetConfig(Key_szHostUpload),
														CAppParam::Instance().GetExecDir(), m_spNextStepCallback);

	return emResult;
}

E_RetCode CKeyMainWnd::Active_Step12()
{
	Sleep(100);
	E_RetCode emResult = m_ptrHostSshUdExec->UploadFile(CKeyConfig::Instance().GetConfig(Key_szPubKeyFilename), CKeyConfig::Instance().GetConfig(Key_szHostUpload),
														CAppParam::Instance().GetExecDir(), m_spNextStepCallback);

	return emResult;
}

E_RetCode CKeyMainWnd::Active_Step13()
{
	Sleep(100);
	char szCliRegBuf[1024 * 10] = { 0 };
	string szCliReg = CKeyConfig::Instance().GetConfig(Key_szCliRegFilename);
	sprintf_s(szCliRegBuf, "%s", szCliReg.c_str());
	string szCmd = "chmod 777 ";
	szCmd += szCliRegBuf;
	szCmd += " && ./";
	szCmd += szCliRegBuf;
	E_RetCode emResult = m_ptrHostSshUdExec->ExecCmd(szCmd, CKeyConfig::Instance().GetConfig(Key_szHostUpload), m_spNextStepCallback);

	return emResult;
}

E_RetCode CKeyMainWnd::Active_Step14()
{
	Sleep(100);
	char szCmdBuf[1024 * 10] = { 0 };
	string szCliReg = CKeyConfig::Instance().GetConfig(Key_szCliRegFilename);
	string szPubKey = CKeyConfig::Instance().GetConfig(Key_szPubKeyFilename);
	string szReg= CKeyConfig::Instance().GetConfig(Key_szRegFilename);
	sprintf_s(szCmdBuf, "rm -f %s %s %s", szCliReg.c_str(), szPubKey.c_str(), szReg.c_str());
	E_RetCode emResult = m_ptrHostSshUdExec->ExecCmd(szCmdBuf, CKeyConfig::Instance().GetConfig(Key_szHostUpload), m_spNextStepCallback);

	return emResult;
}

E_RetCode CKeyMainWnd::RebootRemoteHost()
{
	E_RetCode emResult = emRC_OK;

	if (m_ptrHostSshUdExec)
	{
		string szCmd;// = "date >> ~/date.txt && ";
		szCmd += Name_szRebootCmd;
		string szCmdDir = "/sbin/";
		emResult = m_ptrHostSshUdExec->ExecCmd(szCmd, szCmdDir, m_spSshExecRebootCallback);
		if (emResult == emRC_OK)
		{

		}
	}
	else
	{
		OnSshExecRebootCallback(emRC_PARAM_ERR, nullptr, 0, this);
	}

	return emResult;
}

void CKeyMainWnd::OnNextStepCallback(int nResult, const char *pData, int nDataLen, void *pExtendData)
{
	if (nResult == emRC_OK)
	{
		SetCurActiveStep(SETP_NEXT);
		this->PostMessage(WM_U_EXEC_NEXT_STEP);
	}
	if (nResult != emRC_OK)
	{
		OnReportError((int)nResult);
	}
}

void CKeyMainWnd::OnSshExecRebootCallback(int nResult, const char *pData, int nDataLen, void *pExtendData)
{
	if (nResult == emRC_OK)
	{
		// 提示成功启动重启
		m_pActivedOkBtn->SetVisible(false);
		m_pRebootTipLabel->SetVisible(true);
		Sleep(1000);
	}
	if (nResult != emRC_OK)
	{
		CAtlString wszMsg(L"");
		wszMsg.Format(L"重启激活主机失败.");
		MessageBoxW(m_hWnd, wszMsg, L"错误", MB_ICONERROR);
	}

	m_pActivePage->SetVisible(true);
	m_pActivingPage->SetVisible(false);
	m_pActivedPage->SetVisible(false);
}