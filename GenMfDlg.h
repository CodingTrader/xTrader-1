#pragma once


// GenMfDlg �Ի���

class GenMfDlg : public CDialog
{
	DECLARE_DYNAMIC(GenMfDlg)

public:
	GenMfDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~GenMfDlg();

	static UINT QryMrThread(LPVOID pParam);
	static UINT QryFeeThread(LPVOID pParam);

// �Ի�������
	enum { IDD = IDD_DLG_GENMRFEE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClkBtGenmr();
	afx_msg void OnClkBtGenfee();
	afx_msg void OnDestroy();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	CString m_szGenstat;
	CWinThread *m_pQryMr,*m_pQryFee;
};
