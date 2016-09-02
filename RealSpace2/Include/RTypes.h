// types for realspace 2 . 2001-10-4 created.
#pragma once

#include <string>
#include <list>
#include "d3dx9math.h"
#include "GlobalTypes.h"
#include "RNameSpace.h"

using rvector = D3DXVECTOR3;
using rmatrix = D3DXMATRIX;
using rplane = D3DXPLANE;
using rvector2 = D3DXVECTOR2;

_NAMESPACE_REALSPACE2_BEGIN

#define pi D3DX_PI

enum rsign { NEGATIVE= -1, ZERO= 0, POSITIVE= 1 };

#define RPIXELFORMAT D3DFORMAT

enum RRESULT {
	R_UNKNOWN = -1,
	R_OK = 0,
	R_NOTREADY = 1,
	R_RESTORED = 2,

	R_ERROR_LOADING = 1000,
};

enum class FullscreenType
{
	Fullscreen,
	Borderless,
	Windowed,
};

struct RMODEPARAMS {
	int nWidth, nHeight;
	FullscreenType FullscreenMode;
	RPIXELFORMAT PixelFormat;
};

#define RM_FLAG_ADDITIVE		0x0001
#define RM_FLAG_USEOPACITY		0x0002
#define RM_FLAG_TWOSIDED		0x0004
#define RM_FLAG_NOTWALKABLE		0x0008
#define RM_FLAG_CASTSHADOW		0x0010
#define RM_FLAG_RECEIVESHADOW	0x0020
#define RM_FLAG_PASSTHROUGH		0x0040
#define RM_FLAG_HIDE			0x0080
#define RM_FLAG_PASSBULLET		0x0100
#define RM_FLAG_PASSROCKET		0x0200
#define RM_FLAG_USEALPHATEST	0x0400
#define RM_FLAG_AI_NAVIGATION	0x1000

struct rboundingbox
{
	rboundingbox() {}

	union {
	struct {
		float minx,miny,minz,maxx,maxy,maxz;
	};
	struct {
		rvector vmin,vmax;
	};
	float m[2][3];
	};

	rvector Point(int i) const {
		return rvector( (i&1)?vmin.x:vmax.x, (i&2)?vmin.y:vmax.y, (i&4)?vmin.z:vmax.z );
	}
	
	void Add(const rvector &kPoint)
	{
		if ( vmin.x > kPoint.x )	vmin.x = kPoint.x;
		if ( vmin.y > kPoint.y )	vmin.y = kPoint.y;
		if ( vmin.z > kPoint.z )	vmin.z = kPoint.z;
		if ( vmax.x < kPoint.x )	vmax.x = kPoint.x;
		if ( vmax.y < kPoint.y )	vmax.y = kPoint.y;
		if ( vmax.z < kPoint.z )	vmax.z = kPoint.z;
	}

	bool Intersects(const rboundingbox &rhs) const
	{
		if (vmin.x > rhs.vmax.x)
			return false;
		if (vmax.x < rhs.vmin.x)
			return false;
		if (vmin.y > rhs.vmax.y)
			return false;
		if (vmax.y < rhs.vmin.y)
			return false;
		if (vmin.z > rhs.vmax.z)
			return false;
		if (vmax.z < rhs.vmin.z)
			return false;

		return true;
	}

	void Sort()
	{
		rvector tmin, tmax;
		tmin.x = min(vmin.x, vmax.x);
		tmin.y = min(vmin.y, vmax.y);
		tmin.z = min(vmin.z, vmax.z);
		tmax.x = max(vmin.x, vmax.x);
		tmax.y = max(vmin.y, vmax.y);
		tmax.z = max(vmin.z, vmax.z);
		vmin = tmin;
		vmax = tmax;
	}
};

inline float Magnitude(const rvector &x) { return D3DXVec3Length(&x); }
inline float MagnitudeSq(const rvector &x) { return D3DXVec3LengthSq(&x); }
inline void Normalize(rvector &x) { D3DXVec3Normalize(&x, &x); }
inline float DotProduct(const rvector &a, const rvector &b) { return D3DXVec3Dot(&a, &b); }
inline void CrossProduct(rvector *result, const rvector &a, const rvector &b) { D3DXVec3Cross(result, &a, &b); }

void MakeWorldMatrix(rmatrix *pOut, rvector pos, rvector dir, rvector up);

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

#ifndef TOLER
#define TOLER 0.001
#endif
#define IS_ZERO(a) ((fabs((double)(a)) < (double) TOLER))
#define IS_EQ(a,b) ((fabs((double)(a)-(b)) >= (double) TOLER) ? 0 : 1)
#define IS_EQ3(a,b) (IS_EQ((a).x,(b).x)&&IS_EQ((a).y,(b).y)&&IS_EQ((a).z,(b).z))
#define SIGNOF(a) ( (a)<-TOLER ? NEGATIVE : (a)>TOLER ? POSITIVE : ZERO )
#define RANDOMFLOAT ((float)rand()/(float)RAND_MAX)

