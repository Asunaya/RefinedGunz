#ifndef __RSTYPES_H
#define __RSTYPES_H

#include "RSMVersion.h"
#include "rutils.h"
#include "CMList.h"

class RSMaterial;
class ASSMap;
class RSBase3D;
class RSObject;
class RSVisualObject;
class RSVisualSingleObject;
class RSPointLight;
class RSDecal;
class RSImage;
class ASSSchemeManager;
class RSConsoleRenderer;
class RSEnvironmentObject;
class RSMaterialList;
class RSMaterialManager;

struct rvertex {
public:
    float x,y,z;
	float px,py,pz;
    float sx,sy,sz;
	float pw;
	DWORD color,specular;
    float u,v;
	rvector normal;
	float color_r,color_g,color_b;

	rvertex() {}
	rvertex(rvector hv,float _u,float _v){x=hv.x;y=hv.y;z=hv.z;u=_u;v=_v;	}
	rvertex(float _x,float _y,float _z){x=_x;y=_y;z=_z;}
};

struct rface {
    rvertex *v[3];
    float 	s[3],t[3];
	rplane plane;
    DWORD  	color;
    RSMaterial* material;
    int		isValid,TextureHandle;
};

class rstrip {
public:
	int nVertex;
	rvertex **v;
	float *s,*t;
	rplane plane;
	bool	isOdd;
	bool	isValid;
	rstrip()	{ v=NULL;s=NULL;t=NULL; }
	virtual ~rstrip() { if(v) delete []v;if(s) delete []s;if(t) delete []t; }
};

struct rplueckercoord {
public:
	rplueckercoord() {}
	rplueckercoord(const rvector &origin,const rvector &target) 
		{	u=origin-target;v=CrossProduct(origin,target); }
	rvector u,v;
	inline friend float operator * (const rplueckercoord& p1, const rplueckercoord& p2) 
		{ return DotProduct(p1.u,p2.v)+DotProduct(p1.v,p2.u); }
};

struct RSVIEWPORT {
	int x1,y1,x2,y2;
};

struct RSCHUNKHEADER {
	DWORD ChunkID;
	DWORD ChunkSize;
};

#define RSCHUNKHEADERSIZE sizeof(RSCHUNKHEADER)

enum RSPIXELFORMAT {
	RS_PIXELFORMAT_565 = 1,
	RS_PIXELFORMAT_555 = 2,
	RS_PIXELFORMAT_X888 = 3
};

enum RSCULLSTYPE {
	RS_CULLSTYLE_NONE =0,
	RS_CULLSTYLE_CW	=1,
	RS_CULLSTYLE_CCW	=2
};

enum RSSHADOWSTYLE {
	RS_SHADOWSTYLE_NONE = 0,
	RS_SHADOWSTYLE_DRAFT = 1,
	RS_SHADOWSTYLE_FLAT = 2,
//	RS_SHADOWSTYLE_DETAIL = 3,
//	RS_SHADOWSTYLE_FASTDETAIL = 4,
	RS_SHADOWSTYLE_TEXTUREPROJECTION = 5,
	RS_SHADOWSTYLE_DETAILTEXTUREPROJECTION = 6,
};

enum RSENVIRONMENTSTYLE {
	RS_ENVIRONMENTSTYLE_WRAP_Z = 0,
	RS_ENVIRONMENTSTYLE_CYLINDER_Z	=1
};

enum RSANIMATIONSTYLE {
	RS_ANIMATIONSTYLE_STOP = 0,
	RS_ANIMATIONSTYLE_ROUND = 1,
	RS_ANIMATIONSTYLE_ONCE = 2
};

enum RSALPHASTYLE {
	RS_ALPHASTYLE_NONE		= 0,
	RS_ALPHASTYLE_ADD		= 1,
	RS_ALPHASTYLE_ALPHA		= 2,
	RS_ALPHASTYLE_LIGHTMAP	= 3,
	RS_ALPHASTYLE_ALPHAMAP	= 4,
	RS_ALPHASTYLE_SHADOW	= 5,
	RS_ALPHASTYLE_MARKS		= 6
};

enum RSLIGHTSTYLE {
	RS_LIGHTSTYLE_VERTEX	= 0,
	RS_LIGHTSTYLE_LIGHTMAP	= 1
};

enum RSANISEEKSTYLE {
	RS_SEEK_END = -1,
	RS_SEEK_SET = 0
};

