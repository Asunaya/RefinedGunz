// BirdAgentDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "BirdAgent.h"
#include "BirdAgentDlg.h"
#include "MALConverter.h"
#include ".\birdagentdlg.h"
#include "MDebug.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

MConverter	g_DBConverter;

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


// CBirdAgentDlg 대화 상자



CBirdAgentDlg::CBirdAgentDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBirdAgentDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CBirdAgentDlg::~CBirdAgentDlg()
{
	m_TrayIcon.RemoveIcon();
}

void CBirdAgentDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBirdAgentDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
	ON_MESSAGE(WM_ICON_NOTIFY, OnTrayNotification)
	ON_COMMAND(ID__32774, OnMenuQuit)
	ON_WM_TIMER()
	ON_COMMAND(ID__, OnMenuShow)
	ON_COMMAND(ID__32772, OnMenuHide)
END_MESSAGE_MAP()


// CBirdAgentDlg 메시지 처리기

BOOL CBirdAgentDlg::OnInitDialog()
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
	

	
	
	// 트레이 아이콘 등록
    if (!m_TrayIcon.Create(this, WM_ICON_NOTIFY, _T("트레이 아이콘 테스트"), NULL, IDR_TRAY_MENU))
         return -1;

	// 아이콘 설정
    // 아이콘 아이디에 자신의 아이콘을 넣으세요...
    m_TrayIcon.SetIcon(IDR_MAINFRAME);

	SetTimer(1, 10000, NULL);


	return TRUE;  // 컨트롤에 대한 포커스를 설정하지 않을 경우 TRUE를 반환합니다.
}

void CBirdAgentDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CBirdAgentDlg::OnPaint() 
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

LONG CBirdAgentDlg::OnTrayNotification(UINT wParam, LONG lParam)
{
	if (LOWORD(lParam) == WM_LBUTTONDBLCLK)
	{
		if (IsWindowVisible()) ShowWindow(SW_HIDE);
		else ShowWindow(SW_SHOW);
		return 1;
	}

	return m_TrayIcon.OnTrayNotification(wParam, lParam);
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다. 
HCURSOR CBirdAgentDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CBirdAgentDlg::OnBnClickedButton3()
{
	g_DBConverter.ConvertActionLeague();	
}


void CBirdAgentDlg::OnMenuQuit()
{
	SendMessage(WM_CLOSE, 0, 0);
	
}

void CBirdAgentDlg::OnTimer(UINT nIDEvent)
{
/*
	CTime theTime = CTime::GetCurrentTime();

	static bool st_bDiffDay = false;
	//static int st_nLastDay = theTime.GetDay();
	static int st_nLastDay = 28;
	if (theTime.GetDay() != st_nLastDay) st_bDiffDay = true;
	st_nLastDay = theTime.GetDay();

	if ((st_bDiffDay) && (theTime.GetHour() == 5))
	{
		st_bDiffDay = false;


		g_DBConverter.ConvertActionLeague();
	}


	//blog("Day: %d, Hour: %d, Min: %d, Sec:%d\n", theTime.GetDay(), theTime.GetHour(), theTime.GetMinute(), theTime.GetSecond());

	CDialog::OnTimer(nIDEvent);
*/
}

void CBirdAgentDlg::OnMenuShow()
{
	ShowWindow(SW_SHOW);
}
void CBirdAgentDlg::OnMenuHide()
{
	ShowWindow(SW_HIDE);
}
