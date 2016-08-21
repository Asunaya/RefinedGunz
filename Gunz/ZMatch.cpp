#include "stdafx.h"

#include "ZMatch.h"
#include "ZGame.h"
#include <windows.h>
#include "ZCharacterManager.h"
#include "ZCharacter.h"
#include "ZPost.h"
#include "ZGameInterface.h"
#include "ZApplication.h"
#include "ZGameClient.h"
#include "ZScreenEffectManager.h"
#include "ZActionDef.h"
#include "ZWorldItem.h"
#include "ZApplication.h"
#include "ZRule.h"
#include "ZMapDesc.h"
#include "MMatchGlobal.h"
#include "ZRuleDuel.h"
#include "ZInput.h"

#define READY_COUNT	5
#define ZDELAY_AFTER_DYING		RESPAWN_DELAYTIME_AFTER_DYING		///< 죽고나서 리스폰되는 딜레이시간
#define ZDELAY_AFTER_DYING_MAX	20000	

ZMatch::ZMatch() : m_pRule(NULL)
{
	m_pStageSetting = ZGetGameClient()->GetMatchStageSetting();
	m_nRoundState = MMATCH_ROUNDSTATE_PREPARE;
	m_nLastDeadTime = 0;
	m_nNowTime = 0;
	m_nSoloSpawnTime = -1;
	memset(m_nTeamScore, 0, sizeof(m_nTeamScore));
	memset(m_nTeamKillCount, 0, sizeof(m_nTeamKillCount));
}

ZMatch::~ZMatch()
{

}

bool ZMatch::Create()
{
	m_nNowTime = 0;
	m_nCurrRound = 0;
	memset(m_nTeamScore, 0, sizeof(m_nTeamScore));
	memset(m_nTeamKillCount, 0, sizeof(m_nTeamKillCount));

	m_pRule = ZRule::CreateRule(this, GetMatchType());

	return true;
}

void ZMatch::Destroy()
{
	if (m_pRule)
	{
		delete m_pRule; m_pRule = NULL;
	}

}

void ZMatch::SetRound(int nCurrRound)
{
	m_nCurrRound = nCurrRound;
//	memset(m_nTeamKillCount, 0, sizeof(m_nTeamKillCount));
}


void ZMatch::Update(float fDelta)
{
	m_nNowTime = GetGlobalTimeMS();

	switch (GetRoundState())
	{
	case MMATCH_ROUNDSTATE_PREPARE:
		{

		}
		break;
	case MMATCH_ROUNDSTATE_COUNTDOWN:
		{
			if ((m_nNowTime - m_nStartTime) > ((READY_COUNT+1) * 1000))
			{
				m_nStartTime = m_nNowTime;
			}
		}
		break;
	case MMATCH_ROUNDSTATE_PLAY:
		{
			ProcessRespawn();
		}
		break;
	}

	if (m_pRule) m_pRule->Update(fDelta);
}


void ZMatch::ProcessRespawn()
{
#ifdef _QUEST
	if (ZGetGameTypeManager()->IsQuestDerived(GetMatchType())) return;
#endif
	// 데쓰매치일 경우 죽으면 바로 살아난다.
	if (!IsWaitForRoundEnd() && g_pGame->m_pMyCharacter)
	{
		static bool bLastDead = false;
		if (g_pGame->m_pMyCharacter->IsDie())
		{
			if (bLastDead == false)
			{
				m_nLastDeadTime = m_nNowTime;	// 죽은 시간
			}

			m_nSoloSpawnTime = m_nNowTime - m_nLastDeadTime;

			if (m_nSoloSpawnTime >= ZDELAY_AFTER_DYING_MAX)	// 죽어서 아무것도 안누르고 있으면 15초후 리스폰
			{
				SoloSpawn();
			}
			else if (m_nSoloSpawnTime >= ZDELAY_AFTER_DYING)
			{
				static bool st_bCapturedActionKey = false;
				bool bNow = ZIsActionKeyPressed(ZACTION_USE_WEAPON) ||
					ZIsActionKeyPressed(ZACTION_JUMP);

				if ((st_bCapturedActionKey == true) && (bNow == false))
				{
					SoloSpawn();

				}

				st_bCapturedActionKey = bNow;
			}


		}

		bLastDead = g_pGame->m_pMyCharacter->IsDie();
	}

}

int ZMatch::GetRoundReadyCount(void)
{
	return ( READY_COUNT - (GetGlobalTimeMS() - m_nStartTime) / 1000 );
}


