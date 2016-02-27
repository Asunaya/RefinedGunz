#ifndef _ZCLOTHEMBLEM_H
#define _ZCLOTHEMBLEM_H

//#pragma once

#include "RCloth.h"
//#include "RHalfEdgeMesh.h"
#include "RTypes.h"

//////////////////////////////////////////////////////////////////////////
//	Struct
//////////////////////////////////////////////////////////////////////////

#define GRENADE_SPEAR_EMBLEM_POWER	1000 //2000
#define BULLET_SPEAR_EMBLEM_POWER	1500 //3000
#define SHOTGUN_SPEAR_EMBLEM_POWER	2000 //3500
#define ROCKET_SPEAR_EMBLEM_POWER	2500 //5000
#define EXPLOSION_EMBLEM_POWER		3000 //5000


//////////////////////////////////////////////////////////////////////////
//	ZClothEmblem
//////////////////////////////////////////////////////////////////////////
enum RESTRICTION_AXIS
{
	AXIS_X, AXIS_Y, AXIS_Z,
};
enum RESTRICTION_COMPARE
{
	COMPARE_GREATER, COMPARE_LESS,
};

struct sRestriction
{
	RESTRICTION_AXIS	axis;
	float							position;
	RESTRICTION_COMPARE compare;
};

class ZClothEmblem : public RCloth
{
private:
	static unsigned int	msRef;

protected:
	ZWorld*			m_pWorld;
	RMeshNode*		mpMeshNode;
	RBaseTexture*	mpTex;
	rvector*		mpWind;
	float			mfBaseMaxPower;
	rvector			mBaseWind;
	D3DLIGHT9*		mpLight;

	RWindGenerator	mWndGenerator;

	RealSpace2::rboundingbox	mAABB;
	list<sRestriction*> mRestrictionList;
	DWORD		mMyTime;
	bool		mbIsInFrustrum; // 충돌, 시뮬레이션 대상

protected:
	//LPDIRECT3DINDEXBUFFER9 mIndexBuffer;

protected:
	virtual void	accumulateForces();
	virtual void	varlet();
	virtual void	satisfyConstraints();

public:

	float			m_fDist;
	rmatrix			mWorldMat;
	
public:
	virtual void	update();
	virtual void	render();
	virtual void	UpdateNormal();

	void setOption( int nIter_, float power_, float inertia_ );	// 순서대로 반복횟수, 힘, 관성(0~1)
	void CreateFromMeshNode( RMeshNode* pMeshNdoe_ , ZWorld* pWorld);

	void setForce( float x_, float y_, float z_ )
	{
		mpWind->x	= x_;
		mpWind->y	= y_;
		mpWind->z	= z_;
	};
	void setExplosion( rvector& pos_, float power_ );
	void CheckSpearing( rvector& bullet_begine_, rvector& bullet_end_, float power_ );	// 총알 뚤리는 효과

	void OnInvalidate();
	void OnRestore();

	void SetBaseWind( rvector& w_ )
	{
		mBaseWind	= w_;
	}
	void SetBaseMaxPower( float p_ )
	{
		mfBaseMaxPower	= p_;
	}
	void AddRestriction( sRestriction* rest_ )
	{
		mRestrictionList.push_back( rest_ );
	}
	RWindGenerator* GetWndGenerator(){ return &mWndGenerator; }

	static int GetRefCount() { return msRef;}

public:
	ZClothEmblem(void);
	virtual ~ZClothEmblem(void);

};


//////////////////////////////////////////////////////////////////////////
//	Flag List
//////////////////////////////////////////////////////////////////////////
class ZEmblemList : public list<ZClothEmblem*>
{
protected:
	map<string, ZClothEmblem*> mEmblemMap;
	map<string, ZClothEmblem*>::iterator mEmblemMapItor;

public:
	virtual ~ZEmblemList();

public:
	void	Update();
	void    Draw();
	ZClothEmblem*	Get( int i_ );
	void	SetExplosion( rvector& pos_, float power_ );
	void	CheckSpearing( rvector& bullet_begine_, rvector& bullet_end_, float power_ );

	void	OnInvalidate();
	void	OnRestore();	
	void	Clear();
	void Add( ZClothEmblem* p_, char* pName_ );

	void	InitEnv( char* pFileName_ );
};

#endif