#include "stdafx.h"
#include "MBMatchServer.h"
#include "MatchServerDoc.h"
#include "OutputView.h"
#include <atltime.h>
#include "MatchServer.h"
#include "MMap.h"
#include "MErrorTable.h"
#include "CommandLogView.h"
#include "MDebug.h"
#include "MMatchRule.h"
#include "MBMatchAuth.h"
#include "MDebug.h"
#include "MMatchStatus.h"
#include "MMatchSchedule.h"
#include "MSharedCommandTable.h"
#include "MMatchConfig.h"
#include "MBlobArray.h"
#include "MUtil.h"



bool MBMatchServer::OnCommand(MCommand* pCommand)
{
	if( MMatchServer::OnCommand(pCommand) )
		return true;

	switch( pCommand->GetID() )
	{
	case MC_MATCH_SCHEDULE_ANNOUNCE_MAKE :
		{
			// 서버 스케쥴러에 등록된 공지를 클라이언트로 보냄.

			char szAnnounce[ 512 ];

			pCommand->GetParameter( szAnnounce, 0, MPT_STR, 512 );

			OnScheduleAnnounce( szAnnounce );
		}
		break;

	case MC_MATCH_SCHEDULE_CLAN_SERVER_SWITCH_DOWN : 
		{
			// 클랜 서버일 경우 클랜전을 할수 없게 설정합니다.

			// bool bClanEnabled;
			// pCommand->GetParameter( &bClanEnabled, 0, MPT_BOOL );

			OnScheduleClanServerSwitchDown();		
		}
		break;

	case MC_MATCH_SCHEDULE_CLAN_SERVER_SWITCH_ON :
		{
			OnScheduleClanServerSwitchUp();
		}
		break;

	case MC_REQUEST_KEEPER_CONNECT_MATCHSERVER :
		{
			OnRequestConnectMatchServer( pCommand->GetSenderUID() );
		}
		break;

	case MC_REQUEST_MATCHSERVER_STATUS :
		{
			MUID uidSender = pCommand->GetSenderUID();

			OnResponseServerStatus( uidSender );
		}
		break;

	case MC_REQUEST_SERVER_HEARBEAT :
		{
			OnRequestServerHearbeat( pCommand->GetSenderUID() );
		}
		break;

	case MC_REQUEST_KEEPER_ANNOUNCE :
		{
			char szAnnounce[ 256 ];

			pCommand->GetParameter( szAnnounce, 0, MPT_STR, 256 );

			OnRequestKeeperAnnounce( pCommand->GetSenderUID(), szAnnounce );
		}
		break;

	case MC_REQUEST_ANNOUNCE_STOP_SERVER :
		{
			OnRequestStopServerWithAnnounce( pCommand->GetSenderUID() );
		}
		break;

	case MC_REQUEST_KEEPER_MANAGER_SCHEDULE :
		{
			int nType;
			int nYear;
			int nMonth;
			int nDay;
			int nHour;
			int nMin;
			int nCount;
			int nCommand;
			char szAnnounce[ 256 ] = {0,};

			pCommand->GetParameter( &nType, 0, MPT_INT );
			pCommand->GetParameter( &nYear, 1, MPT_INT );
			pCommand->GetParameter( &nMonth, 2, MPT_INT );
			pCommand->GetParameter( &nDay, 3, MPT_INT );
			pCommand->GetParameter( &nHour, 4, MPT_INT );
			pCommand->GetParameter( &nMin, 5, MPT_INT );
			pCommand->GetParameter( &nCount, 6, MPT_INT );
			pCommand->GetParameter( &nCommand, 7, MPT_INT );
			pCommand->GetParameter( szAnnounce, 8, MPT_STR, 255 );

			OnRequestSchedule( pCommand->GetSenderUID(), 
				nType, 
				nYear, 
				nMonth, 
				nDay, 
				nHour, 
				nMin, 
				nCount, 
				nCommand, 
				szAnnounce );
		}
		break;

	case MC_MATCH_SCHEDULE_STOP_SERVER :
		{
			char szAnnounce[ 256 ] = { 0, };

			pCommand->GetParameter( szAnnounce, 0, MPT_STR, 255 );

			OnRequestKeeperStopServerSchedule( pCommand->GetSenderUID(), szAnnounce );
		}
		break;

	case MC_LOCAL_UPDATE_USE_COUNTRY_FILTER :
		{
			OnLocalUpdateUseCountryFilter();
		}
		break;

	case MC_LOCAL_GET_DB_IP_TO_COUNTRY :
		{
			OnLocalGetDBIPtoCountry();
		}
		break;

	case MC_LOCAL_GET_DB_BLOCK_COUNTRY_CODE : 
		{
			OnLocalGetDBBlockCountryCode();
		}
		break;

	case MC_LOCAL_GET_DB_CUSTOM_IP :
		{
			OnLocalGetDBCustomIP();
		}
		break;

	case MC_LOCAL_UPDATE_IP_TO_COUNTRY :
		{
			OnLocalUpdateIPtoCountry();
		}
		break;

	case MC_LOCAL_UPDATE_BLOCK_COUTRYCODE :
		{
			OnLocalUpdateBlockCountryCode();
		}
		break;

	case MC_LOCAL_UPDATE_CUSTOM_IP :
		{
			OnLocalUpdateCustomIP();
		}
		break;

	case MC_LOCAL_UPDATE_ACCEPT_INVALID_IP :
		{
			OnLocalUpdateAcceptInvaildIP();
		}
		break;

	case MC_REQUEST_RELOAD_CONFIG :
		{
			char szFileList[ 1024 ] = {0,};

			pCommand->GetParameter( szFileList, 0, MPT_STR, 1024 );

			OnRequestReloadServerConfig( pCommand->GetSenderUID(), szFileList );
		}
		break;

	case MC_REQUEST_ADD_HASHMAP :
		{
			char szNewHashValue[ 128 ] = {0,};

			pCommand->GetParameter( szNewHashValue, 0, MPT_STR, 128 );

			OnRequestAddHashMap( pCommand->GetSenderUID(), szNewHashValue );
		}
		break;
	
	case MC_HSHIELD_PING:
		{
		}
		break;

	case MC_HSHIELD_PONG:
		{
			unsigned int nTimeStamp;
			pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT);
			MCommandParameter* pParam = pCommand->GetParameter(1);
			if(pParam->GetType() != MPT_BLOB) 
				break;

			void* pBlob = pParam->GetPointer();
			int nCount = MGetBlobArrayCount(pBlob);
			unsigned char* pbyAckMsg = (unsigned char*)MGetBlobArrayElement(pBlob, 0);

			OnHShieldPong(pCommand->GetSenderUID(), nTimeStamp);

			MMatchObject* pObj = GetObject(pCommand->GetSenderUID());
			if (pObj == NULL) return false;

			DWORD dwRet = HShield_AnalyzeAckMsg(pObj->GetHShieldInfo()->m_pCRCInfo, pbyAckMsg, pObj->GetHShieldInfo()->m_pbyReqInfo);

			if(dwRet != ERROR_SUCCESS)
			{
				++pObj->m_dwHShieldCheckCount;

				LOG(LOG_FILE, "@AnalyzeAckMsg - Find Hacker(%s) (Error code = %x) (Ack Msg = %s) (Check count = %u) (CRC Address = %x)", 
					pObj->GetAccountInfo()->m_szUserID, dwRet, pbyAckMsg,
					pObj->m_dwHShieldCheckCount, pObj->GetHShieldInfo()->m_pCRCInfo );

				MCommand* pNewCmd = CreateCommand(MC_MATCH_FIND_HACKING, MUID(0,0));
				RouteToListener(pObj, pNewCmd);

#ifdef _HSHIELD
				if( MGetServerConfig()->IsUseHShield() )
				{
#ifndef _DEBUG
					// 비정상적 유저이므로 캐릭터 선택할때 접속을 끊는다.
					pObj->SetHacker(true);
					pObj->SetHShieldHackerDisconnectWaitInfo();

					LOG(LOG_FILE, "Hacker(%s) disconnect wait", pObj->GetAccountInfo()->m_szUserID);
#endif
				}
#endif
			}

			LOG(LOG_DEBUG, "Ping from (%u:%u) = %d", 
				pCommand->GetSenderUID().High, pCommand->GetSenderUID().Low, timeGetTime()-nTimeStamp);
		}
		break;

	default :
		{
			// 정의되지 않은 커맨드.
		}
		return false;
	}

	return true;
}