void ZMatch::OnDrawGameMessage()
{

#ifndef _PUBLISH
	// for debug 시간 표시
	float fTime=g_pGame->GetTime();
	int nTimeMinute=fTime/60.f;
	fTime=fmod(fTime,60.f);
	int nTimeSecond=fTime;
	fTime=fmod(fTime,1.f);

	char szTimeMessage[256] = "";
	sprintf_safe(szTimeMessage, "%d:%02d.%02d",nTimeMinute,nTimeSecond,int(fTime*100.f));
	ZApplication::GetGameInterface()->SetTextWidget("Time", szTimeMessage);
#endif

#define CENTERMESSAGE	"CenterMessage"

	switch (GetRoundState())
	{
	case MMATCH_ROUNDSTATE_PREPARE:
		{
		}
		break;
	case MMATCH_ROUNDSTATE_COUNTDOWN:
		{

		}
		break;
	case MMATCH_ROUNDSTATE_PLAY:
		{

		}
		break;
	case MMATCH_ROUNDSTATE_FINISH:
		{

		}
		break;
	default:
		{

		}
		break;
	}

}



void ZMatch::SoloSpawn()
{
	if (GetMatchType() == MMATCH_GAMETYPE_DUEL) return;

	rvector pos = rvector(0.0f, 0.0f, 0.0f);
	rvector dir = rvector(0.0f, 1.0f, 0.0f);

	ZMapSpawnData* pSpawnData;
		
	if (IsTeamPlay())
	{
		int nSpawnIndex[2] = { 0, 0 };
		for (int i = 0; i < 2; i++)
			if (ZApplication::GetGame()->m_pMyCharacter->GetTeamID() == MMT_RED + i)
				pSpawnData = g_pGame->GetMapDesc()->GetSpawnManager()->GetTeamData(i, nSpawnIndex[i]);
	}
	else
		pSpawnData = ZApplication::GetGame()->GetMapDesc()->GetSpawnManager()->GetSoloRandomData();
		
	if (pSpawnData == NULL)
	{
		_ASSERT(0);
	}
	else
	{
		if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_DEBUG) {
			ZPostSpawn(pSpawnData->m_Pos, pSpawnData->m_Dir);
		} else {
			if (g_pGame->GetSpawnRequested() == false) {
				ZPostRequestSpawn(ZGetMyUID(), pSpawnData->m_Pos, pSpawnData->m_Dir);
				g_pGame->SetSpawnRequested(true);
			}
		}	
	}
		
	m_nSoloSpawnTime = -1;
}



