#include "stdafx.h"
#include "RealSpace2.h"
#include "RBspObject.h"

#define TEST_NODE_POS rvector(0,0,200)

struct RWinding {
	int nCount;
	rvector *pVertices;

	RWinding();
	~RWinding() { delete []pVertices; }
	RWinding(int n) { _ASSERT(n>2 && n<100); nCount=n; pVertices=new rvector[n]; }
	RWinding(RWinding *source) { 
		nCount=source->nCount; 
		pVertices=new rvector[nCount]; 
		memcpy(pVertices,source->pVertices,sizeof(rvector)*nCount); }

	void Draw(DWORD color) {
		{
			struct LVERTEX {
				float x, y, z;		// world position
				DWORD color;
			} ;

			RGetDevice()->SetTexture(0,NULL);
			RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );

			LVERTEX lines[1024];

			RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );

			for(int i=0;i<nCount+1;i++)
			{
				lines[i].x=pVertices[i%nCount].x;
				lines[i].y=pVertices[i%nCount].y;
				lines[i].z=pVertices[i%nCount].z;
				lines[i].color=color;
			}
			HRESULT hr=RGetDevice()->DrawPrimitiveUP(D3DPT_LINESTRIP,nCount,lines,sizeof(LVERTEX));
		}
	}

	void Dump()
	{
		static char buffer[256],buffer2[256];
		sprintf_safe(buffer,"n = %d ",nCount);
		for(int i=0;i<nCount;i++)
		{
			sprintf_safe(buffer2,"(%4.f,%4.f,%4.f) ",pVertices[i].x,pVertices[i].y,pVertices[i].z);
			strcat(buffer,buffer2);
		}
		OutputDebugString(buffer);
	}
};

#define	ON_EPSILON	0.1
#define MAX_WINDING_POINTS	1024

enum SIDE {
	SIDE_FRONT,
	SIDE_ON,
	SIDE_BACK
} ;

static RWinding *ClipWinding(RWinding *pWinding, const rplane &plane)
{
	SIDE sides[MAX_WINDING_POINTS];
	float dists[MAX_WINDING_POINTS];

	int nFront=0,nBack=0;
	int i;
	for(i=0;i<pWinding->nCount;i++)
	{
		float fDot=D3DXPlaneDotCoord(&plane,&pWinding->pVertices[i]);
		if(fDot>ON_EPSILON) {
			sides[i]=SIDE_FRONT;
			nFront++;
		} else 
			if(fDot<-ON_EPSILON) {
				sides[i]=SIDE_BACK;
				nBack++;
			}
			else
				sides[i]=SIDE_ON;
		dists[i]=fDot;
	}
	sides[i]=sides[0];
	dists[i]=dists[0];

	if(!nFront)	 // 완전히 잘려나감.
	{
		delete pWinding;
		return NULL;
	}

	if(!nBack)	// 전혀 잘려지지 않음.
		return pWinding;

	int nPointCount=0;

	for(i=0;i<pWinding->nCount;i++)
	{
		if(sides[i]!=SIDE_BACK)
			nPointCount++;

		if( sides[i] != SIDE_ON &&
			sides[i+1] != SIDE_ON &&
			sides[i] != sides[i+1] )
		{
			nPointCount++;
		}
	}

	RWinding *neww= new RWinding(nPointCount);

	nPointCount=0;
	for (i=0;i<pWinding->nCount;i++)
	{
		rvector *p1 = &pWinding->pVertices[i];

		if( sides[i] != SIDE_BACK )
		{
			neww->pVertices[nPointCount]=*p1;
			nPointCount++;
		}

		if( sides[i] != SIDE_ON &&
			sides[i+1] != SIDE_ON &&
			sides[i] != sides[i+1] )
		{
			rvector *p2  = &pWinding->pVertices[(i+1)%pWinding->nCount];
			rvector *mid = &neww->pVertices[nPointCount];
			nPointCount++;

			float t = dists[i] / (dists[i]-dists[i+1]);
			*mid = *p1 + t * ( *p2 - *p1 );
		}
	}

	// 원본은 해제하고 새로 생성된 winding을 반환.
	delete pWinding;
	return neww;
}


struct RNode;

struct RPortal {

	RPortal() { pWinding=NULL; pFrom=pTo=NULL; }
	~RPortal() { SAFE_DELETE(pWinding); }

