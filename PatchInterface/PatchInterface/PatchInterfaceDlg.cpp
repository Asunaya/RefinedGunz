// PatchInterfaceDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "PatchInterface.h"
#include "PatchInterfaceDlg.h"
#include "ReportCtrl.h"
#include ".\patchinterfacedlg.h"
#include "KeeperManager.h"
#include "mcrc32.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 대화 상자 데이터
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원

// 구현
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{

}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CPatchInterfaceDlg 대화 상자



CPatchInterfaceDlg::CPatchInterfaceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPatchInterfaceDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nTimerIDCheckProcess = 0;
}

void CPatchInterfaceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADVERTISEMENT, m_AnnounceEdit);
	DDX_Control(pDX, IDC_SEND, m_AnnounceSendBtn);
	// DDX_Control(pDX, IDC_CRC32, m_CRC32);
}

BEGIN_MESSAGE_MAP(CPatchInterfaceDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_SEND, OnBnClickedSend)
	ON_COMMAND(ID_CONNECT_KEEPER, OnConnectKeeper)
	ON_COMMAND(ID_DISCONNECT_KEEPER, OnDisconnectKeeper)
	ON_COMMAND(ID_CONNECT_SERVER, OnConnectServer)
	ON_COMMAND(ID_DISCONNECT_SERVER, OnDisconnectServer)
	ON_COMMAND(ID_AGENT_STOP, OnAgentStop)
	ON_COMMAND(ID_AGENT_START, OnAgentStart)
	ON_COMMAND(ID_SERVER_STOP, OnServerStop)
	ON_COMMAND(ID_SERVER_START, OnServerStart)
	ON_COMMAND(ID_SERVER_STOP_WITH_ANNCOUNCE, OnServerStopWithAnncounce)
	ON_COMMAND(ID_REFRESH_LIST, OnRefreshList)
	ON_COMMAND(ID_REBOOT, OnReboot)
	ON_COMMAND(ID_PATCH, OnPatch)
	ON_COMMAND(ID_MAKE_CRC, OnMakeCrc)
	ON_COMMAND(ID_SCHEDULE, OnSchedule)
	ON_EN_SETFOCUS(IDC_ADVERTISEMENT, OnEnSetfocusAdvertisement)
	ON_COMMAND(ID_SERVER_INFO, OnServerInfo)
	ON_COMMAND(ID_LoadServerList, OnLoadserverlist)
	ON_COMMAND(ID_SERVER_RELOAD_CONFIG, OnReloadServerConfig)
END_MESSAGE_MAP()


// CPatchInterfaceDlg 메시지 처리기

BOOL CPatchInterfaceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	// 프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	if( !LoadConfig() )
	{
		AfxMessageBox( "CPatchInterfaceDlg::OnInitDialog - config.ini파일 로드 실패." );
		return false;
	}

	m_nTimerIDCheckProcess = SetTimer( TIMERID, TIMER, NULL );

	GetReport.Create( this, 0 );

	if( !GetKeeperMgr.LoadKeeperList(KEEPER_LIST_FILE) )
	{
		AfxMessageBox( "KeeperList.txt기본 파일로 서버 리스트 구성 실패.", 0, 0 );
		// return FALSE;
	}

	if( !GetKeeperMgr.InitKeeperObjs() )
	{
		AfxMessageBox( "Init Keeper Manager's fail", 0, 0 );
		return FALSE;
	}

	if( !CreateReportCtrl() )
		return FALSE;

	if( m_PatchCRCMaker.Create(IDD_PATCH_CRC32MAKER_DIALOG, this) )
	{
		m_PatchCRCMaker.SetRootDir( m_strPatchRootDir );
		m_PatchCRCMaker.SetListFilePath( m_strListFilePath );
	}	
	else
	{
		AfxMessageBox( "CRC32생성 윈도우 생성실패.", 0, 0 );
		return FALSE;
	}

	if( m_ScheduleMaker.Create(IDD_SCHEDULE_MAKER_DLG, this) )
	{
		if( !m_ScheduleMaker.Init() )
			return false;
	}
	else
	{
		AfxMessageBox( "Schedule생성 윈도우 생성실패.", 0, 0 );
		return FALSE;
	}

	if( !m_Patch.Create(IDD_PATCH_DLG, this) )
	{
		AfxMessageBox( "Patch윈도우 생성 실패." );
		return FALSE;
	}

	if( m_ServerInfo.Create(IDD_SERVER_INFO_DLG, this) )
	{
		m_ServerInfo.Init();
	}
	else
	{
		AfxMessageBox( "서버정보 윈도우 생성 실패." );
		return FALSE;
	}

	if( !m_ReloadServerConfigDlg.Create(IDD_ReloadFileListDlg, this) )
	{
		AfxMessageBox( "Reload윈도 생성 실패." );
		return FALSE;
	}
	
	return TRUE;  // 컨트롤에 대한 포커스를 설정하지 않을 경우 TRUE를 반환합니다.
}


