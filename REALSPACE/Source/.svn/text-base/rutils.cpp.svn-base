#include <math.h>
#include <float.h>
#include "rutils.h"
#include "crtdbg.h"

rmatrix ZeroMatrix(void)
{
    rmatrix ret;
	memset(&ret,0,sizeof(rmatrix));
    return ret;
} // end ZeroMatrix

rmatrix IdentityMatrix()
{
	rmatrix r;
	r=ZeroMatrix();
	r._11=1;r._22=1;r._33=1;
	return r;
}

rmatrix ViewMatrix(const rvector& from, 
		   const rvector& at, 
		   const rvector& world_up, 
		   const float roll)
{
    rmatrix view = IdentityMatrix();
    rvector up, right, view_dir;

    view_dir = Normalize(at - from);
	right = CrossProduct(world_up, view_dir);
	up = CrossProduct(view_dir, right);

	right = Normalize(right);
	up = Normalize(up);
	
    view._11 = right.x;
    view._21 = right.y;
    view._31 = right.z;
    view._12 = up.x;
    view._22 = up.y;
    view._32 = up.z;
    view._13 = view_dir.x;
    view._23 = view_dir.y;
    view._33 = view_dir.z;
	
    view._41 = -DotProduct(right, from);
    view._42 = -DotProduct(up, from);
    view._43 = -DotProduct(view_dir, from);

	// Set roll
	if (roll != 0.0f) {
		view = MatrixMult(RotateZMatrix(-roll), view);
	}

    return view;
} // end ViewMatrix

rmatrix ViewMatrix(const rvector& from, 
		   const rvector& viewdir, 
		   const rvector& world_up)
{
    rmatrix view;
    rvector up, right, dir;

	dir=Normalize(viewdir);
	right = Normalize(CrossProduct(world_up, dir));
	up = Normalize(CrossProduct(dir, right));

    view._11 = right.x;
    view._21 = right.y;
    view._31 = right.z;
    view._12 = up.x;
    view._22 = up.y;
    view._32 = up.z;
    view._13 = dir.x;
    view._23 = dir.y;
    view._33 = dir.z;
	
    view._41 = -DotProduct(right, from);
    view._42 = -DotProduct(up, from);
    view._43 = -DotProduct(dir, from);

    return view;
} // end ViewMatrix

rmatrix ViewMatrix(const rvector& from,const rvector &viewdir)
{
	rvector up=rvector(0,0,1);
	if(IS_EQ3(up,viewdir)) up=rvector(1,0,0);
	return ViewMatrix(from,viewdir,up);
}

rmatrix RotateXMatrix(const float rads)
{
	float	cosine, sine;

	cosine = (float) cos(rads);
	sine = (float) sin(rads);
    rmatrix ret = IdentityMatrix();
    ret._22 = cosine;
	ret._33 = cosine;
	ret._23 = -sine;
	ret._32 = sine;
    return ret;
} // end RotateXMatrix

rmatrix RotateYMatrix(const float rads)
{
	float	cosine, sine;

	cosine = (float) cos(rads);
	sine = (float) sin(rads);
    rmatrix ret = IdentityMatrix();
    ret._11 = cosine;
	ret._33 = cosine;
	ret._13 = sine;
	ret._31 = -sine;
    return ret;
} // end RotateY

rmatrix RotateZMatrix(const float rads)
{
	float	cosine, sine;

	cosine = (float) cos(rads);
	sine = (float) sin(rads);
    rmatrix ret = IdentityMatrix();
    ret._11 = cosine;
	ret._22 = cosine;
	ret._12 = -sine;
	ret._21 = sine;
    return ret;
} // end RotateZMatrix

rmatrix TranslateMatrix(const float dx, const float dy, const float dz)
{
    rmatrix ret = IdentityMatrix();
	ret._41 = dx;
	ret._42 = dy;
	ret._43 = dz;
	return ret;
} // end TranslateMatrix

rmatrix TranslateMatrix(const rvector &v)
{
	return TranslateMatrix(v.x,v.y,v.z);
}

rmatrix ScaleMatrix(const float size)
{
    rmatrix ret = IdentityMatrix();
	ret._11 = size;
	ret._22 = size;
	ret._33 = size;
	return ret;
} // end ScaleMatrix

