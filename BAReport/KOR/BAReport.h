// BAReport.h : main header file for the BAREPORT application
//

#if !defined(AFX_BAREPORT_H__807D2CFF_5387_4D50_BE58_A7CE08A37706__INCLUDED_)
#define AFX_BAREPORT_H__807D2CFF_5387_4D50_BE58_A7CE08A37706__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CBAReportApp:
// See BAReport.cpp for the implementation of this class
//

class CBAReportApp : public CWinApp
{
public:
	CBAReportApp();
	virtual ~CBAReportApp();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBAReportApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CBAReportApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

class CFileTransfer;

extern bool g_bAgree;
extern CFileTransfer*		g_pFileTransfer;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BAREPORT_H__807D2CFF_5387_4D50_BE58_A7CE08A37706__INCLUDED_)
