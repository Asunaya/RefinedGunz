/*
 CNewAppMainDlg 클래스(NewAppMainDlg.h)

  desc : Main Dialog 관련 클래스
  date : 2004년 5월 30일
  comp : 임동환
*/


#ifndef _INCLUDED_CNEWAPPMAINDLG_
#define _INCLUDED_CNEWAPPMAINDLG_


#include "resource.h"
#include "CDDBDialog.h"



/////////////////////////////////////////////////////////////////////////////
// CNewAppMainDlg dialog
class CNewAppMainDlg : public CDDBDialog
{
public:
	CNewAppMainDlg(CWnd* pParent = NULL);

	//{{AFX_DATA(CNewAppMainDlg)
	enum { IDD = IDD_DIALOG_MAINDLG };
	CDDBButton		m_cStartGame;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CNewAppMainDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void DrawDialogWindow();
	//}}AFX_VIRTUAL


protected:
	//{{AFX_MSG(CNewAppMainDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedStartgame();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif
