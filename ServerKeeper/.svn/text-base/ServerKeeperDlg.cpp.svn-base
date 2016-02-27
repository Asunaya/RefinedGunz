// ServerKeeperDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "ServerKeeper.h"
#include "ServerKeeperDlg.h"
#include "MProcessController.h"
#include ".\serverkeeperdlg.h"
#include "MSharedCommandTable.h"
#include "Updater.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif


#define FILENAME_SERVERKEEPER	"ServerKeeperConfig.xml"

#define COLOR_ALIVE	RGB(150,150,255)
#define COLOR_DEAD	RGB(255,150,150)

#define TIMERID_CHECK_PROCESS		101
#define TIMER_ELAPSE_CHECK_PROCESS	1000


// CServerKeeperDlg 대화 상자
CServerKeeperDlg::CServerKeeperDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServerKeeperDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nTimerIDCheckProcess = 0;
}

void CServerKeeperDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SERVER, m_ReportCtrl);
}

BEGIN_MESSAGE_MAP(CServerKeeperDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_START, OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_BTN_STOP, OnBnClickedBtnStop)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_MESSAGE(WM_ON_CHKBOX, OnCheckBox)
END_MESSAGE_MAP()


// CServerKeeperDlg 메시지 처리기

BOOL CServerKeeperDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	// 프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	/*
	if( !m_ServerKeeper.Create(7500) )
	{
		mlog( "Fail to m_ServerKeeper.Create\n" );
		return FALSE;
	}
	*/
	
	if( !m_ServerKeeper.LoadFromXml(FILENAME_SERVERKEEPER) )
	{
		mlog( "Fail m_ServerKeeper.LoadFromXml\n" );
		return FALSE;
	}

	if( !m_ServerKeeper.Create() )
	{
		return FALSE;
	}

	m_ImageList.Create(16, 16, ILC_COLORDDB, 0, 4);
	CBitmap bmAlive; bmAlive.LoadBitmap(IDB_ALIVE);
	m_ImageList.Add(&bmAlive, (COLORREF)-1);
	CBitmap bmDead; bmDead.LoadBitmap(IDB_DEAD);		
	m_ImageList.Add(&bmDead, (COLORREF)-1);

	m_ServerKeeper.InitConfigState();
	m_ServerKeeper.InitServerState();

//	m_ReportCtrl.SetColumnHeader(_T("Check,50; Alive,50; Server,50,1; Path, 200 "));
//	m_ReportCtrl.SetImageList(&m_ImageList);
	m_ReportCtrl.SetGridLines(TRUE); // SHow grid lines
	m_ReportCtrl.SetCheckboxeStyle(RC_CHKBOX_NORMAL); // Enable checkboxes

	m_ReportCtrl.InsertColumn(0, _T("Vital"), LVCFMT_LEFT);
	m_ReportCtrl.InsertColumn(1, _T("Status"), LVCFMT_LEFT);
	m_ReportCtrl.InsertColumn(2, _T("ServerType"), LVCFMT_LEFT);
	m_ReportCtrl.InsertColumn(3, _T("Name"), LVCFMT_LEFT);
	m_ReportCtrl.InsertColumn(4, _T("Path"), LVCFMT_LEFT);

	m_ReportCtrl.SetColumnWidth(0, 60);
	m_ReportCtrl.SetColumnWidth(1, 60);
	m_ReportCtrl.SetColumnWidth(2, 80);
	m_ReportCtrl.SetColumnWidth(3, 100);
	m_ReportCtrl.SetColumnWidth(4, LVSCW_AUTOSIZE_USEHEADER);

	UpdateServerListUI();
	CheckServerProcessStatus();
/*
    m_ReportCtrl.InsertItem(0, "");
	m_ReportCtrl.SetItemText(0, 0, "Alive");
	m_ReportCtrl.SetItemBkColor(0,0,RGB(180,180,255));
	m_ReportCtrl.SetItemText(0, 1, "Stop");
	m_ReportCtrl.SetItemText(0, 2, "MatchServer");
	m_ReportCtrl.SetItemText(0, 3, "C:\\GunzServer\\MatchServer");
*/
	m_nTimerIDCheckProcess = SetTimer(TIMERID_CHECK_PROCESS, TIMER_ELAPSE_CHECK_PROCESS, NULL);
	return TRUE;  // 컨트롤에 대한 포커스를 설정하지 않을 경우 TRUE를 반환합니다.
}


void CServerKeeperDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	if (m_nTimerIDCheckProcess)
		KillTimer(m_nTimerIDCheckProcess);
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면 
// 아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
// 프레임워크에서 이 작업을 자동으로 수행합니다.
void CServerKeeperDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

void CServerKeeperDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	CheckServerProcessStatus();

	m_ServerKeeper.Run();
	m_ServerKeeper.SendPingToPatchInterface();
		
	CDialog::OnTimer(nIDEvent);
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다. 
HCURSOR CServerKeeperDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CServerKeeperDlg::UpdateServerListUI()
{
	MServerItemList* pList = m_ServerKeeper.GetServerItemList();
	for (MServerItemList::iterator i = pList->begin(); i!=pList->end(); i++) {
		MServerItem* pItem = *i;
		AddItemUI(pItem);
	}
}

void CServerKeeperDlg::UpdateItemUI(MServerItem* pItem)
{
	int nRow = m_ReportCtrl.GetNextItem(-1, LVNI_ALL);
	while(nRow != -1) {
		int nSlotID = m_ReportCtrl.GetItemData(nRow);
		if (pItem->GetSlotID() == nSlotID) {
			m_ReportCtrl.SetItemText(nRow, 0, "N/A");
			m_ReportCtrl.SetItemText(nRow, 1, pItem->GetStatusToString().c_str());
			m_ReportCtrl.SetItemText(nRow, 2, pItem->GetServerType());
			m_ReportCtrl.SetItemText(nRow, 3, pItem->GetServerName());
			m_ReportCtrl.SetItemText(nRow, 4, pItem->GetServerPath());

			if (pItem->GetVitalCheck() == true)
				m_ReportCtrl.SetCheck(nRow, TRUE);
			else
				m_ReportCtrl.SetCheck(nRow, FALSE);

			if (pItem->GetStatus() == MServerItem::SERVERSTATUS_START)
				m_ReportCtrl.SetItemBkColor(nRow, 1, COLOR_ALIVE);
			else
				m_ReportCtrl.SetItemBkColor(nRow, 1, COLOR_DEAD);			
			break;
		}
		nRow = m_ReportCtrl.GetNextItem(nRow, LVNI_ALL);
	}
}

void CServerKeeperDlg::UpdateItem(MServerItem* pItem) 
{
	HANDLE hProcess = MProcessController::OpenProcessHandleByFilePath(pItem->GetServerPath());
	if (hProcess) {	// Started
		pItem->SetStatus(MServerItem::SERVERSTATUS_START);
		CloseHandle(hProcess);
		UpdateItemUI(pItem);
	} else {		// Stopped
		pItem->SetStatus(MServerItem::SERVERSTATUS_STOP);
		UpdateItemUI(pItem);

		//
		//if( 0 == pItem->GetSlotID() ) // server
		//{
		//	if( m_ServerKeeper.FindAgent() )
		//		m_ServerKeeper.StopAgent();
		//}

		if (pItem->GetVitalCheck() == true) {
			if (timeGetTime() - pItem->GetTimeLastStatusChanged() > TIME_RELAUNCH) {
				MProcessController::StartProcess( pItem->GetServerPath() );
				
				//if( 1 == pItem->GetSlotID() ) // agent
				//{
				//	pItem->
				//	if( m_ServerKeeper.IsOkServerStart() )
				//	{
				//		// 여기서만 Agent를 실행함.
				//		m_ServerKeeper.StartAgent();
				//		UpdateItemUI(pItem);
				//	}
				//}
				//else
				//{
				//	// m_ServerKeeper.StartServer(pItem);
				//	m_ServerKeeper.StartServer();
				//	UpdateItemUI(pItem);
				//}
			}
		}
	}
}

