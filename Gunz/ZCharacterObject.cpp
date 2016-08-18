#include "stdafx.h"
#include "ZCharacterObject.h"
#include "ZMyCharacter.h"
#include "ZNetCharacter.h"
#include "ZActor.h"
#include "RCollisionDetection.h"
#include "ZModule_HPAP.h"
#include "ZGame.h"
#include "ZEffectStaticMesh.h"
#include "ZEffectAniMesh.h"
#include "ZScreenEffectManager.h"
#include "zshadow.h"
#include "ZConfiguration.h"

MImplementRTTI(ZCharacterObject, ZObject);

sCharacterLight	g_CharLightList[NUM_LIGHT_TYPE];

ZCharacterObject::ZCharacterObject() : ZObject()
{
	m_pshadow = NULL;
	m_bDynamicLight = false;

	m_iDLightType = 0;
	m_fLightLife = 0.f;

	m_fTime = 0.f;

	m_pSoundMaterial[0] = 0;

	m_bHero = false;
	m_fTremblePower = 30.0f;		// 캐릭터 기본값
}

ZCharacterObject::~ZCharacterObject()
{

}

bool ZCharacterObject::CreateShadow()
{
	m_pshadow = new ZShadow;
	return true;
}

void ZCharacterObject::DestroyShadow()
{
	SAFE_DELETE( m_pshadow );
}

bool ZCharacterObject::GetWeaponTypePos(WeaponDummyType type,rvector* pos,bool bLeft)
{
	if(m_pVMesh && pos) {
		return m_pVMesh->GetWeaponDummyPos(type,pos,bLeft);
	}
	return false;
}

void ZCharacterObject::UpdateEnchant()
{
	ZC_ENCHANT etype = GetEnchantType();
	REnchantType retype = REnchantType_None;

		 if(etype==ZC_ENCHANT_FIRE)			retype = REnchantType_Fire;
	else if(etype==ZC_ENCHANT_COLD)			retype = REnchantType_Cold;
	else if(etype==ZC_ENCHANT_LIGHTNING)	retype = REnchantType_Lightning;
	else if(etype==ZC_ENCHANT_POISON)		retype = REnchantType_Poison;
	else									retype = REnchantType_None;
	
	if(m_pVMesh) {
		m_pVMesh->SetEnChantType(retype);
	}
}

void ZCharacterObject::DrawEnchantSub(ZC_ENCHANT etype,rvector& pos)
{
	if(etype==ZC_ENCHANT_FIRE)
		ZGetEffectManager()->AddTrackFire( pos );
	else if(etype==ZC_ENCHANT_COLD)
		ZGetEffectManager()->AddTrackCold( pos );
	else if(etype==ZC_ENCHANT_POISON)
		ZGetEffectManager()->AddTrackPoison( pos );
//	else if(etype==ZC_ENCHANT_LIGHTNING)
}

// 칼들고 뛰어다닐떄..

void ZCharacterObject::EnChantMovingEffect(rvector* pOutPos,int cnt,ZC_ENCHANT etype,bool bDoubleWeapon)
{
	int nRand = (GetEffectLevel()+1) * 3;//이펙트 옵션에 따라서..

	if( g_pGame->GetGameTimer()->m_dwUpdateCnt % nRand == 0 ) {

		if(cnt==0) { 
			m_pVMesh->GetWeaponPos( pOutPos ); 	
		}

		float asf = (rand()%10)/10.f;
		rvector pos = pOutPos[0] + (pOutPos[1]-pOutPos[0]) * asf;

		DrawEnchantSub( etype , pos );

		if(bDoubleWeapon) // 양손일경우
		{
			if(cnt==0) {
				m_pVMesh->GetWeaponPos( &pOutPos[2] ,true);
			}

			float asf = (rand()%10)/10.f;
			rvector pos = pOutPos[2] + (pOutPos[3]-pOutPos[2]) * asf;

			DrawEnchantSub( etype , pos );
		}
	}
}

