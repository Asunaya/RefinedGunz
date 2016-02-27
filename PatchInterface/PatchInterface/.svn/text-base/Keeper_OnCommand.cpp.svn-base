#include "StdAfx.h"
#include ".\keeper.h"
#include "MErrorTable.h"
#include "MCommand.h"
#include "MCommandManager.h"
#include "MSharedCommandTable.h"
#include "MCommandBuilder.h"
#include "ReportCtrl.h"



bool CKeeper::OnCommand( MCommand* pCommand )
{
	if( MClient::OnCommand(pCommand) )
		return true;

	switch( pCommand->GetID() )
	{
	case MC_RESPONSE_KEEPER_MANAGER_CONNECT :
		{
			OnResponseConnectKeeperManager( pCommand->GetSenderUID() );
		}
		break;
		
	case MC_CHECK_KEEPER_MANAGER_PING :
		{
			OnRecvPing( pCommand );
			return true;
		}

	case MC_RESPONSE_KEEPER_CONNECT_MATCHSERVER :
		{
			MUID uidKeeper;

			pCommand->GetParameter( &uidKeeper, 0, MPT_UID );

			OnResponseKeeperConnectMatchServer( uidKeeper );
		}
		break;

	case MC_RESPONSE_CONNECTION_STATE :
		{
			CONNECTION_STATE nServerState;

			pCommand->GetParameter( &nServerState, 0, MPT_INT );

			OnResponseConnectionState( nServerState );
		}
		break;
		
	case MC_RESPONSE_CONFIG_STATE :
		{
			CONFIG_STATE nServerConfig;
			CONFIG_STATE nAgentConfig;
			CONFIG_STATE nDownloadConfig;
			CONFIG_STATE nPrepareConfig;
			CONFIG_STATE nPatchConfig;

			pCommand->GetParameter( &nServerConfig, 0, MPT_INT );
			pCommand->GetParameter( &nAgentConfig, 1, MPT_INT );
			pCommand->GetParameter( &nDownloadConfig, 2, MPT_INT );
			pCommand->GetParameter( &nPrepareConfig, 3, MPT_INT );
			pCommand->GetParameter( &nPatchConfig, 4, MPT_INT );

			OnResponseConfigState( nServerConfig, nAgentConfig, nDownloadConfig, nPrepareConfig, nPatchConfig );
		}
		break;

	case MC_RESPONSE_SET_ONE_CONFIG :
		{
			COLUMN_ID		nColumnID;
			CONFIG_STATE	nConfigState;

			pCommand->GetParameter( &nColumnID, 0, MPT_INT );
			pCommand->GetParameter( &nConfigState, 1, MPT_INT );

			OnResponseSetOneConfig( nColumnID, nConfigState );
		}
		break;

	case MC_RESPONSE_LAST_JOB_STATE :
		{
			JOB_STATE nJob;
			JOB_STATE nState;

			pCommand->GetParameter( &nJob, 0, MPT_INT );
			pCommand->GetParameter( &nState, 1, MPT_INT );

			OnResponseLastJobState( nJob, nState );
		}
		break;

	case MC_RESPONSE_ANNOUNCE_STOP_SERVER :
		{
		}
		break;

	case MC_RESPONSE_SERVER_AGENT_STATE :
		{
			RUN_STATE ServerRunState;
			RUN_STATE AgentRunState;

			pCommand->GetParameter( &ServerRunState, 0, MPT_INT );
			pCommand->GetParameter( &AgentRunState, 1, MPT_INT );

			OnResponseServerAgentState( ServerRunState, AgentRunState );
		}
		break;

	case MC_RESPONSE_SERVER_STATUS :
		{
			SERVER_ERR_STATE	ServerErrState;
			char				szServerResVer[ 128 ]	= {0,};
			char				szServerLastMod[ 128 ]	= {0,};
			char				szAgentLastMod[ 128 ]	= {0,};
			char				szKeeperLastMod[ 128 ]	= {0,};
			ULONGLONG			nServerFileSize			= 0;
			ULONGLONG			nAgentFileSize			= 0;
			ULONGLONG			nKeeperFileSize			= 0;
			unsigned char		nAgentCount				= 0;

			pCommand->GetParameter( &ServerErrState, 0, MPT_INT );
			pCommand->GetParameter( szServerResVer, 1, MPT_STR, 127 );
			pCommand->GetParameter( szServerLastMod, 2, MPT_STR, 127 );
			pCommand->GetParameter( szAgentLastMod, 3, MPT_STR, 127 );
			pCommand->GetParameter( szKeeperLastMod, 4, MPT_STR, 127 );
			pCommand->GetParameter( &nServerFileSize, 5, MPT_UINT64 );
			pCommand->GetParameter( &nAgentFileSize, 6, MPT_UINT64 );
			// pCommand->GetParameter( &nKeeperFileSize, 7, MPT_UINT64 ); // °Ç³Ê¶Ú°ÍÀÓ. ¾ø´Â°Ô ¾Æ´Ô!!
			pCommand->GetParameter( &nAgentCount, 8, MPT_UCHAR );
			

			OnResponseServerStatus( ServerErrState, 
									szServerResVer, 
									szServerLastMod, 
									szAgentLastMod, 
									szKeeperLastMod, 
									nServerFileSize, 
									nAgentFileSize, 
									nKeeperFileSize,
									nAgentCount );
		}
		break;

	case MC_RESPONSE_WRITE_CLIENT_CRC :
		{
		}
		break;

	case MC_RESPONSE_KEEPER_ADD_HASHMAP :
		{
			bool bRes;

			pCommand->GetParameter( &bRes, 0, MPT_BOOL );

			OnResponseKeeperAddHashMap( pCommand->GetSenderUID(), bRes );
		}
		break;

	default :
		{
			ASSERT( 0 );
		}
	}

	return true;
}