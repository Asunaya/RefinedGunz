// Torrent.cpp : implementation file
//

#include "stdafx.h"
#include "Downloader.h"
#include "torrent.h"
#include "TorrentDlg.h"
#include "lexicon.h"
#include "sha1.h"
#include "version.h"


// Torrent

IMPLEMENT_DYNAMIC(Torrent, CDialog)
BEGIN_MESSAGE_MAP(Torrent, CDialog)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

CCriticalSection Torrent::m_CheckTorrentLock;
CFile Torrent::m_LogFile;

Torrent::Torrent()
{
	DWORD i;

	m_Parent = NULL;
	m_TorrentComplete = false;
	m_TorrentBlockCount = 0;
	m_TorrentBlockSize = 0;
	m_TorrentSize = 0;
	m_TorrentPiecesPerBlock = 0;
	m_TorrentBlockHash = NULL;
	m_TorrentBlockMap = NULL;
	m_TorrentPieceMap = NULL;
	m_Rarity = NULL;
	m_State = TORRENTSTATE_UNKNOWN;
	m_CheckBlock = NULL;
	m_Tainted = false;
	m_LoadedFromTorrent = false;
	m_ScrapeUpdated = true;
	m_NextScrapeCheck = 0;
	m_NextMinClientCheck = 0;
	m_CompleteBlockCount = 0;
	
	m_NextChokeUpdate = 0;
	m_BytesIn = 0;
	m_BytesOut = 0;
	m_BytesRedundant = 0;
	m_Eta = 0;
	m_BytesLeft = 0;
	m_Prioritize = false;
	m_LastBlockMode = -1;
	m_NextBlockOrderUpdate = 30;	// seconds

	// Threads
	m_CheckTorrentThread = INVALID_HANDLE_VALUE;
	m_GetClientListThread = INVALID_HANDLE_VALUE;
	m_GetScrapeThread = INVALID_HANDLE_VALUE;

	m_Active = false;
	m_Paused = false;

	// Generate Peer Id
	for(i = 0; i < 20; i++)
	{
		m_PeerId[i] = (BYTE)(rand() % 256);
	}
	memcpy(m_PeerId, TORRENT_VERSION_ID, strlen(TORRENT_VERSION_ID));

	m_ShuttingDown = false;

	m_TorrentEventState = TORRENTEVENT_START;

	m_InNotification = false;

	m_FileName = _T("C:\\Gunz.torrent");

	CString strTempPath;
	TCHAR szpath[MAX_PATH +1];

	::GetModuleFileName(AfxGetInstanceHandle(), szpath, MAX_PATH);
		
	strTempPath = szpath;
	strTempPath.Replace(".exe", ".log");

	m_LogFile.Open(strTempPath, CFile::modeCreate | CFile::modeWrite);
}

Torrent::~Torrent()
{
	m_LogFile.Close();
}

void Torrent::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

int Torrent::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	ShowWindow(SW_HIDE);

	m_PeerInfo = new ConnectionInfo(this);
	m_PeerInfo->CreateEx(0, AfxRegisterWndClass(0, 0, GetSysColorBrush(COLOR_3DFACE), AfxGetApp()->LoadIcon(IDI_INFO)), "", WS_OVERLAPPEDWINDOW, CRect(0, 0, 650, 250), NULL, NULL, 0);

	return 0;
}

void Torrent::OnDestroy()
{
	CDialog::OnDestroy();

	if(m_CheckTorrentThread != INVALID_HANDLE_VALUE) TerminateThread(m_CheckTorrentThread, -1);
	if(m_GetClientListThread != INVALID_HANDLE_VALUE) TerminateThread(m_GetClientListThread, -1);
	if(m_GetScrapeThread != INVALID_HANDLE_VALUE) TerminateThread(m_GetScrapeThread, -1);

	while(m_Clients.GetSize())
	{
		MFree(m_Clients[0]);
		m_Clients.Delete(0);
	}

	// Close Open Files
	while(m_TorrentFiles.GetSize())
	{
		if(m_TorrentFiles[0]->m_hFile != INVALID_HANDLE_VALUE && !m_TorrentComplete)
		{
			CloseHandle(m_TorrentFiles[0]->m_hFile);
		}

		MFree(m_TorrentFiles[0]);
		m_TorrentFiles.Delete(0);
	}

	if(m_TorrentBlockHash) MFree(m_TorrentBlockHash);
	if(m_TorrentBlockMap) MFree(m_TorrentBlockMap);
	if(m_TorrentPieceMap) MFree(m_TorrentPieceMap);
	if(m_Rarity) MFree(m_Rarity);
	if(m_CheckBlock) MFree(m_CheckBlock);

	if(m_InNotification)
	{
		Shell_NotifyIcon(NIM_DELETE, &m_NotifyIconData);
	}

	if(m_Active)
	{
		SendStoppedEvent();
	}

	delete m_PeerInfo;
}

void Torrent::Update(void)
{
	DWORD ThreadId;
	Client * c;
	DWORD i;
	bool Valid;
	CString Temp;
	DWORD BpsIn;
	DWORD BpsOut;
	DWORD Seeds;
	DWORD Leechers;
	CString strProgress;
	float Average;

	if(m_NextScrapeCheck > 0) m_NextScrapeCheck--;

	if(m_Active)
	{
		if(m_NextBlockOrderUpdate == 0)
		{
			UpdateBlockOrder(true);
			m_NextBlockOrderUpdate = 30 + (rand() % 10);	// seconds
		}
		else
		{
			UpdateBlockOrder(false);
			m_NextBlockOrderUpdate--;
		}

		if(m_NextMinClientCheck > 0)
		{
			m_NextMinClientCheck--;
		}
		else
		{
			// if Enough clients are connected then just reset the check
			if(m_Clients.GetSize() < Settings::GetDWord("MinClients"))
			{
				m_GetClientListThread = CreateThread(NULL, 0, GetClientListThread, (void *)this, 0, &ThreadId);
			}
			else
			{
				m_NextMinClientCheck = Settings::GetDWord("MinClientCheckInterval") * 10;
			}
		}

		// If there are more pending clients, connect to one per second
		if(m_PendingClients.GetSize() > 0 )
		{
			// Check to make sure we're not reconnecting to an existing client
			Valid = true;
			for(i = 0; i < m_Clients.GetSize(); i++)
			{
				Temp.Format("%s:%d", m_PendingClients[0].m_Ip, m_PendingClients[0].m_Port);
				if(m_Clients[i]->GetIp() == Temp)
				{
					Valid = false;
					break;
				}
			}

			if(Valid)
			{
				DEBUG_PROTOCOL("Connecting to: %s:%d\n", m_PendingClients[0].m_Ip, m_PendingClients[0].m_Port);
				c = MAlloc(Client, 1);
				c->SetParent(this);
				c->SetClientList(&m_PeerInfo->m_cClientList);
				c->Create(m_PendingClients[0].m_Ip, m_PendingClients[0].m_Port, m_PendingClients[0].m_PeerId);
				m_Clients.PushBack(c);
			}

			m_PendingClients.Delete(0);
		}

		if(!m_TorrentComplete)		// 다운로드 중
		{
			strProgress.Format("Packaging... %s (%.2f%%)", Util::FormatBytes(m_CompleteBlockCount * m_TorrentBlockSize), m_CompleteBlockCount * 100.0f / m_TorrentBlockCount);
			m_Parent->m_ProgressText.SetWindowText(strProgress);
		}
		else	// 완료됨
		{
			m_Parent->m_ProgressText.SetWindowText("Complete");
		}

		if(m_AvgBpsIn.GetAverage() != 0)
		{
			m_Eta = m_BytesLeft / m_AvgBpsIn.GetAverage();
			m_Parent->m_EstimatedTime.SetWindowText(Util::FormatTime(m_Eta));
		}
		else
		{
			m_Parent->m_EstimatedTime.SetWindowText("");
		}

	}
	else
	{
		if(m_TorrentComplete)
		{
			m_Parent->m_ProgressText.SetWindowText("Complete");
		}
		else
		{
			if(m_State != TORRENTSTATE_CHECKING)
			{
				m_Parent->m_ProgressText.SetWindowText("");
			}
		}

		m_Parent->m_EstimatedTime.SetWindowText("");		// Erase Completion Time
	}

	PurgeDeadClients();

	BpsIn = 0;
	BpsOut = 0;
	Seeds = 0;
	Leechers = 0;
	Average = 0.0f;
	for(i = 0; i < m_Clients.GetSize(); i++)
	{
		// Client::Update Resets the Bytes_*_Delta so you have to pull it before the update
		m_BytesIn += m_Clients[i]->GetBytesInDelta();
		m_BytesOut += m_Clients[i]->GetBytesOutDelta();

		m_Clients[i]->Update();

		BpsIn += m_Clients[i]->GetBpsIn();
		BpsOut += m_Clients[i]->GetBpsOut();

		if(m_Clients[i]->IsSeed())
		{
			Seeds++;
		}
		else if(m_Clients[i]->IsLeecher())
		{
			Leechers++;
			Average += m_Clients[i]->GetAverage();
		}
	}

	m_AvgBpsIn.Add(BpsIn);
	m_AvgBpsOut.Add(BpsOut);

	if(Leechers != 0)
	{
		Average /= (float)Leechers;
	}

	if(!m_Active)	
	{
		m_Parent->m_EstimatedTime.SetWindowText("");
	}

	if(m_TorrentComplete)
		m_Parent->m_DownloadedRate.SetWindowText(Util::FormatBps(0));
	else
		m_Parent->m_DownloadedRate.SetWindowText(Util::FormatBps(m_AvgBpsIn.GetAverage()));
	
	m_Parent->m_UploadedRate.SetWindowText(Util::FormatBps(m_AvgBpsOut.GetAverage()));
	
	m_Parent->m_Downloaded.SetWindowText(Util::FormatBytes(m_BytesIn));
	m_Parent->m_Uploaded.SetWindowText(Util::FormatBytes(m_BytesOut));

	if(m_NextChokeUpdate == 0)
	{
		m_NextChokeUpdate = Settings::GetDWord("ChokeUpdateInterval");

		UpdateChokes();

	}else{
		m_NextChokeUpdate--;
	}
}

