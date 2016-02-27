#include "MObjectCharacter.h"
#include "MErrorTable.h"
#include "MAction.h"
#include "MActionPipe.h"
#include "MActParam.h"
#include "MZoneServer.h"
#include "MSharedCommandTable.h"
#include "MBlobArray.h"


IMPLEMENT_RTTI(MObjectCharacter)


////////////////////////////////////////////////////////////////////////////////////////
// MObjectCharacter ////////////////////////////////////////////////////////////////////
MObjectCharacter::~MObjectCharacter(void)
{
	ClearItems();
	UnloadActions();
}
MObjectCharacter::MObjectCharacter(MUID& uid, MMap* pMap, rvector& Pos) : MObject(uid, pMap, Pos)
{
	m_nLevel = 1;
	m_nType = MCT_HUMANOID;
	m_nRace = MCR_HUMAN;
	m_nClass = MCC_FIGHTER;
	m_nEXP = 0;

	LoadInitialData(); // 생성하자마자 초기치를 읽어온다. 이것은 나중에 수정해야 한다.
	ClearEquipmentItem();

	m_nMaxHP = CalcMaxHP();
	m_nMaxEN = CalcMaxEN();
	m_fHP = (float)(m_nMaxHP);
	m_fEN = (float)(m_nMaxEN);

	SetMoveMode(MCMM_RUN);
	SetMode(MCM_OFFENSIVE);
	SetState(MCS_STAND);

	m_pActParam = NULL;

	m_nLastHPRegenTime = 0;
	m_nLastENRegenTime = 0;
	m_nDiedTime = 0;
	m_nLastCacheTime = 0;
}

void MObjectCharacter::Tick(unsigned long int nTime)
{
	CacheUpdate(nTime);

	if (GetState() == MCS_DEAD)
		return;

	MObject::Tick(nTime);

	// Player일때에만 Regen - 우선 이것으로 체크
	if (m_nType == MCT_HUMANOID)
	{
		RegenHP(nTime);
		RegenEN(nTime);
	}

	// Action 관련
	for (list<MActionPipe*>::iterator i=m_ActionPipeList.begin(); i!=m_ActionPipeList.end(); i++) {
		MActionPipe* pPipe = *i;
		if (pPipe->CheckTurn(nTime))
			pPipe->Run(nTime, this, GetActParam());
	}

	RouteRepeatInfo(nTime);

	// 임시 AI, 곧 삭제
	if ( (GetSpawnID()) && ((m_nLastCacheTime/1000)%10 == 0) ) {	// 10초에한번?
		
	}
}

int MObjectCharacter::CalcMaxHP()
{
	return MCalculateMaxHP(m_nLevel, m_nClass, m_BasicAttr.nCON, 0);
}

int MObjectCharacter::CalcMaxEN()
{
	return MCalculateMaxEN(m_nLevel, m_nClass, m_BasicAttr.nINT, 0);
}

void MObjectCharacter::RegenHP(unsigned long int nTime)
{
	if ((nTime - m_nLastHPRegenTime) >= g_nHPRegenTickInteval)
	{
		if (m_fHP < m_nMaxHP)
		{
			float fRegen = MCalculateRegenHPPerTick(m_nLevel, m_nRace, 0);
//
char szBuf[128]; sprintf(szBuf, "Regen : UID(%d,%d) HP(%d) +%.2f \n", GetUID().High, GetUID().Low, GetHP(), fRegen);
OutputDebugString(szBuf);
//
			m_fHP += fRegen;
			if (m_fHP > m_nMaxHP) m_fHP = (float)m_nMaxHP;
		}
		m_nLastHPRegenTime = nTime;
	}
}

void MObjectCharacter::RegenEN(unsigned long int nTime)
{
	if ((nTime - m_nLastENRegenTime) >= g_nENRegenTickInteval)
	{
		if (m_fEN < m_nMaxEN)
		{
			float fRegen = MCalculateRegenENPerTick(m_nLevel, m_nClass, 0);
			m_fEN += fRegen;
			if (m_fEN > m_nMaxEN) m_fEN = (float)m_nMaxEN;
		}
		m_nLastENRegenTime = nTime;
	}

}


void MObjectCharacter::LoadInitialData()
{
	int nDescID;

	switch (m_nRace)
	{
	case MCR_HUMAN:		nDescID = 0; break;
	case MCR_ELF:		nDescID = 1; break;
	case MCR_DARKELF:	nDescID = 2; break;
	case MCR_DWARF:		nDescID = 3; break;
	case MCR_OGRE:		nDescID = 4; break;
	}

	MCharacterDesc*	pCharacterDesc = g_CharacterDescManager.Find(nDescID);
	if (pCharacterDesc!=NULL) m_BasicAttr = (*pCharacterDesc->GetDesc());
}

