#ifndef MFORMATION_H
#define MFORMATION_H

#include "CMList.h"
#include "rutils.h"

struct MFORMATIONUNIT{
	rvector Pos;
	float fMaxVelocity;
};

class MFormation{
	CMLinkedList<MFORMATIONUNIT>	m_Units;
	rvector	m_Pos;
	float	m_fMaxVelocity;
public:
	int Add(rvector& Pos, float fMaxVelocity);
	void Update(int i, rvector& Pos, float fMaxVelocity);
	void Del(int i);

	void Update(void);

	void Get(rvector* pPos, float* pMaxVelocity, rvector& Target);
};

#endif