void Torrent::SetParent(TorrentDlg * Parent)
{
	m_Parent = Parent;
}

ListItem Torrent::GetListItem(void)
{
	return m_hItem;
}

bool Torrent::OpenInsideTorrent(int nFileID)
{
	DWORD BytesWrite = 0;
	DWORD BytesRead = 0;
	Lexicon l;
	DWORD i, j;
	FileInfo * fi;
	char Temp[512];
	char Temp2[512];

	m_LoadedFromTorrent = true;

	HANDLE hResInfo, hRes;

	HINSTANCE hInst = AfxGetInstanceHandle();
	if ( hInst == NULL ) return FALSE;

	// 실행파일이 가진 TORRENT 파일 리소스를 찾아 가져온다. (메모리에 읽어둔다.)
	hResInfo = ::FindResource(hInst, MAKEINTRESOURCE(nFileID), "TORRENT");
	hRes     = ::LoadResource((HMODULE) hInst, (HRSRC) hResInfo);

	if(hRes == NULL)
	{
		Util::ErrorMessage("Failed to load '%s'.\n%s", (LPCTSTR)MAKEINTRESOURCE(nFileID), Util::StrError(GetLastError()));
		return FALSE;
	}

	// 리소스를 위한 전역 메모리를 잠근다. 이 때 리턴되는 값은 메모리상에서의
	// 리소스 시작 포인터이다.
	lpRes = (LPSTR)::LockResource(hRes);

	if(lpRes == NULL)
	{
		Util::ErrorMessage("Failed to allocate memory.");
		return FALSE;
	}
	
	DWORD len = SizeofResource(hInst, (HRSRC)hResInfo);
	char* tmpBuffer = MAlloc(char, len);
	CopyMemory(tmpBuffer, lpRes, len);
	
	size_t lenRes = _tcsclen(lpRes);
	
	l.Build(tmpBuffer, len);	

	MFree(tmpBuffer);

	if(l.ToString("failure reason") != "")
	{
		Util::ErrorMessage("Error parsing '%s'.\n", l.ToString("failure reason"));
		return false;
	}

	m_TorrentAnnounce.DeleteAll();

	if(l.ToInteger("announce-list.count") != 0)
	{
		// Read Announce List
		for(i = 0; i < l.ToInteger("announce-list.count"); i++)
		{
			sprintf(Temp, "announce-list.%d.count", i);

			if(l.ToInteger(Temp) != 0)
			{
				for(j = 0; j < l.ToInteger(Temp); j++)
				{
					sprintf(Temp2, "announce-list.%d.%d", i, j);

					m_TorrentAnnounce.PushBack(l.ToString(Temp2));
					TRACE(" announce%d='%s'\n", m_TorrentAnnounce.GetSize() - 1, l.ToString(Temp2));
				}
			}
		}
	}
	else
	{
		m_TorrentAnnounce.PushBack(l.ToString("announce"));
		TRACE(" announce0='%s'\n", l.ToString("announce"));
	}

	for(i = 0; i < m_TorrentAnnounce.GetSize(); i++)
	{
		CString Announce = m_TorrentAnnounce[i];

		Announce.Replace("announce", "scrape");
		m_TorrentScrape.PushBack(Announce);
		TRACE(" scrape%d='%s'\n", i, Announce);
	}

	m_TorrentName = l.ToString("info.name");
	TRACE(" name='%s'\n", m_TorrentName);

	m_TorrentBlockSize = (DWORD)l.ToInteger("info.piece length");
	TRACE(" block size=%d\n", m_TorrentBlockSize);

	l.GetInfoHash(m_TorrentId);

	m_TorrentPiecesPerBlock = ((m_TorrentBlockSize - 1) / PIECESIZE) + 1;

	m_Priority.DeleteAll();
	m_TorrentSize = 0;

	if(l.ToInteger("info.files.count") != 0)
	{
		TRACE(" file count=%d\n", l.ToInteger("info.files.count"));

		for(i = 0; i < l.ToInteger("info.files.count"); i++)
		{
			fi = MAlloc(FileInfo, 1);

			fi->m_FileStartPos = m_TorrentSize;

			sprintf(Temp, "info.files.%d.length", i);
			fi->m_FileSize = l.ToInteger(Temp);
			m_TorrentSize += fi->m_FileSize;
			fi->m_FilePath = m_TorrentName + '\\';	// Todo: Batch Torrent goes in Subfolder Option?
			sprintf(Temp, "info.files.%d.path.count", i);
			for(j = 0; j < l.ToInteger(Temp); j++)
			{
				sprintf(Temp2, "info.files.%d.path.%d", i, j);
				fi->m_FilePath += l.ToString(Temp2);
				if(j != l.ToInteger(Temp) - 1)
				{
					fi->m_FilePath += '\\';
				}
			}

			TRACE(" file %d\n", i);
			TRACE("  file path='%s'\n", fi->m_FilePath);
			TRACE("  file size=%I64d\n", fi->m_FileSize);

			m_TorrentFiles.PushBack(fi);
			m_Priority.PushBack(m_Priority.GetSize());
		}
	}
	else
	{
		TRACE(" file count=1\n");

		fi = MAlloc(FileInfo, 1);

		fi->m_FilePath = l.ToString("info.name");
		fi->m_FileSize = l.ToInteger("info.length");
		fi->m_FileStartPos = 0;
		m_TorrentSize = fi->m_FileSize;


		TRACE(" file 1\n");
		TRACE("  file path='%s'\n", fi->m_FilePath);
		TRACE("  file size=%I64d\n", fi->m_FileSize);

		m_TorrentFiles.PushBack(fi);
		m_Priority.PushBack(m_Priority.GetSize());
	}

	m_TorrentBlockCount = (DWORD)((m_TorrentSize - 1) / m_TorrentBlockSize) + 1;
	TRACE(" blocks=%d\n", m_TorrentBlockCount);

	m_TorrentBlockHash = MAlloc(BYTE, m_TorrentBlockCount * 20);
	l.ToBinary("info.pieces", (char *)m_TorrentBlockHash, m_TorrentBlockCount * 20);

	m_TorrentBlockMap = MAlloc(BYTE, m_TorrentBlockCount);
	m_TorrentPieceMap = MAlloc(BYTE, m_TorrentBlockCount * m_TorrentPiecesPerBlock);
	m_Rarity = MAlloc(WORD, m_TorrentBlockCount);
	memset(m_TorrentBlockMap, BLOCKSTATE_UNKNOWN, m_TorrentBlockCount);
	memset(m_TorrentPieceMap, BLOCKSTATE_UNKNOWN, m_TorrentBlockCount * m_TorrentPiecesPerBlock);
	memset(m_Rarity, 0, m_TorrentBlockCount * sizeof(WORD));

	// Forces the files to be marked tainted if they already exist
	m_FileTime.dwHighDateTime = 0;
	m_FileTime.dwLowDateTime = 0;

	if(!PrepTorrent()) return false;

	// 잠겨진 메모리를 해제한다. (32bit 윈도우즈에선 호출하지 않아도 된다.)
	::UnlockResource( hRes );
	::FreeResource( hRes );

	return true;
}

