#pragma once
#include "afxwin.h"


// CSearchAbuseWord 대화 상자입니다.

class CSearchAbuseWord : public CDialog
{
	DECLARE_DYNAMIC(CSearchAbuseWord)

public:
	CSearchAbuseWord(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CSearchAbuseWord();

	void Reset();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_SEARCH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
private:
	CEdit	m_edtName;
	int		m_nLastSearchItemID;

public:
	afx_msg void OnBnClickedSearch();
	afx_msg void OnEnSetfocusEdit();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};
