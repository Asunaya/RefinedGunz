#pragma once

#include "Keeper.h"
// #include "ServerKeeperConst.h"

#include <vector>
using std::vector;

#define KEEPERMANAGER_UID MUID(0, 3)

static const char KEEPER_LIST_FILE[] = ".\\KeeperList.txt";

typedef vector< CKeeper* >	KeeperVec;
typedef KeeperVec::iterator KeeperVecIter;

class CScheduleInfo;

class CKeeperManager
{
public:
	CKeeperManager(void);
	~CKeeperManager(void);

	KeeperVec& GetKeeperObjs() { return m_vKeeper; }

	const CKeeper* GetKeeperByPos( const int nPos ) const;
	const int GetSize() const { return static_cast<int>(m_vKeeper.size()); }

	void SetPort( const int nPort ) { m_nPort = nPort; }

	bool IsCheckedKeeper( const int nKeeperID );

	void UpdateSertverStatusReport( CListCtrl* pReport );

	bool LoadKeeperList( const char* pszFileName );
	bool InitKeeperObjs();
	void Connect();
	void Disconnect();
	void DeleteKeeperObjs();
	void Release();
	void Update();

	void RequestKeeperConnectMatchServer();
	void RequestConnectionState();
	void RequestRefreshServerList();
	void RequestLastJobState();
	void RequestAnnounce( const char* pszAnnounce, const int nLen );
	void RequestResetPatch();
	void RequestDisconnectServer();
	void RequestRebootWindows();
	void RequestStopServerWithAnnounce();
	void RequestKeeperManagerSchedule( const CScheduleInfo* pScheduleInfo );
	void RequestWriteClientCRC( const DWORD dwClientCRC );
	void RequestReloadServerConfig( const string& strReloadFileList );
	void RequestAddHashMap( const string& strNewHashValue );

	/// MatchServer
	void RequestStartServer();
	void RequestStopServer();
	void RequestPrepareServerPatch();
	void RequestDownloadServerPatchFile();
	void RequestServerPatch();
	///

	/// MatchServer
	void RequestStopAgentServer();
	void RequestStartAgentServer();
	void RequestPrepareAgentPatch();
	void RequestAgentPatch();
	void RequestDownloadAgentPatchFile();
	///

	const CKeeper* operator[] ( const unsigned int nPos ) const { return m_vKeeper[ nPos ]; }

	static CKeeperManager& GetInst()
	{
		static CKeeperManager KeeperManager;
		return KeeperManager;
	}

private :
	KeeperVec	m_vKeeper;

	int m_nPort;
};

#define GetKeeperMgr CKeeperManager::GetInst()