void ZMatch::InitCharactersPosition()
{
	// 팀전일 경우
	if (IsTeamPlay())
	{
		int nSpawnIndex[ 2] = { 0, 0 };

		for (ZCharacterManager::iterator itor = g_pGame->m_CharacterManager.begin();
			itor != g_pGame->m_CharacterManager.end(); ++itor)
		{
			ZCharacter* pCharacter = (*itor).second;
			for (int i = 0; i < 2; i++)
			{
				if (pCharacter->GetTeamID() == MMT_RED + i)
				{
					ZMapSpawnData* pSpawnData = g_pGame->GetMapDesc()->GetSpawnManager()->GetTeamData(i, nSpawnIndex[i]);
					if (pSpawnData != NULL)
					{
						pCharacter->SetPosition(pSpawnData->m_Pos);
						pCharacter->SetDirection(pSpawnData->m_Dir);

						nSpawnIndex[i]++;
					}
				}
			}
		}

		return;
	}

	// 듀얼모드 일 경우
	else if ( ZApplication::GetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
	{
		ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		if ( pDuel)
		{
			int nIndex = 2;
			if ( pDuel->QInfo.m_uidChampion == ZGetMyUID())
				nIndex = 0;
			else if ( pDuel->QInfo.m_uidChallenger == ZGetMyUID())
				nIndex = 1;

			// 듀얼 전용 맵이면 맨 처음과 두번째 위치
			if ( MIsMapOnlyDuel( ZGetGameClient()->GetMatchStageSetting()->GetMapIndex()))
			{
				ZMapSpawnData* pSpawnData = g_pGame->GetMapDesc()->GetSpawnManager()->GetData( nIndex);
				if ( pSpawnData != NULL)
				{
					g_pGame->m_pMyCharacter->SetPosition( pSpawnData->m_Pos);
					g_pGame->m_pMyCharacter->SetDirection( pSpawnData->m_Dir);
				}
			}

			// 듀얼 전용 맵이 아니면 팀전과 같은 방식
			else
			{
				ZMapSpawnData* pSpawnData = g_pGame->GetMapDesc()->GetSpawnManager()->GetTeamData( nIndex, 0);
				if (pSpawnData != NULL)
				{
					g_pGame->m_pMyCharacter->SetPosition( pSpawnData->m_Pos);
					g_pGame->m_pMyCharacter->SetDirection( pSpawnData->m_Dir);
				}
			}

			return;
		}
	}

	// 쏠로일 경우나 이도저도 아니면 랜덤
	ZMapSpawnData* pSpawnData = g_pGame->GetMapDesc()->GetSpawnManager()->GetSoloRandomData();
	if (pSpawnData != NULL)
	{
		g_pGame->m_pMyCharacter->SetPosition(pSpawnData->m_Pos);
		g_pGame->m_pMyCharacter->SetDirection(pSpawnData->m_Dir);
	}
}



void ZMatch::InitRound()
{
	// 시간을 0으로
	g_pGame->InitRound();

	InitCharactersPosition();
	InitCharactersProperties();

	// 월드아이템 초기화
	ZGetWorldItemManager()->Reset();

	// 맨 처음 게임시 스폰시킨다.
	rvector pos = g_pGame->m_pMyCharacter->GetPosition();
	rvector dir = g_pGame->m_pMyCharacter->m_DirectionLower;

	m_nRoundKills = 0;

	bool isObserver = false;

	if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_DEBUG) {
		ZPostSpawn(pos, dir);
	} else {
		if (g_pGame->GetSpawnRequested() == false) {
			if (GetMatchType() == MMATCH_GAMETYPE_DUEL)
			{
//				ZRuleDuel* pDuel = (ZRuleDuel*)GetRule();
//				if (pDuel->GetQueueIdx(ZGetMyUID()) <= 1)
//				{
//					ZPostRequestSpawn(ZGetMyUID(), pos, dir);
//					g_pGame->SetSpawnRequested(true);
//				}
//				else
//					isObserver = true;
				for (ZCharacterManager::iterator itor = g_pGame->m_CharacterManager.begin();
					itor != g_pGame->m_CharacterManager.end(); ++itor)
				{
					ZCharacter* pCharacter = (*itor).second;
					pCharacter->ForceDie();
					pCharacter->SetVisible(false);
				}
			}
			else
			{
				ZPostRequestSpawn(ZGetMyUID(), pos, dir);
				g_pGame->SetSpawnRequested(true);
			}
		}
	}

//	m_nRoundKills = 0;

	// AdminHide 처리
	MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
	if (pObjCache && pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide)) {
		ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);
	} else {
		// 옵져버 모드였으면 해제.
		if (!isObserver)
			g_pGame->ReleaseObserver();
		else
		{
			ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);
			g_pGame->ReserveObserver();
			g_pGame->m_pMyCharacter->ForceDie();

		}
	}
	memset(m_nTeamKillCount, 0, sizeof(m_nTeamKillCount));
}


void ZMatch::InitCharactersProperties()
{
	for (ZCharacterManager::iterator itor = g_pGame->m_CharacterManager.begin();
		itor != g_pGame->m_CharacterManager.end(); ++itor)
	{
		ZCharacter* pCharacter = (*itor).second;
		pCharacter->InitStatus();

		pCharacter->SetVisible(true);
	}

}

void ZMatch::SetRoundState(MMATCH_ROUNDSTATE nRoundState, int nArg)
{
	if (m_nRoundState == nRoundState) return;
	m_nRoundState = nRoundState;

#ifndef _PUBLISH
	char szLog[128];
	sprintf_safe(szLog, "RoundState:%d 으로 바뀜\n", m_nRoundState);
	OutputDebugString(szLog);
#endif

	switch(m_nRoundState) 
	{

	case MMATCH_ROUNDSTATE_PREPARE: 
		{

		}
		break;
	case MMATCH_ROUNDSTATE_COUNTDOWN : 
		{
			InitRound();
		}
		break;
	case MMATCH_ROUNDSTATE_PLAY:
		{
			if (!IsTeamPlay())
			{

			}
		}
		break;
	case MMATCH_ROUNDSTATE_FINISH:
		{
			g_pGame->FlushObserverCommands();

			if (GetMatchType() != MMATCH_GAMETYPE_DUEL)
			{
				if (m_nCurrRound+1 >= GetRoundCount())
				{
					ZGetGameInterface()->FinishGame();
				}
			}
			else
			{
				for (ZCharacterManager::iterator itor = g_pGame->m_CharacterManager.begin();
					itor != g_pGame->m_CharacterManager.end(); ++itor)
				{
					ZCharacter* pCharacter = (*itor).second;
					if (pCharacter->GetKills() >= GetRoundCount())
					{
						ZGetGameInterface()->FinishGame();
						break;
					}
				}				
			}

			if (IsTeamPlay())
			{
				if (nArg == MMATCH_ROUNDRESULT_DRAW)
				{
					// Do nothing...
				} else {
					MMatchTeam nTeamWon = (nArg == MMATCH_ROUNDRESULT_REDWON ? MMT_RED : MMT_BLUE);
					if (nTeamWon == MMT_RED)
						m_nTeamScore[MMT_RED]++;
					else if (nTeamWon == MMT_BLUE)
						m_nTeamScore[MMT_BLUE]++;
				}
			}
		}
		break;
	case MMATCH_ROUNDSTATE_EXIT:
		{
			
		}
		break;
	case MMATCH_ROUNDSTATE_FREE:
		{

		}
		break;
	};
}

