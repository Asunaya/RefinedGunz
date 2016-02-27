#include "MFormation.h"

int MFormation::Add(rvector& Pos, float fMaxVelocity)
{
	MFORMATIONUNIT* pUnit = new MFORMATIONUNIT;
	pUnit->Pos = Pos;
	pUnit->fMaxVelocity = fMaxVelocity;
	m_Units.Add(pUnit);
	return (m_Units.GetCount() - 1);
}

void MFormation::Update(int i, rvector& Pos, float fMaxVelocity)
{
	MFORMATIONUNIT* pUnit = m_Units.Get(i);
	pUnit->Pos = Pos;
	pUnit->fMaxVelocity = fMaxVelocity;
}

void MFormation::Del(int i)
{
	m_Units.Delete(i);
}

void MFormation::Update(void)
{
	rvector Pos(0, 0, 0);
	float fMaxVelocity = 0;
	int nCount = m_Units.GetCount();
	for(int i=0; i<nCount; i++){
		MFORMATIONUNIT* pUnit = m_Units.Get(i);
		Pos += pUnit->Pos;
		fMaxVelocity += pUnit->fMaxVelocity;
	}

	m_Pos = Pos * (1.0f/nCount);
	m_fMaxVelocity = fMaxVelocity / nCount;
}

void MFormation::Get(rvector* pPos, float* pMaxVelocity, rvector& Target)
{
	rvector Diff = Target - m_Pos;
}