// 무기 휘두를때
void ZCharacterObject::EnChantSlashEffect(rvector* pOutPos,int cnt,ZC_ENCHANT etype,bool bDoubleWeapon)
{
	if(cnt==0) {
		m_pVMesh->GetWeaponPos( pOutPos );
	}

	float asf = (3 + rand()%3)/10.f;
	rvector pos = pOutPos[0] + (pOutPos[1]-pOutPos[0]) * asf;

	DrawEnchantSub( etype , pos );

	if(bDoubleWeapon) // 양손일경우
	{
		if(cnt==0) {
			m_pVMesh->GetWeaponPos( &pOutPos[2] ,true);
		}

		float asf = (3 + rand()%3)/10.f;
		rvector pos = pOutPos[2] + (pOutPos[3]-pOutPos[2]) * asf;

		DrawEnchantSub( etype , pos );
	}
}

void ZCharacterObject::EnChantWeaponEffect(ZC_ENCHANT etype,int nLevel)
{
	float fSwordSize = m_pVMesh->GetWeaponSize();

	fSwordSize -= 35.f;//손잡이 크기 정도를 빼준다...

	ZEffectWeaponEnchant* pEWE = NULL; 

	if( etype != ZC_ENCHANT_NONE )
		pEWE = ZGetEffectManager()->GetWeaponEnchant( etype );

	if(pEWE) {

		//표준 사이즈는 카타나... 105 정도..

		float fSIze = fSwordSize / 100.f;

		float fVolSize = 1.f;

			 if(nLevel==0) fVolSize = 0.6f;
		else if(nLevel==1) fVolSize = 0.75f;
		else if(nLevel==2) fVolSize = 1.0f;
		else			   fVolSize = 1.25f;

		rvector vScale = rvector(0.7f*fSIze*fVolSize,0.7f*fSIze*fVolSize,1.1f*fSIze);// 무기의 크기에 따라서..
		pEWE->SetUid( m_UID );
		pEWE->SetAlignType(1);
		pEWE->SetScale(vScale);
		pEWE->Draw(GetGlobalTimeMS());
	}
}

void ZCharacterObject::DrawEnchant(ZC_STATE_LOWER AniState_Lower,bool bCharged)
{
	ZItem* pItem = GetItems()->GetSelectedWeapon();

	if(!pItem || !pItem->GetDesc()) {
//		_ASSERT(0);
		return;
	}

	MMatchItemDesc* pDesc = pItem->GetDesc();

	if (pDesc->m_nType != MMIT_MELEE) 
		return;

	if(m_pVMesh) {

		static rvector pOutPos[8];//보통 양손 최대 4개까지만 쓴다.

		bool bDoubleWeapon = m_pVMesh->IsDoubleWeapon();

		int cnt = m_pVMesh->GetLastWeaponTrackPos(pOutPos);

		bool bSlash = false;

		if( (AniState_Lower == ZC_STATE_SLASH) ||
			(AniState_Lower == ZC_STATE_JUMP_SLASH1) ||
			(AniState_Lower == ZC_STATE_JUMP_SLASH2) )
			bSlash = true;

		ZC_ENCHANT etype = GetEnchantType();

//		etype = ZC_ENCHANT_POISON;
//		etype = ZC_ENCHANT_COLD;
//		etype = ZC_ENCHANT_FIRE;
//		etype = ZC_ENCHANT_LIGHTNING;

		MMatchItemDesc* pENDesc = GetEnchantItemDesc();
		if(pENDesc) {

			int nEFLevel = pENDesc->m_nEffectLevel;

	//		nEFLevel = 3;

			if( (nEFLevel > 2) || ((nEFLevel > 1) && bCharged ) )			// 3은 항상보이고 2는 기모았을때 달리면서도 뿌리고 다님..
			{
				EnChantMovingEffect(pOutPos,cnt,etype,bDoubleWeapon);		// 칼들고 뛰어다닐떄..
			}

			if( bSlash )													// 칼 휘두를떄...
			{ 
				EnChantSlashEffect(pOutPos,cnt,etype,bDoubleWeapon);
			}

			if( (nEFLevel > 1) || bCharged )								// 이펙트 레벨이 1보다 크거나 기가 모인 상태라면 인첸트 이펙트
			{
				EnChantWeaponEffect(etype,nEFLevel);
			}
		}
	}
}

