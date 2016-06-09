#ifndef _ZCHARACTEROBJECT_H
#define _ZCHARACTEROBJECT_H

#include "ZObject.h"
#include "stuff.h"

// ZCharacter 와 ZActor 가 같이쓰는것들을 모은 class ..

class ZShadow;

struct ZBasicInfoItem : public CMemPoolSm<ZBasicInfoItem>
{
	ZBasicInfo info;
	float	fReceivedTime;
	float	fSendTime;			// 보낸시간 (ping으로 추정)
};

class ZBasicInfoHistory : public list<ZBasicInfoItem*> {
};


class ZCharacterObject : public ZObject
{
	MDeclareRTTI;
private:
	float			m_fTremblePower;	///< 총에 맞을때 몸을 떠는 정도
public:
	ZCharacterObject();
	virtual ~ZCharacterObject();

public:

	bool CreateShadow();
	void DestroyShadow();

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

	void DrawShadow();		// Manager에서 호출한다.
	void Draw_SetLight(rvector& vPosition);

	bool IsDoubleGun();

	void SetHero(bool bHero = true) { m_bHero = bHero; }
	bool IsHero() { return m_bHero; }

	void EmptyHistory();
	virtual bool GetHistory(rvector *pos,rvector *direction,float fTime); // 판정/옵저버등 과거의 데이터를 얻는다

protected:

	bool	m_bHero;					///< 내가 조종하는 사람인지 여부

public:

	ZBasicInfoHistory	m_BasicHistory;		///< 판정을 위해 몇초간의 데이터를 가지고있는다

	char	m_pSoundMaterial[16];

	bool	m_bLeftShot;				// 양손 총일 경우 번갈아 가면서 번쩍..번쩍..

	float	m_fTime;					// 시스템 시간

	int		m_iDLightType;				// 라이트의 종류
	float	m_fLightLife;				// 라이트 적용 시간
	rvector	m_vLightColor;				// 라이트 색..

	ZShadow*	m_pshadow;				//그림자
	bool		m_bDynamicLight;		// 추가적인 라이트 효과여부

	// knockback을 적용받아야한다
	virtual void OnKnockback(const rvector& dir, float fForce);
	void SetTremblePower(float fPower)		{ m_fTremblePower = fPower; }
};

#endif//_ZCHARACTEROBJECT_H