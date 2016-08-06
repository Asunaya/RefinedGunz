#include "stdafx.h"
#include "MBMatchServer.h"
#ifdef MFC
#include "MatchServerDoc.h"
#include "OutputView.h"
#include "MatchServer.h"
#include "CommandLogView.h"
#endif
#include <atltime.h>
#include "MMap.h"
#include "MErrorTable.h"
#include "MDebug.h"
#include "MMatchRule.h"
#include "MBMatchAuth.h"
#include "MDebug.h"
#include "MMatchStatus.h"
#include "MMatchSchedule.h"
#include "MSharedCommandTable.h"
#include "MMatchConfig.h"
#include "MMatchEventFactory.h"
#include "HShield/AntiCPSvrfunc.h"
#include "MMatchLocale.h"
#pragma comment(lib, "comsupp.lib")

#ifdef _XTRAP
#include "XTrap/XCrackChk.h"
#pragma comment ( lib, "XTrap/XCrackChk.lib")
#endif

#if defined(_DEBUG) && defined(MFC)
#define new DEBUG_NEW
#endif

bool MBMatchServer::OnCreate(void)
{
	if( !MMatchServer::OnCreate() )
		return false;

	if( !m_ConfigReloader.Create() )
		return false;

	if( !InitHShiled() )
		return false;

	WriteServerInfoLog();

	return true;
}

void MBMatchServer::OnDestroy(void)
{
#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
		_AntiCpSvr_Finalize();
#endif
}

void MBMatchServer::OnPrepareCommand(MCommand* pCommand)
{
#if defined(_DEBUG) && defined(MFC)
#ifndef _DEBUG_PUBLISH
	// 커맨드 로그 남기기
	if(m_pCmdLogView==NULL) return;

	CMatchServerApp* pApp = (CMatchServerApp*)AfxGetApp();
	if (pApp->CheckOutputLog() == false) return;


	CCommandLogView::CCommandType t;
	if(pCommand->m_pCommandDesc->IsFlag(MCDT_LOCAL)==true) t = CCommandLogView::CCT_LOCAL;
	else if(pCommand->m_Sender==m_This) t = CCommandLogView::CCT_SEND;
	else if(pCommand->m_Receiver==m_This) t = CCommandLogView::CCT_RECEIVE;
	else _ASSERT(FALSE);
	
	m_pCmdLogView->AddCommand(GetGlobalClockCount(), t, pCommand);
#endif
#endif
}



MBMatchServer::MBMatchServer(COutputView* pView)
{
#ifdef MFC
	m_pView = pView;
	m_pCmdLogView = NULL;
#endif
	
	SetKeeperUID( MUID(0, 0) );
	SetAuthBuilder(new MBMatchAuthBuilder);
}

void MBMatchServer::Shutdown()
{
	MMatchServer::Shutdown();
#ifdef MFC
	AfxGetMainWnd()->PostMessage(WM_DESTROY);
#endif
}

void MBMatchServer::Log(unsigned int nLogLevel, const char* szLog)
{
	std::lock_guard<std::mutex> Lock(LogMutex);
#ifdef _DEBUG
	if (nLogLevel || LOG_DEBUG)
	{
		OutputDebugString(szLog);
		OutputDebugString("\n");
	}
#endif

	CTime theTime = CTime::GetCurrentTime();
	CString szTime = theTime.Format( "[%c] " );

	if (nLogLevel || LOG_FILE)
	{
		char szTemp[1024];
		strcpy(szTemp, szTime);
		strcat(szTemp, szLog);
		strcat(szTemp, "\n");
		mlog("%s", szTemp);
	}

#ifdef MFC
	if (nLogLevel || LOG_PROG)
	{
		if(m_pView==NULL) return;
		m_pView->AddString(szTime, TIME_COLOR, false);
		m_pView->AddString(szLog, RGB(0,0,0));
	}
#else
	printf("%s%s\n", static_cast<const char*>(szTime), szLog);
#endif
}

