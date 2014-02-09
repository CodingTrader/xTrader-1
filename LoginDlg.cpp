// LoginDlg.cpp : implementation file
//

#include "stdafx.h"
#include "xTrader.h"
#include "LoginDlg.h"
#include "NoticeDlg.h"
#include "DlgNetSel.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern HANDLE g_hEvent;
static UINT LoginThread(LPVOID pParam);

/////////////////////////////////////////////////////////////////////////////
// LoginDlg dialog


LoginDlg::LoginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(LoginDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//((CXTraderApp*)AfxGetApp())->m_cT->m_pDlg = this;

	
	m_szUser = _T("888888");
	m_szPass = _T("hua123456");
	m_bSave = TRUE;
	m_pLogin = NULL;
//	g_bOnce = TRUE;
	m_szArTs.RemoveAll();
	m_szArMd.RemoveAll();
	m_szBrkName = _T("");

}

LoginDlg::~LoginDlg()
{
	SaveConfig();
}

void LoginDlg::LoadConfig()
{
	CXTraderApp* xTApp = (CXTraderApp*)AfxGetApp();

	xml_document doc;
	xml_node proot;
	//char szBrkNm[32];
	xml_parse_result result = doc.load_file(CFG_FILE);
	if (result.status == status_ok)
	{
		proot = doc.child(ROOT);
		if (!proot) return;
		
		m_iSel1 = atoi(proot.child_value(BKR_GRP));
		m_iSel2 = atoi(proot.child_value(SV_GRP));
		m_bSave = atoi(proot.child_value(SV_INF));
		
		char szEncPass[60];
		strcpy(xTApp->m_sINVESTOR_ID, proot.child_value(USER_ID));
		strcpy(szEncPass,proot.child_value(USER_PW));
		strcpy(xTApp->m_pInstList ,proot.child_value(INS_LST));
		
		Base64Decode(xTApp->m_sPASSWORD, (const char*)szEncPass, 0);
		ansi2uni(CP_UTF8,xTApp->m_sINVESTOR_ID,m_szUser.GetBuffer(MAX_PATH));
		m_szUser.ReleaseBuffer();
		ansi2uni(CP_UTF8,xTApp->m_sPASSWORD,m_szPass.GetBuffer(MAX_PATH));
		m_szPass.ReleaseBuffer();

		xml_node nodeWInf = proot.child(WND_INF);
		//strcpy(pFeeRate->InstrumentID, nodeFees.attribute("PID").value());
		xTApp->m_ixPos = nodeWInf.attribute("xPos").as_int();
		xTApp->m_iyPos = nodeWInf.attribute("yPos").as_int();
		xTApp->m_iWidth = nodeWInf.attribute("width").as_int();
		xTApp->m_iHeight = nodeWInf.attribute("height").as_int();
		
	}
	else
	{
		ProgressUpdate(_T("config.xml����ʧ��!"), 0);
		
	}	
	/////////////////////////////////////////
	int i=0,nIndex = 0;
	for (i=0;i<xTApp->m_BkrParaVec.size();i++)
	{
		nIndex = m_ComboBkr.AddString(xTApp->m_BkrParaVec[i]->BkrName);
		m_ComboBkr.SetItemData(nIndex,i);
	}
	
		m_ComboBkr.SetCurSel(m_iSel1);
	/////////////////////////////////////////////////////////////	
	result = doc.load_file(xTApp->m_BkrParaVec[m_iSel1]->XmlPath);
	if (result.status == status_ok) 
	{
		proot = doc.child(ROOT).child("broker");
		if (!proot) return;
		
		strcpy(xTApp->m_sBROKER_ID,proot.attribute("BrokerID").value());
		
		m_szBrkName = xTApp->m_BkrParaVec[m_iSel1]->BkrName;
		
		///////////����������//////////////////
		LPCSTR szSvrRt="//broker/Servers/Server",sNmae="Name",sTrading="Trading",sMData="MarketData";
		char strName[64];
		CString tName,tTrading,tMData;
		CStringArray szAr;
		szAr.RemoveAll();
		
		xpath_node_set sVrs = doc.select_nodes(szSvrRt);
		if (sVrs.empty()) return;
		for (xpath_node_set::const_iterator it = sVrs.begin(); it !=  sVrs.end(); ++it)
		{
			xpath_node node = *it;
			strcpy(strName,node.node().child_value(sNmae));
			ansi2uni(CP_UTF8,strName,tName.GetBuffer(MAX_PATH));
			tName.ReleaseBuffer();
			//ansi2uni(CP_UTF8,strTrading,tTrading.GetBuffer(MAX_PATH));
			szAr.Add(tName);
			//ShowErroTips(tTrading,MY_TIPS);
			
			if (szAr.GetSize()==(m_iSel2+1))
			{
				xml_node tool;
				
				for (tool = node.node().child(sTrading).first_child(); tool; tool = tool.next_sibling())
				{
					ansi2uni(CP_UTF8,(char*)tool.child_value(),tTrading.GetBuffer(MAX_PATH));
					tTrading.ReleaseBuffer();

					m_szArTs.Add(tTrading);
				}
				
				for (tool = node.node().child(sMData).first_child(); tool; tool = tool.next_sibling())
				{
					ansi2uni(CP_UTF8,(char*)tool.child_value(),tMData.GetBuffer(MAX_PATH));
					tMData.ReleaseBuffer();
			
					m_szArMd.Add(tMData);
				}
			}
		}
		////////////////////////////////////////////////////////////////////////////
		//int i=0,nIndex = 0;
		for (i=0;i<szAr.GetSize();i++)
		{
			nIndex = m_ComboIsp.AddString(szAr[i]);
			m_ComboIsp.SetItemData(nIndex,i);
		}

		m_ComboIsp.SetCurSel(m_iSel2);
		::SendMessage(m_hWnd, WM_COMMAND,MAKEWPARAM(IDC_ISPLIST,CBN_SELCHANGE),(LPARAM)m_ComboIsp.GetSafeHwnd());
	}
	else
	{
		ProgressUpdate(_T("broker���ü���ʧ��!"), 0);
		return;
	}
	
}

