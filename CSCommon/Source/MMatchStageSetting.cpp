#include "stdafx.h"
#include "MMatchStageSetting.h"

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

	m_StageSetting.Netcode = NetcodeType::ServerBased;
	m_StageSetting.ForceHPAP = true;
	m_StageSetting.HP = 100;
	m_StageSetting.AP = 50;
	m_StageSetting.NoFlip = true;
	m_StageSetting.SwordsOnly = false;
}

void MMatchStageSetting::SetMapName(const char* pszName)
{ 
	strcpy_safe(m_StageSetting.szMapName, pszName); 

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

	if (MIsCorrectMap(nMapIndex))
	{
		strcpy_safe(m_StageSetting.szMapName, g_MapDesc[nMapIndex].szMapName); 
	}
}

void MMatchStageSetting::Clear()
{
	SetDefault();
	m_CharSettingList.clear();
	m_uidMaster = MUID(0,0);
	m_nStageState = STAGE_STATE_STANDBY;

}

MSTAGE_CHAR_SETTING_NODE* MMatchStageSetting::FindCharSetting(const MUID& uid)
{
	for (auto&& node : m_CharSettingList)
	{
		if (uid == node.uidChar)
			return &node;
	}
	return nullptr;
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
	m_StageSetting = *pSetting;
}

void MMatchStageSetting::UpdateCharSetting(const MUID& uid, unsigned int nTeam, MMatchObjectStageState nStageState)
{
	auto pNode = FindCharSetting(uid);
	if (pNode) {
		pNode->nTeam = nTeam;
		pNode->nState = nStageState;
	} else {
		MSTAGE_CHAR_SETTING_NODE NewNode;
		NewNode.uidChar = uid;
		NewNode.nTeam = nTeam;
		NewNode.nState = nStageState;
		m_CharSettingList.push_back(NewNode);
	}			
}

const MMatchGameTypeInfo* MMatchStageSetting::GetCurrGameTypeInfo()
{ 
	return MGetGameTypeMgr()->GetInfo(m_StageSetting.nGameType); 
}
