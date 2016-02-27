// types for realspace 2 . 2001-10-4 created.

#ifndef __RTYPES_H
#define __RTYPES_H


#include <string>
#include <list>
#include "d3dx9math.h"

using namespace std;

#include "RNameSpace.h"
_NAMESPACE_REALSPACE2_BEGIN

#define pi D3DX_PI

enum rsign { NEGATIVE= -1, ZERO= 0, POSITIVE= 1 };

// Enum형 type들
/*
enum RPIXELFORMAT {
	RPIXELFORMAT_565 = 1,
	RPIXELFORMAT_555 = 2,
	RPIXELFORMAT_X888 = 3
};
*/

// 별필요없는듯해서 그냥 d3dformat 을 쓴다
// 쓰는 포맷은 D3DFMT_X8R8G8B8 , D3DFMT_R5G6B5 , D3DFMT_X1R5G5B5  이정도..

#define RPIXELFORMAT D3DFORMAT

enum RRESULT {
	R_UNKNOWN = -1,
	R_OK = 0,
	R_NOTREADY = 1,
	R_RESTORED = 2,

	R_ERROR_LOADING = 1000,
	
	
};

// 초기화 파라미터
struct RMODEPARAMS {
	int nWidth,nHeight;
	bool bFullScreen;
	RPIXELFORMAT PixelFormat;
};

#define RM_FLAG_ADDITIVE		0x0001
#define RM_FLAG_USEOPACITY		0x0002
#define RM_FLAG_TWOSIDED		0x0004
#define RM_FLAG_NOTWALKABLE		0x0008		// 갈수없는지역을 미리 표시.
#define RM_FLAG_CASTSHADOW		0x0010
#define RM_FLAG_RECEIVESHADOW	0x0020
#define RM_FLAG_PASSTHROUGH		0x0040		// 이 플래그가 있으면 충돌체크를 하지 않는다.
#define RM_FLAG_HIDE			0x0080		// 그리지않는다.
#define RM_FLAG_PASSBULLET		0x0100		// 총알이 통과한다
#define RM_FLAG_PASSROCKET		0x0200		// 로켓/수류탄등이 통과한다.
#define RM_FLAG_USEALPHATEST	0x0400		// alpha test 맵 (나뭇잎 등등 .. )
#define RM_FLAG_AI_NAVIGATION	0x1000		// 퀘스트 AI 네비게이션 용


#define rvector D3DXVECTOR3
#define rmatrix D3DXMATRIX
#define rplane D3DXPLANE

#define rvector2 D3DXVECTOR2

struct rboundingbox
{
	union {
	struct {
		float minx,miny,minz,maxx,maxy,maxz;
	};
	struct {
		rvector vmin,vmax;
	};
	float m[2][3];
	};

	rvector Point(int i) const { return rvector( (i&1)?vmin.x:vmax.x, (i&2)?vmin.y:vmax.y, (i&4)?vmin.z:vmax.z );  }
	
	void Add(const rvector &kPoint)
	{
		if ( vmin.x > kPoint.x )	vmin.x = kPoint.x;
		if ( vmin.y > kPoint.y )	vmin.y = kPoint.y;
		if ( vmin.z > kPoint.z )	vmin.z = kPoint.z;
		if ( vmax.x < kPoint.x )	vmax.x = kPoint.x;
		if ( vmax.y < kPoint.y )	vmax.y = kPoint.y;
		if ( vmax.z < kPoint.z )	vmax.z = kPoint.z;
	}
};

/*
struct rplueckercoord {
public:
	rplueckercoord() {}
	rplueckercoord(const rvector &origin,const rvector &target) 
	{	u=origin-target;CrossProduct(&v,origin,target); }
	rvector u,v;
	inline friend float operator * (const rplueckercoord& p1, const rplueckercoord& p2) 
	{ return DotProduct(p1.u,p2.v)+DotProduct(p1.v,p2.u); }
};
*/

// 벡터

inline float Magnitude(const rvector &x) { return D3DXVec3Length(&x); }
inline float MagnitudeSq(const rvector &x)	{ return D3DXVec3LengthSq(&x); }
inline void Normalize(rvector &x) { D3DXVec3Normalize(&x,&x);}
inline float DotProduct(const rvector &a,const rvector &b) { return D3DXVec3Dot(&a,&b); }
inline void CrossProduct(rvector *result,const rvector &a,const rvector &b) { D3DXVec3Cross(result,&a,&b); }

//void SetPlane(rplane& plane, rvector& point1, rvector& point2, rvector& point3);

// 행렬

void MakeWorldMatrix(rmatrix *pOut,rvector pos,rvector dir,rvector up);			// el 모델의 world matrix 를 만든다.

// 평면 

// FLOAT D3DXPlaneDotCoord( CONST D3DXPLANE *pP, CONST D3DXVECTOR3 *pV);		// ax + by + cz + d  
// FLOAT D3DXPlaneDotNormal( CONST D3DXPLANE *pP, CONST D3DXVECTOR3 *pV);		// ax + by + cz
// D3DXPLANE* D3DXPlaneFromPoints ( D3DXPLANE *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2,CONST D3DXVECTOR3 *pV3);	// Construct a plane from 3 points


// RELEASE & DELETE 매크로 ( from dxutil.h )
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

