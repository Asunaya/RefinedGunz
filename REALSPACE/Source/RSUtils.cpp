#include "rsutils.h"
#include "realspace.h"
#include "RSDebug.h"
#include "RSMMX.h"

#define NVERTEX_BUFFER 15
#define FPLANE(v) (plane->a*(v)->x+plane->b*(v)->y+plane->c*(v)->z+plane->d)

// these routine Draw with clip plane 3 is especially for detailed shadow.
static rvertex added[NVERTEX_BUFFER],*verlist[2][NVERTEX_BUFFER],**v;
static rplane  *plane;
static int nv,nva;

void Check(rvertex *v1,rvertex *v2)
{
	SIGN a=signof(FPLANE(v1)),b=signof(FPLANE(v2));
	if(a==POSITIVE)
	{
		*v=v1;
		v++;
	}
	if(a*b==-1)
	{
		rvertex *temp=&added[nva];
		*v=temp;
		v++;
		nva++;
	
		float dx=v2->x-v1->x,dy=v2->y-v1->y,dz=v2->z-v1->z;
		float t=-(v1->x*plane->a+v1->y*plane->b+v1->z*plane->c+plane->d)/
				(dx*plane->a+dy*plane->b+dz*plane->c);
		temp->x=v1->x+dx*t;
		temp->y=v1->y+dy*t;
		temp->z=v1->z+dz*t;
		temp->sx=v1->sx+(v2->sx-v1->sx)*t;
		temp->sy=v1->sy+(v2->sy-v1->sy)*t;
		temp->sz=v1->sz+(v2->sz-v1->sz)*t;
		temp->pw=v1->pw+(v2->pw-v1->pw)*t;
		temp->u=(v1->u*v1->pw+(v2->u*v2->pw-v1->u*v1->pw)*t)/temp->pw;
		temp->v=(v1->v*v1->pw+(v2->v*v2->pw-v1->v*v1->pw)*t)/temp->pw;

		_ASSERT((0<=t)&&(t<=1));
		temp->color=(DWORD((1-t)*float(v1->color&0xff0000)+float(v2->color&0xff0000)*t)&0xff0000)|
					(DWORD((1-t)*float(v1->color&0xff00)+float(v2->color&0xff00)*t)&0xff00)|
					(DWORD((1-t)*float(v1->color&0xff)+float(v2->color&0xff)*t)&0xff);

	}
}

void CheckIntersectWorldOnly(rvertex *v1,rvertex *v2)
{
	SIGN a=signof(FPLANE(v1)),b=signof(FPLANE(v2));
	if(a==POSITIVE)
	{
		*v=v1;
		v++;
	}
	if(a*b==-1)
	{
		rvertex *temp=&added[nva];
		*v=temp;
		v++;
		nva++;
	
		float dx=v2->x-v1->x,dy=v2->y-v1->y,dz=v2->z-v1->z;
		float t=-(v1->x*plane->a+v1->y*plane->b+v1->z*plane->c+plane->d)/
				(dx*plane->a+dy*plane->b+dz*plane->c);
		temp->x=v1->x+dx*t;
		temp->y=v1->y+dy*t;
		temp->z=v1->z+dz*t;
		temp->u=(v1->u*v1->pw+(v2->u*v2->pw-v1->u*v1->pw)*t)/temp->pw;
		temp->v=(v1->v*v1->pw+(v2->v*v2->pw-v1->v*v1->pw)*t)/temp->pw;

		_ASSERT((0<=t)&&(t<=1));
		temp->color=(DWORD((1-t)*float(v1->color&0xff0000)+float(v2->color&0xff0000)*t)&0xff0000)|
					(DWORD((1-t)*float(v1->color&0xff00)+float(v2->color&0xff00)*t)&0xff00)|
					(DWORD((1-t)*float(v1->color&0xff)+float(v2->color&0xff)*t)&0xff);

	}
}

void DrawFaceWithClip3(rface *face,rplane *planes)
{
//	log("entering draw face with clip ...\n");
	nv=0;nva=0;

// pass 1 : plane 1  face->v[1..3] -> verlist[0]
	v=verlist[0];
	plane=planes;
	Check(face->v[0],face->v[1]);
	Check(face->v[1],face->v[2]);
	Check(face->v[2],face->v[0]);
	if(v==verlist[0]) return;
//	log("pass 1 : %d vertices \n",v-verlist[0]);

// pass 2: plane 2  verlist[0] -> verlist[1]
	rvertex **i=verlist[0],**end=v-1;
	plane++;
	v=verlist[1];
	while(i<end){Check(*i,*(i+1));i++;}Check(*i,verlist[0][0]);
	if(v==verlist[1]) return;
//	log("pass 2 : %d vertices \n",v-verlist[1]);

// pass 3: plane 3  verlist[1] -> verlist[0]
	i=verlist[1];end=v-1;
	plane++;
	v=verlist[0];
	while(i<end){Check(*i,*(i+1));i++;}Check(*i,verlist[1][0]);
	if(v==verlist[0]) return;
//	log("pass 3 : %d vertices \n",v-verlist[0]);

// pass 4 : Draw.
	static rface f;
	i=verlist[0];end=v-2;
	while(i<v) {(*i)->color=0x80000000;(*i)->specular=0xff000000;
				(*i)->sz-=(1.5f/65536.0f);i++;}
	i=verlist[0];
	while(i<end)
	{
		i++;
		RSDrawTriangle(verlist[0][0],*i,*(i+1));
	}
}

