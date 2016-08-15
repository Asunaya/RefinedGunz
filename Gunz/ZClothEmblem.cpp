#include "stdafx.h"

#include "ZGame.h"
#include "ZClothEmblem.h"
#include "RealSpace2.h"
#include "RLightList.h"
#include "RDynamicLight.h"
#include "MDebug.h"


//////////////////////////////////////////////////////////////////////////
//	Define
//////////////////////////////////////////////////////////////////////////
#define Gravity							-7
#define MAX_NUM_CLOTH_PARTICLE			165 //(165*3)
#define LIGHT_DISTANCE					100
#define RESERVED_SPACE					10

//////////////////////////////////////////////////////////////////////////
//	Global
//////////////////////////////////////////////////////////////////////////
RVertex					g_Cloth_Buffer[MAX_NUM_CLOTH_PARTICLE*3];
LPDIRECT3DVERTEXBUFFER9	g_hw_Buffer			= 0;
unsigned int			ZClothEmblem::msRef = 0;
//USHORT					g_index_buffer[MAX_NUM_CLOTH_PARTICLE*3];

namespace 
{
	bool bHardwareBuffer = true;
}

struct testv
{
	rvector n;
	DWORD c;
};

#define testvFVF (D3DFVF_XYZ|D3DFVF_DIFFUSE)

