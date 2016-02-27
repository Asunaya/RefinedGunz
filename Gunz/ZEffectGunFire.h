#ifndef ZEFFECTGUNFIRE_H
#define ZEFFECTGUNFIRE_H

#include "ZEffectManager.h"
#include "ZEffectBillboard.h"
/*
class ZCharacter;

class ZEffectGunFire : public ZEffect{
protected:
	unsigned long int m_nStartTime;
	ZEffectBillboardSource*	m_pSources[2];
	D3DXVECTOR3	m_Pos;
	D3DXVECTOR3	m_Dir;

	float	m_nStartAddTime;

	bool		m_isMovingPos;
	MUID		m_uid;
//	ZCharacter* m_pCharacter;
	bool		m_isLeftWeapon;

public:
	ZEffectGunFire(ZEffectBillboardSource*	pSources[2], D3DXVECTOR3& Pos, D3DXVECTOR3& Dir,ZCharacter* pChar);
	virtual ~ZEffectGunFire(void);

	virtual bool Draw(unsigned long int nTime);

	virtual rvector GetSortPos() {
		return m_Pos;
	}

	void SetStartTime(DWORD _time) {
		m_nStartAddTime = _time;
	}

	void SetIsLeftWeapon(bool b) {
		m_isLeftWeapon = b;
	}
};

// shotgun

class ZEffectGunFire2 : public ZEffect{
protected:
	unsigned long int m_nStartTime;
	ZEffectBillboardSource*	m_pSources[4];

	D3DXVECTOR3	m_Pos;
	D3DXVECTOR3	m_Dir;

	bool		m_isMovingPos;
	MUID		m_uid;
	bool		m_isLeftWeapon;

public:
	ZEffectGunFire2(ZEffectBillboardSource*	pSources[4], D3DXVECTOR3& Pos, D3DXVECTOR3& Dir,ZCharacter* pChar);
	virtual ~ZEffectGunFire2(void);

	virtual bool Draw(unsigned long int nTime);

	virtual rvector GetSortPos() {
		return m_Pos;
	}

	void SetIsLeftWeapon(bool b) {
		m_isLeftWeapon = b;
	}
};
*/
#endif