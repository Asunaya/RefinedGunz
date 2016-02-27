#pragma once

#include "client.h"
//#include "stickydlg.h"
#include "connectionInfo.h"
#include "accrue.h"
#include "afxcmn.h"

// Torrent

#define PIECESIZE	(16 * 1024)

#define BLOCKSTATE_UNKNOWN		0
#define BLOCKSTATE_INCOMPLETE	1
#define BLOCKSTATE_DOWNLOADING	2
#define BLOCKSTATE_COMPLETE		3

#define BLOCKCOLOR_COMPLETE_MIN				RGB(0x00, 0x40, 0x00)
#define BLOCKCOLOR_COMPLETE_MAX				RGB(0xC0, 0xFF, 0xC0)
#define BLOCKCOLOR_INCOMPLETE				RGB(0xFF, 0x80, 0x80)
#define BLOCKCOLOR_INCOMPLETE_MIN			RGB(0x40, 0x00, 0x40)
#define BLOCKCOLOR_INCOMPLETE_MAX			RGB(0xFF, 0xC0, 0xFF)
#define BLOCKCOLOR_DOWNLOADING_TOP_MIN		BLOCKCOLOR_INCOMPLETE_MIN
#define BLOCKCOLOR_DOWNLOADING_TOP_MAX		BLOCKCOLOR_INCOMPLETE_MAX
#define BLOCKCOLOR_DOWNLOADING_MIDDLE		RGB(0x80, 0x80, 0xFF)
#define BLOCKCOLOR_DOWNLOADING_BOTTOM_MIN	BLOCKCOLOR_COMPLETE_MIN
#define BLOCKCOLOR_DOWNLOADING_BOTTOM_MAX	BLOCKCOLOR_COMPLETE_MAX

#define TORRENTSTATE_UNKNOWN	0
#define TORRENTSTATE_WAITING	1
#define TORRENTSTATE_CHECKING	2
#define TORRENTSTATE_READY		3	// Torrent Ready for Downloading

#define TORRENTEVENT_START		0
#define TORRENTEVENT_INCOMPLETE	1
#define TORRENTEVENT_COMPLETE	2

// Torrent List Image Indexes
#define TORRENT_IMG_INACTIVE	0
#define TORRENT_IMG_OK			1
#define TORRENT_IMG_WARNING		2
#define TORRENT_IMG_ERROR		3

class TorrentDlg;

/**		
*
*		@author soyokaze
*		@date   2005-11-02
*/

class Torrent : public CDialog
{
	DECLARE_DYNAMIC(Torrent)
	DECLARE_MESSAGE_MAP()

	struct FileInfo
	{
		FileInfo(void)
		{
			m_hFile = INVALID_HANDLE_VALUE;
			m_Tainted = false;
		}

		CString m_FilePath;
		QWORD m_FileSize;
		QWORD m_FileStartPos;
		HANDLE m_hFile;
		bool m_Tainted;	// true if the file contents may have changed since last loaded
	};

	struct ClientInfo
	{
		CString m_Ip;
		WORD m_Port;
		BYTE m_PeerId[20];
	};

public:
	Torrent();
	virtual ~Torrent();

	enum { IDD = IDD_DUMMY};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	// Interface
	void Pause(void);
	void Resume(void);

	bool OpenInsideTorrent(int nFileID);

	void Activate(void);
	void Deactivate(void);
	void Update(void);
	void UpdateScrape(void);
	void ShuttingDown(void);
	void AttachClient(DataSocket * s);
	static void Log(const char * Format, ...);
	void Restore(void);
	float GetSeedRatio(void);

	// Block States
	void IncrementRarity(DWORD Block);
	void DecrementRarity(DWORD Block);

	void SetBlockState(DWORD Block, BYTE State);
	void SetPieceState(DWORD Block, DWORD Piece, BYTE State);

	BYTE GetBlockState(DWORD Block);
	BYTE GetPieceState(DWORD Block, DWORD Piece);

	void SendHave(DWORD Block);

	// Client Notification
	void QueueClient(Client * c);
	void UnQueueClient(Client * c);

	// Requesting
	DWORD GetIncompleteBlockCount(void);
	DWORD GetIncompleteBlock(DWORD Index);
	void SetIncompleteBlockDownloading(DWORD Block);
	DWORD GetDownloadingBlockCount(void);
	DWORD GetDownloadingBlock(DWORD Block);
	void SetDownloadingBlockComplete(DWORD Block);
	void SetDownloadingBlockIncomplete(DWORD Block);
	DWORD GetCompleteBlockCount(void);

	// Status
	bool IsComplete(void);
	bool IsScrapeUpdated(void);
	bool IsActive(void);
	bool IsPaused(void);

	// Prep
	void SetParent(TorrentDlg * Parent);
	ListItem GetListItem(void);