//////////////////////////////////////////////////////////////////////////
//	CreateFromMeshNode
//////////////////////////////////////////////////////////////////////////
void	ZClothEmblem::CreateFromMeshNode( RMeshNode* pMeshNdoe_ , ZWorld* pWorld)
{
	int			i, nIndices;
	rvector		vecDistance;

	mpMeshNode	= pMeshNdoe_;

	m_nCntP	= mpMeshNode->m_point_num;
	m_nCntC	= mpMeshNode->m_face_num * 3 ;

	// Initialize
	m_pX		= new rvector[m_nCntP];
	m_pOldX		= new rvector[m_nCntP];
	m_pForce	= new rvector[m_nCntP];
	m_pHolds	= new int    [m_nCntP];
	m_pWeights	= new float  [m_nCntP];
	m_pNormal	= new rvector[m_nCntP];

	m_pConst	= new sConstraint[m_nCntC];

	memset( m_pForce    , 0, sizeof(rvector)* m_nCntP );
	memset( m_pHolds    , 0, sizeof(bool)   * m_nCntP );
	memset( m_pWeights  , 0, sizeof(float)  * m_nCntP );

	nIndices = pMeshNdoe_->m_face_num*3;

	//if( bHardwareBuffer )
	//{
	//	SAFE_RELEASE(mIndexBuffer);
	//	if( FAILED( RGetDevice()->CreateIndexBuffer(nIndices*sizeof(USHORT),0,D3DFMT_INDEX16,D3DPOOL_MANAGED,&mIndexBuffer)))
	//	{
	//		mlog( "Fail to Restore Index Buffer for Emblems..\n" );
	//		bHardwareBuffer = false;
	//	}
	//}

	rmatrix World;
	rvector Pos = mpMeshNode->m_mat_base;
	rvector Dir = rvector(0,-1,0);
	rvector Up  = rvector(0,0,1);

	MakeWorldMatrix(&World, Pos, Dir, Up);

	mWorldMat = mpMeshNode->m_mat_result * World;

	D3DXVECTOR4 rVec;
	for( i = 0 ; i < mpMeshNode->m_point_num; ++i )
	{
		D3DXVec3Transform( &rVec, &mpMeshNode->m_point_list[i], &mWorldMat );
		mpMeshNode->m_point_list[i] = (rvector)rVec;
	}

	//	Copy Vertex
	memcpy( m_pX, mpMeshNode->m_point_list, sizeof(rvector) * m_nCntP );
	memcpy( m_pOldX, mpMeshNode->m_point_list, sizeof(rvector) * m_nCntP );

	//	Build Constraints
	for( i = 0 ; i < mpMeshNode->m_face_num; ++i )
	{
		for( int j = 0 ; j < 3; ++j )
		{
			m_pConst[ i*3 + j ].refA = mpMeshNode->m_face_list[i].m_point_index[j];
			if( j + 1 >= 3 )
			{
				m_pConst[ i*3 + j ].refB = mpMeshNode->m_face_list[i].m_point_index[0];
			}
			else
			{
				m_pConst[ i*3 + j ].refB = mpMeshNode->m_face_list[i].m_point_index[j+1];
			}
			vecDistance = mpMeshNode->m_point_list[m_pConst[ i*3 + j ].refA] - mpMeshNode->m_point_list[m_pConst[ i*3 + j ].refB];
			m_pConst[ i*3 + j ].restLength = D3DXVec3Length(&vecDistance);
			
			//g_index_buffer[i*3+j]	= (USHORT)m_pConst[i*3+j].refA;
		}
	}

//	if( bHardwareBuffer )
//	{
//		void* pIndices;
//		if(FAILED( mIndexBuffer->Lock(0,sizeof(USHORT)*nIndices,(VOID**)&pIndices,0)))
//		{
//			bHardwareBuffer = false;
//			mlog("Fail to Lock Index Buffer\n");
//		}
//		else
//		{
//			memcpy( pIndices, g_index_buffer, sizeof(USHORT)*nIndices);
//			mIndexBuffer->Unlock();
//		}
//		
////		delete uIndexList;
//	}

	_ASSERT( mpMeshNode->m_point_color_num );

	for( i = 0 ; i < m_nCntP; ++i )
	{
		m_pHolds[i]	= CLOTH_VALET_ONLY;
		// 만약 붉은색 성분이 있으면 & Exclusive with other Attribute...
		if( mpMeshNode->m_point_color_list[i].x != 0 )
		{
			m_pHolds[i]	= CLOTH_HOLD;
		}
		else 
		{
			//	만약 파란색 성분이 있으면 : 힘!
			//if( mpMeshNode->m_point_color_list[i].z != 0 )
			{
				m_pHolds[i] |= CLOTH_FORCE;
			}
			//	만약 녹색 성분이 있으면 : 충돌!
			if( mpMeshNode->m_point_color_list[i].y != 0 )
			{
				m_pHolds[i] |= CLOTH_COLLISION;
			}
		}
	}

	m_pWorld = pWorld;

	float	minDistance = 999999;
	float	fTemp;
	RLIGHT	*pSelectedLight = nullptr;
	
	for(auto& Light : m_pWorld->GetBsp()->GetObjectLightList())
	{
		fTemp	= D3DXVec3Length( &(Light.Position - m_pX[0]) );
		if( fTemp < minDistance )
		{
			minDistance		= fTemp;
			pSelectedLight	= &Light;
		}
	}

	mpLight	= new D3DLIGHT9;
	memset( mpLight, 0, sizeof(D3DLIGHT9));

	mpLight->Ambient.r = 0.3;
	mpLight->Ambient.g = 0.3;
	mpLight->Ambient.b = 0.3;

	if( pSelectedLight!=0 &&  minDistance < pSelectedLight->fAttnEnd	)
	{	
		mpLight->Type		= D3DLIGHT_POINT;

		mpLight->Diffuse.r	= pSelectedLight->Color.x * pSelectedLight->fIntensity;
		mpLight->Diffuse.g	= pSelectedLight->Color.y * pSelectedLight->fIntensity;
		mpLight->Diffuse.b	= pSelectedLight->Color.z * pSelectedLight->fIntensity;

		mpLight->Position	= pSelectedLight->Position;

		mpLight->Range		= pSelectedLight->fAttnEnd;
		mpLight->Attenuation1	= 0.0001f;
	}
	else	// 가까운 곳에 빛이 없으면 기본 라이팅 적용
	{
		mpLight->Type		= D3DLIGHT_DIRECTIONAL;
		
		mpLight->Diffuse.r	= 0.1f;
		mpLight->Diffuse.g	= 0.1f;
		mpLight->Diffuse.b	= 0.1f;

		mpLight->Direction	= rvector( 1, 1, 1 );
	
		mpLight->Attenuation1	= 0.0f;
		mpLight->Attenuation0	= 0.0f;	

		mpLight->Range		= 0.0f;
	}

	//PreCalculate AABB
	float	mostSmallX, mostSmallY, mostSmallZ;
	float	mostBigX, mostBigY, mostBigZ;

	mostSmallX	= mostSmallY	= mostSmallZ	= 9999999;
	mostBigX	= mostBigY		= mostBigZ		= -9999999;

	rvector* pCurr;
	for( i = 0 ; i < m_nCntP; ++i )
	{
		pCurr	= &mpMeshNode->m_point_list[i];
		
		mostSmallX	= min( mostSmallX, pCurr->x );
		mostSmallY	= min( mostSmallY, pCurr->y );
		mostSmallZ	= min( mostSmallZ, pCurr->z );

		mostBigX	= max( mostBigX, pCurr->x );
		mostBigY	= max( mostBigY, pCurr->y );
		mostBigZ	= max( mostBigZ, pCurr->z );
	}

	mAABB.vmin = rvector( mostSmallX - RESERVED_SPACE, mostSmallY - RESERVED_SPACE, mostSmallZ - RESERVED_SPACE );
	mAABB.vmax = rvector( mostBigX + RESERVED_SPACE, mostBigY + RESERVED_SPACE, mostBigZ + RESERVED_SPACE );

	mMyTime	= 0;
}

