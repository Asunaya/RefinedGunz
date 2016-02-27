/*
 CNewApp 클래스(NewApp.cpp)

  date : 2004년 5월 30일
  comp : 임동환
*/


#include <stdafx.h>
#include "NewApp.h"
#include "NewAppMainDlg.h"


// Variables
CNewAppApp theApp;



/************************************************************************************
  CNewApp
*************************************************************************************/
// CNewAppApp
CNewAppApp::CNewAppApp()
{
}


// Message map
BEGIN_MESSAGE_MAP(CNewAppApp, CWinApp)
	//{{AFX_MSG_MAP(CNewAppApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()



/************************************************************************************
  CNewAppApp initialization
*************************************************************************************/
HANDLE g_hMutex = NULL;
BOOL CNewAppApp::InitInstance()
{
	// Mutex 생성
	BOOL bFound = FALSE; 
	g_hMutex = ::CreateMutex( NULL, TRUE, ZUPDATE_MUTEXNAME);
    
	// 만약 이미 만들어져 있다면 Instance가 이미 존재함
	if( ::GetLastError() == ERROR_ALREADY_EXISTS) 
		bFound = TRUE;

	if( g_hMutex)
		::ReleaseMutex( g_hMutex);

	// 이미 하나의 Instance가 존재하면 프로그램 종료
	if( bFound)
		return FALSE;
	
	
	AfxEnableControlContainer();

	// Open main dialog box
	CNewAppMainDlg dlg;
	dlg.DoModal();

	
	return false;
}
