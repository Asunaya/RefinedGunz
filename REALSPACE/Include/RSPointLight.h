// RSPointLight.h
#ifndef __RSPOINTLIGHT_H
#define __RSPOINTLIGHT_H

#include "rstypes.h"

class RSPointLight  
{
public:
	RSPointLight();
	RSPointLight(rvector Position,rvector Color,float Range, RSLIGHTSTYLE style = RS_LIGHTSTYLE_VERTEX );
	virtual ~RSPointLight();

	void SetPosition(rvector &p) { mPosition=p; }
	void SetColor(rvector &c) { mColor=c; }
	void SetRange(float r) { mRange=r; }
	void SetStyle(RSLIGHTSTYLE style) { m_Style=style; }

	inline rvector	GetPosition() { return mPosition; }
	inline rvector	GetColor() { return mColor; }
	inline float	GetRange() { return mRange; }
	inline RSLIGHTSTYLE	GetStyle() { return m_Style; }

	BOOL BoundingBoxTest(rplane *planes);

	rvector mPosition,mColor;
	float	mRange;
	RSLIGHTSTYLE m_Style;
};

#endif
