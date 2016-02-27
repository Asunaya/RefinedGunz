#include ".\water.h"
#include "RealSpace2.h"
#include "MDebug.h"

using namespace RealSpace2;

static bool screenShot = false;
RVertex ZWater::msCopyTemp[MAX_WATER_SURFACE_VERTEX];

#define  TEXTURE_SIZE 512

	static LPDIRECT3DSURFACE8 stencil;
	static bool done = false;


//////////////////////////////////////////////////////////////////////////
//	생성자
//////////////////////////////////////////////////////////////////////////

ZWater::ZWater(void)
{
	mpRenderToSurfaceForReflection	=	0;
	mpRenderToSurfaceForRefraction	=	0;
	mpWaterReflectionSurface	=	0;
	mpWaterRefractionSurface	=	0;
	mpWaterReflectionTexture	=	0;
	mpWaterRefractionTexture	=	0;
	mpWaterVB	=	0;				
	mpWaterIB	=	0;
	mVertexList = 0;
	mIndexList = 0;

	mWidth = 0;
	mHeight = 0;

	mNumVertices = 0;
	mNumIndices = 0;
}


//////////////////////////////////////////////////////////////////////////
// 소멸자
//////////////////////////////////////////////////////////////////////////

ZWater::~ZWater(void)
{
	Release();
}

//////////////////////////////////////////////////////////////////////////
// Release
//////////////////////////////////////////////////////////////////////////

bool ZWater::Release()
{
	SAFE_RELEASE( mpWaterReflectionSurface );
	SAFE_RELEASE( mpWaterRefractionSurface );
	SAFE_RELEASE( mpWaterReflectionTexture );
	SAFE_RELEASE( mpWaterRefractionTexture );
	SAFE_RELEASE( mpRenderToSurfaceForReflection );
	SAFE_RELEASE( mpRenderToSurfaceForRefraction );
	SAFE_RELEASE( mpWaterVB );
	SAFE_RELEASE( mpWaterIB );
	SAFE_DELETE_ARRAY( mVertexList );
	SAFE_DELETE_ARRAY( mIndexList );

	vector<sWave*>::iterator iter;
	for( iter = mWaveList.begin(); iter != mWaveList.end(); ++iter )
	{
		SAFE_DELETE( *iter );
	}
	mWaveList.clear();

	return true;
}


//////////////////////////////////////////////////////////////////////////
//	Initialize
//////////////////////////////////////////////////////////////////////////
bool ZWater::Initialize( int width_, int heiht_, rvector& pos_, float diff_ /*= 1.0f*/ )
{
	int i,j;

	mWidth = width_;
	mHeight = heiht_;
	mNumVertices = (heiht_+1)*(width_+1);
	mPosZ = pos_.z;

	//	Create Vertex
	//	mVertexList = new RVertex[mNumVertices];
	mVertexList = new D3DXVECTOR3[mNumVertices];
	for( i = 0; i < heiht_ + 1; ++i )
	{
		for( j = 0; j < width_ + 1; ++j )
		{
			// create vertex pos - ( i, j )
			// y(height)
			// ^
			// |
			// ---> x (width)
			mVertexList[i*(width_+1)+j] = rvector( pos_.x + (diff_ * j), pos_.y + (diff_ * i), pos_.z );
		}
	}

	//	Create Index
	//	left hand coordinate & CW
	mNumIndices = heiht_*width_*6;
	mIndexList = new WORD[mNumIndices];
	int baseIndex;
	for( i = 0; i < heiht_; ++i )
	{
		for( j = 0 ; j < width_; ++j )
		{
			// index 
			// 1((i+1)*(width_+1)+j)---------2,4((i+1)*(width_+1)+j+1)
			// |                             |
			// |                             |
			// |                             |
			// 0,3(i*(width_+1)+j)-----------5(i*(width_+1)+j+1)
			baseIndex = (i*width_*6);
			mIndexList[baseIndex + j*6] = i * (width_+1) + j;			// 0
			mIndexList[baseIndex + j*6+1] = (i+1) * (width_+1) + j;		// 1
			mIndexList[baseIndex + j*6+2] = (i+1) * (width_+1) + j+1;	// 2
			mIndexList[baseIndex + j*6+3] = i * (width_+1) + j;			// 3
			mIndexList[baseIndex + j*6+4] = (i+1) * (width_+1) + j+1;	// 4
			mIndexList[baseIndex + j*6+5] = i * (width_+1) + j + 1;		// 5
		}
	}

	// 카메라를 뒤집는 기준이 되는 평면
	D3DXPlaneFromPoints( &mClipPlane, &mVertexList[0], &mVertexList[1], &mVertexList[mWidth+1] );
	D3DXMatrixReflect( &mMirrorMatrix, &mClipPlane );
	RGetDevice()->GetDepthStencilSurface( &mpHoldStencil );
	RGetDevice()->GetRenderTarget( &mpHoldBackBuffer );
	RGetDevice()->GetTransform( D3DTS_PROJECTION, &mProjHolder );

	return CreateBuffer();
}


