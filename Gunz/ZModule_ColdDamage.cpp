#include "stdafx.h"
#include "ZModule_ColdDamage.h"
#include "ZGame.h"
#include "ZApplication.h"
#include "MRTTI.h"

#define DAMAGE_DELAY	1.f			// 데미지 주는 간격
#define EFFECT_DELAY	0.15f			// 이펙트 간격

int GetEffectLevel();

ZModule_ColdDamage::ZModule_ColdDamage()
{
}

void ZModule_ColdDamage::InitStatus()
{
	//ZModule_Movable *pMovableModule = (ZModule_Movable*)m_pContainer->GetModule(ZMID_MOVABLE);
	//_ASSERT(pMovableModule!=NULL);
	//pMovableModule->SetMoveSpeedRatio(1.f);
	Active(false);
}

bool ZModule_ColdDamage::Update(float fElapsed)
{
	//ZModule_Movable *pMovableModule = (ZModule_Movable*)m_pContainer->GetModule(ZMID_MOVABLE);
	//_ASSERT(pMovableModule!=NULL);
	//pMovableModule->SetMoveSpeedRatio(m_fMoveSpeed);

	ZObject *pObj = MStaticCast(ZObject,m_pContainer);

	if(g_pGame->GetTime()>m_fNextDamageTime) {
		m_fNextDamageTime+=DAMAGE_DELAY;

		ZModule_HPAP *pHPModule = (ZModule_HPAP*)m_pContainer->GetModule(ZMID_HPAP);
		_ASSERT(pHPModule!=NULL);

		{
			if(pObj->IsDie()) {

				if( pObj->m_pVMesh->GetVisibility() < 0.5f ) {//이펙트의 Life 타임도 있으니까...
//					pMovableModule->SetMoveSpeedRatio(1.f);
					return false;
				}
			}
			else //살아있을떄만..
			{
//				pObj->OnScream();
			}
		}
	}

	if(g_pGame->GetTime()>m_fNextEffectTime) {

		if(!pObj->IsDie())
		{
			int nEffectLevel = GetEffectLevel()+1;

			m_fNextEffectTime+=EFFECT_DELAY * nEffectLevel;

			ZGetEffectManager()->AddEnchantCold2( pObj );
		}
	}


	if(m_fNextDamageTime-m_fBeginTime>m_fDuration) {
//		pMovableModule->SetMoveSpeedRatio(1.f);
		return false;
	}

	return true;
}

// 데미지를 주기 시작한다
void ZModule_ColdDamage::BeginDamage(float fMoveSpeed,float fDuration)
{
	m_fBeginTime = g_pGame->GetTime();
	m_fNextDamageTime = m_fBeginTime+DAMAGE_DELAY;
	m_fNextEffectTime = m_fBeginTime;

	m_fDuration = fDuration;
//	m_fMoveSpeed = fMoveSpeed;

	ZModule_Movable *pMovableModule = (ZModule_Movable*)m_pContainer->GetModule(ZMID_MOVABLE);
	_ASSERT(pMovableModule!=NULL);
	pMovableModule->SetMoveSpeedRatio(fMoveSpeed,fDuration);

	Active();
}