rmatrix ScaleMatrixXYZ(const float x,const float y,const float z)
{
    rmatrix ret = IdentityMatrix();
	ret._11 = x;
	ret._22 = y;
	ret._33 = z;
	return ret;
} // end ScaleMatrix

rvector TransformVector(rvector &v,rmatrix &m)
{
	static float sx,sy,sz;
	rvector r;

	sx=m._11*v.x+m._21*v.y+m._31*v.z+m._41;
	sy=m._12*v.x+m._22*v.y+m._32*v.z+m._42;
	sz=m._13*v.x+m._23*v.y+m._33*v.z+m._43;
	r.x=sx;
	r.y=sy;
	r.z=sz;
	return r;
}

rvector TransformNormal(rvector &v,rmatrix &m)
{
	rvector t;
	t.x=m._11*v.x+m._21*v.y+m._31*v.z;
	t.y=m._12*v.x+m._22*v.y+m._32*v.z;
	t.z=m._13*v.x+m._23*v.y+m._33*v.z;
	return t;
}

// 4*3 inverse matrix

rmatrix MatrixInverse(const rmatrix & m)
{
	rmatrix inverse;
	float det;
	det=m._11*(m._22*m._33-m._23*m._32)
		-m._12*(m._21*m._33-m._23*m._31)+
		m._13*(m._21*m._32-m._22*m._31);
	det=1.0f/det;

	inverse._11=det*(m._22*m._33-m._23*m._32);
	inverse._12=det*(m._13*m._32-m._12*m._33);
	inverse._13=det*(m._12*m._23-m._13*m._22);

	inverse._21=det*(m._23*m._31-m._21*m._33);
	inverse._22=det*(m._11*m._33-m._13*m._31);
	inverse._23=det*(m._13*m._21-m._11*m._23);

	inverse._31=det*(m._21*m._32-m._22*m._31);
	inverse._32=det*(m._12*m._31-m._11*m._32);
	inverse._33=det*(m._11*m._22-m._12*m._21);

	inverse._41=-m._41*inverse._11-m._42*inverse._21-m._43*inverse._31;
	inverse._42=-m._41*inverse._12-m._42*inverse._22-m._43*inverse._32;
	inverse._43=-m._41*inverse._13-m._42*inverse._23-m._43*inverse._33;

	return inverse;
}

// Aid Functions....

float GetAngle(rvector &a)
{
	if(a.x>=1.0f) return 0.0f;
	if(a.x<=-1.0f) return -pi;
	if(a.y>0)
		return (float)acos(a.x);
	else
		return (float)-acos(a.x);
}

float GetAngleOfVectors(rvector &ta,rvector &tb)
{
	if((ta.x==0.0f)&&(ta.y==0.0f)&&(ta.z==0.0f))return 0;
	if((tb.x==0.0f)&&(tb.y==0.0f)&&(tb.z==0.0f))return 0;
	rvector a=ta,b=tb;
	a.z=0;a=Normalize(a);b.z=0;b=Normalize(b);
	float aa=GetAngle(a);
	float x,y;
	x=(float)(b.x*cos(aa)+b.y*sin(aa));
	y=(float)(b.x*(-sin(aa))+b.y*cos(aa));

	float ret=GetAngle(rvector(x,y,0));
	return ret;
}

rvector InterpolatedVector(rvector &a,rvector &b,float x)
{
/*	float t;
	if(t1<0.5f) t=float(0.5*sin(t1*pi));
	else t=float(0.5*(2.0-sin(t1*pi)));
	return rvector(a.x*t+b.x*(1-t),a.y*t+b.y*(1-t),a.z*t+b.z*(1-t));*/
	float ab,theta,theta1,theta2,costheta1,costheta2,u,v;
	
	ab=(float)min(max(DotProduct(a,b),-1.),1.);
	if(ab==1.0f) return b;	// 각도가 0이면 그냥 리턴
	else if(ab==-1.0f) return TransformVector(a, RotateZMatrix(pi*x));		// 각도가 180이면 Z축으로 회전한다.

	theta=(float)acos(ab);
	//if(theta==0.0f) return a;	// 0도이면 a 리턴

	theta1=theta*x;
	theta2=theta*(1.0f-x);
	costheta1=(float)cos(theta1);
	costheta2=(float)cos(theta2);
	u=costheta1-ab*costheta2;
	v=costheta2-ab*costheta1;
	float D = (1.0f-ab*ab);
	_ASSERT(D!=0.0f);	// 앞에서 ab를 검사하므로 0이 나올 수 없다.
	if(D==0) return a;

	rvector vReturn=(1.0f/D*(u*a+v*b));
	_ASSERT(!_isnan(vReturn.z));
	return vReturn;
}

rmatrix ReflectionMatrix(rvector &dir,rvector &apoint)
{
	rvector up,right;
	up=dir+rvector(dir.x,dir.z,dir.y);
	right=Normalize(CrossProduct(dir,up));
	up=Normalize(CrossProduct(right,dir));
	rmatrix t=rmatrix(-dir.x,up.x,right.x,
						-dir.y,up.y,right.y,
						-dir.z,up.z,right.z,0,0,0),
			t2=rmatrix(dir.x,dir.y,dir.z,
						up.x,up.y,up.z,
						right.x,right.y,right.z,0,0,0);
	return MatrixMult(MatrixMult(MatrixMult(
			TranslateMatrix(-apoint),t),t2),TranslateMatrix(apoint));
}

rmatrix ShadowProjectionMatrix(rvector &normal,rvector &apoint,rvector &lightdir)
{
	rvector up,right;
	up=normal+rvector(normal.x,normal.z,normal.y);
	right=Normalize(CrossProduct(normal,up));
	up=Normalize(CrossProduct(right,normal));
	rmatrix t=rmatrix(lightdir.x,lightdir.y,lightdir.z,
						right.x,right.y,right.z,
						up.x,up.y,up.z,0,0,0),
			t2=IdentityMatrix();
	t2._11=0;
	rmatrix t3=MatrixMult(MatrixMult(MatrixMult(MatrixMult(
		TranslateMatrix(-apoint),MatrixInverse(t)),t2),t),TranslateMatrix(apoint));
	return t3;
}

/*
int _matherr( struct _exception *except )
{
	_ASSERT(0);
	switch( except->type ){
	case _DOMAIN :
		except->retval = 0.001;
//		LOG( "_matherr, _DOMAIN : %s\n", except->name );
		OutputDebugString("Math Error _DOMAIN Occured!\n");
		return 0;
	case _SING :
		except->retval = 0.001;		
//		LOG( "_matherr, _SING: %s, except->name\n" );
		OutputDebugString("Math Error _SING Occured!\n");
		return 0;
	case _OVERFLOW :
		except->retval = 0.001;		
//		LOG( "_matherr, _OVERFLOW: %s, except->name\n" );
		OutputDebugString("Math Error _OVERFLOW Occured!\n");
		return 0;
	case _PLOSS :
		except->retval = 0.001;		
//		LOG( "_matherr, _PLOSS: %s, except->name\n" );
		return 0;
	case _TLOSS :
		except->retval = 0.001;		
//		LOG( "_matherr, TLOSS: %s, except->name\n" );
		return 0;
	case _UNDERFLOW :
		except->retval = 0.001;		
//		LOG( "_matherr, _UNDERFLOW: %s, except->name\n" );
		return 0;
	default :
		return 0;
		break;
	}
	return 1;
}
*/

rmatrix MatrixMult(rmatrix &v1,rmatrix &v2)
{
	rmatrix r;

	r._11=v1._11*v2._11+v1._12*v2._21+v1._13*v2._31;
	r._12=v1._11*v2._12+v1._12*v2._22+v1._13*v2._32;
	r._13=v1._11*v2._13+v1._12*v2._23+v1._13*v2._33;

	r._21=v1._21*v2._11+v1._22*v2._21+v1._23*v2._31;
	r._22=v1._21*v2._12+v1._22*v2._22+v1._23*v2._32;
	r._23=v1._21*v2._13+v1._22*v2._23+v1._23*v2._33;

	r._31=v1._31*v2._11+v1._32*v2._21+v1._33*v2._31;
	r._32=v1._31*v2._12+v1._32*v2._22+v1._33*v2._32;
	r._33=v1._31*v2._13+v1._32*v2._23+v1._33*v2._33;

	r._41=v1._41*v2._11+v1._42*v2._21+v1._43*v2._31+v2._41;
	r._42=v1._41*v2._12+v1._42*v2._22+v1._43*v2._32+v2._42;
	r._43=v1._41*v2._13+v1._42*v2._23+v1._43*v2._33+v2._43;

	return r;
}