	rplane		plane;
	RWinding	*pWinding;
	RNode		*pFrom,*pTo;
	int			nID;
};

struct lplane
{
	bool operator()(const rplane *a, const rplane *b) const
	{
		return 

		a->d < b->d ? true : 
		a->d > b->d ? false :

		a->a < b->a ? true :
		a->a > b->a ? false :

		a->b < b->b ? true :
		a->b > b->b ? false :

		a->c < b->c ? true :
		a->c > b->c ? false :

		false;
	}
};

typedef list<RPortal*> RPORTALLIST;

#define IS_EQ_PLANE(x,y) (IS_EQ(x.a,y.a) && IS_EQ(x.b,y.b) && IS_EQ(x.c,y.c) && IS_EQ(x.d,y.d))

int g_nPortalCount;	// 포탈의 개수. 아이디를 생성할때 쓰임


class RPortalList : public RPORTALLIST {
public:
	iterator find(rplane &plane) {
		for(iterator i=begin();i!=end();i++)
		{
			if(IS_EQ_PLANE((*i)->plane,plane))
				return i;
		}
		return end();
	}

	void Split(rplane &plane) {
		for(iterator i=begin();i!=end();)
		{
			RPortal *pPortal=*i;
			if(!IS_EQ_PLANE(pPortal->plane,plane))
			{
				RPortal *pFront=new RPortal;
				pFront->nID=g_nPortalCount++;
				pFront->pFrom=pPortal->pFrom;
				pFront->pTo=pPortal->pTo;
				pFront->pWinding=new RWinding(pPortal->pWinding);
				pFront->pWinding=ClipWinding(pFront->pWinding,plane);
				if(pFront->pWinding)
				{
					pFront->plane=pPortal->plane;
					i=insert(i,pFront);
					i++;
				}else
					delete pFront;

				// 뒷쪽winding 은 있는걸 활용한다..
				pPortal->pWinding=ClipWinding(pPortal->pWinding,-plane);
				if(!pPortal->pWinding)
				{
					delete(pPortal);
					i=erase(i);
					continue;
				}
			}
			i++;
		}
	}

};

struct RNode {
	RSBspNode *pNode;
	RPortalList portallist;
	RNode *m_pPositive,*m_pNegative;
	bool	bVisited;

	RNode() { m_pPositive=m_pNegative=NULL; }
	~RNode() { 
		SAFE_DELETE(m_pPositive);
		SAFE_DELETE(m_pNegative);
	}

	RNode *GetLeafNode(rvector &pos)
	{
		if(!pNode || (!pNode->m_pNegative && !pNode->m_pPositive)) {
			/*
			rboundingbox *bb=&pNode->bbTree;
			if(pos.x<bb->minx || bb->maxx<pos.x ||
				pos.y<bb->miny || bb->maxy<pos.y || 
				pos.z<bb->minz || bb->maxz<pos.z) return NULL; 
				*/
			return this;
		}
		if(pNode->plane.a*pos.x+pNode->plane.b*pos.y+pNode->plane.c*pos.z+pNode->plane.d>0)
			return m_pPositive ? m_pPositive->GetLeafNode(pos) : NULL;
		else
			return m_pNegative ? m_pNegative->GetLeafNode(pos) : NULL;
	}


};

RWinding *g_testwinding=NULL;
RWinding *g_testwinding2=NULL;
RWinding *g_testwinding3=NULL;

RBspObject *g_pbsp=NULL;
RSBspNode *g_pRootNode=NULL;

// 바운딩박스의 한면에 해당하는 winding을 만들어 리턴한다.
// i순서로 x_max - x_min - y_max - y_min - z_max - z_min
RWinding *NewWindingFromBoundingBox(rboundingbox &bb,int i)
{
	int nAxis=i/2;
	float fCoordAxis= (i%2) ? bb.vmin[nAxis] : bb.vmax[nAxis];

	int nAxis1=(nAxis+1)%3,nAxis2=(nAxis+2)%3;

	RWinding *w= new RWinding(4);
	w->pVertices[0][nAxis]=fCoordAxis;
	w->pVertices[0][nAxis1]=bb.vmin[nAxis1];
	w->pVertices[0][nAxis2]=bb.vmin[nAxis2];

	w->pVertices[1][nAxis]=fCoordAxis;
	w->pVertices[1][nAxis1]=bb.vmin[nAxis1];
	w->pVertices[1][nAxis2]=bb.vmax[nAxis2];

	w->pVertices[2][nAxis]=fCoordAxis;
	w->pVertices[2][nAxis1]=bb.vmax[nAxis1];
	w->pVertices[2][nAxis2]=bb.vmax[nAxis2];

	w->pVertices[3][nAxis]=fCoordAxis;
	w->pVertices[3][nAxis1]=bb.vmax[nAxis1];
	w->pVertices[3][nAxis2]=bb.vmin[nAxis2];

	return w;
}

