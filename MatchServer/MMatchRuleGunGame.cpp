#include "stdafx.h"
#include "MMatchRuleGunGame.h"
#include "MBlobArray.h"

MMatchRuleGunGame::MMatchRuleGunGame(MMatchStage* pStage) : MMatchRule(pStage)
{
}

MMatchRuleGunGame::~MMatchRuleGunGame()
{
}

void MMatchRuleGunGame::OnBegin()
{
	if (m_pGGSet.size() > 0) {
		for (auto itor = m_pGGSet.begin(); itor != m_pGGSet.end(); ++itor)
		{
			delete *itor;
		}
		m_pGGSet.clear();
	}

	int randSet = rand() % MGetGunGame()->GetSetCount();
	m_pGGSet = MGetGunGame()->GetGGSet(randSet);
}

void MMatchRuleGunGame::OnRoundBegin()
{

}

void MMatchRuleGunGame::OnEnd()
{
	for (auto itor = m_pGGSet.begin(); itor != m_pGGSet.end(); ++itor)
	{
		delete *itor;
	}
	m_pGGSet.clear();
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

//	if (pAttacker == pVictim)
//		return;

	SendNewSet(pAttacker->GetUID(), pAttacker->GetKillCount());
}

bool MMatchRuleGunGame::CheckKillCount(MMatchObject* pOutObj)
{
	MMatchStage* pStage = GetStage();
	for (auto itor = pStage->GetObjBegin(); itor != pStage->GetObjEnd(); ++itor)
	{
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
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
	GGSet* pSet = m_pGGSet.at(KillCount);
	if (pSet == nullptr)
	{
		mlog("PLayer has more kills than there are itemsets in gungame.xml\n");
		return;
	}

	MCommand* pCommand = MGetMatchServer()->CreateCommand(MC_MATCH_GUNGAME_SEND_NEW_WEAPON, MUID(0, 0));
	pCommand->AddParameter(new MCmdParamUID(uidPlayer));
	pCommand->AddParameter(new MCmdParamUInt(pSet->WeaponSet[0]));
	pCommand->AddParameter(new MCmdParamUInt(pSet->WeaponSet[1]));
	pCommand->AddParameter(new MCmdParamUInt(pSet->WeaponSet[2]));

	MGetMatchServer()->RouteToBattle(GetStage()->GetUID(), pCommand);
}

bool MMatchRuleGunGame::OnRun()
{
	return MMatchRule::OnRun();
}

void MMatchRuleGunGame::OnEnterBattle(MUID& uidPlayer)
{
	SendNewSet(uidPlayer, 0);
	for (auto itor = GetStage()->GetObjBegin(); itor != GetStage()->GetObjEnd(); ++itor)
	{
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		if (pObj->GetUID() == uidPlayer)
			continue;
		//TODO: make a separate function to send all players weapon info?
		SendNewSet(pObj->GetUID(), pObj->GetKillCount());
	}
}