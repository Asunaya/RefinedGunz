#include "stdafx.h"
#include "RParticleSystem.h"
#include "RealSpace2.h"
//#include "ZWater.h"
#include "MDebug.h"

_USING_NAMESPACE_REALSPACE2
_NAMESPACE_REALSPACE2_BEGIN

const DWORD POINTVERTEX::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

LPDIRECT3DVERTEXBUFFER9 RParticleSystem::m_pVB=NULL;
DWORD RParticleSystem::m_dwBase=DISCARD_COUNT;

// 이 virtual 함수에서 움직임을 결정한다. 반환값이 false이면 해당 파티클제거
bool RParticle::Update(float fTimeElapsed)
{
	// TODO : 시간간격이 커짐에 따라 부정확하다
	velocity+=accel*fTimeElapsed;
	position+=velocity*fTimeElapsed;
	ftime+=fTimeElapsed;

	return true;
}

RParticles::RParticles()
{
	m_Texture = 0;
}

RParticles::~RParticles()
{
	Destroy();
}

bool RParticles::Create(const char *szTextureName,float fSize)
{
	m_Texture = RCreateBaseTexture( szTextureName );
	m_fSize=fSize;

	if( m_Texture == 0 )
	{
		return false;
	}
	//*/
	return true;
}

void RParticles::Destroy()
{
	RDestroyBaseTexture(m_Texture);
	Clear();
}

void RParticles::Clear()
{
    while(size())
	{
		delete *begin();
		erase(begin());
    }
}


inline DWORD FtoDW( FLOAT f ) { return *((DWORD*)&f); }

#define LIFETIME	500.f
bool RParticles::Draw()
{
 	if(size()==0)
		return true;

	LPDIRECT3DDEVICE9 pd3dDevice=RGetDevice();

	HRESULT hr;

	pd3dDevice->SetRenderState( D3DRS_POINTSIZE,     FtoDW(m_fSize) );

	pd3dDevice->SetTexture(0,m_Texture->GetTexture());

	POINTVERTEX* pVertices;
	DWORD        dwNumParticlesToRender = 0;

	RParticleSystem::m_dwBase += FLUSH_COUNT;

	if(RParticleSystem::m_dwBase >= DISCARD_COUNT)
		RParticleSystem::m_dwBase = 0;

	if( FAILED( RParticleSystem::m_pVB->Lock( RParticleSystem::m_dwBase * sizeof(POINTVERTEX), FLUSH_COUNT * sizeof(POINTVERTEX),
		(VOID**) &pVertices, RParticleSystem::m_dwBase ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD ) ) )
	{
		return false;
	}


	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
//*/

	for(iterator i=begin();i!=end();i++)
	{
		RParticle*   pParticle=*i;
		D3DXVECTOR3 vPos(pParticle->position);

		D3DXVECTOR3 vVel(pParticle->velocity);
		FLOAT       fLength = Magnitude(vVel);
		UINT        dwSteps;

		//dwSteps=min((int)(fLength/20.f),4);
		//vVel*=(.05f/dwSteps);

		dwSteps = 1;


		// Render each particle a bunch of times to get a blurring effect
		for( DWORD j = 0; j < dwSteps; j++ )
		{
			pVertices->v     = vPos;
            if(!isInViewFrustum(vPos,m_fSize,RGetViewFrustum())) continue;	// viewfrustum culling

			static D3DXCOLOR czero=D3DXCOLOR(0,0,0,0),cone=D3DXCOLOR(1,1,1,1);

			D3DXCOLOR color;
			D3DXColorLerp( &color, &cone, &czero, pParticle->ftime/LIFETIME );

			pVertices->color = color;
			pVertices++;

			if( ++dwNumParticlesToRender == FLUSH_COUNT)
			{
				RParticleSystem::m_pVB->Unlock();

				if(FAILED( hr = pd3dDevice->DrawPrimitive( D3DPT_POINTLIST, RParticleSystem::m_dwBase, dwNumParticlesToRender)))
					return false;

				RParticleSystem::m_dwBase += FLUSH_COUNT;

				if(RParticleSystem::m_dwBase >= DISCARD_COUNT)
					RParticleSystem::m_dwBase = 0;

				if( FAILED( hr = RParticleSystem::m_pVB->Lock( RParticleSystem::m_dwBase * sizeof(POINTVERTEX), FLUSH_COUNT * sizeof(POINTVERTEX),
					(VOID**) &pVertices, RParticleSystem::m_dwBase ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD ) ) )
				{
					return false;
				}

				dwNumParticlesToRender = 0;
			}
			vPos += vVel;
		}
	}

	// Unlock the vertex buffer
	RParticleSystem::m_pVB->Unlock();

	// Render any remaining particles
	if( dwNumParticlesToRender )
	{
		if(FAILED(hr = pd3dDevice->DrawPrimitive( D3DPT_POINTLIST, RParticleSystem::m_dwBase, dwNumParticlesToRender )))
			return false;
	}

	return true;
}