bool CPatchInterfaceDlg::OnRun()
{
	GetKeeperMgr.Update();

	return true;
}


bool CPatchInterfaceDlg::LoadConfig()
{
	char szVal[ 512 ];
	GetPrivateProfileString( "NETWORK", "PORT", "", szVal, 511, CONFIG_FILE );
	if( 0 == strlen(szVal) ) return false;
	GetKeeperMgr.SetPort( atoi(szVal) );

	GetPrivateProfileString( "PATCH_ROOT_DIR", "DIR", "", szVal, 511, CONFIG_FILE );
	if( 0 == strlen(szVal) ) return false;
	m_strPatchRootDir = szVal;

	GetPrivateProfileString( "OUTPUT_DIR", "LIST_FILE_PATH", "", szVal, 511, CONFIG_FILE );
	if( 0 == strlen(szVal) ) return false;
	m_strListFilePath = szVal;

	return true;
}


// Report List를 생성하여 등록시킴.
bool CPatchInterfaceDlg::CreateReportCtrl()
{
	// GetReport.Create( this, 0 );
	GetReport.ShowWindow( SW_SHOW );
	GetReport.InsertColumns();
	GetReport.SetGridLines( true );
	GetReport.SetFullrowSelectStyle( true );
	GetReport.SetCheckboxStyle( 1 );
	GetReport.InsertItems();

	return true;
}

void CPatchInterfaceDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면 
// 아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
// 프레임워크에서 이 작업을 자동으로 수행합니다.

void CPatchInterfaceDlg::OnPaint() 
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

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다. 
HCURSOR CPatchInterfaceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPatchInterfaceDlg::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	if( IDYES == AfxMessageBox("종료하시겠습니까?", MB_YESNO, 0) )
	{
		GetKeeperMgr.Disconnect();
		GetKeeperMgr.Release();

		OnCancel();
	}
}

void CPatchInterfaceDlg::OnTimer(UINT nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	GetKeeperMgr.Update();

	CDialog::OnTimer(nIDEvent);
}

void CPatchInterfaceDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.

	if (m_nTimerIDCheckProcess)
		KillTimer(m_nTimerIDCheckProcess);

	GetKeeperMgr.Disconnect();
	GetKeeperMgr.Release();
}


void CPatchInterfaceDlg::OnBnClickedSend()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	char szBuf[ 256 ] = {0, };

	m_AnnounceEdit.GetWindowText( szBuf, 255 );

	GetKeeperMgr.RequestAnnounce( szBuf, static_cast<int>(strlen(szBuf)) );
}


void CPatchInterfaceDlg::OnConnectKeeper()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	GetKeeperMgr.Connect(); 
}

void CPatchInterfaceDlg::OnDisconnectKeeper()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	GetKeeperMgr.Disconnect();
}

void CPatchInterfaceDlg::OnConnectServer()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	GetKeeperMgr.RequestKeeperConnectMatchServer();
}

