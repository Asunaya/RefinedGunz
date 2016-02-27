// TransferDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MUpdate.h"
#include "TransferDlg.h"
#include "MUIObserverTransDlg.h"
#include "UserWinMsg.h"


// CTransferDlg dialog
IMPLEMENT_DYNAMIC(CTransferDlg, CDialog)
CTransferDlg::CTransferDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTransferDlg::IDD, pParent)
{
}

CTransferDlg::~CTransferDlg()
{
}

void CTransferDlg::SetInfoText(const char* pszText)
{
	CStatic* pInfo = (CStatic*)GetDlgItem(IDC_STATIC_TRANSFER_INFO);
	pInfo->SetWindowText(pszText);
}

void CTransferDlg::SetFileProgress(int nPercent)
{
	CProgressCtrl* pProgress = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS_FILE);
	pProgress->SetPos(nPercent);
}

void CTransferDlg::SetTotalInfoText(const char* pszText)
{
	CStatic* pTotalInfo = (CStatic*)GetDlgItem(IDC_STATIC_TOTAL_INFO);
	pTotalInfo->SetWindowText(pszText);
}

void CTransferDlg::SetTotalProgress(int nPercent)
{
	CProgressCtrl* pProgress = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS_TOTAL);
	pProgress->SetPos(nPercent);
}

void CTransferDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CTransferDlg, CDialog)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_TRANSFER_INIT, OnTransferInit)
END_MESSAGE_MAP()


// CTransferDlg message handlers

void CTransferDlg::OnBnClickedBtnTest()
{
	TRACE("TEST BUTTON Clicked \n");
}

BOOL CTransferDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	TRACE("CTransferDlg::OnInitDialog() \n");

	CProgressCtrl* pProgress = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS_FILE);
	pProgress->SetRange(0, 100);

	CProgressCtrl* pTotalProgress = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS_TOTAL);
	pTotalProgress->SetRange(0, 100);

	m_UpdateMgr.Create();
	m_UpdateMgr.BindUIObserver(new MUIObserverTransDlg(this));

	PostMessage(WM_TRANSFER_INIT);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CTransferDlg::OnDestroy()
{
	m_UpdateMgr.Destroy();

	CDialog::OnDestroy();

	// TODO: Add your message handler code here
}

long CTransferDlg::OnTransferInit(WPARAM wParam, LPARAM lParam)
{
//	StartUpdate();

	return TRUE;
}

void CTransferDlg::StartUpdate()
{
	m_UpdateMgr.StartUpdate("www.gunz.co.kr", 21, "cowork", "work1248");
}
