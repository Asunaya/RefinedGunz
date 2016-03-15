#include "stdafx.h"
#include "ZGameAction.h"
#include "ZGame.h"
#include "ZGameClient.h"
#include "ZEffectManager.h"
#include "ZApplication.h"
#include "ZSoundEngine.h"
#include "ZMyCharacter.h"
#include "ZPost.h"
#include "ZModule_FireDamage.h"
#include "ZModule_ColdDamage.h"
#include "ZModule_LightningDamage.h"
#include "ZModule_PoisonDamage.h"

#define MAX_ENCHANT_DURATION	10.f

bool ZGameAction::OnCommand(MCommand* pCommand)
{
	switch (pCommand->GetID())
	{
		HANDLE_COMMAND(MC_PEER_ENCHANT_DAMAGE	,OnEnchantDamage)
		HANDLE_COMMAND(MC_PEER_REACTION			,OnReaction)
		HANDLE_COMMAND(MC_PEER_SKILL			,OnPeerSkill)
	}

	return false;
}

bool ZGameAction::OnReaction(MCommand* pCommand)
{
	float fTime;
	int nReactionID;

	pCommand->GetParameter(&fTime,			0, MPT_FLOAT);		// 시간
	pCommand->GetParameter(&nReactionID,	1, MPT_INT);

	ZCharacter *pChar=ZGetCharacterManager()->Find(pCommand->GetSenderUID());
	if(!pChar) return true;

	switch(nReactionID)
	{
		case ZR_CHARGING	: {
			pChar->m_bCharging=true;
			if(!pChar->IsHero())
				pChar->SetAnimationLower(ZC_STATE_CHARGE);
			ZGetEffectManager()->AddChargingEffect(pChar);
		}break;
		case ZR_CHARGED		: {
			pChar->m_bCharged=true;
			pChar->m_fChargedFreeTime = g_pGame->GetTime() + fTime;
			ZGetEffectManager()->AddChargedEffect(pChar);

			ZGetSoundEngine()->PlaySound("fx2/FX_ChargeComplete", pChar->GetPosition());
		}break;
		case ZR_BE_UPPERCUT	: {
			rvector tpos = pChar->GetPosition();
			tpos.z += 130.f;
			ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos,pChar->GetUID());
			ZGetSoundEngine()->PlaySound("uppercut", tpos);
		}break;
		case ZR_DISCHARGED	: {
			pChar->m_bCharged=false;
		}break;
	}

	return true;
}

bool ZGameAction::OnPeerSkill(MCommand* pCommand)
{
	float fTime;
	int nSkill,sel_type;

	pCommand->GetParameter(&fTime, 0, MPT_FLOAT);
	pCommand->GetParameter(&nSkill, 1, MPT_INT);
	pCommand->GetParameter(&sel_type, 2, MPT_INT);

	ZCharacter* pOwnerCharacter = ZGetCharacterManager()->Find(pCommand->GetSenderUID());
	if (pOwnerCharacter == NULL) return true;

	switch(nSkill)	{
		// 띄우기 스킬
		case ZC_SKILL_UPPERCUT		: OnPeerSkill_Uppercut(pOwnerCharacter);break;
			// 강베기 스플래시
		case ZC_SKILL_SPLASHSHOT	: OnPeerSkill_LastShot(fTime,pOwnerCharacter);break;
			// 단검 특수공격
		case ZC_SKILL_DASH			: OnPeerSkill_Dash(pOwnerCharacter);break;
	}

	return true;
}