rmatrix44 MatrixMult(rmatrix &v1,rmatrix44 &v2)
{
	rmatrix44 r;

	r._11=v1._11*v2._11+v1._12*v2._21+v1._13*v2._31;
	r._12=v1._11*v2._12+v1._12*v2._22+v1._13*v2._32;
	r._13=v1._11*v2._13+v1._12*v2._23+v1._13*v2._33;
	r._14=v1._11*v2._14+v1._12*v2._24+v1._13*v2._34;

	r._21=v1._21*v2._11+v1._22*v2._21+v1._23*v2._31;
	r._22=v1._21*v2._12+v1._22*v2._22+v1._23*v2._32;
	r._23=v1._21*v2._13+v1._22*v2._23+v1._23*v2._33;
	r._24=v1._21*v2._14+v1._22*v2._24+v1._23*v2._34;

	r._31=v1._31*v2._11+v1._32*v2._21+v1._33*v2._31;
	r._32=v1._31*v2._12+v1._32*v2._22+v1._33*v2._32;
	r._33=v1._31*v2._13+v1._32*v2._23+v1._33*v2._33;
	r._34=v1._31*v2._14+v1._32*v2._24+v1._33*v2._34;

	r._41=v1._41*v2._11+v1._42*v2._21+v1._43*v2._31+v2._41;
	r._42=v1._41*v2._12+v1._42*v2._22+v1._43*v2._32+v2._42;
	r._43=v1._41*v2._13+v1._42*v2._23+v1._43*v2._33+v2._43;
	r._44=v1._41*v2._14+v1._42*v2._24+v1._43*v2._34+v2._44;

	return r;
}

rmatrix44 MatrixMult(rmatrix44 &v1,rmatrix &v2)
{
	rmatrix44 r;

	r._11=v1._11*v2._11+v1._12*v2._21+v1._13*v2._31+v1._14*v2._41;
	r._12=v1._11*v2._12+v1._12*v2._22+v1._13*v2._32+v1._14*v2._42;
	r._13=v1._11*v2._13+v1._12*v2._23+v1._13*v2._33+v1._14*v2._43;
	r._14=v1._14;

	r._21=v1._21*v2._11+v1._22*v2._21+v1._23*v2._31+v1._24*v2._41;
	r._22=v1._21*v2._12+v1._22*v2._22+v1._23*v2._32+v1._24*v2._42;
	r._23=v1._21*v2._13+v1._22*v2._23+v1._23*v2._33+v1._24*v2._43;
	r._24=v1._24;

	r._31=v1._31*v2._11+v1._32*v2._21+v1._33*v2._31+v1._34*v2._41;
	r._32=v1._31*v2._12+v1._32*v2._22+v1._33*v2._32+v1._34*v2._42;
	r._33=v1._31*v2._13+v1._32*v2._23+v1._33*v2._33+v1._34*v2._43;
	r._34=v1._34;

	r._41=v1._41*v2._11+v1._42*v2._21+v1._43*v2._31+v1._44*v2._41;
	r._42=v1._41*v2._12+v1._42*v2._22+v1._43*v2._32+v1._44*v2._42;
	r._43=v1._41*v2._13+v1._42*v2._23+v1._43*v2._33+v1._44*v2._43;
	r._44=v1._44;

	return r;
}

//////////////////////////////  44 matrix

rmatrix44 ZeroMatrix44(void)
{
    rmatrix44 ret;
	memset(&ret,0,sizeof(rmatrix44));
    return ret;
} // end ZeroMatrix

rmatrix44 IdentityMatrix44()
{
	rmatrix44 r;
	r=ZeroMatrix44();
	r._11=1;r._22=1;r._33=1;r._44=1;
	return r;
}

