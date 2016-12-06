#include "stdafx.h"

#include <MMSystem.h>

#include "ZEffectLightTracer.h"
#include "RealSpace2.h"

#define LIGHTTRACTER_LENGTH	100
#define LIGHTTRACTER_WIDTH	1
#define LIGHTTRACER_SPEED	10.0f	// cm/msec

ZEffectLightTracer::ZEffectLightTracer(ZEffectBillboardSource* pEffectBillboardSource, const rvector& Start, const rvector& End)
: ZEffectBillboard(pEffectBillboardSource)
{
	m_nStartTime = GetGlobalTimeMS();
	m_Start = Start;
	m_End = End;
	m_Scale.x = LIGHTTRACTER_LENGTH;
	m_Scale.y = LIGHTTRACTER_WIDTH;
	m_Scale.z = 1;

	m_nDrawMode = ZEDM_ADD;

	m_LightTracerDir = m_End-m_Start;
	m_fLength = Magnitude(m_LightTracerDir);
	Normalize(m_LightTracerDir);

	m_Pos = m_Start;
}
ZEffectLightTracer::~ZEffectLightTracer(void)
{
}

bool ZEffectLightTracer::Draw(unsigned long int nTime)
{
	DWORD dwDiff = nTime-m_nStartTime;

	rvector right = CrossProduct(m_LightTracerDir, RCameraDirection);
	m_Normal = CrossProduct(m_LightTracerDir, right);
	m_Up = CrossProduct(m_Normal, m_LightTracerDir);
	m_Pos += (m_LightTracerDir*LIGHTTRACER_SPEED*(float)dwDiff);
	m_fLength -= (LIGHTTRACER_SPEED*dwDiff);

	if(m_fLength<0) return false;

	ZEffectBillboard::Draw(nTime);

	return true;
}