float GetDistance(const rvector &position,const rvector &line1,const rvector &line2);
rvector GetNearestPoint(const rvector &position,const rvector &a,const rvector &b);
float GetDistanceLineSegment(const rvector &position,const rvector &a,const rvector &b);
float GetDistanceBetweenLineSegment(const rvector &a,const rvector &aa,const rvector &c,
	const rvector &cc,rvector *ap,rvector *cp);
float GetDistance(const rvector &position,const rplane &plane);
rvector GetNearestPoint(const rvector &a,const rvector &aa,const rplane &plane);
float GetDistance(const rvector &a,const rvector &aa,const rplane &plane);
float GetDistance(const rboundingbox *bb, const rplane *plane);
void GetDistanceMinMax(rboundingbox &bb,rplane &plane,float *MinDist,float *MaxDist);
float GetDistance(const rboundingbox &bb,const rvector &point);
float GetArea(rvector &v1,rvector &v2,rvector &v3);

// Returns the clockwise rotation of ta such that tb aligns with ta on xy
float GetAngleOfVectors(const rvector &ta, const rvector &tb);

__declspec(deprecated("Use Slerp instead.")) rvector InterpolatedVector(rvector &a,rvector &b,float x);

rvector Slerp(const rvector &from, const rvector &to, float t);
template <typename T>
T Lerp(T src, T dest, float t)
{
	return src * (1 - t) + dest * t;
}

inline v3 HadamardProduct(const v3& a, const v3& b)
{
	return{ a.x * b.x, a.y * b.y, a.z * b.z };
}

bool IsIntersect(rboundingbox *bb1,rboundingbox *bb2);
bool isInPlane(const rboundingbox *bb, const rplane *plane);
bool IsInSphere(const rboundingbox &bb,const rvector &point,float radius);
bool isInViewFrustum(const rvector &point,rplane *plane);
bool isInViewFrustum(const rvector &point,float radius,rplane *plane);
bool isInViewFrustum(const rboundingbox *bb, const rplane *plane);
bool isInViewFrustum(const rvector &point1,const rvector &point2,rplane *planes);
bool isInViewFrustumWithZ(rboundingbox *bb,rplane *plane);
bool isInViewFrustumwrtnPlanes(rboundingbox *bb,rplane *plane,int nplane);

bool IsIntersect(const rvector& orig, const rvector& dir,
	rvector& v0, rvector& v1, rvector& v2,float* t);
bool isLineIntersectBoundingBox(rvector &origin,rvector &dir,rboundingbox &bb);
bool IsIntersect( rvector& line_begin_, rvector& line_end_, rboundingbox& box_);
bool IsIntersect(rvector& line_begin_, rvector& line_dir_, rvector& center_, float radius_,
	float* dist = nullptr, rvector* p = nullptr );
bool IsIntersect(const rvector& orig, const rvector& dir, const rvector& center,
	const float radius, rvector* p = nullptr);
bool GetIntersectionOfTwoPlanes(rvector *pOutDir, rvector *pOutAPoint, rplane &plane1, rplane &plane2);

void MergeBoundingBox(rboundingbox *dest, rboundingbox *src);
void TransformBox(rboundingbox* result, const rboundingbox& src, const rmatrix& matrix);

inline rvector GetReflectionVector(const rvector& v, const rvector& n)
{
	auto neg = -v;
	float dot = D3DXVec3Dot(&neg, &n);

	return (2 * dot)*n + v;
}

#define FLOAT2RGB24(r, g, b) ( ( ((long)((r) * 255)) << 16) | (((long)((g) * 255)) << 8) | (long)((b) * 255))
#define VECTOR2RGB24(v)		FLOAT2RGB24((v).x,(v).y,(v).z)
#define BYTE2RGB24(r,g,b)	((DWORD) (((BYTE) (b)|((WORD) (g) << 8))|(((DWORD) (BYTE) (r)) << 16)))
#define BYTE2RGB32(a,r,g,b)	((DWORD) (((BYTE) (b)|((WORD) (g) << 8))|(((DWORD) (BYTE) (r)) << 16)|(((DWORD) (BYTE) (a)) << 24)))
#define DWORD2VECTOR(x)		rvector(float(((x)& 0xff0000) >> 16)/255.f, float(((x) & 0xff00) >> 8)/255.f,float(((x) & 0xff))/255.f)

using RFPROGRESSCALLBACK = void(*)(void *pUserParams,float fProgress);

_NAMESPACE_REALSPACE2_END