int MObjectCharacter::ObtainItem(MObjectItem* pObjectItem)
{
	// 나중에 무게 계산 등 Item을 가질 수 있는지 체크해야 한다.

	m_HasItems.insert(MObjectItemMap::value_type(pObjectItem->GetUID(), pObjectItem));
	return MOK;
}

void MObjectCharacter::ClearItems()
{
	ClearEquipmentItem();
	if (!m_HasItems.empty())
	{
		MObjectItem* pObjectItem = (*m_HasItems.begin()).second;
		m_HasItems.erase(m_HasItems.begin());
	}
}

bool MObjectCharacter::EquipItem(MUID& ItemUID, MEquipmentSlot TargetSlot)
{
	MObjectItem* pObjectItem = NULL;
	pObjectItem = FindItem(ItemUID);
	if (pObjectItem == NULL) return false;

	return EquipItem(pObjectItem, TargetSlot);
}

bool MObjectCharacter::EquipItem(MObjectItem* pObjectItem, MEquipmentSlot TargetSlot)
{
	if (CheckSlotType(pObjectItem->GetDesc()->SlotTypes, TargetSlot))
	{
		m_pEquipmentItems[TargetSlot] = pObjectItem;
		return true;
	}
	return false;
}

void MObjectCharacter::ClearEquipmentItem()
{
	for (int i = 0; i < MES_END; i++) m_pEquipmentItems[i] = NULL;
}

MObjectItem* MObjectCharacter::FindItem(MUID& uidItem)
{
	MObjectItemMap::iterator itor = m_HasItems.find(uidItem);

	if (itor != m_HasItems.end())
	{
		return (*itor).second;
	}
	return NULL;
}

float MObjectCharacter::UpdateHP(float fVal)
{
	m_fHP += fVal;
	if (m_fHP > (float)m_nMaxHP)
		m_fHP = (float)m_nMaxHP;
	if (m_fHP < 0)
		m_fHP = 0;
	return m_fHP;
}

float MObjectCharacter::UpdateEN(float fVal)
{
	m_fEN += fVal;
	if (m_fEN > (float)m_nMaxEN)
		m_fEN = (float)m_nMaxEN;
	if (m_fEN < 0)
		m_fEN = 0;
	return m_fEN;
}

void MObjectCharacter::SetMoveMode(MCharacterMoveMode nMoveMode)
{
	MZoneServer* pZoneServer = MZoneServer::GetInstance();

	m_nMoveMode = nMoveMode; 
	
	MCommand* pCmd = pZoneServer->CreateCommand(MC_OBJECT_MOVEMODE, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(GetUID()));
	pCmd->AddParameter(new MCommandParameterInt(nMoveMode));
	pZoneServer->RouteToRegion(GetMap()->GetID(), GetPos(), 2000.0f, pCmd);
}


void MObjectCharacter::SetMode(MCharacterMode nMode)
{
	MZoneServer* pZoneServer = MZoneServer::GetInstance();

	m_nMode = nMode; 
	
	MCommand* pCmd = pZoneServer->CreateCommand(MC_OBJECT_MODE, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(GetUID()));
	pCmd->AddParameter(new MCommandParameterInt(nMode));
	pZoneServer->RouteToRegion(GetMap()->GetID(), GetPos(), 2000.0f, pCmd);
}

void MObjectCharacter::SetState(MCharacterState nState)
{ 
	MZoneServer* pZoneServer = MZoneServer::GetInstance();

	m_nState = nState; 
	if (nState == MCS_DEAD)
		m_nDiedTime = pZoneServer->GetGlobalClockCount();
	else
		m_nDiedTime = 0;
	
	MCommand* pCmd = pZoneServer->CreateCommand(MC_OBJECT_STATE, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(GetUID()));
	pCmd->AddParameter(new MCommandParameterInt(nState));
	pZoneServer->RouteToRegion(GetMap()->GetID(), GetPos(), 2000.0f, pCmd);
}

void MObjectCharacter::Resurrect()
{
	m_fHP = (float)(m_nMaxHP);
	m_fEN = (float)(m_nMaxEN);

	SetState(MCS_STAND);
}