rmatrix44 ProjectionMatrix(const float near_plane, 
				 const float far_plane, 
				 const float fov_horiz,
				 const float fov_vert,
				 bool bFlipHoriz,bool bFlipVert
				 )
{
    rmatrix44 ret;

//	memset(&ret,0,sizeof(rmatrix));
	ret = ZeroMatrix44();

	float	w, h, Q;

    w = (float)(1/tan(fov_horiz*0.5));  // 1/tan(x) == cot(x)
    h = (float)(1/tan(fov_vert*0.5));   // 1/tan(x) == cot(x)
	Q = far_plane/(far_plane - near_plane);

    ret._11 = bFlipHoriz ? -w : w;
    ret._22 = bFlipVert ? -h : h;
	ret._33 = Q;
	ret._43 = -Q*near_plane;
    ret._34 = 1;
    return ret;
}	// end ProjectionMatrix

rmatrix44 ViewMatrix44(const rvector& from, 
		   const rvector& at, 
		   const rvector& world_up, 
		   const float roll)
{
    rmatrix44 view = IdentityMatrix44();
    rvector up, right, view_dir;

    view_dir = Normalize(at - from);
	right = CrossProduct(world_up, view_dir);
	up = CrossProduct(view_dir, right);

	right = Normalize(right);
	up = Normalize(up);
	
    view._11 = right.x;
    view._21 = right.y;
    view._31 = right.z;
    view._12 = up.x;
    view._22 = up.y;
    view._32 = up.z;
    view._13 = view_dir.x;
    view._23 = view_dir.y;
    view._33 = view_dir.z;
	
    view._41 = -DotProduct(right, from);
    view._42 = -DotProduct(up, from);
    view._43 = -DotProduct(view_dir, from);

	// Set roll
	if (roll != 0.0f) {
		view = RotateZMatrix44(-roll) * view;
	}

    return view;
} // end ViewMatrix

rmatrix44 ViewMatrix44(const rvector& from, 
		   const rvector& viewdir, 
		   const rvector& world_up)
{
    rmatrix44 view;
    rvector up, right, dir;

	dir=Normalize(viewdir);
	right = Normalize(CrossProduct(world_up, dir));
	up = Normalize(CrossProduct(dir, right));

    view._11 = right.x;
    view._21 = right.y;
    view._31 = right.z;		view._14=0;
    view._12 = up.x;
    view._22 = up.y;
    view._32 = up.z;		view._24=0;
    view._13 = dir.x;
    view._23 = dir.y;
    view._33 = dir.z;		view._34=0;
	
    view._41 = -DotProduct(right, from);
    view._42 = -DotProduct(up, from);
    view._43 = -DotProduct(dir, from);		view._44=1;

    return view;
} // end ViewMatrix

rmatrix44 ViewMatrix44(const rvector& from,const rvector &viewdir)
{
	rvector up=rvector(0,0,1);
	if(IS_EQ3(up,viewdir)) up=rvector(1,0,0);
	return ViewMatrix44(from,viewdir,up);
}

rmatrix44 RotateXMatrix44(const float rads)
{
	float	cosine, sine;

	cosine = (float) cos(rads);
	sine = (float) sin(rads);
    rmatrix44 ret = IdentityMatrix44();
    ret._22 = cosine;
	ret._33 = cosine;
	ret._23 = -sine;
	ret._32 = sine;
    return ret;
} // end RotateXMatrix

rmatrix44 RotateYMatrix44(const float rads)
{
	float	cosine, sine;

	cosine = (float) cos(rads);
	sine = (float) sin(rads);
    rmatrix44 ret = IdentityMatrix44();
    ret._11 = cosine;
	ret._33 = cosine;
	ret._13 = sine;
	ret._31 = -sine;
    return ret;
} // end RotateY

rmatrix44 RotateZMatrix44(const float rads)
{
	float	cosine, sine;

	cosine = (float) cos(rads);
	sine = (float) sin(rads);
    rmatrix44 ret = IdentityMatrix44();
    ret._11 = cosine;
	ret._22 = cosine;
	ret._12 = -sine;
	ret._21 = sine;
    return ret;
} // end RotateZMatrix

rmatrix44 TranslateMatrix44(const float dx, const float dy, const float dz)
{
    rmatrix44 ret = IdentityMatrix44();
	ret._41 = dx;
	ret._42 = dy;
	ret._43 = dz;
	return ret;
} // end TranslateMatrix

rmatrix44 TranslateMatrix44(const rvector &v)
{
	return TranslateMatrix44(v.x,v.y,v.z);
}