typedef CMLinkedList<rvertex> RSVertexList;
typedef CMLinkedList<rface> RSFaceList;
typedef CMLinkedList<RSPointLight> RSPointLightList;

// transform 관련
void	TransformVertex(rvertex &v,rmatrix &m);
void	TransformVertex(rvertex &v,rvector &s,rmatrix &m); // v: dest s: source
void	TransformVertexP(rvertex &v,rmatrix &m);
void	TransformVertex(rvertex &v,rmatrix44 &m);
void	TransformVertex(rvertex &v,rvector &s,rmatrix44 &m); // v: dest s: source
void	TransformVertexP(rvertex &v,rmatrix44 &m);
void	TransformVertexASM(rvertex&,rmatrix44&);

// plane관련
void SetupPlane(rvertex *v1,rvertex *v2,rvector *dir,rplane *p);
void SetupPlane(rvector &v1,rvector &v2,rvector &v3,rplane *p);
void SetupPlane(rvector &dir,rvector &apoint,rplane *p);
void SetupWorldPlane(rvertex *v1,rvertex *v2,rvector *dir,rplane *p);

void TransformPlane(rmatrix &m,rplane *tp,const rplane *p);
float GetDistance(const rvector &position,const rvector &lineorigin,const rvector &linedir);	// 점과 직선의 거리
float GetDistance(const rvector &position,const rplane &plane);
float GetDistance(rboundingbox *bb,rplane *plane);
void GetDistanceMinMax(rboundingbox &bb,rplane &plane,float *MinDist,float *MaxDist);
bool isInPlane(rboundingbox *bb,rplane *plane);
bool isInPlane(const rvector &position,const rplane &plane);

void ComputeViewFrustrum(rplane *plane,rmatrix44 *matrix,rvector cameraposition, float x,float y,float z);
void ComputeZPlane(rplane *plane,rvector cameraorientation,rvector cameraposition, float z,int sign);

// raytracing 관련
bool isInTheTriangle(rvertex *v0,rvertex *v1,rvertex *v2,int ix,int iy,float *z);
bool isInTheFace(rface *f,int x,int y,float *z);
bool isLineIntersectBoundingBox(rplueckercoord *line,rboundingbox *bb);
bool IsIntersect(rplueckercoord *p,rface *f);
bool IsIntersect(rplueckercoord &p,rvector &v1,rvector &v2,rvector &v3);

// bounding volume & view Frustrum 관련
void MergeBoundingBox(rboundingbox *dest,rboundingbox *src);
bool isInViewFrustrum(rboundingbox *bb,rplane *plane);
bool isInViewFrustrum(const rvector &point,rplane *plane);					// a point
bool isInViewFrustrum(const rvector &point,float radius,rplane *plane);		// bounding sphere
bool isInViewFrustrumWithZ(rboundingbox *bb,rplane *plane);					// bounding box
bool isInViewFrustrumwrtnPlanes(rboundingbox *bb,rplane *plane,int nplane);

// color convert macros
#define FLOAT2RGB24(r, g, b) ( ( ((long)((r) * 255)) << 16) | (((long)((g) * 255)) << 8) | (long)((b) * 255))
#define VECTOR2RGB24(v)		FLOAT2RGB24((v).x,(v).y,(v).z)
#define BYTE2RGB24(r,g,b)	((DWORD) (((BYTE) (b)|((WORD) (g) << 8))|(((DWORD) (BYTE) (r)) << 16)))
#define BYTE2RGB32(a,r,g,b)	((DWORD) (((BYTE) (b)|((WORD) (g) << 8))|(((DWORD) (BYTE) (r)) << 16)|(((DWORD) (BYTE) (a)) << 24)))
#define DWORD2VECTOR(x)		rvector(float(((x)& 0xff0000) >> 16)/255.f, float(((x) & 0xff00) >> 8)/255.f,float(((x) & 0xff))/255.f)
// color값에 alpha비트를 넣어 추가한다.
#define RSCOMBINEALPHA(color, alpha)	(((long)(alpha)<<24)|(color&0xffffff))
#define RGB322RGB16(dw)		(WORD)( RS_RGB(unsigned char(((dw)&0xff0000)>>16),unsigned char(((dw)&0xff00)>>8),unsigned char((dw)&0xff)) )
#endif