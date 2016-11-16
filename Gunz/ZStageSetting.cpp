#include "stdafx.h"

#include "ZStageSetting.h"
#include "ZGameInterface.h"
#include "MComboBox.h"
#include "MFrame.h"
#include "ZApplication.h"
#include "ZGameClient.h"
#include "ZPost.h"
#include "MMatchGameType.h"
#include "ZLanguageConf.h"
#include "ZChannelRule.h"
#include "ZConfiguration.h"
#include "ZGameTypeList.h"
#include "ZMap.h"

// 최대인원
#define STAGESETTING_MAXPLAYER_MAX		5
#define STAGESETTING_MAXPLAYER_DEFAULT	1
static struct _STAGESETTING_MAXPLAYER
{
	int					Value;
	char				szText[16];
} StageSetting_MaxPlayer[STAGESETTING_MAXPLAYER_MAX] =
{ {4, "4"}, {8, "8"}, {10, "10"}, {12, "12"}, {16, "16"} };

// 라운드
#define STAGESETTING_MAXROUND_MAX		6
#define STAGESETTING_MAXROUND_DEFAULT	3
static struct _STAGESETTING_MAXROUND
{
	int					Value;
	char				szText[32];
} StageSetting_MaxRound[STAGESETTING_MAXROUND_MAX] = {
	{10, "10"}, {20, "20"}, {30, "30"}, {50, "50"}, {70, "70"}, {100, "100"} };

// 제한시간
#define STAGESETTING_LIMITTIME_MAX		6
#define STAGESETTING_LIMITTIME_DEFAULT	4
static struct _STAGESETTING_LIMITTIME_SINGLE
{
	int					Value;
	char				szText[32];
} StageSetting_LimitTime_Single[STAGESETTING_LIMITTIME_MAX] =
{ {99999, "무한"}, {10, "10분"}, {15, "15분"}, {20, "20분"}, {30, "30분"}, {60, "60분"} };
static struct _STAGESETTING_LIMITTIME_TEAM
{
	int					Value;
	char				szText[32];
} StageSetting_LimitTime_Team[STAGESETTING_LIMITTIME_MAX] =
{ {99999, "무한"}, {3, "3분"}, {5, "5분"}, {7, "7분"}, {10, "10분"}, {15, "15분"} };

// 제한레벨
#define STAGESETTING_LIMITLEVEL_MAX		4
#define STAGESETTING_LIMITLEVEL_DEFAULT	0
static struct _STAGESETTING_LIMITLEVEL
{
	int					Value;
	char				szText[32];
} StageSetting_LimitLevel[STAGESETTING_LIMITLEVEL_MAX] =
{ {0, "없음"}, {5, "레벨차 5"}, {10, "레벨차 10"}, {15, "레벨차 15"} };

// 팀킬여부
#define STAGESETTING_TEAMKILL_MAX		2
#define STAGESETTING_TEAMKILL_DEFAULT	0
static struct _STAGESETTING_TEAM
{
	bool	Value;
	char	szText[32];
} StageSetting_TeamKill[STAGESETTING_TEAMKILL_MAX] =
{ {true, "허용"}, {false, "금지"} };

// 난입여부
#define STAGESETTING_FORCEDENTRY_MAX		2
#define STAGESETTING_FORCEDENTRY_DEFAULT	0
static struct _STAGESETTING_FORCEDENTRY
{
	bool	Value;
	char	szText[32];
} StageSetting_ForcedEntry[STAGESETTING_FORCEDENTRY_MAX] =
{ {true, "허용"}, {false, "금지"} };

// 관전 허용
#define STAGESETTING_OBSERVER_MAX		2
#define STAGESETTING_OBSERVER_DEFAULT	1
static struct _STAGESETTING_OBSERVER
{
	bool	Value;
	char	szText[32];
} StageSetting_Observer[STAGESETTING_OBSERVER_MAX] =
{ {true, "허용"}, {false, "금지"} };

