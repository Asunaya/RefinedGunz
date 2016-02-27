#pragma once

#include "listctrl.h"
#include "afxcmn.h"

/**		연결된 클라이언트들의 ip/상태/다운속도/업속도/받은양/보낸양 을 표시한다.
*
*		@author soyokaze
*		@date   2005-11-02
*/

class ConnectionInfo : public CDialog
{
	DECLARE_DYNAMIC(ConnectionInfo)

public:
	ConnectionInfo(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~ConnectionInfo();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_CONNECTION_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	
	ListCtrl m_cClientList;

protected:
	virtual void OnCancel();

public:
	afx_msg void OnClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