bool RParticles::Update(float fTime)
{
	for(iterator i=begin();i!=end();)
	{
		RParticle *pp=*i;
		if ( (pp->ftime > LIFETIME ) || (pp->Update(fTime) == false) )
		{
			delete pp;
			iterator j=i;
			i++;
			erase(j);
		}
		else
		{
            i++;
		}
	}
	return true;
}

RParticleSystem::RParticleSystem()
{
}

RParticleSystem::~RParticleSystem()
{
	Destroy();
}

void RParticleSystem::Destroy()
{
	while(size())
	{
		delete *begin();
		erase(begin());
	}

	Invalidate();
}

bool RParticleSystem::Restore()
{
	LPDIRECT3DDEVICE9 pd3dDevice=RGetDevice();

	HRESULT hr;
	if(FAILED(hr = pd3dDevice->CreateVertexBuffer( DISCARD_COUNT * 
		sizeof(POINTVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_POINTS, 
		POINTVERTEX::FVF, D3DPOOL_DEFAULT, &RParticleSystem::m_pVB ,NULL)))
	{
		return false;
	}

	RParticleSystem::m_dwBase=0;
	return true;
}

bool RParticleSystem::Invalidate()
{
	SAFE_RELEASE( RParticleSystem::m_pVB );
	return true;
}

void RParticleSystem::BeginState()
{
	LPDIRECT3DDEVICE9 pd3dDevice=RGetDevice();

	// Set the render states for using point sprites
	pd3dDevice->SetRenderState( D3DRS_POINTSPRITEENABLE, TRUE );
	pd3dDevice->SetRenderState( D3DRS_POINTSCALEENABLE,  TRUE );
	pd3dDevice->SetRenderState( D3DRS_POINTSIZE_MIN, FtoDW(0.00f) );
	pd3dDevice->SetRenderState( D3DRS_POINTSCALE_A,  FtoDW(0.00f) );
	pd3dDevice->SetRenderState( D3DRS_POINTSCALE_B,  FtoDW(0.00f) );
	pd3dDevice->SetRenderState( D3DRS_POINTSCALE_C,  FtoDW(1.00f) );

	pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
	pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
	pd3dDevice->SetRenderState( D3DRS_LIGHTING,  FALSE );
	pd3dDevice->SetRenderState( D3DRS_CULLMODE,  D3DCULL_NONE );
	pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );

	pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE);


	// Set up the vertex buffer to be rendered
	pd3dDevice->SetStreamSource( 0, RParticleSystem::m_pVB, 0, sizeof(POINTVERTEX) );
	pd3dDevice->SetFVF( POINTVERTEX::FVF );


	// 월드매트릭스 초기화
	rmatrix World;
	D3DXMatrixIdentity(&World);
	RGetDevice()->SetTransform(D3DTS_WORLD, &World);
}

void RParticleSystem::EndState()
{
	LPDIRECT3DDEVICE9 pd3dDevice=RGetDevice();
	// Reset render states
	pd3dDevice->SetRenderState( D3DRS_POINTSPRITEENABLE, FALSE );
	pd3dDevice->SetRenderState( D3DRS_POINTSCALEENABLE,  FALSE );
	pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE);
	pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );

	// 이렇게 해줘야 RParticleSystem::m_pVB 의 ref count 가 줄어든다..
	// 아니면 release 불가..
	pd3dDevice->SetStreamSource( 0, NULL , 0, 0 );	
}

bool RParticleSystem::Draw()
{
	BeginState();
	for(iterator i=begin();i!=end();i++)
	{
		RParticles *pParticles=*i;
		pParticles->Draw();
	}
	EndState();
	return true;
}

