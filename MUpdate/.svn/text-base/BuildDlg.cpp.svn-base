// BuildDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MUpdate.h"
#include "BuildDlg.h"
#include "MPatchBuilder.h"
#include "MainFrm.h"
#include "UserWinMsg.h"


// CBuildDlg dialog
IMPLEMENT_DYNAMIC(CBuildDlg, CDialog)
CBuildDlg::CBuildDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBuildDlg::IDD, pParent)
{
}

CBuildDlg::~CBuildDlg()
{
}

void CBuildDlg::StartBuild()
{
	PostMessage(WM_BUILD_INIT);
}

bool CBuildDlg::Build()
{
	MPatchBuilder PatchBuilder;
	return PatchBuilder.Build("patch.xml", "euc-kr");
}

void CBuildDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CBuildDlg, CDialog)
	ON_MESSAGE(WM_BUILD_INIT, OnBuildInit)
END_MESSAGE_MAP()


// CBuildDlg message handlers
long CBuildDlg::OnBuildInit(WPARAM wParam, LPARAM lParam)
{
	if (Build()) {
		AfxMessageBox("Build Patch.xml succeed", MB_OK);
	} else {
		AfxMessageBox("Build Patch.xml failed", MB_OK);
	}

	// Shutdown 
	exit(0);
//	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
//	pFrame->DestroyWindow();

	return TRUE;
}
