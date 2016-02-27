// AbuseWordDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "AbuseWord.h"
#include "AbuseWordDlg.h"
#include ".\abuseworddlg.h"

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


// CAbuseWordDlg 대화 상자



CAbuseWordDlg::CAbuseWordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAbuseWordDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAbuseWordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ReportCtrl);
}


void CAbuseWordDlg::MakeAbuseWordReportCtrlList()
{
	m_ReportCtrl.DeleteAllItems();

	// Unchecked가 가장 위로 오도록 리스트를 구성함.
	const int nAbuseWordNum = m_AbuseWordDB.GetAbuseListSize();
	for( int i = 0; i < nAbuseWordNum; ++i )
	{
		const ABUSE_WORD_INFO* pAbuseWord = m_AbuseWordDB.GetAbuseWordByPos( i );
		if( 0 != pAbuseWord )
		{
			m_ReportCtrl.InsertItem( i, pAbuseWord->m_strKeyAbuseWord.c_str() );

			m_ReportCtrl.SetItemText( i, 1, pAbuseWord->m_strReplaceWord.c_str() );

			if( AW_USED == pAbuseWord->m_nCheckState )
				m_ReportCtrl.SetCheck( i, true );
		}
	}
}


bool CAbuseWordDlg::LoadConfig()
{
	char szVal[ 512 ];

	GetPrivateProfileString( "DB", "HOST", "", szVal, 511, CONFIG_FILE );
	if( 0 == strlen(szVal) ) return false;
	m_AbuseWordDB.SetHost( szVal );

	GetPrivateProfileString( "DB", "USER", "", szVal, 511, CONFIG_FILE );
	if( 0 == strlen(szVal) ) return false;
	m_AbuseWordDB.SetUser( szVal );

	GetPrivateProfileString( "DB", "PASSWORD", "", szVal, 511, CONFIG_FILE );
	if( 0 == strlen(szVal) ) return false;
	m_AbuseWordDB.SetPassword( szVal );

	GetPrivateProfileString( "DB", "DB", "", szVal, 511, CONFIG_FILE );
	if( 0 == strlen(szVal) ) return false;
	m_AbuseWordDB.SetDB( szVal);

	GetPrivateProfileString( "LOCAL_FILE", "LAST_SAVE_DIR", "", szVal, 511, CONFIG_FILE );
	if( 0 == strlen(szVal) ) return false;
	m_AbuseWordDB.SetLastSaveDir( szVal );

	return true;
}


void CAbuseWordDlg::SaveEnvironInfo()
{
}


void CAbuseWordDlg::DeleteAbuseWord()
{
	if( IDYES == AfxMessageBox("정말 지우겠습니까?", MB_YESNO, 0) )
	{
		if( m_AbuseWordDB.ConnectDB() )
		{
			m_AbuseWordDB.DeleteAbuseWord( m_ReportCtrl.GetSelectedItemText() );
			m_AbuseWordDB.DisconnectDB();

			OnDBGetAbuseList();		
		}
		else
			AfxMessageBox( "단어 제거실패." );
	}
}


BEGIN_MESSAGE_MAP(CAbuseWordDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_DB_UPDATE_DB, OnUpdateDB)
	ON_COMMAND(ID_SAVE_LOCAL_FILE, OnSaveLocalFile)
	ON_COMMAND(ID_DB_GET_ABUSE_LIST, OnDBGetAbuseList)
	ON_BN_CLICKED(ID_DB_UPDATE, OnBnClickedDbUpdate)
	ON_BN_CLICKED(IDCLOSE, OnBnClickedClose)
	ON_COMMAND(ID_EXIT, OnExit)
	ON_COMMAND(ID_SEARCH, OnSearchAbuseWord)
	ON_COMMAND(ID_INSERT, OnInsertAbuseWord)
	ON_COMMAND(ID_DELETE_UNCHECKED, OnDeleteUnchecked)
	ON_COMMAND(ID_DELETE_ABUSE_WORD, OnDeleteAbuseWord)
END_MESSAGE_MAP()


// CAbuseWordDlg 메시지 처리기

BOOL CAbuseWordDlg::OnInitDialog()
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

	InitReportColumn();

	if( !m_SearchAbuseWord.Create(IDD_SEARCH, this) )
	{
		AfxMessageBox( "탐색윈도우 생성 실패." );
		return FALSE;
	}

	if( !m_InsertAbuseWordDlg.Create(IDD_INSERT_ABUSE_WORD, this) )
	{
		AfxMessageBox( "단어 추가윈도우 생성 실패." );
		return FALSE;
	}

	if( !LoadConfig() )
	{
		AfxMessageBox( "Config.ini파일 로드 실패." );
		return FALSE;
	}
	
	return TRUE;  // 컨트롤에 대한 포커스를 설정하지 않을 경우 TRUE를 반환합니다.
}