RWinding *NewLargeWinding(rplane &plane)
{
#define PORTALMAX 100000.f
#define PORTALMIN -100000.f

	RWinding *w=new RWinding(4);

	int nAxis;

	if(fabs(plane.a)>fabs(plane.b) && fabs(plane.a)>fabs(plane.c) )
		nAxis=0;
	else if(fabs(plane.b)>fabs(plane.c))
		nAxis=1;
	else
		nAxis=2;

	int nAxis1=(nAxis+1)%3,nAxis2=(nAxis+2)%3;

	float *fplane=(float*)plane;
	w->pVertices[0][nAxis1]=PORTALMIN;
	w->pVertices[0][nAxis2]=PORTALMIN;
	w->pVertices[0][nAxis]=-(fplane[nAxis1]*w->pVertices[0][nAxis1]+fplane[nAxis2]*w->pVertices[0][nAxis2]+plane.d)/fplane[nAxis];

	w->pVertices[1][nAxis1]=PORTALMAX;
	w->pVertices[1][nAxis2]=PORTALMIN;
	w->pVertices[1][nAxis]=-(fplane[nAxis1]*w->pVertices[1][nAxis1]+fplane[nAxis2]*w->pVertices[1][nAxis2]+plane.d)/fplane[nAxis];

	w->pVertices[2][nAxis1]=PORTALMAX;
	w->pVertices[2][nAxis2]=PORTALMAX;
	w->pVertices[2][nAxis]=-(fplane[nAxis1]*w->pVertices[2][nAxis1]+fplane[nAxis2]*w->pVertices[2][nAxis2]+plane.d)/fplane[nAxis];

	w->pVertices[3][nAxis1]=PORTALMIN;
	w->pVertices[3][nAxis2]=PORTALMAX;
	w->pVertices[3][nAxis]=-(fplane[nAxis1]*w->pVertices[3][nAxis1]+fplane[nAxis2]*w->pVertices[3][nAxis2]+plane.d)/fplane[nAxis];

	return w;

}

// plane 에 잘려지는가 ?
bool CheckWinding(RWinding *pWinding, rplane &plane)
{
	SIDE sides[MAX_WINDING_POINTS];
	float dists[MAX_WINDING_POINTS];

	int nFront=0,nBack=0;
	int i;
	for(i=0;i<pWinding->nCount;i++)
	{
		float fDot=D3DXPlaneDotCoord(&plane,&pWinding->pVertices[i]);
		if(fDot>ON_EPSILON) {
			sides[i]=SIDE_FRONT;
			nFront++;
		} else 
			if(fDot<-ON_EPSILON) {
				sides[i]=SIDE_BACK;
				nBack++;
			}
			else
				sides[i]=SIDE_ON;
		dists[i]=fDot;
	}
	sides[i]=sides[0];
	dists[i]=dists[0];

	if(!nBack)	// 전혀 잘려지지 않음.
		return false;

	return true;
}

void GetPlane(rplane *plane,rboundingbox &bb,int i)
{
	int nAxis=i/2;
	int nAxis1=(nAxis+1)%3,nAxis2=(nAxis+2)%3;

	float *fplane=(float*)*plane;
	fplane[nAxis1]=fplane[nAxis2]=0;
	fplane[nAxis]=(i%2) ? 1 : -1;
	plane->d=(i%2) ?  -bb.vmin[nAxis] : bb.vmax[nAxis];
}


// 인접한 노드를 구한다. 이건 미리 인덱스로 만들어 둘수 있다. i는 방향
RSBspNode *GetAdjacencyNode(RSBspNode *pNode, int i)
{
	rvector pos=.5f*(pNode->bbTree.vmax+pNode->bbTree.vmin);
	int nAxis=i/2;

	pos[nAxis]=(i%2) ? pNode->bbTree.vmin[nAxis]-2.f : pNode->bbTree.vmax[nAxis]+2.f;
	return g_pRootNode->GetLeafNode(pos);
}