void Torrent::SetFileLastModifiedTime(void)
{
	SYSTEMTIME st;
	DWORD i;

	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &m_FileTime);

	for(i = 0; i < m_TorrentFiles.GetSize(); i++)
	{
		if(m_TorrentFiles[i]->m_hFile != INVALID_HANDLE_VALUE)
		{
			SetFileTime(m_TorrentFiles[i]->m_hFile, NULL, NULL, &m_FileTime);
			GetFileTime(m_TorrentFiles[i]->m_hFile, NULL, NULL, &m_FileTime);
		}
	}

	// Mark a torrent as invalid until the final save is performed
	if(m_ShuttingDown == false && m_CompleteBlockCount != m_TorrentBlockCount)
	{
		m_FileTime.dwHighDateTime = 0;
        m_FileTime.dwLowDateTime = 0;
	}
}

DWORD Torrent::GetBlockCount(void)
{
	return m_TorrentBlockCount;
}

DWORD Torrent::GetBlocksComplete(void)
{
	return m_CompleteBlockCount;
}

DWORD Torrent::GetBlockPieceCount(DWORD Block)
{
	ASSERT(Block < m_TorrentBlockCount);

	if(Block != m_TorrentBlockCount - 1)
	{
		return m_TorrentPiecesPerBlock;
	}

	return (GetBlockSize(Block) - 1) / PIECESIZE + 1;
}

DWORD Torrent::GetBlockSize(DWORD Block)
{
	ASSERT(Block < m_TorrentBlockCount);

	if(Block != m_TorrentBlockCount - 1)
	{
		return m_TorrentBlockSize;
	}

	return (DWORD)(m_TorrentSize - ((QWORD)Block * (QWORD)m_TorrentBlockSize));
}

DWORD Torrent::GetPieceSize(DWORD Block, DWORD Piece)
{
	if(Block != m_TorrentBlockCount - 1)
	{
		return PIECESIZE;
	}

	if(Piece != GetBlockPieceCount(Block) - 1)
	{
		return PIECESIZE;
	}

	return GetBlockSize(Block) - (Piece * PIECESIZE);
}