// 강베기 처리한다. 내가 맞았는지만 검사한다
void ZGameAction::OnPeerSkill_LastShot(float fShotTime,ZCharacter *pOwnerCharacter)	// 칼 마지막 방 스플래시
{
	if( pOwnerCharacter == NULL ) return;
	ZItem *pItem = pOwnerCharacter->GetItems()->GetItem(MMCIP_MELEE);
	if(!pItem) return;

	MMatchItemDesc* pDesc = pItem->GetDesc();
	if(!pDesc) return;

	const float fRange = 300.f;			// 범위는 4미터

//	if(pOwnerCharacter->m_AniState_Lower>=ZC_STATE_LOWER_ATTACK3 && pOwnerCharacter->m_AniState_Lower<=ZC_STATE_LOWER_ATTACK5)
	{
		// fShotTime 이 그 캐릭터의 로컬 시간이므로 내 시간으로 변환해준다
		fShotTime-=pOwnerCharacter->m_fTimeOffset;

		rvector OwnerPosition,OwnerDir;
		if(!pOwnerCharacter->GetHistory(&OwnerPosition,&OwnerDir,fShotTime))
			return;


		rvector waveCenter = OwnerPosition; // 폭발의 중심

		rvector _vdir = OwnerDir;
		_vdir.z = 0;
//		Normalize(_vdir);
//		waveCenter += _vdir * 180.f;

		ZC_ENCHANT zc_en_type = pOwnerCharacter->GetEnchantType();

		// 사운드
		ZGetSoundEngine()->PlaySound(pOwnerCharacter->IsObserverTarget() ? "we_smash_2d" : "we_smash", waveCenter );

		// 바닥의 wave 이펙트
		{
			ZGetEffectManager()->AddSwordWaveEffect(waveCenter, pOwnerCharacter->GetDirection());
		}

		for (ZObjectManager::iterator itor = ZGetObjectManager()->begin();
			itor != ZGetObjectManager()->end(); ++itor)
		{
			ZObject* pTar = (*itor).second;

			if (pTar==NULL) continue;
			if (pOwnerCharacter == pTar) continue;

			if(pTar!=g_pGame->m_pMyCharacter &&	// 내 캐릭터나 내가 조종하는 npc 만 체크한다
				(!pTar->IsNPC() || !((ZActor*)pTar)->IsMyControl())) continue;

			if(!ZGetGame()->IsAttackable(pOwnerCharacter,pTar)) continue;
			//// 팀플레이고 같은 팀이고 팀킬 불가로 되어있으면 넘어간다
			//if(ZApplication::GetGame()->GetMatch()->IsTeamPlay() &&
			//	pOwnerCharacter->IsTeam(pTar) && !g_pGame->GetMatch()->GetTeamKillEnabled()) return;

			rvector TargetPosition,TargetDir;

			if(pTar->IsDie()) continue;
			// 적절한 위치를 얻어낼수 없으면 다음으로~
			if( !pTar->GetHistory(&TargetPosition,&TargetDir,fShotTime)) continue;

			rvector checkPosition = TargetPosition + rvector(0,0,80);
			float fDist = Magnitude(waveCenter - checkPosition);

			if (fDist < fRange) {

				if ((pTar) && (pTar != pOwnerCharacter)) {

					if(g_pGame->CheckWall( pOwnerCharacter,pTar )==false) // 중간에 벽이 막고 있는가?
					{
						// 막고있으면 데미지를 안받는다
						if(pTar->IsGuard() && DotProduct(pTar->m_Direction,OwnerDir)<0 )
						{
							rvector addVel = pTar->GetPosition() - waveCenter;
							Normalize(addVel);
							addVel = 500.f*addVel;
							addVel.z = 200.f;
							pTar->AddVelocity(addVel);
						}else
						{
							rvector tpos = pTar->GetPosition();

							tpos.z += 130.f;


							if( zc_en_type == ZC_ENCHANT_NONE ) {

								ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos,pTar->GetUID());
							}
							else {

								ZGetEffectManager()->AddSwordEnchantEffect(zc_en_type,pTar->GetPosition(),20);
							}

							tpos -= pOwnerCharacter->m_Direction * 50.f;

							rvector fTarDir = pTar->GetPosition() - pOwnerCharacter->GetPosition();
							Normalize(fTarDir);

#define MAX_DMG_RANGE	50.f	// 반경이만큼 까지는 최대 데미지를 다 먹는다
#define MIN_DMG			0.3f	// 최소 기본 데미지는 이정도.

							float fDamageRange = 1.f - (1.f-MIN_DMG)*( max(fDist-MAX_DMG_RANGE,0) / (fRange-MAX_DMG_RANGE));
//							pTar->OnDamagedKatanaSplash( pOwnerCharacter, fDamageRange );

#define SPLASH_DAMAGE_RATIO	.4f		// 스플래시 데미지 관통률
#define SLASH_DAMAGE	3		// 강베기데미지 = 일반공격의 x SLASH_DAMAGE
							int damage = (int) pDesc->m_nDamage * fDamageRange;
							
							// 인챈트 속성이 있을때는 1배 데미지만 먹는다. 2005.1.14
							if(zc_en_type == ZC_ENCHANT_NONE)
								damage *=  SLASH_DAMAGE;

							pTar->OnDamaged(pOwnerCharacter,pOwnerCharacter->GetPosition(),ZD_KATANA_SPLASH,MWT_KATANA,damage,SPLASH_DAMAGE_RATIO);
							pTar->OnDamagedAnimation(pOwnerCharacter,SEM_WomanSlash5);

							ZPostPeerEnchantDamage(pOwnerCharacter->GetUID(), pTar->GetUID());

						} // 데미지를 먹는다
					}
				}
			}
		}
#define KATANA_SHOCK_RANGE		1000.f			// 10미터까지 흔들린다

		float fPower= (KATANA_SHOCK_RANGE-Magnitude(g_pGame->m_pMyCharacter->GetPosition()+rvector(0,0,50) - OwnerPosition))/KATANA_SHOCK_RANGE;
		if(fPower>0)
			ZGetGameInterface()->GetCamera()->Shock(fPower*500.f, .5f, rvector(0.0f, 0.0f, -1.0f));

	}
	/*
	else{

#ifndef _PUBLISH

		// 이거 칼질 제대로 안한넘이다. 수상하다.
		char szTemp[256];
		sprintf_s(szTemp, "%s 치트 ?", pOwnerCharacter->GetProperty()->szName);
		ZChatOutput(MCOLOR(0xFFFF0000), szTemp);

		mlog("anistate %d\n",pOwnerCharacter->m_AniState_Lower);

#endif//_PUBLISH

	}
	*/
}

