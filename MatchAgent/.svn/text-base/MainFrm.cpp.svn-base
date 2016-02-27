// MainFrm.cpp : CMainFrame 클래스의 구현
//

#include "stdafx.h"
#include "MatchAgent.h"

#include "MainFrm.h"
#include "MRegistry.h"
#include "MMatchAgent.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ACTIVATE()
	ON_WM_CLOSE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // 상태 줄 표시기
	ID_INDICATOR_SERVERSTATUS,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

#define TIMERID_REFRESH_STATUSBAR	101

// CMainFrame 생성/소멸

CMainFrame::CMainFrame()
{
	// TODO: 여기에 멤버 초기화 코드를 추가합니다.
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("도구 모음을 만들지 못했습니다.\n");
		return -1;      // 만들지 못했습니다.
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("상태 표시줄을 만들지 못했습니다.\n");
		return -1;      // 만들지 못했습니다.
	}
	// TODO: 도구 모음을 도킹할 수 없게 하려면 이 세 줄을 삭제하십시오.
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	ReadPosition();

	// 서버상태바 타이머
	this->SetTimer(TIMERID_REFRESH_STATUSBAR, 1000, NULL);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	// Window 클래스 또는 스타일을 수정합니다.

	return TRUE;
}


// CMainFrame 진단

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame 메시지 처리기


void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CMDIFrameWnd::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
}

void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CMDIFrameWnd::OnActivate(nState, pWndOther, bMinimized);

	// TODO: Add your message handler code here
}

void CMainFrame::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	WritePosition();

	CMatchAgentApp* pApp = (CMatchAgentApp*)AfxGetApp();
	pApp->Shutdown();

	CMDIFrameWnd::OnClose();
}

void CMainFrame::ReadPosition()
{
	RECT rt;
	DWORD dwSize = sizeof(rt);
	if (MRegistry::ReadBinary(HKEY_CURRENT_USER, "MainPosition", (char*)&rt, &dwSize) == true)
		MoveWindow(&rt);
		
}

void CMainFrame::WritePosition()
{
	RECT rt;
	GetWindowRect(&rt);
	if (rt.left < 0 || rt.top < 0 || rt.right - rt.left < 0 || rt.bottom - rt.top < 0)
		return;

	MRegistry::WriteBinary(HKEY_CURRENT_USER, "MainPosition", (char*)&rt, sizeof(rt));
}

void CMainFrame::OnTimer(UINT nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent == TIMERID_REFRESH_STATUSBAR) {
		UpdateServerStatusBar();
	}

	CMDIFrameWnd::OnTimer(nIDEvent);
}


void CMainFrame::UpdateServerStatusBar()
{
	MMatchAgent* pServer = MMatchAgent::GetInstance();

	double dFPS = ((CMatchAgentApp*)AfxGetApp())->GetFPS();

	CString strStatus;
	strStatus.Format("C(%u),N(%u),S(%u) || LPS(%.1f)", 
		pServer->GetClientCount(), pServer->GetCommObjCount(), pServer->GetStageCount(), dFPS);

	int nIndex = m_wndStatusBar.CommandToIndex(ID_INDICATOR_SERVERSTATUS);
	m_wndStatusBar.SetPaneInfo(nIndex, ID_INDICATOR_SERVERSTATUS, SBPS_STRETCH, 200);
	m_wndStatusBar.SetPaneText(nIndex, strStatus);
}