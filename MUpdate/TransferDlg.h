#pragma once


#include "MUpdateMgr.h"


// CTransferDlg dialog
class CTransferDlg : public CDialog
{
	DECLARE_DYNAMIC(CTransferDlg)

protected:
	MUpdateMgr		m_UpdateMgr;

public:
	CTransferDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTransferDlg();

	void SetInfoText(const char* pszText);
	void SetFileProgress(int nPercent);

	void SetTotalInfoText(const char* pszText);
	void SetTotalProgress(int nPercent);

	void StartUpdate();
	

protected:
	// Dialog Data
	enum { IDD = IDD_DLG_TRANSFER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedBtnTest();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg long OnTransferInit(WPARAM wParam, LPARAM lParam);
};