void ZGameAction::OnPeerSkill_Uppercut(ZCharacter *pOwnerCharacter)
{
	if (!g_Rules.CanFlip())
		return;
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SKILLMAP)
		return;

	float fShotTime=g_pGame->GetTime();
	rvector OwnerPosition,OwnerDir;
	OwnerPosition = pOwnerCharacter->GetPosition();
	OwnerDir = pOwnerCharacter->m_Direction;
	OwnerDir.z=0; 
	Normalize(OwnerDir);

	if (!pOwnerCharacter->IsNPC())
	{
		if (pOwnerCharacter->GetProperty()->nSex == MMS_MALE)
			ZGetSoundEngine()->PlaySound("fx2/MAL_shot_01", pOwnerCharacter->GetPosition());
		else
			ZGetSoundEngine()->PlaySound("fx2/FEM_shot_01", pOwnerCharacter->GetPosition());
	}

	for (ZObjectManager::iterator itor = ZGetObjectManager()->begin();
		itor != ZGetObjectManager()->end(); ++itor)
	{
		ZObject* pTar = (*itor).second;
		if (pOwnerCharacter == pTar) continue;

		rvector TargetPosition,TargetDir;

		if(pTar->IsDie()) continue;
		// 적절한 위치를 얻어낼수 없으면 다음으로~
		if( !pTar->GetHistory(&TargetPosition,&TargetDir,fShotTime)) continue;

		float fDist = Magnitude(OwnerPosition + OwnerDir*10.f - TargetPosition);

		if (fDist < 200.0f) {

			if ((pTar) && (pTar != pOwnerCharacter))
			{
				bool bCheck = false;

				if (ZApplication::GetGame()->GetMatch()->IsTeamPlay())
				{
					if (IsPlayerObject(pTar)) {
						if( pOwnerCharacter->IsTeam( (ZCharacter*)pTar ) == false){
							bCheck = true;
						}
					}
					else {
						bCheck = true;
					}
				}
				else if (ZApplication::GetGame()->GetMatch()->IsQuestDrived())
				{
					if (!IsPlayerObject(pTar)) bCheck = true;
				}
				else {
					bCheck = true;
				}

				if(g_pGame->CheckWall(pOwnerCharacter,pTar)==true) //중간에 벽이 막고 있는가?
					bCheck = false;

				if( bCheck) {//팀이아닌경우만

					rvector fTarDir = pTar->GetPosition() - (pOwnerCharacter->GetPosition() - 50.f*OwnerDir);
					Normalize(fTarDir);
					float fDot = D3DXVec3Dot(&OwnerDir, &fTarDir);
					if (fDot>0)
					{
						int cm = g_pGame->SelectSlashEffectMotion(pOwnerCharacter);//남녀 칼 휘두르는 방향

						rvector tpos = pTar->GetPosition();

						tpos.z += 130.f;

						/*
						if (IsPlayerObject(pTar))
						{
							// 우선 플레이어만 이펙트가 나온다. - effect 다 정리하고 NPC도 나오게 바뀌어야 함 -bird
							ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos,(ZCharacter*)pTar);
						}
						*/

						tpos -= pOwnerCharacter->m_Direction * 50.f;

						ZGetEffectManager()->AddBloodEffect( tpos , -fTarDir);
						ZGetEffectManager()->AddSlashEffect( tpos , -fTarDir , cm );

						g_pGame->CheckCombo(pOwnerCharacter, pTar , true);
						if (pTar == g_pGame->m_pMyCharacter) 
						{
							g_pGame->m_pMyCharacter->SetLastThrower(pOwnerCharacter->GetUID(), g_pGame->GetTime()+1.0f);
							ZPostReaction(g_pGame->GetTime(),ZR_BE_UPPERCUT);
						}
						pTar->OnBlast(OwnerDir);

						if (!pTar->IsNPC())
						{
							if (((ZCharacter*)pTar)->GetProperty()->nSex == MMS_MALE)
								ZGetSoundEngine()->PlaySound("fx2/MAL07", pTar->GetPosition());
							else
								ZGetSoundEngine()->PlaySound("fx2/FEM07", pTar->GetPosition());
						}
					}
				}
			}
		}
	}
}

