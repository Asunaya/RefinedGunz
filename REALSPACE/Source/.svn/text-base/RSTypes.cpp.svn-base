#include "RSTypes.h"

// in fact, this is inverse-transform plane. ^_^ and m is affine
// and plane's normal must be unit vector
void TransformPlane(rmatrix &m,rplane *tp,const rplane *p)
{

	tp->a=m._11*p->a+m._21*p->b+m._31*p->c;
	tp->b=m._12*p->a+m._22*p->b+m._32*p->c;
	tp->c=m._13*p->a+m._23*p->b+m._33*p->c;
/*
	// following code need optimize.
	static rvector pos;
	pos=TransformVector(-p->d*rvector(p->a,p->b,p->c),m);
	tp->d=-tp->a*pos.x-tp->b*pos.y-tp->c*pos.z;
*/	// optimized ka~ ka~ ka~
	tp->d=tp->a*(tp->a*p->d-m._41)+tp->b*(tp->b*p->d-m._42)+tp->c*(tp->c*p->d-m._43);
}


void TransformVertex(rvertex &v,rmatrix &m)
{
	v.px=m._11*v.x+m._21*v.y+m._31*v.z+m._41;
	v.py=m._12*v.x+m._22*v.y+m._32*v.z+m._42;
	v.pz=m._13*v.x+m._23*v.y+m._33*v.z+m._43;
	v.sx=v.px;
	v.sy=v.py;
	v.sz=v.pz;
}

void TransformVertex(rvertex &v,rvector &s,rmatrix &m) // v: dest s: source
{
	v.px=m._11*s.x+m._21*s.y+m._31*s.z+m._41;
	v.py=m._12*s.x+m._22*s.y+m._32*s.z+m._42;
	v.pz=m._13*s.x+m._23*s.y+m._33*s.z+m._43;
	v.sx=v.px;
	v.sy=v.py;
	v.sz=v.pz;
}

void TransformVertexP(rvertex &v,rmatrix &m)
{
	v.px=m._11*v.x+m._21*v.y+m._31*v.z+m._41;
	v.py=m._12*v.x+m._22*v.y+m._32*v.z+m._42;
	v.pz=m._13*v.x+m._23*v.y+m._33*v.z+m._43;
}

void TransformVertex(rvertex &v,rmatrix44 &m)
{
	v.px=m._11*v.x+m._21*v.y+m._31*v.z+m._41;
	v.py=m._12*v.x+m._22*v.y+m._32*v.z+m._42;
	v.pz=m._13*v.x+m._23*v.y+m._33*v.z+m._43;
	v.pw=1/(m._14*v.x+m._24*v.y+m._34*v.z+m._44);
	v.sx=v.px*v.pw;
	v.sy=v.py*v.pw;
	v.sz=v.pz*v.pw;
}

void TransformVertex(rvertex &v,rvector &s,rmatrix44 &m) // v: dest s: source
{
	v.px=m._11*s.x+m._21*s.y+m._31*s.z+m._41;
	v.py=m._12*s.x+m._22*s.y+m._32*s.z+m._42;
	v.pz=m._13*s.x+m._23*s.y+m._33*s.z+m._43;
	v.pw=1/(m._14*s.x+m._24*s.y+m._34*s.z+m._44);
	v.sx=v.px*v.pw;
	v.sy=v.py*v.pw;
	v.sz=v.pz*v.pw;
}

void TransformVertexP(rvertex &v,rmatrix44 &m)
{
	static float pw;
	v.px=m._11*v.x+m._21*v.y+m._31*v.z+m._41;
	v.py=m._12*v.x+m._22*v.y+m._32*v.z+m._42;
	v.pz=m._13*v.x+m._23*v.y+m._33*v.z+m._43;
	pw=1/(m._14*v.x+m._24*v.y+m._34*v.z+m._44);
	v.px=v.px*pw;
	v.py=v.py*pw;
	v.pz=v.pz*pw;
}

