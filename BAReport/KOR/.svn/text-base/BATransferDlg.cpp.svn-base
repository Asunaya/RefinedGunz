// BATransferDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BAReport.h"
#include "BATransferDlg.h"
#include "BAReportDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BATransferDlg* g_pTransferDlg = NULL;


/////////////////////////////////////////////////////////////////////////////
// BATransferDlg dialog
BATransferDlg::BATransferDlg(CWnd* pParent /*=NULL*/)
	: CDialog(BATransferDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(BATransferDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void BATransferDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(BATransferDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(BATransferDlg, CDialog)
	//{{AFX_MSG_MAP(BATransferDlg)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BATransferDlg message handlers

BOOL BATransferDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CProgressCtrl* pBar = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS);
	pBar->SetRange(0, 100);

	CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_INFO);
	pStatic->SetWindowText("Sending Data to BattleArena...");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void BATransferDlg::TransferCallback(int nReason, DWORD dwSentSize, DWORD dwLocalSize)
{
	if (g_pTransferDlg) {
		switch(nReason) {
		case REASON_PROGRESS:
			{
				CProgressCtrl* pBar = (CProgressCtrl*)g_pTransferDlg->GetDlgItem(IDC_PROGRESS);
				pBar->SetPos((int)((float)dwSentSize/(float)dwLocalSize*100));
Sleep(100);
			}
			break;
		case REASON_END:
			{
				CBAReportDlg* pDlg = (CBAReportDlg*)AfxGetMainWnd();
				pDlg->PostMessage(WM_QUIT);
			}
			break;
		case REASON_ERROR:
			break;
		}
	}
}

void BATransferDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	if (bShow) 
		g_pTransferDlg = this;
	else
		g_pTransferDlg = NULL;
}

void BATransferDlg::OnCancel() 
{
	CBAReportDlg* pDlg = (CBAReportDlg*)AfxGetMainWnd();
	pDlg->DestroyWindow();
}