rmatrix44 ScaleMatrix44(const float size)
{
    rmatrix44 ret = IdentityMatrix44();
	ret._11 = size;
	ret._22 = size;
	ret._33 = size;
	return ret;
} // end ScaleMatrix

rmatrix44 ScaleMatrixXYZ44(const float x,const float y,const float z)
{
    rmatrix44 ret = IdentityMatrix44();
	ret._11 = x;
	ret._22 = y;
	ret._33 = z;
	return ret;
} // end ScaleMatrix

rvector TransformVector(rvector &v,rmatrix44 &m)
{
	static float sx,sy,sz,sw;
	rvector r;

	sx=m._11*v.x+m._21*v.y+m._31*v.z+m._41;
	sy=m._12*v.x+m._22*v.y+m._32*v.z+m._42;
	sz=m._13*v.x+m._23*v.y+m._33*v.z+m._43;
	sw=m._14*v.x+m._24*v.y+m._34*v.z+m._44;
	r.x=sx/sw;
	r.y=sy/sw;
	r.z=sz/sw;
	return r;
}

rvector TransformNormal(rvector &v,rmatrix44 &m)
{
	rvector t;
	t.x=m._11*v.x+m._21*v.y+m._31*v.z;
	t.y=m._12*v.x+m._22*v.y+m._32*v.z;
	t.z=m._13*v.x+m._23*v.y+m._33*v.z;
	return t;
}

/*
**-----------------------------------------------------------------------------
**  Name:       MatrixInverse
**  Purpose:	Creates the inverse of a 4x4 matrix
**-----------------------------------------------------------------------------
*/

static void	lubksb(rmatrix44 & a, int *indx, float *b);
static void ludcmp(rmatrix44 & a, int *indx, float *d);

rmatrix44 MatrixInverse(const rmatrix44 & m)
{
	rmatrix44	n, y;
	int			i, j, indx[4];
	float		d, col[4];

	n = m;
	ludcmp(n, indx, &d);

	for (j=0; j<4; j++) {
		for (i=0; i<4; i++) {
			col[i] = 0.0f;
		}
		col[j] = 1.0f;
		lubksb(n, indx, col);
		for (i=0; i<4; i++) {
			y(i, j) = col[i];
		}
	}
	return y;
} // end MatrixInverse

/*
**-----------------------------------------------------------------------------
**  Name:       lubksb
**  Purpose:	backward subsitution
**-----------------------------------------------------------------------------
*/

static void 
lubksb(rmatrix44 & a, int *indx, float *b)
{
	int		i, j, ii=-1, ip;
	float	sum;

	for (i=0; i<4; i++) {
		ip = indx[i];
		sum = b[ip];
		b[ip] = b[i];
		if (ii>=0) {
			for (j=ii; j<=i-1; j++) {
				sum -= a(i, j) * b[j];
			}
		} else if (sum != 0.0) {
			ii = i;
		}
		b[i] = sum;
	}
	for (i=3; i>=0; i--) {
		sum = b[i];
		for (j=i+1; j<4; j++) {
			sum -= a(i, j) * b[j];
		}
		b[i] = sum/a(i, i);
	}
} // end lubksb

/*
**-----------------------------------------------------------------------------
**  Name:       ludcmp
**  Purpose:	LU decomposition
**-----------------------------------------------------------------------------
*/

static void 
ludcmp(rmatrix44 & a, int *indx, float *d)
{
	float	vv[4];               /* implicit scale for each row */
	float	big, dum, sum, tmp;
	int		i, imax, j, k;

	*d = 1.0f;
	for (i=0; i<4; i++) {
		big = 0.0f;
		for (j=0; j<4; j++) {
			if ((tmp = (float) fabs(a(i, j))) > big) {
				big = tmp;
			}
		}
		/*
		if (big == 0.0f) {
			printf("ludcmp(): singular matrix found...\n");
			exit(1);
		}
		*/
		vv[i] = 1.0f/big;
	}
	for (j=0; j<4; j++) {
		for (i=0; i<j; i++) {
			sum = a(i, j);
			for (k=0; k<i; k++) {
				sum -= a(i, k) * a(k, j);
			}
			a(i, j) = sum;
		}
		big = 0.0f;
		for (i=j; i<4; i++) {
			sum = a(i, j);
			for (k=0; k<j; k++) {
				sum -= a(i, k)*a(k, j);
			}
			a(i, j) = sum;
			if ((dum = vv[i] * (float)fabs(sum)) >= big) {
				big = dum;
				imax = i;
			}
		}
		if (j != imax) {
			for (k=0; k<4; k++) {
				dum = a(imax, k);
				a(imax, k) = a(j, k);
				a(j, k) = dum;
			}
			*d = -(*d);
			vv[imax] = vv[j];
		}
		indx[j] = imax;
		if (a(j, j) == 0.0f) {
			a(j, j) = 1.0e-20f;      /* can be 0.0 also... */
		}
		if (j != 3) {
			dum = 1.0f/a(j, j);
			for (i=j+1; i<4; i++) {
				a(i, j) *= dum;
			}
		}
	}
} // end ludcmp

