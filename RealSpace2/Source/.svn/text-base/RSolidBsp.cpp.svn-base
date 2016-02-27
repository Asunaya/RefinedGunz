#include "stdafx.h"
#include "MDebug.h"
#include "RealSpace2.h"
#include "RSolidBsp.h"
#include <crtdbg.h>

_USING_NAMESPACE_REALSPACE2

bool RImpactPlanes::Add(rplane &p)
{
	for(iterator i=begin();i!=end();i++)
	{
		if(*i==p) return false;
	}
	push_back(p);
	return  true;
}


RCOLLISIONMETHOD RSolidBspNode::m_ColMethod;
float	RSolidBspNode::m_fColRadius;
float	RSolidBspNode::m_fColHeight;
rvector	RSolidBspNode::m_ColOrigin;
rvector	RSolidBspNode::m_ColTo;
RImpactPlanes* RSolidBspNode::m_pOutList;
float	RSolidBspNode::fImpactDist;
rvector	RSolidBspNode::impact;
rplane	RSolidBspNode::impactPlane;

RSolidBspNode::RSolidBspNode()
{
	m_pPositive=NULL;
	m_pNegative=NULL;
#ifndef _PUBLISH
	pVertices=NULL;
	pNormals=NULL;
#endif
}

RSolidBspNode::~RSolidBspNode()
{
#ifndef _PUBLISH
	SAFE_DELETE(pNormals);
#endif
}

#ifndef _PUBLISH
void RSolidBspNode::DrawPolygon()
{
	//	if(!isInViewFrustum(&m_bb,RGetViewFrustum())) return;

	if(nPolygon && !m_bSolid)
		RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST,nPolygon,pVertices,sizeof(rvector));

	if(m_pPositive)
		m_pPositive->DrawPolygon();

	if(m_pNegative)
		m_pNegative->DrawPolygon();

}

void RSolidBspNode::DrawPolygonWireframe()
{
	//	if(!isInViewFrustum(&m_bb,RGetViewFrustum())) return;

	if(nPolygon && !m_bSolid)
	{
		rvector v[4];
		for(int i=0;i<nPolygon;i++)
		{
			memcpy(v,pVertices[i*3],3*sizeof(rvector));
			v[3]=v[0];

			RGetDevice()->DrawPrimitiveUP(D3DPT_LINESTRIP,3,&v,sizeof(rvector));
		}
	}

	if(m_pPositive)
		m_pPositive->DrawPolygonWireframe();

	if(m_pNegative)
		m_pNegative->DrawPolygonWireframe();

}

void RSolidBspNode::DrawPolygonNormal()
{
	//	if(!isInViewFrustum(&m_bb,RGetViewFrustum())) return;

	if(nPolygon && !m_bSolid)
	{
		rvector v[2];
		for(int i=0;i<nPolygon;i++)
		{
			rvector center=1.f/3.f*(pVertices[i*3]+pVertices[i*3+1]+pVertices[i*3+2]);

			v[0]=center;
			v[1]=center+pNormals[i]*50.f;
			RGetDevice()->DrawPrimitiveUP(D3DPT_LINESTRIP,1,&v,sizeof(rvector));
		}
	}

	if(m_pPositive)
		m_pPositive->DrawPolygonNormal();

	if(m_pNegative)
		m_pNegative->DrawPolygonNormal();

}

void RSolidBspNode::DrawSolidPolygon()
{
	//	if(!isInViewFrustum(&m_bb,RGetViewFrustum())) return;

	if(nPolygon && m_bSolid)
		RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST,nPolygon,pVertices,sizeof(rvector));

	if(m_pPositive)
		m_pPositive->DrawSolidPolygon();

	if(m_pNegative)
		m_pNegative->DrawSolidPolygon();

}

void RSolidBspNode::DrawSolidPolygonWireframe()
{
	//	if(!isInViewFrustum(&m_bb,RGetViewFrustum())) return;

	if(nPolygon && m_bSolid)
	{
		rvector v[4];
		for(int i=0;i<nPolygon;i++)
		{
			memcpy(v,pVertices[i*3],3*sizeof(rvector));
			v[3]=v[0];

			RGetDevice()->DrawPrimitiveUP(D3DPT_LINESTRIP,3,&v,sizeof(rvector));
		}
	}

	if(m_pPositive)
		m_pPositive->DrawSolidPolygonWireframe();

	if(m_pNegative)
		m_pNegative->DrawSolidPolygonWireframe();

}

