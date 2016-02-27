#ifndef _ZITEM_H
#define _ZITEM_H

#include "MMatchItem.h"


// 이거 나중에 MBaseItem을 부모로 하도록 바뀌어야 한다.
class ZItem : public MMatchItem
{
private:
protected:
	int		m_nBullet;		// 총 탄알수 현재 탄창에 들어있는 탄알수는 뺀다. - weapon에만 사용하는 변수
	int		m_nBulletAMagazine;	// 현재 탄창에 들어있는 탄알수 - weapon에만 사용하는 변수
public:
	ZItem();
	virtual ~ZItem();

	// weapon에서만 쓰이는 함수
	void InitBullet(int nBullet);
	bool Shot();
	bool Reload();
	bool isReloadable();

	int GetBullet()				{ return m_nBullet; }
	void SetBullet(int nBullet) { m_nBullet = nBullet; }
	int GetBulletAMagazine()	{ return m_nBulletAMagazine; }
	void SetBulletAMagazine(int nBulletAMagazine)	{ m_nBulletAMagazine = nBulletAMagazine; }

	int* GetBulletPointer()		{ return &m_nBullet; }
	int* GetAMagazinePointer()	{ return &m_nBulletAMagazine; }

	static float GetPiercingRatio(MMatchWeaponType wtype,RMeshPartsType partstype);
	float GetKnockbackForce();
};


#endif