//////////////////////////////////////////////////////////////////////////
//	setOption
//////////////////////////////////////////////////////////////////////////
void ZClothEmblem::setOption( int nIter_, float power_, float inertia_ )
{
	m_nCntIter = nIter_;
	m_fTimeStep = power_;
	m_AccelationRatio = inertia_;
}

//////////////////////////////////////////////////////////////////////////
//	update
//////////////////////////////////////////////////////////////////////////
void ZClothEmblem::update()
{
	if( !isInViewFrustum( &mAABB, RGetViewFrustum() )  || 
		!m_pWorld->GetBsp()->IsVisible(mAABB) ) {
			mbIsInFrustrum = false;
			return;
		}

	DWORD currTime = GetGlobalTimeMS();
	if ( mMyTime - currTime < 33 )			// 초당 30번으로 제한
	{
		return;
	}
	mMyTime = GetGlobalTimeMS();

 	accumulateForces();
	varlet();
	memset( m_pForce, 0, sizeof(rvector)*m_nCntP );
	//memset( mpWind, 0, sizeof(rvector) );
	if(mpWind!=NULL) 
	{
		mpWind->x = 0.f;
		mpWind->y = 0.f;
		mpWind->z = 0.f;
	}
	satisfyConstraints();
	mWndGenerator.Update( GetGlobalTimeMS() );
	mbIsInFrustrum = true; // 다음 루프에서 시뮬레이션 대상에 추가한다
}

//////////////////////////////////////////////////////////////////////////
//	accumulateForces
//////////////////////////////////////////////////////////////////////////
void ZClothEmblem::accumulateForces()
{
	if(	mpWind != NULL )
		*mpWind += mWndGenerator.GetWind();
}

//////////////////////////////////////////////////////////////////////////
//	varlet
//////////////////////////////////////////////////////////////////////////
void ZClothEmblem::varlet()
{
	for( int i = 0 ; i < m_nCntP; ++i )
	{
		if( m_pHolds[i] & CLOTH_HOLD )
		{
			continue;
		}
		else
		{
			rvector force;
			if( m_pHolds[i] & CLOTH_FORCE && mpWind != NULL )
			{
				force	= m_pForce[i] + (*mpWind );
				force.z = Gravity;
			}
			else
			{
				force = m_pForce[i];
				force.z		+= Gravity;
			}
			m_pOldX[i]	= m_pX[i] + m_AccelationRatio * ( m_pX[i] - m_pOldX[i] ) + force * m_fTimeStep * m_fTimeStep; 
		}
	}	

	rvector* swapTemp;

	swapTemp	= m_pX;
	m_pX		= m_pOldX;
	m_pOldX		= swapTemp;
}