bool MBMatchServer::InitSubTaskSchedule()
{
	/*  등록방법을 결정해야 함.
		우선은 코드 속에다가. */

	// TODO: 버그가 있는 것 같아 주석처리 해놓았습니다. 클랜전신청 스위치가 꺼지지 않아야 할 때 꺼집니다. 확인 요망 -bird
	// 수정 했음.

	// clan서버일 경우만.
	if( MSM_CLAN == MGetServerConfig()->GetServerMode() ){
		if( !AddClanServerSwitchDownSchedule() )
			return false;

		if( !AddClanServerAnnounceSchedule() )
		return false;
	}

	return true;
}

// 서버 시작시 클랜전 서버일경우 등록되는 스케쥴.
bool MBMatchServer::AddClanServerSwitchDownSchedule()
{
	int a = 0;

	MCommand* pCmd = CreateCommand( MC_MATCH_SCHEDULE_CLAN_SERVER_SWITCH_DOWN, MUID(0, 0) );
	if( 0 == pCmd )
		return false;

	tm t;
	MMatchGetLocalTime(&t);

	// 이것을 현제는 한번만 실행을 하지만, 매달 시작을 하는것으로 수정을 해야함.
	// 커맨드가 실행을 하는 시점에서 커맨드를 업데이트해서 다시 스케쥴러에 등록을 하는쪽으로 방향을 잡음.
	// OnCommnad계열에서 클랜 서버 다운 커맨드가 실행이 될시에 서버다운 커맨드를 다음달로 재 설정을 하고,
	//  다음달 1일에 클랜 서버가 다시 동작하는 커맨드를 생성하여 추가하는 방식으로 함.
	// 클랜서버 다운 커맨드 실행 -> 다음달 1일 클랜 서버 다시 실행 커맨드 생성, 등록 -> 다음달 클랜서버 다운 커맨드 생성, 등록.

	MMatchScheduleData* pScheduleData = m_pScheduler->MakeOnceScheduleData( t.tm_year - 100, t.tm_mon + 1, GetMaxDay(), 23, 50, pCmd );
	if( 0 == pScheduleData ){
		delete pCmd;
		return false;
	}

	if( !m_pScheduler->AddDynamicSchedule(pScheduleData) ){
		delete pCmd;
		delete pScheduleData;
		return false;
	}

	mlog( "MBMatchServer::AddClanServerSwitchDownSchedule - 클랜서버다운 커맨드 생성 성공. 다음실행시간:%d년%d월%d일 %d시%d분\n",
		pScheduleData->GetYear(), pScheduleData->GetMonth(), pScheduleData->GetDay(),
		pScheduleData->GetHour(), pScheduleData->GetMin() );

	return true;
}


bool MBMatchServer::AddClanServerSwitchUpSchedule()
{
	// 다음달 1일 아침 10시에 클랜서버 클랜전 활성화.
	MCommand* pCmd = CreateCommand( MC_MATCH_SCHEDULE_CLAN_SERVER_SWITCH_ON, MUID(0, 0) );
	if( 0 == pCmd )
	{
		mlog( "MBMatchServer::AddClanServerSwitchUpSchedule - 클랜섭 활성화 커맨드 생성 실패.\n" );
		return false;
	}

	tm t;
	MMatchGetLocalTime(&t);

	unsigned char cYear;
	unsigned char cMon;

	// 다음달이 내년으로 넘어갔는지 검사함.
	if( 12 >= (t.tm_mon + 2) )
	{
		cYear = t.tm_year - 100;
		cMon  = t.tm_mon + 2;
	}
	else
	{
		// 다음달이 내년 1월일 경우.
		cYear = t.tm_year - 99;
		cMon  = 1;
	}

	MMatchScheduleData* pScheduleData = m_pScheduler->MakeOnceScheduleData( cYear, cMon, 1, 9, 0, pCmd );
	if( 0 == pScheduleData )
	{
		delete pCmd;
		return false;
	}

	if( !m_pScheduler->AddDynamicSchedule(pScheduleData) )
	{
		delete pCmd;
		delete pScheduleData;
		return false;
	}

	mlog( "MBMatchServer::AddClanServerSwitchUpSchedule - 클랜서버업 커맨드 생성 성공. 다음실행시간:%d년%d월%d일 %d시%d분\n",
		pScheduleData->GetYear(), pScheduleData->GetMonth(), pScheduleData->GetDay(),
		pScheduleData->GetHour(), pScheduleData->GetMin() );

	return true;
}


