#include "stdafx.h"

#include "ZModule_Movable.h"
#include "ZObjectManager.h"
#include "ZGame.h"
#include "ZGameConst.h"
#include "Portal.h"

// 나 이외의 캐릭터는 위치를 양자화해서 보내므로 오차가 있어서 m_fRadius 를 실제보다 2정도 줄였다

ZModule_Movable::ZModule_Movable() 
			: m_Velocity(0,0,0), m_bFalling(false), m_bLanding(false),m_bRestrict(false),
			m_fMaxSpeed(1000.f), m_fMoveSpeedRatio(1.f)
{
}

void ZModule_Movable::OnAdd()
{
	_ASSERT(MIsDerivedFromClass(ZObject,m_pContainer));
}

void ZModule_Movable::InitStatus()
{
	m_Velocity=rvector(0,0,0);
	m_bRestrict = false;
	m_fMoveSpeedRatio = 1.f;
}

bool ZModule_Movable::Update(float fElapsed)
{
	ZObject *pThisObj = MStaticCast(ZObject,m_pContainer);
	if(!pThisObj->GetInitialized()) return true;
	if(!pThisObj->IsVisible()) return true;

	if(m_bRestrict && g_pGame->GetTime()-m_fRestrictTime > m_fRestrictDuration) {
		// 시간이 되었으므로 이동제한을 풀어준다
		m_bRestrict = false;
		m_fMoveSpeedRatio = 1.f;
	}

	UpdatePosition(fElapsed);

	return true;
}

// TODO : 충돌 범위 / 방법을 유동적으로 가져가자
//#define COLLISION_DIST	70.f


bool ZModule_Movable::Move(rvector &diff)
{
	ZObject *pThisObj = MStaticCast(ZObject,m_pContainer);

#ifdef PORTAL
	if (g_pPortal->Move(pThisObj, diff))
		return true;
#endif

	float fThisObjRadius = pThisObj->GetCollRadius();

	ZObjectManager *pcm=&g_pGame->m_ObjectManager;
	for (ZObjectManager::iterator itor = pcm->begin(); itor != pcm->end(); ++itor)
	{
		ZObject* pObject = (*itor).second;
		if (pObject != pThisObj && pObject->IsCollideable())
		{
			rvector pos=pObject->GetPosition();
			rvector dir=pThisObj->GetPosition()+diff-pos;
			dir.z=0;
			float fDist=Magnitude(dir);

			float fCOLLISION_DIST = pObject->GetCollRadius() + pThisObj->GetCollRadius();

			if(fDist<fCOLLISION_DIST && fabs(pos.z-pThisObj->GetPosition().z) < pObject->GetCollHeight())
			{
				// 거의 같은위치에 있는 경우.. 한쪽방향으로 밀림
				if(fDist<1.f)
				{
					pos.x+=1.f;
					dir=pThisObj->GetPosition()-pos;
				}

				if(DotProduct(dir,diff)<0)	// 더 가까워지는 방향이면
				{
					Normalize(dir);
					rvector newthispos=pos+dir*(fCOLLISION_DIST+1.f);

					rvector newdiff=newthispos-pThisObj->GetPosition();
					diff.x=newdiff.x;
					diff.y=newdiff.y;

				}
			}
		}
	}

	rvector origin,targetpos;
	rplane impactplane;

	// 최소 120이상인 이유는 이동할 수 있는 곳의 각도가 플레이어와 같도록 하기 위함이고,
	// 1.7142857142857143f는 플레이어의 (radius / height)값
	float fCollUpHeight = max(120.0f, pThisObj->GetCollHeight() - pThisObj->GetCollRadius() * 1.7142857142857143f);

	origin=pThisObj->GetPosition()+rvector(0,0,fCollUpHeight);
	targetpos=origin+diff;

	// 나락 이하는 맵 체크하지 않는다.
	if (pThisObj->GetPosition().z > DIE_CRITICAL_LINE)
	{
		m_bAdjusted = ZGetGame()->GetWorld()->GetBsp()->CheckWall(origin,targetpos,fThisObjRadius,60,RCW_CYLINDER,0,&impactplane);
	}
	else
	{
		SetVelocity(0.0f, 0.0f, GetVelocity().z);
		m_bAdjusted = false;
	}

	diff=targetpos-origin;
	pThisObj->SetPosition(targetpos-rvector(0,0,fCollUpHeight));

	if(m_bAdjusted)
		m_fLastAdjustedTime=g_pGame->GetTime();

	return m_bAdjusted;
}

void ZModule_Movable::UpdateGravity(float fDelta)
{
	m_Velocity.z = 
		max( m_Velocity.z - GRAVITY_CONSTANT*fDelta,-MAX_FALL_SPEED);
}

void ZModule_Movable::UpdatePosition(float fDelta)
{
	ZObject *pThisObj = MStaticCast(ZObject,m_pContainer);

	rvector diff=fDelta*m_Velocity;

	bool bUp = (diff.z>0.01f);
	bool bDownward= (diff.z<0.01f);

//	rvector diff2d=rvector(diff.x,diff.y,0);
	if(Magnitude(diff)>0.01f)
		Move(diff);

	rvector floor=g_pGame->GetFloor(pThisObj->GetPosition(),&m_FloorPlane);
	m_fDistToFloor=pThisObj->GetPosition().z-floor.z;

	// 올라가야 하는데 못올라간경우
	if(bUp && diff.z<=0.01f) {
		SetVelocity(GetVelocity().x,GetVelocity().y,0);
	}

	if(!m_bFalling && diff.z<-0.1f && m_fDistToFloor>35.f) {
		m_bFalling = true;
		m_fFallHeight = pThisObj->GetPosition().z;
	}

	if ((pThisObj->GetPosition().z > DIE_CRITICAL_LINE) && (m_bFalling && GetDistToFloor()<1.f)) {
		m_bFalling = false;
		m_bLanding = true;
	}else
		m_bLanding = false;

	m_lastMove = diff;

}
void ZModule_Movable::SetMoveSpeedRatio(float fRatio, float fDuration) 
{ 
	// 더 느린 제한이 있으면 무시한다
	if(m_bRestrict && fRatio<m_fMoveSpeedRatio) return;

	m_bRestrict = true;
	m_fRestrictTime = g_pGame->GetTime();
	m_fRestrictDuration = fDuration;
	m_fMoveSpeedRatio = fRatio; 
}
