// RSSpotLight.cpp: by dubble. 3월 1일. 대한독립만세.
//////////////////////////////////////////////////////////////////////

#include "realspace.h"
#include "RSSpotLight.h"
#include "ASSMap.h"

RSSpotLight::RSSpotLight()
{

}

RSSpotLight::~RSSpotLight()
{

}

RSSpotLight::RSSpotLight(rvector Position,rvector dir,rvector up,
						 float uangle,float vangle,float maxdist,int TextureHandle)
{
	m_Position=Position;
	m_Direction=dir;m_Up=up;
	m_uangle=uangle;m_vangle=vangle;m_maxdist=maxdist;
	m_TextureHandle=TextureHandle;
	SetupPlanes();
}

void RSSpotLight::SetPosition(rvector pos)
{
	m_Position=pos;
	SetupPlanes();
}

void RSSpotLight::DrawMapPolygons(ASSMap *map)
{
	/*
//	float backup[6];
	rmatrix44 proj=ProjectionMatrix(0.01f,100.0f,pi/2.0f,pi/2.0f),mat=MatrixMult(m_Matrix,RSProjection);

	RSSetAlphaState(RS_ALPHASTYLE_ADD);
//	RSSetZBufferState(FALSE,FALSE);
	RSSetTexture(m_TextureHandle);

	RSFaceList fl;rface *f;rvector tem;
	map->GetFaceListWRTPlanes(&fl,m_Planes,4);
	for(int i=0;i<fl.GetCount();i++)
	{
		f=fl.Get(i);
//		memcpy(backup,f->s,sizeof(float)*6); // backup

		tem=TransformVector(*(rvector*)f->v[0],mat);
		f->v[0]->u=tem.x*0.5f+0.5f;f->v[0]->v=tem.y*0.5f+0.5f;
		tem=TransformVector(*(rvector*)f->v[1],mat);
		f->v[1]->u=tem.x*0.5f+0.5f;f->v[1]->v=tem.y*0.5f+0.5f;
		tem=TransformVector(*(rvector*)f->v[2],mat);
		f->v[2]->u=tem.x*0.5f+0.5f;f->v[2]->v=tem.y*0.5f+0.5f;
		f->v[0]->color=f->v[1]->color=f->v[2]->color=0xffffff;
		DrawFaceWithClip4(f,m_Planes);		
//		memcpy(f->s,backup,sizeof(float)*6); // restore
	}
	fl.DeleteRecordAll();
	RSSetAlphaState(RS_ALPHASTYLE_NONE);
//	RSSetZBufferState(TRUE,TRUE);
*/
}

void RSSpotLight::SetupPlanes()
{
	m_Matrix=ViewMatrix(m_Position,m_Direction,m_Up);
	ComputeFrustrum(m_Planes+0, -1,  0, 1);
	ComputeFrustrum(m_Planes+1,  1,  0, 1);
	ComputeFrustrum(m_Planes+2,  0,  1, 1);
	ComputeFrustrum(m_Planes+3,  0, -1, 1);

}

void RSSpotLight::ComputeFrustrum(rplane *plane,float x,float y,float z)
{
	plane->a=m_Matrix._11*x+m_Matrix._12*y+m_Matrix._13*z;
	plane->b=m_Matrix._21*x+m_Matrix._22*y+m_Matrix._23*z;
	plane->c=m_Matrix._31*x+m_Matrix._32*y+m_Matrix._33*z;
	plane->d=-plane->a*m_Position.x
				-plane->b*m_Position.y
				-plane->c*m_Position.z;
}