bool MObjectCharacter::CheckDestroy(int nTime)
{
	#define TIME_CORPSE_DECAY		20000
	if ( (HasCommListener()==false) && (m_nDiedTime!=0) && (nTime - m_nDiedTime >= TIME_CORPSE_DECAY) )
		return true;

	#define	TIME_CORPSE_RESURRECT	10000
	if ( (HasCommListener()==true) && (m_nDiedTime!=0) && (nTime - m_nDiedTime >= TIME_CORPSE_RESURRECT) )
		Resurrect();

	return false;
}

void MObjectCharacter::CacheUpdate(unsigned long int nTime)
{
	#define TIME_CACHEUPDATE_INTERVAL	1000
	if (nTime - m_nLastCacheTime > TIME_CACHEUPDATE_INTERVAL) {
		MZoneServer* pZoneServer = MZoneServer::GetInstance();
		pZoneServer->UpdateObjectRecognition(this);
		m_nLastCacheTime = nTime;
	}
}

void MObjectCharacter::AddAction(MAction* pAction)
{
	m_ActionList.push_back(pAction);
}

void MObjectCharacter::DelAction(int nActID)
{
	for (list<MAction*>::iterator i=m_ActionList.begin(); i!=m_ActionList.end(); i++) {
		MAction* pAct = *i;
		if (pAct->GetID() == nActID) {
			m_ActionList.erase(i);
			return;
		}
	}
}

MAction* MObjectCharacter::FindAction(int nActID)
{
	for (list<MAction*>::iterator i=m_ActionList.begin(); i!=m_ActionList.end(); i++) {
		MAction* pAct = *i;
		if (pAct->GetID() == nActID)
			return pAct;
	}
	return NULL;
}

void MObjectCharacter::AddActionPipe(MActionPipe* pActionPipe)
{
	m_ActionPipeList.push_back(pActionPipe);
}

void MObjectCharacter::DelActionPipe(int nActPipeID)
{
	for (list<MActionPipe*>::iterator i=m_ActionPipeList.begin(); i!=m_ActionPipeList.end(); i++) {
		MActionPipe* pPipe = *i;
		if (pPipe->GetID() == nActPipeID) {
			m_ActionPipeList.erase(i);
			return;
		}
	}
}

MActionPipe* MObjectCharacter::FindActionPipe(int nActPipeID)
{
	for (list<MActionPipe*>::iterator i=m_ActionPipeList.begin(); i!=m_ActionPipeList.end(); i++) {
		MActionPipe* pPipe = *i;
		if (pPipe->GetID() == nActPipeID)
			return pPipe;
	}
	return NULL;
}

void MObjectCharacter::UpdateActParam(MActParam* pActParam)
{
	if (GetActParam())
		delete GetActParam();
	m_pActParam = pActParam;
}

void MObjectCharacter::UnloadActions()
{
	while(m_ActionPipeList.begin()!=m_ActionPipeList.end())
	{
		list<MActionPipe*>::iterator i = m_ActionPipeList.begin();
		delete (*i);
		m_ActionPipeList.erase(i);
	}
	while(m_ActionList.begin()!=m_ActionList.end())
	{
		list<MAction*>::iterator i = m_ActionList.begin();
		delete (*i);
		m_ActionList.erase(i);
	}
	if (GetActParam()) {
		delete m_pActParam;
		m_pActParam = NULL;
	}
}

bool MObjectCharacter::Attack(MObjectCharacter* pTarget)
{
	if (pTarget->GetState() == MCS_DEAD)
		return false;

	MActionPipe* pPipe = FindActionPipe(PIPE_ID_ACTIVE);
	pPipe->ClearAction();

	MAction* pActAttack = FindAction(ACTION_ID_ATTACK);
	if (pActAttack == NULL) return false;
	pPipe->AddAction(pActAttack);

	MActParamDamage* pActParam = new MActParamDamage;
	pActParam->SetTarget(pTarget);
	pActParam->SetActionID(ACTION_ID_ATTACK);
	UpdateActParam(pActParam);

	pPipe->Start();

	SetMode(MCM_OFFENSIVE);

	return true;
}

void MObjectCharacter::StopAttack()
{
	MActionPipe* pPipe = FindActionPipe(PIPE_ID_ACTIVE);
	pPipe->Stop();
	UpdateActParam(NULL);

	SetMode(MCM_OFFENSIVE);
}