MMatchItemDesc* ZCharacterObject::GetEnchantItemDesc()
{
	for(int i=MMCIP_CUSTOM1;i<=MMCIP_CUSTOM2;i++) {
		ZItem *pItem = m_Items.GetItem((MMatchCharItemParts)i);
		MMatchItemDesc* pDesc = pItem->GetDesc();
		if(pDesc && pDesc->IsEnchantItem() ) return pDesc;
	}

	return NULL;
}

ZC_ENCHANT	ZCharacterObject::GetEnchantType()
{
//	return ZC_ENCHANT_FIRE;

	MMatchItemDesc* pDesc = GetEnchantItemDesc();
	if(pDesc)
	{
		switch(pDesc->m_nWeaponType)
		{
			case MWT_ENCHANT_FIRE : return ZC_ENCHANT_FIRE;
			case MWT_ENCHANT_COLD : return ZC_ENCHANT_COLD;
			case MWT_ENCHANT_LIGHTNING: return ZC_ENCHANT_LIGHTNING;
			case MWT_ENCHANT_POISON: return ZC_ENCHANT_POISON;
		}
	}

	return ZC_ENCHANT_NONE;
}

void ZCharacterObject::Draw_SetLight(rvector& vPosition)
{
	//	광원정의
	//	0번 - ambient : 캐릭터의 기본 밝기
	//		- diffuse : 순간적인 광원( 총류를 발사할때 적용)
	//	1번 - ambient : 사용안함
	//		- diffuse : 맵의 디퓨즈 라이트
	//	specular, emit 속성은 사용안함

#define CHARACTER_AMBIENT	0.0
	D3DLIGHT9 Light;
	rvector pos;
	rvector char_pos = vPosition;
	char_pos.z += 180.f;

	memset( &Light, 0, sizeof( D3DLIGHT9 ));	// 초기화

	//	0번 라이트 begine
	Light.Type		= D3DLIGHT_POINT;
	Light.Ambient.r = CHARACTER_AMBIENT;
	Light.Ambient.g = CHARACTER_AMBIENT;
	Light.Ambient.b = CHARACTER_AMBIENT;

	Light.Specular.r = 1.f;
	Light.Specular.g = 1.f;
	Light.Specular.b = 1.f;
	Light.Specular.a = 1.f;

	if( ZGetConfiguration()->GetVideo()->bDynamicLight && m_bDynamicLight )
	{			
		m_vLightColor.x -= 0.03f;
		m_vLightColor.y -= 0.03f;
		m_vLightColor.z -= 0.03f;
		max( m_vLightColor.x, 0.0f );
		max( m_vLightColor.y, 0.0f );
		max( m_vLightColor.z, 0.0f );
		Light.Diffuse.r	= m_vLightColor.x;
		Light.Diffuse.g	= m_vLightColor.y;
		Light.Diffuse.b	= m_vLightColor.z;
		Light.Range = g_CharLightList[m_iDLightType].fRange;

		float lastTime	= m_fTime;
		m_fTime			= GetGlobalTimeMS();
		float lap		= m_fTime - lastTime;
		m_fLightLife	-= lap;

		if( m_fLightLife <= 0.0f )
		{
			m_bDynamicLight = false;
			m_fLightLife	= 0;
		}
	}
	else
	{
		m_bDynamicLight = false;
		m_vLightColor.x = 0.0f;
		m_vLightColor.y = 0.0f;
		m_vLightColor.z = 0.0f;
	}

	if( IsDoubleGun() )
	{
		if( m_bLeftShot ) GetWeaponTypePos( weapon_dummy_muzzle_flash, &pos ,true);
		else			  GetWeaponTypePos( weapon_dummy_muzzle_flash, &pos );

		m_bLeftShot = !m_bLeftShot;
	}
	else 
	{
		GetWeaponTypePos( weapon_dummy_muzzle_flash, &pos );
	}
	Light.Position.x	= pos.x ;
	Light.Position.y	= pos.y ;
	Light.Position.z	= pos.z ;

	Light.Attenuation1	= 0.05f;
	Light.Attenuation2	= 0.001f;

	if (IsNPC())
	{
		Light.Ambient.r = 0.4f;
		Light.Ambient.g = 0.4f;
		Light.Ambient.b = 0.4f;
		Light.Range = 2000.0f;

		Light.Attenuation0 = 1.0f;
		Light.Attenuation1 = 0.0f;
		Light.Attenuation2 = 0.0f;
	}

	m_pVMesh->SetLight(0,&Light,false);

	ZeroMemory( &Light, sizeof(D3DLIGHT9) );
	RLIGHT* pSelectedLight = 0;
	rvector sunDir;
	float distance;
	float SelectedLightDistance = FLT_MAX;

	Light.Specular.r = 1.f;
	Light.Specular.g = 1.f;
	Light.Specular.b = 1.f;
	Light.Specular.a = 1.f;

	auto& LightList = ZGetGame()->GetWorld()->GetBsp()->GetSunLightList();
	if(!LightList.empty())
	{
		for (auto& Light : LightList)
		{
			sunDir	= Light.Position - char_pos;
			distance	= D3DXVec3LengthSq( &sunDir );
			D3DXVec3Normalize( &sunDir, &sunDir );
			RBSPPICKINFO info;
			if( ZGetGame()->GetWorld()->GetBsp()->Pick( char_pos, sunDir, &info,RM_FLAG_ADDITIVE  ) )
			{
				auto vec = char_pos - info.PickPos;
				if( distance > D3DXVec3LengthSq(&vec) )
				{
					continue;
				}
			}
			if( distance < SelectedLightDistance )
			{
				SelectedLightDistance = distance;
				pSelectedLight = &Light;
			}
		}
		Light.Type       = D3DLIGHT_POINT;
		Light.Attenuation1 = 0.00001f;	
		if( pSelectedLight != 0 )
		{
			Light.Diffuse.r	= pSelectedLight->Color.x * pSelectedLight->fIntensity * 0.15f;
			Light.Diffuse.g  = pSelectedLight->Color.y * pSelectedLight->fIntensity * 0.15f;
			Light.Diffuse.b  = pSelectedLight->Color.z * pSelectedLight->fIntensity * 0.15f;
		}
	}

	SelectedLightDistance = FLT_MAX ;
	if( pSelectedLight == 0 )
	{
		for(auto& Light : ZGetGame()->GetWorld()->GetBsp()->GetObjectLightList())
		{
			float fDist = Magnitude(Light.Position - char_pos);
			if( SelectedLightDistance > fDist )
			{
				SelectedLightDistance = fDist;
				pSelectedLight = &Light;
			}
		}

		Light.Type       = D3DLIGHT_POINT;
		Light.Attenuation1 = 0.0025f;

		if(pSelectedLight)
		{
			Light.Diffuse.r  = pSelectedLight->Color.x * pSelectedLight->fIntensity;
			Light.Diffuse.g  = pSelectedLight->Color.y * pSelectedLight->fIntensity;
			Light.Diffuse.b  = pSelectedLight->Color.z * pSelectedLight->fIntensity;
		}
	}

	if( pSelectedLight != NULL )
	{			
		Light.Position = pSelectedLight->Position;
		Light.Range       = pSelectedLight->fAttnEnd;

		m_pVMesh->SetLight(1,&Light,false);
	}
	else
		m_pVMesh->SetLight(1,NULL,false);

	RGetShaderMgr()->setAmbient( 0x00cccccc );
	RGetDevice()->SetRenderState( D3DRS_LIGHTING, TRUE );
	RGetDevice()->SetRenderState( D3DRS_AMBIENT, 0x00cccccc );
}

