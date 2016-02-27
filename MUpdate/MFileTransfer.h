#pragma once


#include <afxinet.h>
#include "MSafeThread.h"
#include "MFTCmd.h"


class MFTListener;
class MFTNotify;


//// Session for MFileTransfer  ///////////////////////
class MFTSession : public CInternetSession {
public:
	MFTSession(LPCTSTR pszAppName = NULL, int nMethod = PRE_CONFIG_INTERNET_ACCESS);
	virtual void OnStatusCallback(DWORD dwContext, DWORD dwInternetStatus,
					LPVOID lpvStatusInfomration, DWORD dwStatusInformationLen);
};


//// MFileTransfer  ///////////////////////////////////
class MFileTransfer : public MSafeThread {
protected:
	bool					m_bShutdown;

	MFTSession				m_INetSession;
	CFtpConnection*			m_pConnection;

	CRITICAL_SECTION		m_csCmdQueueLock;
	MFTCmdList				m_CommandList;
	MFTListener*			m_pListener;
	unsigned long			m_nProgressContext;

protected:
	void LockCmdQueue()					{ EnterCriticalSection(&m_csCmdQueueLock); }
	void UnlockCmdQueue()				{ LeaveCriticalSection(&m_csCmdQueueLock); }

	CFtpConnection* GetConnection()		{ return m_pConnection; }
	MFTListener* GetListener()			{ return m_pListener; }

	void SetProgressContext(unsigned long nContext)	{ m_nProgressContext = nContext; }
	unsigned long GetProgressContext()	{ return m_nProgressContext; }
	void Notify(MFTNotify* pNotify);
	void NotifyProgress(const char* pszRemoteFile, DWORD dwRead, DWORD dwReadSum, DWORD dwLength);

	bool GetFileEx(const char* pszRemoteFile, const char* pszLocalFile);

	virtual void OnRun();

public:
	MFileTransfer();
	virtual ~MFileTransfer();
	virtual void Create(MFTListener* pListener);
	virtual void Destroy();

	bool OpenConnection(const char* pszAddress, unsigned long nPort, const char* pszID, const char* pszPassword);
	void CloseConnection();

	void PostCommand(MFTCmd* pCmd);

	bool ChangeDir(const char* pszDir);
	bool ListFiles();
	bool DownloadFile(const char* pszRemoteFileName, const char* pszLocalFileName);
};
