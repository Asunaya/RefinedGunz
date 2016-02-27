#ifndef _ZMODULE_POISONDAMAGE_H
#define _ZMODULE_POISONDAMAGE_H

#include "ZModule.h"
#include "ZModuleID.h"

class ZModule_PoisonDamage : public ZModule {
	float	m_fBeginTime;	// 데미지 시작한 시간
	float	m_fNextDamageTime;	// 다음번 데미지를 받을 시간
	float	m_fNextEffectTime;	// 다음번 이펙트 뿌릴 시간

	int		m_nDamage;		// 단위시간당 데미지
	float	m_fDuration;	// 지속시간
	MUID	m_Owner;		// 데미지를 준 사람
public:
	DECLARE_ID(ZMID_POISONDAMAGE)
	ZModule_PoisonDamage();

	virtual bool Update(float fElapsed);
	virtual void InitStatus();

	void BeginDamage(MUID owner, int nDamage, float fDuration);	// 데미지를 주기 시작한다
};

#endif