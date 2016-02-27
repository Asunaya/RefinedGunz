
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "TorrentDlg.h"

class DownloaderApp : public CWinApp
{
public:
	DECLARE_MESSAGE_MAP()

public:
	DownloaderApp();

	virtual BOOL InitInstance();

	TorrentDlg m_Wnd;
	virtual int ExitInstance();

protected:
	char * GetArg(DWORD Arg);

	CMutex m_Mutex;
	CMutex m_FileLoad;
};

extern DownloaderApp theApp;