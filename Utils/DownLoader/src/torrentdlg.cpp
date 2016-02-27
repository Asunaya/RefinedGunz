// TorrentDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Downloader.h"
#include "TorrentDlg.h"
#include "sha1.h"
#include "version.h"

IMPLEMENT_DYNAMIC(TorrentDlg, CDialog)
BEGIN_MESSAGE_MAP(TorrentDlg, CDialog)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_DROPFILES()
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_NOTIFICATION_EXIT, OnNotificationExit)
	ON_COMMAND(ID_NOTIFICATION_RESTORE, OnNotificationRestore)
	ON_COMMAND(ID_TORRENT_PAUSE, OnTorrentPause)
	ON_COMMAND(ID_TORRENT_RESUME, OnTorrentResume)
	ON_COMMAND(ID_CONNECTIONINFO, OnConnectioninfo)
END_MESSAGE_MAP()

WORD TorrentDlg::m_Port;
UPnP TorrentDlg::m_UPnP;

TorrentDlg::TorrentDlg(CWnd* pParent /*=NULL*/)
: CDialog(TorrentDlg::IDD, pParent)
{
	m_ProcessScrapesThread = INVALID_HANDLE_VALUE;
	m_InNotification = false;
}

TorrentDlg::~TorrentDlg()
{
}

void TorrentDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Control(pDX, IDS_DOWNLOADED, m_Downloaded);
	DDX_Control(pDX, IDS_DOWNLOAD_RATE, m_DownloadedRate);
	DDX_Control(pDX, IDS_ESTIMATED_TIME, m_EstimatedTime);
	DDX_Control(pDX, IDS_PATH_TEXT, m_Path);
	DDX_Control(pDX, IDS_PROGRESS_TEXT, m_ProgressText);
	DDX_Control(pDX, IDS_STATUS_TEXT, m_Status);
	DDX_Control(pDX, IDS_UPLOADED, m_Uploaded);
	DDX_Control(pDX, IDS_UPLOADED_RATE, m_UploadedRate);
	DDX_Control(pDX, IDC_CHECK_AUTOEXEC, m_AutoExec);
}

int TorrentDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_MainMenu.LoadMenu(IDR_TORRENT_MENU);
	SetMenu(&m_MainMenu);

	m_NextScrapeCheck = Settings::GetDWord("ScrapeUpdateInterval") * 60;

	// Listen for Incomming Clients
	m_ListeningPort.SetCallbacks(_OnTorrentAccept, NULL, NULL, NULL, NULL, (DWORD_PTR)this);

	// UPnP
	if(Settings::GetBool("UPnPEnable"))
	{
		// Get Random Port
		WORD PortMin;
		WORD PortMax;

		PortMin = (WORD)Settings::GetDWord("UPnPPortMin");
		PortMax = (WORD)Settings::GetDWord("UPnPPortMax");

		if(PortMax - PortMin <= 0)
		{
			m_Port = PortMin;
		}
		else
		{
			m_Port = rand() % (PortMax - PortMin) + PortMin;
		}

		// Activate Port
		if(m_UPnP.Create(m_Port))
		{
			TRACE("UPnP: Port: %d\n", m_Port);
		}
		else
		{
			// Failed: Use Default Port
			TRACE("UPnP: Failed to forward port\n");
			Sleep(1000);
			m_Port = (WORD)Settings::GetDWord("Port");
		}
	}
	else
	{
		TRACE("UPnP: UPnP is disbled\n");
		// Use Default Port
		m_Port = (WORD)Settings::GetDWord("Port");
	}

	TRACE("Listening on port %d\n", m_Port);
	m_ListeningPort.Create(m_Port);
	m_ListeningPort.Listen();

	// Set Update Timer
	SetTimer(0, 1000, NULL);

	return 0;
}

void TorrentDlg::OnTimer(UINT nIDEvent)
{
	switch(nIDEvent)
	{
	case 0:
		Update();
		break;
	}

	CDialog::OnTimer(nIDEvent);
}

