
#include "stdafx.h"
#include "Downloader.h"
#include "sha1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(DownloaderApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

DownloaderApp theApp;

DownloaderApp::DownloaderApp() :
	m_Mutex(0, "Torrent", NULL) ,
	m_FileLoad(0, "TorrentFile", NULL)
{
}

BOOL DownloaderApp::InitInstance()
{
	char Buffer[1024];

	InitCommonControls();

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

//	AfxInitRichEdit2();

	AfxEnableControlContainer();

	srand((unsigned int)time(NULL));

	// Initialize COM
	TRACE("Initializing COM: ");
	if(FAILED(CoInitialize(NULL)))
	{
		TRACE("Failed\n");
	}else{
		TRACE("Ok\n");
	}

	if(m_Mutex.Lock(250) == 0)
	{
		if(GetArg(1))
		{
			m_FileLoad.Lock(INFINITE);

			GetTempPath(sizeof(Buffer) - 1, Buffer);
			strcat(Buffer, "temp.torrent");
			if(!CopyFile(GetArg(1), Buffer, FALSE))
			{
				Util::ErrorMessage("Error copying '%s'.\n%s", GetArg(1), Util::StrError(GetLastError()));
			}
			else
			{

				HWND hWnd = FindWindow(NULL, "Downloader");

				SendMessage(hWnd, WM_USER, 0, 0);
			}

			m_FileLoad.Unlock();
		}
		else
		{
			Util::WarningMessage("Another instance of Maiet Downloader is already running.");
		}

		return FALSE;
	}

	TRACE("----------------------------------------\n");
	TRACE("Loading Settings:\n");

	Settings::InitDWord ("ChokeUpdateInterval", 30, 10, 1000);		// 30 seconds
	Settings::InitDWord ("ClientKeepAlive", 2, 1, 5);				// 2 minutes
	Settings::InitDWord ("ClientTimeout", 10, 1, 5);				// 10 minutes
	Settings::InitDWord ("MaxActiveTorrents", 2, 1, 100);			// 2 torrents
	Settings::InitDWord ("MaxUploadRate", 10240, 1, 0x10000000);	// upload limit (in Bps) Max=256MBps
	Settings::InitBool  ("MaxUploadRateUnlimited", true);
	Settings::InitDWord ("MaxUploads", 30, 2, 50);					// 4 uploads per torrent
	Settings::InitDWord ("MinClientCheckInterval", 2, 1, 120);		// 2 minutes
	Settings::InitDWord ("MinClients", 25, 10, 200);				// 50 clients per torrent
	Settings::InitDWord ("PieceTimeout", 5, 1, 20);					// 5 minutes
	Settings::InitDWord ("Port", 6881, 0, 65535);					// port 6881 (default bittorrent)
	Settings::InitDWord ("UPnPPortMin", 10000, 0, 65535);			// UPnP Port Minimum
	Settings::InitDWord ("UPnPPortMax", 20000, 0, 65535);			// UPnP Port Maximum
	Settings::InitBool  ("UPnPEnable", true);						// UPnP Port Enable
	Settings::InitDWord ("ScrapeUpdateInterval", 10, 5, 120);		// 10 minutes
	Settings::InitBool  ("TorrentCloseToNotify", false);
	Settings::InitBool  ("SendStopOnClose", false);
	Settings::InitDWord ("BlockSelection", 0, 0, 2);				// 0=Rarest, 1=Random, 2=Sequential
	Settings::InitDWord ("OverviewFlags", 0xFFFFFFFF, 0, 0xFFFFFFFF);
	Settings::InitDWord ("ClientListFlags", 0xFFFFFFFF, 0, 0xFFFFFFFF);

//	InitAdminPassword();

	TRACE("----------------------------------------\n");
	TRACE("\n");

	TorrentDlg dlg;
	m_pMainWnd = &dlg;

	INT_PTR nResponse = dlg.DoModal();

	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

#ifdef DEBUG_MEMORY
	atexit(MDump);
#endif

	return TRUE;
}

int DownloaderApp::ExitInstance()
{
	Settings::Cleanup();

	m_Mutex.Unlock();

	CoUninitialize();

	return CWinApp::ExitInstance();
}

char * DownloaderApp::GetArg(DWORD Arg)
{
	static char Buffer[1024];
	DWORD i;
	char * CmdLine;
	bool InQuotes = false;
	DWORD CurArg = 0;

	Buffer[0] = 0;

	CmdLine = GetCommandLine();

	for(i = 0; i < strlen(CmdLine); i++)
	{
		switch(CmdLine[i])
		{
		case '"':
			InQuotes = !InQuotes;
			break;

		case ' ':
			if(!InQuotes)
			{
				if(i != 0)
				{
					if(CmdLine[i - 1] != ' ')
					{
						CurArg++;
					}
				}
				break;
			}

			// [Space] passes through to here if in quotes
		default:
			if(CurArg == Arg)
			{
				Buffer[strlen(Buffer) + 1] = 0;
				Buffer[strlen(Buffer)] = CmdLine[i];
			}
			break;
		}
	}

	if(Buffer[0] == 0)
	{
		return NULL;
	}

	return Buffer;
}
