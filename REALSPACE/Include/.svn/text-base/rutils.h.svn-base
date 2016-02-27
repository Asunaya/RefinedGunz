#ifndef __RUTILS_H
#define __RUTILS_H

#include <math.h>
#include <windows.h>
#include <crtdbg.h>

const float pi=3.141592654f;

typedef enum {NEGATIVE= -1, ZERO= 0, POSITIVE= 1} SIGN;

#define TOLER 0.0000076
#define IS_EQ(a,b) ((fabs((double)(a)-(b)) >= (double) TOLER) ? 0 : 1)
#define IS_EQ3(a,b) (IS_EQ((a).x,(b).x)&&IS_EQ((a).y,(b).y)&&IS_EQ((a).z,(b).z))
#define signof(f) (((f) < -TOLER) ? NEGATIVE : ((f) > TOLER ? POSITIVE : ZERO))
#define RANDOMFLOAT ((float)rand()/(float)RAND_MAX)

// RELEASE & DELETE ¸ÅÅ©·Î ( from dxutil.h )
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

typedef struct { float x1,y1,x2,y2; } FRECT,*LPFRECT;
typedef union {
	struct {
		float Minx,Maxx,Miny,Maxy,Minz,Maxz;
	};
	float m[3][2];
} rboundingbox;

struct rvector {
public:
	float x,y,z;

	rvector(float _x,float _y,float _z) { x=_x;y=_y;z=_z;}
	rvector(const rvector &v) { x=v.x;y=v.y;z=v.z; }
	rvector() {};

	inline friend rvector operator - (const rvector &v) { return rvector(-v.x,-v.y,-v.z); }
	inline friend rvector operator - (const rvector& v1, const rvector& v2) { return rvector(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z);	}
	inline friend rvector operator + (const rvector& v1, const rvector& v2) { return rvector(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z);	}
	inline friend void operator += (rvector& v, const rvector& v2) { v.x+=v2.x;v.y+=v2.y;v.z+=v2.z; }
	inline friend void operator -= (rvector& v, const rvector& v2) { v.x-=v2.x;v.y-=v2.y;v.z-=v2.z; }
	inline friend void operator *= (rvector& v, const float c) { v.x*=c;v.y*=c;v.z*=c; }
	inline friend void operator /= (rvector& v, const float c) { v.x/=c;v.y/=c;v.z/=c; }
	inline friend rvector operator * (const float c, const rvector& v) { return rvector(c*v.x, c*v.y, c*v.z); }
	inline friend rvector operator * (const rvector& v,const float c) { return rvector(c*v.x, c*v.y, c*v.z); }
	inline friend rvector operator / (const rvector& v,const float c) { return rvector(v.x/c, v.y/c, v.z/c); }
	inline friend bool operator==(const rvector& l, const rvector& r) { return (l.x==r.x && l.y==r.y && l.z==r.z); }
	inline friend bool operator!=(const rvector& l, const rvector& r) { return !(l.x==r.x && l.y==r.y && l.z==r.z); }

	inline float GetDistance(const rvector &v) const { return sqrtf((v.x-x)*(v.x-x)+(v.y-y)*(v.y-y)+(v.z-z)*(v.z-z)); }
	inline bool IsZero(void) const { return (x==0.0f && y==0.0f && z==0.0f); }
	inline float GetMagnitude() const { return sqrtf(x*x+y*y+z*z); }
	inline float GetSafeMagnitude() { if(IsZero()) return 0; else return (float)sqrt(x*x+y*y+z*z); }
	inline void Normalize(){
		float t=GetMagnitude();
		_ASSERT(t>0);
		x = x/t;
		y = y/t;
		z = z/t;
	}
	inline friend rvector Normalize(const rvector & v){
		rvector r;
		float t;

		t = ((rvector*)&v)->GetMagnitude();
		_ASSERT(t>0);
		r.x = v.x/t;
		r.y = v.y/t;
		r.z = v.z/t;
		return r;
	}
	/*
	inline friend rvector SafeNormalize(const rvector & v){
		if(v.IsZero()) return v;
		return Normalize(v);
	}
	*/
	inline friend float DotProduct (const rvector& v1, const rvector& v2) { return v1.x*v2.x + v1.y * v2.y + v1.z*v2.z; }
	inline friend rvector CrossProduct (const rvector& v1, const rvector& v2)
	{
		rvector result;
		result.x = v1.y * v2.z - v1.z * v2.y;
		result.y = v1.z * v2.x - v1.x * v2.z;
		result.z = v1.x * v2.y - v1.y * v2.x;
	//	_ASSERT((result.x!=0.0f)||(result.y!=0.0f)||(result.z!=0.0f));
		return result;
	};
};

struct rplane {
	union {
		struct {
			float a,b,c,d; 
		};
		struct {
			rvector normal;
			float d;
		};
		float m[4];
	};
};


