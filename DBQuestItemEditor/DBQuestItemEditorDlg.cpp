// DBQuestItemEditorDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "DBQuestItemEditor.h"
#include "DBQuestItemEditorDlg.h"
#include ".\dbquestitemeditordlg.h"


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


// CDBQuestItemEditorDlg 대화 상자



CDBQuestItemEditorDlg::CDBQuestItemEditorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDBQuestItemEditorDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDBQuestItemEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FindCharName, m_szCharName);
	DDX_Control(pDX, IDC_FindCID, m_szCID);
	DDX_Control(pDX, IDC_ChoiceQIID, m_szChoiceQIID);
	DDX_Control(pDX, IDC_SetCount, m_szSetCount);
	DDX_Control(pDX, IDC_QItemList, m_QItemList );
	DDX_Control(pDX, IDC_EditQItem, m_EditBtn);
}

BEGIN_MESSAGE_MAP(CDBQuestItemEditorDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_FindUser, OnBnClickedFinduser)
	ON_BN_CLICKED(IDC_EditQItem, OnBnClickedEditqitem)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
//	ON_WM_LBUTTONDBLCLK()
ON_BN_CLICKED(ID_CLEAR, OnBnClickedClear)
ON_BN_CLICKED(ID_REFRESH, OnBnClickedRefresh)
END_MESSAGE_MAP()


// CDBQuestItemEditorDlg 메시지 처리기

BOOL CDBQuestItemEditorDlg::OnInitDialog()
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

	InitProcess();
	
	return TRUE;  // 컨트롤에 대한 포커스를 설정하지 않을 경우 TRUE를 반환합니다.
}

void CDBQuestItemEditorDlg::OnSysCommand(UINT nID, LPARAM lParam)
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


void CDBQuestItemEditorDlg::SetChoiceQIID( const string strQIID )
{
	m_szChoiceQIID.SetWindowText( strQIID.c_str() );
}


void CDBQuestItemEditorDlg::SetFocusToChoiceQIID()
{
	m_szChoiceQIID.SetFocus();
}

void CDBQuestItemEditorDlg::SetFocusToCount()
{
	m_szSetCount.SetFocus();
}

void CDBQuestItemEditorDlg::Clear()
{
	m_QItemList.Clear();
	m_szCharName.SetWindowText("");
	m_szCID.SetWindowText("");
	m_szChoiceQIID.SetWindowText("");
	m_szSetCount.SetWindowText("");
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면 
// 아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
// 프레임워크에서 이 작업을 자동으로 수행합니다.

void CDBQuestItemEditorDlg::OnPaint() 
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
HCURSOR CDBQuestItemEditorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CDBQuestItemEditorDlg::InitProcess()
{
	MMatchStringResManager::MakeInstance();
	MGetStringResManager()->Init("");	// 실행파일과 같은 폴더에 xml파일이 있다.

	m_QItemList.InitListColumn();

	if( !m_QItemList.LoadQuestItemXML() )
	{
		AfxMessageBox( "Quest item xml로드 실패.\n", 0, 0 );
		return;
	}
}

void CDBQuestItemEditorDlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	if( IDOK == AfxMessageBox("DB Update를 하시겠습니까?", MB_OKCANCEL, 0) )
	{
		m_QItemList.UpdateDB();
	}
	
	// OnOK();
}

void CDBQuestItemEditorDlg::OnBnClickedFinduser()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	char szCID[ 64 ]		= {0};
	char szCharName[ 33 ]	= {0};

	m_szCharName.GetWindowText( szCharName, 33 );
	m_szCID.GetWindowText( szCID, 64 );
	
	if( (0 != m_szCharName.GetWindowTextLength()) && (m_szCID.GetWindowTextLength()) )
	{
		// 캐릭터 이름과 CID가 같이 입력될 경우.

		int nCID = atoi( szCID );
		const string strCharName = m_QItemList.FindCharNameByCID( nCID );
		if( strCharName.empty() )
			return;

		if( 0 == strnicmp(szCharName, strCharName.c_str(), strCharName.size()) )
		{
			if( !m_QItemList.GetDBQItemInfo(nCID) )
				return;
		}
		else
			AfxMessageBox( "입력한 데이터가 일치하지 않습니다.", 0, 0 );
	}
	else
	{
		if( 0 != m_szCharName.GetWindowTextLength() )
		{
			// 이름으로 검색.
			if( !m_QItemList.GetDBQItemInfo(szCharName) )
				return;
		}
		else
		{
			// CID로 검색.
			if( 0 == m_szCID.GetWindowTextLength() )
				return;

			int nCID = atoi( szCID );
			if( !m_QItemList.GetDBQItemInfo(nCID) )
				return;
		}
	}

	m_QItemList.MakeQItemInfoColumns();

	m_szChoiceQIID.SetWindowText( "" );
	m_szSetCount.SetWindowText( "" );
}

void CDBQuestItemEditorDlg::OnBnClickedEditqitem()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	char	szQIID[ 64 ];
	char	szCount[ 64 ];
	long	nQIID;
	int		nCount;

	m_szChoiceQIID.GetWindowText( szQIID, 64 );
	if( 0 == strlen(szQIID) )
	{
		return;
	}

	nQIID = atol( szQIID );

	m_szSetCount.GetWindowText( szCount, 64 );
	if( 0 == strlen(szCount) )
	{
		return;
	}

	nCount = atoi( szCount );

	m_QItemList.EditQItem( nQIID, nCount );
	m_QItemList.MakeQItemInfoColumns();
}

void CDBQuestItemEditorDlg::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if( IDOK == AfxMessageBox("DB Update를 하기전의 작업내용은 저장되지 않습니다.\n종료하시겠습니까?", MB_OKCANCEL, 0) )
        OnCancel();
}
void CDBQuestItemEditorDlg::OnBnClickedClear()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	if( IDOK == AfxMessageBox("Clear하시가전에 DB를 업데이트 하시겠습니까?",MB_OKCANCEL, 0) )
		m_QItemList.UpdateDB();
	
	Clear();
}

void CDBQuestItemEditorDlg::OnBnClickedRefresh()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
