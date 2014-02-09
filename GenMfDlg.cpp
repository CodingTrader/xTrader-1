// GenMfDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "xTrader.h"
#include "GenMfDlg.h"

extern HANDLE g_hEvent;
// GenMfDlg �Ի���

IMPLEMENT_DYNAMIC(GenMfDlg, CDialog)

GenMfDlg::GenMfDlg(CWnd* pParent /*=NULL*/)
	: CDialog(GenMfDlg::IDD, pParent)
{
	m_szGenstat =_T("");
	m_pQryFee = NULL;
	m_pQryMr = NULL;
}

GenMfDlg::~GenMfDlg()
{
}

void GenMfDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_GENSTAT, m_szGenstat);
}


BEGIN_MESSAGE_MAP(GenMfDlg, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BT_GENMR, OnClkBtGenmr)
	ON_BN_CLICKED(IDC_BT_GENFEE, OnClkBtGenfee)
END_MESSAGE_MAP()

BOOL GenMfDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	return TRUE;  
}

void GenMfDlg::OnDestroy()
{
	CDialog::OnDestroy();
	
	if (m_pQryMr)
	{
		TerminateThread(m_pQryMr->m_hThread,0);
		m_pQryMr = NULL;
	}

	if (m_pQryFee)
	{
		TerminateThread(m_pQryFee->m_hThread,0);
		m_pQryFee = NULL;
	}
	delete this;
}

UINT GenMfDlg::QryMrThread(LPVOID pParam)
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	GenMfDlg* pDlg = (GenMfDlg*) pParam;

	pApp->m_cT->m_MargRateVec.clear();
	DWORD dwRet;
	PMGRATE pMgr = new MGRATE;

	xml_document doc;
	xml_node proot;
	xml_node nodeMgrs;
	xml_parse_result result;
	
	char outMgrXml[MAX_PATH];
	sprintf(outMgrXml,MGR_XML,pApp->m_sINVESTOR_ID);

	TCHAR szFile[MAX_PATH];
	ansi2uni(CP_ACP,outMgrXml,szFile);
	if (!PathFileExists(szFile))
	{
		CFile fLog(szFile, CFile::modeReadWrite | CFile::modeCreate | CFile::typeText);
		
		LPCSTR szHdr ="<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
		BYTE bBom[3]={0xEF,0xBB,0xBF};
		fLog.Write(&bBom,3);

		fLog.Write(szHdr, strlen(szHdr));
		fLog.Close();
	}
	/////////////////////////////////////////////
	result = doc.load_file(outMgrXml,parse_full);
	if (result.status!=status_ok) {return FALSE;}
	doc.remove_child("Mgrs");
	proot = doc.append_child("Mgrs");

	for (UINT i=0; i<pApp->m_cT->m_InsinfVec.size();i++)
	{	
		pApp->m_cT->ReqQryInstMgr(pApp->m_cT->m_InsinfVec[i]->iinf.ExchangeInstID);
		dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
		if (dwRet==WAIT_OBJECT_0)
		{
			Sleep(100);
			memcpy(pMgr,pApp->m_cT->m_MargRateVec[i],sizeof(MGRATE));
			//////////////////////////////////////////////////////////
			if (i==0)
			{
				char szInvRg[6];
				sprintf(szInvRg,"%c",pMgr->InvestorRange);
				proot.append_attribute("InvRange") = szInvRg;
				proot.append_attribute("BkrID") = pMgr->BrokerID;
				proot.append_attribute("InvID") = pMgr->InvestorID;
				sprintf(szInvRg,"%c",pMgr->HedgeFlag);
				proot.append_attribute("Flag") = szInvRg;
			}
			nodeMgrs = proot.append_child("Mgr");
			
			nodeMgrs.append_attribute("InstID") = pMgr->InstrumentID;
			nodeMgrs.append_attribute("LongByM") = pMgr->LongMarginRatioByMoney;
			nodeMgrs.append_attribute("LongByV") = pMgr->LongMarginRatioByVolume;
			nodeMgrs.append_attribute("ShortByM") = pMgr->ShortMarginRatioByMoney;
			nodeMgrs.append_attribute("ShortByV") = pMgr->ShortMarginRatioByVolume;
			doc.save_file(outMgrXml,PUGIXML_TEXT("\t"),format_default,encoding_utf8);
			
			/////////////////////////////////////////////////////////

			pDlg->m_szGenstat.Format(_T("��ѯ��%d����֤���ʳɹ�,%d�ֽ�"),i,sizeof(MGRATE));
			pDlg->UpdateData(FALSE);
			ResetEvent(g_hEvent);
		}
		else
		{
			pDlg->m_szGenstat = _T("��ѯ��֤����ʧ��!");
			pDlg->UpdateData(FALSE);
			break;
		}
		Sleep(1000);
	}

	pDlg->m_szGenstat = _T("�Ѿ�������в���!");
	pDlg->UpdateData(FALSE);

	/////////////////////////////////////////////
	pDlg->m_pQryMr = NULL;
	delete pMgr;
	return 0;
}