rmatrix44 MatrixMult44_NonSSE(rmatrix44 &v1,rmatrix44 &v2)
{
	rmatrix44 r;

	r._11=v1._11*v2._11+v1._12*v2._21+v1._13*v2._31+v1._14*v2._41;
	r._12=v1._11*v2._12+v1._12*v2._22+v1._13*v2._32+v1._14*v2._42;
	r._13=v1._11*v2._13+v1._12*v2._23+v1._13*v2._33+v1._14*v2._43;
	r._14=v1._11*v2._14+v1._12*v2._24+v1._13*v2._34+v1._14*v2._44;

	r._21=v1._21*v2._11+v1._22*v2._21+v1._23*v2._31+v1._24*v2._41;
	r._22=v1._21*v2._12+v1._22*v2._22+v1._23*v2._32+v1._24*v2._42;
	r._23=v1._21*v2._13+v1._22*v2._23+v1._23*v2._33+v1._24*v2._43;
	r._24=v1._21*v2._14+v1._22*v2._24+v1._23*v2._34+v1._24*v2._44;

	r._31=v1._31*v2._11+v1._32*v2._21+v1._33*v2._31+v1._34*v2._41;
	r._32=v1._31*v2._12+v1._32*v2._22+v1._33*v2._32+v1._34*v2._42;
	r._33=v1._31*v2._13+v1._32*v2._23+v1._33*v2._33+v1._34*v2._43;
	r._34=v1._31*v2._14+v1._32*v2._24+v1._33*v2._34+v1._34*v2._44;

	r._41=v1._41*v2._11+v1._42*v2._21+v1._43*v2._31+v1._44*v2._41;
	r._42=v1._41*v2._12+v1._42*v2._22+v1._43*v2._32+v1._44*v2._42;
	r._43=v1._41*v2._13+v1._42*v2._23+v1._43*v2._33+v1._44*v2._43;
	r._44=v1._41*v2._14+v1._42*v2._24+v1._43*v2._34+v1._44*v2._44;

	return r;
}

#ifdef __cplusplus
extern "C" {
#endif

void __cdecl  PIII_Mult00_4x4_4x4(float *src1, float *src2, float *dst);


#ifdef __cplusplus
}
#endif

rmatrix44 MatrixMult_SSE(rmatrix44 &v1,rmatrix44 &v2)
{
	rmatrix44 r;
	PIII_Mult00_4x4_4x4((float*)&v1,(float*)&v2,(float*)&r);
	return r;
}

_MatrixMult44 *MatrixMult44=MatrixMult44_NonSSE;
bool g_bSSE=false;

void RUtils_Initilize()
{
    SYSTEM_INFO si;
    int nCPUFeatures=0;

    GetSystemInfo(&si);

    if (si.dwProcessorType != PROCESSOR_INTEL_386 && si.dwProcessorType != PROCESSOR_INTEL_486){
        try {
            __asm
            {
                push eax
                push ebx
                push ecx
                push edx

                mov eax,1
				cpuid
                mov nCPUFeatures,edx

                pop edx
                pop ecx
                pop ebx
                pop eax
            }
        }catch(...){
			g_bSSE=false;
        }
    }

    g_bSSE = ((nCPUFeatures&0x02000000)!=0);
	MatrixMult44=g_bSSE ? MatrixMult_SSE : MatrixMult44_NonSSE;
}