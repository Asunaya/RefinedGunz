// LocatorTesterDlg.h : 헤더 파일
//

#pragma once


#include "Tester.h"
#include "afxwin.h"


// CLocatorTesterDlg 대화 상자
class CLocatorTesterDlg : public CDialog
{
// 생성
public:
	CLocatorTesterDlg(CWnd* pParent = NULL);	// 표준 생성자

private :
	Tester m_Tester;

// 대화 상자 데이터
	enum { IDD = IDD_LOCATORTESTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원


// 구현
protected:
	HICON m_hIcon;

	// 메시지 맵 함수를 생성했습니다.
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_Elapsed;
	afx_msg void OnBnClickedOk();
	CEdit m_Out;
};