void RSolidBspNode::DrawSolidPolygonNormal()
{
	//	if(!isInViewFrustum(&m_bb,RGetViewFrustum())) return;

	if(nPolygon && m_bSolid)
	{
		rvector v[2];
		for(int i=0;i<nPolygon;i++)
		{
			rvector center=1.f/3.f*(pVertices[i*3]+pVertices[i*3+1]+pVertices[i*3+2]);

			v[0]=center;
			v[1]=center+pNormals[i]*50.f;
			RGetDevice()->DrawPrimitiveUP(D3DPT_LINESTRIP,1,&v,sizeof(rvector));
		}
	}

	if(m_pPositive)
		m_pPositive->DrawSolidPolygonNormal();

	if(m_pNegative)
		m_pNegative->DrawSolidPolygonNormal();

}

void RSolidBspNode::DrawPos(rvector &pos)
{
	if(nPolygon) {
		if((timeGetTime()/500) %2 == 0) {
			RGetDevice()->SetFVF( D3DFVF_XYZ );
			RGetDevice()->SetRenderState(D3DRS_ZENABLE, FALSE );
			RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
			RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
			RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
			RGetDevice()->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
			RGetDevice()->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
			RGetDevice()->SetRenderState(D3DRS_TEXTUREFACTOR ,   0xff00ffff);
			RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST,nPolygon,pVertices,sizeof(rvector));

			float fSize=10.f;
			RGetDevice()->SetRenderState(D3DRS_POINTSIZE,   *(DWORD*)&fSize);
			RGetDevice()->DrawPrimitiveUP(D3DPT_POINTLIST,nPolygon*3,pVertices,sizeof(rvector));

			//			float fArea=GetArea(pVertices[0],pVertices[1],pVertices[2]);
		}
		return;
	}

	if(D3DXPlaneDotCoord(&m_Plane,&pos)<0) {
		if(m_pNegative)
			m_pNegative->DrawPos(pos);
	}else {
		if(m_pPositive)
			m_pPositive->DrawPos(pos);
	}
}

void RSolidBspNode::DrawPlaneVertices(rplane &plane)
{
	if(nPolygon) {
		if((timeGetTime()/500) %2 == 0) {
			RGetDevice()->SetFVF( D3DFVF_XYZ );
			RGetDevice()->SetRenderState(D3DRS_ZENABLE, FALSE );
			RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
			RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
			RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
			RGetDevice()->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
			RGetDevice()->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

			float fSize=10.f;
			RGetDevice()->SetRenderState(D3DRS_POINTSIZE,   *(DWORD*)&fSize);
			for(int i=0;i<nPolygon*3;i++)
			{
				if(fabs(D3DXPlaneDotCoord(&plane,pVertices+i))<5.f)
					RGetDevice()->DrawPrimitiveUP(D3DPT_POINTLIST,1,pVertices+i,sizeof(rvector));
			}
		}
		return;
	}

	if(m_pNegative)
		m_pNegative->DrawPlaneVertices(plane);
	if(m_pPositive)
		m_pPositive->DrawPlaneVertices(plane);
}