//////////////////////////////////////////////////////////////////////////
//	satisfyConstraints
//////////////////////////////////////////////////////////////////////////
void ZClothEmblem::satisfyConstraints()
{	
	sConstraint*	c;
	rvector*		x1;
	rvector*		x2;
	rvector			delta;
	float			deltaLegth;
	float			diff;
	int				i,j;

//	if( mbIsInFrustrum )
	{
		for( i = 0 ; i < m_nCntIter; ++i )
		{
			// 캐릭터와 충돌 체크
			for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();
				itor != ZGetCharacterManager()->end(); ++itor)
			{
				ZCharacter*	pCharacter	= (*itor).second;

				// 현재 화면에 보이는 캐릭터만 대상으로 체크.. 뷰프러스텀 컬링
				if( !isInViewFrustum( pCharacter->m_Position, CHARACTER_RADIUS + 10, RGetViewFrustum() ) )
				{
					continue;
				}
				
				if( pCharacter->IsDie() && pCharacter->m_bBlastDrop && !pCharacter->IsVisible() )
				{
					continue;
				}

				for( j = 0 ; j < m_nCntP; ++j )
				{
 					rvector	pos		= pCharacter->m_Position;
					rvector myPos	= m_pX[j];

					if( pos.z + 190 < myPos.z || pos.z > myPos.z )
					{
						continue;
					}
					
					pos.z	= 0;
					myPos.z	= 0;

					rvector dir		= myPos - pos;
					
					float lengthsq	= D3DXVec3LengthSq( &dir );
					if( lengthsq > CHARACTER_RADIUS*CHARACTER_RADIUS )
					{
						continue;
					}
	                
					D3DXVec3Normalize( &dir, &dir );

					myPos		= pos + dir * ( CHARACTER_RADIUS );
					m_pX[j].x		= myPos.x;
					m_pX[j].y		= myPos.y;

					//break;
				}
			}
		}


		// Restriction
		for( list<sRestriction*>::iterator itor = mRestrictionList.begin();
			itor != mRestrictionList.end(); ++itor )
		{
			for( int j = 0 ; j < m_nCntP; ++j )
			{
				float* p		= (float*)&m_pX[j];
				sRestriction* r = *itor;
                
				p += (int)r->axis; // 축결정
				if( r->compare == COMPARE_GREATER)
				{
					if( *p > r->position )
					{
						*p = r->position;
					}
				}
				else
				{
					if( *p < r->position -3)
					{
						*p = r->position;
					}
				}
			}
			
		}
		
		

		// Relaxation
		for( j = 0 ; j < m_nCntC; ++j )
		{
			c	= &m_pConst[j];

			x1	= &m_pX[ c->refA ];
			x2	= &m_pX[ c->refB ];

			delta = *x2 - *x1;
			deltaLegth = D3DXVec3Length( &delta );
			diff = (float) ( ( deltaLegth - c->restLength ) / (deltaLegth ));

			*x1		+= delta * diff * 0.5;
			*x2		-= delta * diff * 0.5; 
		}
	}

	for( i = 0 ; i < m_nCntP; ++i )
	{
		if( m_pHolds[i] & CLOTH_HOLD )
		{
			m_pX[i]	= m_pOldX[i];
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//	Render
//////////////////////////////////////////////////////////////////////////
void ZClothEmblem::render()
{
 //	if( !isInViewFrustum( &mAABB, RGetViewFrustum() ) )	
	//{
	//	return;
	//}
//	if(g_pGame==NULL || !ZGetGame()->GetWorld()->GetBsp()->IsVisible(mAABB)) return;

	if( !mbIsInFrustrum ) return;

	int		i, index;

	UpdateNormal();

	//memset(g_Cloth_Buffer,0, sizeof(RVertex)*MAX_NUM_CLOTH_PARTICLE*3 );

	//for( i = 0 ; i < mpMeshNode->m_point_num; ++i )
	//{
	//	g_Cloth_Buffer[i].p	= m_pX[i];
	//	g_Cloth_Buffer[i].n	= m_pNormal[i];
	//}

 //   for( i = 0 ; i < mpMeshNode->m_face_num; ++i )
	//{
	//	for( int j = 0 ; j < 3; ++j )
	//	{
	//		index	= mpMeshNode->m_face_list[i].m_point_index[j];
	//		g_Cloth_Buffer[index].tu	= mpMeshNode->m_face_list[i].m_point_tex[j].x;
	//		g_Cloth_Buffer[index].tv	= mpMeshNode->m_face_list[i].m_point_tex[j].y;
	//		g_index_buffer[i*3+j] = index;
	//	}
	//}
	for( i = 0 ; i < mpMeshNode->m_face_num; ++i )
	{
		for( int j = 0 ; j < 3; ++j )
		{
			index	= mpMeshNode->m_face_list[i].m_point_index[j];
			g_Cloth_Buffer[3*i+j].p	= m_pX[index];
			g_Cloth_Buffer[3*i+j].n	= m_pNormal[index];
			g_Cloth_Buffer[3*i+j].tu = mpMeshNode->m_face_list[i].m_point_tex[j].x;
			g_Cloth_Buffer[3*i+j].tv = mpMeshNode->m_face_list[i].m_point_tex[j].y;
//			g_index_buffer[i*3+j] = index;
		}
	}

	D3DMATERIAL9	mtrl;
	mtrl.Ambient.r	= 1.f;	mtrl.Ambient.g	= 1.f;	mtrl.Ambient.b	= 1.f;	mtrl.Ambient.a	= 0.f;	
	mtrl.Diffuse.r	= 1.0f;	mtrl.Diffuse.g	= 1.0f;	mtrl.Diffuse.b	= 1.0f;	mtrl.Diffuse.a	= 1.f;
	mtrl.Specular.r	= 0.f;	mtrl.Specular.g	= 0.f;	mtrl.Specular.b	= 0.f;	mtrl.Specular.a	= 0.f;
	mtrl.Emissive.r	= 0.f;	mtrl.Emissive.g	= 0.f;	mtrl.Emissive.b	= 0.f;	mtrl.Emissive.a	= 0.f;
	mtrl.Power	= 0.0f;
	RGetDevice()->SetMaterial( &mtrl );

	RMtrlMgr* pMtrlMgr	= &mpMeshNode->m_pParentMesh->m_mtrl_list_ex;
	RMtrl* pMtrl		= pMtrlMgr->Get_s(mpMeshNode->m_mtrl_id,-1);
	RGetDevice()->SetTexture( 0, pMtrl->GetTexture() );
	RGetDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	RGetDevice()->SetRenderState( D3DRS_SPECULARENABLE, FALSE );

	RGetDevice()->SetRenderState( D3DRS_AMBIENT, 0x00555555 );
	RGetShaderMgr()->setAmbient( 0x00555555 );

	RGetDevice()->SetRenderState( D3DRS_LIGHTING, TRUE );
	RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	RGetDevice()->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	RGetDevice()->SetLight( 0, mpLight );
	RGetDynamicLightManager()->SetLight( m_pX[0], 1, LIGHT_DISTANCE );

	rmatrix Identity;
	D3DXMatrixIdentity( &Identity );
	RGetDevice()->SetTransform( D3DTS_WORLD, &Identity );

	RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
 	RGetDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	RGetDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	RGetDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );

	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );

	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

	RGetDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	RGetDevice()->LightEnable( 0, TRUE );
	RGetDevice()->LightEnable( 1, FALSE );

	RGetDevice()->SetFVF( RVertexType );
	//RGetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME );
	
	if( bHardwareBuffer )
	{
		// TODO: 무조건 DISCARD 로 lock할것이 아니라, nooverwrite 로 적절히 낭비되지않도록해주자.
		VOID* pVertex;
		if( FAILED( g_hw_Buffer->Lock( 0,  mpMeshNode->m_point_num * 3 * sizeof(RVertex), (VOID**)&pVertex, D3DLOCK_DISCARD )))
		{
			mlog(" Fail to Lock Emblem hw buffer.. Check Buffer Size.. \n" );
			bHardwareBuffer = false;
			return;
		}
		memcpy( pVertex, g_Cloth_Buffer, mpMeshNode->m_face_num*3 * sizeof(RVertex) );
		if( FAILED( g_hw_Buffer->Unlock() ))
		{
			mlog(" Fail to unLock Emblem hw buffer.. Check Buffer Size.. \n" );
			bHardwareBuffer = false;
			return;
		}

		RGetDevice()->SetStreamSource( 0, g_hw_Buffer, 0, sizeof(RVertex) );
		//RGetDevice()->SetIndices( mIndexBuffer);
		//RGetDevice()->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, mpMeshNode->m_point_num, 0, mpMeshNode->m_face_num );
		RGetDevice()->DrawPrimitive(D3DPT_TRIANGLELIST,0,mpMeshNode->m_face_num);
	}
	else
	{
 		//RGetDevice()->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, mpMeshNode->m_point_num, mpMeshNode->m_face_num, g_index_buffer, D3DFMT_INDEX16, g_Cloth_Buffer, sizeof(RVertex) );
		RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, mpMeshNode->m_face_num, g_Cloth_Buffer, sizeof(RVertex) );
	}	

	RGetDevice()->LightEnable( 0, FALSE );
	RGetDevice()->LightEnable( 1, FALSE );
	
	//{
	//	static testv v[1000];
	//	for( int i = 0; i < m_nCntP; ++i )
	//	{
	//		v[2*i].n = m_pX[i];
	//		v[2*i].c	= 0xffffffff;
	//		v[2*i + 1].n = m_pX[i] + mpNormal[i] * 50.f;
	//		v[2*i + 1].c	= 0xffffffff;
	//	}
	//	RGetDevice()->SetFVF( testvFVF );
	//	RGetDevice()->DrawPrimitiveUP( D3DPT_LINELIST, m_nCntP, v, sizeof(testv) );
	//}
}

