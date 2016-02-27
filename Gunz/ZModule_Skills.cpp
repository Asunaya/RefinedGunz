#include "stdafx.h"
#include "ZModule_Skills.h"

ZModule_Skills::ZModule_Skills()
{
}

void ZModule_Skills::Init(int nSkills, const int *pSkillIDs)
{
	m_nSkillCount = min(nSkills,MAX_SKILL);

	for(int i=0;i<m_nSkillCount;i++)
	{
		m_Skills[i].Init(pSkillIDs[i],(ZObject*)m_pContainer);
	}
}

void ZModule_Skills::InitStatus()
{
	for(int i=0;i<m_nSkillCount;i++)
	{
		m_Skills[i].InitStatus();
	}
	Active(false);
}

bool ZModule_Skills::Update(float fElapsed)
{
	bool bActive = false;
	for(int i=0;i<m_nSkillCount;i++)
	{
		bActive|=m_Skills[i].Update(fElapsed);
	}

	return bActive;	// 활성화 되어있는게 없으면 return false. this도 비활성화된다
}

int ZModule_Skills::GetSkillCount()
{
	return m_nSkillCount;
}

ZSkill *ZModule_Skills::GetSkill(int nSkill)	// 스킬을 얻어낸다.
{
	if(nSkill<0 || nSkill>=MAX_SKILL) return 0;
	return &m_Skills[nSkill];
}

void ZModule_Skills::PreExcute(int nSkill,MUID uidTarget,rvector targetPosition)
{
	if(nSkill<0 || nSkill>=MAX_SKILL) return;
	m_Skills[nSkill].PreExecute(uidTarget,targetPosition);
}

void ZModule_Skills::Excute(int nSkill,MUID uidTarget,rvector targetPosition)		// 실행한다
{
	if(nSkill<0 || nSkill>=MAX_SKILL) return;
	m_Skills[nSkill].Execute(uidTarget,targetPosition);
	
	Active();
}

void ZModule_Skills::LastExcute(int nSkill,MUID uidTarget,rvector targetPosition)
{
	if(nSkill<0 || nSkill>=MAX_SKILL) return;
	m_Skills[nSkill].LastExecute(uidTarget,targetPosition);
}
