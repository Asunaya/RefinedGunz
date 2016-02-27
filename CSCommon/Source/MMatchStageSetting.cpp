#include "stdafx.h"
#include "MMatchStageSetting.h"
#include "MMatchServer.h"

MMatchStageSetting::MMatchStageSetting()
{
	Clear();
}


MMatchStageSetting::~MMatchStageSetting()
{
	Clear();
}

unsigned long MMatchStageSetting::GetChecksum()
{
	return (m_StageSetting.nMapIndex + m_StageSetting.nGameType + m_StageSetting.nMaxPlayers);
}

void MMatchStageSetting::SetDefault()
{
	m_StageSetting.nMapIndex = 0;
	strcpy_safe(m_StageSetting.szMapName, MMATCH_DEFAULT_STAGESETTING_MAPNAME);
	m_StageSetting.nGameType = MMATCH_DEFAULT_STAGESETTING_GAMETYPE;
	m_StageSetting.bTeamKillEnabled = MMATCH_DEFAULT_STAGESETTING_TEAMKILL;
	m_StageSetting.bTeamWinThePoint = MMATCH_DEFAULT_STAGESETTING_TEAM_WINTHEPOINT;
	m_StageSetting.bForcedEntryEnabled = MMATCH_DEFAULT_STAGESETTING_FORCEDENTRY;
	m_StageSetting.nLimitTime = MMATCH_DEFAULT_STAGESETTING_LIMITTIME;
	m_StageSetting.nMaxPlayers = MMATCH_DEFAULT_STAGESETTING_MAXPLAYERS;
	m_StageSetting.nRoundMax = MMATCH_DEFAULT_STAGESETTING_ROUNDMAX;
	m_StageSetting.nLimitLevel = MMATCH_DEFAULT_STAGESETTING_LIMITLEVEL;
	m_StageSetting.bAutoTeamBalancing = MMATCH_DEFAULT_STAGESETTING_AUTOTEAMBALANCING;
	m_StageSetting.uidStage = MUID(0,0);

#ifdef _VOTESETTING
	m_StageSetting.bVoteEnabled = true;
	m_StageSetting.bObserverEnabled = false;
#endif
}

void MMatchStageSetting::SetMapName(char* pszName)
{ 
	strcpy_safe(m_StageSetting.szMapName, pszName); 

	// MapIndex까지 함께 세팅해준다.
	m_StageSetting.nMapIndex = 0;
	for (int i = 0; i < MMATCH_MAP_MAX; i++)
	{
		if (!_stricmp(g_MapDesc[i].szMapName, pszName))
		{
			m_StageSetting.nMapIndex = g_MapDesc[i].nMapID;
			break;
		}
	}
}

void MMatchStageSetting::SetMapIndex(int nMapIndex)
{
	m_StageSetting.nMapIndex = nMapIndex; 

	// MapName까지 함께 세팅해준다.
	if (MIsCorrectMap(nMapIndex))
	{
		strcpy_safe(m_StageSetting.szMapName, g_MapDesc[nMapIndex].szMapName); 
	}
}

void MMatchStageSetting::Clear()
{
	SetDefault();
	m_CharSettingList.DeleteAll();
	m_uidMaster = MUID(0,0);
	m_nStageState = STAGE_STATE_STANDBY;

}

MSTAGE_CHAR_SETTING_NODE* MMatchStageSetting::FindCharSetting(const MUID& uid)
{
	for (MStageCharSettingList::iterator i=m_CharSettingList.begin();i!=m_CharSettingList.end();i++) {
		if (uid == (*i)->uidChar) return (*i);
	}
	return NULL;
}

bool MMatchStageSetting::IsTeamPlay()
{
	return MGetGameTypeMgr()->IsTeamGame(m_StageSetting.nGameType);
}

bool MMatchStageSetting::IsWaitforRoundEnd()
{
	return MGetGameTypeMgr()->IsWaitForRoundEnd(m_StageSetting.nGameType);
}

bool MMatchStageSetting::IsQuestDrived()
{
	return MGetGameTypeMgr()->IsQuestDerived(m_StageSetting.nGameType);
}

void MMatchStageSetting::UpdateStageSetting(MSTAGE_SETTING_NODE* pSetting)
{
	memcpy(&m_StageSetting, pSetting, sizeof(MSTAGE_SETTING_NODE));
}


void MMatchStageSetting::UpdateCharSetting(const MUID& uid, unsigned int nTeam, MMatchObjectStageState nStageState)
{
	MSTAGE_CHAR_SETTING_NODE* pNode = FindCharSetting(uid);
	if (pNode) {
		pNode->nTeam = nTeam;
		pNode->nState = nStageState;
	} else {
		MSTAGE_CHAR_SETTING_NODE* pNew = new MSTAGE_CHAR_SETTING_NODE;
		pNew->uidChar = uid;
		pNew->nTeam = nTeam;
		pNew->nState = nStageState;
		m_CharSettingList.push_back(pNew);
	}			
}



const MMatchGameTypeInfo* MMatchStageSetting::GetCurrGameTypeInfo()
{ 
	return MGetGameTypeMgr()->GetInfo(m_StageSetting.nGameType); 
}