void ZCharacterObject::DrawShadow()
{
	__BP(28, "ZCharacter::Draw::Shadow");

	if(!m_pshadow) return;

	//RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
	if(!IsDie())
	{
		float fSize = 100.f;//기본..
		
		ZActor* pActor = MDynamicCast(ZActor,this);

		if(pActor) { 
			fSize = pActor->GetNPCInfo()->fCollRadius * 3.0f;
		}

		if(m_pshadow->setMatrix( *this ,fSize ))
			m_pshadow->draw();
	}

	__EP(28);
}


bool ZCharacterObject::IsDoubleGun() { 

	if(m_pVMesh) {
		if(m_pVMesh->m_SelectWeaponMotionType==eq_wd_pistol) {
			return true;
		} else if(m_pVMesh->m_SelectWeaponMotionType==eq_wd_smg) {
			return true;
		}
	}
	return false;
}

int ZCharacterObject::GetWeapondummyPos(rvector* v ) 
{
	if(!v) return 3;

	int size = 3;//기본..

	if(!GetWeaponTypePos(weapon_dummy_muzzle_flash,&v[0],false)) {	}
	if(!GetWeaponTypePos(weapon_dummy_cartridge01,&v[1],false)) { v[1]=v[0]; }
	if(!GetWeaponTypePos(weapon_dummy_cartridge02,&v[2],false)) { v[2]=v[0]; }

	//양손총..
	if( IsDoubleGun() ) {

		size = 6;

		if(!GetWeaponTypePos(weapon_dummy_muzzle_flash,&v[3],true)) { }
		if(!GetWeaponTypePos(weapon_dummy_cartridge01,&v[4],true))  { v[4]=v[3]; }
		if(!GetWeaponTypePos(weapon_dummy_cartridge02,&v[5],true))  { v[5]=v[4]; }
	}

	return size;
}

