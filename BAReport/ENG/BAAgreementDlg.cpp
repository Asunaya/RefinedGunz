// BAAgreementDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BAReport.h"
#include "BAAgreementDlg.h"
#include "BAReportDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// BAAgreementDlg dialog


BAAgreementDlg::BAAgreementDlg(CWnd* pParent /*=NULL*/)
	: CDialog(BAAgreementDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(BAAgreementDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void BAAgreementDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(BAAgreementDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(BAAgreementDlg, CDialog)
	//{{AFX_MSG_MAP(BAAgreementDlg)
	ON_BN_CLICKED(ID_AGREE, OnAgree)
	ON_BN_CLICKED(ID_DISAGREE, OnDisagree)
	//}}AFX_MSG_MAP
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BAAgreementDlg message handlers

BOOL BAAgreementDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_AGREEMENT);
	if (!g_bAgree)
		pEdit->SetWindowText("This program has encountered a critical error, and will have to be terminated. To fix the problem, we highly recommend you send your system specifications and memory status now. If you press 'Accept', GunZ will gather your system specifications, such as your OS, amount of RAM, Video Card information, current memory dump, etc. and send this information to MAIET entertainment, Inc. This information will only be used for program enhancement.");


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void BAAgreementDlg::OnCancel()
{
}

void BAAgreementDlg::OnAgree() 
{
	CBAReportDlg* pDlg = (CBAReportDlg*)AfxGetMainWnd();
	pDlg->Upload();
	DestroyWindow();
}

void BAAgreementDlg::OnDisagree() 
{
	CBAReportDlg* pDlg = (CBAReportDlg*)AfxGetMainWnd();
	pDlg->DestroyWindow();
}

BOOL BAAgreementDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (LOWORD(wParam) == ID_AGREE)
		TRACE("SHIT!");
	return CDialog::OnCommand(wParam, lParam);
}

void BAAgreementDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

}