void DrawFaceWithClip4(rface *face,rplane *planes)
{
//	log("entering draw face with clip ...\n");
	nv=0;nva=0;

	face->v[0]->u=face->s[0];face->v[0]->v=face->t[0];
	face->v[1]->u=face->s[1];face->v[1]->v=face->t[1];
	face->v[2]->u=face->s[2];face->v[2]->v=face->t[2];

// pass 1 : plane 1  face->v[1..3] -> verlist[0]
	v=verlist[0];
	plane=planes;
	Check(face->v[0],face->v[1]);
	Check(face->v[1],face->v[2]);
	Check(face->v[2],face->v[0]);
	if(v==verlist[0]) return;
//	log("pass 1 : %d vertices \n",v-verlist[0]);

// pass 2: plane 2  verlist[0] -> verlist[1]
	rvertex **i=verlist[0],**end=v-1;
	plane++;
	v=verlist[1];
	while(i<end){Check(*i,*(i+1));i++;}Check(*i,verlist[0][0]);
	if(v==verlist[1]) return;
//	log("pass 2 : %d vertices \n",v-verlist[1]);

// pass 3: plane 3  verlist[1] -> verlist[0]
	i=verlist[1];end=v-1;
	plane++;
	v=verlist[0];
	while(i<end){Check(*i,*(i+1));i++;}Check(*i,verlist[1][0]);
	if(v==verlist[0]) return;
//	log("pass 3 : %d vertices \n",v-verlist[0]);

// pass 4: plane 4  verlist[0] -> verlist[1]
	i=verlist[0];end=v-1;
	plane++;
	v=verlist[1];
	while(i<end){Check(*i,*(i+1));i++;}Check(*i,verlist[0][0]);
	if(v==verlist[1]) return;
//	log("pass 4 : %d vertices \n",v-verlist[1]);

// pass 5 : Draw.
	static rface f;
	i=verlist[1];end=v-2;
	while(i<end)
	{
		i++;
		RSDrawTriangle(verlist[1][0],*i,*(i+1));
	}
}

void DrawWorldFaceWithClip3(rface *face,rplane *planes,rmatrix44 *m)
{
//	log("entering draw face with clip ...\n");
	nv=0;nva=0;

// pass 1 : plane 1  face->v[1..3] -> verlist[0]
	v=verlist[0];
	plane=planes;
	CheckIntersectWorldOnly(face->v[0],face->v[1]);
	CheckIntersectWorldOnly(face->v[1],face->v[2]);
	CheckIntersectWorldOnly(face->v[2],face->v[0]);
	if(v==verlist[0]) return;
//	log("pass 1 : %d vertices \n",v-verlist[0]);

// pass 2: plane 2  verlist[0] -> verlist[1]
	rvertex **i=verlist[0],**end=v-1;
	plane++;
	v=verlist[1];
	while(i<end){CheckIntersectWorldOnly(*i,*(i+1));i++;}CheckIntersectWorldOnly(*i,verlist[0][0]);
	if(v==verlist[1]) return;
//	log("pass 2 : %d vertices \n",v-verlist[1]);

// pass 3: plane 3  verlist[1] -> verlist[0]
	i=verlist[1];end=v-1;
	plane++;
	v=verlist[0];
	while(i<end){CheckIntersectWorldOnly(*i,*(i+1));i++;}CheckIntersectWorldOnly(*i,verlist[1][0]);
	if(v==verlist[0]) return;
//	log("pass 3 : %d vertices \n",v-verlist[0]);

// pass 4 : Draw.
	static rface f;
	i=verlist[0];end=v-2;
	while(i<v) 
	{
		TransformVertex(**i,*m);			
			(*i)->color=0x80000000;(*i)->specular=0xff000000;
				(*i)->sz-=(2.f/65536.0f);i++;}
	i=verlist[0];
	while(i<end)
	{
		i++;
		RSDrawTriangle(verlist[0][0],*i,*(i+1));
	}
}

