#ifndef ZEFFECTBULLETMARK_H
#define ZEFFECTBULLETMARK_H

#include "ZEffectBillboard.h"

class ZEffectBulletMark : public ZEffectBillboard{
protected:
	unsigned long int m_nStartTime;
public:
	ZEffectBulletMark(ZEffectBillboardSource* pEffectBillboardSource, D3DXVECTOR3& Pos, D3DXVECTOR3& Normal);
	virtual ~ZEffectBulletMark(void);

	virtual bool Draw(unsigned long int nTime);
	virtual bool DrawTest(unsigned long int nTime);
};

#endif
