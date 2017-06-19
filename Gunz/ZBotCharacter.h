#pragma once

#include "ZMyCharacter.h"
#include "BasicInfo.h"

class ZBotCharacter : public ZCharacter
{
public:
	ZBotCharacter()
	{
		m_pModule_HPAP->SetRealDamage(true);
	}

	void OnUpdate(float) override;
	virtual void OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType,
		float fDamage, float fPiercingRatio = 1.f, int nMeleeType = -1) override;

	u64 LastTime = 0;

private:
	template <typename... T>
	void PostBotCmd(int ID, T&&... Args);
	void CheckDead();
	void PostBasicInfo();

	BasicInfoNetState BasicInfoState;
};