void LoginDlg::SaveConfig()
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();

	if (m_bSave)
	{
		UpdateData();
		
		xml_document doc;
		xml_node proot;
		xml_parse_result result = doc.load_file(CFG_FILE,parse_full);
		
		if (result.status==status_ok)
		{
			proot = doc.child(ROOT);
			
			proot.remove_child(SV_INF);
			proot.remove_child(SV_GRP);
			proot.remove_child(BKR_GRP);
			proot.remove_child(USER_ID);
			proot.remove_child(USER_PW);
			//proot.remove_child(NTP_SVR);
			//proot.remove_child(MK_DATA);
			
			char szUid[2*MAX_PATH],szPass[2*MAX_PATH],szSave[4],szGrp1[4],szGrp2[4];
			sprintf(szSave,"%d",m_bSave);
			sprintf(szGrp1,"%d",m_iSel1);
			sprintf(szGrp2,"%d",m_iSel2);
			uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)m_szUser,szUid);
			uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)m_szPass,szPass);
			
			char szEncPass[60];
			Base64Encode(szEncPass, (const char*)szPass, 0); 
			xml_node nodeSi = proot.append_child(SV_INF);
			nodeSi.append_child(node_pcdata).set_value(szSave);
			
			xml_node nodeGrp1 = proot.append_child(BKR_GRP);
			nodeGrp1.append_child(node_pcdata).set_value(szGrp1);
			
			xml_node nodeGrp2 = proot.append_child(SV_GRP);
			nodeGrp2.append_child(node_pcdata).set_value(szGrp2);
			
			xml_node nodeUid = proot.append_child(USER_ID);
			nodeUid.append_child(node_pcdata).set_value(szUid);
			
			xml_node nodeUpw = proot.append_child(USER_PW);
			nodeUpw.append_child(node_pcdata).set_value(szEncPass);
			
			doc.save_file(CFG_FILE,PUGIXML_TEXT("\t"),format_default,encoding_utf8);
		}
	}
}

void LoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(LoginDlg)
	DDX_Control(pDX, IDC_ISPLIST, m_ComboIsp);
	DDX_Control(pDX, IDC_BKR_LIST, m_ComboBkr);
	DDX_Text(pDX, IDC_USERNAME, m_szUser);
	DDV_MaxChars(pDX, m_szUser, 13);
	DDX_Text(pDX, IDC_PASSWORD, m_szPass);
	DDV_MaxChars(pDX, m_szPass, 41);
	DDX_Check(pDX, IDC_SAVE, m_bSave);
	DDX_Control(pDX, IDC_PROGRESS, m_prgs);
	DDX_Control(pDX, IDC_LOGINFO, m_staInfo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(LoginDlg, CDialog)
	//{{AFX_MSG_MAP(LoginDlg)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, OnLogin)
	ON_BN_CLICKED(IDCANCEL, OnQuit)
	ON_BN_CLICKED(IDC_NETSET, OnNetset)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_EN_CHANGE(IDC_USERNAME, OnChangeUser)
	ON_EN_CHANGE(IDC_PASSWORD, OnChangePass)
	ON_CBN_SELCHANGE(IDC_ISPLIST, OnSelISP)
	ON_CBN_SELCHANGE(IDC_BKR_LIST, OnSelBkr)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL LoginDlg::OnInitDialog()
{

	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_prgs.SetRange(0, 100);
	ProgressUpdate(_T("δ��½!"), 0);

	LoadConfig();
	UpdateData(FALSE);

	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// LoginDlg message handlers

void LoginDlg::OnLogin() 
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();

	((CButton*)GetDlgItem(IDC_NETSET))->EnableWindow(FALSE);
	((CButton*)GetDlgItem(IDOK))->EnableWindow(FALSE);
	//g_bOnce = FALSE;
	UpdateData(true);
	if (m_szUser.IsEmpty() || m_szPass.IsEmpty())
	{
		ShowErroTips(_T("�û��������ʺ�Ϊ��!"),MY_TIPS);
	}
	else
	{
		pApp->ReleaseApi();
		pApp->CreateApi();
		pApp->m_szTitle.Format(_T("%s@%s-%s"),m_szUser,m_szBrkName,LoadString(IDS_TITLE));
		
		if (!m_pLogin)
		{
			AfxBeginThread((AFX_THREADPROC)LoginThread, this);
			m_pLogin=NULL;
		}
		//else
		//{
			//ResetEvent(g_hEvent);
		//	TerminateThread(m_pLogin->m_hThread, 0);
		//	m_pLogin= NULL;
		//}

	}
	
}

void LoginDlg::OnQuit() 
{
	PostQuitMessage(0);		
}

void LoginDlg::OnDestroy()
{
	//SaveConfig();
	CDialog::OnDestroy();
}

void LoginDlg::OnNetset() 
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	CString str;

	m_ComboIsp.GetWindowText(str);

	//AfxMessageBox(str);
	DlgNetSel dNetSet;
	dNetSet.m_szGrpName = str;
	dNetSet.m_iIdx = m_iSel1;

	int iRet = dNetSet.DoModal();

	//dNetSet.InitLists();
	if (iRet!=IDOK)
	{
		return;
	}
}

void LoginDlg::OnSave() 
{
	m_bSave = !m_bSave;	
}

void LoginDlg::OnSelISP() 
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	if(m_ComboIsp.GetCurSel() >= 0)
	{
		m_iSel2 = m_ComboIsp.GetCurSel();

		CString str;
		m_ComboIsp.GetWindowText(str);

		pApp->AddSvr2Ar(m_szArTs,m_szArMd,str,m_iSel1);
	}
}