enum CLIP_SEPERATOR_DIR
{ 
	CLIP_SEPERATOR_FORWARD,
	CLIP_SEPERATOR_BACKWARD
};

RWinding *ClipToSeperators ( RWinding *source, RWinding *pass, RWinding *target, CLIP_SEPERATOR_DIR clipdir )
{
	long	cur_src_point;
	long	next_src_point;
	long	cur_pass_point;
	rvector	v1, v2;
	rplane	plane;

	long	i;
	float	d;
	float	length;
	long	front_count;

	if( ! source || ! pass ) return target;

	//	source의 모서리와 pass의 한점을 선택하는 모든 조합을 만든다.
	for (cur_src_point=0 ; cur_src_point<source->nCount; cur_src_point++)
	{
		// source 로부터 한 모서리를 선택한다.
		next_src_point = (cur_src_point+1)%source->nCount;
		v1 = source->pVertices[next_src_point] - source->pVertices[cur_src_point];

		for (cur_pass_point=0 ; cur_pass_point<pass->nCount; cur_pass_point++)
		{
			// pass 로부터 한 점을 선택한다.
			v2 = pass->pVertices[cur_pass_point] - source->pVertices[cur_src_point];

			// 선택한 모서리와 점으로 평면(plane)을 만든다.
			rvector normal;
			CrossProduct( &normal,  v1 ,v2  );
			length = DotProduct( normal, normal );			
			if (length < ON_EPSILON)
				continue;
			length = (float)sqrt(length);
			plane.a = normal.x/length;
			plane.b = normal.y/length;
			plane.c = normal.z/length;
			plane.d = -D3DXPlaneDotNormal(&plane,&pass->pVertices[cur_pass_point]);

			// plane과 source의위치관계를 조사한다.
			front_count = 0;
			for (i=0 ; i<source->nCount ; i++)
			{
				if (i == cur_src_point || i == next_src_point)
					continue;
				d = D3DXPlaneDotCoord(&plane,&source->pVertices[i]);
				if (d < -ON_EPSILON)
					break;
				if (d > ON_EPSILON)
				{
					front_count++;
					// source는 plane의 양쪽에 걸쳐져 있을 수 없다.
					// 따라서 한점만 앞에있어도 source가 앞에있다고 판단.
					break;
				}
			}
			// source가 plane상에 있는경우는 취급하지 않는다.
			// 이것은 [ 가시성 판단 3 ] 에서 처리한다.
			// [ 가시성 판단 3 ] 을 수행하지 않고 이 함수를 수행하려 한다면
			// 함수가 좀 복잡해진다.
			if (i == source->nCount)
				continue;

			// source가 plane의 뒤쪽에 위치하도록 평면방정식을 조정한다.			
			if ( front_count )
				plane=-plane;

			// plane과 pass의위치관계를 조사한다.
			// pass가 plane의 앞쪽에 완전히 포함되는 경우,
			// plane으로 target을 클리핑한다.
			front_count = 0;
			for (i=0 ; i<pass->nCount; i++)
			{
				if (i==cur_pass_point)
					continue;
				d = D3DXPlaneDotCoord(&plane,&pass->pVertices[i]);
				if (d < -ON_EPSILON)
					break;
				if (d > ON_EPSILON)
					front_count++;
			}
			// 한점이라도 plane의 뒤쪽에 있으면 안된다.
			if (i != pass->nCount)
				continue;
			// pass가 평면에 포함되어서도 안된다.
			if ( ! front_count )
				continue;

			// 입력된 parameter 가
			// source -> pass -> target 순이 아니라
			// pass -> source -> target 순이라면 plane을 뒤집어야 한다.
			if ( clipdir == CLIP_SEPERATOR_BACKWARD )
			{
				plane=-plane;
			}

			// 여기까지 왔다면 plane은 뷰볼륨을 이루는 평면이다.
			// target을 잘라낸다.
			target = ClipWinding (target, plane);
			if (!target)
				return NULL;		// target is not visible
		}
	}

	return target;
}

list<RSBspNode *> g_nodelist;

