#ifndef ZEFFECTLIGHTTRACER_H
#define ZEFFECTLIGHTTRACER_H

#include "ZEffectBillboard.h"

#include "mempool.h"

// 사용안하는것들은 지운다..

class ZEffectLightTracer : public ZEffectBillboard , public CMemPoolSm<ZEffectLightTracer>
{
protected:
	unsigned long int m_nStartTime;

	rvector	m_LightTracerDir;
	rvector	m_Start, m_End;
	float		m_fLength;

public:
	ZEffectLightTracer(ZEffectBillboardSource* pEffectBillboardSource, const rvector& Start, const rvector& End);
	virtual ~ZEffectLightTracer(void);

	virtual bool Draw(unsigned long int nTime);
};

#endif