//////////////////////////////////////////////////////////////////////////
//	createBuffer
//////////////////////////////////////////////////////////////////////////

bool ZWater::CreateBuffer()
{
	if( FAILED( RGetDevice()->CreateVertexBuffer( mNumVertices * sizeof(RVertex), 
		D3DUSAGE_WRITEONLY, RVertexType, D3DPOOL_MANAGED, &mpWaterVB ) ) )
		return false;

	if( FAILED( RGetDevice()->CreateIndexBuffer( mNumIndices * sizeof(WORD), 
		D3DUSAGE_WRITEONLY,	D3DFMT_INDEX16, D3DPOOL_MANAGED, &mpWaterIB ) ) )
		return false;

	VOID * pIndexes;
	if( FAILED( mpWaterIB->Lock( 0, mNumIndices * sizeof(WORD), (BYTE**)&pIndexes, 0 ) ) )
		return false;
	memcpy( pIndexes, mIndexList, mNumIndices * sizeof(WORD) );
	mpWaterIB->Unlock();

	D3DDISPLAYMODE mode;
	RGetDevice()->GetDisplayMode( &mode );

	// Texture for Reflection
	if(FAILED( D3DXCreateTexture( RGetDevice(), TEXTURE_SIZE, TEXTURE_SIZE, 1, 
		D3DUSAGE_RENDERTARGET, mode.Format, D3DPOOL_DEFAULT, &mpWaterReflectionTexture))
		&& FAILED( D3DXCreateTexture( RGetDevice(), TEXTURE_SIZE, TEXTURE_SIZE, 1, 
		D3DUSAGE_DYNAMIC, mode.Format, D3DPOOL_DEFAULT, &mpWaterReflectionTexture)))
	{
		return false;
	}

	mpWaterReflectionTexture->GetSurfaceLevel( 0, &mpWaterReflectionSurface );
		
	// Texture for Refraction
	if(FAILED( D3DXCreateTexture( RGetDevice(), TEXTURE_SIZE, TEXTURE_SIZE, 1, 
		D3DUSAGE_RENDERTARGET, mode.Format, D3DPOOL_DEFAULT, &mpWaterRefractionTexture)) &&
		FAILED( D3DXCreateTexture( RGetDevice(), TEXTURE_SIZE, TEXTURE_SIZE, 1, 
		D3DUSAGE_DYNAMIC, mode.Format, D3DPOOL_DEFAULT, &mpWaterRefractionTexture)))
	{
		return false;
	}

	mpWaterRefractionTexture->GetSurfaceLevel( 0, &mpWaterRefractionSurface );
	
	return true;
}



//////////////////////////////////////////////////////////////////////////
//	CopyVertices
//////////////////////////////////////////////////////////////////////////

void ZWater::CopyVertices()
{
	for( int i = 0 ; i < mNumVertices; ++i )
	{
		msCopyTemp[i].p = mVertexList[i];
	}
}


//////////////////////////////////////////////////////////////////////////
//	SetBuffer
//////////////////////////////////////////////////////////////////////////

bool ZWater::SetBuffer()
{
	void* pVertices;
	if(FAILED( mpWaterVB->Lock( 0, mNumVertices * sizeof(RVertex), (BYTE**)&pVertices, 0 ) ))
	{
		return false;
	}

	memcpy( pVertices, msCopyTemp, mNumVertices* sizeof(RVertex) );

	mpWaterVB->Unlock();

	return true;
}


//////////////////////////////////////////////////////////////////////////
//	ReflectionBegin
//////////////////////////////////////////////////////////////////////////

bool ZWater::ReflectionBegin()
{
	D3DSURFACE_DESC desc;

	if( !done )
	{
		mpWaterReflectionSurface->GetDesc( &desc );
		RGetDevice()->CreateDepthStencilSurface( desc.Width, desc.Height, D3DFMT_D16, D3DMULTISAMPLE_NONE , &stencil );
		done = true;
	}

	RGetDevice()->SetRenderTarget( mpWaterReflectionSurface, stencil );
	RGetDevice()->Clear( 0 , NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0x00000000, 1.0f, 0.0f );

	/*
	// camera 뒤집기 - view Matrix
	RGetDevice()->GetTransform( D3DTS_VIEW, &mViewHolder );

	rvector pos = RCameraPosition;
	pos.z = -pos.z + mPosZ;

	rvector dir = RCameraDirection;
	dir.z = -dir.z;
	rvector at = pos + dir;

	rvector up = RCameraUp;
	up.z = -up.z;

	rmatrix view_;
	D3DXMatrixLookAtLH( &view_, &pos, &at, &up );
	RGetDevice()->SetTransform( D3DTS_VIEW, &view_ );

	int i,j;
	for( i = 0; i < mHeight + 1; ++i )
	{
		for( j = 0; j < mWidth + 1; ++j )
		{
			rvector temp = mVertexList[i*(mWidth+1)+j]* view_ * mProjHolder;
			msCopyTemp[i*(mWidth+1)+j].tu = ((temp.x + 1) * 0.5);
			msCopyTemp[i*(mWidth+1)+j].tv = -(temp.y + 1) * 0.5;
		}
	}
	*/

	return true;
}