void TransformVertexASM(rvertex&v,rmatrix44&m)
{
	__asm
	{

		mov edx,[ebp+12]		// m : matrix
		mov ecx,[ebp+8]			// v : vertex

		fld1

		fld		dword ptr [ecx]
		fmul	dword ptr [edx+12]
		fld		dword ptr [ecx+4]
		fmul	dword ptr [edx+12+16]
		fld		dword ptr [ecx+8]
		fmul	dword ptr [edx+12+32]
		fadd
		fadd
		fadd	dword ptr [edx+12+48]
		fdiv
		fst		dword ptr [ecx+36] ; save pw

		fld		dword ptr [ecx]
		fmul	dword ptr [edx]
		fld		dword ptr [ecx+4]
		fmul	dword ptr [edx+16]
		fld		dword ptr [ecx+8]
		fmul	dword ptr [edx+32]
		fadd
		fadd
		fadd	dword ptr [edx+48]
		fmul	st,st(1)
		fstp	dword ptr [ecx+24] ; save sx
		
		fld		dword ptr [ecx]
		fmul	dword ptr [edx+4]
		fld		dword ptr [ecx+4]
		fmul	dword ptr [edx+4+16]
		fld		dword ptr [ecx+8]
		fmul	dword ptr [edx+4+32]
		fadd
		fadd
		fadd	dword ptr [edx+4+48]
		fmul	st,st(1)
		fstp	dword ptr [ecx+28] ; save sy

		fld		dword ptr [ecx]
		fmul	dword ptr [edx+8]
		fld		dword ptr [ecx+4]
		fmul	dword ptr [edx+8+16]
		fld		dword ptr [ecx+8]
		fmul	dword ptr [edx+8+32]
		fadd
		fadd
		fadd	dword ptr [edx+8+48]
		fmul	st,st(1)
		fstp	dword ptr [ecx+32] ; save sz

		fstp	st
	}
}

void MergeBoundingBox(rboundingbox *dest,rboundingbox *src)
{
	dest->m[0][0]=min(dest->m[0][0],src->m[0][0]);	
	dest->m[1][0]=min(dest->m[1][0],src->m[1][0]);	
	dest->m[2][0]=min(dest->m[2][0],src->m[2][0]);	
	dest->m[0][1]=max(dest->m[0][1],src->m[0][1]);	
	dest->m[1][1]=max(dest->m[1][1],src->m[1][1]);	
	dest->m[2][1]=max(dest->m[2][1],src->m[2][1]);	
}

float GetDistance(const rvector &position,const rplane &plane)		// 한점에서 평면까지의 거리
{
	return plane.a*position.x+plane.b*position.y+plane.c*position.z+plane.d;
}

float GetDistance(rboundingbox *bb,rplane *plane)		// 평면에서 boundingbox와의 최대거리
{
	float a,b,c;
	a=(plane->a>0)?bb->m[0][1]:bb->m[0][0];
	b=(plane->b>0)?bb->m[1][1]:bb->m[1][0];
	c=(plane->c>0)?bb->m[2][1]:bb->m[2][0];
	return plane->a*a+plane->b*b+plane->c*c+plane->d;
}

// 평면에서 boundingbox와의 최소,최대거리
void GetDistanceMinMax(rboundingbox &bb,rplane &plane,float *MinDist,float *MaxDist)
{
	float a,b,c,a2,b2,c2;
	if(plane.a>0) { a=bb.m[0][1];a2=bb.m[0][0]; } else { a=bb.m[0][0];a2=bb.m[0][1]; }
	if(plane.b>0) { b=bb.m[1][1];b2=bb.m[1][0]; } else { b=bb.m[1][0];b2=bb.m[1][1]; }
	if(plane.c>0) { c=bb.m[2][1];c2=bb.m[2][0]; } else { c=bb.m[2][0];c2=bb.m[2][1]; }
	*MaxDist=plane.a*a+plane.b*b+plane.c*c+plane.d;
	*MinDist=plane.a*a2+plane.b*b2+plane.c*c2+plane.d;
}

bool isInPlane(rboundingbox *bb,rplane *plane)
{
	return (GetDistance(bb,plane)>=0);
}

bool isInPlane(const rvector &position,const rplane &plane)
{
	return GetDistance(position,plane)>=0;
}

bool isInViewFrustrum(const rvector &point,rplane *plane)
{
#define FN(i) ((plane[i].a*point.x+plane[i].b*point.y+plane[i].c*point.z+plane[i].d)>=0)
	return FN(0) && FN(1) && FN(2) && FN(3);
}

