#include "stdafx.h"
#include "RSphere.h"
#include "RealSpace2.h"

using namespace RealSpace2;

RSphere::RSphere(void)
//:mCentre(0,0,0), mRadius(10), mSphere(0)
{
	mCentre	= rvector(0,0,0);
	mRadius	=  10.f;
	mSphere = 0;
}

RSphere::~RSphere(void)
{
#ifdef _DEBUG
	SAFE_RELEASE( mSphere );
#endif
}


//////////////////////////////////////////////////////////////////////////
// isCollide
//////////////////////////////////////////////////////////////////////////

bool RSphere::isCollide( CDInfo* data_, CDInfoType cdType_ )
{
	rvector distance = *data_->clothCD.v - mCentre;
	if( D3DXVec3LengthSq( &distance ) < ( mRadius * mRadius ) )
	{
		*data_->clothCD.pos = mCentre + ( mRadius * *data_->clothCD.n);
		return true;
	}
	return false;
}



//////////////////////////////////////////////////////////////////////////
// draw
//////////////////////////////////////////////////////////////////////////

void RSphere::draw()
{
#ifdef _DEBUG
	RGetDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

	if(mSphere == 0)
	{
		D3DXCreateSphere( RGetDevice(), mRadius, 20, 20, &mSphere, NULL );
	}

	RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
	RGetDevice()->SetTransform( D3DTS_WORLD, &mWorld );

 	mSphere->DrawSubset( 0 );

	RGetDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
#endif
}
