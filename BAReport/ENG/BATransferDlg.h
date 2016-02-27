#if !defined(AFX_BATRANSFERDLG_H__D34A5EC5_FADB_4C84_899B_91CD416FD7BD__INCLUDED_)
#define AFX_BATRANSFERDLG_H__D34A5EC5_FADB_4C84_899B_91CD416FD7BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BATransferDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// BATransferDlg dialog

class BATransferDlg : public CDialog
{
// Construction
public:
	BATransferDlg(CWnd* pParent = NULL);   // standard constructor
	static void TransferCallback(int nReason, DWORD dwSentSize, DWORD dwLocalSize);

// Dialog Data
	//{{AFX_DATA(BATransferDlg)
	enum { IDD = IDD_DLG_TRANSFER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BATransferDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(BATransferDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BATRANSFERDLG_H__D34A5EC5_FADB_4C84_899B_91CD416FD7BD__INCLUDED_)
