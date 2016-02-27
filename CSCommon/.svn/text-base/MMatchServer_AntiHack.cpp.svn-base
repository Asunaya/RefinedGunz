#include "stdafx.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MMatchObject.h"
#include "Msg.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MDebug.h"
#include "MMatchAuth.h"
#include "MAsyncDBJob.h"
#include "RTypes.h"
#include "MMatchUtil.h"
#include "MMatchPremiumIPCache.h"
#include "MCommandBuilder.h"
#include "MMatchAntiHack.h"
#include "MUtil.h"


void MMatchServer::RequestNewHashValue( const MUID& uidChar, const char* szNewRandom )
{
	MCommand* pCmd = CreateCommand( MC_REQUEST_XTRAP_HASHVALUE, uidChar );
	if( 0 != pCmd )
	{
		if( pCmd->AddParameter(new MCmdParamStr(szNewRandom)) )
			Post( pCmd );
	}
}


void MMatchServer::OnResponeNewHashValue( const MUID& uidChar, char* szSerialKey )
{
#ifdef _XTRAP
	if( MGetServerConfig()->IsUseXTrap() )
	{
		MMatchObject* pObj = GetObject( uidChar );
		if( 0 != pObj )
		{
#ifdef _DEBUG
			pObj->SetLastRecvNewHashValueTime( GetGlobalClockCount() );
			return;
#endif
			if( MGetServerConfig()->IsDebugServer() && MGetServerConfig()->IsDebugLoginIPList(pObj->GetIPString()) )
			{
				pObj->SetLastRecvNewHashValueTime( GetGlobalClockCount() );
				return;
			}

			if( MMatchAntiHack::CrackCheck(szSerialKey, pObj->GetAntiHackInfo()->m_szRandomValue) )
			{
				pObj->SetLastRecvNewHashValueTime( GetGlobalClockCount() );
			}
			else
			{
				if( MMDS_DISCONN_WAIT > pObj->GetDisconnStatusInfo().GetStatus() )
				{
					// hacking!!
					mlog( "find hacker. CID(%u), ", (0 != pObj->GetCharInfo()) ? pObj->GetCharInfo()->m_nCID : 0 );
					mlog( "serialkey(%s), randomvalue(%s)\n", szSerialKey, pObj->GetAntiHackInfo()->m_szRandomValue );

					pObj->SetXTrapHackerDisconnectWaitInfo();
				}
			}
		}
	}
#endif
}