bool isInViewFrustrum(const rvector &point,float radius,rplane *plane)		// bounding sphere
{
	if((GetDistance(point,plane[0])>-radius) &&
		(GetDistance(point,plane[1])>-radius) &&
		(GetDistance(point,plane[2])>-radius) &&
		(GetDistance(point,plane[3])>-radius) &&
		(GetDistance(point,plane[5])>-radius))
		return true;
	return false;
}

bool isInViewFrustrum(rboundingbox *bb,rplane *plane)
{
	if(!isInPlane(bb,plane)||!isInPlane(bb,plane+1)||
		!isInPlane(bb,plane+2)||!isInPlane(bb,plane+3)) return false;
	return true;
}

bool isInViewFrustrumWithZ(rboundingbox *bb,rplane *plane)
{
	if(!isInPlane(bb,plane)||!isInPlane(bb,plane+1)||
		!isInPlane(bb,plane+2)||!isInPlane(bb,plane+3)||
		!isInPlane(bb,plane+4)||!isInPlane(bb,plane+5)) return false;
	return true;
}

bool isInViewFrustrumwrtnPlanes(rboundingbox *bb,rplane *plane,int nplane)
{
	for(int i=0;i<nplane;i++)
	{
		if(!isInPlane(bb,plane+i)) return false;
	}
	return true;
}

void SetupPlane(rvertex *v1,rvertex *v2,rvector *dir,rplane *p)
{
	rvector t=CrossProduct(rvector(v1->sx-v2->sx,v1->sy-v2->sy,v1->sz-v2->sz),-*dir);
	p->a=t.x;p->b=t.y;p->c=t.z;
	p->d=-v1->sx*p->a-v1->sy*p->b-v1->sz*p->c;
}

void SetupWorldPlane(rvertex *v1,rvertex *v2,rvector *dir,rplane *p)
{
	rvector t=Normalize(CrossProduct(rvector(v1->x-v2->x,v1->y-v2->y,v1->z-v2->z),-*dir));
	p->a=t.x;p->b=t.y;p->c=t.z;
	p->d=-v1->x*p->a-v1->y*p->b-v1->z*p->c;
}

void SetupPlane(rvector &v1,rvector &v2,rvector &v3,rplane *p)
{
	rvector normal=Normalize(CrossProduct(v1-v2,v1-v3));
	p->a=normal.x;p->b=normal.y;p->c=normal.z;
	p->d=-DotProduct(v1,normal);
}

void SetupPlane(rvector &dir,rvector &apoint,rplane *p)
{
	p->a=dir.x;p->b=dir.y;p->c=dir.z;
	p->d=-DotProduct(apoint,dir);
}

bool isInTheTriangle(rvertex *ver0,rvertex *ver1,rvertex *ver2,int ix,int iy,float *z)
{
	static float a1,b1,c1,a2,b2,c2,a3,b3,c3;
	static rvector v1,v2,v3,va;
	static float x,y;

	x=(float)ix;y=(float)iy;

	v1.x=ver0->sx;v1.y=ver0->sy;
	v2.x=ver1->sx;v2.y=ver1->sy;
	v3.x=ver2->sx;v3.y=ver2->sy;

	va.x=(v1.x+v2.x+v3.x)/3;
	va.y=(v1.y+v2.y+v3.y)/3;
	va.z=ver0->sz+ver1->sz+ver2->sz;

	a1=v1.y-v2.y;b1=v2.x-v1.x;c1=-v1.y*v2.x+v1.x*v2.y;
	a2=v2.y-v3.y;b2=v3.x-v2.x;c2=-v2.y*v3.x+v2.x*v3.y;
	a3=v1.y-v3.y;b3=v3.x-v1.x;c3=-v1.y*v3.x+v1.x*v3.y;

	if(	(signof(a1*x+b1*y+c1)==signof(a1*va.x+b1*va.y+c1))&&
		(signof(a2*x+b2*y+c2)==signof(a2*va.x+b2*va.y+c2))&&
		(signof(a3*x+b3*y+c3)==signof(a3*va.x+b3*va.y+c3))	)
		{
			*z=va.z/3.0f;
			return true;
		}
	return false;
}

bool isInTheFace(rface *f,int ix,int iy,float *z)
{
	return isInTheTriangle(f->v[0],f->v[1],f->v[2],ix,iy,z);
}

