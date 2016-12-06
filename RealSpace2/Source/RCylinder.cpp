#include "stdafx.h"
#include "Rcylinder.h"
#include "RealSpace2.h"

using namespace RealSpace2;

RCylinder::RCylinder()
:mTopCentre(0,0,0),	mBottomCentre(0,0,0), mHeight(0), mRadius(0), mCylinder(0)
{
	GetIdentityMatrix(mWorld);
}

RCylinder::~RCylinder()
{
	SAFE_RELEASE( mCylinder );
}

bool RCylinder::isCollide( CDInfo* data_, CDInfoType cdType_ )
{
	rvector intersection;
	float distance;

	if( !getDistanceBetLineSegmentAndPoint( mTopCentre, mBottomCentre, data_->clothCD.v,  &intersection , NULL, distance ) )
	{
		return false;
	}

	if( distance > mRadius )
	{
		return false;
	}

	*data_->clothCD.pos = intersection + ( (*data_->clothCD.n) * mRadius * 1.2 );

	return true;
}

bool getDistanceBetLineSegmentAndPoint( const rvector& lineStart_, 
									   const rvector& lineEnd_, 
									   rvector* point_, 
									   rvector* intersection_, 
									   rvector* direction_, 
									   float& distance_ )
{
	rvector line = lineEnd_ - lineStart_;
	rvector cross_line = *point_ - lineStart_;
	float line_length_square = MagnitudeSq(line);

	float u = DotProduct(cross_line, line) / line_length_square;

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
		auto vec = *point_ - intersection;
		*direction_ = Normalized(vec);
	}

	distance_ = Magnitude(*point_ - intersection);

	return true;
}

void RCylinder::draw()
{
	rvector centre;
	rmatrix tr;

	rmatrix fix_rotation;
	fix_rotation = RGetRotX(PI_FLOAT * 0.5f);

	centre = ( mTopCentre + mBottomCentre ) * 0.5;
	tr = TranslationMatrix(centre + v3{ 0, 0, 50 });

    mWorld = mWorld * fix_rotation ;
	{
 		RGetDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
	}

	if( mCylinder == 0 )  
	{
		D3DXCreateCylinder( RGetDevice(), mRadius, mRadius, mHeight, 20, 10, &mCylinder, NULL );
	}

	RGetDevice()->SetTransform(D3DTS_WORLD, static_cast<D3DMATRIX*>(mWorld));

	mCylinder->DrawSubset( 0 );
	{
		RGetDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	}
}