	// Torrent Info
	DWORD GetBlockCount(void);
	DWORD GetBlocksComplete(void);
	DWORD GetBlockPieceCount(DWORD Block);
	DWORD GetBlockSize(DWORD Block);
	DWORD GetPieceSize(DWORD Block, DWORD Piece);
	BYTE * GetTorrentId(void);
	BYTE * GetPeerId(void);
	CString GetTorrentName(void);

	// Data
	void WriteData(BYTE * Data, DWORD Block, DWORD Offset, DWORD Len);
	void ReadData(BYTE * Data, DWORD Block, DWORD Offset, DWORD Len);
	bool CheckBlock(DWORD Block, bool CheckPieceCount);
	void AddRedundantData(DWORD Bytes);
	void SetMaxUpload(DWORD Rate);
	void SetActiveFlags(DWORD f);

	// Messages
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNotificationRestore();

protected:
	virtual LRESULT DefWindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);
	static int SortClientChokes(Client * a, Client * b);
	void PrimeTorrent(void);
	void UpdateCompleteBlockCount(void);
	void PurgeDeadClients(void);
	void SetFileLastModifiedTime(void);
	bool PrepTorrent();
	void UpdateChokes(void);
	void SendStoppedEvent(void);
	void UpdateBlockOrder(bool ForceUpdate);
	static int SortBlocksSequential(DWORD a, DWORD b);
	QWORD CalculateBytesLeft(void);

	// Checking
	static DWORD WINAPI CheckTorrentThread(void * Parameter);
	void CheckTorrent(void);

	static CCriticalSection m_CheckTorrentLock;
	BYTE * m_CheckBlock;

	// Announce
	static DWORD WINAPI GetClientListThread(void * Parameter);
	void GetClientList(void);

	// Scrape
	static DWORD WINAPI GetScrapeThread(void * Parameter);
	void GetScrape(void);

	DWORD m_ScrapeComplete;
	DWORD m_ScrapeDownloaded;
	DWORD m_ScrapeIncomplete;

	DWORD m_NextScrapeCheck;
	bool m_ScrapeUpdated;

	// Essential Torrent Info
	BYTE m_TorrentId[20];
	BYTE m_PeerId[20];
	BYTE * m_TorrentBlockHash;			// Array of SHA1 Hashes for the Blocks (Every 20 BYTES)
	BYTE * m_TorrentBlockMap;			// Array of States of Pieces
	BYTE * m_TorrentPieceMap;			// Array of States of Pieces
	WORD * m_Rarity;					// Block Rarity Map
	List<CString> m_TorrentAnnounce;
	List<CString> m_TorrentScrape;
	CString m_TorrentName;
	bool m_TorrentComplete;
	DWORD m_TorrentBlockCount;
	DWORD m_TorrentBlockSize;
	QWORD m_TorrentSize;				// Size of All the Files Put together
	DWORD m_TorrentPiecesPerBlock;
	DWORD m_CompleteBlockCount;			// Number of Completed Blocks

	CString m_TorrentPath;
	List<FileInfo *> m_TorrentFiles;

	List<DWORD> m_Priority;
	bool m_Prioritize;

	DWORD m_NextMinClientCheck;

	bool m_Tainted;
	bool m_LoadedFromTorrent;

	DWORD m_State;

	FILETIME m_FileTime;

	ListItem m_hItem;
	TorrentDlg * m_Parent;

	HANDLE m_CheckTorrentThread;
	HANDLE m_GetClientListThread;
	HANDLE m_GetScrapeThread;

	List<ClientInfo> m_PendingClients;
	List<Client *> m_Clients;
	List<Client *> m_InterestedClients;

	bool m_Active;
	bool m_Paused;

	DWORD m_NextChokeUpdate;

	List<DWORD> m_IncompleteBlockQueue;
	List<DWORD> m_DownloadBlockQueue;

	// Byte Counts: Saved in torrent file
	QWORD m_BytesIn;
	QWORD m_BytesOut;
	QWORD m_BytesRedundant;

	Accrue<DWORD> m_AvgBpsIn;
	Accrue<DWORD> m_AvgBpsOut;

	bool m_ShuttingDown;

	DWORD m_TorrentEventState;
	DWORD m_ActivationTime;

	QWORD m_Eta;
	QWORD m_BytesLeft;

	NOTIFYICONDATA m_NotifyIconData;
	bool m_InNotification;

	DWORD m_LastBlockMode;
	DWORD m_NextBlockOrderUpdate;

	CString m_FileName;

	LPSTR lpRes;
	char * Buffer;

	static	CFile m_LogFile;
	CString m_TmpFilePath;

public:
	void CompleteAction(void);

	ConnectionInfo* m_PeerInfo;
};