bool Torrent::PrepTorrent()
{
	DWORD i;
	FileInfo * fi;
	char Drive[_MAX_DRIVE];
	char Dir[_MAX_DIR];
	char FName[_MAX_FNAME];
	char Ext[_MAX_EXT];
	FILETIME FileTime;
	DWORD Low, High;
	bool Invalid;
	DWORD ThreadId;
	CString ErrorMessage;
	DWORD ErrorCount = 0;
	int Result;

	m_Tainted = false;

	if(!Util::BrowsePath("Select Download Location", m_TorrentPath))
	{
		m_Parent->EndDialog(IDCANCEL);
		return false;
	}

	Util::CheckPath(m_TorrentPath);

	m_Parent->m_Path.SetWindowText(m_TorrentPath + m_TorrentName);

	// Open all the Files
	for(i = 0; i < m_TorrentFiles.GetSize(); i++)
	{
		fi = m_TorrentFiles[i];

		_splitpath((m_TorrentPath + fi->m_FilePath).GetBuffer(0), Drive, Dir, FName, Ext);

		if(!Util::CreateDirectory(CString(Drive) + Dir))
		{
			TRACE("Error creating directory '%s' Error: %d '%s'\n", CString(Drive) + Dir, GetLastError(), Util::StrError(GetLastError()));
			Log("Error creating directory '%s' Error: %d '%s'\n", CString(Drive) + Dir, GetLastError(), Util::StrError(GetLastError()));
		}

		// 일단 완료된 exe 파일이 있는지 확인한다.
		fi->m_hFile = CreateFile(m_TorrentPath + fi->m_FilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if(fi->m_hFile == INVALID_HANDLE_VALUE)	// 파일이 없다.
		{
			// tmp 파일 생성한다.
			m_TmpFilePath.Format("%s%s", m_TorrentPath ,fi->m_FilePath);
			m_TmpFilePath.Replace(".exe", ".tmp");
			fi->m_hFile = CreateFile(m_TmpFilePath /*m_TorrentPath + fi->m_FilePath*/, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
		}

		if(fi->m_hFile == INVALID_HANDLE_VALUE)
		{
			if(GetLastError() != 2)
			{
				TRACE("Error opening '%s' Error: %d '%s'\n", m_TorrentPath + fi->m_FilePath, GetLastError(), Util::StrError(GetLastError()));
				Log("Error opening '%s' Error: %d '%s'\n", m_TorrentPath + fi->m_FilePath, GetLastError(), Util::StrError(GetLastError()));

				if(ErrorCount < 10)
				{
					ErrorMessage += "Error opening '" + m_TorrentPath + fi->m_FilePath + "'.\n" + Util::StrError(GetLastError()) + "\n";
					ErrorCount++;
				}
			}
			else
			{
				if(m_TorrentComplete)
				{
					Result = MessageBox("'" + m_TorrentName + "' has previously completed but is missing file(s).\nRedownload missing files?\nCancel will delete this file(s).", "Warning", MB_OKCANCEL | MB_ICONWARNING);
					
					if(Result == IDCANCEL)
					{
						DeleteFile(m_TorrentPath + fi->m_FilePath);
						m_Parent->EndDialog(IDOK);
						return false;	
					}

					m_TorrentComplete = false;
				}

				// Create File
				fi->m_hFile = CreateFile(m_TmpFilePath /*m_TorrentPath + fi->m_FilePath*/, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);

				if(fi->m_hFile == INVALID_HANDLE_VALUE)
				{
					// Something Really Blew Up
					TRACE("Error creating '%s' Error: %d '%s'\n", fi->m_FilePath, GetLastError(), Util::StrError(GetLastError()));
					Log("Error creating '%s' Error: %d '%s'\n", fi->m_FilePath, GetLastError(), Util::StrError(GetLastError()));

					if(ErrorCount < 10)
					{
						ErrorMessage += "Error creating '" + fi->m_FilePath + "' " + Util::StrError(GetLastError()) + "\n";
						ErrorCount++;
					}
				}
				else
				{
					// New File. Attempt to make it the correct size
					TRACE(" '%s' created\n", fi->m_FilePath);
					Low = LOWDWORD(fi->m_FileSize);
					High = HIGHDWORD(fi->m_FileSize);
					SetFilePointer(fi->m_hFile, (LONG)Low, (LONG *)&High, FILE_BEGIN);
					if(SetEndOfFile(fi->m_hFile) == 0)
					{
						TRACE(" Error resizing new file '%s' Error: %d '%s'\n", fi->m_FilePath, GetLastError(), Util::StrError(GetLastError()));
						Log(" Error resizing new file '%s' Error: %d '%s'\n", fi->m_FilePath, GetLastError(), Util::StrError(GetLastError()));

						if(ErrorCount < 10)
						{
							ErrorMessage += "Error resizing '" + fi->m_FilePath + "' " + Util::StrError(GetLastError()) + "\n";
							ErrorCount++;
						}
						CloseHandle(fi->m_hFile);
						DeleteFile(m_TorrentPath + fi->m_FilePath);
						fi->m_hFile = INVALID_HANDLE_VALUE;
					}
					else
					{
						TRACE(" '%s' resized\n", fi->m_FilePath);

						memset(m_TorrentBlockMap, BLOCKSTATE_INCOMPLETE, m_TorrentBlockCount);
						memset(m_TorrentPieceMap, BLOCKSTATE_INCOMPLETE, m_TorrentBlockCount * m_TorrentPiecesPerBlock);
						memset(m_Rarity, 0, m_TorrentBlockCount * sizeof(WORD));
					}
				}
			}
		}
		else
		{
			// File already exists

			if(m_LoadedFromTorrent)
			{
				m_Tainted = true;
			}

			TRACE(" '%s' opened existing\n", fi->m_FilePath);
			GetFileTime(fi->m_hFile, NULL, NULL, &FileTime);

			if(memcmp(&FileTime, &m_FileTime, sizeof(FileTime)) != 0)
			{
				TRACE(" File '%s' has been tainted\n", fi->m_FilePath);
				fi->m_Tainted = true;
			}
			else
			{
				TRACE(" File '%s' is clean\n", fi->m_FilePath);
			}
		}
	}

	Invalid = false;
	for(i = 0; i < m_TorrentFiles.GetSize(); i++)
	{
		if(m_TorrentFiles[i]->m_hFile == INVALID_HANDLE_VALUE) Invalid = true;
		if(m_TorrentFiles[i]->m_Tainted) m_Tainted = true;
	}

	if(Invalid)
	{
		if(ErrorCount >= 10)
		{
			ErrorMessage += "Error: Too many errors.\n";
		}
		Util::ErrorMessage(ErrorMessage);

		m_Tainted = true;
		return false;
	}

	if(m_Tainted)
	{
		m_State = TORRENTSTATE_WAITING;
		m_Parent->m_Status.SetWindowText("Waiting...");
		m_CheckTorrentThread = CreateThread(NULL, 0, CheckTorrentThread, (void *)this, 0, &ThreadId);
	}
	else
	{
		PrimeTorrent();
	}

	return true;
}

void Torrent::WriteData(BYTE * Data, DWORD Block, DWORD Offset, DWORD Len)
{
	QWORD Pos;
	DWORD i;
	DWORD NumBytes;
	QWORD DataStart;
	DWORD ToWrite;

	QWORD q;
	DWORD High;
	DWORD Low;

	DataStart = (QWORD)Block * (QWORD)m_TorrentBlockSize + Offset;
	Pos = 0;

	for(i = 0; i < (DWORD)m_TorrentFiles.GetSize(); i++)
	{
		if(DataStart >= m_TorrentFiles[i]->m_FileStartPos && DataStart < m_TorrentFiles[i]->m_FileStartPos + m_TorrentFiles[i]->m_FileSize)
		{
			ToWrite = (DWORD)(m_TorrentFiles[i]->m_FileSize - (DataStart - m_TorrentFiles[i]->m_FileStartPos));
			if(ToWrite > Len - Pos) ToWrite = (DWORD)(Len - Pos);
			q = DataStart - m_TorrentFiles[i]->m_FileStartPos;
			High = HIGHDWORD(q);
			Low = LOWDWORD(q);
			SetFilePointer(m_TorrentFiles[i]->m_hFile, Low, (PLONG)&High, FILE_BEGIN);
			WriteFile(m_TorrentFiles[i]->m_hFile, &Data[Pos], ToWrite, &NumBytes, NULL);

			Pos += NumBytes;
			DataStart += NumBytes;

			if(Pos >= Len) return;
		}
	}
}

void Torrent::ReadData(BYTE * Data, DWORD Block, DWORD Offset, DWORD Len)
{
	QWORD Pos;
	DWORD i;
	DWORD NumBytes;
	QWORD DataStart;
	DWORD ToRead;
	DWORD Length;

	QWORD q;
	DWORD High;
	DWORD Low;

	Length = Len;
	DataStart = (QWORD)Block * (QWORD)m_TorrentBlockSize + Offset;
	Pos = 0;

	for(i = 0; i < (DWORD)m_TorrentFiles.GetSize(); i++)
	{
		if(DataStart >= m_TorrentFiles[i]->m_FileStartPos && DataStart < m_TorrentFiles[i]->m_FileStartPos + m_TorrentFiles[i]->m_FileSize)
		{
			ToRead = (DWORD)(m_TorrentFiles[i]->m_FileSize - (DataStart - m_TorrentFiles[i]->m_FileStartPos));
			if(ToRead > Length - Pos) ToRead = (DWORD)(Length - Pos);
			q = DataStart - m_TorrentFiles[i]->m_FileStartPos;
			High = HIGHDWORD(q);
			Low = LOWDWORD(q);
			SetFilePointer(m_TorrentFiles[i]->m_hFile, Low, (PLONG)&High, FILE_BEGIN);
			ReadFile(m_TorrentFiles[i]->m_hFile, &Data[Pos], ToRead, &NumBytes, NULL);

			Pos += NumBytes;
			DataStart += NumBytes;

			if(Pos >= Length) return;
		}
	}
}

bool Torrent::CheckBlock(DWORD Block, bool CheckPieceCount)
{
	BYTE * Data;
	BYTE Hash[20];
	DWORD BlockSize;
	bool Result;
	DWORD i;

	if(CheckPieceCount)
	{
		for(i = 0; i < GetBlockPieceCount(Block); i++)
		{
			if(GetPieceState(Block, i) != BLOCKSTATE_COMPLETE)
			{
				return false;
			}
		}
		return true;
	}

	BlockSize = GetBlockSize(Block);

	Data = MAlloc(BYTE, BlockSize);

	ReadData(Data, Block, 0, BlockSize);

	Sha1(Hash, Data, BlockSize);

	if(memcmp(Hash, &m_TorrentBlockHash[Block * 20], 20) == 0)
	{
		Result = true;
	}else{
		Result = false;
	}

	MFree(Data);

	return Result;
}

DWORD WINAPI Torrent::CheckTorrentThread(void * Parameter)
{
	((Torrent *)Parameter)->CheckTorrent();
	((Torrent *)Parameter)->m_CheckTorrentThread = INVALID_HANDLE_VALUE;
	TRACE("CheckTorrentThread Finished\n");
	return 0;
}

void Torrent::CheckTorrent(void)
{
	DWORD i, j;
	BYTE Hash[20];
	DWORD Update;
	DWORD BlockSize;
	CString Status;

	DWORD NumComplete = 0;
	DWORD NumIncomplete = 0;

	m_CheckTorrentLock.Lock(INFINITE);

	m_State = TORRENTSTATE_CHECKING;
	m_Parent->m_Status.SetWindowText("Checking...");

	m_CheckBlock = MAlloc(BYTE, m_TorrentBlockSize);

	m_Parent->m_Progress.SetRange(0, (short)m_TorrentBlockCount - 1);
	m_Parent->m_Progress.SetPos(0);

	Update = 0;
	for(i = 0; i < m_TorrentBlockCount; i++)
	{
		BlockSize = GetBlockSize(i);

		ReadData(m_CheckBlock, i, 0, BlockSize);

		Sha1(Hash, m_CheckBlock, BlockSize);

		if(memcmp(Hash, &m_TorrentBlockHash[i * 20], 20) == 0)
		{
			NumComplete++;
			m_TorrentBlockMap[i] = BLOCKSTATE_COMPLETE;
			for(j = 0; j < m_TorrentPiecesPerBlock; j++)
			{
				m_TorrentPieceMap[i * m_TorrentPiecesPerBlock + j] = BLOCKSTATE_COMPLETE;
			}
		}else{
			NumIncomplete++;
			m_TorrentBlockMap[i] = BLOCKSTATE_INCOMPLETE;
			for(j = 0; j < m_TorrentPiecesPerBlock; j++)
			{
				m_TorrentPieceMap[i * m_TorrentPiecesPerBlock + j] = BLOCKSTATE_INCOMPLETE;
			}
		}

//		float progress = i * 1.f / m_TorrentBlockCount * 100.f;
//		m_Parent->m_Progress.SetPos((int)progress);

		// Todo: Update BlockMap Less Frequently? (seems to be fine)
		if(Update == 0)
		{
			Update = 32;

			// Update Block Counts
			Status.Format("Checking... (%d%%)", i * 100 / m_TorrentBlockCount);
			m_Parent->m_Status.SetWindowText(Status);
		}
		else
		{
			Update--;
		}
	}

	// Set Final Check Results
	m_Tainted = false;

	MFree(m_CheckBlock);

	m_CheckTorrentLock.Unlock();

	m_Parent->m_Status.SetWindowText("");

	PrimeTorrent();
}

DWORD WINAPI Torrent::GetClientListThread(void * Parameter)
{
	((Torrent *)Parameter)->GetClientList();
	((Torrent *)Parameter)->m_GetClientListThread = INVALID_HANDLE_VALUE;
	TRACE("GetClientListThread Finished\n");
	return 0;
}

void Torrent::GetClientList(void)
{
	CString TrackerUrl;
	char Buffer[32];
	char Temp[256];
	BYTE * Data;
	DWORD DataLen;
	Lexicon l;
	DWORD i, Count;
	ClientInfo ci;
	WORD Port;
	CString Key;

	Port = TorrentDlg::GetListeningPort();

	m_NextMinClientCheck = Settings::GetDWord("MinClientCheckInterval") * 10;

	Key.Format("%.2x%.2x%.2x%.2x", rand() % 256, rand() % 256, rand() % 256, rand() % 256);

	TrackerUrl =
		m_TorrentAnnounce[0] +
		"?info_hash=" + Util::HttpEncode(m_TorrentId, 20) +
		"&peer_id=" + Util::HttpEncode(m_PeerId, 20) +
		"&port=" + itoa(Port, Buffer, 10) +
		"&key=" + Key +	// No Idea what this is? (Has no relevance whatsoever)
		"&uploaded=" + _i64toa(m_BytesOut, Buffer, 10) +
		"&downloaded=" + _i64toa(m_BytesIn, Buffer, 10) +
		"&left=" + _i64toa(((QWORD)m_TorrentBlockCount - (QWORD)m_CompleteBlockCount) * (QWORD)m_TorrentBlockSize, Buffer, 10) +
		"&compact=1";	// support compact announces

	if(m_TorrentEventState == TORRENTEVENT_START)
	{
		TrackerUrl += "&event=started";
		m_TorrentEventState = TORRENTEVENT_INCOMPLETE;
	}

	if(m_TorrentEventState == TORRENTEVENT_INCOMPLETE && m_TorrentComplete)
	{
		TrackerUrl += "&event=completed";
		m_TorrentEventState = TORRENTEVENT_COMPLETE;
	}

	Log("Retrieving client list from '%s'.\n", m_TorrentAnnounce[0]);
	if(!Util::ReadHttpFile(TrackerUrl, Data, DataLen))
	{
		Log("Failed to retrieve client list: %s\n", Data);
		MFree(Data);
		m_NextMinClientCheck = 1 * 60;

		if(m_TorrentAnnounce.GetSize() != 1) m_NextMinClientCheck = 5;	// 5 second cycle if bad tracker
		m_TorrentAnnounce.PushBack(m_TorrentAnnounce.Pop());
		m_TorrentScrape.PushBack(m_TorrentScrape.Pop());
		return;
	}

	l.Build((char *)Data, DataLen);

	MFree(Data);

	if(l.ToString("failure reason") != "")
	{
		Log("Failed to retrieve client list: %s\n", l.ToString("failure reason"));
		m_NextMinClientCheck = 1 * 60;

		m_TorrentAnnounce.PushBack(m_TorrentAnnounce.Pop());
		m_TorrentScrape.PushBack(m_TorrentScrape.Pop());
		return;
	}

	Count = (DWORD)l.ToInteger("peers.count");
	if(Count == 0)	// No peers?
	{
		// Check for compact announce
		if(l.GetKeyLen("peers") == 0)
		{
			// Something went wrong
			Log("Failed to retrieve client list: No peer data provided.\n");
			m_NextMinClientCheck = 10 * 60;

			m_TorrentAnnounce.PushBack(m_TorrentAnnounce.Pop());
			m_TorrentScrape.PushBack(m_TorrentScrape.Pop());
			return;
		}

		Data = MAlloc(BYTE, l.GetKeyLen("peers"));
		l.ToBinary("peers", (char *)Data, l.GetKeyLen("peers"));

		for(i = 0; i < l.GetKeyLen("peers") / 6; i++)
		{
			sprintf(Temp, "%d.%d.%d.%d", Data[i * 6 + 0], Data[i * 6 + 1], Data[i * 6 + 2], Data[i * 6 + 3]);
			ci.m_Ip = Temp;
			ci.m_Port = SW(CW(Data[i * 6 + 4]));
			memset(ci.m_PeerId, 0, 20);

			m_PendingClients.PushBack(ci);
		}

		MFree(Data);
	}else{
		for(i = 0; i < Count; i++)
		{
			sprintf(Temp, "peers.%d.ip", i);
			ci.m_Ip = l.ToString(Temp);
			sprintf(Temp, "peers.%d.port", i);
			ci.m_Port = (WORD)l.ToInteger(Temp);
			sprintf(Temp, "peers.%d.peer id", i);
			l.ToBinary(Temp, (char *)ci.m_PeerId, 20);

			m_PendingClients.PushBack(ci);
		}
	}

	Log("Client list updated.\n");
}

void Torrent::Activate(void)
{
	DWORD ThreadId;

	if(m_Active) return;

	if(m_State < TORRENTSTATE_READY) return;

//	m_cTorrentList->SetItemImage(m_hItem, TORRENT_IMG_ERROR);

	m_Active = true;
	Resume();	// Unpause

	m_ActivationTime = clock();
	m_BytesLeft = CalculateBytesLeft();

	// Connect to Clients
	m_GetClientListThread = CreateThread(NULL, 0, GetClientListThread, (void *)this, 0, &ThreadId);

	m_NextBlockOrderUpdate = 30;	// Seconds
}

void Torrent::Deactivate(void)
{
	DWORD i;

	if(!m_Active) return;

	m_Active = false;

	for(i = 0; i < m_Clients.GetSize(); i++)
	{
		m_Clients[i]->Kill();
	}

	SendStoppedEvent();

	m_BytesLeft = 0;
}

void Torrent::Pause(void)
{
	if(m_Paused) return;

	m_Paused = true;

	// Todo:
	// Choke Clients
}

void Torrent::Resume(void)
{
	if(!m_Paused) return;

	m_Paused = false;

	// Todo: Get things going again?
	// Todo: Request Data from unchoked clients
}

bool Torrent::IsComplete(void)
{
	return m_TorrentComplete;
}

void Torrent::UpdateScrape(void)
{
	DWORD ThreadId;

	if(m_State < TORRENTSTATE_READY) return;
	if(m_NextScrapeCheck != 0) return;

	m_ScrapeUpdated = false;
	m_NextScrapeCheck = Settings::GetDWord("ScrapeUpdateInterval") * 60;

	m_GetScrapeThread = CreateThread(NULL, 0, GetScrapeThread, (void *)this, 0, &ThreadId);
}

void Torrent::ShuttingDown(void)
{
	m_ShuttingDown = true;
}

DWORD WINAPI Torrent::GetScrapeThread(void * Parameter)
{
	((Torrent *)Parameter)->GetScrape();
	((Torrent *)Parameter)->m_GetScrapeThread = INVALID_HANDLE_VALUE;
	TRACE("GetClientListThread Finished\n");
	return 0;
}

void Torrent::GetScrape(void)
{
	CString TrackerUrl;
	BYTE * Data;
	DWORD DataLen;
	Lexicon l;
	char Temp[256];

	if(m_TorrentScrape[0] == "")
	{
		return;
	}

	TrackerUrl = m_TorrentScrape[0] + "?info_hash=" + Util::HttpEncode(m_TorrentId, 20);

	Log("Retrieving scrape from '%s'.\n", m_TorrentScrape[0]);
	if(!Util::ReadHttpFile(TrackerUrl, Data, DataLen))
	{
		// Error Message in Data
		Log("Failed to retrieve scrape: %s\n", Data);
		MFree(Data);
		m_ScrapeUpdated = true;
		return;
	}

	l.Build((char *)Data, DataLen);

	MFree(Data);

	if(l.ToString("failure reason") != "")
	{
		// Tracker Failed
		Log("Failed to retrieve scrape: %s\n", l.ToString("failure reason"));
		m_ScrapeUpdated = true;
		return;
	}

	sprintf(Temp, "files.%s.complete", Lexicon::ConvertBinaryKey((char *)m_TorrentId, 20));
	m_ScrapeComplete = (DWORD)l.ToInteger(Temp);
	sprintf(Temp, "files.%s.downloaded", Lexicon::ConvertBinaryKey((char *)m_TorrentId, 20));
	m_ScrapeDownloaded = (DWORD)l.ToInteger(Temp);
	sprintf(Temp, "files.%s.incomplete", Lexicon::ConvertBinaryKey((char *)m_TorrentId, 20));
	m_ScrapeIncomplete = (DWORD)l.ToInteger(Temp);

	Log("Scrape updated.\n");

	m_ScrapeUpdated = true;
}

bool Torrent::IsScrapeUpdated(void)
{
	return m_ScrapeUpdated;
}

bool Torrent::IsActive(void)
{
	return m_Active;
}

bool Torrent::IsPaused(void)
{
	return m_Paused;
}

void Torrent::PurgeDeadClients(void)
{
	DWORD i;

	for(i = 0; i < m_Clients.GetSize(); i++)
	{
		if(m_Clients[i]->IsDead())
		{
			MFree(m_Clients[i]);
			m_Clients.Delete(i);
			i--;
		}
	}
}

void Torrent::AttachClient(DataSocket * s)
{
	Client * c;

	if(!m_Active)
	{
		MFree(s);
		return;
	}

	c = MAlloc(Client, 1);
	c->SetParent(this);
	c->SetClientList(&m_PeerInfo->m_cClientList);
	c->Create(s);

	m_Clients.PushBack(c);
}

BYTE * Torrent::GetTorrentId(void)
{
	return m_TorrentId;
}

BYTE * Torrent::GetPeerId(void)
{
	return m_PeerId;
}

CString Torrent::GetTorrentName(void)
{
	return m_TorrentName;
}

void Torrent::IncrementRarity(DWORD Block)
{
	m_Rarity[Block]++;
}

void Torrent::DecrementRarity(DWORD Block)
{
	m_Rarity[Block]--;
}

void Torrent::OnClose()
{
	if(Settings::GetBool("TorrentCloseToNotify"))
	{
		if(m_InNotification == false)
		{
			ZeroMemory(&m_NotifyIconData, sizeof(m_NotifyIconData));

			if(Util::GetOSVersion() >= 5.0)
			{
				m_NotifyIconData.cbSize = sizeof(m_NotifyIconData);
			}else{
				m_NotifyIconData.cbSize = NOTIFYICONDATA_V1_SIZE;
			}

			m_NotifyIconData.hWnd = m_hWnd;
			m_NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE;
			m_NotifyIconData.uCallbackMessage = WM_NOTIFICATIONTRAY;
			m_NotifyIconData.hIcon = AfxGetApp()->LoadIcon(IDI_LOGO);

			Shell_NotifyIcon(NIM_ADD, &m_NotifyIconData);
			
			ShowWindow(SW_HIDE);

			m_InNotification = true;
		}
	}else{
		ShowWindow(SW_HIDE);
	}
}

void Torrent::OnSize(UINT nType, int cx, int cy)
{
	CRect cRect;

	CDialog::OnSize(nType, cx, cy);

	GetClientRect(&cRect);
}


// Do not send more that 1024 bytes of information to this function
void Torrent::Log(const char * Format, ...)
{
	char Buffer[1024];

	va_list Args;

	va_start(Args, Format);
	vsprintf(Buffer, Format, Args);
	va_end(Args);

	m_LogFile.Write(Buffer, (UINT)_tcslen(Buffer));
}

void Torrent::UpdateCompleteBlockCount(void)
{
	DWORD i;

	m_CompleteBlockCount = 0;
	for(i = 0; i < m_TorrentBlockCount; i++)
	{
		if(m_TorrentBlockMap[i] == BLOCKSTATE_COMPLETE)
		{
			m_CompleteBlockCount++;
		}
		else
		{
			// Add block to Incomplete List
			m_IncompleteBlockQueue.PushBack(i);
		}
	}
}

void Torrent::PrimeTorrent(void)
{
	// Get Complete Block Count and Init the Incomplete Block List
	UpdateCompleteBlockCount();
	
	if(m_CompleteBlockCount == m_TorrentBlockCount)
	{
		m_TorrentComplete = true;
		m_TorrentEventState = TORRENTEVENT_COMPLETE;
		CompleteAction();
	}

	CString strProgress;
	strProgress.Format("Packaging... %s (%.2f%%)", Util::FormatBytes(m_CompleteBlockCount * m_TorrentBlockSize), m_CompleteBlockCount * 100.0f / m_TorrentBlockCount);

	m_Parent->m_Progress.SetRange32(0, m_TorrentBlockCount);
	m_Parent->m_Progress.SetPos(m_CompleteBlockCount);
	m_Parent->m_ProgressText.SetWindowText(strProgress);

	m_Parent->m_Progress.SetRange(0, (short)m_TorrentBlockCount);
	m_Parent->m_Progress.SetPos(m_CompleteBlockCount);

	// Can't directly call UpdateData from a thread. Mfc complains that I'm passing C++ Objects around in threads
	// Have to send a message and let the Dialog Update it's own data when it gets the chance

	m_State = TORRENTSTATE_READY;
	m_Parent->CheckScrapes();
}

DWORD Torrent::GetIncompleteBlockCount(void)
{
	return m_IncompleteBlockQueue.GetSize();
}

DWORD Torrent::GetIncompleteBlock(DWORD Index)
{
	ASSERT(Index < m_IncompleteBlockQueue.GetSize());
	return m_IncompleteBlockQueue[Index];
}

void Torrent::SetIncompleteBlockDownloading(DWORD Block)
{
	DWORD i;
	DWORD d;

	for(i = 0; i < m_IncompleteBlockQueue.GetSize(); i++)
	{
		if(m_IncompleteBlockQueue[i] == Block)
		{
			d = m_IncompleteBlockQueue[i];
			m_IncompleteBlockQueue.Delete(i);
			m_DownloadBlockQueue.PushBack(d);
			return;
		}
	}

	ASSERT(FALSE);
}

void Torrent::SetBlockState(DWORD Block, BYTE State)
{
	ASSERT(Block < m_TorrentBlockCount);

	m_TorrentBlockMap[Block] = State;
}

void Torrent::SetPieceState(DWORD Block, DWORD Piece, BYTE State)
{
	BYTE OldState;

	ASSERT(Block < m_TorrentBlockCount);
	ASSERT(Piece < m_TorrentPiecesPerBlock);

	OldState = m_TorrentPieceMap[Block * m_TorrentPiecesPerBlock + Piece];
	m_TorrentPieceMap[Block * m_TorrentPiecesPerBlock + Piece] = State;

	if(OldState != State)
	{
		if(State == BLOCKSTATE_COMPLETE)
		{
			m_BytesLeft -= GetPieceSize(Block, Piece);
		}

		if(OldState == BLOCKSTATE_COMPLETE && State == BLOCKSTATE_INCOMPLETE)
		{
			m_BytesLeft += GetPieceSize(Block, Piece);
		}
	}
}

BYTE Torrent::GetBlockState(DWORD Block)
{
	ASSERT(Block < m_TorrentBlockCount);

	return m_TorrentBlockMap[Block];
}

BYTE Torrent::GetPieceState(DWORD Block, DWORD Piece)
{
	ASSERT(Block < m_TorrentBlockCount);
	ASSERT(Piece < m_TorrentPiecesPerBlock);

	return m_TorrentPieceMap[Block * m_TorrentPiecesPerBlock + Piece];
}

void Torrent::SendHave(DWORD Block)
{
	DWORD i;

	for(i = 0; i < m_Clients.GetSize(); i++)
	{
		m_Clients[i]->Send_Have(Block);
	}
}

DWORD Torrent::GetDownloadingBlockCount(void)
{
	return m_DownloadBlockQueue.GetSize();
}

DWORD Torrent::GetDownloadingBlock(DWORD Block)
{
	if(Block >= m_DownloadBlockQueue.GetSize())
	{
		ASSERT(FALSE);
		return -1;
	}

	return m_DownloadBlockQueue[Block];
}

void Torrent::SetDownloadingBlockComplete(DWORD Block)
{
	DWORD i;

	for(i = 0; i < m_DownloadBlockQueue.GetSize(); i++)
	{
		if(m_DownloadBlockQueue[i] == Block)
		{
			m_DownloadBlockQueue.Delete(i);
			break;
		}
	}

	m_CompleteBlockCount++;

	if(m_CompleteBlockCount == m_TorrentBlockCount)
	{
		m_TorrentComplete = true;
		m_TorrentEventState = TORRENTEVENT_COMPLETE;
		m_Parent->NotificationAlert("'" + m_TorrentName + "' has completed downloading.");
		CompleteAction();
	}

	CString strProgress;
	strProgress.Format("Packaging... %s (%.2f%%)", Util::FormatBytes(m_CompleteBlockCount * m_TorrentBlockSize), m_CompleteBlockCount * 100.0f / m_TorrentBlockCount);

	m_Parent->m_Progress.SetPos(m_CompleteBlockCount);
	m_Parent->m_ProgressText.SetWindowText(strProgress);

	m_Parent->m_Progress.SetPos(m_CompleteBlockCount);
}

void Torrent::SetDownloadingBlockIncomplete(DWORD Block)
{
	DWORD i;
	DWORD d;

	for(i = 0; i < m_DownloadBlockQueue.GetSize(); i++)
	{
		if(m_DownloadBlockQueue[i] == Block)
		{
			d = m_DownloadBlockQueue[i];
			m_DownloadBlockQueue.Delete(i);
			m_IncompleteBlockQueue.Push(d);

			m_TorrentComplete = false;

			return;
		}
	}

	ASSERT(FALSE);
}

int Torrent::SortClientChokes(Client * a, Client * b)
{
	if(a->IsClientInterested() && !b->IsClientInterested()) return 1;
	if(!a->IsClientInterested() && b->IsClientInterested()) return -1;

	if(a->GetAvgBpsIn() > b->GetAvgBpsIn()) return 1;
	return -1;
}

void Torrent::UpdateChokes(void)
{
	DWORD i;
	List<Client *> Sort;
	Client * c;

	// Get list of all clients currently sending to
	for(i = 0; i < m_Clients.GetSize(); i++)
	{
		Sort.InsertSorted(m_Clients[i], SortClientChokes);
	}

	// Unchoke best interested senders
	for(i = 0; i < Settings::GetDWord("MaxUploads") - 1 && i < Sort.GetSize(); i++)
	{
		if(Sort[i]->IsClientInterested())
		{
			Sort[i]->Send_UnChoke();
		}
	}

	// Choke everyone else
	for(; i < Sort.GetSize(); i++)
	{
		Sort[i]->Send_Choke();
	}

	// Optimistic unchoke
	if(m_InterestedClients.GetSize() > 0)
	{
		c = m_InterestedClients.Pop();
		c->Send_UnChoke();
	}
}

void Torrent::QueueClient(Client * c)
{
	DWORD i;

	for(i = 0; i < m_InterestedClients.GetSize(); i++)
	{
		if(m_InterestedClients[i] == c) return;
	}

	m_InterestedClients.PushBack(c);

}

void Torrent::UnQueueClient(Client * c)
{
	DWORD i;

	for(i = 0; i < m_InterestedClients.GetSize(); i++)
	{
		if(m_InterestedClients[i] == c)
		{
			m_InterestedClients.Delete(i);
			return;
		}
	}
}

DWORD Torrent::GetCompleteBlockCount(void)
{
	return m_CompleteBlockCount;
}

void Torrent::AddRedundantData(DWORD Bytes)
{
	m_BytesRedundant += Bytes;
}

void Torrent::SetMaxUpload(DWORD Rate)
{
	DWORD i;
	DWORD Count;

	Count = 0;
	for(i = 0; i < m_Clients.GetSize(); i++)
	{
		if(!m_Clients[i]->IsChoked()) Count++;
	}

	if(Count > 0) Rate /= Count;

	for(i = 0; i < m_Clients.GetSize(); i++)
	{
		if(!m_Clients[i]->IsChoked()) m_Clients[i]->SetMaxUpload(Rate);
	}
}

LRESULT Torrent::DefWindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_NOTIFICATIONTRAY:
		switch(lParam)
		{
		case WM_LBUTTONUP:
			OnNotificationRestore();
			break;

		}
		return 0;
	}

	return CDialog::DefWindowProc(Msg, wParam, lParam);
}

