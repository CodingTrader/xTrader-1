#pragma once
#include "XListCtrl/XListCtrl.h"


// DlgBfLog �Ի���

class DlgBfLog : public CDialog
{
	DECLARE_DYNAMIC(DlgBfLog)

public:
	DlgBfLog(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~DlgBfLog();

// �Ի�������
	enum { IDD = IDD_DLG_BFDETAILS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_LstLog;
	void InitList();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnDestroy();
};
