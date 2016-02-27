#include "stdafx.h"
#include "RealSpace2.h"
#include "RDynamicLight.h"
#include "RShaderMgr.h"

using namespace RealSpace2;

//////////////////////////////////////////////////////////////////////////
//	Global
//////////////////////////////////////////////////////////////////////////
RDynamicLightManager	RDynamicLightManager::msInstance;
sMapLight				RDynamicLightManager::msMapLightList[MAP_LIGHT_NUM];


//////////////////////////////////////////////////////////////////////////
//	Define
//////////////////////////////////////////////////////////////////////////
struct node
{
	node* p;
	node* n;
	float fDist;
	sMapLightObj* pl;
	node() : p(0),n(0),fDist(0),pl(0) {};
};
void compare_and_add( node* first_, node* node_, rvector pos_ );

//////////////////////////////////////////////////////////////////////////
//	AddLight
//////////////////////////////////////////////////////////////////////////
bool RDynamicLightManager::AddLight( MAP_LIGHT_TYPE light_type_, rvector& pos_ )
{
	sMapLightObj temp;

	if( light_type_ == GUNFIRE )
	{
		if( mbGunLight )
		{
			mGunLight.fLife					= msMapLightList[GUNFIRE].fLife;
			mGunLight.vLightColor.x += 0.01;
			mGunLight.vLightColor.y += 0.01;
			mGunLight.vLightColor.z += 0.01;
			mGunLight.vLightColor.x = min(mGunLight.vLightColor.x, 1.0f );
			mGunLight.vLightColor.y = min(mGunLight.vLightColor.y, 1.0f );
			mGunLight.vLightColor.z = min(mGunLight.vLightColor.z, 1.0f );
			mGunLight.vPos			= pos_;
		}
		else
		{
			mGunLight.fLife			= msMapLightList[GUNFIRE].fLife;
			mGunLight.vLightColor	= msMapLightList[GUNFIRE].vLightColor;
			mGunLight.fRange		= msMapLightList[GUNFIRE].fRange;
			mGunLight.vPos			= pos_;
		}
		mbGunLight = TRUE;
	}
	else if( light_type_ == EXPLOSION )
	{
		if( mExplosionLightList.size() >= MAX_EXPLOSION_LIGHT )
		{
			return false;
		}
		temp.fLife	= msMapLightList[EXPLOSION].fLife;
		temp.fRange	= msMapLightList[EXPLOSION].fRange;
		temp.vLightColor	= msMapLightList[EXPLOSION].vLightColor;
		temp.vPos = pos_;
		mExplosionLightList.push_back( temp );
	}
	return true;
}




//////////////////////////////////////////////////////////////////////////
//	생성자 / 소멸자
//////////////////////////////////////////////////////////////////////////
RDynamicLightManager::RDynamicLightManager()
{
	/*
	//TEST
	msMapLightList[0].fLife = 1000;
	msMapLightList[0].fRange = 100;
	msMapLightList[0].iType = GUNFIRE;
	msMapLightList[0].vLightColor.x = 1.0f;
	msMapLightList[0].vLightColor.y = 1.0f;
	msMapLightList[0].vLightColor.z = 1.0f;

	msMapLightList[1].fLife = 2000;
	msMapLightList[1].fRange = 10000;
	msMapLightList[1].iType = EXPLOSION;
	msMapLightList[1].vLightColor.x = 1.0f;
	msMapLightList[1].vLightColor.y = 0.1f;
	msMapLightList[1].vLightColor.z = 0.1f;

	miNumEnableLight = 3;
	//END TEST
	//*/
	Initialize();
}

RDynamicLightManager::~RDynamicLightManager()
{
	mExplosionLightList.clear();
}

