#ifndef ZEFFECTSMOKE_H
#define ZEFFECTSMOKE_H

#include "ZEffectBillboard.h"
#include "RTypes.h"

#include "mempool.h"

// 사용안하는것들은 지운다..

class ZEffectSmoke : public ZEffectBillboard , public CMemPoolSm<ZEffectSmoke>
{
protected:
	unsigned long int m_nStartTime;
	float m_fMinScale;
	float m_fMaxScale;
	unsigned long int m_nLifeTime;
	rvector	m_OrigPos;
	rvector	m_Velocity;
public:
	ZEffectSmoke(ZEffectBillboardSource* pEffectBillboardSource, rvector& Pos, rvector& Velocity, float fMinScale, float fMaxScale, unsigned long int nLifeTime);
	virtual ~ZEffectSmoke(void);

	virtual bool Draw(unsigned long int nTime);
};

class ZEffectLandingSmoke : public ZEffectBillboard , public CMemPoolSm<ZEffectLandingSmoke>
{
protected:
	unsigned long int m_nStartTime;
	float m_fMinScale;
	float m_fMaxScale;
	unsigned long int m_nLifeTime;
	rvector	m_OrigPos;
	rvector	m_Velocity;

public:
	ZEffectLandingSmoke(ZEffectBillboardSource* pEffectBillboardSource, rvector& Pos, rvector& Velocity, float fMinScale, float fMaxScale, unsigned long int nLifeTime);
	virtual ~ZEffectLandingSmoke(void);

	virtual bool Draw(unsigned long int nTime);
};
 
class ZEffectSmokeGrenade : public ZEffectBillboard , public CMemPoolSm<ZEffectSmokeGrenade>
{
protected:
	unsigned long int m_nStartTime;
	float m_fMinScale;
	float m_fMaxScale;
	unsigned long int m_nLifeTime;
	rvector	m_OrigPos;
	rvector	m_Velocity;

public:
	virtual bool Draw( unsigned long int nTime );

public:
	ZEffectSmokeGrenade(ZEffectBillboardSource* pEffectBillboardSource, rvector& Pos, rvector& Velocity, float fMinScale, float fMaxScale, unsigned long int nLifeTime);
	virtual ~ZEffectSmokeGrenade();
};


#endif