// 서버 시작시 등록되어야 하는 공지사항 스케쥴.
bool MBMatchServer::AddClanServerAnnounceSchedule()
{
	char szTest[] = "클랜전 게임을 11시 50분에 닫도록 하겠습니다.";

	MCommand* pCmd = CreateCommand( MC_MATCH_SCHEDULE_ANNOUNCE_MAKE, MUID(0, 0) );
	if( 0 == pCmd )
		return false;

	MCommandParameterString* pCmdPrmStr = new MCommandParameterString( szTest );
	if( 0 == pCmdPrmStr ){
		delete pCmd;
		return false;
	}
	
	if( !pCmd->AddParameter(pCmdPrmStr) ){
		delete pCmd;
		return false;
	}

	tm t;
	MMatchGetLocalTime(&t);

	MMatchScheduleData* pScheduleData = m_pScheduler->MakeOnceScheduleData( t.tm_year - 100, t.tm_mon + 1, GetMaxDay(), 23, 40, pCmd );
	if( 0 == pScheduleData ){
		delete pCmd;
		return false;
	}

	if( !m_pScheduler->AddDynamicSchedule(pScheduleData) ){
		pScheduleData->Release();
		delete pScheduleData;
		return false;
	}
	
	return true;
}



char log_buffer[65535];

void AddStr(const char* pFormat,...)
{
	va_list args;
	char temp[1024];

	va_start(args, pFormat);
	vsprintf(temp, pFormat, args);

	strcat(log_buffer, temp);
	va_end(args);
}

void MBMatchServer::OnViewServerStatus()
{
	MGetServerStatusSingleton()->SaveToLogFile();

/*
	char temp[256];
	log_buffer[0] = 0;

	AddStr("\n== 전체 정보 보기 ==\n");
	AddStr("전체 접속자 = %d\n", (int)m_Objects.size());
	AddStr("개설된 방 = %d\n", (int)m_StageMap.size());

	AddStr("== 방 정보 보기 ==\n");

	int nIndex = 0;
	for(MMatchStageMap::iterator itor=m_StageMap.begin(); itor!=m_StageMap.end(); ++itor)
	{
		MMatchStage* pStage = (*itor).second;
		MMatchStageSetting* pStageSetting = pStage->GetStageSetting();

		AddStr("[%2d] %s (%d명) ", nIndex, pStage->GetName(), pStage->GetObjCount());

		AddStr(" ( ");
		for (MUIDRefCache::iterator ObjItor = pStage->GetObjBegin(); ObjItor != pStage->GetObjEnd(); ++ObjItor)
		{
			MMatchObject* pObj = (MMatchObject*)(*ObjItor).second;
			AddStr("%s ", pObj->GetName());
		}
		AddStr(")\n");

		temp[0] = 0;
		if (pStage->GetRule()) 
		{
			switch (pStage->GetRule()->GetRoundState())
			{
			case MMATCH_ROUNDSTATE_WAIT: strcpy(temp, "wait"); break;
			case MMATCH_ROUNDSTATE_COUNTDOWN: strcpy(temp, "countdown"); break;
			case MMATCH_ROUNDSTATE_PLAY: strcpy(temp, "play"); break;
			case MMATCH_ROUNDSTATE_FINISH: strcpy(temp, "finish"); break;
			case MMATCH_ROUNDSTATE_EXIT: strcpy(temp, "exit"); break;
			case MMATCH_ROUNDSTATE_FREE: strcpy(temp, "free"); break;
			}
		}
		AddStr("라운드 상태: %s , ", temp);

		AddStr("맵이름: %s , ", pStageSetting->GetMapName());
		AddStr("게임타입: %s , ", GetGameTypeStr(pStageSetting->GetGameType()));
		int nNowRound = 0;
		if (pStage->GetRule()) 
		{
			nNowRound = pStage->GetRule()->GetRoundCount();
		}

		AddStr("전체라운드: %d , 현재라운드: %d , ", pStageSetting->GetRoundMax(), nNowRound);
		AddStr("제한시간: %d분 , 제한레벨: %d , 최대인원: %d", pStageSetting->GetLimitTime(),
				pStageSetting->GetLimitLevel(), pStageSetting->GetMaxPlayers());

		
		AddStr("\n------------------------------------------------------------------------------\n");
	}

	AddStr("< 전체접속자 = %d , 개설된방 = %d >\n", (int)m_Objects.size(), (int)m_StageMap.size());

//	Log(log_buffer);
*/
}