//////////////////////////////////////////////////////////////////////////
//	Update
//////////////////////////////////////////////////////////////////////////
void RDynamicLightManager::Update()
{
	float lastTime = mTime;
	mTime = timeGetTime();
	float lap = mTime - lastTime;
	
	// 등록된 라이트가 없으면 그냥 반환
	if( !mbGunLight && mExplosionLightList.size() <= 0 )
	{
		return;
	}

	if( mbGunLight )
	{
		mGunLight.fLife -= lap;
		if( mGunLight.fLife <= 0 )
		{
			mbGunLight = false;
		}
		mGunLight.vLightColor.x -= 0.01;
		mGunLight.vLightColor.y -= 0.01;
		mGunLight.vLightColor.z -= 0.01;
	}

	vector<sMapLightObj>::iterator itor;
	sMapLightObj* pCurr;

	for( itor = mExplosionLightList.begin(); itor != mExplosionLightList.end(); )
	{
		pCurr = &(*itor);
		pCurr->fLife -= lap;
		if( pCurr->fLife <= 0 )
		{
			itor = mExplosionLightList.erase( itor );
		}
		else
		{
			++itor;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//	Initialize
//////////////////////////////////////////////////////////////////////////
void RDynamicLightManager::Initialize()
{
	mExplosionLightList.reserve( MAX_EXPLOSION_LIGHT );
}

//////////////////////////////////////////////////////////////////////////
//	SetPosition
//////////////////////////////////////////////////////////////////////////
void RDynamicLightManager::SetPosition( rvector& pos_ )
{
	mvPosition = pos_;
}


//////////////////////////////////////////////////////////////////////////
//	SetLight
//	선택기준(우선순위)
//	1.	내 캐릭터의 총 발사 라이트.. 최우선 (정말?)	-> 마지막 슬롯 사용
//	2.	나머지 가능한 슬롯은 모두 폭발 라이트임..
//		2.0.	카메라 시야 안에 있는 라이트만 고려
//		2.1.	내 캐릭터에 가까운 폭발 라이트 우선
//		2.2.	현재 보여지고 있는 폭발 라이트 우선
//		2.3.	수명이 많이남은 라이트 우선
//////////////////////////////////////////////////////////////////////////
int RDynamicLightManager::SetLight( rvector pos_ )
{
	D3DLIGHT9	Light;
	memset( &Light, 0, sizeof(D3DLIGHT9) );
	Light.Type = D3DLIGHT_POINT;
	int available_light_slot;
	int base_slot = 0;
	//int current_slot = 0;
	node result;
	result.fDist = 0;

	if( miNumEnableLight <= 0 )						//	라이트 사용 안할경우 그냥 반환
	{
		RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
		return base_slot;
	}

	if( mbGunLight || mExplosionLightList.size() > 0 )
	{
		RGetDevice()->SetRenderState( D3DRS_LIGHTING, TRUE );
		RGetDevice()->SetRenderState( D3DRS_AMBIENT, 0x00000000 );
		
		RGetShaderMgr()->setAmbient( 0x00000000 );
	}
	else
	{
		RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
	}

	if( mbGunLight )	
	{
		Light.Attenuation1	= 0.00001;
		Light.Diffuse.r		= mGunLight.vLightColor.x;
		Light.Diffuse.g		= mGunLight.vLightColor.y;
		Light.Diffuse.b		= mGunLight.vLightColor.z;
		Light.Range			= mGunLight.fRange;
		Light.Position.x		= mGunLight.vPos.x;
		Light.Position.y		= mGunLight.vPos.y;
		Light.Position.z		= mGunLight.vPos.z;
		Light.Type			= D3DLIGHT_POINT;

		RGetDevice()->SetLight( 0, &Light );
		RGetDevice()->LightEnable( 0, true );
		available_light_slot = miNumEnableLight - 1;
		base_slot = 1;
	}
	else
	{
		available_light_slot = miNumEnableLight;	// 총의 라이트가 없으면 폭발 라이트가 그 공간도 이용함
		base_slot = 0;
	}

    if( available_light_slot >= 0 )
	{
		float	decisionTest;
		bool	bCull = false;
		rplane* frustum	= RGetViewFrustum();
		sMapLightObj*	pCurr;

		for( vector<sMapLightObj>::iterator  itor = mExplosionLightList.begin(); 
			itor != mExplosionLightList.end(); ++ itor )
		{
			pCurr = &(*itor);
			
			// Check Visible
			for( int i = 0 ; i < 6; ++i )
			{
				decisionTest = D3DXPlaneDotCoord( &frustum[i], &pCurr->vPos );
				if( decisionTest < 0 )
				{
					bCull = true;
					break;
				}
			}
			if( bCull )	// 뷰 프러스텀에 들지 않으므로 제외
			{
				continue;
			}

			// 캐릭터에 가까운 순으로 정렬(사용여부와 수명 고려)
			node n;
			n.pl	= pCurr;
			compare_and_add( &result, &n, mvPosition );
		}
	}

	// 가용한 슬롯에 결과로 정렬된 결과값을 가지고 라이트 셋팅
	node* pCurr = result.n;
	for( int i = 0 ; i < available_light_slot; ++i )
	{
		if( pCurr == NULL )
		{
			break;
		}
		Light.Attenuation1	= 0.01;
		Light.Position.x	= pCurr->pl->vPos.x;
		Light.Position.y	= pCurr->pl->vPos.x;
		Light.Position.z	= pCurr->pl->vPos.x;
		
		if( pCurr->pl->bUsing )
		{
			// 라이트 값의 변화를 주는 것도 재미있을 것 같음	
		}
		
		Light.Diffuse.r		= pCurr->pl->vLightColor.x;
		Light.Diffuse.g		= pCurr->pl->vLightColor.y;
		Light.Diffuse.b		= pCurr->pl->vLightColor.z;
		Light.Range			= pCurr->pl->fRange;
		Light.Type			= D3DLIGHT_POINT;
		
		pCurr->pl->bUsing = TRUE;	// 사용중임을 표시..

		RGetDevice()->LightEnable( base_slot, TRUE );
		RGetDevice()->SetLight( base_slot++, &Light );		
	}

	return base_slot;
}

bool RDynamicLightManager::SetLight( rvector& pos_, int lightIndex_, float maxDistance_  )
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
//	Release Light
//////////////////////////////////////////////////////////////////////////
void RDynamicLightManager::ReleaseLight()
{
	for( int i = 0 ; i < miNumEnableLight; ++i )
	{
		RGetDevice()->LightEnable( i, FALSE );
	}
}


//////////////////////////////////////////////////////////////////////////
//	Global Method
//////////////////////////////////////////////////////////////////////////
RDynamicLightManager* RGetDynamicLightManager()
{
	return RDynamicLightManager::GetInstance();
}

sMapLight*	RGetMapLightList()
{
	return RDynamicLightManager::GetLightMapList();
}


//////////////////////////////////////////////////////////////////////////
//	Helper
//	FastLinkedList - To 정렬
//////////////////////////////////////////////////////////////////////////
#define DISTANCE_CLOSE	50

// 오름차순 정렬
void compare_and_add( node* first_, node* node_, rvector pos_/*현재 캐릭터 좌표*/ )
{
	node*	pCurr	= first_;
	node*	pTemp;
	float	Dist;
	while( pCurr != NULL )
	{
		Dist = fabs( pCurr->fDist - node_->fDist );

		if( pCurr->fDist > node_->fDist )
		{
			pTemp = pCurr;
			pCurr = node_;
			pCurr->n = pTemp;

			return;
		}
		else if( Dist < DISTANCE_CLOSE )
		{
			if( !pCurr->pl->bUsing && node_->pl->bUsing )
			{
				pTemp = pCurr;
				pCurr = node_;
				pCurr->n = pTemp;

				return;
			}
			if( pCurr->pl->fLife < node_->pl->fLife )
			{
				pTemp = pCurr;
				pCurr = node_;
				pCurr->n = pTemp;

				return;
			}
		}
		
		pCurr = pCurr->n;
	}

	pCurr = node_;
}

//////////////////////////////////////////////////////////////////////////
// 초기화 ...
//////////////////////////////////////////////////////////////////////////
sMapLightObj::sMapLightObj()
{
	bUsing = false;
};
