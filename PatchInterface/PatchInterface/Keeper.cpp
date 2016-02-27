#include "StdAfx.h"
#include ".\keeper.h"
#include "MErrorTable.h"
#include "MCommand.h"
#include "MCommandManager.h"
#include "MSharedCommandTable.h"
#include "MCommandBuilder.h"
#include "ReportCtrl.h"


CKeeper::CKeeper(void)
{
}


CKeeper::CKeeper( const int nID, const int nPort )  : m_bIsConnected( false ), m_ConnectionState( CNN_NOT_CONNECTED_KEEPER ), 
	m_dwLastRecvCmdTime( timeGetTime() ), m_LastJobState( JOB_NON ), m_nID( nID ), m_nPort( nPort ), 
	m_ServerRunState( RS_NO ), m_AgentRunState( RS_NO ), m_ServerErrState( SES_NO ), m_nServerFileSize( 0 ), m_nAgentFileSize( 0 ), 
	m_nKeeperFileSize( 0 ), m_nAgentCount( 0 )
{
}


CKeeper::~CKeeper(void)
{
}


bool CKeeper::OneTimeInit()
{
	MAddSharedCommandTable( GetCommandManager(), MSCT_AGENT );
	InitConfigStateColumns();

	return true;
}


bool CKeeper::Connect()
{
	if( IsConnectedKeeper() ) return true;

	SOCKET sock;
	if( MOK != MClient::Connect(&sock, const_cast<char*>(m_strAddr.c_str()), m_nPort) )
	// if( MOK != MClient::Connect(&sock, "222.111.150.82", m_nPort) )
	{
		return false;
	}

	SetConnectionState( CNN_CONNECTING_TO_KEEPER );

	SetLastRecvCmdTime();

	return true;
}


bool CKeeper::PostConnect()
{
	// connect.
	MCommand* pCmd = new MCommand( MC_KEEPER_MANAGER_CONNECT, GetUID(), KEEPERSERVER_UID, GetCommandManager() );
	if( 0 == pCmd )
		return false;

	if( !pCmd->AddParameter(new MCommandParameterInt(11)) )
		return false;

	if( !Post(pCmd) )
		return false;

	return true;
}



void CKeeper::SetConnectionState( const CONNECTION_STATE nState )
{
	if( (0 <= nState) && (nState < CNN_END) )
	{
		m_ConnectionState = nState;

		GetReport.Update( m_nID );
	}
}


void CKeeper::RequestDownloadServerPatchFile()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_DOWNLOAD_SERVER_PATCH_FILE, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		Post( pCmd );
	}
}


void CKeeper::RequestStopServer()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_STOP_SERVER, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		Post( pCmd );
	}
}





// 서버의 현제 동작 상태를 요청함.
void CKeeper::RequestConnectionState()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_CONNECTION_STATE, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		Post( pCmd );
	}
}


void CKeeper::Update()
{
	// 커뮤니케이터.
	Run();

	CheckKeeperHeartbeat();

	GetReport.Update( m_nID );	
}


void CKeeper::CheckKeeperHeartbeat()
{
	if( !IsConnected() )
	{
		if( CNN_NOT_CONNECTED_KEEPER != GetConnectionState() )
			SetConnectionState( CNN_NOT_CONNECTED_KEEPER );
	}
}


bool CKeeper::SetAddr( const char* pszAddr )
{
	if( 0 == pszAddr )
		return false;

	if( ADD_MAX_LEN < strlen(pszAddr) )
		return false;

	m_strAddr = pszAddr;
	return true;
}


bool CKeeper::SetName( const char* pszName )
{
	if( 0 == pszName )
		return false;

	m_strName = pszName;
	return true;
}


void CKeeper::RequestStartServer()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_START_SERVER, GetUID(), GetServerUID(), GetCommandManager() );
		if( !pCmd )
			return;

		Post( pCmd );
	}
}


void CKeeper::SetLastRecvCmdTime( const DWORD dwTime )
{
	 m_dwLastRecvCmdTime = dwTime;
}


void CKeeper::RequestKeeperConnectMatchServer()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_KEEPER_CONNECT_MATCHSERVER, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		Post( pCmd );
	}
}


void CKeeper::RequestRefreshServer()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_REFRESH_SERVER, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		Post( pCmd );
	}
}