bool MObjectCharacter::Skill(int nSkillID, MObjectCharacter* pTarget)
{
	MActionPipe* pActivePipe = FindActionPipe(PIPE_ID_ACTIVE);
	if (pActivePipe->GetState() == PIPE_STATE_RUNNING)
		pActivePipe->Pause();

	MSkillPipe* pSkillPipe = (MSkillPipe*)FindActionPipe(PIPE_ID_SKILL);
	MAction* pActAttack = FindAction(ACTION_ID_SKILL);
	if (pActAttack == NULL) return false;
	pSkillPipe->ClearAction();
	pSkillPipe->AddAction(pActAttack);

	MActParamSkill* pActParam = new MActParamSkill;
	pActParam->SetTarget(pTarget);
	pActParam->SetActionID(ACTION_ID_SKILL);
	pActParam->SetSkillID(nSkillID);
	UpdateActParam(pActParam);
	
	pSkillPipe->Start();
	
	return true;
}

bool MObjectCharacter::Warning(MObjectCharacter* pAttacker)
{
	// AI 준비되면 반응맏김

	MZoneServer* pZoneServer = MZoneServer::GetInstance();
	MActivePipe* pPipe = (MActivePipe*)FindActionPipe(PIPE_ID_ACTIVE);
	if (pPipe->GetState() == PIPE_STATE_STOPPED) {
		pPipe->GiveDelayPenalty(pZoneServer->GetGlobalClockCount(), 2000);

		Attack(pAttacker);
	}

	return true;
}

bool MObjectCharacter::Damage(int nDamage, const MUID& uidAttacker)
{
	MZoneServer* pZoneServer = MZoneServer::GetInstance();

	MCommand* pCmd = pZoneServer->CreateCommand(MC_OBJECT_DAMAGE, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidAttacker));
	pCmd->AddParameter(new MCommandParameterUID(GetUID()));
	pCmd->AddParameter(new MCommandParameterInt(0));
	pCmd->AddParameter(new MCommandParameterInt(nDamage));
	pZoneServer->RouteToRegion(GetMap()->GetID(), GetPos(), 2000.0f, pCmd);

//
char szBuf[128]; sprintf(szBuf, "UID(%d,%d) Attacks UID(%d,%d) with Damage (%d) \n", 
						 uidAttacker.High, uidAttacker.Low, GetUID().High, GetUID().Low, nDamage);
OutputDebugString(szBuf);
//

	if (GetHP() <= nDamage) {
		m_fHP = 0.0f;
		SetState(MCS_DEAD);

		MCommand* pCmd = pZoneServer->CreateCommand(MC_OBJECT_KILLED, MUID(0,0));
		pCmd->AddParameter(new MCommandParameterUID(uidAttacker));
		pCmd->AddParameter(new MCommandParameterUID(GetUID()));
		pZoneServer->RouteToRegion(GetMap()->GetID(), GetPos(), 2000.0f, pCmd);

		return false;
	} else {
		m_fHP -= (float)nDamage;
		return true;
	}	
}

bool MObjectCharacter::InvokeActionPipe(int nPipeID, int nTime, MActParam* pActParam)
{
	bool bResult = false;

	switch(nPipeID) {
	case PIPE_ID_DEFENSE:
		{
			MDefensePipe* pPipe = (MDefensePipe*)FindActionPipe(nPipeID);
			if(pPipe)
				bResult = pPipe->Run(nTime, this, pActParam);
		}
		break;
	};
	return bResult;
}

void MObjectCharacter::RouteRepeatInfo(int nTime)
{
	#define TIME_CHAR_REPEATINFO	1000

	if (nTime - m_RepeatInfo.nLastTime < TIME_CHAR_REPEATINFO)
		return;

	if ( (m_RepeatInfo.fHP == GetHP()) &&
		 (m_RepeatInfo.fEN == GetEN()) ) 
		return;

	void* pArray = (MBLOB_BASICINFO_UPDATE*)MMakeBlobArray(sizeof(MBLOB_BASICINFO_UPDATE), 1);
	MBLOB_BASICINFO_UPDATE* pInfo= (MBLOB_BASICINFO_UPDATE*)MGetBlobArrayElement(pArray, 0);
	pInfo->uid = GetUID();
	pInfo->fHP = (float)GetHP();
	pInfo->fEN = (float)GetEN();

	MZoneServer* pZoneServer = MZoneServer::GetInstance();
	MCommand* pCmd = pZoneServer->CreateCommand(MC_OBJECT_BASICINFO_UPDATE, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterBlob(pArray, MGetBlobArraySize(pArray)));
	MEraseBlobArray(pArray);

	pZoneServer->RouteToRegion(GetMap()->GetID(), GetPos(), 2000.0f, pCmd);

	// Record current character info
	m_RepeatInfo.nLastTime = nTime;
	m_RepeatInfo.fHP = (float)GetHP();
	m_RepeatInfo.fEN = (float)GetEN();	
}