void ZGameAction::OnPeerSkill_Dash(ZCharacter *pOwnerCharacter)
{
	if(pOwnerCharacter->m_AniState_Lower!=ZC_STATE_LOWER_UPPERCUT) return;

	float fShotTime=g_pGame->GetTime();
	rvector OwnerPosition,OwnerDir;
	OwnerPosition = pOwnerCharacter->GetPosition();
	OwnerDir = pOwnerCharacter->m_Direction;
	OwnerDir.z=0; 
	Normalize(OwnerDir);

	ZItem *pItem = pOwnerCharacter->GetItems()->GetItem(MMCIP_MELEE);
	if(!pItem) return;
	MMatchItemDesc *pDesc = pItem->GetDesc();
	if(!pDesc) { _ASSERT(FALSE); return; }

//	ZGetEffectManager()->AddSkillDashEffect(pOwnerCharacter->GetPosition(),pOwnerCharacter->m_Direction,pOwnerCharacter);

//	for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();
//		itor != ZGetCharacterManager()->end(); ++itor)
	for (ZObjectManager::iterator itor = ZGetObjectManager()->begin();
		itor != ZGetObjectManager()->end(); ++itor)
	{
//		ZCharacter* pTar = (*itor).second;
		ZObject* pTar = (*itor).second;

		if (pOwnerCharacter == pTar) continue;

		rvector TargetPosition,TargetDir;

		if(pTar->IsDie()) continue;

		// 적절한 위치를 얻어낼수 없으면 다음으로~
		if( !pTar->GetHistory(&TargetPosition,&TargetDir,fShotTime)) continue;

		float fDist = Magnitude(OwnerPosition + OwnerDir*10.f - TargetPosition);

		if (fDist < 600.0f) {// 6m

			if ((pTar) && (pTar != pOwnerCharacter)) {

				bool bCheck = false;
/*
				if (ZApplication::GetGame()->GetMatch()->IsTeamPlay()){
					if( pOwnerCharacter->IsTeam( pTar ) == false){
						bCheck = true;
					}
				}
				else {
					bCheck = true;
				}
*/
				if (ZApplication::GetGame()->GetMatch()->IsTeamPlay()){
					if (IsPlayerObject(pTar)) {
						if( pOwnerCharacter->IsTeam( (ZCharacter*)pTar ) == false){
							bCheck = true;
						}
					}
					else {
						bCheck = true;
					}
				}
				else {
					bCheck = true;
				}

				if(g_pGame->CheckWall(pOwnerCharacter,pTar)==true) //중간에 벽이 막고 있는가?
					bCheck = false;

				if( bCheck) {//팀이아닌경우만
					//				if( pOwnerCharacter->IsTeam( pTar ) == false) {//팀이아닌경우만

					rvector fTarDir = pTar->GetPosition() - pOwnerCharacter->GetPosition();
					Normalize(fTarDir);

					float fDot = D3DXVec3Dot(&OwnerDir, &fTarDir);

					bool bDamage = false;

					if( fDist < 100.f) { // 1m 안쪽은 앞에만 있어도..
						if(fDot > 0.f) {
							bDamage = true;
						}
					}
					else if(fDist < 300.f) {
						if(fDot > 0.5f) {
							bDamage = true;
						}
					}
					else {// 2m ~ 6m
						if(fDot > 0.96f) {
							bDamage = true;
						}
					}

					if ( bDamage ) {

						int cm = g_pGame->SelectSlashEffectMotion(pOwnerCharacter);//남녀 칼 휘두르는 방향

						float add_time = 0.3f * (fDist / 600.f);
						float time = g_pGame->GetTime() + add_time;			// 거리에 따라서 시간을 달리하도록 수정하기..

						rvector tpos = pTar->GetPosition();

						tpos.z += 180.f;//좀더 높임

						ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos,pTar->GetUID(),(DWORD)(add_time*1000));
//						ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos,pTar);

						tpos -= pOwnerCharacter->m_Direction * 50.f;

//						ZGetEffectManager()->AddBloodEffect( tpos , -fTarDir);
//						ZGetEffectManager()->AddSlashEffect( tpos , -fTarDir , cm );
						// 소리도 특정 시간 뒤에
						ZGetSoundEngine()->PlaySound("uppercut", tpos );

						if (pTar == g_pGame->m_pMyCharacter) {
							rvector _dir = pTar->GetPosition() - pOwnerCharacter->GetPosition();
							_dir.z = 0.f;

//							m_pMyCharacter->OnDashAttacked(_dir);
							g_pGame->m_pMyCharacter->ReserveDashAttacked( pOwnerCharacter->GetUID(), time,_dir );
						}
						pTar->OnBlastDagger(OwnerDir,OwnerPosition);

						float fDamage = pDesc->m_nDamage * 1.5f;// 기본 무기데미지 * 150 %
						float fRatio = pItem->GetPiercingRatio( pDesc->m_nWeaponType , eq_parts_chest );

						if(ZGetGame()->IsAttackable(pOwnerCharacter,pTar))//공격 가능한 경우만.. 퀘스트의 경우 데미지는 없다..
							pTar->OnDamagedSkill(pOwnerCharacter,pOwnerCharacter->GetPosition(),ZD_MELEE,MWT_DAGGER,fDamage,fRatio);

						g_pGame->CheckCombo(pOwnerCharacter, pTar,true);
					}

				}//IsTeam
			}
		}
	}
}


