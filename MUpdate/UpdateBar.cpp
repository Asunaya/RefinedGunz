// UpdateBar.cpp : implementation file
//

#include "stdafx.h"
#include "MUpdate.h"
#include "UpdateBar.h"
#include "MainFrm.h"
#include "UserWinMsg.h"


// CUpdateBar dialog
IMPLEMENT_DYNAMIC(CUpdateBar, CDialogBar)
CUpdateBar::CUpdateBar(CWnd* pParent /*=NULL*/)
	: CDialogBar()
{
}

CUpdateBar::~CUpdateBar()
{
}

void CUpdateBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CUpdateBar, CDialogBar)
	ON_WM_CREATE()
	ON_MESSAGE(WM_SELFUPDATE_INIT, OnSelfUpdateInit)
END_MESSAGE_MAP()


// CUpdateBar message handlers
int CUpdateBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	m_dlgTransfer.Create(IDD_DLG_TRANSFER, this);
	m_dlgBuild.Create(IDD_DLG_BUILD, this);
	
	CMUpdateApp* pApp = (CMUpdateApp*)AfxGetApp();
	if (pApp->m_lpCmdLine[0] != _T('\0')) {
		if (strcmp("/build", pApp->m_lpCmdLine)==0) {				// '/build'
			m_dlgBuild.ShowWindow(SW_SHOW);
			m_dlgBuild.StartBuild();
		} else if (strcmp("/selfupdate", pApp->m_lpCmdLine)==0) {	// '/selfupdate'
			PostMessage(WM_SELFUPDATE_INIT);
		} else {
			AfxMessageBox("Unknown Argument");
		}
	} else {
		m_dlgTransfer.ShowWindow(SW_SHOW);
		m_dlgTransfer.StartUpdate();
	}

	return 0;
}

long CUpdateBar::OnSelfUpdateInit(WPARAM wParam, LPARAM lParam)
{
	// Wait for terminating MUpdate.exe
	HANDLE hMutex = OpenMutex( MUTEX_ALL_ACCESS, FALSE, MUPDATE_MUTEXNAME);
	if (hMutex) {
		DWORD dwVal = WaitForSingleObjectEx(hMutex, INFINITE, TRUE);
		TRACE("MUTEX Result = %d \n", dwVal);
	}

	// Copy MUpdate_New.exe to MUpdate.exe
	if (!CopyFile("MUpdate_New.exe", "MUpdate.exe", FALSE)) {
		DWORD dwError = GetLastError();
		TRACE("CopyFile Error : %d \n", dwError);
	}

	DeleteFile("MUpdate_New.exe");

	// Shutdown
	exit(0);
//	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
//	pFrame->DestroyWindow();
//	pFrame->PostMessage(WM_CLOSE);

	return TRUE;
}