void CAbuseWordDlg::InitReportColumn()
{
	m_ReportCtrl.SetExtendedStyle( LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	InsertReportColumns();
}


void CAbuseWordDlg::InsertReportColumns()
{
	char* pszColumns[] = {"AbuseWord", "ReplaceWord" };

	for( int i = 0; i < 2; ++i )
		m_ReportCtrl.InsertColumn( i, pszColumns[i], LVCFMT_LEFT, 150 );
}


void CAbuseWordDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CAbuseWordDlg::OnPaint() 
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
HCURSOR CAbuseWordDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAbuseWordDlg::OnUpdateDB()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	if( m_AbuseWordDB.ConnectDB() )
	{
		if( m_AbuseWordDB.UpdateDB() )
		{
			AfxMessageBox( "DB업데이트 완료.\n리스트를 업데이트된 DB의 내용으로 갱신합니다.", MB_OK, 0 );
			OnDBGetAbuseList();
		}
		else
			AfxMessageBox( "DB업데이트 실패." );

		m_AbuseWordDB.DisconnectDB();
	}
}

void CAbuseWordDlg::OnSaveLocalFile()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	OPENFILENAME ofn;
	char szFileName[ MAX_PATH ] = {0,};
	char szFileTitle[ MAX_PATH ] = {0,};

	memset( &ofn, 0, sizeof(OPENFILENAME) );
	
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter="단어리스트(*.txt)\0*.txt";
	ofn.lpstrFile = szFileName;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrInitialDir = m_AbuseWordDB.GetLastSaveDir().c_str();

	GetSaveFileName( &ofn );

	if( m_AbuseWordDB.SaveLocalFile(string(szFileName)) )
	{
		AfxMessageBox( "저장 완료." );

		m_AbuseWordDB.SetLastSaveDir( szFileName );
	}
	else
		AfxMessageBox( "저장 실패" );
}

void CAbuseWordDlg::OnDBGetAbuseList()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	if( m_AbuseWordDB.ConnectDB() )
	{
		m_AbuseWordDB.ClearList();

		m_AbuseWordDB.GetDBAbuseList();
		m_AbuseWordDB.DisconnectDB();

		MakeAbuseWordReportCtrlList();		
	}
}


void CAbuseWordDlg::OnBnClickedDbUpdate()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	OnUpdateDB();
}


void CAbuseWordDlg::OnBnClickedClose()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	if( IDYES == AfxMessageBox("마지막 업데이트 후의 작업은 DB에 적용되지 않습니다.\n종료하시겠습니까?", MB_YESNO, 0) )
	{
		OnCancel();
	}
}


void CAbuseWordDlg::OnExit()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	OnBnClickedClose();
}


void CAbuseWordDlg::OnSearchAbuseWord()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	m_SearchAbuseWord.Reset();
	m_SearchAbuseWord.ShowWindow( SW_SHOW );
}

void CAbuseWordDlg::OnInsertAbuseWord()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	m_InsertAbuseWordDlg.Reset();
	m_InsertAbuseWordDlg.ShowWindow( SW_SHOW );
}


void CAbuseWordDlg::OnDeleteUnchecked()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	if( m_AbuseWordDB.DeleteUnchecked() )
	{
		AfxMessageBox( "표시되지 않은 단어제거 완료.\n리스트를 업데이트된 DB의 내용으로 갱신합니다." );
		OnDBGetAbuseList();
	}
	else
		AfxMessageBox( "표시되지 않은 단어제거 실패." );
}

void CAbuseWordDlg::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	SaveEnvironInfo();

	CDialog::OnCancel();
}
LRESULT CAbuseWordDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return CDialog::WindowProc(message, wParam, lParam);
}

BOOL CAbuseWordDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	switch( pMsg->message)    {
	case WM_KEYDOWN:
		{
			const int nKey = (int)pMsg->wParam;
			switch( nKey )
			{
			case 46 : // Delete key.
				{
					DeleteAbuseWord();
				}
				break;
			};
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CAbuseWordDlg::OnDeleteAbuseWord()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	DeleteAbuseWord();
}
