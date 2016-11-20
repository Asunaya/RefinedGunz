#pragma once

#include <memory>
#include <list>

#include "ZObject.h"
#include "stuff.h"
#include "BasicInfoHistory.h"

class ZShadow;

namespace RealSpace2
{
struct RLIGHT;
}

struct ZBasicInfoItem : public CMemPoolSm<ZBasicInfoItem>
{
	ZBasicInfo info;
	float fReceivedTime;
	float fSendTime;
};

using ZBasicInfoHistory = std::list<ZBasicInfoItem*>;

class ZCharacterObject : public ZObject
{
	MDeclareRTTI;
public:
	ZCharacterObject();
	~ZCharacterObject();

	void CreateShadow();

	bool GetWeaponTypePos(WeaponDummyType type,rvector* pos,bool bLeft=false);

	int GetWeapondummyPos(rvector* pVec);

	bool GetCurrentWeaponDirection(rvector* dir);

	void UpdateEnchant();

	void DrawEnchantSub(ZC_ENCHANT etype,rvector& pos);

	void DrawEnchant(ZC_STATE_LOWER AniState_Lower,bool bCharged);
	void EnChantWeaponEffect(ZC_ENCHANT etype,int nLevel);
	void EnChantSlashEffect(rvector* pOutPos,int cnt,ZC_ENCHANT etype,bool bDoubleWeapon);
	void EnChantMovingEffect(rvector* pOutPos,int cnt,ZC_ENCHANT etype,bool bDoubleWeapon);

	MMatchItemDesc* GetEnchantItemDesc();
	ZC_ENCHANT	GetEnchantType();

	void DrawShadow();
	void Draw_SetLight(const rvector& vPosition);

	bool IsDoubleGun();

	void SetHero(bool bHero = true) { m_bHero = bHero; }
	bool IsHero() const { return m_bHero; }

	virtual void OnKnockback(const rvector& dir, float fForce);
	void SetTremblePower(float fPower) { m_fTremblePower = fPower; }

	char	m_pSoundMaterial[16];
	bool	m_bLeftShot;
	float	m_fTime;
	int		m_iDLightType;
	float	m_fLightLife;
	rvector	m_vLightColor;

	std::unique_ptr<ZShadow> m_pshadow;
	bool m_bDynamicLight;

	ZModule_HPAP			*m_pModule_HPAP;
	ZModule_Resistance		*m_pModule_Resistance;
	ZModule_FireDamage		*m_pModule_FireDamage;
	ZModule_ColdDamage		*m_pModule_ColdDamage;
	ZModule_PoisonDamage	*m_pModule_PoisonDamage;
	ZModule_LightningDamage	*m_pModule_LightningDamage;

protected:
	bool m_bHero;

private:
	float m_fTremblePower;

	void SetGunLight();
};

class ZCharacterObjectHistory : public ZCharacterObject
{
	MDeclareRTTI;
public:
	virtual bool GetHistory(rvector *pos, rvector *direction, float fTime, rvector* camerapos = nullptr) override;
	void EmptyHistory();

	ZBasicInfoHistory	m_BasicHistory;
};