bool RParticleSystem::Update(float fTime)
{
	for(iterator i=begin();i!=end();i++)
	{
		RParticles *pParticles=*i;
		pParticles->Update(fTime);
	}

	return true;
}

RParticles *RParticleSystem::AddParticles(const char *szTextureName,float fSize)
{
	RParticles *pp=new RParticles;
	if( !(pp->Create(szTextureName,fSize)) )
	{
		return NULL ;
	}
	push_back(pp);
	
	return pp;
}

_NAMESPACE_REALSPACE2_END

/*
//////////////////////////////////////////////////////////////////////////
//	ZWaterSplash -> Draw
//////////////////////////////////////////////////////////////////////////
bool ZWaterSplash::Draw()
{
	if(	0 == size() )
	{
		return true;
	}

	HRESULT hr;
	LPDIRECT3DDEVICE9 pd3dDevice	= RGetDevice();

	pd3dDevice->SetRenderState( D3DRS_POINTSIZE, FtoDW(m_fSize));
	pd3dDevice->SetTexture( 0, m_Texture->GetTexture() );

	POINTVERTEX* pVertices;
	DWORD        dwNumParticlesToRender = 0;

	RParticleSystem::m_dwBase += FLUSH_COUNT;

	if(RParticleSystem::m_dwBase >= DISCARD_COUNT)
	{
		RParticleSystem::m_dwBase = 0;
	}

	if( FAILED( RParticleSystem::m_pVB->Lock( RParticleSystem::m_dwBase * sizeof(POINTVERTEX), FLUSH_COUNT * sizeof(POINTVERTEX),
		(VOID**) &pVertices, RParticleSystem::m_dwBase ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD ) ) )
	{
		return false;
	}

	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

	for( iterator i = begin(); i != end(); ++i ) 
	{
		RParticle*		pParticle	= *i;
		D3DXVECTOR3		vPos(pParticle->position);

		D3DXVECTOR3 vVel(pParticle->velocity);
		FLOAT       fLength = Magnitude(vVel);
		UINT        dwSteps;

		dwSteps		= min((int)(fLength/20.f),4);
		vVel		*= (.05f/dwSteps);

		dwSteps = 1;

		// Render each particle a bunch of times to get a blurring effect
		for( DWORD j = 0; j < dwSteps; j++ )
		{
			pVertices->v     = vPos;

			static D3DXCOLOR czero=D3DXCOLOR(0,0,0,0),cone=D3DXCOLOR(1,1,1,1);

			D3DXCOLOR color;
			float height	= fabs(vPos.z - mfDeadLine);
#define	MAX_SPLASH_HEIGHT	300	// 사람키의 2배 높이

			if( pParticle->velocity.z > 0 )
			{
				D3DXColorLerp( &color, &cone, &czero, height / MAX_SPLASH_HEIGHT * 0.5  );
			}
			else
			{
				D3DXColorLerp( &color, &cone, &czero, MAX_SPLASH_HEIGHT / (MAX_SPLASH_HEIGHT + height) );
			}

			pVertices->color = color;
			pVertices++;

			if( ++dwNumParticlesToRender == FLUSH_COUNT)
			{
				RParticleSystem::m_pVB->Unlock();

				if(FAILED( hr = pd3dDevice->DrawPrimitive( D3DPT_POINTLIST, RParticleSystem::m_dwBase, dwNumParticlesToRender)))
					return false;

				RParticleSystem::m_dwBase += FLUSH_COUNT;

				if(RParticleSystem::m_dwBase >= DISCARD_COUNT)
					RParticleSystem::m_dwBase = 0;

				if( FAILED( hr = RParticleSystem::m_pVB->Lock( RParticleSystem::m_dwBase * sizeof(POINTVERTEX), FLUSH_COUNT * sizeof(POINTVERTEX),
					(VOID**) &pVertices, RParticleSystem::m_dwBase ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD ) ) )
				{
					return false;
				}

				dwNumParticlesToRender = 0;
			}
			vPos += vVel;
		}
	}

	// Unlock the vertex buffer
	RParticleSystem::m_pVB->Unlock();

	// Render any remaining particles
	if( dwNumParticlesToRender )
	{
		if(FAILED(hr = pd3dDevice->DrawPrimitive( D3DPT_POINTLIST, RParticleSystem::m_dwBase, dwNumParticlesToRender )))
			return false;
	}

	return true;
}
//*/