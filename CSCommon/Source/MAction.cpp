#include "MAction.h"
#include "MActParam.h"
#include "MActionPipe.h"
#include "MObjectCharacter.h"
#include "MSkillDesc.h"


bool MActionAttack::Run(int nTime, MObject* pActor, MActParam* pActParam)
{
	if ((pActParam == NULL) || (pActParam->GetType()!=ACTPARAM_TYPE_DAMAGE)) {
		OutputDebugString("MActionAttack::Run() Invalid ActParam \n");
		return false;
	}

	MObjectCharacter* pTarget = (MObjectCharacter*)pActParam->GetTarget();
	if (pTarget == NULL) {
		OutputDebugString("MActionAttack::Run() Invalid ActParam Target \n");
		return false;
	}

	pTarget->Warning((MObjectCharacter*)pActor);

	// Actor State 체크 ( Sleep, Sturn 등)
	// Attack Miss 판정
	// 공격횟수 산정
	// Damage 산정
	int nDamage = 15;

	if (pActor->HasCommListener())
		nDamage += 7;

	// 같은 Target을 여러대 때리면 Process 여러개 Add후 Target당 InvokeDefense하나씩
	MActParamDamage* pDamage = new MActParamDamage();
	pDamage->SetTarget(pTarget);
	pDamage->SetDamage(nDamage);

	pTarget->InvokeActionPipe(PIPE_ID_DEFENSE, nTime, pDamage);

	bool bAlive = pTarget->Damage(pDamage->GetDamage(), pActor->GetUID());

	if (bAlive == false) {	// Dead already 
		((MObjectCharacter*)pActor)->StopAttack();
		return false;
	}

	return true;
}

bool MActionDefense::Run(int nTime, MObject* pActor, MActParam* pActParam)
{
	if (pActParam->GetType()!=ACTPARAM_TYPE_DAMAGE) {
		OutputDebugString("MActionDefense::Run() Invalid ActParam \n");
		return false;
	}

	MActParamDamage* pDamage = (MActParamDamage*)pActParam;
	int nDamage = pDamage->GetDamage();
	if (rand()%3==1)
		nDamage = nDamage/2;
	pDamage->SetDamage(nDamage);

	return true;
}

bool MActionSkill::Run(int nTime, MObject* pActor, MActParam* pActParam)
{
	if (pActParam->GetType()!=ACTPARAM_TYPE_SKILL) {
		OutputDebugString("MActionSkill::Run() Invalid ActParam \n");
		return false;
	}

	MSkillDesc*	pSkillDesc = g_SkillDescManager.Find( ((MActParamSkill*)pActParam)->GetSkillID() );
	if (pSkillDesc == NULL) {
		OutputDebugString("MActionSkill::Run() No SkillDesc found. \n");
		return false;
	}

	MObjectCharacter* pChar = (MObjectCharacter*)pActor;
	MObjectCharacter* pTarget = (MObjectCharacter*)pActParam->GetTarget();
	if (pTarget == NULL) {
		OutputDebugString("MActionAttack::Run() Invalid ActParam Target \n");
		return false;
	}

	// Cost 적용
	if (pSkillDesc->nHP)
		pChar->UpdateHP((float)pSkillDesc->nHP);
	if (pSkillDesc->nEN)
		pChar->UpdateEN((float)pSkillDesc->nEN);

	// MOD 적용
	if (pSkillDesc->ExamMODFlag(MSF_MOD_HEALTH)) {
		pTarget->UpdateHP(pSkillDesc->MOD_HP);
		pTarget->UpdateEN(pSkillDesc->MOD_EN);
	}
/*	if (pSkillDesc->ExamMODFlag(MSF_MOD_STAT)) {
	}
	if (pSkillDesc->ExamMODFlag(MSF_MOD_RESIST)) {
	}
	if (pSkillDesc->ExamMODFlag(MSF_MOD_DEFENSE)) {
	}
	if (pSkillDesc->ExamMODFlag(MSF_MOD_OFFENSE)) {
	}
	if (pSkillDesc->ExamMODFlag(MSF_MOD_NORMAL)) {
	}
	if (pSkillDesc->ExamMODFlag(MSF_MOD_STATUS)) {
	}
	if (pSkillDesc->ExamMODFlag(MSF_MOD_PRIMA)) {
	}
	if (pSkillDesc->ExamMODFlag(MSF_MOD_KEY)) {
	}
*/
	return true;
}