RNode *testnode=NULL;
RNode *g_Tree;

void DrawTest(RNode *pNode)
{
	if(!pNode) return;

	RPORTALLIST::iterator i=pNode->portallist.begin();
	for(size_t n=0;n<pNode->portallist.size();n++)
	{
		RPortal *pPortal=*i;
		if(pPortal->pWinding)
			pPortal->pWinding->Draw(0xffff00ff);
		i++;
	}

	/*
	DrawTest(pNode->m_pPositive);
	DrawTest(pNode->m_pNegative);
	*/

	/*
	for(int i=0;i<testnode->pNode->nPolygon;i++)
	{
	testnode->pNode->DrawWireFrame(i,0x808080);
	}
	testnode->pNode->DrawWireFrame(0,0x202020);
	testnode->pNode->DrawWireFrame(1,0x404040);
	testnode->pNode->DrawWireFrame(2,0x606060);
	//		testnode->pNode->DrawWireFrame(3,0x808080);
	*/

}


void DrawDebug()
{
/*
	//	g_pRootNode->DrawBoundingBox(0xffffffff);

	{
		for(list<RSBspNode*>::iterator i=g_nodelist.begin();i!=g_nodelist.end();i++)
		{
			RSBspNode *pnode=*i;
			pnode->DrawBoundingBox(0xffffffff);
		}
	}

	RNode *testnode=g_Tree->GetLeafNode(RCameraPosition);

	if(testnode)
	{
		static DWORD lastprinttime=GetGlobalTimeMS();
		if(GetGlobalTimeMS()-lastprinttime>200)
		{
			lastprinttime=GetGlobalTimeMS();
			mlog("current node has %d portals\n",testnode->portallist.size());
		}
	}

	RSBspNode *pNode=testnode->pNode;
	if(pNode)
		pNode->DrawBoundingBox(0xffffffff);

	DrawTest(testnode);
*/

	/*
	// 기본노드
	if(g_pRootNode)
	{
	RSBspNode *pNode=g_pRootNode->GetLeafNode(rvector(1,1,1));
	pNode->DrawBoundingBox(0x602020);
	}
	*/

	

	if(g_testwinding)
		g_testwinding->Draw(0xff0000);
	if(g_testwinding2)
		g_testwinding2->Draw(0xffff00);
	if(g_testwinding3)
		g_testwinding3->Draw(0x00ffff);

}

struct RFindPVSParam {
	int nDirFrom;
	RNode	*pNode;
	RWinding *pSource;
	RWinding *pTarget;
	rplane targetplane;

	RFindPVSParam() {
		pSource=NULL;
		pTarget=NULL;
	}
	~RFindPVSParam() { 
		SAFE_DELETE(pSource);
		SAFE_DELETE(pTarget);
	}
};

/*
int nCount=0;

void FindNodes(RNode *pNodeFrom,RFindPVSParam *pParam)
{
	nCount++;
	if(nCount>10) 
	{
		//		return;
	}

	RSBspNode *pNode=pParam->pNode->pNode;

	if(!pNode->bVisibletest)
	{
		pNode->bVisibletest=true;
		g_nodelist.push_back(pNode);
	}

	RPORTALLIST *portallist=&pParam->pNode->portallist;
	for(RPORTALLIST::iterator i=portallist->begin();i!=portallist->end();i++)
	{
		RPortal *pPortal=*i;
		_ASSERT(pPortal->pWinding->nCount>0 && pPortal->pWinding->nCount<100);

		if(pPortal->pTo==pNodeFrom) continue;

		RFindPVSParam *pp=new RFindPVSParam;
		pp->pNode=pPortal->pTo;
		pp->pSource=new RWinding(pParam->pSource);
		pp->pTarget=new RWinding(pPortal->pWinding);
		pp->targetplane=pPortal->plane;

		if(pParam->pTarget)
		{
			pp->pTarget=ClipWinding(pp->pTarget,-pParam->targetplane);
			if(!pp->pTarget) goto exit;
			pp->pTarget=ClipToSeperators(pp->pSource,pParam->pTarget,pp->pTarget,CLIP_SEPERATOR_FORWARD);
			if(!pp->pTarget) goto exit;
			pp->pTarget=ClipToSeperators(pParam->pTarget,pp->pSource,pp->pTarget,CLIP_SEPERATOR_BACKWARD);
			if(!pp->pTarget) goto exit;
			pp->pSource=ClipWinding(pp->pSource,pParam->targetplane);
			if(!pp->pSource) goto exit;
			pp->pSource=ClipToSeperators(pp->pTarget,pParam->pTarget,pp->pSource,CLIP_SEPERATOR_FORWARD);
			if(!pp->pSource) goto exit;
			pp->pSource=ClipToSeperators(pParam->pTarget,pp->pTarget,pp->pSource,CLIP_SEPERATOR_BACKWARD);
			if(!pp->pSource) goto exit;
		}

		FindNodes(pParam->pNode,pp);
exit:
		delete pp;
	}
}

void MakeVisTest(RBspObject *pbsp)
{
	g_pRootNode=pbsp->GetOcRootNode();

	RNode *pNode=g_Tree->GetLeafNode(TEST_NODE_POS);

	RPORTALLIST *portallist=&pNode->portallist;
	for(RPORTALLIST::iterator i=portallist->begin();i!=portallist->end();i++)
	{
		RPortal *pPortal=*i;

		RFindPVSParam *pp=new RFindPVSParam;
		pp->pNode=pPortal->pTo;
		pp->pSource=new RWinding(pPortal->pWinding);
		pp->pTarget=NULL;

		pp->pSource->Dump();
		FindNodes(pNode,pp);
		delete pp;
	}
}
*/

