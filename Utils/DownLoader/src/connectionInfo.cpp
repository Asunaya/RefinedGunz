// connectionInfo.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "connectionInfo.h"
#include ".\connectioninfo.h"

// ConnectionInfo 대화 상자입니다.

IMPLEMENT_DYNAMIC(ConnectionInfo, CDialog)
ConnectionInfo::ConnectionInfo(CWnd* pParent /*=NULL*/)
	: CDialog(ConnectionInfo::IDD, pParent)
{
}

ConnectionInfo::~ConnectionInfo()
{
}

void ConnectionInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CLIENT_LIST, m_cClientList);
}


BEGIN_MESSAGE_MAP(ConnectionInfo, CDialog)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// ConnectionInfo 메시지 처리기입니다.

BOOL ConnectionInfo::OnInitDialog()
{
	CDialog::OnInitDialog();

	
	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

int ConnectionInfo::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetWindowText("Connection Info");
	ShowWindow(SW_HIDE);

	m_cClientList.Create(0, CRect(0, 0, 0, 0), this, 0);

	m_cClientList.AddColumn("Ip", 150, 0);
	m_cClientList.AddColumn("Status", 100, 1);
	m_cClientList.AddColumn("Bps In", 75, 2);
	m_cClientList.AddColumn("Bps Out", 75, 3);
	m_cClientList.AddColumn("Bytes Received", 125, 4);
	m_cClientList.AddColumn("Bytes Sent", 125, 5);

	m_cClientList.SetActiveFlags(Settings::GetDWord("ClientListFlags"));

	return 0;
}

void ConnectionInfo::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	CRect cRect;
	GetClientRect(&cRect);

	m_cClientList.MoveWindow(&cRect);
}

void ConnectionInfo::OnCancel()
{
	ShowWindow(SW_HIDE);
}

void ConnectionInfo::OnClose()
{
	ShowWindow(SW_HIDE);
}

void ConnectionInfo::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	if(bShow)
		m_cClientList.ShowWindow(SW_SHOW);
}