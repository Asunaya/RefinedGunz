#ifndef _RSPHERE_H
#define _RSPHERE_H


//#pragma once

//  [10/13/2003]
//////////////////////////////////////////////////////////////////////////
// RSphere
// Bound Sphere
// Magicbell
//////////////////////////////////////////////////////////////////////////

#include "RBoundary.h"
#include "RTypes.h"

// Declare

// class RSphere
class RSphere :	public RBoundary
{
public:
	rvector mCentre;
	float mRadius;

public:
	bool isCollide(	CDInfo* data_, CDInfoType cdType_ );
	inline void setSphere( rvector centre_, float radius_ );
	void setSphere( rvector& centre_ )
	{
		mCentre	= centre_;
	}
	void setSphere( float radius_ )
	{
		mRadius	= radius_;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//	<<<	Rendering Primitives >>>
	//	
	LPD3DXMESH mSphere;
	rmatrix mWorld;
	inline void setTransform( rmatrix& world_ );
	void draw();
	//
	//////////////////////////////////////////////////////////////////////////

public:
	RSphere(void);
	virtual ~RSphere(void);
};

void RSphere::setSphere( rvector centre_, float radius_ )
{
	mCentre = centre_;
	mRadius = radius_;
	D3DXMatrixTranslation( &mWorld, centre_.x, centre_.y + 100, centre_.z );
}

void RSphere::setTransform( rmatrix& world_ )
{
	D3DXMatrixTranslation( &mWorld, mCentre.x, mCentre.y, mCentre.z );
	mWorld *= world_;
}

#endif