void CServerKeeperDlg::CheckServerProcessStatus()
{
	MServerItemList* pList = m_ServerKeeper.GetServerItemList();
	for (MServerItemList::iterator i = pList->begin(); i!=pList->end(); i++) {
		MServerItem* pItem = *i;
		UpdateItem(pItem);
	}	
}

void CServerKeeperDlg::AddItemUI(MServerItem* pItem)
{
	int nRow = pItem->GetSlotID();
    nRow = m_ReportCtrl.InsertItem(nRow, "");
	m_ReportCtrl.SetItemText(nRow, 0, "N/A");
	m_ReportCtrl.SetItemText(nRow, 1, "Stop");
	m_ReportCtrl.SetItemText(nRow, 2, pItem->GetServerType());
	m_ReportCtrl.SetItemText(nRow, 3, pItem->GetServerName());
	m_ReportCtrl.SetItemText(nRow, 4, pItem->GetServerPath());
	m_ReportCtrl.SetItemData(nRow, pItem->GetSlotID());
}

void CServerKeeperDlg::OnBnClickedBtnStart()
{
	// TODO: Add your control notification handler code here
	POSITION pos = m_ReportCtrl.GetFirstSelectedItemPosition();
	if (pos) {
		int nItem = m_ReportCtrl.GetNextSelectedItem(pos);
		int nSlotID = m_ReportCtrl.GetItemData(nItem);
		MServerItem* pServerItem = m_ServerKeeper.FindServerItemBySlotID(nSlotID);
		if (pServerItem) {
			if (pServerItem->GetStatus() == MServerItem::SERVERSTATUS_START) {
				AfxMessageBox("Already Started", MB_OK);
				return;
			}
			m_ServerKeeper.StartServer(pServerItem);
			UpdateItem(pServerItem);
		}
	}	
}

void CServerKeeperDlg::OnBnClickedBtnStop()
{
	POSITION pos = m_ReportCtrl.GetFirstSelectedItemPosition();
	if (pos) {
		int nItem = m_ReportCtrl.GetNextSelectedItem(pos);
		int nSlotID = m_ReportCtrl.GetItemData(nItem);
		MServerItem* pServerItem = m_ServerKeeper.FindServerItemBySlotID(nSlotID);
		if (pServerItem) {
			m_ServerKeeper.StopServer(pServerItem);
			UpdateItem(pServerItem);
		}
	}
}

LRESULT CServerKeeperDlg::OnCheckBox(WPARAM wParam, LPARAM lParam)
{
	// Called when the user clicked on a checkbox in the list control
	int nItem = (int)wParam; // Item index
	UINT nMouseEvent  = (UINT)lParam; // Mouse event, usually being WM_LBUTTONDOWN or WM_RBUTTONDOWN

	// TODO: Add your message handler code here
	POSITION pos = m_ReportCtrl.GetFirstSelectedItemPosition();
	if (pos) {
		int nSlotID = m_ReportCtrl.GetItemData(nItem);
		MServerItem* pServerItem = m_ServerKeeper.FindServerItemBySlotID(nSlotID);
		if (pServerItem) {
			if (m_ReportCtrl.GetCheck(nItem) == TRUE)
			{
				pServerItem->SetVitalCheck(true);
				
				if( 0 == nItem )
					m_ServerKeeper.OnRequestSetOneConfig( COLUMN_SERVER_START, CONFIG_AUTO_START_SERVER );
				else if( 1 == nItem )
					m_ServerKeeper.OnRequestSetOneConfig( COLUMN_AGENT_START, CONFIG_AUTO_START_AGENT );
			}
			else
			{
				pServerItem->SetVitalCheck(false);

				if( 0 == nItem )
					m_ServerKeeper.OnRequestSetOneConfig( COLUMN_SERVER_START, CONFIG_MANUAL_START_SERVER );
				else if( 1 == nItem )
					m_ServerKeeper.OnRequestSetOneConfig( COLUMN_AGENT_START, CONFIG_MANUAL_START_AGENT );
			}
			UpdateItem(pServerItem);
		}
	}

	
	return (LRESULT)0;
}
