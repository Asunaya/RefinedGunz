#include "StdAfx.h"
#include ".\keeper.h"
#include "MErrorTable.h"
#include "MCommand.h"
#include "MCommandManager.h"
#include "MSharedCommandTable.h"
#include "MCommandBuilder.h"
#include "ReportCtrl.h"



void CKeeper::OnRecvPing( MCommand* pCommand )
{
	if( 0 == pCommand )
		return;

	MCommand* pCmd = new MCommand( MC_CHECK_KEEPER_MANAGER_PING, GetUID(), GetServerUID(), GetCommandManager() );
	if( 0 == pCmd )
		return;

	Post( pCmd );

	SetLastRecvCmdTime();

#ifdef _DEBUG
	mlog( "CKeeper::OnRecvPing - ID:%d\n", m_nID );
#endif
}



void CKeeper::OnResponseConnectKeeperManager( MUID& uidMyUID )
{
	ASSERT( uidMyUID.IsValid() );

	m_bIsConnected = true;

	SetLastRecvCmdTime();
}



int CKeeper::OnConnected(SOCKET sock, MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp)
{
	m_This = *pAllocUID;

	if( MOK == MClient::OnConnected(sock, pTargetUID, pAllocUID, nTimeStamp) )
	{
		if( PostConnect() )
			return  MOK;
	}

	return MERR_UNKNOWN;
}


void CKeeper::OnResponseKeeperConnectMatchServer( const MUID& uidKeeper )
{
	m_Server = uidKeeper;

	MCommandBuilder* pCmdBuilder = GetCommandBuilder();
	if( 0 == pCmdBuilder )
	{
		ASSERT( 0 );
	}

	pCmdBuilder->SetUID( GetUID(), uidKeeper );

	SetDefaultReceiver( uidKeeper );
}


void CKeeper::OnResponseConnectionState( const CONNECTION_STATE nServerState )
{
	SetConnectionState( nServerState );
}


void CKeeper::OnResponseLastJobState( const JOB_STATE nJob, const JOB_STATE nResult )
{
	SetLastJobState( nResult );
}


void CKeeper::OnResponseConfigState( const CONFIG_STATE nServerConfig, const CONFIG_STATE nAgentConfig, const CONFIG_STATE nDownloadConfig,
									 const CONFIG_STATE nPrepareConfig, const CONFIG_STATE nPatchConfig )
{
	SetConfigState( COLUMN_SERVER_START, nServerConfig );
	SetConfigState( COLUMN_AGENT_START, nAgentConfig );
	SetConfigState( COLUMN_DOWNLOAD, nDownloadConfig );
	SetConfigState( COLUMN_PREPARE, nPrepareConfig );
	SetConfigState( COLUMN_PATCH, nPatchConfig );
}


void CKeeper::OnResponseSetOneConfig( const COLUMN_ID nColumnID, const CONFIG_STATE nConfigState )
{
	if( (nColumnID < COLUMN_END ) && (nConfigState < CONFIG_END) )
	{
		SetConfigState( nColumnID, nConfigState );
	}
}


void CKeeper::OnResponseServerStopWithAnnounce()
{
	SetLastJobState( JOB_ANNOUNCE_STOP_SERVER );
}


void CKeeper::OnResponseServerAgentState( const RUN_STATE ServerRunState, const RUN_STATE AgentRunState )
{
	SetServerRunState( ServerRunState );
	SetAgentRunState( AgentRunState );
}


void CKeeper::OnResponseServerStatus( const SERVER_ERR_STATE ServerErrState, 
									  const char* pszServerResVer, 
									  const char* pszServerLastMod, 
									  const char* pszAgentLastMod,
									  const char* pszKeeperLastMod,
									  const ULONGLONG nServerFileSize,
									  const ULONGLONG nAgentFileSize,
									  const ULONGLONG nKeeperFileSize,
									  const unsigned char nAgentCount )
{
	SetServerErrState( ServerErrState );

	m_strServerResVer	= pszServerResVer;
	m_strServerLastMod	= pszServerLastMod;
	m_strAgentLastMod	= pszAgentLastMod;
	m_strKeeperLastMod	= pszKeeperLastMod;
	m_nServerFileSize	= nServerFileSize;
	m_nAgentFileSize	= nAgentFileSize;
	m_nKeeperFileSize	= nKeeperFileSize;
	m_nAgentCount		= nAgentCount;
}


void CKeeper::OnResponseKeeperAddHashMap( const MUID& ServerUID, const bool bRes )
{
	if( bRes )
	{
		mlog( "add hash ok\n" );
	}
}