// 투표 가능
#define STAGESETTING_VOTE_MAX		2
#define STAGESETTING_VOTE_DEFAULT	0
static struct _STAGESETTING_VOTE
{
	bool	Value;
	char	szText[32];
} StageSetting_Vote[STAGESETTING_VOTE_MAX] =
{ {true, "허용"}, {false, "금지"} };

// 팀 밸런스
#define STAGESETTING_TEAMBALANCING_MAX		2
#define STAGESETTING_TEAMBALANCING_DEFAULT	0
static struct _STAGESETTING_TEAMBALANCING
{
	bool	Value;
	char	szText[32];
} StageSetting_TeamBalancing[STAGESETTING_TEAMBALANCING_MAX] =
{ {true, "ON"}, {false, "OFF"} };

static struct _STAGESETTING_NETCODE
{
	NetcodeType	Value;
	char	szText[32];
} StageSetting_Netcode[3] =
{ { NetcodeType::ServerBased, "Server-based" },{ NetcodeType::P2PAntilead, "Peer to Peer Antilead" },{ NetcodeType::P2PLead, "Peer to Peer Lead" }, };





static bool BuildStageSetting(MSTAGE_SETTING_NODE* pOutNode)
{
	ZeroMemory(pOutNode, sizeof(MSTAGE_SETTING_NODE));
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// 게임타입
	{
		MComboBox* pCB = (MComboBox*)pResource->FindWidget("StageType");
		if (pCB == NULL) return false;
		bool bExistGameType = false;
		for (int i = 0; i < MMATCH_GAMETYPE_MAX; i++)
		{
			if (!strcmp(pCB->GetText(), ZGetGameTypeManager()->GetGameTypeStr(MMATCH_GAMETYPE(i))))
			{
				bExistGameType = true;
				pOutNode->nGameType = ZGetGameTypeManager()->GetInfo(MMATCH_GAMETYPE(i))->nGameTypeID;
				break;
			}
		}
		if (!bExistGameType) return false;
	}

	strcpy_safe(pOutNode->szStageName, ZGetGameClient()->GetStageName());

	// 맵
	{
		MComboBox* pCB = (MComboBox*)pResource->FindWidget("MapSelection");
		if (pCB)
			strcpy_safe(pOutNode->szMapName, pCB->GetText());
	}

	auto BuildStageSettingListItem = [&](const char* WidgetItemName,
		auto& NodeVariable, const auto& ItemList)
	{
		MComboBox* pCB = (MComboBox*)pResource->FindWidget(WidgetItemName);
		if (pCB && (pCB->GetSelIndex() >= 0))
		{
			int nItemCount = 0;
			auto itr = ItemList.begin();
			for (int i = 0; i < pCB->GetSelIndex(); i++)
			{
				if (itr == ItemList.end())
				{
					itr = ItemList.begin();
					break;
				}
				itr++;
			}
			NodeVariable = (*itr)->m_nValue;
		}
	};

	ZGameTypeConfig* pGameTypeCfg = ZGetConfiguration()->GetGameTypeList()->GetGameTypeCfg(pOutNode->nGameType);
	if (pGameTypeCfg)
	{
		BuildStageSettingListItem("StageMaxPlayer", pOutNode->nMaxPlayers, pGameTypeCfg->m_MaxPlayers);
		BuildStageSettingListItem("StageRoundCount", pOutNode->nRoundMax, pGameTypeCfg->m_Round);
		BuildStageSettingListItem("StageLimitTime", pOutNode->nLimitTime, pGameTypeCfg->m_LimitTime);
	}

	auto BuildStageSettingItem = [&](const char* WidgetItemName,
		auto& NodeVariable, auto& ItemList, int MaxItemCount)
	{
		MComboBox* pCB = (MComboBox*)pResource->FindWidget(WidgetItemName);
		if (pCB == nullptr) return;
		if (pCB->GetSelIndex() < MaxItemCount)
		{
			NodeVariable = ItemList[pCB->GetSelIndex()].Value;
		}
		else
		{
			return;
		}
	};

	BuildStageSettingItem("StageLevelLimit", pOutNode->nLimitLevel,
		StageSetting_LimitLevel, STAGESETTING_LIMITLEVEL_MAX);
	BuildStageSettingItem("StageTeamKill", pOutNode->bTeamKillEnabled,
		StageSetting_TeamKill, STAGESETTING_TEAMKILL_MAX);
	BuildStageSettingItem("StageIntrude", pOutNode->bForcedEntryEnabled,
		StageSetting_ForcedEntry, STAGESETTING_FORCEDENTRY_MAX);
	BuildStageSettingItem("StageTeamBalancing", pOutNode->bAutoTeamBalancing,
		StageSetting_TeamBalancing, STAGESETTING_FORCEDENTRY_MAX);

	if (pOutNode->nGameType == MMATCH_GAMETYPE_GLADIATOR_SOLO
		|| pOutNode->nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM)
	{
		pOutNode->Netcode = NetcodeType::P2PLead;
	}
	else
	{
		BuildStageSettingItem("StageNetcode", pOutNode->Netcode,
			StageSetting_Netcode, 3);
	}

	auto ForceHPAPWidget = static_cast<MButton*>(pResource->FindWidget("StageForceHPAP"));
	if (ForceHPAPWidget)
		pOutNode->ForceHPAP = ForceHPAPWidget->GetCheck();

	auto GetWidgetInt = [&](const char* WidgetName, int& Ret)
	{
		auto Widget = static_cast<MEdit*>(pResource->FindWidget(WidgetName));
		if (!Widget)
			return;

		auto Text = Widget->GetText();
		auto Pair = StringToInt(Text);

		if (!Pair.first)
			return;

		Ret = Pair.second;
	};

	GetWidgetInt("StageHP", pOutNode->HP);
	GetWidgetInt("StageAP", pOutNode->AP);

	auto Widget = static_cast<MButton*>(pResource->FindWidget("StageNoFlip"));
	if (Widget)
		pOutNode->NoFlip = Widget->GetCheck();

	Widget = static_cast<MButton*>(pResource->FindWidget("StageSwordsOnly"));
	if (Widget)
		pOutNode->SwordsOnly = Widget->GetCheck();

	return true;
}