ULONG MBMatchServer::HShield_MakeGuidReqMsg(unsigned char *pbyGuidReqMsg, unsigned char *pbyGuidReqInfo)
{
#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
		return _AntiCpSvr_MakeGuidReqMsg(pbyGuidReqMsg, pbyGuidReqInfo);
#endif
	return 0L;
}

ULONG MBMatchServer::HShield_AnalyzeGuidAckMsg(unsigned char *pbyGuidAckMsg, unsigned char *pbyGuidReqInfo, unsigned long **ppCrcInfo)
{
#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
		return _AntiCpSvr_AnalyzeGuidAckMsg(pbyGuidAckMsg, pbyGuidReqInfo, ppCrcInfo);
#endif
	return 0L;
}

ULONG MBMatchServer::HShield_MakeReqMsg(unsigned long *pCrcInfo, unsigned char *pbyReqMsg, unsigned char *pbyReqInfo, unsigned long ulOption)
{
#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
		return _AntiCpSvr_MakeReqMsg(pCrcInfo, pbyReqMsg, pbyReqInfo, ulOption);
#endif
	return 0L;
}

ULONG MBMatchServer::HShield_AnalyzeAckMsg(unsigned long *pCrcInfo, unsigned char *pbyAckMsg, unsigned char *pbyReqInfo)
{
#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
		return _AntiCpSvr_AnalyzeAckMsg(pCrcInfo, pbyAckMsg, pbyReqInfo);
#endif
	return 0L;
}

void MBMatchServer::InitLocator()
{
	if (!MGetServerConfig()->IsMasterServer())
		return;

	Log(LOG_ALL, "Creating locator...");

	Locator = std::make_unique<MLocator>();
	if (!Locator->Create())
	{
		Log(LOG_ALL, "Failed to create locator");
		Locator.reset();
		return;
	}

	Log(LOG_ALL, "Locator created!");
}

void MBMatchServer::XTrap_RandomKeyGenW(char* strKeyValue)
{
#ifdef _XTRAP
	if( MGetServerConfig()->IsUseXTrap() )
		RandomKeyGenW(strKeyValue);
#endif
}


int MBMatchServer::XTrap_XCrackCheckW(char* strSerialKey, char* strRandomValue, char* strHashValue)
{
#ifdef _XTRAP
	if( MGetServerConfig()->IsUseXTrap() )
		return XCrackCheckW(strSerialKey, strRandomValue, strHashValue);
#endif
	return TRUE;
}


bool MBMatchServer::IsKeeper( const MUID& uidKeeper )
{
	MMatchObject* pObj = GetObject( uidKeeper );
	if( 0 == pObj )
		return false;

	if( !MGetServerConfig()->IsKeeperIP(pObj->GetIPString()) )
	{
		mlog( "Keeper hacking. " );
		if( 0 != pObj->GetIPString() )
			mlog( "IP:%s, ", pObj->GetIPString() );

		if( (0 != pObj->GetCharInfo()) && (0 != pObj->GetCharInfo()->m_szName) )
			mlog( "Name:%s", pObj->GetCharInfo()->m_szName );

		mlog( "\n" );

		return false;
	}

	return true;
}