// Called once per second
void TorrentDlg::Update(void)
{
	DWORD i;
	bool UpdateScrapes = false;
	DWORD ActiveTorrents = 0;
	DWORD MaxSpeed;

	if(m_NextScrapeCheck == 0)
	{
		m_NextScrapeCheck = Settings::GetDWord("ScrapeUpdateInterval") * 60;
		UpdateScrapes = true;
		TRACE("Updating Scrapes\n");
	}else{
		m_NextScrapeCheck--;
	}

	for(i = 0; i < m_Torrents.GetSize(); i++)
	{
		if(m_Torrents[i]->IsActive()) ActiveTorrents++;
	}

	MaxSpeed = Settings::GetDWord("MaxUploadRate");
	if(ActiveTorrents > 0) MaxSpeed /= ActiveTorrents;
	if(Settings::GetBool("MaxUploadRateUnlimited")) MaxSpeed = 0;

	for(i = 0; i < m_Torrents.GetSize(); i++)
	{
		if(m_Torrents[i]->IsActive()) m_Torrents[i]->SetMaxUpload(MaxSpeed);
		m_Torrents[i]->Update();
	}

	if(UpdateScrapes)
	{
		CheckScrapes();
	}

//	TRACE("Memory Usage: %d\n", MUsage());
}

void TorrentDlg::OnSize(UINT nType, int cx, int cy)
{
	CRect cRect;
	CDialog::OnSize(nType, cx, cy);
}

void TorrentDlg::OnFileExit()
{
	DestroyWindow();
}

void TorrentDlg::OnFileOpen()
{
	LoadFile();
}

void TorrentDlg::LoadFile(void)
{
	Torrent * t;

	t = MAlloc(Torrent, 1);

	t->SetParent(this);
	m_Path.SetWindowText("Loading...");
	t->CreateEx(0, AfxRegisterWndClass(0, 0, GetSysColorBrush(COLOR_3DFACE), AfxGetApp()->LoadIcon(IDI_LOGO)), "", WS_OVERLAPPEDWINDOW, CRect(0, 0, 400, 250), NULL, NULL, 0);
	
	m_Priority.PushBack(m_Priority.GetSize());
	m_Torrents.PushBack(t);

	if(!t->OpenInsideTorrent(IDR_TORRENT))
	{
		KillTorrent(t);
	}

	PostMessage(WM_SIZE);
}

void TorrentDlg::OnDestroy()
{
	Torrent * t;

	CDialog::OnDestroy();

	if(m_ProcessScrapesThread != INVALID_HANDLE_VALUE) TerminateThread(m_ProcessScrapesThread, -1);

	while(m_Torrents.GetSize())
	{
		t = m_Torrents[0];
		t->ShuttingDown();	// Tells Torrent to save the FileStamp on shutdown for Checkless Loading. Torrent will recheck files if this does not occur.
		t->DestroyWindow();
		m_Torrents.Delete(0);	// Todo: This causes a CDialog Destruction warning? I think the DestroyWindow call is non blocking? Perhaps move the deletion to the destructor?
		MFree(t);
	}

	while(m_PendingConnections.GetSize())
	{
		MFree(m_PendingConnections[0]);
		m_PendingConnections.Delete(0);
	}

	if(m_InNotification)
	{
		Shell_NotifyIcon(NIM_DELETE, &m_NotifyIconData);
	}

	m_UPnP.Destroy();
}

void TorrentDlg::KillTorrent(Torrent * t)
{
	DWORD i;
	DWORD j;

	for(i = 0; i < m_Torrents.GetSize(); i++)
	{
		if(m_Torrents[i] == t)
		{
			// Update the Priority List
			for(j = 0; j < m_Priority.GetSize(); j++)
			{
				if(m_Priority[j] == i)
				{
					m_Priority.Delete(j);
					j--;
				}else{
					if(m_Priority[j] > i)
					{
						m_Priority[j]--;
					}
				}
			}

			m_Torrents.Delete(i);

			break;
		}
	}

	t->DestroyWindow();

	MFree(t);
}