/*
void ClipPortal(RNode *rn,RPortalList *pportallist)
{
	RSBspNode *pNode=rn->pNode;

	// leaf node 여야한다
	_ASSERT(!rn->m_pPositive && !rn->m_pNegative);

	//// 일단 포탈이 아닌넘들을 지운다
	//for(RPORTALLIST::iterator i=pportallist->begin();i!=pportallist->end();)
	//{
	//	RPortal *pPortal=*i;
	//	if(pPortal->pFrom==pPortal->pTo)
	//	{
	//		i=pportallist->erase(i);
	//		delete pPortal;
	//	}else
	//		i++;
	//}

	if(pportallist->empty()) return;

	if(pNode)
	{
		for(int j=0;j<pNode->nPolygon;j++)
		{
			RPOLYGONINFO *pInfo=&pNode->pInfo[j];

			for(RPORTALLIST::iterator i=pportallist->begin();i!=pportallist->end();)
			{
				RPortal *pPortal=*i;
				_ASSERT(pPortal->pWinding->nCount>0 && pPortal->pWinding->nCount<100);

				if(IS_EQ_PLANE(pPortal->plane,pInfo->plane))
				{
					bool bSkipSplit=false;
					rplane planes[3];

					for(int k=0;k<3;k++)
					{
						rvector apoint=*pNode->pVertices[j*3+k].Coord();
						rvector edge=apoint-*pNode->pVertices[j*3+(k+1)%3].Coord();
						rvector normal;
						CrossProduct(&normal,edge,rvector(pInfo->plane.a,pInfo->plane.b,pInfo->plane.c));
						Normalize(normal);

						D3DXPlaneFromPointNormal(&planes[k],&apoint,&normal);
						if(!CheckWinding(pPortal->pWinding,planes[k]))
						{
							bSkipSplit=true;
							break;
						}
					}
					if(bSkipSplit) 
					{
						i++;
						continue;	// 완전히 바깥에 있으므로 자를 필요가 없다.
					}

					for(k=0;k<3;k++)
					{
						rplane *edgeplane=&planes[k];
						RWinding *pPortalWinding;
						if(k==2)
						{
							pPortalWinding=ClipWinding(pPortal->pWinding,*edgeplane);
							pPortal->pWinding=NULL;
						}
						else
						{
							pPortalWinding=new RWinding(pPortal->pWinding);
							pPortalWinding=ClipWinding(pPortalWinding,*edgeplane);
							pPortal->pWinding=ClipWinding(pPortal->pWinding,-*edgeplane);
						}

						if(pPortalWinding)
						{
							RPortal *pp=new RPortal;
							pp->pWinding=pPortalWinding;
							_ASSERT(pp->pWinding->nCount>0 && pp->pWinding->nCount<100);
							pp->plane=pPortal->plane;
							pp->pFrom=pPortal->pFrom;
							pp->pTo=pPortal->pTo;

							//if(!g_testwinding)
							//g_testwinding=pPortalWinding;
							//else
							//if(!g_testwinding2)
							//g_testwinding2=pPortalWinding;
							//else
							//if(!g_testwinding3)
							//g_testwinding3=pPortalWinding;

							i=pportallist->insert(i,pp);
							i++;
						}
						if(!pPortal->pWinding) break;
					}

					delete pPortal;
					i=pportallist->erase(i);
				}else
					i++;
			}
		}
	}

	rn->portallist.insert(rn->portallist.end(),pportallist->begin(),pportallist->end());
	while(!pportallist->empty())
	{
		pportallist->erase(pportallist->begin());
	}

}

void PushPortal(RNode *rn, RPortal *pPortal)
{
	RSBspNode *pNode=rn->pNode;

	// leaf node 이면 OK
	if(!rn->m_pPositive && !rn->m_pNegative)
	{
		return;
	}

	if(pNode->plane==pPortal->plane)		// 같은 평면위에 있으면 앞뒤 양쪽으로 넣는다.
	{
		if(rn->m_pPositive && rn->m_pNegative)
		{
			RPortal *pBack=new RPortal;
			pBack->nID=pPortal->nID;		// 같은 ID로 밀어넣는다
			pBack->pWinding=new RWinding(pPortal->pWinding);
			pBack->plane=-pPortal->plane;


			// front
			pPortal->pFrom=rn->m_pPositive;
			PushPortal(rn->m_pPositive,pPortal);

			// back
			pBack->pFrom=rn->m_pNegative;
			PushPortal(rn->m_pNegative,pBack);

			pBack->pTo=pPortal->pFrom;
			pPortal->pTo=pBack->pFrom;

			if(pPortal->pFrom!=pPortal->pTo){
				RPortalList portallist;
				portallist.push_back(pPortal);
				ClipPortal(pPortal->pFrom,&portallist);

				portallist.push_back(pBack);
				ClipPortal(pBack->pFrom,&portallist);
			}
		}else
		{
			_ASSERT(FALSE);
		}
	}else
	{
		int np=0,nz=0,nn=0;
		for(int i=0;i<pPortal->pWinding->nCount;i++)	// 그렇지 않으면 앞 혹은 뒤로 밀어넣는다.
		{
			float fSide;
			fSide=D3DXPlaneDotCoord(&pNode->plane,&pPortal->pWinding->pVertices[i]);

			// 발견되면 바로 return 해도 됨. 나중에 수정요망.
			if(fSide<-ON_EPSILON)
			{
				nn++;
			}else
				if(fSide>ON_EPSILON)
				{
					np++;
				}else
					nz++;
		}

		_ASSERT(np*nn==0);

		if(np)
		{
			if(pPortal->pTo==rn) pPortal->pTo=rn->m_pPositive;
			if(pPortal->pFrom==rn) pPortal->pFrom=rn->m_pPositive;
			PushPortal(rn->m_pPositive,pPortal);
		}

		if(nn)
		{
			if(pPortal->pTo==rn) pPortal->pTo=rn->m_pNegative;
			if(pPortal->pFrom==rn) pPortal->pFrom=rn->m_pNegative;
			PushPortal(rn->m_pNegative,pPortal);
		}
	}
}

void SplitPortal(RNode *rn,RPortal *pSourcePortal,RPortalList *portallist)
{
	if(!pSourcePortal->pWinding) {
		delete pSourcePortal;	
		return;
	}

	if(!rn->m_pPositive && !rn->m_pNegative) {
		delete pSourcePortal;	
		return;
	}

	RSBspNode *pNode=rn->pNode;

	RPortal *pFront=new RPortal;
	pFront->plane=pSourcePortal->plane;
	pFront->pWinding=new RWinding(pSourcePortal->pWinding);

	if(pNode->plane!=pSourcePortal->plane) {
		pFront->pWinding=ClipWinding(pFront->pWinding,pNode->plane);
		// 뒷쪽winding 은 있는걸 활용한다..
		pSourcePortal->pWinding=ClipWinding(pSourcePortal->pWinding,-pNode->plane);

		portallist->Split(pNode->plane);
	}

	if(rn->m_pPositive)
		SplitPortal(rn->m_pPositive,pFront,portallist);
	if(rn->m_pNegative)
		SplitPortal(rn->m_pNegative,pSourcePortal,portallist);
}
*/

