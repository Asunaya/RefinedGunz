#ifndef __RSANIMATIONINFO_H
#define __RSANIMATIONINFO_H

#include <string.h>
#include <stdio.h>
#include <list>

using namespace std;

#include "rutils.h"
#include "rquaternion.h"

class RSObject;

#define RSA_HEADER_ID		667
#define RSA_HEADER_BUILD	2

struct RSAHEADER{
	int			RSAID;
	int			Build;
};

typedef enum _RSANIMATIONMETHOD
{
	RSANIMATIONMETHOD_TRANSFORM	=0,
	RSANIMATIONMETHOD_VERTEX	=1,
	RSANIMATIONMETHOD_KEYFRAME	=2
} RSANIMATIONMETHOD;

struct RSVisibilityKey {
	float time,value;
};

struct RSScaleKey {
	float time;
	rvector value;
};

struct RSRotationKey {
	float time;
	rquaternion value;
};

struct RSPositionKey {
	float time;
	rvector value;
};

template<class NEW_TYPE> class RSKeyController : public list < NEW_TYPE* > {
public:
	list < NEW_TYPE* > :: iterator m_Current;
public:
	virtual ~RSKeyController() {
		while(size())
		{
			delete *(begin());
			erase(begin());
		}
	}

	bool Save(FILE *file) {
		list < NEW_TYPE* > ::iterator i;
		int nSize=size();
		fwrite(&nSize,sizeof(int),1,file);
		for(i=begin();i!=end();i++)
			fwrite(*i,sizeof(NEW_TYPE),1,file);
		return true;
	}

	bool Open(FILE *file) {
		int i,nSize;
		fread(&nSize,sizeof(int),1,file);
		for(i=0;i<nSize;i++)
		{
			NEW_TYPE *p=new NEW_TYPE;
			fread(*p,sizeof(NEW_TYPE),1,file);
			push_back(p);
		}
		return true;
	}

	virtual void Get(float fTime,NEW_TYPE *ret) {}
};

class RSVisibilityControl : public RSKeyController <RSVisibilityKey>
{
public:
	void Get(float fTime,RSVisibilityKey *ret);
	float GetVisibility(float time) { RSVisibilityKey ret;Get(time,&ret);return ret.value; }
};

class RSAnimationInfo {
public:
// General Functions
	void		SetName(char *n)	{ if(name) delete []name;name=new char[strlen(n)+1];strcpy(name,n); }
	const char* GetName()			{ return name; }
	
	int			GetFrameCount()		{ return nFrame; }			// 프레임 수

	void		SetSpeed(float s)	{ speed=s; }				// 익스포터에서 정해진 스피드.
	float		GetSpeed()			{ return speed; }
	
	void		SetFrameSpeed(float s)		{ m_FrameSpeed=s;	}		// 원래 재생 속도. (FPS)
	float		GetFrameSpeed()				{ return m_FrameSpeed; }

	float		GetDuration();									// 애니메이션 한바퀴 도는데 걸리는 시간. (초)

	RSANIMATIONMETHOD	GetAnimationMethod() { return m_AnimationMethod;	}
// for animation
	int			GetMeshCount()		{ return nMesh; }
	rmatrix*	GetTransformMatrix(int iMesh,int iFrame);
// for vertex animation
	int			isContainingVertexAnimation(int iMesh);
	rvector*	GetAnimationVertices(int iMesh,int nFrame);

//	bool		Save(FILE *file);
	bool		Load(FILE *file);

	RSVisibilityControl *GetVisibilityControl(int nMesh) { return Visibility+nMesh; }

	RSAnimationInfo();
	virtual ~RSAnimationInfo();

private:
	char	*name;
	float	speed,m_FrameSpeed;
	
	int		nFrame,nMesh;
	RSANIMATIONMETHOD m_AnimationMethod;
	rmatrix *tm;

// for vertex animation
	int		*nVertAnim;
	rvector **vertanim;

// for visibility animation
	 RSVisibilityControl *Visibility;

	friend RSObject;
};

#endif