void MBMatchServer::WriteServerInfoLog()
{
	mlog( "\n" );
	mlog( "================================== Server configure info ==================================\n" );

	char szTemp[256];
	sprintf(szTemp, "Release Date : %s", __DATE__);
	Log(LOG_ALL, szTemp);
	
#ifdef _XTRAP
	// #define _XTRAP
	if( MGetServerConfig()->IsUseXTrap() )
	{
		LOG( LOG_ALL, "X-Trap On" );
		LOG( LOG_ALL, "HashMapSize : (%u)", MMatchAntiHack::GetHashMapSize() );
		LOG( LOG_ALL, "X-Trap usable state : (true)" );
	}
	else
		LOG( LOG_ALL, "X-Trap Off" );
#endif

#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
	{
		LOG( LOG_ALL, "Hack Shield On" );
		LOG( LOG_ALL, "Hack Shield usable state : (true)" );
	}
	else
		LOG( LOG_ALL, "Hack Shield Off" );
#endif

	if( MC_KOREA == MGetLocale()->GetCountry() )
		LOG( LOG_ALL, "Server Country : KOREA" );
	else if( MC_US == MGetLocale()->GetCountry() )
		LOG( LOG_ALL, "Server Country : US" );
	else if( MC_JAPAN == MGetLocale()->GetCountry() )
		LOG( LOG_ALL, "Server Country : JAPAN" );
	else if( MC_BRAZIL == MGetLocale()->GetCountry() )
		LOG( LOG_ALL, "Server Country : BRAZIL" );
	else if( MC_INDIA == MGetLocale()->GetCountry() )
		LOG( LOG_ALL, "Server Country : INDIA" );
	else
	{
		ASSERT( 0 && "국가 설정을 해주세요." );
		LOG( LOG_ALL, "!!!!!!!!!Not setted country code!!!!!!!!!!!!!" );
	}
	
	LOG( LOG_ALL, "Command version : (%u)", MCOMMAND_VERSION );
	LOG( LOG_ALL, "Event usable state : (%s)", MGetServerConfig()->IsUseEvent() ? "true" : "false" );
	LOG( LOG_ALL, "Load event size : (%u)", MMatchEventFactoryManager::GetInstance().GetLoadEventSize() );
	LOG( LOG_ALL, "FileCRCCheckSum usable state : (%s)", MGetServerConfig()->IsUseFileCrc() ? "true" : "false" );
	LOG( LOG_ALL, "FileCRC size : (%u)", MMatchAntiHack::GetFielCRCSize() );
	LOG( LOG_ALL, "Country Code Filter usalbe state : (%s)", MGetServerConfig()->IsUseFilter() ? "true" : "false" );
	LOG( LOG_ALL, "Accept Invalied IP state : (%s)", MGetServerConfig()->IsAcceptInvalidIP() ? "true" : "false" );
	LOG( LOG_ALL, "Keeper IP : (%s)", MGetServerConfig()->GetKeeperIP().c_str() );

	mlog( "===========================================================================================\n" );
	mlog( "\n" );
}


bool MBMatchServer::InitHShiled()
{
#ifdef _HSHIELD
	// HSHIELD Init
	if( MGetServerConfig()->IsUseHShield() )
	{
		// HShield Init 지금은 사용하지 않음.
		// if(MPacketHShieldCrypter::Init() != ERROR_SUCCESS)
		// 	bResult = false;
	
		GetCurrentDirectory( sizeof( m_strFileCrcDataPath), m_strFileCrcDataPath);
		strcat( m_strFileCrcDataPath, "\\HackShield.crc");
		ULONG dwRet = _AntiCpSvr_Initialize(m_strFileCrcDataPath);

		if( dwRet != ERROR_SUCCESS )
		{
			AfxMessageBox("_AntiCpSvr_Initialize Failed!");
			return false;
		}

		LOG( LOG_ALL, "HackShield Init OK." );
	}
#endif

	return true;
}

void MBMatchServer::OnRun()
{
	if (Locator)
		Locator->Run();

	MMatchServer::OnRun();
}