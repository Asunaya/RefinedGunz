#pragma once

// TorrentDlg
#include "torrent.h"
//#include "stickydlg.h"
#include "upnp.h"
#include "afxcmn.h"
#include "afxwin.h"

/**		메인 윈도우
*
*		@author soyokaze
*		@date   2005-11-02
*/

class TorrentDlg : public CDialog
{
	DECLARE_DYNAMIC(TorrentDlg)

public:
	TorrentDlg(CWnd* pParent = NULL);
	virtual ~TorrentDlg();

	enum { IDD = IDD_DOWNLOADER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	void KillTorrent(Torrent * t);

	void CheckScrapes(void);	// Makes a Download Check

	static WORD GetListeningPort(void);

	void LoadFile();

	void NotificationAlert(const char * Message);

	// Messages
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnFileExit();
	afx_msg void OnFileOpen();
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnNotificationExit();
	afx_msg void OnNotificationRestore();
	afx_msg void OnTorrentPause();
	afx_msg void OnTorrentResume();
	afx_msg void OnConnectioninfo();
	DECLARE_MESSAGE_MAP()

protected:

	// Torrent Connection Retrieval
	static void _OnTorrentAccept(DataSocket * s, DWORD_PTR Custom, int ErrorCode);
	static void _OnTorrentClose(DataSocket * s, DWORD_PTR Custom, int ErrorCode);
	static void _OnTorrentReceive(DataSocket * s, DWORD_PTR Custom, int ErrorCode);
	void OnTorrentAccept(DataSocket * s, int ErrorCode);
	void OnTorrentClose(DataSocket * s, int ErrorCode);
	void OnTorrentReceive(DataSocket * s, int ErrorCode);

	// Other
	void Update(void);
	Torrent * ItemToTorrent(ListItem Item);
	void ActivateTorrents(void);
	static int SortCompleteTorrents(Torrent * a, Torrent * b);


	// Messages
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();

	// Scrape
	static DWORD WINAPI ProcessScrapesThread(void * Parameter);
	void ProcessScrapes(void);

	HANDLE m_ProcessScrapesThread;

	List<Torrent *> m_Torrents;
	
	DWORD m_NextScrapeCheck;

	DataSocket m_ListeningPort;

	List<DataSocket *> m_PendingConnections;

	static WORD m_Port;
	static UPnP m_UPnP;

	NOTIFYICONDATA m_NotifyIconData;
	bool m_InNotification;

	List<DWORD> m_Priority;

	CMenu m_MainMenu;
public:

	CProgressCtrl m_Progress;
	CStatic m_Downloaded;
	CStatic m_DownloadedRate;
	CStatic m_EstimatedTime;
	CStatic m_Path;
	CStatic m_ProgressText;
	CStatic m_Status;
	CStatic m_Uploaded;
	CStatic m_UploadedRate;
	CButton m_AutoExec;
};