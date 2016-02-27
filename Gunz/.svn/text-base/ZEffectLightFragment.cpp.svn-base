#include "stdafx.h"

#include <MMSystem.h>
#include "ZEffectLightFragment.h"
#include "RealSpace2.h"
#include "Physics.h"

#define LIGHTTRACTER_LENGTH	4.0f
#define LIGHTTRACTER_WIDTH	0.8f
#define LIGHTFRAGMENT_LIFETIME	600

ZEffectLightFragment::ZEffectLightFragment(ZEffectBillboardSource* pEffectBillboardSource, rvector& Pos, rvector& Velocity)
: ZEffectBillboard(pEffectBillboardSource)
{
	m_nStartTime = timeGetTime();
	m_nPrevTime = m_nStartTime;
	m_OrigPos = m_Pos = Pos;
	m_Velocity = Velocity;
	m_Scale.x = LIGHTTRACTER_LENGTH;
	m_Scale.y = LIGHTTRACTER_WIDTH;
	m_Scale.z = 1;

	m_nDrawMode = ZEDM_ADD;
}
ZEffectLightFragment::~ZEffectLightFragment(void)
{
}

bool ZEffectLightFragment::Draw(unsigned long int nTime)
{
	DWORD dwDiff = nTime-m_nStartTime;
	DWORD dwPrevDiff = nTime-m_nPrevTime;

	float fSec = (float)dwDiff/1000.0f;
	rvector Distance = ParabolicMotion(m_Velocity, fSec) * 100;	// *100은 미터에서 센티로 변환
	rvector NewPos = m_OrigPos + Distance;
	rvector Acceleration = NewPos - m_Pos;
	m_Pos = NewPos;
	m_fOpacity = (LIGHTFRAGMENT_LIFETIME-dwDiff)/(float)LIGHTFRAGMENT_LIFETIME;

	rvector right;
	D3DXVec3Cross(&right, &Acceleration, &RealSpace2::RCameraDirection);
	D3DXVec3Cross(&m_Normal, &Acceleration, &right);
	D3DXVec3Cross(&m_Up, &m_Normal, &Acceleration);

	if(dwDiff>LIGHTFRAGMENT_LIFETIME) return false;

	ZEffectBillboard::Draw(nTime);
	m_nPrevTime = nTime;

	return true;
}

ZEffectLightFragment2::ZEffectLightFragment2(LPDIRECT3DTEXTURE9 pEffectBillboardTexture, rvector& Pos, rvector& Velocity)
: ZEffectBillboard2(pEffectBillboardTexture)
{
	m_nStartTime = timeGetTime();
	m_nPrevTime = m_nStartTime;
	m_OrigPos = m_Pos = Pos;
	m_Velocity = Velocity;
	m_Scale.x = LIGHTTRACTER_WIDTH;
	m_Scale.y = LIGHTTRACTER_WIDTH;
	m_Scale.z = 1;

	m_nDrawMode = ZEDM_ALPHAMAP;
}
ZEffectLightFragment2::~ZEffectLightFragment2(void)
{
}

bool ZEffectLightFragment2::Draw(unsigned long int nTime)
{
	DWORD dwDiff = nTime-m_nStartTime;
	DWORD dwPrevDiff = nTime-m_nPrevTime;

	float fSec = (float)dwDiff/1000.0f;
	rvector Distance = ParabolicMotion(m_Velocity, fSec) * 100;	// *100은 미터에서 센티로 변환
	rvector NewPos = m_OrigPos + Distance;
	rvector Acceleration = NewPos - m_Pos;
	m_Pos = NewPos;
	m_fOpacity = (LIGHTFRAGMENT_LIFETIME-dwDiff)/(float)LIGHTFRAGMENT_LIFETIME;

	rvector right;
	D3DXVec3Cross(&right, &Acceleration, &RealSpace2::RCameraDirection);
	D3DXVec3Cross(&m_Normal, &Acceleration, &right);
	D3DXVec3Cross(&m_Up, &m_Normal, &Acceleration);

	if(dwDiff>LIGHTFRAGMENT_LIFETIME) return false;

	ZEffectBillboard2::Draw(nTime);
	m_nPrevTime = nTime;

	return true;
}