void ZStageSetting::ShowStageSettingDialog(MSTAGE_SETTING_NODE* pStageSetting, bool bShowAll)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (pResource == NULL) return;


	{
		MComboBox* pCB = (MComboBox*)pResource->FindWidget("StageType");
		if (pCB)
		{
			for (int i = 0; i < MMATCH_GAMETYPE_MAX; i++)
			{
				if (pStageSetting->nGameType == ZGetGameTypeManager()->GetInfo(MMATCH_GAMETYPE(i))->nGameTypeID)
				{
					int nGameType = i;

					for (int j = 0; j < pCB->GetCount(); j++)
					{
						if (!_stricmp(pCB->GetString(j), ZGetGameTypeManager()->GetInfo(MMATCH_GAMETYPE(i))->szGameTypeStr))
						{
							pCB->SetSelIndex(j);
							break;
						}
					}
				}
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////
#define SHOWSTAGESETTING_LISTITEM( _WidgetItemName, _NodeVariable, _ItemList, _MaxValue)	\
{																				\
	MComboBox* pCB = (MComboBox*)pResource->FindWidget( _WidgetItemName);		\
	if ( pCB)																	\
	{																			\
		int nSelect = pCB->GetSelIndex(), nItemCount = 0;						\
		pCB->RemoveAll();														\
		MGAMETYPECFGDATA::iterator itr = _ItemList.begin();						\
		for ( ; itr != _ItemList.end();  itr++)									\
		{																		\
			if ( (*itr)->m_nValue <= _MaxValue)									\
			{																	\
				pCB->Add( (*itr)->m_szString);									\
				if ( (*itr)->m_nValue == _NodeVariable)							\
					nSelect = nItemCount;										\
				nItemCount++;													\
			}																	\
		}																		\
		if ( nSelect >= nItemCount)												\
			nSelect = nItemCount - 1;											\
		pCB->SetSelIndex( nSelect);												\
	}																			\
}
/////////////////////////////////////////////////////////////////////////////////

	ZGameTypeConfig* pGameTypeCfg = ZGetConfiguration()->GetGameTypeList()->GetGameTypeCfg(pStageSetting->nGameType);
	if (pGameTypeCfg)
	{
		SHOWSTAGESETTING_LISTITEM("StageMaxPlayer", pStageSetting->nMaxPlayers, pGameTypeCfg->m_MaxPlayers, g_MapDesc[pStageSetting->nMapIndex].nMaxPlayers);

		SHOWSTAGESETTING_LISTITEM("StageRoundCount", pStageSetting->nRoundMax, pGameTypeCfg->m_Round, 99999);

		SHOWSTAGESETTING_LISTITEM("StageLimitTime", pStageSetting->nLimitTime, pGameTypeCfg->m_LimitTime, 99999);
	}

	[&]()
	{
		auto cb = static_cast<MComboBox*>(pResource->FindWidget("StageNetcode"));

		if (!cb)
			return;

		cb->RemoveAll();

		cb->Add("Server-based");
		cb->Add("Peer to Peer Antilead");
		cb->Add("Peer to Peer Lead");

		bool LeadOnly = IsSwordsOnly(pStageSetting->nGameType)
			|| pStageSetting->SwordsOnly
			|| MGetGameTypeMgr()->IsQuestDerived(pStageSetting->nGameType);

		if (LeadOnly)
			cb->SetSelIndex(2);
		else
			cb->SetSelIndex((int)pStageSetting->Netcode);

		cb->Enable(!LeadOnly);
	}();


	/////////////////////////////////////////////////////////////////////////////////////////
#define SHOWSTAGESETTING_ITEM(_WidgetItemName, _NodeVariable, _nItemCount, _ItemList)	\
{																						\
	MComboBox* pCB = (MComboBox*)pResource->FindWidget(_WidgetItemName);				\
	if (pCB)																			\
	{																					\
		for (int i = 0; i < _nItemCount; i++)											\
		{																				\
			if (_ItemList[i].Value == _NodeVariable)									\
			{																			\
				pCB->SetSelIndex(i);													\
				break;																	\
			}																			\
		}																				\
	}																					\
}
/////////////////////////////////////////////////////////////////////////////////////////

	SHOWSTAGESETTING_ITEM("StageLevelLimit", pStageSetting->nLimitLevel,
		STAGESETTING_LIMITLEVEL_MAX, StageSetting_LimitLevel);

	SHOWSTAGESETTING_ITEM("StageTeamKill", pStageSetting->bTeamKillEnabled,
		STAGESETTING_TEAMKILL_MAX, StageSetting_TeamKill);

	SHOWSTAGESETTING_ITEM("StageIntrude", pStageSetting->bForcedEntryEnabled,
		STAGESETTING_FORCEDENTRY_MAX, StageSetting_ForcedEntry);

	SHOWSTAGESETTING_ITEM("StageTeamBalancing", pStageSetting->bAutoTeamBalancing,
		STAGESETTING_TEAMBALANCING_MAX, StageSetting_TeamBalancing);


	MComboBox* pCBTeamBanlance = (MComboBox*)pResource->FindWidget("StageTeamBalancing");
	if (pCBTeamBanlance)
		pCBTeamBanlance->Enable(ZGetGameTypeManager()->IsTeamGame(pStageSetting->nGameType));

	auto ForceHPAP = static_cast<MButton*>(pResource->FindWidget("StageForceHPAP"));
	if (ForceHPAP)
	{
		ForceHPAP->SetCheck(pStageSetting->ForceHPAP);
	}

	char buf[64];
	auto HP = static_cast<MEdit*>(pResource->FindWidget("StageHP"));
	if (HP)
	{
		itoa_safe(pStageSetting->HP, buf, 10);
		HP->SetText(buf);
	}

	auto AP = static_cast<MEdit*>(pResource->FindWidget("StageAP"));
	if (AP)
	{
		itoa_safe(pStageSetting->AP, buf, 10);
		AP->SetText(buf);
	}

	auto NoFlip = static_cast<MButton*>(pResource->FindWidget("StageNoFlip"));
	if (NoFlip)
	{
		NoFlip->SetCheck(pStageSetting->NoFlip);
	}

	auto SwordsOnly = static_cast<MButton*>(pResource->FindWidget("StageSwordsOnly"));
	if (SwordsOnly)
	{
		SwordsOnly->SetCheck(pStageSetting->SwordsOnly);

		SwordsOnly->Enable(!IsSwordsOnly(pStageSetting->nGameType));
	}

	if (bShowAll)
	{
		MWidget* pFindWidget = pResource->FindWidget("StageSettingFrame");
		if (pFindWidget != NULL) pFindWidget->Show(true, true);
	}
}



void ZStageSetting::InitStageSettingDialog()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (pResource == NULL) return;


#define INITSTAGESETTING_ITEM(_WidgetItemName, _NodeVariable, _nItemCount, _ItemList, _nItemDefaultIndex)	\
{																				\
	MComboBox* pCB = (MComboBox*)pResource->FindWidget(_WidgetItemName);		\
	if(pCB!=NULL)																\
	{																			\
		pCB->RemoveAll();														\
		for (int i = 0; i < _nItemCount; i++)									\
		{																		\
			pCB->Add(_ItemList[i].szText);										\
		}																		\
		pCB->SetSelIndex(_nItemDefaultIndex);									\
	}																			\
}

	char szText[256];


	// 게임 타입
	InitStageSettingGameType();



	// 레벨제한
#define GET_LIMITLEVEL(_value)		sprintf_safe(szText,"%s %d",ZMsg(MSG_WORD_LEVELDIFF),_value);
	strcpy_safe(StageSetting_LimitLevel[0].szText, ZMsg(MSG_WORD_NONE));
	GET_LIMITLEVEL(5);	strcpy_safe(StageSetting_LimitLevel[1].szText, szText);
	GET_LIMITLEVEL(10);	strcpy_safe(StageSetting_LimitLevel[2].szText, szText);
	GET_LIMITLEVEL(15);	strcpy_safe(StageSetting_LimitLevel[3].szText, szText);
	INITSTAGESETTING_ITEM("StageLevelLimit", pStageSetting->nLimitLevel,
		STAGESETTING_LIMITLEVEL_MAX, StageSetting_LimitLevel, STAGESETTING_LIMITLEVEL_DEFAULT);

	// 팀킬여부
	strcpy_safe(StageSetting_TeamKill[0].szText, ZMsg(MSG_WORD_PERMIT));
	strcpy_safe(StageSetting_TeamKill[1].szText, ZMsg(MSG_WORD_PROHIBIT));
	INITSTAGESETTING_ITEM("StageTeamKill", pStageSetting->bTeamKillEnabled,
		STAGESETTING_TEAMKILL_MAX, StageSetting_TeamKill, STAGESETTING_TEAMKILL_DEFAULT);

	// 게임중참가
	strcpy_safe(StageSetting_ForcedEntry[0].szText, ZMsg(MSG_WORD_PERMIT));
	strcpy_safe(StageSetting_ForcedEntry[1].szText, ZMsg(MSG_WORD_PROHIBIT));
	INITSTAGESETTING_ITEM("StageIntrude", pStageSetting->bForcedEntryEnabled,
		STAGESETTING_FORCEDENTRY_MAX, StageSetting_ForcedEntry, STAGESETTING_FORCEDENTRY_DEFAULT);

	// 팀 밸런스
	strcpy_safe(StageSetting_TeamBalancing[0].szText, ZMsg(MSG_WORD_ON));
	strcpy_safe(StageSetting_TeamBalancing[1].szText, ZMsg(MSG_WORD_OFF));
	INITSTAGESETTING_ITEM("StageTeamBalancing", pStageSetting->bAutoTeamBalancing,
		STAGESETTING_TEAMBALANCING_MAX, StageSetting_TeamBalancing, STAGESETTING_TEAMBALANCING_DEFAULT);

	// 관전 허용
	strcpy_safe(StageSetting_Observer[0].szText, ZMsg(MSG_WORD_PERMIT));
	strcpy_safe(StageSetting_Observer[1].szText, ZMsg(MSG_WORD_PROHIBIT));
	INITSTAGESETTING_ITEM("StageObserver", pStageSetting->bObserverEnabled,
		STAGESETTING_OBSERVER_MAX, StageSetting_Observer, STAGESETTING_OBSERVER_DEFAULT);

	// 투표 가능
	strcpy_safe(StageSetting_Vote[0].szText, ZMsg(MSG_WORD_PERMIT));
	strcpy_safe(StageSetting_Vote[1].szText, ZMsg(MSG_WORD_PROHIBIT));
	INITSTAGESETTING_ITEM("StageVote", pStageSetting->bVoteEnabled,
		STAGESETTING_VOTE_MAX, StageSetting_Vote, STAGESETTING_VOTE_DEFAULT);

	INITSTAGESETTING_ITEM("StageNetcode", (int)pStageSetting->Netcode,
		3, StageSetting_Netcode, 0);

	auto ForceHPAP = static_cast<MButton*>(pResource->FindWidget("StageForceHPAP"));
	if (ForceHPAP)
	{
		ForceHPAP->SetCheck(true);
	}

	auto HP = static_cast<MEdit*>(pResource->FindWidget("StageHP"));
	if (HP)
	{
		HP->SetText("100");
	}

	auto AP = static_cast<MEdit*>(pResource->FindWidget("StageAP"));
	if (AP)
	{
		AP->SetText("50");
	}

	auto NoFlip = static_cast<MButton*>(pResource->FindWidget("StageNoFlip"));
	if (NoFlip)
	{
		NoFlip->SetCheck(true);
	}

	auto SwordsOnly = static_cast<MButton*>(pResource->FindWidget("StageSwordsOnly"));
	if (SwordsOnly)
	{
		SwordsOnly->SetCheck(true);
	}
}

void ZStageSetting::ApplyStageSettingDialog()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (pResource == NULL) return;

	MWidget* pWidget = pResource->FindWidget("StageSettingFrame");
	if (pWidget != NULL) pWidget->Show(false);

	PostDataToServer();
}

