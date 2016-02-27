// BirdAgentDlg.h : 헤더 파일
//

#pragma once

#include "TrayIcon.h"

#define	WM_ICON_NOTIFY      WM_USER+10       // 사용자 정의 메세지

// CBirdAgentDlg 대화 상자
class CBirdAgentDlg : public CDialog
{
// 생성
public:
	CBirdAgentDlg(CWnd* pParent = NULL);	// 표준 생성자
	virtual ~CBirdAgentDlg();

	CTrayIcon m_TrayIcon;

// 대화 상자 데이터
	enum { IDD = IDD_BIRDAGENT_DIALOG };

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
	afx_msg LONG OnTrayNotification(UINT wParam, LONG lParam);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton3();
	afx_msg void OnMenuQuit();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnMenuShow();
	afx_msg void OnMenuHide();
};
