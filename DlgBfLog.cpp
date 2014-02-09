// DlgBfLog.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "xTrader.h"
#include "DlgBfLog.h"


//extern vector<CThostFtdcRspTransferField*> BfTransVec;

IMPLEMENT_DYNAMIC(DlgBfLog, CDialog)

DlgBfLog::DlgBfLog(CWnd* pParent /*=NULL*/)
	: CDialog(DlgBfLog::IDD, pParent)
{

}

DlgBfLog::~DlgBfLog()
{
}

void DlgBfLog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LST_BFDETAILS, m_LstLog);
}


BEGIN_MESSAGE_MAP(DlgBfLog, CDialog)
	//ON_BN_CLICKED(IDOK, OnOK)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


BOOL DlgBfLog::OnInitDialog()
{

	InitList();
	return TRUE;
}

void DlgBfLog::InitList()
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();

	m_LstLog.SubclassDlgItem(IDC_LST_BFDETAILS,this);

	TCHAR* lpHdrs[6] = {_T("��ˮ��"),_T("�����˺�"),_T("��������"),_T("�������"),_T("����ʱ��"),_T("��Ϣ")};
	int iWidths[6] = {46,140,100,80,60,180};
	UINT i;
	int total_cx = 0;
	LVCOLUMN lvcolumn;
	memset(&lvcolumn, 0, sizeof(lvcolumn));
	
	for (i = 0;i<6 ; i++)
	{
		lvcolumn.mask     = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH|LVCFMT_IMAGE;
		lvcolumn.fmt      = LVCFMT_CENTER;
		lvcolumn.iSubItem = i;
		lvcolumn.pszText  = lpHdrs[i];
		lvcolumn.cx       = iWidths[i];
		
		total_cx += lvcolumn.cx;
		m_LstLog.InsertColumn(i, &lvcolumn);
	}

	UINT uSize = pApp->m_cT->m_BfTransVec.size();

	UINT iSubItem = 0;
	CString szItems[6];
	for (i=0;i<uSize;i++)
	{
		//ansi2uni(CP_ACP,BfTransVec[i]->BankSerial,szItems[0].GetBuffer(MAX_PATH));
		//szItems[0].ReleaseBuffer();
		szItems[0].Format(_T("%d"),pApp->m_cT->m_BfTransVec[i]->FutureSerial);

		ansi2uni(CP_ACP,pApp->m_cT->m_BfTransVec[i]->BankAccount,szItems[1].GetBuffer(MAX_PATH));
		szItems[1].ReleaseBuffer();

		szItems[2]=JgBfTdType(pApp->m_cT->m_BfTransVec[i]->TradeCode);
		szItems[3].Format(_T("%.2f"),pApp->m_cT->m_BfTransVec[i]->TradeAmount);
		szItems[4] = TransTime(pApp->m_cT->m_BfTransVec[i]->TradeTime);

		ansi2uni(CP_ACP,pApp->m_cT->m_BfTransVec[i]->ErrorMsg,szItems[5].GetBuffer(3*MAX_PATH));
		szItems[5].ReleaseBuffer();
		

		m_LstLog.SetRedraw(FALSE);
		for (iSubItem=0;iSubItem<6;iSubItem++)
		{
			if (iSubItem==0) { m_LstLog.InsertItem(i,NULL);}

			m_LstLog.SetItemText(i,iSubItem,szItems[iSubItem]);
		}
		m_LstLog.SetRedraw(TRUE);
	}

}

void DlgBfLog::OnOK()
{
	
	CDialog::OnOK();
	DestroyWindow();
}


void DlgBfLog::OnDestroy()
{
	CDialog::OnDestroy();

	delete this;
}

void DlgBfLog::OnCancel()
{
	CDialog::OnCancel();
	DestroyWindow();
}
