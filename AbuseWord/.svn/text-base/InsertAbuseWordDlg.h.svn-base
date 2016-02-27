#pragma once
#include "afxwin.h"


// CInsertAbuseWordDlg 대화 상자입니다.

class CInsertAbuseWordDlg : public CDialog
{
	DECLARE_DYNAMIC(CInsertAbuseWordDlg)

public:
	CInsertAbuseWordDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CInsertAbuseWordDlg();

	void Reset();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_INSERT_ABUSE_WORD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
private:
	CEdit m_edtName;
public:
	afx_msg void OnBnClickedInsert();
	afx_msg void OnEnSetfocusEdit();
};