//////////////////////////////////////////////////////////////////////////
//	ReflectionEnd
//////////////////////////////////////////////////////////////////////////

bool ZWater::ReflectionEnd()
{
	RGetDevice()->SetRenderTarget( mpHoldBackBuffer, mpHoldStencil );
	
	RGetDevice()->SetTransform( D3DTS_VIEW, &mViewHolder );
	RGetDevice()->SetTransform( D3DTS_PROJECTION, &mProjHolder );
	RGetDevice()->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0x00000000, 1.0f, 0.0f );

	return true;
}


//////////////////////////////////////////////////////////////////////////
//	RefractionBegin
//////////////////////////////////////////////////////////////////////////

bool ZWater::RefractionBegin()
{
	D3DSURFACE_DESC desc;
	if( !done )
	{
		mpWaterReflectionSurface->GetDesc( &desc );
		RGetDevice()->CreateDepthStencilSurface( desc.Width, desc.Height, D3DFMT_D16, D3DMULTISAMPLE_NONE , &stencil );
		done = true;
	}

	RGetDevice()->GetTransform( D3DTS_VIEW, &mViewHolder );
	RGetDevice()->GetTransform( D3DTS_PROJECTION, &mProjHolder );

	RGetDevice()->SetRenderTarget( mpWaterRefractionSurface, stencil );
	RGetDevice()->Clear( 0 , NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0x00000000, 1.0f, 0.0f );

	RGetDevice()->SetTransform( D3DTS_PROJECTION, &mProjHolder );

	int i,j;
	for( i = 0; i < mHeight + 1; ++i )
	{
		for( j = 0; j < mWidth + 1; ++j )
		{
			rvector temp = mVertexList[i*(mWidth+1)+j]* mViewHolder * mProjHolder;
			msCopyTemp[i*(mWidth+1)+j].tu = (temp.x + 1) * 0.5;
			msCopyTemp[i*(mWidth+1)+j].tv = -(temp.y + 1) * 0.5;
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//	RefractionEnd
//////////////////////////////////////////////////////////////////////////

bool ZWater::RefractionEnd()
{
	RGetDevice()->SetRenderTarget( mpHoldBackBuffer, mpHoldStencil );

	RGetDevice()->SetTransform( D3DTS_VIEW, &mViewHolder );
	RGetDevice()->SetTransform( D3DTS_PROJECTION, &mProjHolder );
	RGetDevice()->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0x00000000, 1.0f, 0.0f );

	return true;
}


//////////////////////////////////////////////////////////////////////////
//	Draw
//////////////////////////////////////////////////////////////////////////

void ZWater::Draw()
{
	if( !SetBuffer() )
	{
		_ASSERT( ! "Fail to Set Vertex Buffer for Water Surface" );
	}

	rmatrix world;
	D3DXMatrixIdentity( &world );
	LPDIRECT3DDEVICE8 dev = RGetDevice();
	RGetDevice()->SetTransform( D3DTS_WORLD, &world );

	RGetDevice()->SetTexture(0,NULL);
	RGetDevice()->SetTexture(1,NULL);
	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	RGetDevice()->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	RGetDevice()->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
//*/
	
	//RGetDevice()->SetTextureStageState( D3DTEXTUREADDRESS , D3DTADDRESS_MIRROR  );

 	RGetDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
	RGetDevice()->SetRenderState( D3DRS_ZENABLE, TRUE );
	RGetDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	RGetDevice()->SetVertexShader( RVertexType );
	RGetDevice()->SetStreamSource( 0, mpWaterVB, sizeof(RVertex) );
	RGetDevice()->SetIndices( mpWaterIB, 0 );
	//RGetDevice()->SetTexture( 0 , mpWaterReflectionTexture);
	RGetDevice()->SetTexture( 0 , mpWaterRefractionTexture );
	RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
	RGetDevice()->SetRenderState( D3DRS_FILLMODE, /*D3DFILL_WIREFRAME*/ D3DFILL_SOLID );
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, false );
	
	HRESULT hr = RGetDevice()->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, mNumVertices, 0, mNumIndices / 3 );
	if( FAILED (hr))
	{
		_ASSERT(!"NANGPE");
	}
	RGetDevice()->SetRenderState( D3DRS_ZENABLE, TRUE );
	RGetDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
	RGetDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
}