void RSolidBspNode::ConstructBoundingBox()
{
	if(!m_pPositive && !m_pNegative) return;

	if(nPolygon)
	{
		int i,j;
		m_bb.vmin=pVertices[0];
		m_bb.vmax=pVertices[0];
		for(i=1;i<nPolygon*3;i++){
			for(j=0;j<3;j++)
			{
				m_bb.vmin[j]=min(m_bb.vmin[j],pVertices[i][j]);
				m_bb.vmax[j]=max(m_bb.vmax[j],pVertices[i][j]);
			}
		}
		return;
	}

	if(m_pPositive)
		m_pPositive->ConstructBoundingBox();

	if(m_pNegative)
		m_pNegative->ConstructBoundingBox();

	if(m_pPositive)
	{
		m_bb=m_pPositive->m_bb;
		if(m_pNegative)
			MergeBoundingBox(&m_bb,&m_pNegative->m_bb);
	}else
	{
		if(m_pNegative)
			m_bb=m_pNegative->m_bb;
	}
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////
//  여기서부터 충돌체크에 관한 부분

//  solid bsp + shift plane 방법 ( 일정 두께로 띄워져서 검사 )
//  Dynamic Plane Shifting BSP Traversal ( by Stan Melax ) 참조


#define TOLERENCE 0.001f

bool RSolidBspNode::m_bTracePath = false;

// TODO : 멤버로 집어넣는다
#define MAX_DEPTH	256
rplane	m_SolidPlanes[MAX_DEPTH];		// 마지막 solid 노드를 이루는 평면들 (경로)

// 평면의 m_pNegative 와 cross 되는지를 리턴
bool IsCross(const rplane &plane,const rvector &v0,const rvector &v1,float *fParam)
{
#define CSIGN(x) ( (x)<-0.1? -1 : (x)>0.1? 1 : 0 )
	float dotv0=D3DXPlaneDotCoord(&plane,&v0);
	float dotv1=D3DXPlaneDotCoord(&plane,&v1);
	int signv0=CSIGN(dotv0),signv1=CSIGN(dotv1);

	if (signv0==1 && signv1==1) return false;

	rvector dir=v1-v0;
	if(D3DXPlaneDotNormal(&plane,&dir)>0) {
		*fParam=-1;
	}else
		if(signv0==0) {

			if(signv1==0)
				*fParam=1;
			else
				*fParam=0;
		}else
			if(dotv0*dotv1<=0) {
				// 선분이 평면을 교차하는 경우
				*fParam = ( dotv0 / ( dotv0-dotv1));
				*fParam = min(1,*fParam);
			}else
			{
				*fParam=-1;
			}

			return true;
}

bool RSolidBspNode::GetColPlanes_Recurse(int nDepth)
{
	bool bHit=false;

	if(!m_pPositive && !m_pNegative) {	// leaf
		if(m_bSolid) {

//			_ASSERT(nDepth<MAX_DEPTH);

			bool bInSolid=true;
			for(int i=0;i<nDepth;i++) {
				rplane *pPlane=m_SolidPlanes+i;
				float dotv0=D3DXPlaneDotCoord(pPlane,&m_ColOrigin);
				if(dotv0>-0.1f) {
					bInSolid=false;
					break;
				}
			}
//			_ASSERT(!bInSolid);	// 이미 solid 영역 안이 있는 경우다
 
			rvector dir=m_ColTo-m_ColOrigin;

			// solid 영역과 어느 점에서 만나는지 찾는다
			float fMaxParam = 0;
			for(int i=0;i<nDepth;i++) {
				rplane *pPlane=m_SolidPlanes+i;

				float dotv0=D3DXPlaneDotCoord(pPlane,&m_ColOrigin);
				float dotv1=D3DXPlaneDotCoord(pPlane,&m_ColTo);

				// solid 영역을 이루는 평면을 따라가면 충돌이 아니다
				if(fabs(dotv0)<0.1f && fabs(dotv1)<0.1f) {
					m_pOutList->Add(*pPlane);
					return false;	
				}

				if(D3DXPlaneDotNormal(pPlane,&dir)>0) continue;	// 뒷면 제거

				if(0<dotv0 && dotv0<dotv1) continue; // solid 에서 멀어지고 있는 경우 제거
				if(fabs(dotv0-dotv1)<0.01f) continue; // 평면과 평행하게 움직이는 경우
				float fParam = ( dotv0 / ( dotv0-dotv1));

				fMaxParam = max(fMaxParam,fParam);
			}

			rvector colPos = m_ColOrigin+(m_ColTo-m_ColOrigin)*fMaxParam;
			
			float fDist = Magnitude(colPos-m_ColOrigin);
/*
			if(fDist>fImpactDist+0.1) 
				return false;	// 더 멀리있는것들은 볼필요 없다.	// 뭔가 버그가있다 -_-;
*/
			int nCount=0;
			for(int i=0;i<nDepth;i++) {
				rplane *pPlane=m_SolidPlanes+i;
				if(D3DXPlaneDotNormal(pPlane,&dir)>0) continue;	// 뒷면 제거
				if(fabs(D3DXPlaneDotCoord(pPlane,&colPos))<0.1f) {
					m_pOutList->Add(*pPlane);
					if(fDist<fImpactDist)
					{
						fImpactDist = fDist;
						impact = colPos;
						impactPlane = *pPlane;
					}
					nCount++;
				}
			}

			return true;
		}
		return false;
	}

	float fShift;
	if(m_ColMethod==RCW_CYLINDER)
	{
		rvector rimpoint=rvector(-m_Plane.a,-m_Plane.b,0);
		if(IS_ZERO(rimpoint.x) && IS_ZERO(rimpoint.y))
			rimpoint.x=1.f;
		Normalize(rimpoint);
		rimpoint= rimpoint*m_fColRadius;
		rimpoint.z +=  (m_Plane.c < 0 ) ? m_fColHeight : -m_fColHeight;
		fShift=-D3DXPlaneDotNormal(&m_Plane,&rimpoint);
	}else
	{
		fShift=m_fColRadius;
	}

	rplane shiftPlane=m_Plane;
	shiftPlane.d=m_Plane.d-fShift;

	float fCurParam;
	if(m_pNegative!=NULL && IsCross(shiftPlane,m_ColOrigin,m_ColTo,&fCurParam))
	{
		m_SolidPlanes[nDepth]=shiftPlane;
		bHit=m_pNegative->GetColPlanes_Recurse(nDepth+1);
	}

	shiftPlane.d=m_Plane.d+fShift;
	shiftPlane=-shiftPlane;

	if(m_pPositive!=NULL && IsCross(shiftPlane,m_ColOrigin,m_ColTo,&fCurParam))
	{
		m_SolidPlanes[nDepth]=shiftPlane;
		bHit|=m_pPositive->GetColPlanes_Recurse(nDepth+1);
	}

	return bHit;
}

bool RSolidBspNode::GetColPlanes_Sphere(RImpactPlanes *pOutList,const rvector &origin,const rvector &to,float fRadius)
{
	m_ColMethod = RCW_SPHERE;
	m_ColOrigin = origin;
	m_ColTo = to;
	m_pOutList = pOutList;

	m_fColRadius = fRadius;

	fImpactDist=FLT_MAX;
	impact=rvector(0,0,0);

	return GetColPlanes_Recurse();    
}

bool RSolidBspNode::GetColPlanes_Cylinder(RImpactPlanes *pOutList,const rvector &origin,const rvector &to,float fRadius,float fHeight)
{
	m_ColMethod = RCW_CYLINDER;
	m_ColOrigin = origin;
	m_ColTo = to;
	m_pOutList = pOutList;

	m_fColRadius = fRadius;
	m_fColHeight = fHeight;

	fImpactDist=FLT_MAX;
	impact=rvector(0,0,0);

	return GetColPlanes_Recurse();    
}


rvector checkwallorigin;
rvector checkwalldir;

// 무언가에 걸려서 1case 혹은 2case 로 오면 이 펑션으로 들어온다
bool RSolidBspNode::CheckWall2(RSolidBspNode *pRootNode,RImpactPlanes &impactPlanes,rvector &origin,rvector &targetpos,float fRadius,float fHeight,RCOLLISIONMETHOD method)
{
	checkwallorigin=origin;

	if(m_bTracePath) {	// 순전히 출력하는 부분
		rvector dif=targetpos-origin;
		mlog(" from ( %3.5f %3.5f %3.5f ) by ( %3.3f %3.3f %3.3f ) "
			,origin.x,origin.y,origin.z,dif.x,dif.y,dif.z);
	}

	bool bIntersectThis;

	impactPlanes.erase(impactPlanes.begin(),impactPlanes.end());

	if(method==RCW_SPHERE)
		bIntersectThis=pRootNode->GetColPlanes_Sphere(&impactPlanes,origin,targetpos,fRadius);
	else
		bIntersectThis=pRootNode->GetColPlanes_Cylinder(&impactPlanes,origin,targetpos,fRadius,fHeight);

	RImpactPlanes::iterator i;
/*
	// 가는방향과 상관없는 벽을 먼저 제거한다
	rvector godir=targetpos-origin;
	for(i=impactPlanes.begin();i!=impactPlanes.end();)
	{
		rplane p=*i;
		if(D3DXPlaneDotNormal(&p,&godir)>TOLERENCE)
		{
			if(m_bTracePath) {
				float fDist = D3DXPlaneDotCoord(&p,&origin);
				mlog(" del_back{d = %3.3f [%3.3f %3.3f %3.3f %3.3f]}",fDist,p.a,p.b,p.c,p.d);
			}
			i=impactPlanes.erase(i)	;
		}else
			i++;
	}
*/

	if(m_bTracePath) {	// 순전히 출력하는 부분
		if(impactPlanes.size())
		{
			mlog(" :::: %d planes ",impactPlanes.size());
			for(RImpactPlanes::iterator i=impactPlanes.begin();i!=impactPlanes.end();i++)
			{
				rplane p=*i;
				float fDist = D3DXPlaneDotCoord(&p,&origin);
				mlog(" d = %3.3f [%3.3f %3.3f %3.3f %3.3f] ",fDist,p.a,p.b,p.c,p.d);
			}
		}
	}

	if(impactPlanes.size())
	{
		bool bFound=false;
		rplane simulplanes[100];	// 최대 동시에 맞닿는 면의 개수

		rvector diff=targetpos-origin;
		float fMinProjDist;
		float fMinDistToOrigin;

		for(i=impactPlanes.begin();i!=impactPlanes.end();i++)
		{
			rplane plane = *i;

			if(plane==rplane(0,0,0,0)) 
			{
				_ASSERT(FALSE);	// 현재 이런경우는 발생하지 않는다.
				i=impactPlanes.erase(i);
				continue;	// 아마도 벽속에 들어가 있을 확률이 높다
			}

			float fDistToOrigin = D3DXPlaneDotCoord(&plane,&origin);		// origin 에서 평면까지의 최단거리
			float fDistToTarget = D3DXPlaneDotCoord(&plane,&targetpos);		// origin 에서 평면까지의 최단거리

			// 
			if(fDistToOrigin>-.1f && fDistToTarget>-.1f )		//  진행방향과 평행한 평면은 통과
			{
				continue;
			}


			float fProjDist = -D3DXPlaneDotNormal(&plane,&diff);			// diff 가 평면의 법선으로 projection 된 길이

			if(fDistToOrigin<=0 && fDistToTarget<fDistToOrigin) { // 이미 평면의 뒤에 있는 경우이다.
				fProjDist = 1;
				fDistToOrigin = 0;
			}

			if(fProjDist==0) {  // 완전히 평행한경우이다.
				// 완전히 0인 경우에는 floating point exception 이 일어난다.
				fProjDist = 1;
				fDistToOrigin = 1;
			}

			// 따라서 교차하는 지점은 origin + (fDistToOrigin/fProjDist) * diff 이다.
			// (fDistToOrigin/fProjDist) 가 최소가 되는 평면을 찾으면 가장 먼저 맞닿는 면이다.
			if(!bFound)
			{
				bFound = true;
				fMinProjDist = fProjDist;
				fMinDistToOrigin = fDistToOrigin;
			}else
			{
				// 정확도를 높이기 위해 위의 나누기 대신에 이렇게 비교한다. (Gaming Programming Gems 3 권 2.2 벡터분수 참조)
				//				if( fDistToOrigin/fProjDist < fMinDistToOrigin/fMinProjDist )
				if( fDistToOrigin * fMinProjDist < fMinDistToOrigin * fProjDist  )
				{
					fMinProjDist = fProjDist;
					fMinDistToOrigin = fDistToOrigin;
				}
			}
		}

		if(!bFound) {
			return false;
		}

		float fInter = max(0,min(1.f,fMinDistToOrigin/fMinProjDist));
		rvector currentorigin;
		currentorigin = origin + fInter * diff;

		targetpos = currentorigin;

		// 새 origin 에서부터의 거리가 같은 면들을 찾는다

		int nSimulCount=0;

		for(i=impactPlanes.begin();i!=impactPlanes.end();i++) 
		{
			//			if(fabs(fDist-i->first)<0.01f) { // 동시에 닿는 면이다.
			rplane p=*i;
			if(fabs(D3DXPlaneDotCoord(&p,&currentorigin))<0.05) {
				simulplanes[nSimulCount++]=p;
			}
		}

	if(m_bTracePath) {
		mlog("%d simul ",nSimulCount);
	}

		return true;
	}

	// 아무것도 걸리는게 없으면 그냥 리턴한다

	return false;
}

bool g_bchecktest=false;
rvector g_checkdebug;

// origin 에서 targetpos 로 이동하는데 미끄러짐을 감안해서 targetpos 를 조절해서 리턴해준다.
bool RSolidBspNode::CheckWall(RSolidBspNode *pRootNode,rvector &origin,rvector &targetpos,float fRadius,float fHeight,RCOLLISIONMETHOD method,int nDepth,rplane *pimpactplane)
{
	checkwallorigin=origin;
	checkwalldir=targetpos-origin;
	Normalize(checkwalldir);

	bool bIntersectThis;

	RImpactPlanes impactPlanes;
	impactPlanes.erase(impactPlanes.begin(),impactPlanes.end());

	if(method==RCW_SPHERE)
		bIntersectThis=pRootNode->GetColPlanes_Sphere(&impactPlanes,origin,targetpos,fRadius);
	else
		bIntersectThis=pRootNode->GetColPlanes_Cylinder(&impactPlanes,origin,targetpos,fRadius,fHeight);

//	if(impactPlanes.size()) 
	{
		if(m_bTracePath) {	// 순전히 출력하는 부분
			mlog("\n");
			for(int i=0;i<nDepth;i++) mlog("    ");

			rvector dif=targetpos-origin;
			mlog("d%d from ( %3.5f %3.5f %3.5f ) by ( %3.3f %3.3f %3.3f ) ",nDepth
				,origin.x,origin.y,origin.z,dif.x,dif.y,dif.z);
		}

		/*
		if(g_bchecktest)
		{
			_ASSERT(Magnitude(g_checkdebug-origin)<1.f);
		}
		g_bchecktest=false;
		*/
	}

	RImpactPlanes::iterator i;

	/*
	// 가는방향과 상관없는 벽을 먼저 제거한다
	rvector godir=targetpos-origin;
	for(i=impactPlanes.begin();i!=impactPlanes.end();)
	{
		rplane p=*i;
		if(D3DXPlaneDotNormal(&p,&godir)>TOLERENCE)
		{
			if(m_bTracePath) {
				float fDist = D3DXPlaneDotCoord(&p,&origin);
				mlog(" del_back{d = %3.3f [%3.3f %3.3f %3.3f %3.3f]}",fDist,p.a,p.b,p.c,p.d);
			}
			i=impactPlanes.erase(i)	;
		}else
			i++;
	}
	*/


	if(m_bTracePath) {	// 순전히 출력하는 부분
		if(impactPlanes.size())
		{
			mlog(" :::: %d planes ",impactPlanes.size());
			for(RImpactPlanes::iterator i=impactPlanes.begin();i!=impactPlanes.end();i++)
			{
				rplane p=*i;
				float fDist = D3DXPlaneDotCoord(&p,&origin);
				mlog(" d = %3.3f [%3.3f %3.3f %3.3f %3.3f] ",fDist,p.a,p.b,p.c,p.d);
			}
		}
	}

	if(impactPlanes.size())
	{
		bool bFound=false;
		rplane simulplanes[100];	// 최대 동시에 맞닿는 면의 개수

		rvector diff=targetpos-origin;
		float fMinProjDist = 1.f;
		float fMinDistToOrigin = 0.f;

		// 골라내어진 평면들 중에 가장 먼저 맞닿는 평면을 골라낸다
		for(i=impactPlanes.begin();i!=impactPlanes.end();i++)
		{
			rplane plane = *i;

			if(plane==rplane(0,0,0,0)) 
			{
				_ASSERT(FALSE);
				i=impactPlanes.erase(i);
				continue;	// 아마도 벽속에 들어가 있을 확률이 높다
			}

			float fProjDist = -D3DXPlaneDotNormal(&plane,&diff);			// diff 가 평면의 법선으로 projection 된 길이
			float fDistToOrigin = D3DXPlaneDotCoord(&plane,&origin);		// origin 에서 평면까지의 최단거리

			if(fDistToOrigin<=-0.1) { // 이미 평면의 뒤에 있는 경우이다.
				fProjDist = 1;
				fDistToOrigin = 0;
			}
			if(fProjDist==0) {  // 완전히 평행한경우이다.
				// 완전히 0인 경우에는 floating point exception 이 일어난다.
				fProjDist = 1;
				fDistToOrigin = 1;
			}

			// 따라서 교차하는 지점은 origin + (fDistToOrigin/fProjDist) * diff 이다.
			// (fDistToOrigin/fProjDist) 가 최소가 되는 평면을 찾으면 가장 먼저 맞닿는 면이다.
			if(!bFound)
			{
				bFound = true;
				fMinProjDist = fProjDist;
				fMinDistToOrigin = fDistToOrigin;
			}else
			{
				if( fDistToOrigin/fProjDist < fMinDistToOrigin/fMinProjDist )

// 부호를 맟춰야 하기 때문에 보류.. (이들중 하나가 -값을 가지면 부등호의 방향이 바뀐다.)
// 정확도를 높이기 위해 위의 나누기 대신에 이렇게 비교한다. (Gaming Programming Gems 3 권 2.2 벡터분수 참조)
//				if( fDistToOrigin * fMinProjDist < fMinDistToOrigin * fProjDist  )
				{
					fMinProjDist = fProjDist;
					fMinDistToOrigin = fDistToOrigin;
				}
			}
		}

		if(!bFound) {
			return false;
		}

		// 가장 먼저 맞닿는 평면과의 교점을 구한다 (currentorigin)
		float fInter = max(0.f,min(1.f,fMinDistToOrigin/fMinProjDist));
		rvector currentorigin = origin + fInter * diff;

		if(nDepth==0 && pimpactplane)
			*pimpactplane=simulplanes[0];

		// 새 origin 에서부터의 거리가 같은 면들을 찾는다

		int nSimulCount=0;

		for(i=impactPlanes.begin();i!=impactPlanes.end();i++) 
		{
			//			if(fabs(fDist-i->first)<0.01f) { // 동시에 닿는 면이다.
			rplane p=*i;
			if(fabs(D3DXPlaneDotCoord(&p,&currentorigin))<0.1) {
				simulplanes[nSimulCount++]=p;
			}
		}

		if(m_bTracePath) {
			mlog("fInter = %3.3f %d simul ",fInter,nSimulCount);
		}

		float fBestCase=0.f;
		rvector newtargetpos=currentorigin;

		// 한 면을 타고 미끄러지는 경우가 1case 이고..
		// 두 면의 교선을 타고 미끄러지는 경우가 2case 이다.

		bool b1Case=false;
		rplane plane1case;
		{
			for(int i=0;i<nSimulCount;i++)
			{
				rplane plane=simulplanes[i];

				// 미끄러짐.. diff 에다 평면의 법선방향으로 평면에서부터의 거리만큼 더한다

				// Ni 는 평면의 법선벡터
				rvector Ni=rvector(plane.a,plane.b,plane.c);

				// targetpos 를 평면으로 수선의 발을 내린점으로 조절
				rvector adjtargetpos =  targetpos  + Ni * -D3DXPlaneDotCoord(&plane,&targetpos);

				// 직접 가본다
				if(m_bTracePath) {
					mlog("\n    check 1 : ");
				}
				rvector checktargetpos = adjtargetpos;
				CheckWall2(pRootNode,impactPlanes,currentorigin,checktargetpos,fRadius,fHeight,method);
				float fDot=DotProduct(checkwalldir,checktargetpos-origin);
				if(m_bTracePath) {
					mlog("dot = %3.3f ",fDot);
				}

				if(fDot>fBestCase)
				{
					b1Case=true;
					fBestCase=fDot;
					newtargetpos=adjtargetpos;
					plane1case=plane;
				}
			}
		}

//		if(!b1Case)

		bool b2Case=false;
		{
			// 두개의 평면에 동시에 닿는 경우이다.  (같은평면인 경우는 이미 없다)
			// 이럴때는 두개의 평면에 동시에 수직인 방향으로 진행한다.


			rvector dif=targetpos-currentorigin;
			for(int i=0;i<nSimulCount;i++)
			{
				for(int j=i+1;j<nSimulCount;j++)
				{
					rvector aPoint,dir;
					if(GetIntersectionOfTwoPlanes(&dir,&aPoint,simulplanes[i],simulplanes[j]))
					{
						Normalize(dir);
						rvector aPointToTarget = targetpos-aPoint;
						rvector adjtargetpos =  aPoint + DotProduct(dir,aPointToTarget)*dir;
					

						// 직접 가본다
						if(m_bTracePath) {
							mlog("\n   check 2 : ");
						}
						rvector checktargetpos = adjtargetpos;
						CheckWall2(pRootNode,impactPlanes,currentorigin,checktargetpos,fRadius,fHeight,method);
						float fDot=DotProduct(checkwalldir,checktargetpos-origin);
						//						float fDot=DotProduct(checkwalldir,adjtargetpos-origin);
						if(fDot>fBestCase)
						{
							fBestCase=fDot;
							newtargetpos=adjtargetpos;
							b2Case=true;
						}
					}


					/*
					rvector dir;
					CrossProduct(&dir,rvector(simulplanes[i].a,simulplanes[i].b,simulplanes[i].c),
						rvector(simulplanes[j].a,simulplanes[j].b,simulplanes[j].c));
					if(Magnitude(dir)<TOLERENCE) continue;
					
					Normalize(dir);

					rvector prjdif=dir*DotProduct(dir,dif);

					// targetpos 는 동시에 수직인 방향벡터로 수선의 발을 내린다
					rvector adjtargetpos=currentorigin+prjdif;
					*/

				}
			}
		}

		if(m_bTracePath) {
			mlog("\n");
			if(b2Case)
				mlog("2 case ");
			else if(b1Case)
			{
				mlog("1 case ( %3.3f %3.3f %3.3f %3.3f)",plane1case.a,plane1case.b,plane1case.c,plane1case.d);
			}
		}

		rvector newdir=targetpos-currentorigin;
		if(DotProduct(newdir,checkwalldir)<-0.01) {
		if(m_bTracePath) {
			mlog(" -> over dir.");
		}
			targetpos=currentorigin;
			return false;
		}

		// 어느방향으로도 진행이 불가능
		if(!b2Case && !b1Case) {
			targetpos = currentorigin;
			if(m_bTracePath) {
				mlog("@ %3.3f = final ( %3.3f %3.3f %3.3f )",fInter,targetpos.x,targetpos.y,targetpos.z);
				}

			g_checkdebug = targetpos;
			g_bchecktest=true;
			return true;
		}

		rvector checktargetpos = newtargetpos;
		CheckWall2(pRootNode,impactPlanes,currentorigin,checktargetpos,fRadius,fHeight,method);
		for(i=impactPlanes.begin();i!=impactPlanes.end();i++)
		{
			rplane plane = *i;
			float fDist = D3DXPlaneDotCoord(&plane,&newtargetpos);
			if(fDist<-0.05) {
				targetpos = checktargetpos;
				if(m_bTracePath) {
					mlog("! final ( %3.3f %3.3f %3.3f )",targetpos.x,targetpos.y,targetpos.z);
				}
				return true;
			}
		}

		targetpos = newtargetpos;
		if(m_bTracePath) {
			mlog("!! final ( %3.3f %3.3f %3.3f )",targetpos.x,targetpos.y,targetpos.z);
		}
		return true;
	}

	// 아무것도 걸리는게 없으면 그냥 리턴한다

	return false;
}