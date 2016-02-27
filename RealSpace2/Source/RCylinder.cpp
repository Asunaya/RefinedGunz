#include "stdafx.h"
#include "Rcylinder.h"
#include "RealSpace2.h"

using namespace RealSpace2;

//////////////////////////////////////////////////////////////////////////
//	생성자
//////////////////////////////////////////////////////////////////////////

RCylinder::RCylinder(void)
:mTopCentre(0,0,0),	mBottomCentre(0,0,0), mHeight(0), mRadius(0), mCylinder(0)
{
	D3DXMatrixIdentity(&mWorld);
}


//////////////////////////////////////////////////////////////////////////
//	소멸자
//////////////////////////////////////////////////////////////////////////

RCylinder::~RCylinder(void)
{
	SAFE_RELEASE( mCylinder );
}


//////////////////////////////////////////////////////////////////////////
// isCollide Function
//////////////////////////////////////////////////////////////////////////

bool RCylinder::isCollide( CDInfo* data_, CDInfoType cdType_ )
{
	rvector intersection;
	rvector direction;
	float distance;

	if( !getDistanceBetLineSegmentAndPoint( mTopCentre, mBottomCentre, data_->clothCD.v,  &intersection , NULL, distance ) )
	{
		return false;
	}

	if( distance > mRadius )
	{
		return false;
	}

	// 충돌이 있을 경우 input vertex의 normal 방향으로 vertex를 이동시킨다
	*data_->clothCD.pos = intersection + ( (*data_->clothCD.n) * mRadius * 1.2 );
	//*data_->clothCD.pos = intersection + *data_->clothCD.n;

	return true;
}


//////////////////////////////////////////////////////////////////////////
// getDistanceBetLineSegmentAndPoint
//////////////////////////////////////////////////////////////////////////
bool getDistanceBetLineSegmentAndPoint( const rvector& lineStart_, 
									   const rvector& lineEnd_, 
									   rvector* point_, 
									   rvector* intersection_, 
									   rvector* direction_, 
									   float& distance_ )
{
	rvector line = lineEnd_ - lineStart_;
	rvector cross_line = *point_ - lineStart_;
	float line_length_square = D3DXVec3LengthSq( &line );

	float u = ( D3DXVec3Dot( &cross_line, &line ) ) / line_length_square ;

	if( u < 0.0f || u > 1.0f )
	{
		return false;
	}

	rvector intersection = lineStart_ + u * (lineEnd_ - lineStart_);

	if( intersection_ != NULL )
	{
		*intersection_	= intersection;
	}

	if( direction_ != NULL )
	{
		D3DXVec3Normalize( direction_, &( *point_ - intersection ) );
	}

	distance_ = D3DXVec3Length( &( *point_ - intersection ) );

	return true;
}


//////////////////////////////////////////////////////////////////////////
// draw
//////////////////////////////////////////////////////////////////////////

void RCylinder::draw()
{
	rvector centre;
	rmatrix tr;

	rmatrix fix_rotation;
	D3DXMatrixRotationX( &fix_rotation, D3DX_PI * 0.5 );

	centre = ( mTopCentre + mBottomCentre ) * 0.5;
	D3DXMatrixTranslation( &tr, centre.x, centre.y, centre.z + 50 );

    mWorld = mWorld * fix_rotation ;
	{
 		RGetDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
	}

	if( mCylinder == 0 )  
	{
		D3DXCreateCylinder( RGetDevice(), mRadius, mRadius, mHeight, 20, 10, &mCylinder, NULL );
	}

 	RGetDevice()->SetTransform( D3DTS_WORLD, &mWorld ); 

	mCylinder->DrawSubset( 0 );
	{
		RGetDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	}
}