bool ZGameAction::OnEnchantDamage(MCommand* pCommand)
{
	MUID ownerUID;
	MUID targetUID;
	pCommand->GetParameter(&ownerUID,	0, MPT_UID);
	pCommand->GetParameter(&targetUID,	1, MPT_UID);

	ZCharacter* pOwnerCharacter = ZGetCharacterManager()->Find(ownerUID);
	ZObject* pTarget= ZGetObjectManager()->GetObject(targetUID);

	if (pOwnerCharacter == NULL || pTarget == NULL ) return true;

	bool bMyChar = (targetUID == ZGetMyUID());
	
	MMatchItemDesc* pDesc = pOwnerCharacter->GetEnchantItemDesc();
	if(pDesc)
	{
		bool bObserverTarget = pTarget->GetUID()==ZGetCombatInterface()->GetTargetUID();
		rvector soundPos = pTarget->GetPosition();
		// 데미지는 내가 HP를 관리하는 캐릭터에 한해서 준다
		switch(pOwnerCharacter->GetEnchantType())
		{
			case ZC_ENCHANT_FIRE : {
					ZGetSoundEngine()->PlaySound(bObserverTarget ? "we_enfire_2d" : "we_enfire",soundPos);
					ZModule_FireDamage *pMod = (ZModule_FireDamage*)pTarget->GetModule(ZMID_FIREDAMAGE);
					if(pMod) pMod->BeginDamage( pOwnerCharacter->GetUID(), bMyChar ? pDesc->m_nDamage : 0 ,0.001f * (float)pDesc->m_nDelay);
				}break;
			case ZC_ENCHANT_COLD : {
				ZGetSoundEngine()->PlaySound(bObserverTarget ? "we_enice_2d" : "we_enice",soundPos);
					ZModule_ColdDamage *pMod = (ZModule_ColdDamage*)pTarget->GetModule(ZMID_COLDDAMAGE);
					if(pMod) pMod->BeginDamage( 0.01f*(float)pDesc->m_nLimitSpeed , 0.001f * (float)pDesc->m_nDelay);
				}break;
			case ZC_ENCHANT_POISON : {
					ZGetSoundEngine()->PlaySound(bObserverTarget ? "we_enpoison_2d" : "we_enpoison",soundPos);
					ZModule_PoisonDamage *pMod = (ZModule_PoisonDamage*)pTarget->GetModule(ZMID_POISONDAMAGE);
					if(pMod) pMod->BeginDamage( pOwnerCharacter->GetUID(), bMyChar ? pDesc->m_nDamage : 0 ,0.001f * (float)pDesc->m_nDelay);
				}break;
			case ZC_ENCHANT_LIGHTNING : {
					ZGetSoundEngine()->PlaySound(bObserverTarget ? "we_enlight_2d" : "we_enlight",soundPos);
					ZModule_LightningDamage *pMod = (ZModule_LightningDamage*)pTarget->GetModule(ZMID_LIGHTNINGDAMAGE);
					if(pMod) pMod->BeginDamage( pOwnerCharacter->GetUID(), bMyChar ? pDesc->m_nDamage : 0 ,0.001f * (float)pDesc->m_nDelay);
				}break;
		};
	}

	return true;
}