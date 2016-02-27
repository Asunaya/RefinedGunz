#include "MActParam.h"
#include "MActionPipe.h"
#include "MAction.h"
#include "MObjectCharacter.h"
#include "MZoneServer.h"
#include "MSharedCommandTable.h"
#include "MSkillDesc.h"


/*
pseudo MObject::OnInit()
{
	MActionPipe* pPipe = FindPipe(PIPE_ID_OFFENSE);
	pPipe->ClearAction();
	pPipe->AddAction(GetAction(ACT_DEFENSE));
}

pseudo MObject::Attack()
{
	MActionPipe* pPipe = FindPipe(PIPE_ID_ACTIVE);
	pPipe->ClearAction();
	pPipe->AddAction(GetAction(ACT_ATTACK));
	pPipe->Start();
}

pseudo MObject::Tick(unsigned long int nTime)
{
	for (int i=0; i<GetPipeCount(); i++) {
		MActionPipe* pPipe = GetPipe(i);

		if (pPipe->CheckTurn())
			pPipe->Run(nTime, GetActParam());
	}
}
*/

////////////////////////////////////////////////
// MActionPipe
MActionPipe::MActionPipe()
{
	SetState(PIPE_STATE_STOPPED);
}

MActionPipe::~MActionPipe()
{
	ClearAction();
}

void MActionPipe::AddAction(MAction* pAction)
{
	m_ActionList.push_back(pAction);
}

void MActionPipe::DelAction(int nID)
{
	for(list<MAction*>::iterator i=m_ActionList.begin(); i!=m_ActionList.end();) {
		MAction* pAct = *i;
		if (pAct->GetID() == nID) {
			m_ActionList.erase(i);
			return;
		}
	}
}

void MActionPipe::ClearAction()
{
	m_ActionList.clear();
}

bool MActionPipe::Run(int nTime, MObject* pActor, MActParam* pParam)
{
	for(list<MAction*>::iterator i=m_ActionList.begin(); i!=m_ActionList.end();) {
		MAction* pAct = *i;

		bool bActResult = pAct->Run(nTime, pActor, pParam);

		if (pAct->GetType() == ACTION_TYPE_ONCE)
			i = m_ActionList.erase(i);	
		else
			++i;
		
		if (bActResult == true)
			continue;
		else
			break;
	}

	return true;
}


////////////////////////////////////////////////
// MActionPipe
MActivePipe::MActivePipe()
{
	SetID(PIPE_ID_ACTIVE);

	SetPrimaryInterval(4000);
	SetSecondaryInterval(0);

	SetLastPrimaryRunTime(0);
	SetLastSecondaryRunTime(0);
}

MActivePipe::~MActivePipe()
{
}

bool MActivePipe::CheckTurn(int nTime)
{
	if ((GetState() == PIPE_STATE_STOPPED) || (GetState() == PIPE_STATE_PENDING))
		return false;
	if (GetActionCount() <= 0)
		return false;

	if (GetPrimaryInterval() != 0)
	if (nTime - GetLastPrimaryRunTime() >= GetPrimaryInterval())
		return true;

	if (GetSecondaryInterval() != 0)
	if (nTime - GetLastSecondaryRunTime() >= GetSecondaryInterval())
		return true;

	return false;
}

bool MActivePipe::Run(int nTime, MObject* pActor, MActParam* pParam)
{
	if (nTime - GetLastPrimaryRunTime() >= GetPrimaryInterval())
		SetLastPrimaryRunTime(nTime);
	if (nTime - GetLastSecondaryRunTime() >= GetSecondaryInterval())
		SetLastSecondaryRunTime(nTime);

	return MActionPipe::Run(nTime, pActor, pParam);
}

void MActivePipe::GiveDelayPenalty(int nTime, int nDelay)
{
	SetLastPrimaryRunTime(nTime+nDelay);
	SetLastSecondaryRunTime(nTime+nDelay);
}

////////////////////////////////////////////////
// MSkillPipe
MSkillPipe::MSkillPipe()
{
	SetID(PIPE_ID_SKILL);

	SetInterval(100);
	SetLastRunTime(0);
}

MSkillPipe::~MSkillPipe()
{
}

bool MSkillPipe::CheckTurn(int nTime)
{
	if ((GetState() == PIPE_STATE_STOPPED) || (GetState() == PIPE_STATE_PENDING))
		return false;
	if (GetActionCount() <= 0)
		return false;

	if (GetInterval() != 0)
	if (nTime - GetLastRunTime() >= GetInterval())
		return true;

	return false;
}

