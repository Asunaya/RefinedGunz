// BugsAnalyzerDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"
#include "CBugsAnalyzer.h"
#include "afxcmn.h"

// CBugsAnalyzerDlg 대화 상자
class CBugsAnalyzerDlg : public CDialog
{
// 생성
public:
	CBugsAnalyzerDlg(CWnd* pParent = NULL);	// 표준 생성자

// 대화 상자 데이터
	enum { IDD = IDD_BUGSANALYZER_DIALOG };

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
	afx_msg void OnBnClickedBtnOpenfolder();
protected:
public:
	CString m_strFolderName;
	afx_msg void OnBnClickedBtnGo();
protected:
	CListBox m_lbOutput;
	CListBox m_lbFileList;
	CBugsAnalyzer	analyzer;
public:
	afx_msg void OnLbnSelchangeOutput();
	afx_msg void OnLbnDblclkFileList();
	CProgressCtrl m_pcProgress;
};
