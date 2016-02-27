// RSPointLight.cpp: implementation of the RSPointLight class.
//
//////////////////////////////////////////////////////////////////////

#include "RSPointLight.h"

RSPointLight::RSPointLight() {}
RSPointLight::~RSPointLight() {}

RSPointLight::RSPointLight(rvector Position,rvector Color,float range, RSLIGHTSTYLE style)
{	
	mPosition=Position;
	mColor=Color;
	mRange=max(range,2.f);
	m_Style=style;
}

BOOL RSPointLight::BoundingBoxTest(rplane *planes)
{
	if( (GetDistance(mPosition,planes[0])>-mRange)&&
		(GetDistance(mPosition,planes[1])>-mRange)&&
		(GetDistance(mPosition,planes[2])>-mRange)&&
		(GetDistance(mPosition,planes[3])>-mRange)&&
		(GetDistance(mPosition,planes[4])>-mRange)&&
		(GetDistance(mPosition,planes[5])>-mRange) )
		return true;
	return false;
}