void Torrent::OnNotificationRestore()
{
	Restore();
}

void Torrent::Restore(void)
{
	if(m_InNotification == true)
	{
		Shell_NotifyIcon(NIM_DELETE, &m_NotifyIconData);
		ShowWindow(SW_SHOW);
		if(IsIconic()) ShowWindow(SW_RESTORE);
		m_InNotification = false;
	}else{
		ShowWindow(SW_SHOW);
		if(IsIconic()) ShowWindow(SW_RESTORE);
	}
}

void Torrent::SendStoppedEvent(void)
{
	CString TrackerUrl;
	char Buffer[20];
	WORD Port;
	BYTE * Data;
	DWORD DataLen;
	CString Key;

	if(Settings::GetBool("SendStopOnClose"))
	{
		Port = TorrentDlg::GetListeningPort();

		Key.Format("%.2x%.2x%.2x%.2x", rand() % 256, rand() % 256, rand() % 256, rand() % 256);

		TrackerUrl =
			m_TorrentAnnounce[0] +
			"?info_hash=" + Util::HttpEncode(m_TorrentId, 20) +
			"&peer_id=" + Util::HttpEncode(m_PeerId, 20) +
			"&port=" + itoa(Port, Buffer, 10) +
			"&key=" + Key +
			"&uploaded=" + _i64toa(m_BytesOut, Buffer, 10) +
			"&downloaded=" + _i64toa(m_BytesIn, Buffer, 10) +
			"&left=" + _i64toa(((QWORD)m_TorrentBlockCount - (QWORD)m_CompleteBlockCount) * (QWORD)m_TorrentBlockSize, Buffer, 10) +
			"&compact=1" +
			"&event=stopped";

		Util::ReadHttpFile(TrackerUrl, Data, DataLen);
		MFree(Data);
	}
}