UINT GenMfDlg::QryFeeThread(LPVOID pParam)
{
	//FeeRateList.clear();
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	GenMfDlg* pDlg = (GenMfDlg*) pParam;
	//CMemFile m_mf;

	xml_document doc;
	xml_node proot;
	xml_node nodeFees;
	xml_parse_result result;

	PFEERATE  pFee= new FEERATE;
	DWORD dwRet;

	char outFeeXml[MAX_PATH];
	sprintf(outFeeXml,FEE_XML,pApp->m_sINVESTOR_ID);
	/////////////////////////////////////////////////////////////////
	CFile fLog;
	TCHAR szFile[MAX_PATH];
	ansi2uni(CP_ACP,outFeeXml,szFile);

	if (!PathFileExists(szFile))
	{
		CFile fLog(szFile, CFile::modeReadWrite | CFile::modeCreate | CFile::typeText);
		
		LPCSTR szHdr ="<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
		BYTE bBom[3]={0xEF,0xBB,0xBF};
		fLog.Write(&bBom,3);
		fLog.Write(szHdr, strlen(szHdr));
		fLog.Close();
	}
	//////////////////////////////////////////////////////
	result = doc.load_file(outFeeXml,parse_full);
	if (result.status!=status_ok) {return FALSE;}
	doc.remove_child("Fees");
	proot = doc.append_child("Fees");

	TThostFtdcInstrumentIDType pIDOld;
	strcpy(pIDOld,"UNKN");

	for (UINT i=0; i<pApp->m_cT->m_InsinfVec.size();i++)
	{
		if (strcmp(pIDOld,pApp->m_cT->m_InsinfVec[i]->iinf.ProductID)==0) { continue; }

		if (i>0) Sleep(1000);

		pApp->m_cT->ReqQryInstFee(pApp->m_cT->m_InsinfVec[i]->iinf.InstrumentID);
		dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
		if (dwRet==WAIT_OBJECT_0)
		{
			Sleep(100);
			if (strcmp(pApp->m_cT->m_FeeRateRev.InstrumentID,""))
			{
				memcpy(pFee,&pApp->m_cT->m_FeeRateRev,sizeof(FEERATE));
			}
			
			else
			{
				pDlg->m_szGenstat = _T("CTP���ݹ���,�޷���ɲ���!");
				pDlg->UpdateData(FALSE);
				pDlg->m_pQryFee = NULL;
				delete pFee;
				return 0;
			}
			strcpy(pIDOld,pFee->InstrumentID);
			//////////////////////////////////////////////////////////
			if (i==0)
			{
				char szInvRg[6];
				sprintf(szInvRg,"%c",pFee->InvestorRange);
				proot.append_attribute("InvRange") = szInvRg;
				proot.append_attribute("BkrID") = pFee->BrokerID;
				proot.append_attribute("InvID") = pFee->InvestorID;
			}
			nodeFees = proot.append_child("Fee");
			
			nodeFees.append_attribute("PID") = pFee->InstrumentID;
			nodeFees.append_attribute("OpenByM") = pFee->OpenRatioByMoney;
			nodeFees.append_attribute("OpenByV") = pFee->OpenRatioByVolume;
			nodeFees.append_attribute("CloseByM") = pFee->CloseRatioByMoney;
			nodeFees.append_attribute("CloseByV") = pFee->CloseRatioByVolume;
			nodeFees.append_attribute("ClosetByM") = pFee->CloseTodayRatioByMoney;
			nodeFees.append_attribute("ClosetByV") = pFee->CloseTodayRatioByVolume;
			doc.save_file(outFeeXml,PUGIXML_TEXT("\t"),format_default,encoding_utf8);

			/////////////////////////////////////////////////////////
			//m_mf.Write(pFee,sizeof(FEERATE));

			pDlg->m_szGenstat.Format(_T("��ѯ��%d���������ʳɹ�,%d�ֽ�"),i,sizeof(FEERATE));
			pDlg->UpdateData(FALSE);
			ResetEvent(g_hEvent);
		}
		else
		{
			pDlg->m_szGenstat = _T("��ѯ��������ʧ��!");
			pDlg->UpdateData(FALSE);
			break;
		}

	}

	pDlg->m_szGenstat = _T("�Ѿ�������в���!");
	pDlg->UpdateData(FALSE);
	///////////////////////////////////////////////////////////////
	pDlg->m_pQryFee = NULL;
	delete pFee;
	return 0;
}

void GenMfDlg::OnClkBtGenmr()
{


	if (!m_pQryMr)
	{
		AfxBeginThread((AFX_THREADPROC)GenMfDlg::QryMrThread, this);
		m_pQryMr=NULL;
	}
	

}


void GenMfDlg::OnClkBtGenfee()
{
	if (!m_pQryFee)
	{
		AfxBeginThread((AFX_THREADPROC)GenMfDlg::QryFeeThread, this);
		m_pQryFee=NULL;
	}

}

void GenMfDlg::OnOK()
{
	CDialog::OnOK();
	DestroyWindow();
}

void GenMfDlg::OnCancel()
{
	CDialog::OnCancel();
	DestroyWindow();
}