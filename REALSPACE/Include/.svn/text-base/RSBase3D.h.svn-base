// RSBase3D.h: interface for the RSBase3D class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RSBASE3D_H__0B615FA1_F10F_11D2_B022_00AA006E4A4E__INCLUDED_)
#define AFX_RSBASE3D_H__0B615FA1_F10F_11D2_B022_00AA006E4A4E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "rsutils.h"
#include "CMList.h"

class RealSpace;

class RSBase3D  
{
	friend RealSpace;

public:
	RSBase3D();
	virtual ~RSBase3D();

// virtual functions ( its are required function for Realspace's 3D Object. )
	virtual void Transform();
	virtual void TransformAndLight(RSPointLightList *PointLightList);
	virtual void SetupLight(RSPointLightList *PointLightList);
	virtual void Draw();
	virtual void DrawShadow(ASSMap *TargetMap);
	virtual void MoveAnimation(float elapsedtime);
	virtual void SetFOW(float);
	virtual void SetFogColor(DWORD dwColor);
	virtual bool BoundingBoxTest(rplane *planes);
	virtual bool BoundingBoxTest_Shadow(rplane *planes,ASSMap *TargetMap);
	virtual bool BoundingSphereTest(rvector *center,float radius);
	virtual bool IsFog(rvector pos,float r);

// position & Orientation ...
	virtual rvector GetPosition() { return position; }		// 로컬 좌표로의 위치.
	virtual rvector GetWorldPosition();						// hierachy의 영향을 받은 월드좌표
	virtual void SetPosition(float,float,float);
	virtual void SetPosition(float x,float y){SetPosition(x,y,position.z);}
	virtual void SetPosition(const rvector &r){SetPosition(r.x,r.y,r.z);}

	virtual void AddRotationZ(float theta);
	virtual void AddRotationY(float theta);
	virtual void AddRotationX(float theta);

	virtual void GetOrientation(rvector *dir, rvector *up);
	virtual void SetOrientation(const rvector &,const rvector &);
	virtual void SetRole(float role);

	virtual void SetScale(float);
	virtual void SetScale(float,float,float);
	virtual void GetScale(float*,float*,float*);
	virtual float GetMaxScaleFactor();

	virtual void SetVisible(bool bVisible) { m_bVisible=bVisible; }
	virtual bool GetVisible() { return m_bVisible; }

	bool isInScreen() { return m_bIsInScreen; }

	void UpdateMaxScaleFactor();
	void SetShadowState(RSSHADOWSTYLE s) { nShadowStyle=s; }

// this allow us to make Hierachy
	void AddChild(RSBase3D *);
	virtual rmatrix GetMatrix();
	void UpdateMatrix();

	float ScaleFactorX,ScaleFactorY,ScaleFactorZ,RoleFactor,MaxScaleFactor;
	rmatrix mat,matm;
	RSBase3D *parent;
	bool m_bVisible;
	bool m_bIsInScreen;
	int	LastFrame;

protected:
// member variables...
	bool isVisualObject;
	rvector position,m_dir,m_up;
	CMLinkedList <RSBase3D> m_ChildrenList;
	RSSHADOWSTYLE nShadowStyle;

	void GetDirectionVectors();
};

#endif // !defined(AFX_RSBASE3D_H__0B615FA1_F10F_11D2_B022_00AA006E4A4E__INCLUDED_)
