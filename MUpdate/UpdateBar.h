#pragma once


#include "TransferDlg.h"
#include "BuildDlg.h"


// CUpdateBar dialog
class CUpdateBar : public CDialogBar
{
	DECLARE_DYNAMIC(CUpdateBar)

protected:
	CTransferDlg	m_dlgTransfer;
	CBuildDlg		m_dlgBuild;

public:
	CUpdateBar(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUpdateBar();

// Dialog Data
	enum { IDD = IDD_DLG_UPDATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
	afx_msg long OnSelfUpdateInit(WPARAM wParam, LPARAM lParam);

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};