void Torrent::UpdateBlockOrder(bool ForceUpdate)
{
	List<DWORD> Blocks;
	DWORD i, j;
	DWORD BlockStart, BlockEnd;
	DWORD Mode;
	DWORD Rarity;
	//DWORD StartTime;

	Mode = Settings::GetDWord("BlockSelection");

	if(m_LastBlockMode != Mode || ForceUpdate)
	{
		//StartTime = clock();
		m_LastBlockMode = Mode;
		if(Mode == 0)	// Rarest
		{
			//TRACE("Rarest\n");
			while(m_IncompleteBlockQueue.GetSize())
			{
				j = 0;
				Rarity = 5000;
				for(i = 0; i < m_IncompleteBlockQueue.GetSize(); i++)
				{
					if(m_Rarity[m_IncompleteBlockQueue[i]] < Rarity)
					{
						j = i;
						Rarity = m_Rarity[m_IncompleteBlockQueue[i]];
					}
				}

				Blocks.PushBack(m_IncompleteBlockQueue[j]);
				m_IncompleteBlockQueue.Delete(j);
			}

			m_IncompleteBlockQueue = Blocks;
			Blocks.DeleteAll();
		}else

		if(Mode == 1)	// Random
		{
			//TRACE("Random\n");
			while(m_IncompleteBlockQueue.GetSize())
			{
				i = rand() % m_IncompleteBlockQueue.GetSize();
				Blocks.PushBack(m_IncompleteBlockQueue[i]);
				m_IncompleteBlockQueue.Delete(i);
			}

			m_IncompleteBlockQueue = Blocks;
			Blocks.DeleteAll();
		}else

		if(Mode == 2)	// Sequential
		{
			//TRACE("Sequential\n");
			while(m_IncompleteBlockQueue.GetSize())
			{
				Blocks.InsertSorted(m_IncompleteBlockQueue[0], SortBlocksSequential);
				m_IncompleteBlockQueue.Delete(0);
			}

			m_IncompleteBlockQueue = Blocks;
			Blocks.DeleteAll();
		}

		// File Prioritization
		if(m_Prioritize)
		{
			for(i = 0; i < m_TorrentFiles.GetSize(); i++)
			{
				BlockStart = (DWORD)(m_TorrentFiles[m_Priority[i]]->m_FileStartPos / m_TorrentBlockSize);
				BlockEnd = (DWORD)((m_TorrentFiles[m_Priority[i]]->m_FileStartPos + m_TorrentFiles[m_Priority[i]]->m_FileSize) / m_TorrentBlockSize);

				for(j = 0; j < m_IncompleteBlockQueue.GetSize(); j++)
				{
					if(m_IncompleteBlockQueue[j] >= BlockStart && m_IncompleteBlockQueue[j] <= BlockEnd)
					{
						Blocks.PushBack(m_IncompleteBlockQueue[j]);
						m_IncompleteBlockQueue.Delete(j);
						j--;
					}
				}
			}
			
			m_IncompleteBlockQueue = Blocks;
		}

		//TRACE("Block Ordering took %.4f seconds.\n", (clock() - StartTime) / 1000.0f);

		//for(i = 0; i < m_IncompleteBlockQueue.GetSize(); i++)
		//{
		//	TRACE("%d - %d\n", m_IncompleteBlockQueue[i], m_Rarity[m_IncompleteBlockQueue[i]]);
		//}
	}
}

