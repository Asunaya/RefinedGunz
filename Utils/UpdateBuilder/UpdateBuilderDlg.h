// UpdateBuilderDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CUpdateBuilderDlg 대화 상자
class CUpdateBuilderDlg : public CDialog
{
// 생성
public:
	CUpdateBuilderDlg(CWnd* pParent = NULL);	// 표준 생성자

// 대화 상자 데이터
	enum { IDD = IDD_UPDATEBUILDER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원

// 구현
protected:
	HICON m_hIcon;

	// 메시지 맵 함수를 생성했습니다.
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	// 화면에 로그를 남기기 위한
	CRichEditCtrl m_EditLog;
	void Log(const char *pFormat,...);
	void ClearLog();

	// 진짜 일하는곳
	BOOL Build();

public:
	CString m_strOld;
	CString m_strNew;
	CString m_strOutput;

	afx_msg void OnBnClickedButtonBrowseOld();
	afx_msg void OnBnClickedButtonBrowseNew();
	afx_msg void OnBnClickedButtonBrowseOutput();

	afx_msg void OnBnClickedBuild();
	afx_msg void OnEnChangeEditOld();
	afx_msg void OnEnChangeEditNew();
	afx_msg void OnEnChangeEditOutput();
};