void CKeeper::RequestPrepareServerPatch()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_PREPARE_SERVER_PATCH, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		Post( pCmd );
	}
}


void CKeeper::RequestServerPatch()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_SERVER_PATCH, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		Post( pCmd );
	}
}


void CKeeper::SetLastJobState( const JOB_STATE nState )
{
	m_LastJobState = nState;

	GetReport.Update( m_nID );
}


void CKeeper::RequestLastJobState()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_LAST_JOB_STATE, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		Post( pCmd );
	}
}


void CKeeper::RequestStopAgentServer()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_STOP_AGENT_SERVER, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		Post( pCmd );
	}
}


void CKeeper::RequestStartAgentServer()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_START_AGENT_SERVER, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		Post( pCmd );
	}
}


const CONFIG_STATE CKeeper::GetConfigState( const COLUMN_ID nColumnID)
{
	ConfigStateMap::iterator it = m_ConfigStateMap.find( nColumnID );
	if( m_ConfigStateMap.end() != it )
		return it->second;
	return CONFIG_ERROR;
}


void CKeeper::SetConfigState( const COLUMN_ID nColumnID, const CONFIG_STATE nConfigState )
{
	ConfigStateMap::iterator it = m_ConfigStateMap.find( nColumnID );
	if( m_ConfigStateMap.end() != it )
	{
		it->second = nConfigState;
		GetReport.Update( m_nID );
	}
}


void CKeeper::InitConfigStateColumns()
{
	AddColumnConfigState( COLUMN_SERVER_START, CONFIG_NO );
	AddColumnConfigState( COLUMN_AGENT_START, CONFIG_NO );
	AddColumnConfigState( COLUMN_DOWNLOAD, CONFIG_NO );
	AddColumnConfigState( COLUMN_PREPARE, CONFIG_NO );
	AddColumnConfigState( COLUMN_PATCH, CONFIG_NO );
}


void CKeeper::AddColumnConfigState( const COLUMN_ID nColumnID, const CONFIG_STATE nConfigState )
{
	if( nColumnID < CONFIG_END )
	{
		ConfigStateMap::iterator it = m_ConfigStateMap.find( nColumnID );
		if( m_ConfigStateMap.end() == it )
		{
			m_ConfigStateMap.insert( ConfigStateMap::value_type(nColumnID, nConfigState) );
		}
		else
		{
			ASSERT( 0 );
		}
	}
	else
	{
		ASSERT( 0 );
	}
}


void CKeeper::ResetColumnConfigState()
{
	ConfigStateMap::iterator it, end;
	end = m_ConfigStateMap.end();
	for( it = m_ConfigStateMap.begin(); it != end; ++it )
	{
		it->second = CONFIG_NO;
		GetReport.Update( m_nID );
	}
}


void CKeeper::InvertServerConfigState()
{
	if( CONFIG_AUTO_START_SERVER == GetConfigState(COLUMN_SERVER_START) )
	{
		PostSetOneConfig( COLUMN_SERVER_START, CONFIG_MANUAL_START_SERVER );
	}
	else if( CONFIG_MANUAL_START_SERVER == GetConfigState(COLUMN_SERVER_START) )
	{
		PostSetOneConfig( COLUMN_SERVER_START, CONFIG_AUTO_START_SERVER );
	}
	else
	{
		ASSERT( 0 );
	}

	GetReport.Update( m_nID );
}


void CKeeper::InvertAgentConfigState()
{
	if( CONFIG_AUTO_START_AGENT == GetConfigState(COLUMN_AGENT_START) )
	{
		PostSetOneConfig( COLUMN_AGENT_START, CONFIG_MANUAL_START_AGENT );
	}
	else if( CONFIG_MANUAL_START_AGENT == GetConfigState(COLUMN_AGENT_START) )
	{
		PostSetOneConfig( COLUMN_AGENT_START, CONFIG_AUTO_START_AGENT );
	}
	else
	{
		ASSERT( 0 );
	}
}


