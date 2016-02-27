// MatchAgent.cpp : 응용 프로그램에 대한 클래스 동작을 정의합니다.
//

#include "stdafx.h"
#include "MatchAgent.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "MatchAgentDoc.h"
#include "MatchAgentView.h"
#include "OutputView.h"
#include "CommandLogView.h"
#include "MRegistry.h"
#include "MDebug.h"
#include "MSync.h"
#include "Shlwapi.h"
#include "Matchagent.h"
#include "MMatchagent.h"
#include "MDebugUtil.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define APPLICATION_NAME	"MatchAgent"


// 유일한 CMatchAgentApp 개체입니다.
CMatchAgentApp theApp;
MSingleRunController	g_SingleRunController("AgentServer"); 

// CMatchAgentApp
BEGIN_MESSAGE_MAP(CMatchAgentApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// 표준 파일을 기초로 하는 문서 명령입니다.
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// 표준 인쇄 설정 명령입니다.
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_COMMAND(IDM_CMD_DEBUGPEER, OnCmdDebugpeer)
END_MESSAGE_MAP()


static bool GetRecommandLogFileName(char* pszBuf)
{
	if (PathIsDirectory("Log") == FALSE)
		CreateDirectory("Log", NULL);

	time_t		tClock;
	struct tm*	ptmTime;
	
	time(&tClock);
	ptmTime = localtime(&tClock);

	char szFileName[_MAX_DIR];

	int nFooter = 1;
	while(TRUE) {
		sprintf(szFileName, "Log/AgentLog_%02d-%02d-%02d-%d.txt", 
			ptmTime->tm_year+1900, ptmTime->tm_mon+1, ptmTime->tm_mday, nFooter);

		if (PathFileExists(szFileName) == FALSE)
			break;

		nFooter++;
		if (nFooter > 100) return false;
	}
	strcpy(pszBuf, szFileName);
	return true;
}


// CMatchAgentApp 생성
CMatchAgentApp::CMatchAgentApp()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}

// CMatchAgentApp 초기화
BOOL CMatchAgentApp::InitInstance()
{
	if (g_SingleRunController.Create(true) == false)
		return FALSE;

	char szLogFileName[_MAX_DIR];
	if (GetRecommandLogFileName(szLogFileName) == false) 
		return FALSE;

	InitLog(MLOGSTYLE_DEBUGSTRING|MLOGSTYLE_FILE, szLogFileName);

	MRegistry::szApplicationName=APPLICATION_NAME;

	if(m_ZFS.Create(".")==false){
		AfxMessageBox("MAIET Zip File System Initialize Error");
		return FALSE;
	}

	// 응용 프로그램 매니페스트가 ComCtl32.dll 버전 6 이상을 사용하여 비주얼 스타일을
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControls()가 필요합니다. 
	// InitCommonControls()를 사용하지 않으면 창을 만들 수 없습니다.
	InitCommonControls();

	CWinApp::InitInstance();

	// OLE 라이브러리를 초기화합니다.
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// 표준 초기화
	// 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
	// 아래에서 필요 없는 특정 초기화 루틴을 제거해야 합니다.
	// 해당 설정이 저장된 레지스트리 키를 변경하십시오.
	// TODO: 이 문자열을 회사 또는 조직의 이름과 같은
	// 적절한 내용으로 수정해야 합니다.
	SetRegistryKey(_T("로컬 응용 프로그램 마법사에서 생성된 응용 프로그램"));
	LoadStdProfileSettings(4);  // MRU를 포함하여 표준 INI 파일 옵션을 로드합니다.

	// 응용 프로그램의 문서 템플릿을 등록합니다. 문서 템플릿은
	// 문서, 프레임 창 및 뷰 사이의 연결 역할을 합니다.
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_MatchAgentTYPE,
		RUNTIME_CLASS(CMatchAgentDoc),
		RUNTIME_CLASS(CChildFrame), // 사용자 지정 MDI 자식 프레임입니다.
		RUNTIME_CLASS(COutputView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	m_pDocTemplateOutput = pDocTemplate;

	// Template
	m_pDocTemplateCmdLogView = new CMultiDocTemplate(IDR_MatchAgentTYPE,
		RUNTIME_CLASS(CMatchAgentDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CCommandLogView));
//	AddDocTemplate(m_pDocTemplateCmdLogView);

	// 주 MDI 프레임 창을 만듭니다.
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;
	// 접미사가 있을 경우에만 DragAcceptFiles를 호출합니다.
	// MDI 응용 프로그램에서는 m_pMainWnd를 설정한 후 바로 이러한 호출이 발생해야 합니다.
	// 표준 셸 명령, DDE, 파일 열기에 대한 명령줄을 구문 분석합니다.
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// 명령줄에 지정된 명령을 디스패치합니다. 응용 프로그램이 /RegServer, /Register, /Unregserver 또는 /Unregister로 시작된 경우 FALSE를 반환합니다.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// 주 창이 초기화되었으므로 이를 표시하고 업데이트합니다.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();
	return TRUE;
}

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

// 대화 상자를 실행하기 위한 응용 프로그램 명령입니다.
void CMatchAgentApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CMatchAgentApp 메시지 처리기


int CMatchAgentApp::ExitInstance()
{
	// TODO: Add your specialized code here and/or call the base class

	return CWinApp::ExitInstance();
}

void CMatchAgentApp::HeartBeat()
{
	POSITION p = GetFirstDocTemplatePosition(); 
	CDocTemplate* pTemplate = GetNextDocTemplate(p); 
	p = pTemplate->GetFirstDocPosition(); 
	CMatchAgentDoc* pDoc = (CMatchAgentDoc*)pTemplate->GetNextDoc(p); 
	if(pDoc!=NULL) pDoc->Run();
	Sleep(1);
}

double CMatchAgentApp::UpdateFPS()
{
	static DWORD nFrameCount=0;
	nFrameCount++;

	DWORD tmCurrent = timeGetTime();
	static DWORD tmLastUpdate = tmCurrent;

	static double dFPS = 0;

	if (tmCurrent - tmLastUpdate > 1000)	// Update every 1 second
	{
		dFPS = (double)nFrameCount/(double)(tmCurrent-tmLastUpdate)*1000.0f;

		tmLastUpdate = tmCurrent;
		nFrameCount = 0;
	}
	return dFPS;
}

int CMatchAgentApp::Run()
{
	ASSERT_VALID(this);
	_AFX_THREAD_STATE* pState = AfxGetThreadState();

	// acquire and dispatch messages until a WM_QUIT message is received.
	for (;;)
	{
		m_dFPS = UpdateFPS();

		// phase1: check to see if we can do idle work
		if (::PeekMessage(&(pState->m_msgCur), NULL, NULL, NULL, PM_NOREMOVE))
		{
			if (!PumpMessage())
				return ExitInstance();
		}

		if (m_bShutdown == false)
			HeartBeat();
	}
}

BOOL CMatchAgentApp::PreTranslateMessage(MSG* pMsg)
{
	if(GetKeyState(17)<0)
	{
		if(pMsg->message==WM_KEYDOWN && pMsg->wParam=='C')	// For Crash Test
		{
			return TRUE;
		}
		if(pMsg->message==WM_KEYDOWN && pMsg->wParam=='D') // For UI Debug
		{
			MMatchAgent* pServer = (MMatchAgent*)MMatchAgent::GetInstance();
			pServer->RequestLiveCheck();

			return TRUE;
		}
	}

	return CWinApp::PreTranslateMessage(pMsg);
}

void CMatchAgentApp::OnCmdDebugpeer()
{
	MDebugUtil_PrintStagePeerList();
}
