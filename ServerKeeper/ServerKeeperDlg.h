// ServerKeeperDlg.h : 헤더 파일
//

#pragma once
#include "afxcmn.h"
#include "ReportCtrl.h"
#include "MServerKeeper.h"


// CServerKeeperDlg 대화 상자
class CServerKeeperDlg : public CDialog
{
// 생성
public:
	CServerKeeperDlg(CWnd* pParent = NULL);	// 표준 생성자

// 대화 상자 데이터
	enum { IDD = IDD_SERVERKEEPER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원


// 구현
protected:
	HICON			m_hIcon;
	UINT_PTR		m_nTimerIDCheckProcess;

	MServerKeeper	m_ServerKeeper;

	CImageList		m_ImageList;
	CReportCtrl		m_ReportCtrl;

protected:
	// 메시지 맵 함수를 생성했습니다.
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

protected:
	void UpdateServerListUI();
	void AddItemUI(MServerItem* pItem);
	void UpdateItemUI(MServerItem* pItem);
	void UpdateItem(MServerItem* pItem);
	void CheckServerProcessStatus();

public:
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnBnClickedBtnStop();
	afx_msg LRESULT OnCheckBox(WPARAM wParam, LPARAM lParam);
};
