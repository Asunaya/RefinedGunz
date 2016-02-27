#pragma once


#include "MPatchNode.h"
#include "MFileTransfer.h"
#include "MFTNotify.h"


class MUpdateMgr;
class MUIObserver;


// TODO	/////////////////////////////////
//	OpenConnection
//		Get 'patch.xml'
//		Check file nodes in 'patch.xml'
//		Get updated files
//	CloseConnection
/////////////////////////////////////////


//// Listener for MUpdateMgr //////////////////
class MBasicListener : public MFTListener {
	MUpdateMgr*		m_pUpdateMgr;
protected:
	MUpdateMgr* GetUpdateMgr()			{ return m_pUpdateMgr; }
public:
	MBasicListener(MUpdateMgr* pMgr)	{ m_pUpdateMgr = pMgr; }
	virtual ~MBasicListener()			{ }
	virtual void Notify(MFTNotify* pNotify);
};


//// Update Manager ///////////////////////////
class MUpdateMgr {
private:
	static MUpdateMgr*	m_pInstance;
	bool				m_bShutdown;
	bool				m_bSelfUpdate;

protected:	
	bool				m_bVersionCheck;
	MFileTransfer		m_FileTransfer;
	MBasicListener*		m_pListener;

	MPatchList			m_PatchList;
	int					m_nTotalPatchFileCount;
	int					m_nPatchFileCount;

	DWORD				m_nTotalTransfer;
	DWORD				m_nCurrentTransfer;

	CRITICAL_SECTION	m_csNotifyLock;
	MFTNotifyList		m_NotifyList;

	MUIObserver*		m_pUIObserver;

protected:
	bool LoadPatchInfo(const char* pszFileName);

	void SetSelfUpdate(bool bVal)			{ m_bSelfUpdate = bVal; }
	bool GetSelfUpdate()					{ return m_bSelfUpdate; }

	void LockNotify()						{ EnterCriticalSection(&m_csNotifyLock); }
	void UnlockNotify()						{ LeaveCriticalSection(&m_csNotifyLock); }

//	bool OnCommand(MFTCmd* pCmd);
	void OnNotify(MFTNotify* pNotify);

	bool CheckSelfUpdate(const char* pszFileName);
	bool AnalizePatchInfo(const char* pszFileName);
	void ValidatePatches();
	MPatchNode* FindPatchNode(const char* pszFileName);
	bool UpdatePatchNode(const char* pszFileName);
	
	void SetTotalPatchFileCount(int nVal)	{ m_nTotalPatchFileCount = nVal; }
	void ClearTotalPatchFileCount()			{ m_nTotalPatchFileCount = 0; }

	int UpdatePatchFileCount(int nVal)		{ return (m_nPatchFileCount += nVal); }	// can be minus arg
	void ClearPatchFileCount()				{ m_nPatchFileCount = 0; }

	void UpdateTotalTransfer(DWORD nVal)	{ m_nTotalTransfer += nVal; }
	void ClearTotalTransfer()				{ m_nTotalTransfer = 0; }

	void UpdateCurrentTransfer(DWORD nVal)	{ m_nCurrentTransfer += nVal; }
	void ClearCurrentTransfer()				{ m_nCurrentTransfer = 0; }

public:
	int GetTotalPatchFileCount()			{ return m_nTotalPatchFileCount; }
	int GetPatchFileCount()					{ return m_nPatchFileCount; }
	DWORD GetTotalTransfer()				{ return m_nTotalTransfer; }
	DWORD GetCurrentTransfer()				{ return m_nCurrentTransfer; }

public:
	static MUpdateMgr* GetInstance(void)	{ return m_pInstance; }

	MUpdateMgr();
	virtual ~MUpdateMgr();

	void Create();
	void Destroy();

//	void PostCommand(MFTCmd* pCmd);
	void PostNotify(MFTNotify* pNotify);

	void OnRun();

	void BindUIObserver(MUIObserver* pObserver)	{ m_pUIObserver = pObserver; }
	MUIObserver* GetUIObserver()				{ return m_pUIObserver; }

	bool StartUpdate(const char* pszAddress, unsigned long nPort, const char* pszID, const char* pszPassword);
	bool CheckVersion(unsigned long nVersion);
	void LaunchGunz();
};