void LoginDlg::OnSelBkr() 
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	if(m_ComboBkr.GetCurSel() >= 0)
	{
		m_iSel1 = m_ComboBkr.GetCurSel();
		
		xml_document doc;
		xml_node proot;
		//CString szBrkID;
		xml_parse_result result = doc.load_file(pApp->m_BkrParaVec[m_iSel1]->XmlPath);
		if (result.status == status_ok) 
		{
			proot = doc.child(ROOT).child("broker");
			if (!proot) return;
			
			strcpy(pApp->m_sBROKER_ID,proot.attribute("BrokerID").value());
			m_szBrkName = pApp->m_BkrParaVec[m_iSel1]->BkrName;
			
			///////////����������//////////////////
			LPCSTR szSvrRt="//broker/Servers/Server",sNmae="Name",sTrading="Trading",sMData="MarketData";
			char strName[64];
			CString tName,tTrading,tMData;
			CStringArray szAr;
			szAr.RemoveAll();
			
			xpath_node_set sVrs = doc.select_nodes(szSvrRt);
			if (sVrs.empty()) return;
			for (xpath_node_set::const_iterator it = sVrs.begin(); it !=  sVrs.end(); ++it)
			{
				xpath_node node = *it;
				strcpy(strName,node.node().child_value(sNmae));
				ansi2uni(CP_UTF8,strName,tName.GetBuffer(MAX_PATH));
				tName.ReleaseBuffer();
				//ansi2uni(CP_UTF8,strTrading,tTrading.GetBuffer(MAX_PATH));
				szAr.Add(tName);
				
			}
			m_ComboIsp.ResetContent();
			////////////////////////////////////////////////////////////////////////////
			int i=0,nIndex = 0;
			for (i=0;i<szAr.GetSize();i++)
			{
				nIndex = m_ComboIsp.AddString(szAr[i]);
				m_ComboIsp.SetItemData(nIndex,i);
			}
			
			m_ComboIsp.SetCurSel(0);

			::SendMessage(m_hWnd, WM_COMMAND,MAKEWPARAM(IDC_ISPLIST,CBN_SELCHANGE),(LPARAM)m_ComboIsp.GetSafeHwnd());
			///////////////////////////////////////////
			
		}
	}
}

void LoginDlg::OnChangeUser() 
{
	CXTraderApp* xTApp = (CXTraderApp*)AfxGetApp();
	
	UpdateData(true);
	uni2ansi(CP_ACP,m_szUser.GetBuffer(MAX_PATH),xTApp->m_sINVESTOR_ID);
	m_szUser.ReleaseBuffer();
}


void LoginDlg::OnChangePass() 
{
	CXTraderApp* xTApp = (CXTraderApp*)AfxGetApp();
	
	UpdateData(true);
	uni2ansi(CP_ACP,m_szPass.GetBuffer(MAX_PATH),xTApp->m_sPASSWORD);
	m_szPass.ReleaseBuffer();
	
}

void LoginDlg::ProgressUpdate(LPCTSTR szMsg, const int nPercentDone)
{
    ASSERT (AfxIsValidString(szMsg));
    ASSERT ( nPercentDone >= 0  &&  nPercentDone <= 100 );
	
    m_staInfo.SetWindowText(szMsg);
    //m_stCustomMsg.SetWindowText ( szCustomMsg );
    m_prgs.SetPos(nPercentDone);
}


