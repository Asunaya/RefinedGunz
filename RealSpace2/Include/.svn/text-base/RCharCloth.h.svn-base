#ifndef _RCHARCLOTH_H
#define _RCHARCLOTH_H

//////////////////////////////////////////////////////////////////////////
//	Cloth for Gunz
//	- 코트전용
//	Magicbell
//  [10/21/2003]
//////////////////////////////////////////////////////////////////////////

//#pragma once

#include "RCloth.h"
#include "RMesh.h"
#include "RCylinder.h"
#include "RSphere.h"

enum CLOTH_BONE
{
	BIPS01,
	LTHIGH,
	RTHIGH,
	LCALF,
	RCALF,
	LFOOT,
	RFOOT,
	NUM_CLOTH_BONE,
};

enum UPDATE_STATUS
{
	ALL					= 0x0,
	NOT_COLLISION		= 0x01,
	NOT_VALET			= 0x02,
	NOT_CAL_LENGTH		= 0x04,
	CHARACTER_DIE_STATE = 0x08,
};

using namespace RealSpace2;

class RCharCloth : public RCloth
{
public:
	RCharCloth(void);
	virtual ~RCharCloth(void);

public:
	void create( RMesh* pMesh_, RMeshNode* pMeshNode_ );
	virtual void render();
	void renderforce();

	void setForce( rvector& force_ );
	void update( bool bGame,rmatrix* pWorldMat_, float fDist_  = 0 );


	inline void setWorldMatrix( rmatrix& mat_ ) { mWorldMat = mat_; };
	inline void SetStatus( const int state )	{	mUpdateStatus = state;		}

	void OnInvalidate();
	void OnRestore();

protected:

	virtual void accumulateForces(bool bGame);
	virtual void satisfyConstraints();
	virtual void valet();
	virtual void UpdateNormal();

	void initialize( );
	void updatePosition( rmatrix* pWorldMat_ );
	void updateCO();

	static void prerender();
	static void postrender();

public:
	RMesh*		mpMesh;
	RMeshNode*	mpMeshNode;

	rmatrix		mLocalMat;
	rmatrix		mLocalInv;

protected:
	rmatrix		mWorldMat;		
	rmatrix		mWorldInv;		//	world -> local
	rvector		mForceField;	//	input force
	rvector*	mpInitNormal;

	bool		mInitParticle;				//	리스폰 같이 순간적으로 캐릭터가 멀리 뛸 경우 옷이 갑자기 늘어나는 현상 방지용
	RSphere		mSphere[6];

	RMeshNode*	mBips[NUM_CLOTH_BONE];
	int			mUpdateStatus;
	DWORD mTime;
};


#endif