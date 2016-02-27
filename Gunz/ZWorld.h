#ifndef _ZWORLD_H
#define _ZWORLD_H

#include "ZWater.h"
#include "ZMapDesc.h"
#include "ZClothEmblem.h"

// 앞으로 월드를 이루고 있는 것은 여기다 넣도록 합시다. (특히 그리는 것 관련된 것들)
// 게임 구동 관련은 ZGame에서 하고, 월드를 이루고 있는 것들이나, 그리는 것 관련 등은 여기서 했으면 좋겠습니다.
// 최종 목표는 ZWorld를 따로 띄어낼 수 있어서 어디서든지 쓸 수 있도록 하는것. (캐릭터 선택 화면, 게임 등등)

// ZGame에서 하나씩하나씩 시간날때마다 여기로 옮겨놓도록 하겠습니다. - bird

class ZWorldManager;
class ZMapDesc;
class ZSkyBox;
class ZLoadingProgress;

class ZWorld
{
	friend ZWorldManager;

private:
	// ZGame에서 여기로 이사올 예정인 것
	// 	RParticles			*m_pParticles;
	//	ZMapDesc			m_MapDesc;
	//	RBspObject			m_bsp;

	bool			m_bWaterMap;
	float			m_fWaterHeight;
	ZWaterList		m_waters;
	ZEmblemList		m_flags;
	ZSkyBox*		m_pSkyBox;

	bool	m_bFog; // 포그
	DWORD	m_dwFogColor;
	float	m_fFogNear;
	float	m_fFogFar;

	char		m_szName[64];
	char		m_szBspName[_MAX_PATH];
	RBspObject	*m_pBsp;
	ZMapDesc	*m_pMapDesc;

	int		m_nRefCount;	// 레퍼런스 카운트
	bool	m_bCreated;

	ZWorld();		// 클래스를 직접 생성하지 말것. ZWorldManager를 통해서 관리한다.
public:
	~ZWorld();

	void Update(float fDelta);
	void Draw();
	bool Create(ZLoadingProgress *pLoading );
	void Destroy();

	void OnInvalidate();
	void OnRestore();

	RBspObject	*GetBsp() { return m_pBsp; }
	ZMapDesc	*GetDesc() { return m_pMapDesc; }
	ZEmblemList	*GetFlags() { return &m_flags; }
	ZWaterList	*GetWaters() { return &m_waters; }

	void SetFog(bool bFog);
	bool IsWaterMap() { return m_bWaterMap; }
	float GetWaterHeight() { return m_fWaterHeight; }
	bool IsFogVisible()		{ return m_bFog; }
};

#endif