bool isLineIntersectBoundingBox(rplueckercoord *line,rboundingbox *bb)
{
	static int faces[6][4]={0,1,2,3, 6,7,3,2, 1,5,6,2, 0,4,5,1, 7,4,5,6, 3,7,4,0 };
	static rvector normals[6]= { rvector(0,0,1),rvector(-1,0,0),rvector(0,1,0),
								rvector(1,0,0),rvector(0,0,-1),rvector(0,-1,0) };
	static rvector bv[8];
	static rplueckercoord plu;

	bv[0]=rvector(bb->Maxx,bb->Miny,bb->Maxz);
	bv[1]=rvector(bb->Maxx,bb->Maxy,bb->Maxz);
	bv[2]=rvector(bb->Minx,bb->Maxy,bb->Maxz);
	bv[3]=rvector(bb->Minx,bb->Miny,bb->Maxz);
	bv[4]=rvector(bb->Maxx,bb->Miny,bb->Minz);
	bv[5]=rvector(bb->Maxx,bb->Maxy,bb->Minz);
	bv[6]=rvector(bb->Minx,bb->Maxy,bb->Minz);
	bv[7]=rvector(bb->Minx,bb->Miny,bb->Minz);

	for(int i=0;i<6;i++)
	{
		if((DotProduct(normals[i],line->u)>0)&&
			(rplueckercoord(bv[faces[i][0]],bv[faces[i][1]])**line>=0)&&
			(rplueckercoord(bv[faces[i][1]],bv[faces[i][2]])**line>=0)&&
			(rplueckercoord(bv[faces[i][2]],bv[faces[i][3]])**line>=0)&&
			(rplueckercoord(bv[faces[i][3]],bv[faces[i][0]])**line>=0))
			return true;
	}
	return false;
}

bool IsIntersect(rplueckercoord *p,rface *f)
{
#define RVECTOR(a) rvector(f->v[a]->x,f->v[a]->y,f->v[a]->z)

	static rplueckercoord p2;
	static rvector v1,v2,v3;
	v1=RVECTOR(0);v2=RVECTOR(1);v3=RVECTOR(2);
	if(rplueckercoord(v1,v2)**p<0) return false;
	if(rplueckercoord(v2,v3)**p<0) return false;
	if(rplueckercoord(v3,v1)**p<0) return false;
	return true;
}

bool IsIntersect(rplueckercoord &p,rvector &v1,rvector &v2,rvector &v3)
{
	if(rplueckercoord(v1,v2)*p<0) return false;
	if(rplueckercoord(v2,v3)*p<0) return false;
	if(rplueckercoord(v3,v1)*p<0) return false;
	return true;
}

float GetDistance(const rvector &position,const rvector &lineorigin,const rvector &linedir)
{
	float maglinedirsqr=linedir.GetMagnitude();
	maglinedirsqr=maglinedirsqr*maglinedirsqr;
	float c=DotProduct(position-lineorigin,linedir)/maglinedirsqr;
	rvector NearestPosition=lineorigin+c*linedir;
	return NearestPosition.GetDistance(position);
}

void ComputeViewFrustrum(rplane *plane,rmatrix44 *matrix,rvector cameraposition, float x,float y,float z)
{
	plane->a=matrix->_11*x+matrix->_12*y+matrix->_13*z;
	plane->b=matrix->_21*x+matrix->_22*y+matrix->_23*z;
	plane->c=matrix->_31*x+matrix->_32*y+matrix->_33*z;
	plane->d=-plane->a*cameraposition.x
				-plane->b*cameraposition.y
				-plane->c*cameraposition.z;
}

void ComputeZPlane(rplane *plane,rvector cameraorientation,rvector cameraposition, float z,int sign)
{
	static rvector normal,t;
	t=cameraposition+z*cameraorientation;
	normal.x=float(sign)*cameraorientation.x;
	normal.y=float(sign)*cameraorientation.y;
	normal.z=float(sign)*cameraorientation.z;
	normal=Normalize(normal);
	plane->a=normal.x;plane->b=normal.y;plane->c=normal.z;
	plane->d=-plane->a*t.x-plane->b*t.y-plane->c*t.z;
}
