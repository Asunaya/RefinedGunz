// BugsAnalyzerDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "BugsAnalyzer.h"
#include "BugsAnalyzerDlg.h"
#include ".\bugsanalyzerdlg.h"
#include "CBugsAnalyzer.h"

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


// CBugsAnalyzerDlg 대화 상자



CBugsAnalyzerDlg::CBugsAnalyzerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBugsAnalyzerDlg::IDD, pParent)
	, m_strFolderName(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBugsAnalyzerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TARGET, m_strFolderName);
	DDX_Control(pDX, IDC_LIST1, m_lbOutput);
	DDX_Control(pDX, IDC_LIST2, m_lbFileList);
	DDX_Control(pDX, IDC_PROGRESS1, m_pcProgress);
}

BEGIN_MESSAGE_MAP(CBugsAnalyzerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_OPENFOLDER, OnBnClickedBtnOpenfolder)
	ON_BN_CLICKED(IDC_BTN_GO, OnBnClickedBtnGo)
	ON_LBN_SELCHANGE(IDC_LIST1, OnLbnSelchangeOutput)
	ON_LBN_DBLCLK(IDC_LIST2, OnLbnDblclkFileList)
END_MESSAGE_MAP()


// CBugsAnalyzerDlg 메시지 처리기

BOOL CBugsAnalyzerDlg::OnInitDialog()
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
	
	return TRUE;  // 컨트롤에 대한 포커스를 설정하지 않을 경우 TRUE를 반환합니다.
}

void CBugsAnalyzerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CBugsAnalyzerDlg::OnPaint() 
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
HCURSOR CBugsAnalyzerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

#include "FolderDlg.h"
void CBugsAnalyzerDlg::OnBnClickedBtnOpenfolder()
{
	CFolderDlg dlg("폴더를 선택하십시오.", NULL, NULL, NULL, this);
	if(dlg.DoModal() != IDOK) return;
	m_strFolderName = dlg.GetFolderPath();

	UpdateData(FALSE);
}

void CBugsAnalyzerDlg::OnBnClickedBtnGo()
{
	int nCount = m_lbOutput.GetCount();
	for (int i = 0; i < nCount; i++)
	{
		m_lbOutput.DeleteString(0);
	}


	analyzer.Create(m_strFolderName);
	analyzer.Execute();

	
	BugAnalysisList* pAnalysisList = analyzer.GetAnalysisList();

	for (BugAnalysisList::iterator itor = pAnalysisList->begin();
		itor != pAnalysisList->end(); ++itor)
	{
		BugAnalysisItem* pItem = (*itor);
		char szStr[1024];

		sprintf(szStr, "%4d : %s", pItem->nCount, pItem->szFuncName);
		m_lbOutput.AddString(szStr);
	}
	
}

void CBugsAnalyzerDlg::OnLbnSelchangeOutput()
{
	int nCount = m_lbFileList.GetCount();
	for (int i = 0; i < nCount; i++)
	{
		m_lbFileList.DeleteString(0);
	}
	
	BugInfoList* pBugInfoList = analyzer.GetBugInfoList();
	char szFuncName[256];
	CString strCurFunc;
	int nCurIndex = m_lbOutput.GetCurSel();
	m_lbOutput.GetText(nCurIndex, strCurFunc);
	strcpy(szFuncName, strCurFunc);


	for (BugInfoList::iterator itor = pBugInfoList->begin();
		itor != pBugInfoList->end(); ++itor)
	{
		BugInfo* pBugInfo = (*itor);
		if (!stricmp(&szFuncName[7], pBugInfo->szFuncName))
		{
			m_lbFileList.AddString(pBugInfo->szLogFileName);
		}
	}

}

void CBugsAnalyzerDlg::OnLbnDblclkFileList()
{
	char szFileName[256];
	int nCurIndex = m_lbFileList.GetCurSel();
	m_lbFileList.GetText(nCurIndex, szFileName);

	char szExeName[256];
	sprintf(szExeName, "NotePad %s", szFileName);

	WinExec(szExeName, SW_SHOW);	// Launch again

}