bool MSkillPipe::Run(int nTime, MObject* pActor, MActParam* pParam)
{
	if (nTime - GetLastRunTime() >= GetInterval())
		SetLastRunTime(nTime);

	if (CheckStateReady(MSkillPipe::SKILLSTATE_ENTER)) {
		SetTimeStamp(nTime);
		OnEnter(nTime, pActor, pParam);
		SetNextSkillState(MSkillPipe::SKILLSTATE_EXCUTE);
	}
	if (CheckStateReady(MSkillPipe::SKILLSTATE_EXCUTE)) {
		if (nTime >= GetTimeStamp()+GetEnterDelay()) {
			SetTimeStamp(nTime);
			OnExcute(nTime, pActor, pParam);
			SetNextSkillState(MSkillPipe::SKILLSTATE_EXIT);
		}
	}
	if (CheckStateReady(MSkillPipe::SKILLSTATE_EXIT)) {
		if (nTime >= GetTimeStamp()+GetExitDelay()) {
			OnExit(nTime, pActor, pParam);
			SetNextSkillState(MSkillPipe::SKILLSTATE_IDLE);
		}
	}
	if (CheckStateReady(MSkillPipe::SKILLSTATE_IDLE)) {
		Stop();
	}
	return true;
}

bool MSkillPipe::OnEnter(int nTime, MObject* pActor, MActParam* pParam)
{
	if (pParam->GetType()!=ACTPARAM_TYPE_SKILL) {
		OutputDebugString("MSkillPipe::OnEnter() Invalid ActParam \n");
		return false;
	}
	MActParamSkill* pSkillParam = (MActParamSkill*)pParam;
	MObjectCharacter* pTarget = (MObjectCharacter*)pSkillParam->GetTarget();
	if (pTarget == NULL) {
		OutputDebugString("MSkillPipe::OnEnter() Invalid ActParam Target \n");
		return false;
	}

	// Ref SkillDesc
	MSkillDesc* pSkill = g_SkillDescManager.Find(pSkillParam->GetSkillID());
	if (pSkill == NULL) {
		OutputDebugString("MSkillPipe::OnEnter() Unknown SkillID \n");
		return false;
	}
	if (pSkill->nActivationTime > 0)
		SetEnterDelay(pSkill->nActivationTime);

	MZoneServer* pZoneServer = MZoneServer::GetInstance();
	MCommand* pCmd = pZoneServer->CreateCommand(MC_OBJECT_SKILL, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(pActor->GetUID()));
	pCmd->AddParameter(new MCommandParameterUID(pTarget->GetUID()));
	pCmd->AddParameter(new MCommandParameterInt(pSkillParam->GetSkillID()));
	pZoneServer->RouteToRegion(pActor->GetMap()->GetID(), pActor->GetPos(), 2000.0f, pCmd);

	return true;
}

bool MSkillPipe::OnExcute(int nTime, MObject* pActor, MActParam* pParam)
{
	if (pParam->GetType()!=ACTPARAM_TYPE_SKILL) {
		OutputDebugString("MSkillPipe::OnEnter() Invalid ActParam \n");
		return false;
	}
	MActParamSkill* pSkillParam = (MActParamSkill*)pParam;
	MObjectCharacter* pTarget = (MObjectCharacter*)pSkillParam->GetTarget();
	if (pTarget == NULL) {
		OutputDebugString("MSkillPipe::OnEnter() Invalid ActParam Target \n");
		return false;
	}

	// Ref SkillDesc
	MSkillDesc* pSkill = g_SkillDescManager.Find(pSkillParam->GetSkillID());
	if (pSkill == NULL) {
		OutputDebugString("MSkillPipe::OnEnter() Unknown SkillID \n");
		return false;
	}
	if (pSkill->nReactivationTime > 0)
		SetExitDelay(pSkill->nReactivationTime);

	for(list<MAction*>::iterator i=m_ActionList.begin(); i!=m_ActionList.end();i++) {
		MAction* pAct = *i;

		if (pAct->Run(nTime, pActor, pParam) == true)
			continue;
		else
			break;
	}
	return true;
}

bool MSkillPipe::OnExit(int nTime, MObject* pActor, MActParam* pParam)
{
	MActionPipe* pActivePipe = ((MObjectCharacter*)pActor)->FindActionPipe(PIPE_ID_ACTIVE);
	if (pActivePipe->GetState() == PIPE_STATE_PENDING)
		pActivePipe->Start();
	return true;
}


////////////////////////////////////////////////
// MDefensePipe
MDefensePipe::MDefensePipe()
{
	SetID(PIPE_ID_DEFENSE);
}

MDefensePipe::~MDefensePipe()
{
}

bool MDefensePipe::CheckTurn(int nTime)
{
	return false;
}
