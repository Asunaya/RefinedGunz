#include "stdafx.h"
#include "MMatchRuleGunGame.h"
#include "MBlobArray.h"
#include <algorithm>
#include <random>

MMatchRuleGunGame::MMatchRuleGunGame(MMatchStage* pStage) : MMatchRule(pStage)
{
}

MMatchRuleGunGame::~MMatchRuleGunGame()
{
}

void MMatchRuleGunGame::OnBegin()
{
	m_MatchSet = MGetGunGame()->GetGGSet(rand() % MGetGunGame()->GetSetCount());
	//To shuffle or not to shuffle? that is the question.
	std::random_shuffle(m_MatchSet.begin(), m_MatchSet.end());
}

void MMatchRuleGunGame::OnRoundBegin()
{

}

void MMatchRuleGunGame::OnEnd()
{
	m_MatchSet.clear();
}

void MMatchRuleGunGame::OnRoundEnd()
{
}

bool MMatchRuleGunGame::RoundCount()
{
	if (++m_nRoundCount < 1) return true;
	return false;
}


void MMatchRuleGunGame::OnRoundTimeOut()
{
	SetRoundArg(MMATCH_ROUNDRESULT_DRAW);
}

void MMatchRuleGunGame::OnGameKill(const MUID& uidAttacker, const MUID& uidVictim)
{
	MMatchObject* pAttacker = MGetMatchServer()->GetObject(uidAttacker);
	MMatchObject* pVictim = MGetMatchServer()->GetObject(uidVictim);
	if (!GetStage())
		return;
	if (!pAttacker || !pVictim)
		return;

	if (pAttacker == pVictim)
		return;

	SendNewSet(pAttacker->GetUID(), pAttacker->GetKillCount());
}

bool MMatchRuleGunGame::CheckKillCount(MMatchObject* pOutObj)
{
	MMatchStage* pStage = GetStage();
	for (auto* pObj : pStage->GetObjectList())
	{
		if (pObj->GetEnterBattle() == false) continue;

		if (pObj->GetKillCount() >= (unsigned int)pStage->GetStageSetting()->GetRoundMax())
		{
			pOutObj = pObj;
			return true;
		}
	}
	return false;
}

bool MMatchRuleGunGame::OnCheckRoundFinish()
{
	MMatchObject* pObj = nullptr;

	if (CheckKillCount(pObj))
	{
		return true;
	}
	return false;
}

void MMatchRuleGunGame::SendNewSet(const MUID& uidPlayer,int KillCount)
{
	//TODO: fix this possible bug??
	GGSet pSet = m_MatchSet.at(KillCount);
	//if (pSet.WeaponSet[0] == 0 && pSet.WeaponSet[0]  == nullptr)
	//{
	//	mlog("PLayer has more kills than there are itemsets in gungame.xml\n");
	//	return;
	//}

	MCommand* pCommand = MGetMatchServer()->CreateCommand(MC_MATCH_GUNGAME_SEND_NEW_WEAPON, MUID(0, 0));
	pCommand->AddParameter(new MCmdParamUID(uidPlayer));
	pCommand->AddParameter(new MCmdParamUInt(pSet.WeaponSet[0]));
	pCommand->AddParameter(new MCmdParamUInt(pSet.WeaponSet[1]));
	pCommand->AddParameter(new MCmdParamUInt(pSet.WeaponSet[2]));

	MGetMatchServer()->RouteToBattle(GetStage()->GetUID(), pCommand);
}

bool MMatchRuleGunGame::OnRun()
{
	return MMatchRule::OnRun();
}

void MMatchRuleGunGame::OnEnterBattle(MUID& uidPlayer)
{
	SendNewSet(uidPlayer, 0);
	for (auto* pObj : GetStage()->GetObjectList())
	{
		if (pObj->GetUID() == uidPlayer)
			continue;
		if (!pObj->GetEnterBattle())
			continue;
		//TODO: make a separate function to send all players weapon info?
		SendNewSet(pObj->GetUID(), pObj->GetKillCount());
	}
}