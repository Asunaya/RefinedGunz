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
#include "CDDBAniBmp.h"
#include "explorer.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "ZUpdate.h"


// Launcher Version
#define GUNZLAUNCHER_VERSION		"1,0,0,6"


/////////////////////////////////////////////////////////////////////////////
// CNewAppMainDlg dialog
class CNewAppMainDlg : public CDDBDialog
{
public:
	BOOL			m_bThreadContinue;					// Thread destroyer
	BOOL			m_bFail;
	ZUpdate			m_Update;
	char			m_szClientVersion[ 25];
	bool			m_bQuit;
	bool			m_bUpdating;

	CNewAppMainDlg(CWnd* pParent = NULL);
	bool OnUpdateStart();
	void OnUpdateStop();
	void OnRunUpdateMgr();
	void UpdateProgressUI( bool bClear);
	bool GetGunzExeVersion();
	void OnAbout();
	void OnQuit();

	//{{AFX_DATA(CNewAppMainDlg)
	enum { IDD = IDD_DIALOG_MAINDLG };
	CDDBButton		m_cStartGame;
	CDDBButton		m_cCancelDownload;
	CStatic			m_cUpdateInfo;
	CStatic			m_cPercent1;
	CStatic			m_cPercent2;
	CDDBAniBmp		m_cAnibmpRun;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CNewAppMainDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void DrawDialogWindow();
	virtual void DrawPanel( int x, int y, int width, int height, LPCTSTR lpszTitle, UINT nIconNum, UINT nIDMenuResource, UINT nSubMenu);
	virtual void CreateQuitButton( int x, int y);
	virtual void CreateAboutButton( int x, int y);
	//}}AFX_VIRTUAL


protected:
	char m_szServerIP[ 128];
	int  m_nPort;
	DWORD m_idThread;

	CExplorer		m_cExplorer;
	CWinThread*		m_pThreadUpdate;

	int				m_nSkip;

	//{{AFX_MSG(CNewAppMainDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedRefresh();
	afx_msg void OnBnClickedUpdate();
	afx_msg void OnBnClickedStartgame();
	afx_msg void OnBnClickedCanceldownload();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg LRESULT OnCompleteUpdate( WPARAM wParam, LPARAM lParam);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif
