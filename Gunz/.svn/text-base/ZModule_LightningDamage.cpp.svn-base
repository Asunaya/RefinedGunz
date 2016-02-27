#include "stdafx.h"
#include "ZModule_LightningDamage.h"
#include "ZGame.h"
#include "ZApplication.h"
#include "ZModule_HPAP.h"


#define DAMAGE_DELAY	1.f			// 데미지 주는 간격

ZModule_LightningDamage::ZModule_LightningDamage()
{
}

void ZModule_LightningDamage::InitStatus()
{
	Active(false);
}

bool ZModule_LightningDamage::Update(float fElapsed)
{
	// object 에서 상속받은게 아니면 낭패
	if(MIsDerivedFromClass(ZObject,m_pContainer))
	{
		ZObject *pObj = MStaticCast(ZObject,m_pContainer);

		ZModule_Movable *pMovable = (ZModule_Movable*)m_pContainer->GetModule(ZMID_MOVABLE);
		if(pMovable)
			pMovable->SetVelocity(0,0,0);

		// 데미지 간격 DAMAGE_DELAY
		if(g_pGame->GetTime()>m_fNextDamageTime) {
			m_fNextDamageTime+=DAMAGE_DELAY;

			// 데미지 받고 있는 이펙트 죽었더라도 완전히 투명해지지 않은 상태에서는 보인다..
			if(pObj->IsDie()) {

				if( pObj->m_pVMesh->GetVisibility() < 0.5f ) {//이펙트의 Life 타임도 있으니까...
					return false;
				}

			}
			else //살아있을떄만..
			{
				float fFR = 0;
				float fDamage = 6 * (1.f-fFR) + (float)m_nDamage;

				ZModule_HPAP *pModule = (ZModule_HPAP*)m_pContainer->GetModule(ZMID_HPAP);
				if(pModule) {
					pModule->OnDamage(m_Owner,fDamage,0);
//					pObj->OnScream();
				}
			}
		}
	}

	if(m_fNextDamageTime-m_fBeginTime>m_fDuration) {
		return false;
	}
	return true;
}

// 데미지를 주기 시작한다
void ZModule_LightningDamage::BeginDamage(MUID owner, int nDamage, float fDuration)
{
	m_fBeginTime = g_pGame->GetTime();
	m_fNextDamageTime = m_fBeginTime+DAMAGE_DELAY;

	m_Owner = owner;
	m_nDamage = nDamage;
	m_fDuration = fDuration;

	Active();
}