UINT LoginThread(LPVOID pParam)
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	LoginDlg* pDlg = (LoginDlg*) pParam;
	//��ʼ������ͽ���API,ע����ǰ�ñ���
	
	pApp->m_cT->ClrAllVecs();

	int iTdSvr = pDlg->m_szArTs.GetSize();
	int i =0,iLen;
	char szTd[MAX_PATH],szMd[MAX_PATH];
	for (i=0;i<iTdSvr;i++)
	{
		iLen = pDlg->m_szArTs[i].GetLength();
		uni2ansi(CP_ACP,pDlg->m_szArTs[i].GetBuffer(iLen),szTd);
		pDlg->m_szArTs[i].ReleaseBuffer();

		pApp->m_TApi->RegisterFront(szTd);
	}
	pApp->m_TApi->Init();	
	DWORD dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��½���׳ɹ�!"), 10);
		ResetEvent(g_hEvent);
	}
	else
	{
		//ResetEvent(g_hEvent);
		::EnableWindow(GetDlgItem(pDlg->GetSafeHwnd(),IDOK),TRUE);
		::EnableWindow(GetDlgItem(pDlg->GetSafeHwnd(),IDC_NETSET),TRUE);
		pDlg->m_pLogin = NULL;
		pDlg->ProgressUpdate(_T("CTPƽ̨������!"),0);

		return 0;
	}
	
	if (pApp->m_cT->IsErrorRspInfo(&pApp->m_cT->m_RspMsg))
	{
		//ResetEvent(g_hEvent);
		::EnableWindow(GetDlgItem(pDlg->GetSafeHwnd(),IDOK),TRUE);
		::EnableWindow(GetDlgItem(pDlg->GetSafeHwnd(),IDC_NETSET),TRUE);

		TCHAR szMsg[MAX_PATH];
		ansi2uni(CP_ACP,pApp->m_cT->m_RspMsg.ErrorMsg,szMsg);
		pDlg->ProgressUpdate(szMsg,0);
		pDlg->m_pLogin = NULL;
	
		return 0;
	}

	int iMdSvr = pDlg->m_szArMd.GetSize();
	for (i=0;i<iTdSvr;i++)
	{
		iLen = pDlg->m_szArMd[i].GetLength();
		uni2ansi(CP_ACP,pDlg->m_szArMd[i].GetBuffer(iLen),szMd);
		pDlg->m_szArMd[i].ReleaseBuffer();

		pApp->m_MApi->RegisterFront(szMd);
	}
	pApp->m_MApi->Init();
	
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��½����ɹ�!"), 20);
		ResetEvent(g_hEvent);
	}

	///////////////////////////////////////////////////////////
	pApp->m_cT->ReqSettlementInfoConfirm();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("ȷ�Ͻ��㵥!"), 40);
		ResetEvent(g_hEvent);
	}
	else
	{
		pDlg->m_pLogin = NULL;
		::EnableWindow(GetDlgItem(pDlg->GetSafeHwnd(),IDOK),TRUE);

		pDlg->ProgressUpdate(_T("ȷ�Ͻ��㳬ʱ!"),0);
		return 0;
	}
	
	pApp->m_cT->ReqQryInst(NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("���Լ�б�!"), 60);
		ResetEvent(g_hEvent);
	}
	else
	{
		pDlg->m_pLogin = NULL;
		::EnableWindow(GetDlgItem(pDlg->GetSafeHwnd(),IDOK),TRUE);

		pDlg->ProgressUpdate(_T("���Լ��ʱ!"),0);
		return 0;
	}


	Sleep(1000);
	pApp->m_cT->ReqQryInvPos(NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ֲ���Ϣ!"), 70);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->m_pLogin = NULL;
		::EnableWindow(GetDlgItem(pDlg->GetSafeHwnd(),IDOK),TRUE);

		pDlg->ProgressUpdate(_T("��ֲ���Ϣ��ʱ!"),0);
		return 0;
	}
	
	Sleep(1000);
	pApp->m_cT->ReqQryTdAcc();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("���ʽ��˻�!"), 80);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->m_pLogin = NULL;
		::EnableWindow(GetDlgItem(pDlg->GetSafeHwnd(),IDOK),TRUE);

		pDlg->ProgressUpdate(_T("���˻���ʱ!"),0);
		return 0;
	}

#ifdef _REAL_CTP_
	Sleep(1000);
	pApp->m_cT->ReqQryAccreg();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��������Ϣ!"), 90);
		ResetEvent(g_hEvent);
		
	}
	else
	{
		pDlg->m_pLogin = NULL;
		::EnableWindow(GetDlgItem(pDlg->GetSafeHwnd(),IDOK),TRUE);

		pDlg->ProgressUpdate(_T("��������Ϣ��ʱ!"),0);
		return 0;
	}
	
	Sleep(1000);
	pApp->m_cT->ReqQryTradingCode();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("�齻�ױ���!"), 99);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->m_pLogin = NULL;
		::EnableWindow(GetDlgItem(pDlg->GetSafeHwnd(),IDOK),TRUE);

		pDlg->ProgressUpdate(_T("�齻�ױ��볬ʱ!"),0);
		return 0;
	}
	
#endif

	pDlg->m_pLogin = NULL;
	pDlg->EndDialog(IDOK);
	return 0;

}