int Torrent::SortBlocksSequential(DWORD a, DWORD b)
{
	if(a > b) return -1;
	if(a < b) return 1;
	return 0;
}

float Torrent::GetSeedRatio(void)
{
	if(m_ScrapeIncomplete == 0)
	{
		return 100.0f;
	}

	return (float)m_ScrapeComplete / (float)m_ScrapeIncomplete;
}

QWORD Torrent::CalculateBytesLeft(void)
{
	DWORD i;
	DWORD j;

	m_BytesLeft = 0;

	for(i = 0; i < m_TorrentBlockCount; i++)
	{
		if(GetBlockState(i) != BLOCKSTATE_COMPLETE)
		{
			for(j = 0; j < GetBlockPieceCount(i); j++)
			{
				if(GetPieceState(i, j) != BLOCKSTATE_COMPLETE)
				{
					m_BytesLeft += GetPieceSize(i, j);
				}
			}
		}
	}

	return m_BytesLeft;
}

void Torrent::SetActiveFlags(DWORD f)
{
	
}

void Torrent::CompleteAction(void)
{
	m_Parent->GetDlgItem(ID_CANCEL)->SetWindowText(_T("Close"));
	CloseHandle(m_TorrentFiles[0]->m_hFile);
	rename(m_TmpFilePath, m_TorrentPath +m_TorrentFiles[0]->m_FilePath);

	if(m_Parent->m_AutoExec.GetCheck())
	{
		::WinExec(m_TorrentPath + m_TorrentName, SW_SHOWNORMAL);
	}
	else
	{
		if(MessageBox("Execute Download File?","Auto Execution", MB_YESNO) == IDYES)
		{
			::WinExec(m_TorrentPath + m_TorrentName, SW_SHOWNORMAL);
		}
	}
	m_Parent->ShowWindow(SW_MINIMIZE);
}