void ZStageSetting::PostDataToServer()
{
	MSTAGE_SETTING_NODE SettingNode;
	BuildStageSetting(&SettingNode);
	ZPostStageSetting(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), &SettingNode);
}


void ZStageSetting::InitStageSettingGameType()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (pResource == NULL) return;

	MComboBox* pCB = (MComboBox*)pResource->FindWidget("StageType");
	MChannelRule* pRule = ZGetChannelRuleMgr()->GetCurrentRule();
	if (pRule == NULL) return;
	if (pCB == NULL) return;

	MChannelRuleGameTypeList* pGameTypeList = pRule->GetGameTypeList();

	pCB->RemoveAll();

	for (MChannelRuleGameTypeList::iterator itor = pGameTypeList->begin(); itor != pGameTypeList->end(); ++itor)
	{
		int nGameType = (*itor);
		if ((nGameType < 0) || (nGameType >= MMATCH_GAMETYPE_MAX)) continue;
		pCB->Add(ZGetGameTypeManager()->GetGameTypeStr(MMATCH_GAMETYPE(nGameType)));
	}

#ifdef _QUEST
	{
		if ((ZGetGameClient()) && (ZGetGameClient()->GetServerMode() == MSM_TEST))
		{
			pCB->Add(ZGetGameTypeManager()->GetGameTypeStr(MMATCH_GAMETYPE(MMATCH_GAMETYPE_QUEST)));
		}
	}
