#pragma once
#include "afxwin.h"


// BASingleDlg 대화 상자입니다.

class BASingleDlg : public CDialog
{
	DECLARE_DYNAMIC(BASingleDlg)

public:
	BASingleDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~BASingleDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DLG_SINGLE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
public:
	CString *m_pOut;

	virtual BOOL OnInitDialog();
protected:
	virtual void OnCancel();
public:
	CEdit m_Name;
	CEdit m_Symptom;
};