struct rmatrix43 {
    union {
        struct {
            float        _11, _12, _13;
            float        _21, _22, _23;
            float        _31, _32, _33;
            float        _41, _42, _43;

        };
        float m[4][3];
	};
	rmatrix43() {}
	rmatrix43(	float m11,float m12,float m13,
				float m21,float m22,float m23,
				float m31,float m32,float m33,
				float m41,float m42,float m43)
	{	_11=m11;_12=m12;_13=m13;
		_21=m21;_22=m22;_23=m23;
		_31=m31;_32=m32;_33=m33;
		_41=m41;_42=m42;_43=m43;	}
};

#pragma pack(16)
struct rmatrix44 {
    union {
        struct {
            float        _11, _12, _13, _14;
            float        _21, _22, _23, _24;
            float        _31, _32, _33, _34;
            float        _41, _42, _43, _44;

        };
        float m[4][4];
	};
	rmatrix44() {}
	rmatrix44(	float m11,float m12,float m13,float m14,
				float m21,float m22,float m23,float m24,
				float m31,float m32,float m33,float m34,
				float m41,float m42,float m43,float m44)
	{	_11=m11;_12=m12;_13=m13;_14=m14;
		_21=m21;_22=m22;_23=m23;_24=m24;
		_31=m31;_32=m32;_33=m33;_34=m34;
		_41=m41;_42=m42;_43=m43;_44=m44;	}
    float& operator()(int iRow, int iColumn) { return m[iRow][iColumn]; }
    const float& operator()(int iRow, int iColumn) const { return m[iRow][iColumn]; }
};
#pragma pack()

typedef rmatrix43 rmatrix;

// Arithmatic Functions of Matrix
rmatrix ZeroMatrix();
rmatrix IdentityMatrix();
rmatrix RotateZMatrix(const float rads);
rmatrix RotateYMatrix(const float rads);
rmatrix RotateXMatrix(const float rads);
rmatrix TranslateMatrix(const float dx, const float dy, const float dz);
rmatrix TranslateMatrix(const rvector &v);
rmatrix ScaleMatrix(const float size);
rmatrix ScaleMatrixXYZ(const float x,const float y,const float z);
rvector	TransformNormal(rvector &v,rmatrix &m);
rvector	TransformVector(rvector &v,rmatrix &m);
rmatrix ViewMatrix(const rvector& from,const rvector& at,const rvector& world_up,const float roll);
rmatrix ViewMatrix(const rvector& from,const rvector& dir,const rvector& world_up);
rmatrix ViewMatrix(const rvector &from,const rvector &dir);
rmatrix MatrixInverse(const rmatrix & m);

// followings are 44 matrix functions
rmatrix44 ZeroMatrix44();
rmatrix44 IdentityMatrix44();
rmatrix44 RotateZMatrix44(const float rads);
rmatrix44 RotateYMatrix44(const float rads);
rmatrix44 RotateXMatrix44(const float rads);
rmatrix44 TranslateMatrix44(const float dx, const float dy, const float dz);
rmatrix44 TranslateMatrix44(const rvector &v);
rmatrix44 ScaleMatrix44(const float size);
rmatrix44 ScaleMatrixXYZ44(const float x,const float y,const float z);
rvector	TransformNormal(rvector &v,rmatrix44 &m);
rvector	TransformVector(rvector &v,rmatrix44 &m);
rmatrix44 ViewMatrix44(const rvector& from,const rvector& at,const rvector& world_up,const float roll);
rmatrix44 ViewMatrix44(const rvector& from,const rvector& dir,const rvector& world_up);
rmatrix44 ViewMatrix44(const rvector &from,const rvector &dir);
rmatrix44 ProjectionMatrix(const float near_plane,const float far_plane,
						   const float fov_horiz,const float fov_vert,
						   bool bFlipHoriz=false,bool bFlipVert=false);
rmatrix44 MatrixInverse(const rmatrix44 & m);

rmatrix MatrixMult(rmatrix &m1,rmatrix &m2);
rmatrix44 MatrixMult(rmatrix &m1,rmatrix44 &m2);
rmatrix44 MatrixMult(rmatrix44 &m1,rmatrix &m2);

typedef rmatrix44(_MatrixMult44)(rmatrix44 &m1,rmatrix44 &m2);
extern _MatrixMult44 *MatrixMult44;

inline rmatrix operator * (rmatrix& m1,rmatrix& m2) { return MatrixMult(m1,m2); }
inline rmatrix44 operator * (rmatrix& m1,rmatrix44& m2) { return MatrixMult(m1,m2); }
inline rmatrix44 operator * (rmatrix44& m1,rmatrix& m2) { return MatrixMult(m1,m2); }
inline rmatrix44 operator * (rmatrix44& m1,rmatrix44& m2) { return MatrixMult44(m1,m2); }

// help functions
float	GetAngle(rvector &a);
float	GetAngleOfVectors(rvector &ta,rvector &tb);
rvector InterpolatedVector(rvector &a,rvector &b,float t1);

rmatrix ReflectionMatrix(rvector &dir,rvector &apoint);
rmatrix ShadowProjectionMatrix(rvector &normal,rvector &apoint,rvector &lightdir);

// Initialize Æã¼Ç..
void RUtils_Initilize();
extern bool g_bSSE;

#endif