const char* ZMatch::GetTeamName(int nTeamID)
{
	switch (nTeamID)
	{
	case MMT_SPECTATOR:
		return MMATCH_SPECTATOR_STR;
	case MMT_RED:
		return MMATCH_TEAM1_NAME_STR;
	case MMT_BLUE:
		return MMATCH_TEAM2_NAME_STR;
	default:
		return "";
	}
	return "";
}


int ZMatch::GetRoundCount()
{
//	if (IsWaitForRoundEnd())
		return m_pStageSetting->GetStageSetting()->nRoundMax;

	return 1;
}


void ZMatch::GetTeamAliveCount(int* pnRedTeam, int* pnBlueTeam)
{
	int nRedTeam = 0, nBlueTeam = 0;
	if (IsTeamPlay())
	{
		for (ZCharacterManager::iterator itor = g_pGame->m_CharacterManager.begin();
			itor != g_pGame->m_CharacterManager.end(); ++itor)
		{
			ZCharacter* pCharacter = (*itor).second;
			if (!pCharacter->IsDie())
			{
				if (pCharacter->GetTeamID() == 0)
				{
					nRedTeam++;
				}
				else
				{
					nBlueTeam++;
				}
			}
		}
	}

	*pnRedTeam = nRedTeam;
	*pnBlueTeam = nBlueTeam;
}

void ZMatch::RespawnSolo(bool bForce)
{
	if ((!IsWaitForRoundEnd() && g_pGame->m_pMyCharacter->IsDie()) || bForce)
	{
		SoloSpawn();
	}
}

void ZMatch::OnForcedEntry(ZCharacter* pCharacter)
{
	if (pCharacter == NULL)
	{
		_ASSERT(0); return;
	}
	// 난입한 플레이어가 나 자신일 경우
	if (pCharacter == ZApplication::GetGame()->m_pMyCharacter)
	{
		// AdminHide 처리
		MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
		if (pObjCache && pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide)) {
			ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);
		} else {
			// 팀플레이면 스팩테이터
			if (IsWaitForRoundEnd()/* && ZApplication::GetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL*/)
			{
				pCharacter->SetVisible(false);
				pCharacter->ForceDie();
				ZApplication::GetGameInterface()->GetCombatInterface()->SetObserverMode(true);
			}	
			else	// 솔로일경우는 바로 스폰
			{
				InitRound();
			}
		}
	}
	else
	{
		// 팀플레이에 다른 플레이어가 난입하면 다음판부터 진행해야 하기때문에 캐릭터 보여주지 않음
		// 단 FREE상태일 때에는 보여준다
		if (IsWaitForRoundEnd() && (GetRoundState() != MMATCH_ROUNDSTATE_FREE))
		{
			if (pCharacter != NULL)
			{
				pCharacter->SetVisible(false);
			}
			pCharacter->ForceDie();
		}
	}
}

int ZMatch::GetRemainedSpawnTime()
{
	int nTimeSec = -1;

	if (GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
	{
		if (!IsWaitForRoundEnd())
		{
			if (g_pGame->m_pMyCharacter->IsDie())
			{
				if (m_nSoloSpawnTime < 0) return -1;
				int nElapsedTime = m_nSoloSpawnTime;
				if (nElapsedTime > ZDELAY_AFTER_DYING) nElapsedTime = ZDELAY_AFTER_DYING;

				nTimeSec = (((ZDELAY_AFTER_DYING - nElapsedTime)+999) / 1000);
			}
		}
	}

	return nTimeSec;
}


void ZMatch::SetRoundStartTime( void)
{
	m_dwStartTime = GetGlobalTimeMS();
}

DWORD ZMatch::GetRemaindTime( void)
{
	return ( GetGlobalTimeMS() - m_dwStartTime);
}


bool ZMatch::OnCommand(MCommand* pCommand)
{
	if (m_pRule) return m_pRule->OnCommand(pCommand);
	return false;
}

void ZMatch::OnResponseRuleInfo(MTD_RuleInfo* pInfo)
{
	if (pInfo->nRuleType != GetMatchType()) return;

	if (m_pRule)
	{
		m_pRule->OnResponseRuleInfo(pInfo);
	}
}