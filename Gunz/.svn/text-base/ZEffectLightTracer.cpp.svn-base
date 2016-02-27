#include "stdafx.h"

#include <MMSystem.h>

#include "ZEffectLightTracer.h"
#include "RealSpace2.h"

#define LIGHTTRACTER_LENGTH	100
#define LIGHTTRACTER_WIDTH	1
#define LIGHTTRACER_SPEED	10.0f	// cm/msec

ZEffectLightTracer::ZEffectLightTracer(ZEffectBillboardSource* pEffectBillboardSource, D3DXVECTOR3& Start, D3DXVECTOR3& End)
: ZEffectBillboard(pEffectBillboardSource)
{
	m_nStartTime = timeGetTime();
	m_Start = Start;
	m_End = End;
	m_Scale.x = LIGHTTRACTER_LENGTH;
	m_Scale.y = LIGHTTRACTER_WIDTH;
	m_Scale.z = 1;

	m_nDrawMode = ZEDM_ADD;

	m_LightTracerDir = m_End-m_Start;
	m_fLength = D3DXVec3Length(&m_LightTracerDir);
	D3DXVec3Normalize(&m_LightTracerDir, &m_LightTracerDir);

	m_Pos = m_Start;
}
ZEffectLightTracer::~ZEffectLightTracer(void)
{
}

bool ZEffectLightTracer::Draw(unsigned long int nTime)
{
	DWORD dwDiff = nTime-m_nStartTime;

	D3DXVECTOR3 right;
	D3DXVec3Cross(&right, &m_LightTracerDir, &RealSpace2::RCameraDirection);
	D3DXVec3Cross(&m_Normal, &m_LightTracerDir, &right);
	D3DXVec3Cross(&m_Up, &m_Normal, &m_LightTracerDir);
	// 잘못된 수식, 수정 필요
	m_Pos += (m_LightTracerDir*LIGHTTRACER_SPEED*(float)dwDiff);
	m_fLength -= (LIGHTTRACER_SPEED*dwDiff);

	if(m_fLength<0) return false;

	ZEffectBillboard::Draw(nTime);

	return true;
}