Torrent * TorrentDlg::ItemToTorrent(ListItem Item)
{
	DWORD i;

	for(i = 0; i < m_Torrents.GetSize(); i++)
	{
		if(m_Torrents[i]->GetListItem() == Item)
		{
			return m_Torrents[i];
		}
	}

	return NULL;
}

DWORD WINAPI TorrentDlg::ProcessScrapesThread(void * Parameter)
{
	((TorrentDlg *)Parameter)->ProcessScrapes();
	((TorrentDlg *)Parameter)->m_ProcessScrapesThread = INVALID_HANDLE_VALUE;
	return 0;
}

void TorrentDlg::ProcessScrapes(void)
{
	DWORD i;
	bool UpdateComplete;

	TRACE("Updating Scrapes\n");
	for(i = 0; i < m_Torrents.GetSize(); i++)
	{
		m_Torrents[i]->UpdateScrape();
	}

	do
	{
		Sleep(1000);
		UpdateComplete = true;

		for(i = 0; i < m_Torrents.GetSize(); i++)
		{
			if(!m_Torrents[i]->IsScrapeUpdated())
			{
				UpdateComplete = false;
				break;
			}
		}
	}while(UpdateComplete == false);

	ActivateTorrents();
}

void TorrentDlg::CheckScrapes(void)
{
	DWORD ThreadId;
	m_ProcessScrapesThread = CreateThread(NULL, 0, ProcessScrapesThread, (void *)this, 0, &ThreadId);
}

void TorrentDlg::_OnTorrentAccept(DataSocket * s, DWORD_PTR Custom, int ErrorCode)
{
	((TorrentDlg *)Custom)->OnTorrentAccept(s, ErrorCode);
}

void TorrentDlg::_OnTorrentClose(DataSocket * s, DWORD_PTR Custom, int ErrorCode)
{
	((TorrentDlg *)Custom)->OnTorrentClose(s, ErrorCode);
}

void TorrentDlg::_OnTorrentReceive(DataSocket * s, DWORD_PTR Custom, int ErrorCode)
{
	((TorrentDlg *)Custom)->OnTorrentReceive(s, ErrorCode);
}

void TorrentDlg::OnTorrentAccept(DataSocket * s, int ErrorCode)
{
	DataSocket * ns;

	ns = MAlloc(DataSocket, 1);

	ns->SetCallbacks(NULL, _OnTorrentClose, NULL, _OnTorrentReceive, NULL, (DWORD_PTR)this);

	s->Accept(*ns);

	m_PendingConnections.PushBack(ns);
}

void TorrentDlg::OnTorrentClose(DataSocket * s, int ErrorCode)
{
	DWORD i;

	for(i = 0; i < m_PendingConnections.GetSize(); i++)
	{
		if(m_PendingConnections[i] == s)
		{
			m_PendingConnections.Delete(i);
			MFree(s);
			return;
		}
	}
}

void TorrentDlg::OnTorrentReceive(DataSocket * s, int ErrorCode)
{
	BYTE * Data;
	DWORD Len;
	BYTE TorrentId[20];
	DWORD i, j;
	CString Address;
	WORD Port;
	CString Temp;

	s->GetPeerName(Address, Port);
	Temp.Format("%s:%d", Address, Port);

	if(ErrorCode)
	{
		DEBUG_PROTOCOL("%21s - TorrentDlg::OnTorrentReceive - Error (%d) %s\n", Temp, ErrorCode, Util::StrError(ErrorCode));
		return;
	}

	Data = s->GetData();
	Len = s->GetDataLen();

	if(Len >= 48)
	{
		if(
			Data[0] == 19 &&
			memcmp(&Data[1], "BitTorrent protocol", 19) == 0
			)
		{

			memcpy(TorrentId, &Data[28], 20);

			for(i = 0; i < m_Torrents.GetSize(); i++)
			{
				if(memcmp(TorrentId, m_Torrents[i]->GetTorrentId(), 20) == 0)
				{
					for(j = 0; j < m_PendingConnections.GetSize(); j++)
					{
						if(m_PendingConnections[j] == s)
						{
							m_PendingConnections.Delete(j);
							break;
						}
					}
					m_Torrents[i]->AttachClient(s);
				}
			}
		}else{
			DEBUG_PROTOCOL("Client Requested Unkown Torrent\n");
			for(j = 0; j < m_PendingConnections.GetSize(); j++)
			{
				if(m_PendingConnections[j] == s)
				{
					m_PendingConnections.Delete(j);
					MFree(s);
					break;
				}
			}
		}
	}
}