/*
void GiveIDToEachPortal()
{
	for(RPortalList::iterator i=g_Portals.begin();i!=g_Portals.end();i++)
	{
		(*i)->nID=g_nPortalCount;
		g_nPortalCount++;
	}
}
*/

/*
void GeneratePortals(RNode *rn)
{
	if(!rn->m_pPositive && !rn->m_pNegative) return;

	RSBspNode *pNode=rn->pNode;
	if(!pNode->m_pPositive && !pNode->m_pNegative) return;

//	if(g_Portals.find(pNode->plane)==g_Portals.end())
	{
		//		_ASSERT(fmod(pNode->plane.d,800)<2.f);
		RPortal *pPortal=new RPortal;
		pPortal->nID=g_nPortalCount++;
		pPortal->pWinding=NewLargeWinding(pNode->plane);
		pPortal->plane=pNode->plane;

		for(int k=0; k<6; k++)
		{
			rplane plane;
			GetPlane(&plane,g_pRootNode->bbTree,k);
			pPortal->pWinding=ClipWinding(pPortal->pWinding,plane);
			if(!pPortal->pWinding) break;
		}
		if(pPortal->pWinding)
		{
			pPortal->plane=pNode->plane;

			RPortalList portallist;
			portallist.push_back(pPortal);

			static int nCount=0;
			mlog("push portal %d %d \n",nCount++,g_nPortalCount);

			RPortal *pSourcePortal=new RPortal;
			pSourcePortal->plane=pPortal->plane;
			pSourcePortal->pWinding=new RWinding(pPortal->pWinding);
			SplitPortal(g_Tree,pSourcePortal,&portallist);
			
			RPortalList::iterator i;
			for(i=portallist.begin();i!=portallist.end();i++)
			{
				PushPortal(g_Tree,*i);
			}

		}else
			delete pPortal;
	}

	if(rn->m_pPositive)
		GeneratePortals(rn->m_pPositive);
	if(rn->m_pNegative)
		GeneratePortals(rn->m_pNegative);
}

RNode *MakeTree(RSBspNode *pNode)
{
	if(!pNode) return NULL;

	RNode *pnode=new RNode;
	pnode->pNode=pNode;

	pnode->m_pNegative=MakeTree(pNode->m_pNegative);
	pnode->m_pPositive=MakeTree(pNode->m_pPositive);

	if(pnode->m_pNegative!=NULL && pnode->m_pPositive==NULL )
	{
		pnode->m_pPositive=new RNode;
		pnode->m_pPositive->pNode=NULL;
	}


	if(pnode->m_pNegative==NULL && pnode->m_pPositive!=NULL )
	{
		pnode->m_pNegative=new RNode;
		pnode->m_pNegative->pNode=NULL;
	}

	return pnode;
}

void ClearCheckEmpty(RNode *rn)
{
	if(NULL == rn) return;

	rn->bVisited=false;

	ClearCheckEmpty(rn->m_pPositive);
	ClearCheckEmpty(rn->m_pNegative);
}

int g_nEmptyCount=0;

void CheckEmpty(RNode *rn)
{
	if(rn->bVisited) return;
	rn->bVisited=true;

	if(rn->pNode)
	{
		rn->pNode->m_bSolid=false;
		g_nEmptyCount++;
	}

	for(RPortalList::iterator i=rn->portallist.begin();i!=rn->portallist.end();i++)
	{
		RPortal *pPortal=*i;

		_ASSERT(pPortal->pFrom==rn);
		
		CheckEmpty(pPortal->pTo);
	}
}
*/

void RBspObject::test_MakePortals()
{
	/*
	g_pRootNode=GetRootNode();
	g_Tree=MakeTree(GetRootNode());
	*/

	/*

	mlog("node count %d\n",GetBspNodeCount());

	g_nPortalCount=0;
	mlog("Generate Portals\n");
	GeneratePortals(g_Tree);

	mlog("Find Visible Nodes\n");
//	MakeVisTest(this);

	RNode *testnode=g_Tree->GetLeafNode(rvector(0,0,100));
	ClearCheckEmpty(g_Tree);
	CheckEmpty(testnode);

	mlog("\nempty node count %d\n",g_nEmptyCount);
*/
}

