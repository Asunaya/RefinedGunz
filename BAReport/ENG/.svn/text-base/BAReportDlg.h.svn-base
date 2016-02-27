// BAReportDlg.h : header file
//

#if !defined(AFX_BAREPORTDLG_H__F8BD4329_AA33_4F3C_B8ED_56CA72C25963__INCLUDED_)
#define AFX_BAREPORTDLG_H__F8BD4329_AA33_4F3C_B8ED_56CA72C25963__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "FileTransfer.h"
#include "BAAgreementDlg.h"
#include "BATransferDlg.h"
#include "BASingleDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CBAReportDlg dialog
class CBAReportDlg : public CDialog
{
public:

	BASingleDlg			m_SingleDlg;
	BAAgreementDlg		m_AgreementDlg;
	BATransferDlg		m_TransferDlg;

// Construction
public:
	CBAReportDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CBAReportDlg)
	enum { IDD = IDD_BAREPORT_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBAReportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CBAReportDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	bool	m_bProgress;		// 프로그레스바
	bool	m_bSingle;			// 싱글모드

public:
	void SetProgress(bool bProgress) { m_bProgress = bProgress; }
	void SetSingle(bool bSingle) { m_bSingle = bSingle; }

	void Upload();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};



void SetCommand(LPTSTR pszCmd);
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BAREPORTDLG_H__F8BD4329_AA33_4F3C_B8ED_56CA72C25963__INCLUDED_)