#endif

	pCB->SetSelIndex(MMATCH_GAMETYPE_DEFAULT);
}


void ZStageSetting::InitStageSettingGameFromGameType()
{
	if (!ZGetGameClient())
		return;

	if (ZGetGameClient()->IsForcedEntry())
		return;


	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (pResource == NULL) return;

	/////////////////////////////////////////////////////////////////////////////////
#define INITSTAGESETTING_LISTITEM( _WidgetItemName, _ItemList, _nItemDefaultIndex)	\
{																				\
	MComboBox* pCB = (MComboBox*)pResource->FindWidget( _WidgetItemName);		\
	if ( pCB)																	\
	{																			\
		int nSelect = pCB->GetSelIndex(), nItemCount = 0;						\
		pCB->RemoveAll();														\
		MGAMETYPECFGDATA::iterator itr = _ItemList.begin();						\
		for ( ; itr != _ItemList.end();  itr++)									\
			pCB->Add( (*itr)->m_szString);										\
		pCB->SetSelIndex( _nItemDefaultIndex);									\
	}																			\
}
/////////////////////////////////////////////////////////////////////////////////


	int nGameType = 0;
	MMATCH_GAMETYPE nPrevGameType = ZGetGameClient()->GetMatchStageSetting()->GetGameType();

	MComboBox* pCBType = (MComboBox*)pResource->FindWidget("StageType");
	if (pCBType && (pCBType >= 0))
	{
		// 이거 완전 무식한 하드코드... 고민끝에 나두 어쩔수 읍땅.... -_-;
		if (strcmp(pCBType->GetString(pCBType->GetSelIndex()), ZMsg(MSG_MT_DEATHMATCH_SOLO)) == 0)
			nGameType = 0;
		else if (strcmp(pCBType->GetString(pCBType->GetSelIndex()), ZMsg(MSG_MT_DEATHMATCH_TEAM)) == 0)
			nGameType = 1;
		else if (strcmp(pCBType->GetString(pCBType->GetSelIndex()), ZMsg(MSG_MT_GLADIATOR_SOLO)) == 0)
			nGameType = 2;
		else if (strcmp(pCBType->GetString(pCBType->GetSelIndex()), ZMsg(MSG_MT_GLADIATOR_TEAM)) == 0)
			nGameType = 3;
		else if (strcmp(pCBType->GetString(pCBType->GetSelIndex()), ZMsg(MSG_MT_ASSASSINATE)) == 0)
			nGameType = 4;
		else if (strcmp(pCBType->GetString(pCBType->GetSelIndex()), ZMsg(MSG_MT_TRAINING)) == 0)
			nGameType = 5;
		else if (strcmp(pCBType->GetString(pCBType->GetSelIndex()), ZMsg(MSG_MT_SURVIVAL)) == 0)
			nGameType = 6;
		else if (strcmp(pCBType->GetString(pCBType->GetSelIndex()), ZMsg(MSG_MT_QUEST)) == 0)
			nGameType = 7;
		else if (strcmp(pCBType->GetString(pCBType->GetSelIndex()), ZMsg(MSG_MT_BERSERKER)) == 0)
			nGameType = 8;
		else if (strcmp(pCBType->GetString(pCBType->GetSelIndex()), ZMsg(MSG_MT_DEATHMATCH_TEAM2)) == 0)
			nGameType = 9;
		else if (strcmp(pCBType->GetString(pCBType->GetSelIndex()), ZMsg(MSG_MT_DUEL)) == 0)
			nGameType = 10;
	}

	// Set game type
	ZGetGameClient()->GetMatchStageSetting()->SetGameType((MMATCH_GAMETYPE)nGameType);

	// Set game setting
	ZGameTypeConfig* pGameTypeCfg = ZGetConfiguration()->GetGameTypeList()->GetGameTypeCfg(nGameType);
	if (pGameTypeCfg)
	{
		// 최대 인원
		INITSTAGESETTING_LISTITEM("StageMaxPlayer", pGameTypeCfg->m_MaxPlayers, pGameTypeCfg->m_nDefaultMaxPlayers);

		// 라운드
		INITSTAGESETTING_LISTITEM("StageRoundCount", pGameTypeCfg->m_Round, pGameTypeCfg->m_nDefaultRound);

		// 제한시간
		INITSTAGESETTING_LISTITEM("StageLimitTime", pGameTypeCfg->m_LimitTime, pGameTypeCfg->m_nDefaultLimitTime);
	}

	// Set map
	MComboBox* pCB = (MComboBox*)pResource->FindWidget("MapSelection");
	if (pCB)
		InitMaps(pCB);
}