void ZWater::ss()
{
	screenShot = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//	Update
//////////////////////////////////////////////////////////////////////////
bool ZWater::Update()
{
	sWave* pCurrWave;

	vector<sWave*>::iterator itor;
	for( itor = mWaveList.begin(); itor != mWaveList.end(); )
	{
		pCurrWave = *itor;
	
		switch( pCurrWave->iType ) 
		{
		
		case PERTURBATION: 
			Perturbation( pCurrWave );
			break;
		
		case RIPPLE:
			Ripple( pCurrWave );
			break;
		
		default:
			_ASSERT(! "incoming Wave Type is not Defined" );
		
		}
		
		++pCurrWave->iTimer;
		if( pCurrWave->iTimer > pCurrWave->iLimitTime )
		{
			if( pCurrWave->bRefresh )	// 다시 살리기
			{
				pCurrWave->iTimer = 0;
				pCurrWave->fRadius = 0;
				pCurrWave->fShift = 0;
				++itor;
			}
			else						// 소멸 시키기
			{
				delete *itor;
				itor = mWaveList.erase( itor );
			}
		}
		else
		{
			++pCurrWave->iTimer;
			pCurrWave->fShift += pCurrWave->fSpeed;
			pCurrWave->fRadius += pCurrWave->fSpeed * 10;
			++itor;
		}
	}
	
	CopyVertices();
//	UpdateNormal();

	return true;
}


//////////////////////////////////////////////////////////////////////////
//	Ripple
//////////////////////////////////////////////////////////////////////////

void ZWater::Ripple( sWave* pWave_ )
{
	int i;
	float radius, temp_x, temp_y;
	float Time_Elapsed = (float) pWave_->iLimitTime / (pWave_->iLimitTime + pWave_->iTimer);

	for( i = 0 ; i < mNumVertices; ++i )
	{
		temp_x = ( mVertexList[i].x - pWave_->vPos.x );
		temp_y = ( mVertexList[i].y - pWave_->vPos.y );
		radius = (float) sqrt ( (double)( temp_x * temp_x + temp_y * temp_y ) );

		if( radius < pWave_->fRadius )
		{
			mVertexList[i].z = mPosZ +  Time_Elapsed * ( pWave_->fAmplitude * (float)(sin ( pWave_->fFrequency * ( radius ) - pWave_->fShift ))) ;
		}
		else
		{
			mVertexList[i].z = mPosZ;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//	Perturbation
//////////////////////////////////////////////////////////////////////////

void ZWater::Perturbation( sWave* pWave_ )
{
	int i;
	float temp_x, temp_y;
	float shift = pWave_->fSpeed * pWave_->iTimer;

	for( i = 0 ; i < mNumVertices; ++i )
	{
		temp_x = ( mVertexList[i].x * pWave_->vPos.x );
		temp_y = ( mVertexList[i].y * pWave_->vPos.y );
		mVertexList[i].z = 
			pWave_->fAmplitude * (float)(sin ( pWave_->fFrequency * ( temp_x + temp_y ) - shift )) ;
	}
}

//////////////////////////////////////////////////////////////////////////
//	UpdateNormal
//////////////////////////////////////////////////////////////////////////

void ZWater::UpdateNormal()
{
	D3DXPLANE	plane;
	D3DXVECTOR3	vv[3];
	rvector* normal_temp = new rvector[mNumIndices / 3];

	int i = 0, j = 0;

	for( i = 0;i < mNumIndices / 3; ++i )
	{

			vv[0] = msCopyTemp[mIndexList[3*i]].p;
			vv[1] = msCopyTemp[mIndexList[3*i + 1]].p;
			vv[2] = msCopyTemp[mIndexList[3*i + 2]].p;

			D3DXPlaneFromPoints(&plane,&vv[0],&vv[1],&vv[2]);
			D3DXPlaneNormalize(&plane,&plane);

			normal_temp[i].x = plane.a;
			normal_temp[i].y = plane.b;
			normal_temp[i].z = plane.c;
	}

	///////////////////////////////////////////////////

	static int p_cnt[1000];

	memset(p_cnt,0,sizeof(int)*1000);

	for(i = 0;i < mNumIndices / 3;i++) {
		
		for(j = 0;j < 3; j++) {
			msCopyTemp[ mIndexList[3*i + j] ].n =
				msCopyTemp[ mIndexList[3*i + j] ].n + normal_temp[i];

			p_cnt[ mIndexList[3*i + j] ]++;
		}
	}

	for(i=0;i<mNumVertices;i++) {
		msCopyTemp[i].n = msCopyTemp[i].n / (float)p_cnt[i];
		D3DXVec3Normalize(&msCopyTemp[i].n,&msCopyTemp[i].n);
	}

	delete[] normal_temp;
}