//////////////////////////////////////////////////////////////////////////
//	UpdateNormal
//////////////////////////////////////////////////////////////////////////
void ZClothEmblem::UpdateNormal()
{
	int	i, j, index, indexTemp[3];
	//static rplane	planeTemp; 
	//rvector* pPoint[3];
	rvector Point[3];

	// 초기화
	memset( m_pNormal, 0, sizeof(rvector)*m_nCntP );
	memset( indexTemp, 0, sizeof(int)*3 );

	for( i = 0 ; i < mpMeshNode->m_face_num; ++i )
	{
		for( j = 0 ; j < 3; ++j )
		{
			index		= mpMeshNode->m_face_list[i].m_point_index[j];
			//Debug
			if( index < 0 || index >= m_nCntP )
			{
				_ASSERT(FALSE);
				mlog("Index of Particle is not profit to calculate...\n");
				continue;
			}
			//End Debug
			//pPoint[j]	= &m_pX[index];
			Point[j]			= m_pX[index];
			indexTemp[j]= index;
		}
		rvector n;
		//D3DXVec3Cross( &n, &(*pPoint[2] - *pPoint[0]), &(*pPoint[2] - *pPoint[1]) );
		D3DXVec3Cross( &n, &(Point[2] - Point[0]), &(Point[2] - Point[1] ));
		D3DXVec3Normalize(&n,&n);

		for( j = 0 ; j < 3; ++j )
		{
			m_pNormal[indexTemp[j]] += n;
		}
	}

	for( i = 0 ; i < m_nCntP; ++i )
	{
		D3DXVec3Normalize( &m_pNormal[i], &m_pNormal[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
//	SetExplosion
//////////////////////////////////////////////////////////////////////////
void ZClothEmblem::setExplosion( rvector& pos_, float power_ )
{
	rvector	dir		= m_pX[0] - pos_;
	float lengthsq	= D3DXVec3LengthSq( &dir );
	if( lengthsq	 > 250000 )	// 5미터 안의 것들만 영향을 받음..
	{
		return;
	}

	D3DXVec3Normalize( &dir, &dir );
	*mpWind	+= dir * power_ / sqrt(lengthsq) * 10;
}

//////////////////////////////////////////////////////////////////////////
//	CheckSpearing
//////////////////////////////////////////////////////////////////////////
void ZClothEmblem::CheckSpearing( rvector& bullet_begin_, rvector& bullet_end_, float power_ )
{	
	if(mbIsInFrustrum)
	{
		if(!isInViewFrustum( &mAABB, RGetViewFrustum())) 
			mbIsInFrustrum =false;
	}
	else return;

	rvector dir = bullet_end_ - bullet_begin_;
	D3DXVec3Normalize( &dir, &dir );

	// test line vs AABB
	if( !D3DXBoxBoundProbe( &mAABB.vmin , &mAABB.vmax,	&bullet_begin_, &dir ) )
	{
		return;
	}

	// line vs triangle test and determine which particle get power
	int index[3], sIndex = -1;
	rvector uvt;
	rvector* v[3];

	for( int i = 0 ; i < mpMeshNode->m_face_num; ++i )
	{
        for( int j = 0 ; j < 3; ++j )
		{
			index[j]= mpMeshNode->m_face_list[i].m_point_index[j];
			v[j]	= &m_pX[index[j]];
		}

		if( D3DXIntersectTri( v[0], v[1], v[2], &bullet_begin_, &dir, &uvt.x, &uvt.y, &uvt.z ) ) 
		{
			if( uvt.x + uvt.y < 0.66 )
			{
				sIndex	= index[2];
			}
			else if( uvt.x > uvt.y )
			{
				sIndex	= index[0];
			}
			else
			{
				sIndex	= index[1];
			}

			m_pForce[sIndex]	+= dir * power_;
			break;
		}
	}	
}

//////////////////////////////////////////////////////////////////////////
//	OnInvalidate
//////////////////////////////////////////////////////////////////////////
void ZClothEmblem::OnInvalidate()
{
	SAFE_RELEASE( g_hw_Buffer );
}

//////////////////////////////////////////////////////////////////////////
//	OnRestore
//////////////////////////////////////////////////////////////////////////
void ZClothEmblem::OnRestore()
{
	if( g_hw_Buffer == 0 )
	{
		if( FAILED( RGetDevice()->CreateVertexBuffer( MAX_NUM_CLOTH_PARTICLE* 3 * sizeof( RVertex ), 
			D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, RVertexType, D3DPOOL_DEFAULT, &g_hw_Buffer ,NULL) ))
		{
			mlog( "Fail to Restore Vertex Buffer for Emblems..\n" );
			bHardwareBuffer = false;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//	생성자 / 소멸자
//////////////////////////////////////////////////////////////////////////
ZClothEmblem::ZClothEmblem(void)
{
	//setOption( 1, 0.10, 1.0f );	//	초기값
	setOption(1, 0.15f,1.0f);
	mpWind	= new rvector(0,0,0);
	mpLight	= 0;
	mpMeshNode	= 0;
	mpTex				= 0;
	mBaseWind	= rvector(0,1,0);
	mfBaseMaxPower	= 0;
	m_fDist = 0.f;
	if(msRef==0) OnRestore();
	++msRef;
	//mIndexBuffer = 0;
	mbIsInFrustrum = true;
}

ZClothEmblem::~ZClothEmblem(void)
{
	SAFE_DELETE(mpWind);
	SAFE_DELETE(mpLight);
	for( list<sRestriction*>::iterator iter = mRestrictionList.begin() ; iter != mRestrictionList.end(); )
	{
		sRestriction* p = *iter;
		SAFE_DELETE(p);
		iter = mRestrictionList.erase(iter);
	}
	//SAFE_RELEASE(mIndexBuffer);
	--msRef;
	if(msRef==0) OnInvalidate();
}

bool e_clothemblem_sort_float(ZClothEmblem* _a,ZClothEmblem* _b) {
	if( _a->m_fDist > _b->m_fDist )
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////
//	ZEmblemList
//////////////////////////////////////////////////////////////////////////
void ZEmblemList::Draw()
{
	rvector camera_pos = RealSpace2::RCameraPosition;
	rvector t_vec;
	rvector t_pos;

	for( iterator iter = begin(); iter != end(); ++iter )
	{
		ZClothEmblem* pCurr	= *iter;

		if( pCurr == NULL) continue;

		t_pos.x = pCurr->mWorldMat._41;
		t_pos.y = pCurr->mWorldMat._42;
		t_pos.z = pCurr->mWorldMat._43;

		t_vec = camera_pos - t_pos;
		pCurr->m_fDist = Magnitude(t_vec);
	}

	sort(e_clothemblem_sort_float);

	for( iterator iter = begin(); iter != end(); ++iter )
	{
		ZClothEmblem* pCurr	= *iter;
		if(pCurr != 0)	pCurr->render();
	}
}

ZClothEmblem* ZEmblemList::Get( int i_ )
{
	if( i_ >= (int)size() )
	{
		return NULL;
	}
	iterator iter = begin();
	for( int i = 0 ; i < i_; ++i )
	{
		++iter;
	}
	return *iter;
}

void ZEmblemList::Update()
{
	for( iterator iter = begin(); iter != end(); ++iter )
	{
		ZClothEmblem* pCurr	= *iter;
		pCurr->update();
	}
}

ZEmblemList::~ZEmblemList()
{
	Clear();
}

void ZEmblemList::Clear()
{
	for( iterator iter = begin(); iter != end(); ++iter )
	{
		SAFE_DELETE( *iter );
	}
	clear();
}

void ZEmblemList::SetExplosion(  rvector& pos_, float power_ )
{
	for( iterator iter = begin(); iter != end(); ++iter )
	{
		ZClothEmblem* pCurr	= *iter;
		pCurr->setExplosion( pos_, power_ );
	}
}

void ZEmblemList::CheckSpearing( rvector& bullet_begine_, rvector& bullet_end_, float power_ )
{
	for( iterator iter = begin(); iter != end(); ++iter )
	{
		ZClothEmblem* pCurr	= *iter;
		pCurr->CheckSpearing( bullet_begine_, bullet_end_, power_ );
	}
}

//////////////////////////////////////////////////////////////////////////
//	OnInvalidate
//////////////////////////////////////////////////////////////////////////
void ZEmblemList::OnInvalidate()
{
	if( ZClothEmblem::GetRefCount() != 0 )
	{
		ZClothEmblem* p = *(begin());
		if(p != 0 ) p->OnInvalidate();
	}
}

//////////////////////////////////////////////////////////////////////////
//	OnRestore
//////////////////////////////////////////////////////////////////////////
void ZEmblemList::OnRestore()
{
	if( ZClothEmblem::GetRefCount() != 0 )
	{
		ZClothEmblem* p = *(begin());
		if(p != 0 ) p->OnRestore();
	}
}

//////////////////////////////////////////////////////////////////////////
//	InitEnv
//////////////////////////////////////////////////////////////////////////
void ZEmblemList::InitEnv( char* pFileName_ )
{
	MXmlDocument	Data;
	Data.Create();

	MZFile mzf;
	if( !mzf.Open( pFileName_, g_pFileSystem ))
	{
		return;
	}
	char* buffer;
	buffer	= new char[mzf.GetLength() + 1];
	mzf.Read( buffer, mzf.GetLength() );
	buffer[mzf.GetLength()] = 0;

	if( !Data.LoadFromMemory(buffer) )
	{
		delete buffer;
		return;
	}
	delete buffer;
	mzf.Close();

	MXmlElement root, child;
	char TagName[256];
	char Attribute[256];
	root = Data.GetDocumentElement();
	int iCount = root.GetChildNodeCount();	

	for( int i = 0 ; i < iCount; ++i )
	{
		child		= root.GetChildNode(i);
		child.GetTagName( TagName );
		if( TagName[0] == '#' )
		{
			continue;
		}
		child.GetAttribute( Attribute, "NAME" );
		mEmblemMapItor	= mEmblemMap.find( Attribute );
		if( mEmblemMapItor	!= mEmblemMap.end() )
		{
			ZClothEmblem* p		= mEmblemMapItor->second;
			
			if( child.GetAttribute( Attribute, "DIRECTION" ))
			{
				D3DXMATRIX RotMat;
				rvector dir = rvector( 0,1,0 );
				int theta;
				sscanf( Attribute, "%d", &theta );
				D3DXMatrixRotationAxis( &RotMat, &rvector(0,0,1), ((float)theta*D3DX_PI/180) );
				dir = dir*RotMat;
				//p->SetBaseWind( dir );
				p->GetWndGenerator()->SetWindDirection( dir );
			}

			if( child.GetAttribute( Attribute, "POWER" ))
			{
				float power;
				sscanf( Attribute, "%f", &power );
				p->GetWndGenerator()->SetWindPower( power );
			}
			
			MXmlElement dummy;
			int iDummyNum = child.GetChildNodeCount();
			for( int j = 0 ; j < iDummyNum; ++j )
			{
				dummy = child.GetChildNode( j );
				dummy.GetTagName( TagName );
				if( TagName[0] == '#' )
				{
					continue;
				}
				if( _stricmp( TagName, "RESTRICTION" ) == 0 )
				{
					sRestriction* rest = new sRestriction;
					int iValue = 0;
					float fValue = 0.f;
					if( dummy.GetAttribute( Attribute, "AXIS" ))
					{
						sscanf( Attribute, "%d", &iValue );
						rest->axis	=(RESTRICTION_AXIS)iValue;
					}				
					if( dummy.GetAttribute( Attribute, "POSITION") )
					{
						sscanf( Attribute, "%f", &fValue );
						rest->position = fValue;
					}
					if( dummy.GetAttribute(Attribute, "COMPARE") )
					{
						sscanf( Attribute, "%d", &iValue );
						rest->compare =(RESTRICTION_COMPARE)iValue;
					}
					p->AddRestriction( rest );
				}
				else if( _stricmp( TagName, "WINDTYPE" ) == 0 )
				{
					int iValue = 0;
					if( dummy.GetAttribute( Attribute, "TYPE" ) )
					{
						sscanf( Attribute, "%d", &iValue );
						p->GetWndGenerator()->SetWindType( (WIND_TYPE) iValue );
					}
					if( dummy.GetAttribute( Attribute, "DELAY" ))
					{
						sscanf( Attribute, "%d", &iValue );
						p->GetWndGenerator()->SetDelayTime( iValue );
					}
				}
			}
		}
	}

	for( list<ZClothEmblem*>::iterator iter = begin(); iter != end(); ++iter )
	{
		// 처음 몇 프레임을 계산하고 시작한다..
		for( int i = 0 ; i < 100; ++i )
			(*iter)->update();
	}
}

void ZEmblemList::Add( ZClothEmblem* p_, char* pName_ )
{
	push_back( p_ );
	mEmblemMap.insert( map<string, ZClothEmblem*>::value_type( pName_, p_ ) );
}