WORD TorrentDlg::GetListeningPort(void)
{
	return m_Port;
}

LRESULT TorrentDlg::DefWindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_NOTIFICATIONTRAY:
		switch(lParam)
		{
		case WM_LBUTTONUP:
			OnNotificationRestore();
			return 0;
		}
		break;
	}

	return CDialog::DefWindowProc(Msg, wParam, lParam);
}

void TorrentDlg::OnNotificationRestore()
{
	if(m_InNotification == true)
	{
		Shell_NotifyIcon(NIM_DELETE, &m_NotifyIconData);
		ShowWindow(SW_SHOW);
		if(IsIconic())
		{
			ShowWindow(SW_RESTORE);
		}
		m_InNotification = false;
	}
}

void TorrentDlg::OnNotificationExit()
{
	DestroyWindow();
}

void TorrentDlg::OnTorrentPause()
{
}

void TorrentDlg::OnTorrentResume()
{
}

void TorrentDlg::ActivateTorrents(void)
{
	DWORD i;
	List<Torrent *> Incomplete;
	List<Torrent *> Complete;
	DWORD MaxActive;

	// Sort the Torrents into groups by priority
	for(i = 0; i < m_Torrents.GetSize(); i++)
	{
		if(m_Torrents[m_Priority[i]]->IsComplete())
		{
			Complete.InsertSorted(m_Torrents[m_Priority[i]], SortCompleteTorrents);
		}
		else
		{
			Incomplete.PushBack(m_Torrents[m_Priority[i]]);
		}
	}

	MaxActive = Settings::GetDWord("MaxActiveTorrents");

	if(Incomplete.GetSize() > 0)
	{
		// Deactivate Complete Torrents
		for(i = 0; i < Complete.GetSize(); i++)
		{
			Complete[i]->Deactivate();
		}

		// Activate highest priority torrents
		for(i = 0; i < MaxActive && i < Incomplete.GetSize(); i++)
		{
			Incomplete[i]->Activate();
		}

		// Deactivate everything else
		for(; i < Incomplete.GetSize(); i++)
		{
			Incomplete[i]->Deactivate();
		}
	}
	else
	{
		for(i = 0; i < Complete.GetSize() && i < Settings::GetDWord("MaxActiveTorrents"); i++)
		{
			Complete[i]->Activate();
		}

		for(; i < Complete.GetSize(); i++)
		{
			Complete[i]->Deactivate();
		}
	}
}

void TorrentDlg::NotificationAlert(const char * Message)
{
	if(m_InNotification)
	{
		if(Util::GetOSVersion() >= 5.0f)
		{
			m_NotifyIconData.uFlags = NIF_INFO;
			strncpy(m_NotifyIconData.szInfo, Message, sizeof(m_NotifyIconData.szInfo) - 1);
			m_NotifyIconData.uTimeout = 10000;
			strncpy(m_NotifyIconData.szInfoTitle, "Torrent", sizeof(m_NotifyIconData.szInfoTitle) - 1);
			m_NotifyIconData.dwInfoFlags = NIIF_INFO;

			Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
		}
	}
}

int TorrentDlg::SortCompleteTorrents(Torrent * a, Torrent * b)
{
	if(a->GetSeedRatio() < b->GetSeedRatio()) return 1;
	return -1;
}

BOOL TorrentDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_LOGO)), TRUE);
	m_AutoExec.SetCheck(BST_CHECKED);

	LoadFile();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void TorrentDlg::OnClose()
{
	CDialog::OnClose();
	PostMessage(WM_DESTROY);
}
void TorrentDlg::OnConnectioninfo()
{
	m_Torrents[0]->m_PeerInfo->ShowWindow(SW_SHOW);
}
