// RSSpotLight.h: interface for the RSSpotLight class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RSSPOTLIGHT_H__D7D69FA1_CF86_11D2_B3E9_00AA006E4A4E__INCLUDED_)
#define AFX_RSSPOTLIGHT_H__D7D69FA1_CF86_11D2_B3E9_00AA006E4A4E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "rutils.h"

class RSSpotLight  
{
public:
	RSSpotLight();
	RSSpotLight(rvector Position,rvector dir,rvector up,
				float uangle,float vangle,float maxdist,int TextureHandle);
	virtual ~RSSpotLight();

	void DrawMapPolygons(ASSMap *map);
	void SetOrientation(rvector,rvector);
	void SetLightAngles(float,float);
	void SetPosition(rvector);

private:
	void ComputeFrustrum(rplane *plane,float x,float y,float z);
	void SetupPlanes();
	rvector m_Position,m_Direction,m_Up;
	float m_uangle,m_vangle,m_maxdist;
	int m_TextureHandle;
	rmatrix m_Matrix;
	rplane m_Planes[5];
};

#endif // !defined(AFX_RSSPOTLIGHT_H__D7D69FA1_CF86_11D2_B3E9_00AA006E4A4E__INCLUDED_)