bool ZCharacterObject::GetCurrentWeaponDirection(rvector* dir)
{
	if(m_pVMesh && dir) {

		rmatrix* mat = &m_pVMesh->m_WeaponDummyMatrix[weapon_dummy_muzzle_flash];//총구

		dir->x = mat->_21;
		dir->y = mat->_22;
		dir->z = mat->_23;

		return true;
	}
	return false;
}


#define MAX_KNOCKBACK_VELOCITY		1700.f

void ZCharacterObject::OnKnockback(const rvector& dir, float fForce)
{
	AddVelocity(dir * fForce);

	// 넉백 최대속도에 맞춘다
	rvector vel = GetVelocity();
	if (Magnitude(vel) > MAX_KNOCKBACK_VELOCITY) {
		Normalize(vel);
		vel *= MAX_KNOCKBACK_VELOCITY;
		SetVelocity(vel);
	}


	// 타격감 - 꿈틀
	rvector dir1 = m_Direction;
	rvector dir2 = dir;
	Normalize(dir2);

	float cosAng1 = DotProduct(dir1, dir2);
	float fMaxValue = m_fTremblePower;

	if (cosAng1 < 0.f)	{
		fMaxValue = -fMaxValue;
	}
	Tremble(fMaxValue, 50, 100);
}

MImplementRTTI(ZCharacterObjectHistory, ZCharacterObject);

void ZCharacterObjectHistory::EmptyHistory()
{
	while (m_BasicHistory.size())
	{
		delete *m_BasicHistory.begin();
		m_BasicHistory.erase(m_BasicHistory.begin());
	}
}

bool ZCharacterObjectHistory::GetHistory(rvector *pos, rvector *direction, float fTime)
{
	if (GetVisualMesh() == NULL)
		return false;

	auto SetReturnValues = [&](const rvector& Pos, const rvector& Dir)
	{
		if (isnan(Pos.x) || isnan(Pos.y) || isnan(Pos.z))
			return false;

		if (isnan(Dir.x) || isnan(Dir.y) || isnan(Dir.z))
			return false;

		if (pos)
			*pos = Pos;
		if (direction)
			*direction = Dir;

		return true;
	};

	if (m_BasicHistory.size() > 1)
	{
		ZBasicInfoHistory::iterator hi;
		hi = m_BasicHistory.end();

		ZBasicInfoItem *bi = NULL, *binext = NULL;

		do {
			hi--;
			binext = bi;
			bi = *hi;
		} while (hi != m_BasicHistory.begin() && bi->fSendTime > fTime);

		if (fTime < bi->fSendTime)
			return false;

		ZBasicInfoItem *pnext, next;

		if (!binext)
		{
			if (fTime >= g_pGame->GetTime())
			{
				return SetReturnValues(m_Position, m_Direction);
			}

			next.fSendTime = g_pGame->GetTime();
			next.info.position = m_Position;
			next.info.direction = m_Direction;
			pnext = &next;
		}
		else
		{
			pnext = binext;
		}

		float t = (fTime - bi->fSendTime) / (pnext->fSendTime - bi->fSendTime);

		return SetReturnValues(Lerp(bi->info.position, pnext->info.position, t),
			Slerp(bi->info.direction, pnext->info.direction, t));
	}
	else
	{
		return SetReturnValues(m_Position, m_Direction);
	}

	return false;
}