void CPatchInterfaceDlg::OnDisconnectServer()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	GetKeeperMgr.RequestDisconnectServer();
}

void CPatchInterfaceDlg::OnAgentStop()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	GetKeeperMgr.RequestStopAgentServer();
}

void CPatchInterfaceDlg::OnAgentStart()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	GetKeeperMgr.RequestStartAgentServer();
}

void CPatchInterfaceDlg::OnServerStop()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	GetKeeperMgr.RequestStopServer();
}

void CPatchInterfaceDlg::OnServerStart()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	GetKeeperMgr.RequestStartServer();
}

void CPatchInterfaceDlg::OnServerStopWithAnncounce()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	GetKeeperMgr.RequestStopServerWithAnnounce();
}

void CPatchInterfaceDlg::OnRefreshList()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	GetKeeperMgr.RequestRefreshServerList();
}

void CPatchInterfaceDlg::OnReboot()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	if( IDYES == AfxMessageBox("서버를 재부팅 하시겠습니까?", MB_YESNO, 0) )
	{
		GetKeeperMgr.RequestRebootWindows();
	}
}

void CPatchInterfaceDlg::OnPatch()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	m_Patch.ShowWindow( SW_SHOW );
}

void CPatchInterfaceDlg::OnMakeCrc()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	m_PatchCRCMaker.Clear();
	m_PatchCRCMaker.ShowWindow( SW_SHOW );
}

void CPatchInterfaceDlg::OnSchedule()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	m_ScheduleMaker.Reset();
	m_ScheduleMaker.ShowWindow( SW_SHOW );
}


void CPatchInterfaceDlg::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	// CDialog::OnOK();
}

void CPatchInterfaceDlg::OnEnSetfocusAdvertisement()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

void CPatchInterfaceDlg::OnServerInfo()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	m_ServerInfo.Reset();
	m_ServerInfo.ShowWindow( SW_SHOW );
}

void CPatchInterfaceDlg::OnLoadserverlist()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	// 여기에서 리스트를 읽어오면 이전의 리스트는 릴리즈하고 다시 구성을 함.

	if( 0 != GetReport.GetItemCount() )
	{
		if( IDYES != AfxMessageBox("키퍼와의 모든 연결이 종료됩니다. \n계속 하시겠습니까?", MB_YESNO, 0) )
			return;
	}

	OPENFILENAME OFN;

	char lpstrFile[MAX_PATH]="";
	char szFileTitle[ MAX_PATH ] = "";
	char szTempName[ MAX_PATH ] = "";

	memset(&OFN, 0, sizeof(OPENFILENAME));
	OFN.lStructSize = sizeof(OPENFILENAME);
	OFN.hwndOwner= 0;
	OFN.lpstrFilter="리스트파일(*.txt)\0*.txt";
	OFN.lpstrFile=lpstrFile;
	OFN.lpstrFileTitle = szFileTitle;
	OFN.lpTemplateName = szTempName;
	OFN.nMaxFile= MAX_PATH;
	OFN.nMaxFileTitle = MAX_PATH;
	OFN.lpstrInitialDir=".\\";

	GetOpenFileName( &OFN );
	
	if( 0 == strlen(lpstrFile) ) return;

	GetKeeperMgr.DeleteKeeperObjs();

	if( !GetKeeperMgr.LoadKeeperList(lpstrFile) ) 
	{
		AfxMessageBox( "파일 읽기 실패.", 0, 0 );
		return;
	}

	if( !GetKeeperMgr.InitKeeperObjs() )
	{
		AfxMessageBox( "Keeper Manager초기화 실패.", 0, 0 );
		return;
	}
	GetReport.DeleteAllItems();
	CreateReportCtrl();
}

void CPatchInterfaceDlg::OnReloadServerConfig()
{
	// make reload file list dialog box.
	m_ReloadServerConfigDlg.ShowWindow( SW_SHOW );
	// GetKeeperMgr.RquestReloadServerConfig();
}