void CKeeper::InvertDownloadConfigState()
{
	if( CONFIG_OK_DOWNLOAD == GetConfigState(COLUMN_DOWNLOAD) )
	{
		PostSetOneConfig( COLUMN_DOWNLOAD, CONFIG_NO_DOWNLOAD );
	}
	else if( CONFIG_NO_DOWNLOAD == GetConfigState(COLUMN_DOWNLOAD) )
	{
		PostSetOneConfig( COLUMN_DOWNLOAD, CONFIG_OK_DOWNLOAD );
	}
	else
	{
		ASSERT( 0 );
	}
}


void CKeeper::InvertPrepareConfigState()
{
	if( CONFIG_OK_PREPARE == GetConfigState(COLUMN_PREPARE) )
	{
		PostSetOneConfig( COLUMN_PREPARE, CONFIG_NO_PREPARE );
	}
	else if( CONFIG_NO_PREPARE == GetConfigState(COLUMN_PREPARE) )
	{
		PostSetOneConfig( COLUMN_PREPARE, CONFIG_OK_PREPARE );
	}
	else
	{
		ASSERT( 0 );
	}
}


void CKeeper::InvertPatchConfigState()
{
	if( CONFIG_OK_PATCH == GetConfigState(COLUMN_PATCH) )
	{
		PostSetOneConfig( COLUMN_PATCH, CONFIG_NO_PATCH );
	}
	else if( CONFIG_NO_PATCH == GetConfigState(COLUMN_PATCH) )
	{
		PostSetOneConfig( COLUMN_PATCH, CONFIG_OK_PATCH );
	}
	else
	{
		ASSERT( 0 );
	}
}


void CKeeper::PostSetOneConfig( const COLUMN_ID nColumnID, const CONFIG_STATE nConfigState )
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_SET_ONE_CONFIG, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		pCmd->AddParameter( new MCmdParamInt(nColumnID) );
		pCmd->AddParameter( new MCmdParamInt(nConfigState) );

		Post( pCmd );
	}
}


void CKeeper::Disconnect(MUID uid)
{
	m_ClientSocket.SimpleDisconnect();

	m_bIsConnected = false;

	m_ConnectionState  = CNN_NOT_CONNECTED_KEEPER;
	m_LastJobState		= JOB_NON;
	SetServerRunState( RS_NO );
	SetAgentRunState( RS_NO );

	ResetColumnConfigState();
}


void CKeeper::ReleaseAll()
{
	m_ClientSocket.Disconnect();

	m_bIsConnected = false;
}


void CKeeper::RequestAnnounce( const char* pszAnnounce, const int nLen )
{
	if( (0 == pszAnnounce) || (strlen(pszAnnounce) != nLen) || (255 < strlen(pszAnnounce)) )
		return;

	if( IsConnected() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_KEEPERMGR_ANNOUNCE, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		pCmd->AddParameter( new MCmdParamStr(pszAnnounce) );

		Post( pCmd );
	}
}


void CKeeper::RequestPrepareAgentPatch()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_PREPARE_AGENT_PATCH, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		Post( pCmd );
	}
}



void CKeeper::RequestAgentPatch()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_AGENT_PATCH, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		Post( pCmd );
	}
}


void CKeeper::RequestDownloadAgentPatchFile()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_DOWNLOAD_AGENT_PATCH_FILE, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		Post( pCmd );
	}
}


void CKeeper::ReqeustResetPatch()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_RESET_PATCH, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		Post( pCmd );
	}
}


void CKeeper::RequestDisconnectServer()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_DISCONNECT_SERVER, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		Post( pCmd );
	}
}


void CKeeper::RequestRebootWindows()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_REBOOT_WINDOWS, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		Post( pCmd );
	}
}


void CKeeper::RequestServerStopWithAnnounce()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_ANNOUNCE_STOP_SERVER, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 == pCmd )
			return;

		Post( pCmd );
	}
}

// 커맨드 생성에 필요한 정보 구조를 위해서.
class CScheduleInfo
{
public :
	CScheduleInfo( const int nType, const int nYear, const int nMonth, 
		const int nDay, const int nHour, const int nMin, const int nCount, 
		const int nCmd, const string& strAnnounce ) :
	m_nType( nType ), m_nYear( nYear ), m_nMonth( nMonth ), m_nDay( nDay ), m_nHour( nHour ),
		m_nMin( nMin ), m_nCount( nCount ), m_nCmd( nCmd ), m_strAnnounce( strAnnounce ) 
	{
	}

