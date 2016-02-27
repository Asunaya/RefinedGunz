#pragma once

#include "MUID.h"
#include "MCommandManager.h"


class MCommand;
class MCommandManager;
class MLocatorDBMgr;
class MSafeUDP;
class MServerStatusMgr;
class MLocatorUDPInfo;
class MUDPManager;
class MCountryFilter;

struct MPacketHeader;

class MLocator
{
public:
	MLocator(void);
	virtual ~MLocator(void);

	bool Create();
	void Destroy();

	bool IsBlocker( const DWORD dwIPKey, const DWORD dwEventTime );
	bool IsDuplicatedUDP( const DWORD dwIPKey, 
						  MUDPManager& rfCheckUDPManager, 
						  const DWORD dwEventTime	);

	void IncreaseRecvCount()		{ ++m_nRecvCount; }
	void IncreaseSendCount()		{ ++m_nSendCount; }
	void IncreaseDuplicatedCount()	{ ++m_nDuplicatedCount; }

	const MServerStatusMgr* GetServerStatusMgr() const { return m_pServerStatusMgr; }

	void DumpLocatorStatusInfo();
	
	void Run();

public :
	/// 커맨드 매니져 얻기
	MCommandManager* GetCommandManager(void) { return &m_CommandManager; }
	MCommand* GetCommandSafe() { return m_CommandManager.GetCommand(); }

	MCommand* CreateCommand(int nCmdID, const MUID& TargetUID);

#ifdef _DEBUG
	void TestDo();
	void InitDebug();
	void DebugOutput( void* vp );
#endif

private :
	bool InitDBMgr();
	bool InitSafeUDP();
	bool InitServerStatusMgr();
	bool InitUDPManager();
	bool InitCountryCodeFilter();
	
	MLocatorDBMgr* GetLocatorDBMgr() { return m_pDBMgr; }

	void  GetDBServerStatus( const DWORD dwEventTime, const bool bIsWithoutDelayUpdate = false );
	const DWORD GetUpdatedServerStatusTime()		{ return m_dwLastServerStatusUpdatedTime; }
	const DWORD GetLastUDPManagerUpdateTime()		{ return m_dwLastUDPManagerUpdateTime; }
	const DWORD GetLastLocatorStatusUpdatedTime()	{ return m_dwLastLocatorStatusUpdatedTime; }

	MUDPManager& GetRecvUDPManager()  { return *m_pRecvUDPManager; }
	MUDPManager& GetSendUDPManager()  { return *m_pSendUDPManager; }
	MUDPManager& GetBlockUDPManager() { return *m_pBlockUDPManager; }

	const DWORD GetRecvCount() const		{ return m_nRecvCount; }
	const DWORD GetSendCount() const		{ return m_nSendCount; }
	const DWORD GetDuplicatedCount() const	{ return m_nDuplicatedCount; }

	void ResetRecvCount()		{ m_nRecvCount = 0; }
	void ResetSendCount()		{ m_nSendCount = 0; }
	void ResetDuplicatedCount()	{ m_nDuplicatedCount = 0; }

	MCountryFilter* GetCountryFilter() { return m_pCountryFilter; }

	void ReleaseDBMgr();
	void ReleaseSafeUDP();
	void ReleaseServerStatusMgr();
	void ReleaseServerStatusInfoBlob();
	void ReleaseUDPManager();
	void ReleaseValidCountryCodeList();
	void ReleaseCommand();

	bool IsElapedServerStatusUpdatedTime( const DWORD dwEventTime );
	void UpdateLastServerStatusUpdatedTime( const DWORD dwTime )	{ m_dwLastServerStatusUpdatedTime = dwTime; }
	void UpdateLastUDPManagerUpdateTime( const DWORD dwTime )		{ m_dwLastUDPManagerUpdateTime = dwTime; }
	void UpdateLastLocatorStatusUpdatedTime( const DWORD dwTime )	{ m_dwLastLocatorStatusUpdatedTime = dwTime; }
	
	void ParseUDPPacket( char* pData, 
						 MPacketHeader* pPacketHeader, 
						 DWORD dwIP, 
						 unsigned int nPort);
	void PostSafeCommand( MCommand* pCmd );

	void CommandQueueLock()		{ EnterCriticalSection( &m_csCommandQueueLock ); }
	void CommandQueueUnlock()	{ LeaveCriticalSection( &m_csCommandQueueLock ); }

	void ResponseServerStatusInfoList( DWORD dwIP, int nPort );
	void ResponseBlockCountryCodeIP( DWORD dwIP, 
									 int nPort, 
									 const string& strCountryCode, 
									 const string& strRoutingURL );

	bool IsLiveUDP( const MLocatorUDPInfo* pRecvUDPInfo, const DWORD dwEventTime );
	bool IskLIveBlockUDP( const MLocatorUDPInfo* pBlkRecvUDPInfo, const DWORD dwEventTime );
	bool IsOverflowedNormalUseCount( const MLocatorUDPInfo* pRecvUDPInfo );

	const int	MakeCmdPacket( char* pOutPacket, const int nMaxSize, MCommand* pCmd );
	void		SendCommandByUDP( DWORD dwIP, int nPort, MCommand* pCmd );
	
	void UpdateUDPManager( const DWORD dwEventTime );
	void FlushRecvQueue( const DWORD dwEventTime );
	void UpdateLocatorStatus( const DWORD dwEventTime );
	void UpdateLocatorLog( const DWORD dwEventTime );
	void UpdateCountryCodeFilter( const DWORD dwEventTime );
	void UpdateLogManager();

	bool GetCustomIP( const string& strIP, string& strOutCountryCode, bool& bIsBlock, string& strOutComment );

	bool IsValidCountryCodeIP( const string& strIP, 
							   string& strOutCountryCode, 
							   string& strOutRoutingURL );

	void OnRegisterCommand(MCommandManager* pCommandManager);

	static bool UDPSocketRecvEvent( DWORD dwIP, 
									WORD wRawPort, 
									char* pPacket, 
									DWORD dwSize );

	void DeleteCountryFilter();

private :
	MCommandManager	m_CommandManager;	///< 커맨드 매니저
	MUID			m_This;				///< 자기 커뮤니케이터 UID

	MSafeUDP*			m_pSafeUDP;
	MServerStatusMgr*	m_pServerStatusMgr;

	DWORD m_dwLastServerStatusUpdatedTime;
	DWORD m_dwLastLocatorStatusUpdatedTime;

	CRITICAL_SECTION m_csCommandQueueLock;

	MUDPManager* m_pRecvUDPManager;
	MUDPManager* m_pSendUDPManager;
	MUDPManager* m_pBlockUDPManager;
	DWORD		 m_dwLastUDPManagerUpdateTime;

	DWORD m_nRecvCount;
	DWORD m_nSendCount;
	DWORD m_nDuplicatedCount;

	MLocatorDBMgr*	m_pDBMgr;
	MCountryFilter* m_pCountryFilter;

	void*		m_vpServerStatusInfoBlob;
	int			m_nLastGetServerStatusCount;
	int			m_nServerStatusInfoBlobSize;
};