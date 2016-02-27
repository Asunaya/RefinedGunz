#include "stdafx.h"

#include <MMSystem.h>

#include "ZEffectBulletMark.h"

#define BULLETMARK_SCALE	5.0f

static int g_bullet_mark_cnt = 0;

ZEffectBulletMark::ZEffectBulletMark(ZEffectBillboardSource* pEffectBillboardSource, D3DXVECTOR3& Pos, D3DXVECTOR3& Normal)
: ZEffectBillboard(pEffectBillboardSource)
{
	m_nStartTime = timeGetTime();
	m_Pos = Pos;
	m_Normal = Normal;
	m_Scale.x = m_Scale.y = m_Scale.z = BULLETMARK_SCALE;
	m_nDrawMode = ZEDM_ALPHAMAP;

//	g_bullet_mark_cnt++;
}

ZEffectBulletMark::~ZEffectBulletMark(void)
{
//	g_bullet_mark_cnt--;
}

#define BULLETMARK_LIFE_TIME		10000		// BULLETMARK Life Time
#define BULLETMARK_VANISH_TIME		1000		// BULLETMARK Life Time

bool ZEffectBulletMark::Draw(unsigned long int nTime)
{
	DWORD dwDiff = nTime-m_nStartTime;
	//m_fOpacity = (BULLETMARK_LIFE_TIME-dwDiff)/(float)BULLETMARK_LIFE_TIME;

	if(BULLETMARK_LIFE_TIME-dwDiff<BULLETMARK_VANISH_TIME){
		m_fOpacity = (BULLETMARK_LIFE_TIME-dwDiff)/(float)BULLETMARK_VANISH_TIME;
	}
	if(dwDiff>BULLETMARK_LIFE_TIME) return false;

	ZEffectBillboard::Draw(nTime);

//	static char buffer[40];
//	sprintf(buffer,"ZEffectBulletMark::Draw : %d \n",g_bullet_mark_cnt);
//	OutputDebugString(buffer);

	return true;
}

bool ZEffectBulletMark::DrawTest(unsigned long int nTime)
{
	ZEffectBillboard::Draw(nTime);
	return true;
}