	int		m_nType;
	int		m_nYear;
	int		m_nMonth;
	int		m_nDay;
	int		m_nHour;
	int		m_nMin;
	int		m_nCount;
	int		m_nCmd;
	string	m_strAnnounce;

private :
	CScheduleInfo() {}
};


void CKeeper::RequestSchedule( const CScheduleInfo* pScheduleInfo )
{
	if( (0 != pScheduleInfo) && IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_KEEPER_MANAGER_SCHEDULE, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 != pCmd )
		{
			pCmd->AddParameter( new MCmdParamInt(pScheduleInfo->m_nType) );
			pCmd->AddParameter( new MCmdParamInt(pScheduleInfo->m_nYear) );
			pCmd->AddParameter( new MCmdParamInt(pScheduleInfo->m_nMonth) );
			pCmd->AddParameter( new MCmdParamInt(pScheduleInfo->m_nDay) );
			pCmd->AddParameter( new MCmdParamInt(pScheduleInfo->m_nHour) );
			pCmd->AddParameter( new MCmdParamInt(pScheduleInfo->m_nMin) );
			pCmd->AddParameter( new MCmdParamInt(pScheduleInfo->m_nCount) );
			pCmd->AddParameter( new MCmdParamInt(pScheduleInfo->m_nCmd) );
			pCmd->AddParameter( new MCmdParamStr(pScheduleInfo->m_strAnnounce.c_str()) );

			Post( pCmd );
		}
	}
}


void CKeeper::ReqeustServerAgentState()
{
	if( IsConnectedKeeper() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_SERVER_AGENT_STATE, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 != pCmd )
		{
			Post( pCmd );
		}
	}
}


void CKeeper::SetServerRunState( const RUN_STATE ServerRunState	)
{
	m_ServerRunState = ServerRunState;

	GetReport.Update( m_nID );
}


void CKeeper::SetAgentRunState( const RUN_STATE AgentRunState )
{
	m_AgentRunState = AgentRunState;

	GetReport.Update( m_nID );
}


void CKeeper::SetServerErrState( const SERVER_ERR_STATE ServerState )
{
	if( (SES_END <= ServerState) && (ServerState < SES_END) )
	{
		m_ServerErrState = ServerState;

		GetReport.Update( m_nID );
	}
}


void CKeeper::SendCommand( MCommand* pCommand )
{
	if( 0 == pCommand )
		return;

	/// MMatchClient용으로 만들어 놓은 함수를 복사해 놓은것.
	static unsigned char nSerial = 0;
	nSerial++;
	pCommand->m_nSerialNumber = nSerial;
	///

	// MakeTCPCommandSerialNumber(pCommand);
	MClient::SendCommand( pCommand );
}


void CKeeper::RequestWriteClientCRC( const DWORD dwClientCRC )
{
	if( 0 < dwClientCRC )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_WRITE_CLIENT_CRC, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 != pCmd )
		{
			pCmd->AddParameter( new MCmdParamUInt(dwClientCRC) );

			Post( pCmd );
		}
	}
}


void CKeeper::OnResponseWriteClientCRC( const bool bResult )
{
	ASSERT( 0 );
}


bool CKeeper::OnSockDisconnect(SOCKET sock)
{
	Disconnect( MUID(0, 0) );	
	return true;
}


void CKeeper::RequestReloadServerConfig( const string& strReloadFileList )
{
	if( strReloadFileList.empty() )
		return;

	MCommand* pCmd = new MCommand( MC_REQUEST_KEEPER_RELOAD_SERVER_CONFIG, GetUID(), GetServerUID(), GetCommandManager() );
	if( 0 != pCmd )
	{
		pCmd->AddParameter( new MCmdParamStr(strReloadFileList.c_str()) );
		Post( pCmd );
	}
}


void CKeeper::RequestAddHashMap( const string& strNewHashMap )
{
	if( !strNewHashMap.empty() )
	{
		MCommand* pCmd = new MCommand( MC_REQUEST_KEEPER_ADD_HASHMAP, GetUID(), GetServerUID(), GetCommandManager() );
		if( 0 != pCmd )
		{
			pCmd->AddParameter( new MCmdParamStr(strNewHashMap.c_str()) );
			Post( pCmd );
		}
	}
}