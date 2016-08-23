#include "stdafx.h"

#include ".\zshadow.h"
#include ".\ZCharacter.h"
#include "RBspObject.h"
#include "MDebug.h"

#define VALID_SHADOW_LENGTH 250
#define VALID_SHADOW_BOUNDARY_SQUARE 62500

extern ZApplication g_app;

rvector ZShadow::mNormal;

//////////////////////////////////////////////////////////////////////////
//	Constructor / Desturctor
//////////////////////////////////////////////////////////////////////////
ZShadow::ZShadow(void)
{
	// normal은 +z
	mNormal = rvector( 0.f, 0.f, 1.f);

	D3DXMatrixIdentity( &mWorldMatLF );
	D3DXMatrixIdentity( &mWorldMatRF );

	bLFShadow = false;
	bRFShadow = false;
}

//////////////////////////////////////////////////////////////////////////
//	draw
//////////////////////////////////////////////////////////////////////////
void ZShadow::draw(bool bForced)
{
	if( (bLFShadow==false) && ((bRFShadow==false)))
		return;

	float blend_factor = ( mfDistance )/ VALID_SHADOW_BOUNDARY_SQUARE;

		 if( blend_factor >= 1  )	blend_factor = 0;
	else if( mfDistance <= 0 )		blend_factor = 1;
	else							blend_factor = 1 - blend_factor;

	DWORD _color = ((DWORD)(blend_factor * 255))<<24 | 0xffffff;

	if(bLFShadow)
		ZGetEffectManager()->AddShadowEffect(mWorldMatLF,_color);

	if(bRFShadow)
		ZGetEffectManager()->AddShadowEffect(mWorldMatRF,_color);

}

//////////////////////////////////////////////////////////////////////////
//	setSize
//	(desc) return scale matrix which scales by size_
//	(ref) this shadow object is unit length(1) 2d box
//	+ helper function
//////////////////////////////////////////////////////////////////////////
rmatrix ZShadow::setSize( float size_  )
{
	rmatrix ScaleMatrix;
	D3DXMatrixScaling( &ScaleMatrix, size_, size_, size_ );
	return ScaleMatrix;
}

//////////////////////////////////////////////////////////////////////////
//	setDirection
//	(desc) return rotation matrix to match shadow object's normal to input direction
//	+ helper function
//////////////////////////////////////////////////////////////////////////
rmatrix ZShadow::setDirection( rvector& dir_ )
{
	rmatrix xRotMat;
	rmatrix yRotMat;

	rvector xVector = dir_;
	xVector.y = 0;
	float xtheta = D3DXVec3Dot( &mNormal, &xVector );

	rvector yVector = dir_;
	yVector.x = 0;
	float yTheta = D3DXVec3Dot( &mNormal, &yVector );

	D3DXMatrixRotationX( &xRotMat, xtheta );
	D3DXMatrixRotationY( &yRotMat, yTheta );

	return xRotMat*yRotMat;
}

//////////////////////////////////////////////////////////////////////////
//	setMatirx
//	(desc) call by outside
//		set transformation matrix
//////////////////////////////////////////////////////////////////////////
bool ZShadow::setMatrix(ZCharacterObject& char_, float size_  )
{
	return setMatrix( *char_.m_pVMesh ,size_);
}

bool ZShadow::setMatrix( RVisualMesh& vmesh, float size_ /* = 100.f  */, RBspObject* p_map /* = 0 */)
{
	// 캐릭터의 발의 위치 얻어오기
	rvector footPosition[2];
	footPosition[0] = vmesh.GetLFootPosition();
	footPosition[1] = vmesh.GetRFootPosition();
	// 발의 위치로 부터의 floor의 위치 얻어오기
	if( p_map == 0 )
		p_map = g_app.GetGame()->GetWorld()->GetBsp();

	rvector floorPosition[2];
	rvector dir[2];

	bLFShadow = true;
	bRFShadow = true;

	if( !p_map->GetShadowPosition( footPosition[0], rvector( 0, 0, -1), &dir[0], &floorPosition[0] ))
	{
		if(g_pGame) {
			floorPosition[0] = g_pGame->GetFloor(footPosition[0]);
		} else { // 그냥발위치..
			bLFShadow = false;
		}

	}
	if( !p_map->GetShadowPosition( footPosition[1], rvector( 0, 0, -1), &dir[1], &floorPosition[1] ))
	{
		if(g_pGame) {
			floorPosition[1] = g_pGame->GetFloor(footPosition[1]);
		} else { 
			bRFShadow = false;
		}
	}

	if( (bLFShadow==false) && ((bRFShadow==false)))
		return false;

	// 거리 측정하여 그림자를 그려줄 것인지 결정
	float distanceL , distanceR;
	auto vecx = footPosition[0] - floorPosition[0];
	auto vecy = footPosition[1] - floorPosition[1];
	distanceL = D3DXVec3LengthSq(&vecx) - 200;
	distanceR = D3DXVec3LengthSq(&vecy) - 200;
	//float boundarySquare = VALID_SHADOW_LENGTH * VALID_SHADOW_LENGTH;
	
	if( VALID_SHADOW_BOUNDARY_SQUARE >= distanceL && floorPosition[0].z < footPosition[0].z )	bLFShadow = true;
	else	bLFShadow = false;
	
	if( VALID_SHADOW_BOUNDARY_SQUARE >= distanceR && floorPosition[1].z < footPosition[1].z)	bRFShadow = true;
	else	bRFShadow = false;
		
	mfDistance = ( distanceL + distanceR ) * 0.5 ;
	
	//matrix setup
	float fSize = vmesh.m_vScale.x * size_;
	rmatrix scaleMat = setSize( size_ );
	
	if( bLFShadow )
	{
		// 왼발 위치의 floor의 normal
		rmatrix tlanslationMat;
		D3DXMatrixTranslation( &tlanslationMat, floorPosition[0].x, floorPosition[0].y, floorPosition[0].z + 1 );
		mWorldMatLF = scaleMat*tlanslationMat;
	}
	if( bRFShadow )
	{
		// 오른발 위치의 floor의 normal
		rmatrix tlanslationMat;
		D3DXMatrixTranslation( &tlanslationMat, floorPosition[1].x, floorPosition[1].y, floorPosition[1].z + 1 );
		mWorldMatRF = scaleMat*tlanslationMat;
	}

	return true;
}