// help 펑션들..
#ifndef TOLER
#define TOLER 0.001
#endif
#define IS_ZERO(a) ((fabs((double)(a)) < (double) TOLER))
#define IS_EQ(a,b) ((fabs((double)(a)-(b)) >= (double) TOLER) ? 0 : 1)
#define IS_EQ3(a,b) (IS_EQ((a).x,(b).x)&&IS_EQ((a).y,(b).y)&&IS_EQ((a).z,(b).z))
#define SIGNOF(a) ( (a)<-TOLER ? NEGATIVE : (a)>TOLER ? POSITIVE : ZERO )
#define RANDOMFLOAT ((float)rand()/(float)RAND_MAX)

// 한점에서 직선까지의 거리.. line1,line2 는 직선위의 두 점.
float GetDistance(const rvector &position,const rvector &line1,const rvector &line2);
// 한점에서 가장 가까운 선분위의 점
rvector GetNearestPoint(const rvector &position,const rvector &a,const rvector &b);
// 한점에서 선분까지의 거리
float GetDistanceLineSegment(const rvector &position,const rvector &a,const rvector &b);
// 선분과 선분 사이의 거리.. 선분 (a,aa) 과 선분 (c,cc)의 거리.
float GetDistanceBetweenLineSegment(const rvector &a,const rvector &aa,const rvector &c,const rvector &cc,rvector *ap,rvector *cp);
// 한점에서 평면까지의 거리
float GetDistance(const rvector &position,const rplane &plane);
// 선분(a,aa) 에서 평면까지의 가장 가까운 선분위의 점.
rvector GetNearestPoint(const rvector &a,const rvector &aa,const rplane &plane);
// 선분(a,aa) 에서 평면까지의 거리
float GetDistance(const rvector &a,const rvector &aa,const rplane &plane);
// 평면에서 boundingbox와의 최대거리
float GetDistance(rboundingbox *bb,rplane *plane);
// 평면에서 boundingbox와의 최소,최대거리
void GetDistanceMinMax(rboundingbox &bb,rplane &plane,float *MinDist,float *MaxDist);
// 한점과 boundingbox의 최소거리
float GetDistance(const rboundingbox &bb,const rvector &point);
// 삼각형의 면적
float GetArea(rvector &v1,rvector &v2,rvector &v3);

// 두 벡터의 x, y상에서의 각도
float GetAngleOfVectors(rvector &ta,rvector &tb);

// 원형보간된 vector.. a,b는 normalized 되어있어야함.
rvector InterpolatedVector(rvector &a,rvector &b,float x);

bool IsIntersect(rboundingbox *bb1,rboundingbox *bb2);
bool isInPlane(rboundingbox *bb,rplane *plane);
bool IsInSphere(const rboundingbox &bb,const rvector &point,float radius);
bool isInViewFrustum(const rvector &point,rplane *plane);
bool isInViewFrustum(const rvector &point,float radius,rplane *plane);		// bounding sphere
bool isInViewFrustum(rboundingbox *bb,rplane *plane);
bool isInViewFrustum(const rvector &point1,const rvector &point2,rplane *planes);	// 선분
bool isInViewFrustumWithZ(rboundingbox *bb,rplane *plane);
bool isInViewFrustumwrtnPlanes(rboundingbox *bb,rplane *plane,int nplane);

bool IsIntersect( const rvector& orig, const rvector& dir, rvector& v0, rvector& v1, rvector& v2, float* t);
bool isLineIntersectBoundingBox(rvector &origin,rvector &dir,rboundingbox &bb);
bool IsIntersect( rvector& line_begin_, rvector& line_end_, rboundingbox& box_);
bool IsIntersect(rvector& line_begin_, rvector& line_dir_, rvector& center_, float radius_, float* dist = NULL, rvector* p = NULL );

// 원과 선분의 교차점 구하는 함수. dir는 normalize되어 있어야 한다
bool IsIntersect(const rvector& orig, const rvector& dir, const rvector& center, const float radius, rvector* p = NULL);

// 두 평면을 지나는 직선의 방정식을 구한다 
bool GetIntersectionOfTwoPlanes(rvector *pOutDir,rvector *pOutAPoint,rplane &plane1,rplane &plane2);

void MergeBoundingBox(rboundingbox *dest,rboundingbox *src);

// aabb box 를 트랜스폼 한다. 더 커진다
void TransformBox( rboundingbox* result, const rboundingbox& src, const rmatrix& matrix );


// 변환 매크로들

#define FLOAT2RGB24(r, g, b) ( ( ((long)((r) * 255)) << 16) | (((long)((g) * 255)) << 8) | (long)((b) * 255))
#define VECTOR2RGB24(v)		FLOAT2RGB24((v).x,(v).y,(v).z)
#define BYTE2RGB24(r,g,b)	((DWORD) (((BYTE) (b)|((WORD) (g) << 8))|(((DWORD) (BYTE) (r)) << 16)))
#define BYTE2RGB32(a,r,g,b)	((DWORD) (((BYTE) (b)|((WORD) (g) << 8))|(((DWORD) (BYTE) (r)) << 16)|(((DWORD) (BYTE) (a)) << 24)))
#define DWORD2VECTOR(x)		rvector(float(((x)& 0xff0000) >> 16)/255.f, float(((x) & 0xff00) >> 8)/255.f,float(((x) & 0xff))/255.f)

// progress 콜백 펑션타입
typedef void (*RFPROGRESSCALLBACK)(void *pUserParams,float fProgress);

_NAMESPACE_REALSPACE2_END

#endif