bool GetVerlistClipped(rvertex **in,int nin,rvertex ***out,int *nout)
{
	nv=0;nva=0;

	if(nin>NVERTEX_BUFFER) return false;

	rvertex **i,**end;
	int iVerBuff=0;
	
	RSMemCopy(verlist[0],in,sizeof(void*)*nin);v=&verlist[0][nin];
	//memcpy(verlist[0],in,sizeof(void*)*nin);v=&verlist[0][nin];
	for(int p=0;p<6;p++)
	{
		plane=RSViewFrustrum+p;
		i=verlist[iVerBuff];end=v-1;
		iVerBuff=!iVerBuff;
		v=verlist[iVerBuff];
		while(i<end)
		{
			CheckIntersectWorldOnly(*i,*(i+1));
			i++;
			if(v-verlist[iVerBuff]>=NVERTEX_BUFFER) return false;
		}
		CheckIntersectWorldOnly(*i,verlist[!iVerBuff][0]);
		if(v-verlist[iVerBuff]>=NVERTEX_BUFFER) return false;
		if(v==verlist[!iVerBuff]) { *nout=0;return true; }
	}
	*out=verlist[0];*nout=v-verlist[0];
	return true;
}

// BSpline
rvector GetBSpline(rvector *pPath,float f)
{
	int index=(int)f;
	float u=f-index;
	float u2=u*u;
	float u3=u2*u;
	
	static rvector ret;

	ret.x= ( 
		(-1*u3 +  3*u2 + -3*u + 1) * pPath[index].x +
		( 3*u3 + -6*u2 +  0*u + 4) * pPath[index+0].x +
		(-3*u3 +  3*u2 +  3*u + 1) * pPath[index+1].x +
		( 1*u3 +  0*u2 +  0*u + 0) * pPath[index+2].x) / 6.f;
	
	ret.y= ( 
		(-1*u3 +  3*u2 + -3*u + 1) * pPath[index].y +
		( 3*u3 + -6*u2 +  0*u + 4) * pPath[index+0].y +
		(-3*u3 +  3*u2 +  3*u + 1) * pPath[index+1].y +
		( 1*u3 +  0*u2 +  0*u + 0) * pPath[index+2].y) / 6.f;

	ret.z= ( 
		(-1*u3 +  3*u2 + -3*u + 1) * pPath[index].z +
		( 3*u3 + -6*u2 +  0*u + 4) * pPath[index+0].z +
		(-3*u3 +  3*u2 +  3*u + 1) * pPath[index+1].z +
		( 1*u3 +  0*u2 +  0*u + 0) * pPath[index+2].z) / 6.f;

	return ret;
}
// Catmull-Rom spline
rvector GetCRSpline(rvector *pPath,float f)
{
	int index=(int)f;
	float u=f-index;
	float u2=u*u;
	float u3=u2*u;
	
	static rvector ret;

	ret.x= ( 
		(-1*u3 +  2*u2 + -1*u + 0) * pPath[index].x +
		( 3*u3 + -5*u2 +  0*u + 2) * pPath[index+0].x +
		(-3*u3 +  4*u2 +  1*u + 0) * pPath[index+1].x +
		( 1*u3 + -1*u2 +  0*u + 0) * pPath[index+2].x) / 2.f;
	
	ret.y= ( 
		(-1*u3 +  2*u2 + -1*u + 0) * pPath[index].y +
		( 3*u3 + -5*u2 +  0*u + 2) * pPath[index+0].y +
		(-3*u3 +  4*u2 +  1*u + 0) * pPath[index+1].y +
		( 1*u3 + -1*u2 +  0*u + 0) * pPath[index+2].y) / 2.f;

	ret.z= ( 
		(-1*u3 +  2*u2 + -1*u + 0) * pPath[index].z +
		( 3*u3 + -5*u2 +  0*u + 2) * pPath[index+0].z +
		(-3*u3 +  4*u2 +  1*u + 0) * pPath[index+1].z +
		( 1*u3 + -1*u2 +  0*u + 0) * pPath[index+2].z) / 2.f;

	return ret;
}

rvector GetCRSplineDiff(rvector *pPath,float f)
{
	int index=(int)f;
	float u=f-index;
	float u2=u*u;
	float u3=u2*u;
	
	static rvector ret;

	ret.x= ( 
		(-3*u3 + -1*u2 +  4*u +-1) * pPath[index].x +
		( 9*u3 +  3*u2 +-10*u + 0) * pPath[index+0].x +
		(-9*u3 + -3*u2 +  8*u + 1) * pPath[index+1].x +
		( 3*u3 +  1*u2 + -2*u + 0) * pPath[index+2].x) / 2.f;
	
	ret.y= ( 
		(-3*u3 + -1*u2 +  4*u +-1) * pPath[index].y +
		( 9*u3 +  3*u2 +-10*u + 0) * pPath[index+0].y +
		(-9*u3 + -3*u2 +  8*u + 1) * pPath[index+1].y +
		( 3*u3 +  1*u2 + -2*u + 0) * pPath[index+2].y) / 2.f;

	ret.z= ( 
		(-3*u3 + -1*u2 +  4*u +-1) * pPath[index].z +
		( 9*u3 +  3*u2 +-10*u + 0) * pPath[index+0].z +
		(-9*u3 + -3*u2 +  8*u + 1) * pPath[index+1].z +
		( 3*u3 +  1*u2 + -2*u + 0) * pPath[index+2].z) / 2.f;

	return ret;
}