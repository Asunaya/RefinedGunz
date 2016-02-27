#ifndef _RSOLIDBSP_H
#define _RSOLIDBSP_H

//#pragma	once

#include "RTypes.h"
#include "RNameSpace.h"

_NAMESPACE_REALSPACE2_BEGIN

class RImpactPlanes : public list<rplane> {
public:
	bool Add(rplane &p);
};

enum RCOLLISIONMETHOD{
	RCW_SPHERE,
	RCW_CYLINDER
};

class RSolidBspNode
{
private:
	bool GetColPlanes_Recurse(int nDepth=0);

	static RCOLLISIONMETHOD m_ColMethod;
	static float	m_fColRadius;
	static float	m_fColHeight;
	static rvector	m_ColOrigin;
	static rvector	m_ColTo;
	static RImpactPlanes* m_pOutList;
	static float	fImpactDist;
	static rvector	impact;
	static rplane	impactPlane;

public:
	// 디버그용 정보
	static bool				m_bTracePath;

	rplane			m_Plane;
	RSolidBspNode	*m_pPositive,*m_pNegative;
	bool			m_bSolid;

	RSolidBspNode();
	virtual ~RSolidBspNode();

	// origin 에서 to 로 이동할때 충돌하는 모든 평면을 리턴받는다. 가장 먼저 닿는점은 아래의 GetLastColPos() 로 얻어낸다
	bool GetColPlanes_Cylinder(RImpactPlanes *pOutList,const rvector &origin,const rvector &to,float fRadius,float fHeight);
	bool GetColPlanes_Sphere(RImpactPlanes *pOutList,const rvector &origin,const rvector &to,float fRadius);
	rvector GetLastColPos() { return impact; }
	rplane GetLastColPlane() { return impactPlane; }

	// origin 에서 targetpos 로 이동하는데 미끄러짐을 감안해서 targetpos 를 조절해서 리턴해준다.
	static bool CheckWall(RSolidBspNode *pRootNode,rvector &origin,rvector &targetpos,float fRadius,float fHeight=0.f,RCOLLISIONMETHOD method=RCW_CYLINDER,int nDepth=0,rplane *pimpactplane=NULL);
	static bool CheckWall2(RSolidBspNode *pRootNode,RImpactPlanes &impactPlanes,rvector &origin,rvector &targetpos,float fRadius,float fHeight,RCOLLISIONMETHOD method);

#ifndef _PUBLISH
	int				nPolygon;
	rvector			*pVertices;
	rvector			*pNormals;

	rboundingbox m_bb;

	void DrawPolygon();
	void DrawPolygonWireframe();
	void DrawPolygonNormal();

	void DrawSolidPolygon();
	void DrawSolidPolygonWireframe();
	void DrawSolidPolygonNormal();

	void DrawPos(rvector &pos);
	void DrawPlaneVertices(rplane &plane);
	void ConstructBoundingBox();
#endif
};

_NAMESPACE_REALSPACE2_END

#endif