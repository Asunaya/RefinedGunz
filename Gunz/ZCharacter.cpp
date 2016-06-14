#include "stdafx.h"

#include "ZApplication.h"
#include "ZGame.h"
#include "ZCharacter.h"
#include "RVisualMeshMgr.h"
#include "RealSpace2.h"
#include "MDebug.h"
#include "MObject.h"
#include "ZPost.h"
#include "ZGameInterface.h"
#include "RBspObject.h"
#include "zshadow.h"
#include "MProfiler.h"
#include "RShaderMgr.h"
#include "ZScreenEffectManager.h"
#include "RDynamicLight.h"
#include "ZConfiguration.h"
#include "MMatchObject.h"
#include "RCollisionDetection.h"
#include "ZEffectStaticMesh.h"
#include "ZEffectAniMesh.h"
#include "ZModule_HPAP.h"
#include "ZModule_Movable.h"
#include "ZModule_Resistance.h"
#include "ZModule_FireDamage.h"
#include "ZModule_ColdDamage.h"
#include "ZModule_LightningDamage.h"
#include "ZModule_PoisonDamage.h"
#include "ZModule_QuestStatus.h"
#include "ZGameConst.h"

#include "RGMain.h"
#include "Portal.h"
#include "Rules.h"

#define ANGLE_TOLER			.1f
#define ANGLE_SPEED			12.f

#define ROLLBACK_TOLER		20.f

#define ENABLE_CHARACTER_COLLISION

//#ifdef _PUBLISH
#define ENABLE_FALLING_DAMAGE
//#endif

#define IsKeyDown(key) ((GetAsyncKeyState(key) & 0x8000)!=0)
//////////////////////////////////////////////////////////////////////////
//	GLOBAL
//////////////////////////////////////////////////////////////////////////

bool Enable_Cloth	= true;

// teen 버전은 강제로 바꿔준다~..
bool CheckTeenVersionMesh(RMesh** ppMesh)
{
	RWeaponMotionType type = eq_weapon_etc;

	type = (*ppMesh)->GetMeshWeaponMotionType();

	if( ZApplication::GetGameInterface()->GetTeenVersion() ) {

		if(type==eq_wd_katana) {
			*ppMesh = ZGetWeaponMeshMgr()->Get( "katana_wood" );
			return true;
		}
		else if(type==eq_ws_dagger) {
			*ppMesh = ZGetWeaponMeshMgr()->Get( "dagger_wood" );
			return true;
		}
		else if(type==eq_wd_sword) {
			*ppMesh = ZGetWeaponMeshMgr()->Get( "sword_wood" );
			return true;
		
		}
		else if(type==eq_wd_blade) {
			*ppMesh = ZGetWeaponMeshMgr()->Get( "blade_wood" );
			return true;
		}
	}

	return false;
}

void ChangeEquipParts(RVisualMesh* pVMesh, unsigned long int* pItemID)
{
	pVMesh->ClearParts();

	struct _ZPARTSPAIR
	{
		_RMeshPartsType			meshparts;
		MMatchCharItemParts		itemparts;
	};

	static _ZPARTSPAIR PartsPair[] = 
	{
		{eq_parts_head, MMCIP_HEAD},
		{eq_parts_chest, MMCIP_CHEST},
		{eq_parts_hands, MMCIP_HANDS},
		{eq_parts_legs, MMCIP_LEGS},
		{eq_parts_feet, MMCIP_FEET}
	};

	for (int i = 0; i < 5; i++)
	{
		if (pItemID[PartsPair[i].itemparts] != 0)
		{
			MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(pItemID[PartsPair[i].itemparts]);
			if (pDesc != NULL)
			{
				pVMesh->SetParts(PartsPair[i].meshparts, pDesc->m_szMeshName);
			}
			else
			{

			}
		}
		else
		{
			pVMesh->SetBaseParts( PartsPair[i].meshparts );
		}
	}

	pVMesh->SetBaseParts(eq_parts_face);
}

void ChangeCharFace(RVisualMesh* pVMesh, MMatchSex nSex, int nFaceIndex)
{
	if ((nFaceIndex < 0) || (nFaceIndex >= MAX_COSTUME_FACE)) return;
	if (pVMesh == NULL) return;

	char szMeshName[256];
	
	if (nSex == MMS_MALE)
	{
		strcpy_safe(szMeshName, g_szFaceMeshName[nFaceIndex][MMS_MALE].c_str());
	}
	else
	{
		strcpy_safe(szMeshName, g_szFaceMeshName[nFaceIndex][MMS_FEMALE].c_str());
	}

	pVMesh->SetParts(eq_parts_face, szMeshName);
}

void ChangeCharHair(RVisualMesh* pVMesh, MMatchSex nSex, int nHairIndex)
{
	if ((nHairIndex < 0) || (nHairIndex >= MAX_COSTUME_HAIR)) return;
	if (pVMesh == NULL) return;

	char szMeshName[256];
	if (nSex == MMS_MALE)
	{
		strcpy_safe(szMeshName, g_szHairMeshName[nHairIndex][MMS_MALE].c_str());
	}
	else
	{
		strcpy_safe(szMeshName, g_szHairMeshName[nHairIndex][MMS_FEMALE].c_str());
	}

	// 나중에 hair를 따로 만들어야 할듯..
	pVMesh->SetParts(eq_parts_head, szMeshName);
}

void ZChangeCharParts(RVisualMesh* pVMesh, MMatchSex nSex, int nHair, int nFace, unsigned long int* pItemID)
{
	if (pVMesh == NULL)
	{
		_ASSERT(0);
		return;
	}

	ChangeEquipParts(pVMesh, pItemID);		// hair, face보다 장비를 먼저 바꿔야 한다.

	// 지금은 머리아이템과 머리카락 메쉬를 서로 공유하고 있다. - 나중에 떼어놓도록 하자
	if (pItemID[MMCIP_HEAD] == 0)
	{
		ChangeCharHair(pVMesh, nSex, nHair);
	}
	
	ChangeCharFace(pVMesh, nSex, nFace);
}

void ZChangeCharWeaponMesh(RVisualMesh* pVMesh, unsigned long int nWeaponID)
{
	if( pVMesh ) 
	{
		if (nWeaponID == 0) 
		{
			RWeaponMotionType type = eq_ws_dagger;
			pVMesh->AddWeapon(type , NULL);
			pVMesh->SelectWeaponMotion(type);

			return;	// id가 0이면 아무것도 착용하지 않는다.
		}


		RWeaponMotionType type = eq_weapon_etc;
		MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nWeaponID);
		
		if (pDesc == NULL)
		{
//			_ASSERT(0);
			return;
		}

		RMesh* pMesh = ZGetWeaponMeshMgr()->Get( pDesc->m_szMeshName );

		if( pMesh ) 
		{
			type = pMesh->GetMeshWeaponMotionType();
			CheckTeenVersionMesh(&pMesh);
			pVMesh->AddWeapon(type , pMesh);
			pVMesh->SelectWeaponMotion(type);
		}
	}

}


///////////////////////////////////////////////////////////////////////////////////////
MImplementRTTI(ZCharacter, ZCharacterObject);

ZCharacter::ZCharacter() : ZCharacterObject(), m_DirectionLower(1,0,0),m_DirectionUpper(1,0,0),m_TargetDir(1,0,0),m_bAdminHide(false)
{ 

	m_nMoveMode = MCMM_RUN;
	m_nMode		= MCM_OFFENSIVE;
	m_nState	= MCS_STAND;

	m_nVMID = -1;
//	m_fLastUpdateTime = 0.f;

	m_nSelectSlot = 0;

//	m_dwBackUpTime = 0;

	// 충돌처리값
	m_Collision.fRadius = CHARACTER_RADIUS-2;
	m_Collision.fHeight = CHARACTER_HEIGHT;
	

	m_fLastKillTime = 0;
	m_nKillsThisRound = 0;
	m_LastDamageType = ZD_NONE;
	m_LastDamageWeapon = MWT_NONE;
	m_LastDamageDot = 0.f;
	m_LastDamageDistance = 0.f;

	m_LastThrower = MUID(0,0);

	m_bDie = false;

	m_bCommander = false;
	m_bTagger = false;

//	m_bAutoDir = false;
	m_bStylishShoted = false;
	m_bStun = false;

	m_nBlastType = 0;
	m_bBlast = false;
	m_bBlastFall = false;
	m_bBlastDrop = false;
	m_bBlastStand = false;
	m_bBlastAirmove = false;

	m_bSpMotion = false;
	m_SpMotion = ZC_STATE_TAUNT;

	m_nTeamID = MMT_ALL;
	m_bIsLowModel = false;
	
	m_AniState_Upper=ZC_STATE_UPPER_NONE;
	m_AniState_Lower=ZC_STATE_LOWER_NONE;

//	m_pAnimationInfo_Lower=&g_AnimationInfoTableLower[ZC_STATE_LOWER_IDLE1];
//	m_pAnimationInfo_Lower=&g_AnimationInfoTableUpper[ZC_STATE_UPPER_NONE];

	m_pAnimationInfo_Lower=NULL;
	m_pAnimationInfo_Upper=NULL;

	m_vProxyPosition = m_vProxyDirection = rvector(0.0f,0.0f,0.0f);

	for(int i=0;i<6;i++) {
		m_t_parts[i]  = 2;//남자는 2번 코드모델이 초기모델..
		m_t_parts2[i] = 0;
	}

	m_fLastReceivedTime=0;

	/*
	m_fAveragePingTime=0;
	m_fAccumulatedTimeError=0;
	m_nTimeErrorCount=0;
	*/

	m_fLastValidTime = 0.0f;
	m_Accel = rvector(0.0f, 0.0f, 0.0f);
	m_bRendered = false;

	m_nStunType = ZST_NONE;
	m_nWallJumpDir = 0;

	/*
	m_fClimbZ = 0.f;

	m_ClimbDir.x = 0.f;
	m_ClimbDir.y = 0.f;
	m_ClimbDir.z = 0.f;
	*/

	m_RealPositionBefore = rvector(0.f,0.f,0.f);
	m_AnimationPositionDiff = rvector(0.f,0.f,0.f);

	m_fGlobalHP = 0.f;
	m_nReceiveHPCount = 0;

	//m_FloorPlane.a = 0.f;
	//m_FloorPlane.b = 0.f;
	//m_FloorPlane.c = 0.f;	
	//m_FloorPlane.d = 0.f;

	m_fAttack1Ratio = 1.f;

	m_bDamaged = false;
	m_bFallingToNarak = false;

	m_nWhichFootSound = 0;

	m_fTimeOffset = 0;
	m_nTimeErrorCount = 0;
	m_fAccumulatedTimeError = 0;

//	RegisterModules();

	m_pModule_HPAP = new ZModule_HPAP;
	m_pModule_Resistance = new ZModule_Resistance;
	m_pModule_FireDamage = new ZModule_FireDamage;
	m_pModule_ColdDamage = new ZModule_ColdDamage;
	m_pModule_PoisonDamage = new ZModule_PoisonDamage;
	m_pModule_LightningDamage = new ZModule_LightningDamage;
	m_pModule_QuestStatus = NULL;
	
	AddModule(m_pModule_HPAP);
	AddModule(m_pModule_Resistance);
	AddModule(m_pModule_FireDamage);
	AddModule(m_pModule_ColdDamage);
	AddModule(m_pModule_PoisonDamage);
	AddModule(m_pModule_LightningDamage);

	// 퀘스트 관련 게임타입일 경우에만 QuestStatus 생성, 등록
	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
	{
		m_pModule_QuestStatus = new ZModule_QuestStatus();
		AddModule(m_pModule_QuestStatus);
	}

	m_szUserName[0]=0;
	m_szUserAndClanName[0]=0;

	SetInvincibleTime(0);

	m_bChatEffect = false;
}

//void ZCharacter::RegisterModules()
//{
//	ZObject::RegisterModules();
//
//	// 모듈 등록한다
//	RegisterModule(&m_Module_HPAP);
//	RegisterModule(&m_Module_Movable);
//	RegisterModule(&m_Module_Resistance);
//
//	RegisterModule(&m_Module_FireDamage);
//	RegisterModule(&m_Module_ColdDamage);
//	RegisterModule(&m_Module_PoisonDamage);
//	RegisterModule(&m_Module_LightningDamage);
//}

ZCharacter::~ZCharacter()
{
	RemoveModule(m_pModule_HPAP);
	RemoveModule(m_pModule_Resistance);
	RemoveModule(m_pModule_FireDamage);
	RemoveModule(m_pModule_ColdDamage);
	RemoveModule(m_pModule_PoisonDamage);
	RemoveModule(m_pModule_LightningDamage);
	RemoveModule(m_pModule_QuestStatus);

	delete m_pModule_HPAP;
	delete m_pModule_Resistance;
	delete m_pModule_FireDamage;
	delete m_pModule_ColdDamage;
	delete m_pModule_PoisonDamage;
	delete m_pModule_LightningDamage;
	SAFE_DELETE(m_pModule_QuestStatus);

	EmptyHistory();

	Destroy();
}

void ZCharacter::EmptyHistory()
{
	if (m_bInitialized == false) return;

	while(m_BasicHistory.size())
	{
		delete *m_BasicHistory.begin();
		m_BasicHistory.erase(m_BasicHistory.begin());
	}
	//while(m_HPHistory.size())
	//{
	//	delete *m_HPHistory.begin();
	//	m_HPHistory.erase(m_HPHistory.begin());
	//}
}

void ZCharacter::Stop()
{
	//SetState(ZC_STATE_IDLE);
	SetAnimationLower(ZC_STATE_LOWER_IDLE1);
}

bool ZCharacter::IsTeam(ZCharacter* pChar)
{
	if(pChar) {
		if( pChar->GetTeamID() == GetTeamID()) {
			return true;
		}
	}
	return false;
}

bool ZCharacter::IsMoveAnimation()
{
	return g_AnimationInfoTableLower[m_AniState_Lower].bMove;
}

void ZCharacter::SetAnimation(char *AnimationName,bool bEnableCancel,int time)
{
	if (m_bInitialized == false) return;

	SetAnimation(ani_mode_lower,AnimationName,bEnableCancel,time);
}

void ZCharacter::SetAnimation(RAniMode mode,char *AnimationName,bool bEnableCancel,int time)
{
	if (m_bInitialized == false) 
		return;

	if(!m_pVMesh) 
		return;

	if(time) {
		m_pVMesh->SetReserveAnimation(mode,AnimationName,time);
	}
	else {
		m_pVMesh->SetAnimation(mode,AnimationName,bEnableCancel);
	}
}

//#define TRACE_ANIMATION

// 시간 test 위한 변수
DWORD g_dwLastAnimationTime=timeGetTime();

void ZCharacter::SetAnimationLower(ZC_STATE_LOWER nAni)
{
	if (m_bInitialized == false) return;
	if ((IsDie()) && (IsHero())) return;

	if(nAni==m_AniState_Lower) return;
	_ASSERT(nAni>=0 && nAni<ZC_STATE_LOWER_END);

	if( nAni< 0 || nAni >= ZC_STATE_LOWER_END )
	{
		return;
	}

//	if(!m_pAnimationInfo_Lower || m_pAnimationInfo_Lower->bEnableCancel)
//	if( m_pVMesh->m_isPlayDone || m_pAnimationInfo_Upper->bEnableCancel )
	//{
		m_AniState_Lower=nAni;
		m_pAnimationInfo_Lower=&g_AnimationInfoTableLower[nAni];
		
		SetAnimation(ani_mode_lower,m_pAnimationInfo_Lower->Name,m_pAnimationInfo_Lower->bEnableCancel,0);

		// 움직이는 애니메이션이 연결될때는 여기에 추가해주어야 한다
//		if(IsMoveAnimation())
		{
			if(!g_AnimationInfoTableLower[nAni].bContinuos)
				m_RealPositionBefore=rvector(0,0,0);
		}

	//m_RealPositionBefore=m_RealPosition=m_Position;


#ifdef TRACE_ANIMATION
	{
		DWORD curtime = timeGetTime();
		if(g_pGame->m_pMyCharacter==this)
			mlog("animation - %d %s   - %d frame , interval %d \n",nAni,
			m_pVMesh->GetFrameInfo(ani_mode_lower)->m_pAniSet->GetName(),m_pVMesh->GetFrameInfo(ani_mode_lower)->m_nFrame,
			curtime-g_dwLastAnimationTime);
		g_dwLastAnimationTime = curtime;
	}
#endif

//	m_pVMesh->m_nFrame[0]=0;
	//*/
}

void ZCharacter::SetAnimationUpper(ZC_STATE_UPPER nAni)
{
	if (m_bInitialized == false) return;
	if ((IsDie()) && (IsHero())) return;

	if(nAni==m_AniState_Upper) 	return;

	_ASSERT(nAni>=0 && nAni<ZC_STATE_UPPER_END);

	if( nAni< 0 || nAni >= ZC_STATE_UPPER_END )
	{
		return;
	}

#ifdef TRACE_ANIMATION
	{
		DWORD curtime = timeGetTime();
		mlog("upper Animation Index : %d %s @ %d \n", nAni ,g_AnimationInfoTableUpper[nAni].Name,curtime-g_dwLastAnimationTime);
		if(m_AniState_Upper==3 && nAni==0)
		{
			bool a=false;
		}
		g_dwLastAnimationTime = curtime;
	}
#endif

//	if(!m_pAnimationInfo_Upper || m_pAnimationInfo_Upper->bEnableCancel)
	//if( m_AniState_Upper != ZC_STATE_UPPER_NONE )
	//{
	//	if( m_pVMesh->m_isPlayDone || m_pAnimationInfo_Upper->bEnableCancel )
	//	{	
			m_AniState_Upper=nAni;
			m_pAnimationInfo_Upper=&g_AnimationInfoTableUpper[nAni];
			if( m_pAnimationInfo_Upper == NULL || m_pAnimationInfo_Upper->Name == NULL )
			{
				mlog("Fail to Get Animation Info.. Ani Index : [%d]\n", nAni );
				return;
			}
			SetAnimation(ani_mode_upper,m_pAnimationInfo_Upper->Name,m_pAnimationInfo_Upper->bEnableCancel,0);
	//	}
	//}
}

void ZCharacter::UpdateLoadAnimation()
{
	if (m_bInitialized == false) return;

	if(m_pAnimationInfo_Lower)
	{
		SetAnimation(m_pAnimationInfo_Lower->Name,m_pAnimationInfo_Lower->bEnableCancel,0);
		SetAnimationUpper(ZC_STATE_UPPER_NONE);
		SetAnimationUpper(ZC_STATE_UPPER_LOAD);
		m_bPlayDone_upper=false;
	}
}


// 모션의 감도때문에 수시로 결정
// 하반신 카메라방향으로 보간돌리기
// 상반신 파트 목표 벡터 설정

bool CheckVec(rvector& v1,rvector& v2) {
	if( fabs(v1.x - v2.x) < 0.03f )
		if( fabs(v1.y - v2.y) < 0.03f )
			if( fabs(v1.z - v2.z) < 0.03f )
				return false;
	return true;
}

bool g_debug_rot = false;


// 이 펑션은 허리돌리기때문에..
void ZCharacter::UpdateMotion(float fDelta)
{
	if (m_bInitialized==false) return;
	// 점프로 모션 바꾸기 - 이전상태 백업
	// 점프는 어떤 상태에서든 모션이바뀔수있으므로..
	// run , idle

	// 자신의 타겟방향에 캐릭터의 방향을 맞춘다..
	if (IsDie()) { //허리 변형 없다~

		m_pVMesh->m_vRotXYZ.x = 0.f;
		m_pVMesh->m_vRotXYZ.y = 0.f;
		m_pVMesh->m_vRotXYZ.z = 0.f;
		
		return;
	}


	if (g_Rules.IsVanillaMode())
	{
		if ((m_AniState_Lower == ZC_STATE_LOWER_IDLE1) ||
			(m_AniState_Lower == ZC_STATE_LOWER_RUN_FORWARD) ||
			(m_AniState_Lower == ZC_STATE_LOWER_RUN_BACK))
		{

			m_Direction = m_TargetDir;

			//	m_DirectionLower = rvector(1,0,0);
			//	m_DirectionLower = m_Direction;

			rvector targetdir = m_TargetDir;
			targetdir.z = 0;
			Normalize(targetdir);

			rvector dir = m_Accel;
			dir.z = 0;

			if (Magnitude(dir) < 10.f) {
				dir = targetdir;
			}
			else
				Normalize(dir);

			// 뒤 / 뒤+왼쪽 / 뒤+오른쪽 3개에 한해서 방향을 반대로.
			// 예를들면 뒤+왼쪽은 앞+오른쪽과 같은 발방향을 한다..

			bool bInversed = false;
			//	if(IsKeyDown('S'))
			if (DotProduct(targetdir, dir) < -cos(D3DX_PI / 4.f) + 0.01f)
			{
				dir = -dir;
				bInversed = true;
			}

			// fAngleLower 는 현재 발방향과 해야하는 발방향의 각도 차이
			float fAngleLower = GetAngleOfVectors(dir, m_DirectionLower);

			rmatrix mat;

#define ROTATION_SPEED	400.f

			// 일정각도 이상되면 하체를 틀어준다
			if (fAngleLower > 5.f / 180.f*D3DX_PI)
			{
				D3DXMatrixRotationZ(&mat, max(-ROTATION_SPEED*fDelta / 180.f*D3DX_PI, -fAngleLower));
				m_DirectionLower = m_DirectionLower * mat;
			}

			if (fAngleLower < -5.f / 180.f*D3DX_PI)
			{
				D3DXMatrixRotationZ(&mat, min(ROTATION_SPEED*fDelta / 180.f*D3DX_PI, -fAngleLower));
				m_DirectionLower = m_DirectionLower * mat;
			}


			// 상체가 향해야 하는 방향은 언제나 타겟방향
			float fAngle = GetAngleOfVectors(m_TargetDir, m_DirectionLower);

			// 그러나 하체와의 각도를 항상 일정각도 이하로 유지한다.

			if (fAngle < -65.f / 180.f*D3DX_PI)
			{
				fAngle = -65.f / 180.f*D3DX_PI;
				D3DXMatrixRotationZ(&mat, -65.f / 180.f*D3DX_PI);
				m_DirectionLower = m_Direction * mat;
			}

			if (fAngle >= 65.f / 180.f*D3DX_PI)
			{
				fAngle = 65.f / 180.f*D3DX_PI;
				D3DXMatrixRotationZ(&mat, 65.f / 180.f*D3DX_PI);
				m_DirectionLower = m_Direction * mat;
			}

			m_pVMesh->m_vRotXYZ.x = -fAngle * 180 / D3DX_PI *.9f;

			// 실제보다 약간 고개를 들어준다 :)
			m_pVMesh->m_vRotXYZ.y = (m_TargetDir.z + 0.05f) * 50.f;
		}
		else // 달리기/가만있기등의 애니메이션이 아니면 허리안돌린다.
		{
			m_Direction = m_TargetDir;
			m_DirectionLower = m_Direction;

			m_pVMesh->m_vRotXYZ.x = 0.f;
			m_pVMesh->m_vRotXYZ.y = 0.f;
			m_pVMesh->m_vRotXYZ.z = 0.f;
		}
	}
	else
	{
		m_Direction = m_TargetDir;
		m_DirectionLower = m_Direction;

		m_pVMesh->m_vRotXYZ.x = 0.f;
		m_pVMesh->m_vRotXYZ.y = (m_TargetDir.z + 0.05f) * 50.f;
		m_pVMesh->m_vRotXYZ.z = 0.f;
	}
}

void GetDTM(bool* pDTM,int mode,bool isman)
{
//	남자 우 좌 무 쌍
//	여자 좌 우 좌 우

	if(!pDTM) return;

//  여자도 모션이 같아졌다..

//	if(isman) {

		     if(mode==0) { pDTM[0]=true; pDTM[1]=false; }
		else if(mode==1) { pDTM[0]=false;pDTM[1]=true;  }
		else if(mode==2) { pDTM[0]=false;pDTM[1]=false; }
		else if(mode==3) { pDTM[0]=true; pDTM[1]=true;  }
/*
	}
	else {

		     if(mode==0) { pDTM[0]=false;pDTM[1]=true;  }
		else if(mode==1) { pDTM[0]=true; pDTM[1]=false; }
		else if(mode==2) { pDTM[0]=false;pDTM[1]=true;  }
		else if(mode==3) { pDTM[0]=true; pDTM[1]=false; }
	}
*/
}

void ZCharacter::CheckDrawWeaponTrack()
{
	if(m_pVMesh==NULL) return;

	bool bDrawTracks = false;//칼 궤적을 그릴것인가?

	if (ZGetConfiguration()->GetDrawTrails()
#ifdef PORTAL
		&& !g_pPortal->IsDrawingFakeChar()
#endif
		)
	{
		if ((m_pVMesh->m_SelectWeaponMotionType == eq_wd_katana) ||
			(m_pVMesh->m_SelectWeaponMotionType == eq_wd_sword) ||
			(m_pVMesh->m_SelectWeaponMotionType == eq_wd_blade))
		{

			if ((ZC_STATE_LOWER_ATTACK1 <= m_AniState_Lower && m_AniState_Lower <= ZC_STATE_LOWER_GUARD_CANCEL) ||
				(ZC_STATE_UPPER_LOAD <= m_AniState_Upper && m_AniState_Upper <= ZC_STATE_UPPER_GUARD_CANCEL))
			{
				if (m_AniState_Upper != ZC_STATE_UPPER_GUARD_IDLE) {

					// 복귀할때는 안 그린다
					if ((m_AniState_Lower != ZC_STATE_LOWER_ATTACK1_RET) &&
						(m_AniState_Lower != ZC_STATE_LOWER_ATTACK2_RET) &&
						(m_AniState_Lower != ZC_STATE_LOWER_ATTACK3_RET) &&
						(m_AniState_Lower != ZC_STATE_LOWER_ATTACK4_RET))
						bDrawTracks = true;
				}
			}
		}
	}

	// 이도류일 경우

	bool bDTM[2];

	bDTM[0] = true;
	bDTM[1] = true;

	bool bMan = IsMan();

	if(m_pVMesh->m_SelectWeaponMotionType == eq_wd_blade) 
	{
			 if( m_AniState_Lower == ZC_STATE_LOWER_ATTACK1 ) GetDTM(bDTM,0,bMan);
		else if( m_AniState_Lower == ZC_STATE_LOWER_ATTACK2 ) GetDTM(bDTM,1,bMan);
		else if( m_AniState_Lower == ZC_STATE_LOWER_ATTACK3 ) GetDTM(bDTM,2,bMan);
		else if( m_AniState_Lower == ZC_STATE_LOWER_ATTACK4 ) GetDTM(bDTM,3,bMan);
	}

	m_pVMesh->m_bDrawTracksMotion[0] = bDTM[0];
	m_pVMesh->m_bDrawTracksMotion[1] = bDTM[1];

	m_pVMesh->SetDrawTracks(bDrawTracks);

}

// 렌더링하면서 모션의 프레임 타임에 의존적인 무기의 경우

void ZCharacter::UpdateSpWeapon()
{
	if(!m_pVMesh) return;

	m_pVMesh->UpdateSpWeaponFire();

	if(m_pVMesh->m_bAddGrenade) {//모두가 알고있다~

		rvector vWeapon[2];

		vWeapon[0] = m_pVMesh->GetCurrentWeaponPosition();

		// 벽뒤로 던지는 경우가 생기면
//		rvector nPos = m_Position + rvector(0,0,100);
		rvector nPos = m_pVMesh->GetBipTypePosition(eq_parts_pos_info_Spine1);// m_Position + rvector(0,0,100);
		rvector nDir = vWeapon[0] - nPos;

		Normalize(nDir);

		RBSPPICKINFO bpi;
		// 아래를 보면서 던지면 벽을 통과해서...
//		if(ZGetGame()->GetWorld()->GetBsp()->Pick(m_Position+rvector(0,0,100),m_TargetDir,&bpi))
		if(ZGetGame()->GetWorld()->GetBsp()->Pick(nPos,nDir,&bpi))
		{
			if(D3DXPlaneDotCoord(&(bpi.pInfo->plane),&vWeapon[0])<0){
//				vWeapon[0]=m_Position+rvector(0,0,140);
				vWeapon[0] = bpi.PickPos - nDir;
//				OutputDebugString("벽뒤로 갔다...\n");
			}
		}

		vWeapon[1] = m_TargetDir;//rvector();//시작방향
		vWeapon[1].z += 0.1f;//약간위쪽으로..
//		vWeapon[2] = rvector(1,1,1);//아직사용안함

		Normalize(vWeapon[1]);

		if(m_UID==g_pGame->m_pMyCharacter->m_UID) {

			int type = ZC_WEAPON_SP_GRENADE;//기본수류탄

			RVisualMesh* pWVMesh = m_pVMesh->GetSelectWeaponVMesh();

			if( pWVMesh ) {
				if(pWVMesh->m_pMesh) {
					if(strncmp( pWVMesh->m_pMesh->GetName(),"flashbang",9) == 0) {
						type = ZC_WEAPON_SP_FLASHBANG;
					}
					else if(strncmp( pWVMesh->m_pMesh->GetName(),"smoke",5) == 0) {
						type = ZC_WEAPON_SP_SMOKE;
					}
					else if(strncmp( pWVMesh->m_pMesh->GetName(),"tear_gas",8) == 0) {
						type = ZC_WEAPON_SP_TEAR_GAS;	
					}
				}
			}

			int sel_type = GetItems()->GetSelectedWeaponParts();

			ZPostShotSp(g_pGame->GetTime(),vWeapon[0],vWeapon[1],type,sel_type);
			m_pVMesh->m_bAddGrenade = false;
		}
	}
}

bool ZCharacter::IsMan() 
{
	if(m_pVMesh) {
		if(m_pVMesh->m_pMesh) {
			if(strcmp(m_pVMesh->m_pMesh->GetName(),"heroman1")==0) {
				return true;
			}
		}
	}
	return false;
}

void ZCharacter::OnDraw()
{
	m_bRendered = false;

	if (m_bInitialized==false) return;
	if (!IsVisible()) return;
	if(IsAdminHide()) return;

	__BP(39,"ZCharacter::Draw");

	if( m_pVMesh && !Enable_Cloth )	m_pVMesh->DestroyCloth();

	//////////////
	// 광원 세팅

	//#define SHOW_LIGHT_TEAPOT			// 광원에 주전자 표시

	if(m_nVMID==-1)//초기화도 안된상태
	{
		__EP(39);
		return;
	}

	rboundingbox bb;
	static constexpr auto Radius = 150;
	static constexpr auto Height = 190;
	bb.vmax = m_Position + rvector(Radius, Radius, Height);
	bb.vmin = m_Position - rvector(Radius, Radius, 0);
	if (!ZGetGame()->GetWorld()->GetBsp()->IsVisible(bb)) return;
	if (!isInViewFrustum(&bb, RGetViewFrustum())) return;

	__BP(24, "ZCharacter::Draw::Light");

	Draw_SetLight( m_Position );

	__EP(24);

	if( g_pGame->m_bShowWireframe ) 
	{
		RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
		RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	}

	// 나락에 떨어질때 맵에 포그가 깔려있으면 ZBuffer를 꺼준다. 나락 판때기에 캐릭터가 가려지기 때문
	bool bNarakSetState = ((m_bFallingToNarak) && ((g_pGame->GetWorld()->IsFogVisible())));
	if (bNarakSetState)
	{
		RGetDevice()->SetRenderState(D3DRS_FOGENABLE, FALSE );
		RSetWBuffer(false);
	}


	///////////////////////////////////////////////////////////////////////
	//	rvector dir = m_Direction;dir.z = 0;

	CheckDrawWeaponTrack();

	RGetDevice()->SetRenderState( D3DRS_LIGHTING, TRUE );

	// 죽은 넘이면 투명해진다.
	if(IsDie())
	{
#define TRAN_AFTER		3.f		// 이 시간 이후부터
#define VANISH_TIME		2.f		// 이 시간동안 투명해진다

		float fOpacity = max(0.f,min(1.0f,(
			VANISH_TIME-(g_pGame->GetTime()-GetDeadTime() - TRAN_AFTER))/VANISH_TIME));
		m_pVMesh->SetVisibility(fOpacity);
	}
	else
	if (!m_bHero) m_pVMesh->SetVisibility(ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SKILLMAP ? 0.4 : 1.f);

	__BP(25, "ZCharacter::Draw::VisualMesh::Render");

//	m_bIsLowModel = true;

	UpdateEnchant();

	bool bGame = g_pGame ? true : false;	

	rvector cpos = ZApplication::GetGameInterface()->GetCamera()->GetPosition();
	cpos = m_vProxyPosition - cpos;
	float dist = Magnitude(cpos);

	m_pVMesh->SetClothValue(bGame, fabs(dist));
	m_pVMesh->Render(false);// m_bIsLowModel);

	m_bRendered = m_pVMesh->m_bIsRender;

	if(m_pVMesh->m_bIsRenderWeapon && (m_pVMesh->GetVisibility() > 0.05f))
	{
		DrawEnchant(m_AniState_Lower,m_bCharged);
	}

#ifdef PORTAL
	g_pPortal->DrawFakeCharacter(this);
#endif
	
	if (bNarakSetState)
	{
		RSetWBuffer(true);
	}

	__EP(25);


	__EP(39);
}


bool ZCharacter::CheckDrawGrenade()
{
	if(m_Items.GetSelectedWeapon()==NULL) return false;

	if( m_pVMesh ) {
		if( m_pVMesh->m_SelectWeaponMotionType==eq_wd_grenade ) {
			if( m_Items.GetSelectedWeapon()->GetBulletAMagazine() ) {
				return true;
			}
		}
	}
	return false;
}

bool ZCharacter::Pick(rvector& pos,rvector& dir, RPickInfo* pInfo)
{
	if (m_bInitialized==false) return false ;

	return ZObject::Pick(pos, dir, pInfo);
}

/*
bool ZCharacter::Pick(int x,int y,rvector* v,float* f)
{
	RPickInfo info;
	bool hr = Pick(x,y,&info);
	*v = info.vOut;
	*f = info.t;
	return hr;
}
*/
/*
bool ZCharacter::Pick(int x,int y,RPickInfo* pInfo) {


	if(m_pVMesh) {
		return m_pVMesh->Pick(x,y,pInfo);
	}

	return false;
}
*/

#define GRAVITY_CONSTANT	2500.f			// 중력의 영향
#define DAMAGE_VELOCITY		1700.f			// 속도가 이 이상되면 폴링데미지를 받는다.
#define MAX_FALL_SPEED		3000.f			// 최대 낙하속도
#define MAX_FALL_DAMAGE		50.f			// 최대 데미지
#define BLASTED_KNOCKBACK_RATIO	3.f			// 띄워진상태에서의 Knockback을 과장(배수)

void ZCharacter::UpdateHeight(float fDelta)
{
	if (m_bFallingToNarak) return;

	m_bJumpUp=(GetVelocity().z>0);

	if(GetVelocity().z<0 && GetDistToFloor()>35.f)	// 계단 내려갈때 정도는 그냥 내려간다.
	{
		if(!m_bJumpDown) {
//			m_fFallHeight = m_Position.z;
			m_bJumpDown=true;
			m_bJumpUp = false;
		}
	}

	if(!m_bWallJump)
	{
		//if(GetVelocity().z<1.f && GetDistToFloor()<1.f)
		//{
		//	if(!m_bLand )
		if(m_pModule_Movable->isLanding())
		{
			if(m_Position.z + 100.f < m_pModule_Movable->GetFallHeight())
			{
//				m_fFallHeight = m_Position.z;
				float fSpeed=fabs(GetVelocity().z);
				if(fSpeed>DAMAGE_VELOCITY)
				{
					float fDamage = MAX_FALL_DAMAGE * ( fSpeed-DAMAGE_VELOCITY) / (MAX_FALL_SPEED-DAMAGE_VELOCITY) ;
#ifdef	ENABLE_FALLING_DAMAGE
					//				DamagedFalling(fDamage);
#endif

				}

				RBspObject* r_map = g_pGame->GetWorld()->GetBsp();

				// 점프 착지시 먼지..
				rvector vPos = GetPosition();
				rvector vDir = rvector(0.f,0.f,-1.f);
				vPos.z += 50.f;

				RBSPPICKINFO pInfo;

				if(r_map->Pick(vPos,vDir,&pInfo)) {
/*
					if( g_pGame->m_waters.CheckSpearing( vPos+rvector(0.f,0.f,200.f), pInfo.PickPos, 250, 0.3 ) )	{
								

					}
					else {
*/						
					vPos = pInfo.PickPos;

					vDir.x = pInfo.pInfo->plane.a;
					vDir.y = pInfo.pInfo->plane.b;
					vDir.z = pInfo.pInfo->plane.c;

					ZGetEffectManager()->AddLandingEffect(vPos,vDir);//내부에서 옵션에 따라~

					///////////////////////////////////////////////////////////
					// 착지 sound .. 착지 시점을 정확하게 하려면 ..

					AniFrameInfo* pInfo = m_pVMesh->GetFrameInfo(ani_mode_lower);
					RAniSoundInfo* pSInfo = &pInfo->m_SoundInfo;// &m_pVMesh->m_SoundInfo[0];

					if(pSInfo->Name[0]) {
						pSInfo->isPlay = true;
						UpdateSound();
//							ZApplication::GetSoundEngine()->PlaySound(pSInfo->Name,vPos.x,vPos.y,vPos.z);
					}
					else {//벽점프후 사운드등은 등록되어 있지 않다..
						strcpy_safe(pSInfo->Name,"man_jump");
						pSInfo->isPlay = true;
						UpdateSound();
					}

//						}
				}

			}
		}
	}

	return;
}

int ZCharacter::GetSelectWeaponDelay(MMatchItemDesc* pSelectItemDesc)
{
//	칼의 경우는 모션의 길이가 포함되어 있다..
//	무기타잎별로 하나만 등록되어 있으니까 기본공격 모션의 길이를 기준으로 한다..

	if(!pSelectItemDesc) return 0;

	int nReturnDelay = pSelectItemDesc->m_nDelay;

	if(pSelectItemDesc->m_nType != MMIT_MELEE)
		return 0;

	switch(pSelectItemDesc->m_nWeaponType)
	{
		case MWT_DAGGER:		// attackS 공격 길이만큼... man_dagger_standshot.elu.ani기준..
			nReturnDelay -= 266;
			break;

		case MWT_DUAL_DAGGER:	// attack1 공격 길이만큼.... man_2hdagger_standshot_1.elu.ani
			nReturnDelay -= 299;
			break;

		case MWT_KATANA:		//	attack1 공격 길이만큼....man_knife_standshot_1.elu.ani
			nReturnDelay -= 299;
			break;

		case MWT_DOUBLE_KATANA:	//	attack1 공격 길이만큼.... man_blade_standshot_1.elu.ani 
			nReturnDelay -= 299;
			break;

		case MWT_GREAT_SWORD:	// attack1 공격 길이만큼.... man_sword_standshot_1.elu.ani
			nReturnDelay -= 399;
			break;

		default:
			_ASSERT(0);
			break;
	}

	return nReturnDelay;
/*
	실시간으로 검색할 필요없을것 같다..

	RAnimation* pAniSet = m_pVMesh->GetFrameInfo( ani_mode_upper )->m_pAniSet;

	if( pAniSet==NULL )
		pAniSet = m_pVMesh->GetFrameInfo( ani_mode_lower )->m_pAniSet;

	if( pAniSet ) {
		if( pAniSet->m_pAniData ) {
			DWORD _ms  = pAniSet->m_pAniData->m_max_frame / 4.8f;
			nWeaponDelay -= _ms;
		}
	}
*/

}

void ZCharacter::UpdateSpeed()
{
	//////////////////////////////////////////////////////////////////
	// update animation speed

	if(m_pVMesh==NULL)
		return;

	float speed = 4.8f;
	float speed_upper = 4.8f;

	if( GetItems() && GetItems()->GetSelectedWeapon() && GetItems()->GetSelectedWeapon()->GetDesc() ) {

		if( GetItems()->GetSelectedWeapon()->GetDesc()->m_nType==MMIT_MELEE) {

			// 첫번째 공격은 비율을 계산한다..
/*
			if(m_AniState_Lower == ZC_STATE_LOWER_ATTACK1) {

				MMatchItemDesc* pRangeDesc = GetItems()->GetSelectedWeapon()->GetDesc();

//				DWORD nWeaponDelay = (DWORD)((float)pRangeDesc->m_nDelay * 0.7f);// 반으로 나눠서~
				int  nWeaponDelay = pRangeDesc->m_nDelay;

				int max_frame = 0;

				max_frame = m_pVMesh->GetMaxFrame(ani_mode_upper);

				if(max_frame==0)
					max_frame = m_pVMesh->GetMaxFrame(ani_mode_lower);

				if(max_frame) {

					int _time = (int)(max_frame / 4.8f); 

					int as = _time + nWeaponDelay;

					if(as < 1)	as = 1;

					float fas = 0.f;

					fas = ( _time / (float)( as));

					m_fAttack1Ratio = fas;

				}
			}

			// 에니메이션 상태에 따라~

			if( (m_AniState_Lower == ZC_STATE_LOWER_ATTACK1)	 ||
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK1_RET) ||  
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK2)	 ||
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK2_RET) ||  
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK3)	 ||
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK3_RET) ||  
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK4)	 ||
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK4_RET) ||  
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK5)	 ||
				(m_AniState_Lower == ZC_STATE_LOWER_JUMPATTACK)  ||
				(m_AniState_Upper == ZC_STATE_UPPER_SHOT) ) {
//				(m_AniState_Lower == ZC_STATE_LOWER_UPPERCUT) ) {

				speed = 4.8f * m_fAttack1Ratio;

				if(speed < 0.1f)
					speed = 0.1f;
			}
*/		
			if( (m_AniState_Lower == ZC_STATE_LOWER_ATTACK1)	 ||
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK1_RET) ||  
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK2)	 ||
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK2_RET) ||  
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK3)	 ||
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK3_RET) ||  
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK4)	 ||
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK4_RET) ||  
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK5)	 ||
				(m_AniState_Lower == ZC_STATE_LOWER_JUMPATTACK)  ||
				(m_AniState_Upper == ZC_STATE_UPPER_SHOT) ) {
//				(m_AniState_Lower == ZC_STATE_LOWER_UPPERCUT) ) {

				MMatchItemDesc* pRangeDesc = GetItems()->GetSelectedWeapon()->GetDesc();

//				DWORD nWeaponDelay = (DWORD)((float)pRangeDesc->m_nDelay * 0.7f);// 반으로 나눠서~
				int  nWeaponDelay = GetSelectWeaponDelay( pRangeDesc );

				int max_frame = 0;

				max_frame = m_pVMesh->GetMaxFrame(ani_mode_upper);

				if(max_frame==0)
					max_frame = m_pVMesh->GetMaxFrame(ani_mode_lower);

				if(max_frame) {

					int _time = (int)(max_frame / 4.8f); 

					int as = _time + nWeaponDelay;

					if(as < 1)	as = 1;

					float fas = 0.f;

					fas = ( _time / (float)( as));

					m_fAttack1Ratio = fas;

					speed = 4.8f * m_fAttack1Ratio;
				}

				if(speed < 0.1f)
					speed = 0.1f;

			} 
		}
/*
		zitem.xml 바뀐후에 적용.....
		// 장전중이라면..
		else {

			if(m_AniState_Upper==ZC_STATE_UPPER_RELOAD) {

				MMatchItemDesc* pRangeDesc = GetItems()->GetSelectedWeapon()->GetDesc();
				int  nReloadDelay = pRangeDesc->m_nReloadTime; // 장전딜레이

				int max_frame = 0;

				max_frame = m_pVMesh->GetMaxFrame(ani_mode_upper);

				if(max_frame) {

					int _time = (int)(max_frame / 4.8f); 

					int as = _time + nReloadDelay;

					if(as < 1)	as = 1;

					float fas = 0.f;

					fas = ( _time / (float)( as));

					m_fAttack1Ratio = fas;

					speed_upper = 4.8f * m_fAttack1Ratio;
				}

				if(speed_upper < 0.1f)
					speed_upper = 0.1f;

//				static char temp[256];
//				sprintf_safe(temp,"reload time %f \n", speed );
//				OutputDebugString( temp );
			}
		}
*/
	}

	if(m_AniState_Upper == ZC_STATE_UPPER_LOAD) 
	{
		speed = 4.8f * 1.2f;
		speed_upper = 4.8f * 1.2f;
	}

//	m_pVMesh->SetSpeed(speed,speed_upper);
	m_pVMesh->SetSpeed(speed,speed);
}

void ZCharacter::OnUpdate(float fDelta)
{
	if (m_bInitialized==false) return;
	if (!IsVisible()) return;

	UpdateSpeed();// 아이템에 따른 에니메이션 속도 조절..

	if(m_pVMesh) {
		m_pVMesh->SetVisibility(1.f);//투명 버그 때문...
		m_pVMesh->Frame();
	}

	UpdateSound();
	UpdateMotion(fDelta);

	if( m_pVMesh && Enable_Cloth  && m_pVMesh->isChestClothMesh() )
	{
		if(IsDie())
		{
			rvector force = rvector( 0,0,-150);
			m_pVMesh->UpdateForce( force);
			m_pVMesh->SetClothState( CHARACTER_DIE_STATE );
		}
		else
		{
			rvector force = -GetVelocity() * 0.15;
			force.z += -90;
			m_pVMesh->UpdateForce( force );
		}
	}


	// draw에서 띄어왔다
	rvector vRot;

	// 옵저브를 하고 있을때는 모든 캐릭터가 일정시간 이전의 모습을 한다
	ZObserver *pObserver = ZApplication::GetGameInterface()->GetCombatInterface()->GetObserver();
	if (pObserver->IsVisible())
	{
		rvector _vDir;
		//rvector headpos;
		if(!GetHistory(&m_vProxyPosition,&_vDir,g_pGame->GetTime()-pObserver->GetDelay()))
			return;

		if (g_Rules.IsVanillaMode())
		{
			m_vProxyDirection = m_DirectionLower;

			float fAngle = GetAngleOfVectors(_vDir,m_vProxyDirection);

			vRot.x = -fAngle*180/ D3DX_PI *.9f;
			vRot.y = (_vDir.z+0.05f) * 50.f;
			vRot.z = 0.f;

			m_pVMesh->m_vRotXYZ = vRot;
		}
		else
		{
			m_vProxyDirection = _vDir;
			m_DirectionLower = _vDir;
		}

/*
		//rvector _vDir;
		//rvector headpos;
		GetHistory(&m_Position,&m_Direction,g_pGame->GetTime()-pObserver->GetDelay());

		float fAngle = GetAngleOfVectors(m_Direction,m_DirectionLower);

		vRot.x = -fAngle*180/pi *.9f;
		vRot.y = (m_Direction.z+0.05f) * 50.f;
		vRot.z = 0.f;

		m_pVMesh->m_vRotXYZ = vRot;
*/
	}
	else
	{
		m_vProxyPosition = m_Position;
		m_vProxyDirection = m_DirectionLower;

/*
//		m_vProxyPosition = m_Position;
//		m_vProxyDirection = m_DirectionLower;
		m_Direction = m_DirectionLower;
*/
	}

	if(IsDie()) {
		m_vProxyDirection = m_Direction;//rvector(0,1,0);
	}

	m_vProxyDirection.z = 0;
	//m_Direction.z = 0;
	Normalize(m_vProxyDirection);

	if(m_nVMID==-1) return;		//초기화도 안된상태

	D3DXMATRIX world;
	MakeWorldMatrix(&world,rvector(0,0,0),m_vProxyDirection,rvector(0,0,1));

	rvector MeshPosition ;

	if(IsMoveAnimation())		// 움직임이 있는 애니메이션은 위치조절을 해줘야 한다.
	{
		// 발의 위치를 로컬 좌표계로 얻어낸다
		rvector footposition = m_pVMesh->GetFootPosition();

		rvector RealPosition = footposition * world;

		// TODO : Mesh의 위치를 결정할때 어느부분을 참조하는지 일반화 할수 있으면 좋다
		if(m_AniState_Lower==ZC_STATE_LOWER_RUN_WALL)
		{
			// TODO : 허리위치를 로컬 좌표계로 얻어내는 펑션을 만들어 아래부분을 간소화하자

			// 머리위치를 로컬 좌표계로 얻어낸다
			rvector headpos = rvector(0.f,0.f,0.f);

			if(m_pVMesh) {

				AniFrameInfo* pAniLow = m_pVMesh->GetFrameInfo(ani_mode_lower);
				AniFrameInfo* pAniUp = m_pVMesh->GetFrameInfo(ani_mode_upper);
//				m_pVMesh->m_pMesh->SetAnimation( m_pVMesh->m_pAniSet[0],m_pVMesh->m_pAniSet[1] );
//				m_pVMesh->m_pMesh->SetFrame(m_pVMesh->m_nFrame[0],m_pVMesh->m_nFrame[1]);
				m_pVMesh->m_pMesh->SetAnimation( pAniLow->m_pAniSet,pAniUp->m_pAniSet );
				m_pVMesh->m_pMesh->SetFrame( pAniLow->m_nFrame , pAniUp->m_nFrame);
				m_pVMesh->m_pMesh->SetMeshVis(m_pVMesh->m_fVis);
				m_pVMesh->m_pMesh->SetVisualMesh(m_pVMesh);

				m_pVMesh->m_pMesh->RenderFrame();

				RMeshNode* pNode = NULL;

				pNode = m_pVMesh->m_pMesh->FindNode(eq_parts_pos_info_Head);

				if(pNode) { 

					headpos.x = pNode->m_mat_result._41;
					headpos.y = pNode->m_mat_result._42;
					headpos.z = pNode->m_mat_result._43;
				}
			}
			// 허리위치를 (머리+발) / 2 라고 대강 잡았다.
			rvector rootpos = 0.5f*(footposition + headpos) * world ;
			
			MeshPosition = m_vProxyPosition + rvector(0,0,90) - rootpos ;
		}
		else
			MeshPosition = m_vProxyPosition - RealPosition ;

		m_AnimationPositionDiff = footposition - m_RealPositionBefore;

		// 애니메이션의 움직임을 월드의 움직임으로 변환한다
		m_AnimationPositionDiff = m_AnimationPositionDiff * world;

		m_RealPositionBefore = footposition;

	}
	else
		MeshPosition = m_vProxyPosition;

	/* // debug
	if(g_pGame->m_pMyCharacter!=this)
	{
		mlog("animation - %d %s %d frame : pos %3.3f %3.3f %3.3f    meshpos %3.3f %3.3f %3.3f \n",m_AniState_Lower,m_pVMesh->m_pAniSet[0]->GetName(),
			m_pVMesh->m_nFrame[0],
			m_Position.x,m_Position.y,m_Position.z,
			MeshPosition.x,MeshPosition.y,MeshPosition.z);
	}
	*/

	MakeWorldMatrix(&world,MeshPosition,m_vProxyDirection,rvector(0,0,1));
	m_pVMesh->SetWorldMatrix(world);

	rvector cpos = ZApplication::GetGameInterface()->GetCamera()->GetPosition();
	cpos = m_vProxyPosition - cpos;
	float dist = Magnitude(cpos);

	m_bIsLowModel = (fabs(dist) > 3000.f);
	if (m_bFallingToNarak) m_bIsLowModel = false;	// 나락으로 떨어질땐 LowModel을 쓰지 않는다.

	m_bDamaged = false;

	CheckLostConn();

	if(m_bCharging && 
		(m_AniState_Lower!=ZC_STATE_CHARGE && 
		m_AniState_Lower!=ZC_STATE_LOWER_ATTACK1)) {
		// 애니메이션이 다르다면 풀린것으로 간주
		m_bCharging = false;
	}

	// 일정시간 지나면 charged 풀림
	if(m_bCharged && g_pGame->GetTime()>m_fChargedFreeTime) {
		m_bCharged = false;
	}

	// 모션의 프레임 타임에 의존적인 무기의 경우 

	UpdateSpWeapon();
}

void ZCharacter::CheckLostConn()
{
	// 마지막으로 데이터를 받은지 1초가 지났으면 머리에 아이콘 띄워주자
	if (g_pGame->GetTime()-m_fLastReceivedTime > 1.f)
	{
		if(!m_bLostConEffect)
		{
			m_bLostConEffect=true;
			ZGetEffectManager()->AddLostConIcon(this);
		}
		SetVelocity(rvector(0,0,0));
	}else
		m_bLostConEffect=false;
}

float ZCharacter::GetMoveSpeedRatio()
{
	float fRatio = 1.f;

	// 나중에 반지 아이템들과의 조합도 추가..
	// update 에서 한번만 계산해 주자..
	MMatchItemDesc* pMItemDesc = GetSelectItemDesc();

	if(pMItemDesc)
		fRatio = pMItemDesc->m_nLimitSpeed/100.f;

	return m_pModule_Movable->GetMoveSpeedRatio()*fRatio;
}


// 감속은 mycharacter와 공통으로 쓴다
void ZCharacter::UpdateVelocity(float fDelta)
{
	rvector dir=rvector(GetVelocity().x,GetVelocity().y,0);
	float fSpeed=Magnitude(dir);
	Normalize(dir);

	float fRatio = GetMoveSpeedRatio();

	float max_speed = MAX_SPEED * fRatio;

	// 최대속도 이상이면 최대속도에 맞춘다..
	if(fSpeed>max_speed)
		fSpeed=max_speed;

	bool bTumble= !IsDie() && (m_bTumble ||
		(ZC_STATE_LOWER_TUMBLE_FORWARD<=m_AniState_Lower && m_AniState_Lower<=ZC_STATE_LOWER_TUMBLE_LEFT));

	if(m_bLand && !m_bWallJump && !bTumble)
	{
		// 달리는 속도 이상이면 빠르게 감속한다.
		rvector forward=m_TargetDir;
		forward.z=0;
		Normalize(forward);

		// 최대값을 비율로 제어한다.
		float run_speed = RUN_SPEED * fRatio;
		float back_speed = BACK_SPEED * fRatio;
		float stop_formax_speed = STOP_FORMAX_SPEED * (1/fRatio);  

		if(DotProduct(forward,dir)>cosf(10.f*D3DX_PI /180.f))	// 앞방향이면
		{
			if(fSpeed>run_speed)
				fSpeed=max(fSpeed-stop_formax_speed*fDelta,run_speed);
		}
		else
		{
			if(fSpeed>back_speed)
				fSpeed=max(fSpeed-stop_formax_speed*fDelta,back_speed);
		}
	}

//	if(fSpeed>200)
//		int k=0;
	// 아무키도 안눌려있으면 감속한다.
	if(IS_ZERO(Magnitude(m_Accel)) && (m_bLand && !m_bWallJump && !m_bWallJump2 && !bTumble ))
	{
		if( m_bBlast && m_nBlastType == 1) {
			// 대거 대쉬 공격받은 상태라면 제외..
//			fSpeed = 1000.f;//*fDelta*HP_SCALE;

//			static char _temp[256];
//			sprintf_safe(_temp,"speed = %f \n",fSpeed);
//			OutputDebugString(_temp);

		}
		else {
			fSpeed = max(fSpeed-STOP_SPEED*fDelta,0);
		}
	}

	SetVelocity(dir.x*fSpeed, dir.y*fSpeed, GetVelocity().z);
}

void ZCharacter::UpdateAnimation()
{
	if (m_bInitialized==false) return;
	SetAnimationLower(ZC_STATE_LOWER_IDLE1);
}

//bool ZCharacter::Move(rvector &diff)
//{
//	if (m_bInitialized==false) return false;
//	if (!IsVisible()) return false;
//
//#ifdef ENABLE_CHARACTER_COLLISION
#define CHARACTER_COLLISION_DIST	70.f
//	// 캐릭터끼리의 충돌체크를 한다.
///*
//	rvector dir=m_Position+diff-m_Position;
//	//float t = ColTest(m_Position, dir, );
//	rplane pln;
//	//float t = SweepTest(rsphere(m_Position, 100.0f), dir, rsphere(rvector(0.0f,0.0f,0.0f), 100.0f), &pln);
//
//	rvector pos22 = m_Position + rvector(0.0f, 0.0f, 60.0f);
//	rcapsule cap = rcapsule(rvector(0.0f,0.0f,0.0f), rvector(0.0f, 0.0f, 180.0f), 100.0f);
//
//	float t = SweepTest(rsphere(pos22, 100.0f), dir, cap, &pln);
//	if (t < 1.0f) {
//		return false;
//	}
//*/
//	if(!IsDie())
//	{
//		ZObjectManager *pcm=&g_pGame->m_ObjectManager;
//		for (ZObjectManager::iterator itor = pcm->begin(); itor != pcm->end(); ++itor)
//		{
//			ZObject* pObject = (*itor).second;
//			if (pObject != this && pObject->IsCollideable())
//			{
//				rvector pos=pObject->m_Position;
//				rvector dir=m_Position+diff-pos;
//				dir.z=0;
//				float fDist=Magnitude(dir);
//				if(fDist<CHARACTER_COLLISION_DIST && fabs(pos.z-m_Position.z)<180.f)
//				{
//					// 거의 같은위치에 있는 경우.. 한쪽방향으로 밀림
//					if(fDist<1.f)
//					{
//						pos.x+=1.f;
//						dir=m_Position-pos;
//					}
//
//					if(DotProduct(dir,diff)<0)	// 더 가까워지는 방향이면
//					{
//						Normalize(dir);
//						rvector newthispos=pos+dir*(CHARACTER_COLLISION_DIST+1.f);
//
//						rvector newdiff=newthispos-m_Position;
//						diff.x=newdiff.x;
//						diff.y=newdiff.y;
//
//					}
//				}
//			}
//		}
//	}
//#endif
//	// 이렇게되면 195cm 정도까지 못지나간다.
//	
//	rvector origin,targetpos;
//	rplane impactplane;
//
//	origin=m_Position+rvector(0,0,120);
//	targetpos=origin+diff;
//	m_bAdjusted=ZGetGame()->GetWorld()->GetBsp()->CheckWall(origin,targetpos,CHARACTER_RADIUS,60,RCW_CYLINDER,0,&impactplane);
//
//	/*
//	// 지금 턱에걸려 올라가는 상황이 아니라면 턱에 걸린것인지 체크.
//
//	// 조건 1. 막혀서 거의 못움직이는 상태이며 거의 위아래로는 움직이지 않을때
//
//	if(Magnitude(GetVelocity())>0.01f)
//	{
//		if(!m_bClimb && 
//			//Magnitude(targetpos-origin)<0.1f && 
//			m_bAdjusted &&
//			fabs(GetVelocity().z)<1.f )
//		{
//			// 2. 현재 딛고있는 바닥이 거의 수직인 바닥인가
//			// 2. 현재 부딪힌 면이 거의 수직인가 !
//			if(D3DXPlaneDotNormal(&impactplane,&rvector(0,0,1))<0.01f)
//			{
//				// 4. 위로 50cm 만큼 갈수있는가 ? 
//				rvector uptest_origin=m_Position+rvector(0,0,90);
//				rvector uptest_targetpos=uptest_origin+rvector(0,0,50);
//				rplane testimpactplane;
//
//				bool bAdjust=ZGetGame()->GetWorld()->GetBsp()->CheckWall(uptest_origin,uptest_targetpos,CHARACTER_RADIUS,89,RCW_CYLINDER,0,&testimpactplane);
//				if(!bAdjust || Magnitude(uptest_targetpos-uptest_origin)>45.f)
//				{
//					rvector forwardtest_origin=uptest_targetpos;
////					rvector dir=-rvector(impactplane.a,impactplane.b,impactplane.c);
//					rvector dir=diff;
//					Normalize(dir);
//					rvector forwardtest_targetpos=forwardtest_origin+30.f*dir;
//
//					bool bAdjust=ZGetGame()->GetWorld()->GetBsp()->CheckWall(forwardtest_origin,forwardtest_targetpos,CHARACTER_RADIUS,89,RCW_CYLINDER,0,&testimpactplane);
//					// 3. 50cm 만큼 위에서 벽방향으로 진행하면 갈수있는가 ? 
//					if(!bAdjust || (Magnitude(forwardtest_targetpos-forwardtest_origin)>20.f && impactplane!=testimpactplane) )
//					{
//						m_bClimb=true;
//						m_ClimbDir=GetVelocity();
//						Normalize(m_ClimbDir);
//						m_fClimbZ=m_Position.z;
//						m_ClimbPlane=impactplane;
//						return m_bAdjusted;
//					}
//				}
//			}
//		}
//	}
//	*/
//
//	if(m_bAdjusted)
//	{
//		m_fLastAdjustedTime=g_pGame->GetTime();
//	}
//
//	diff=targetpos-origin;
//
//	m_Position+=diff;
////	m_RealPosition+=diff;
////	m_RealPositionBefore+=diff;
//
//	return m_bAdjusted;
//}

/*
	임시 ->	확장 필요 없으면 멤버로 옮기거나 ,
	커질것 같으면 파일관리를 포함하는 class 만들기..
*/

#define ST_MAX_WEAPON 200
#define ST_MAX_PARTS  200

struct WeaponST {
	int		id;
	char*	name;
	RWeaponMotionType weapontype;//무기종류
};

// 등록 이름 , 타잎..
/*
WeaponST g_WeaponST[ ST_MAX_WEAPON ] = {
//	{ 0,"w_none",	eq_weapon_etc},
//	{ 0,"katana01",	eq_ws_pistol },
//	{ 1,"rifle01",	eq_ws_pistol },
	{ 0,"pistol01",	eq_ws_pistol },
	{ 1,"pistol02",	eq_ws_pistol },
//	{ 0,"katana01",	eq_ws_pistol },
//	{ 1,"rifle01",	eq_ws_pistol },
	{ 2,"katana01",	eq_wd_katana },
	{ 3,"rifle01",	eq_wd_rifle  },

};
*/

// 등록 mesh_id , 등록이름 , 모션 연결 타잎

WeaponST g_WeaponST[ ST_MAX_WEAPON ] = {
	{ 0,"pistol01",	eq_wd_katana },
	{ 1,"pistol02",	eq_wd_katana },
	{ 2,"katana01",	eq_wd_katana },
	{ 3,"rifle01",	eq_wd_rifle  },
};

void ZCharacter::SetTargetDir(rvector vTarget) {

	Normalize(vTarget);
	m_TargetDir = vTarget;
//	m_dwBackUpTime = timeGetTime();
}

// 가진 무기중에서만 선택하게 된다...

void ZCharacter::OnChangeWeapon(char* WeaponModelName)
{
	// 초기 무기가 틀려지는 문제일수도..
	// 로딩이 안되었다면 모션은 동기화가 안된다? 유저들의 로딩속도에 따라 틀려질수도?

	if(m_bInitialized==false) 
		return;

	if( m_pVMesh ) {

		RWeaponMotionType type = eq_weapon_etc;

		RMesh* pMesh = ZGetWeaponMeshMgr()->Get( WeaponModelName );

		if( pMesh ) {

			type = pMesh->GetMeshWeaponMotionType();

			CheckTeenVersionMesh(&pMesh);

			m_pVMesh->AddWeapon(type , pMesh);
			m_pVMesh->SelectWeaponMotion(type);
			UpdateLoadAnimation();//상태는 그대로지만 무기가바뀌었으니 모션파일이 바뀌어야한다..
		}

		if( eq_wd_katana == type )
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("fx_blade_sheath",m_Position,IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySound("fx_blade_sheath",m_Position,IsObserverTarget());
#endif
		}
		else if( (eq_wd_dagger == type) || (eq_ws_dagger == type) )
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("fx_dagger_sheath",m_Position,IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySound("fx_dagger_sheath",m_Position, IsObserverTarget());
#endif
		}
		else if( eq_wd_sword == type )
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("fx_dagger_sheath",m_Position,IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySound("fx_dagger_sheath",m_Position, IsObserverTarget());
#endif
		}
		else if( eq_wd_blade == type )
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("fx_dagger_sheath",m_Position,IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySound("fx_dagger_sheath",m_Position, IsObserverTarget());
#endif
		}
	}
	
}

// 디자이너 작업 때문에... Debug 모드에서만 사용~

char* GetPartsNextName(RMeshPartsType ptype,RVisualMesh* pVMesh,bool bReverse)
{

	static bool bFirst = true;
	static vector<RMeshNode*> g_table[6*2];
	static int g_parts[6*2];

	if(bFirst) {

		RMesh* pMesh = ZGetMeshMgr()->Get("heroman1");//원하는 모델을 붙여주기..

		if(pMesh) { //man

			pMesh->GetPartsNode( eq_parts_chest,g_table[0]);
			pMesh->GetPartsNode( eq_parts_head ,g_table[1]);
			pMesh->GetPartsNode( eq_parts_hands,g_table[2]);
			pMesh->GetPartsNode( eq_parts_legs ,g_table[3]);
			pMesh->GetPartsNode( eq_parts_feet ,g_table[4]);
			pMesh->GetPartsNode( eq_parts_face ,g_table[5]);
		}

		pMesh = ZGetMeshMgr()->Get("herowoman1");//원하는 모델을 붙여주기..
		
		if(pMesh) { //woman

			pMesh->GetPartsNode( eq_parts_chest,g_table[6]);
			pMesh->GetPartsNode( eq_parts_head ,g_table[7]);
			pMesh->GetPartsNode( eq_parts_hands,g_table[8]);
			pMesh->GetPartsNode( eq_parts_legs ,g_table[9]);
			pMesh->GetPartsNode( eq_parts_feet ,g_table[10]);
			pMesh->GetPartsNode( eq_parts_face ,g_table[11]);
		}

		bFirst = false;
	}

	int mode = 0;

		 if(ptype==eq_parts_chest)	mode = 0;
	else if(ptype==eq_parts_head)	mode = 1;
	else if(ptype==eq_parts_hands)	mode = 2;
	else if(ptype==eq_parts_legs)	mode = 3;
	else if(ptype==eq_parts_feet)	mode = 4;
	else if(ptype==eq_parts_face)	mode = 5;
	else return NULL;

	// isman()

	if(pVMesh) {
		if(pVMesh->m_pMesh) {
			if(strcmp(pVMesh->m_pMesh->GetName(),"heroman1")!=0) {
				mode +=6;
			}
		}
	}

	if(bReverse) { // 역순검색..

		g_parts[mode]--;

		if(g_parts[mode] < 0) {
			g_parts[mode] = (int)g_table[mode].size()-1;
		}

	}
	else {

		g_parts[mode]++;

		if(g_parts[mode] > (int)g_table[mode].size()-1) {
			g_parts[mode] = 0;
		}
	}

	return g_table[mode][g_parts[mode]]->GetName();
}

void ZCharacter::OnChangeParts(RMeshPartsType partstype,int PartsID)
{
#ifndef _PUBLISH
	if (m_bInitialized==false) return;
	if( m_pVMesh ) {

		// 기획서에 정해진 고유한 이름이 있어야 한다..

		if(partstype > eq_parts_etc && partstype < eq_parts_left_pistol) {
		
			if(PartsID == 0) { // clear
				m_pVMesh->SetBaseParts( partstype );
			}
			else {

				char* Name = NULL;

				if(MEvent::GetCtrlState()) {
					Name = GetPartsNextName( partstype,m_pVMesh ,true);//이전옷...
				}
				else {
					Name = GetPartsNextName( partstype,m_pVMesh ,false);
				}

				if(Name)
					m_pVMesh->SetParts( partstype, Name );
			}
		}
	}

	if(Enable_Cloth)
		m_pVMesh->ChangeChestCloth(1.f,1);
#endif
}


void ZCharacter::OnAttack(int type,rvector& pos)
{

}

int ZCharacter::GetHP()	{ 
	return m_pModule_HPAP->GetHP(); 
}

int ZCharacter::GetAP()	{ 
	return m_pModule_HPAP->GetAP(); 
}

// 모든 캐릭터의 HP/AP 의 변경은 이 평션을 통해 이루어진다
void ZCharacter::SetHP(int nHP)
{ 
	m_pModule_HPAP->SetHP(nHP);
}

void ZCharacter::SetAP(int nAP)
{ 
	m_pModule_HPAP->SetAP(nAP); 
}

void ZCharacter::Die()
{
	OnDie();
}

void ZCharacter::OnDie()
{
	if (m_bInitialized==false) return;
	if (!IsVisible()) return;
/*
	if ((GetStateLower() != ZC_STATE_LOWER_DIE1) && (GetStateLower() != ZC_STATE_LOWER_DIE2))
	{
		if(DotProduct(m_Direction,m_LastDamageDir)<0)
			SetAnimationLower(ZC_STATE_LOWER_DIE1);
		else
			SetAnimationLower(ZC_STATE_LOWER_DIE2);
	}
	if (GetStateUpper() != ZC_STATE_UPPER_NONE)
	{
		SetAnimationUpper(ZC_STATE_UPPER_NONE);
	}
*/
	m_bDie = true;
	m_Collision.bCollideable = false;
	m_bPlayDone = false;
	
}

void ZCharacter::GetPositions(v3 & Head, v3 & Foot, double Time)
{
	auto GetItemDesc = [&](MMatchCharItemParts slot)
	{
		return m_Items.GetDesc(slot);
	};

	BasicInfoHistory.GetPositions(Head, Foot, Time, GetItemDesc, m_Property.nSex);
}

// 부활 - 이것은 게임룰에 따라 달라질 수도 있다.
void ZCharacter::Revival()
{
	if (m_bInitialized==false) return;
//	if (!IsVisible()) return;

	InitStatus();

	m_bDie = false;
	m_Collision.bCollideable = true;

	if(IsAdminHide())
		m_bDie = true;

	SetAnimationLower(ZC_STATE_LOWER_IDLE1);
}



void ZCharacter::SetDirection(rvector& dir)
{
	m_Direction = dir;
	m_DirectionLower = dir;
	m_DirectionUpper = dir;
	m_TargetDir = dir;
}

/*
void ZCharacter::SetAnimationForce(ZC_STATE nState, ZC_STATE_SUB nStateSub)
{
	m_State = nState;
	m_StateSub = nStateSub;

	char szName[256];
	if (nState != ZC_STATE_IDLE)
		strcpy_safe(szName, g_AnimationInfoSubRunTable[nStateSub].Name);
	else strcpy_safe(szName, "idle");

	SetAnimation(szName, g_AnimationInfoTable[m_State].bEnableCancel, 0);

	if(m_State == ZC_STATE_ATTACK) 
	{
		//아래모션은 놔두고..상체만
		SetAnimation(ani_mode_upper,"attackS",true,0);
	}
}
*/

void ZCharacter::OnKnockback(const rvector& dir, float fForce)
{
	// 남의 캐릭터는 넉백을 없앤다
	if(IsHero())
		ZCharacterObject::OnKnockback(dir,fForce);
}

//void ZCharacter::OnDamagedFalling(float fDamage)
//{
//	if (m_bInitialized==false) return;
//
//	m_LastAttacker = m_UID;
//	m_LastDamageType = ZD_FALLING;
//	m_LastDamageDir = m_Direction;
//
//	SetHP(GetHP() - fDamage);
//}
//
//#define SPLASH_DAMAGE_RATIO	.4f		// 스플래시 데미지 관통률
//
//void ZCharacter::OnDamagedKatanaSplash(ZCharacter* pAttacker,float fDamageRange)
//{
//	if (m_bInitialized==false) return;
//	if (!IsVisible()) return;
//
//	// 거리에 따라서 데미지~
//	bool bCanAttack = g_pGame->IsAttackable(pAttacker,this);
//
//	if (bCanAttack) m_LastAttacker= pAttacker->m_UID;
//	m_LastDamageType = ZD_KATANA_SPLASH;
//	m_LastDamageDir = m_Position-pAttacker->m_Position;
//	Normalize(m_LastDamageDir);
//
//	int damage = 0;
//
//	MMatchItemDesc* pDesc = pAttacker->GetItems()->GetSelectedWeapon()->GetDesc();
//	if (pDesc == NULL)
//	{
//		_ASSERT(0);
//		return;
//	}
//
//	if (bCanAttack) {
//
//#define SLASH_DAMAGE	3		// 강베기데미지 = 일반공격의 x SLASH_DAMAGE
//		damage = (int) pDesc->m_nDamage * fDamageRange * SLASH_DAMAGE;
//
//		OnDamage(damage, SPLASH_DAMAGE_RATIO);
//		//SetHP(GetStatus()->nHP - damage);
//	}
//}
//
//void ZCharacter::OnDamagedGrenade(MUID uidOwner, rvector& dir, float fDamage, int nTeamID)
//{
//	if (m_bInitialized==false) return;
//	if (!IsVisible()) return;
//
//	// 공격한 사람이 나갔어도 이미 있는 수류탄은 터져야한다
////	if (pAttacker == NULL) return;
//
//	// 지금 데미지를 줄수 있는 상황인가 ?
//	bool bCanAttack = g_pGame->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY;
//
//	if (bCanAttack) {
//
//		m_LastAttacker= uidOwner;
//		m_LastDamageType = ZD_EXPLOSION;
//		m_LastDamageDir = dir;
//
//		ZCharacter* pOwnerCharacter = g_pGame->m_CharacterManager.Find( uidOwner );
//
//		bool bForce = this == pOwnerCharacter;// 자신은 팀플이라도 데미지를 입는다.
//
//		float fRatio = 1.0f;
//
////		MMatchWeaponType wtype = z_wd_grenade;//pAttacker->m_pVMesh->GetSelectWeaponType();
//
//		fRatio = ZItem::GetPiercingRatio( MWT_FRAGMENTATION , eq_parts_chest );//수류탄과 로켓 구분없다..
//
//		if(bForce || GetTeamID()!=nTeamID)
//			OnDamage((int)fDamage,fRatio);
//
//		if( pOwnerCharacter == g_pGame->m_pMyCharacter )
//			g_pGame->m_pMyCharacter->HitSound();
//	}
//}

//void ZCharacter::OnKnockback(ZItem *pItem, rvector& pos, rvector& dir, int nHitCount)
//{
//	if (m_bInitialized==false) return;
//	if (!IsVisible() || IsDie()) return;
//
//	float fKnockBackForce = 0.0f;
//	bool bMelee = false;
//	bool bKnockBackSkip = false;
//
//	if(nHitCount != -1) {	// Shotgun 임...
//		if(nHitCount > 5) {	// 6발까지만 KnockBack 에 영향을 준다..
//			bKnockBackSkip = true;
//		}
//	}
//
//	MMatchItemDesc* pDesc = pItem->GetDesc();
//	if (pDesc->m_nType == MMIT_MELEE) {
//		fKnockBackForce = 0.0f;
//		bMelee = true;
//	}
//	else if (pDesc->m_nType == MMIT_RANGE) {
//
//		if (pDesc->m_pEffect != NULL)
//		{
//			fKnockBackForce = (float)(pDesc->m_pEffect->m_nKnockBack);
//		}
//		else
//		{
//			//_ASSERT(0);
//			fKnockBackForce = 0.0f;
//			fKnockBackForce = 200.0f;
//		}
//	}
//	else {
//		_ASSERT(0);
//		return;
//	}
//
//	float fRatio = 1.0f;
//
//	// KnockBack
//	if (IsHero())
//	{
//		if(bKnockBackSkip==false) {
//
//			if(m_bBlast || m_bBlastFall) {	// 떠있을때 넉백
//				rvector vKnockBackDir = dir;
//				Normalize(vKnockBackDir);
//				vKnockBackDir *= (fKnockBackForce * BLASTED_KNOCKBACK_RATIO);
//				vKnockBackDir.x = vKnockBackDir.x * 0.2f;
//				vKnockBackDir.y = vKnockBackDir.y * 0.2f;
//				SetVelocity(vKnockBackDir);
////				SetAccel(vKnockBackDir);
//			} else {	// 그냥 넉백
//				rvector vKnockBackDir = dir;
//				Normalize(vKnockBackDir);
//				if (bMelee) vKnockBackDir = rvector(0.0f, 0.0f, 1.0f);
//				KnockBack(vKnockBackDir, fKnockBackForce);
//			}
//		}
//	}
//}

void ZCharacter::UpdateSound()
{
	if (m_bInitialized==false) return;
	if(m_pVMesh) {

		char szSndName[128];
		RMATERIAL* pMaterial = NULL;
		RBSPPICKINFO bpi;
		if(ZGetGame()->GetWorld()->GetBsp()->Pick(m_Position+rvector(0,0,100),rvector(0,0,-1),&bpi)) {
			pMaterial = ZGetGame()->GetWorld()->GetBsp()->GetMaterial(bpi.pNode, bpi.nIndex);
		}


		//	발자국 소리 하드코드 !    

		AniFrameInfo* pInfo = m_pVMesh->GetFrameInfo(ani_mode_lower);

		int nFrame = pInfo->m_nFrame;//m_pVMesh->m_nFrame[ani_mode_lower];

		int nCurrFoot = 0;

#define FRAME(x) int(float(x)/30.f*4800.f)
		if(m_AniState_Lower==ZC_STATE_LOWER_RUN_FORWARD ||	// 일반적인 걷기
			m_AniState_Lower==ZC_STATE_LOWER_RUN_BACK) {
			
			if(FRAME(8) < nFrame && nFrame < FRAME(18) )	// 대략 8, 18 프레임에서 소리가 난다
				nCurrFoot = 1;
		}

		if(m_AniState_Lower==ZC_STATE_LOWER_RUN_WALL_LEFT ||	// 벽달릴때
			m_AniState_Lower==ZC_STATE_LOWER_RUN_WALL_RIGHT ) {

			if (nFrame < FRAME(9) ) nCurrFoot = 1;
			else if (nFrame < FRAME(17) ) nCurrFoot = 0;
			else if (nFrame < FRAME(24) ) nCurrFoot = 1;
			else if (nFrame < FRAME(32) ) nCurrFoot = 0;
			else if (nFrame < FRAME(40) ) nCurrFoot = 1;
			else if (nFrame < FRAME(48) ) nCurrFoot = 0;
			else if (nFrame < FRAME(55) ) nCurrFoot = 1;
		}

		if(m_AniState_Lower==ZC_STATE_LOWER_RUN_WALL ) {	// 앞으로 벽탈때

			if (nFrame < FRAME(8) ) nCurrFoot = 1;
			else if (nFrame < FRAME(16) ) nCurrFoot = 0;
			else if (nFrame < FRAME(26) ) nCurrFoot = 1;
			else if (nFrame < FRAME(40) ) nCurrFoot = 0;
		}

		//	지금 왼발인지 오른발인지 판단하고 발이 바뀌면 소리를 낸다
		if(m_nWhichFootSound!=nCurrFoot && pMaterial) {	
			if(m_nWhichFootSound==0)
			{	
				// 왼발
				rvector pos = m_pVMesh->GetLFootPosition();
				char *szSndName=g_pGame->GetSndNameFromBsp("man_fs_l", pMaterial);

#ifdef _BIRDSOUND
				ZApplication::GetSoundEngine()->PlaySoundCharacter(szSndName,pos,IsObserverTarget());
#else
				ZApplication::GetSoundEngine()->PlaySound(szSndName,pos,IsObserverTarget());
#endif
			}else
			{
				rvector pos = m_pVMesh->GetRFootPosition();
				char *szSndName=g_pGame->GetSndNameFromBsp("man_fs_r", pMaterial);
#ifdef _BIRDSOUND
				ZApplication::GetSoundEngine()->PlaySoundCharacter(szSndName,pos,IsObserverTarget());
#else
				ZApplication::GetSoundEngine()->PlaySound(szSndName,pos,IsObserverTarget());
#endif
			}
			m_nWhichFootSound=nCurrFoot;
		}
         
		RAniSoundInfo* pSInfo;
		RAniSoundInfo* pSInfoTable[2];

		rvector p;

		AniFrameInfo* pAniLow = m_pVMesh->GetFrameInfo(ani_mode_lower);
		AniFrameInfo* pAniUp  = m_pVMesh->GetFrameInfo(ani_mode_upper);

		pSInfoTable[0] = &pAniLow->m_SoundInfo;
		pSInfoTable[1] = &pAniUp->m_SoundInfo;

		for(int i=0;i<2;i++) {//상하

			pSInfo = pSInfoTable[i];//&m_pVMesh->m_SoundInfo[i];

			if(pSInfo->isPlay) 
			{
				p = pSInfo->Pos;

//				if(strcmp(pSInfo->Name,"fx_dash")==0)
//					int k=0;

				if(pMaterial)	// picking 성공했으면
				{
					strcpy_safe(szSndName, g_pGame->GetSndNameFromBsp(pSInfo->Name, pMaterial));

					int nStr = (int)strlen(szSndName);
					strncpy(m_pSoundMaterial, szSndName + (nStr - 6), 7);

					ZApplication::GetSoundEngine()->PlaySoundElseDefault(szSndName, pSInfo->Name, p, IsObserverTarget());
				}
				else {
					m_pSoundMaterial[0] = 0;

					strcpy_safe(szSndName, pSInfo->Name);
#ifdef _BIRDSOUND
					ZApplication::GetSoundEngine()->PlaySoundCharacter(szSndName,p,IsObserverTarget());
#else
					ZApplication::GetSoundEngine()->PlaySound(szSndName,p,IsObserverTarget());
#endif
				}

				pSInfo->Clear();
			}
		}
	}

	// TODO : onscream 으로 정리하자
	if ( m_bDamaged && (!IsDie()) && (GetHP() < 30.f))
	{
		if(GetProperty()->nSex==MMS_MALE)
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("ooh_male",m_Position,IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySound("ooh_male",m_Position,IsObserverTarget());
#endif
		}
		else			
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("ooh_female",m_Position,IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySound("ooh_female",m_Position,IsObserverTarget());
#endif
		}
		m_bDamaged = false;
	}
}

bool ZCharacter::DoingStylishMotion()
{
	if ((m_AniState_Lower >= ZC_STATE_LOWER_RUN_WALL_LEFT) && 
		(m_AniState_Lower <= ZC_STATE_LOWER_JUMP_WALL_BACK))
	{
		return true;
	}

	return false;
}

void ZCharacter::UpdateStylishShoted()
{
	if (DoingStylishMotion())
	{
		m_bStylishShoted = true;
	}
	else
	{
		m_bStylishShoted = false;
	}
}

void ZCharacter::InitHPAP()
{
	m_pModule_HPAP->SetMaxHP(m_Property.fMaxHP);
	m_pModule_HPAP->SetMaxAP(m_Property.fMaxAP);

	m_pModule_HPAP->SetHP(m_Property.fMaxHP);
	m_pModule_HPAP->SetAP(m_Property.fMaxAP);
	// HP, AP 초기화
	SetHP(m_Property.fMaxHP);
	SetAP(m_Property.fMaxAP);

}

void ZCharacter::InitBullet()
{
	// 총알 초기화
	if (!m_Items.GetItem(MMCIP_PRIMARY)->IsEmpty()) 
	{
		int nBullet = m_Items.GetItem(MMCIP_PRIMARY)->GetDesc()->m_nMaxBullet;
		m_Items.GetItem(MMCIP_PRIMARY)->InitBullet(nBullet);
	}
	if (!m_Items.GetItem(MMCIP_SECONDARY)->IsEmpty()) 
	{
		int nBullet = m_Items.GetItem(MMCIP_SECONDARY)->GetDesc()->m_nMaxBullet;
		m_Items.GetItem(MMCIP_SECONDARY)->InitBullet(nBullet);
	}
	if (!m_Items.GetItem(MMCIP_CUSTOM1)->IsEmpty()) 
	{
		int nBullet = m_Items.GetItem(MMCIP_CUSTOM1)->GetDesc()->m_nMaxBullet;
		m_Items.GetItem(MMCIP_CUSTOM1)->InitBullet(nBullet);
	}
	if (!m_Items.GetItem(MMCIP_CUSTOM2)->IsEmpty()) 
	{
		int nBullet = m_Items.GetItem(MMCIP_CUSTOM2)->GetDesc()->m_nMaxBullet;
		m_Items.GetItem(MMCIP_CUSTOM2)->InitBullet(nBullet);
	}
}


void ZCharacter::InitStatus()
{
	InitHPAP();
	InitBullet();


	SetVelocity(0,0,0);
//	m_fLastUpdateTime=g_pGame->GetTime();

	m_bTagger = false;
	m_bCommander = false;
	m_bDie = false;		// 아직 정보가 안온 캐릭터는 잠정적으로 살았다고 생각하기로함
	m_Collision.bCollideable = true;
//	m_bAutoDir = false;
	m_bStylishShoted = false;
	m_bStun = false;

	m_bBlast = false;
	m_bBlastFall = false;
	m_bBlastDrop = false;
	m_bBlastStand = false;
	m_bBlastAirmove = false;

	m_bSpMotion = false;
	m_SpMotion = ZC_STATE_TAUNT;

//	m_bClimb = false;

	m_fLastReceivedTime=0;
	/*
	m_fAveragePingTime=0;
	m_fAccumulatedTimeError=0;
	m_nTimeErrorCount=0;
	*/

	m_fLastKillTime = 0;
	m_nKillsThisRound = 0;
	m_LastDamageType = ZD_NONE;
	SetLastThrower(MUID(0,0), 0.0f);

	EmptyHistory();

	/*
	for(int i=0;i<ZCI_END;i++)
	{
		m_fIconStartTime[i]=-HP_SCALE;
	}
	*/

	// 죽을때 투명해졌으므로..
	if(m_pVMesh)
		m_pVMesh->SetVisibility(1);

	m_bLostConEffect = false;
	//m_bChatEffect	= false;

	m_bCharged = false;
	m_bCharging = false;
	m_bFallingToNarak = false;

	// AdminHide 처리
	if(IsAdminHide()) {
		m_bDie = true;
		SetVisible(false);
	}

#ifndef _PUBLISH
	char szLog[128];
	sprintf_safe(szLog, "ZCharacter::InitStatus() - %s(%u) Initialized \n", 
		GetProperty()->szName, m_UID.Low);
	OutputDebugString(szLog);
#endif

	InitModuleStatus();
}

//당장 보여주기 위한걸로 사용한다..
void ZCharacter::TestChangePartsAll()
{
	if( IsMan() ) {

		OnChangeParts(eq_parts_chest,0);
		OnChangeParts(eq_parts_head	,0);
		OnChangeParts(eq_parts_hands,0);
		OnChangeParts(eq_parts_legs	,0);
		OnChangeParts(eq_parts_feet	,0);
		OnChangeParts(eq_parts_face	,0);

	}
	else {

		OnChangeParts(eq_parts_chest,0);
		OnChangeParts(eq_parts_head	,0);
		OnChangeParts(eq_parts_hands,0);
		OnChangeParts(eq_parts_legs	,0);
		OnChangeParts(eq_parts_feet	,0);
		OnChangeParts(eq_parts_face	,0);
	}
}

#define AddText(s) { str.Add(#s,false); str.Add(" :",false); str.Add(s);}
#define AddTextEnum(s,e) {str.Add(#s,false); str.Add(" :",false); str.Add(#e);}

void ZCharacter::OutputDebugString_CharacterState()
{
	return;

	RDebugStr str;

	str.Add("//////////////////////////////////////////////////////////////" );

	AddText( m_bInitialized );
	AddText( m_bHero );

	AddText( m_nVMID );

	AddText( m_UID.High );
	AddText( m_UID.Low  );
	AddText( m_nTeamID );

	str.AddLine();

	str.Add("######  m_Property  #######\n");


	AddText( m_Property.szName );
	AddText( m_Property.nSex );//기타생략

	str.AddLine();

	str.Add("######  m_Status  #######\n");

	AddText( GetHP() );
	AddText( GetAP() );
	AddText( m_Status.nLife );
//	AddText( m_Status.nScore );
	AddText( m_Status.nKills );
	AddText( m_Status.nDeaths );
	AddText( m_Status.nLoadingPercent );
	AddText( m_Status.nCombo );
	AddText( m_Status.nMaxCombo );
	AddText( m_Status.nAllKill );
	AddText( m_Status.nExcellent );
	AddText( m_Status.nFantastic );
	AddText( m_Status.nHeadShot );
	AddText( m_Status.nUnbelievable );

	str.AddLine();

	str.Add("######  m_Items  #######\n");


	ZItem* pItem = m_Items.GetSelectedWeapon();

	// 선택된 무기
#define IF_SITEM_ENUM(a)		if(a==m_Items.GetSelectedWeaponType())		{ AddTextEnum(m_Items.GetSelectedWeaponType(),a); }
#define ELSE_IF_SITEM_ENUM(a)	else if(a==m_Items.GetSelectedWeaponType())	{ AddTextEnum(m_Items.GetSelectedWeaponType(),a); }

	IF_SITEM_ENUM(MMCIP_HEAD)
	ELSE_IF_SITEM_ENUM(MMCIP_CHEST)
	ELSE_IF_SITEM_ENUM(MMCIP_HANDS)
	ELSE_IF_SITEM_ENUM(MMCIP_LEGS)
	ELSE_IF_SITEM_ENUM(MMCIP_FEET)
	ELSE_IF_SITEM_ENUM(MMCIP_FINGERL)
	ELSE_IF_SITEM_ENUM(MMCIP_FINGERR)
	ELSE_IF_SITEM_ENUM(MMCIP_MELEE)
	ELSE_IF_SITEM_ENUM(MMCIP_PRIMARY)
	ELSE_IF_SITEM_ENUM(MMCIP_SECONDARY)
	ELSE_IF_SITEM_ENUM(MMCIP_CUSTOM1)
	ELSE_IF_SITEM_ENUM(MMCIP_CUSTOM2)



	AddText( m_bDie );
	AddText( m_bStylishShoted );
	AddText( IsVisible() );
	AddText( m_bStun );
	AddText( m_nStunType );
	AddText( m_bPlayDone );

	AddText( m_nKillsThisRound );
	AddText( m_fLastKillTime );

	str.AddLine(1);

#define IF_LD_ENUM(a)		if(a==m_LastDamageType)			{ AddTextEnum(m_LastDamageType,a); }
#define ELSE_IF_LD_ENUM(a)	else if(a==m_LastDamageType)	{ AddTextEnum(m_LastDamageType,a); }

	IF_LD_ENUM(ZD_NONE)
	ELSE_IF_LD_ENUM(ZD_BULLET)
	ELSE_IF_LD_ENUM(ZD_MELEE)
	ELSE_IF_LD_ENUM(ZD_FALLING)
	ELSE_IF_LD_ENUM(ZD_EXPLOSION)
	ELSE_IF_LD_ENUM(ZD_BULLET_HEADSHOT)
	ELSE_IF_LD_ENUM(ZD_KATANA_SPLASH)
	ELSE_IF_LD_ENUM(ZD_HEAL)
	ELSE_IF_LD_ENUM(ZD_REPAIR)
/*
		 if(m_LastDamageType==ZD_NONE)			{	AddTextEnum(m_LastDamageType,ZD_NONE);}
	else if(m_LastDamageType==ZD_BULLET)		{	AddTextEnum(m_LastDamageType,ZD_BULLET);}
*/
	AddText( m_LastDamageDir );
	AddText( GetSpawnTime() );


	AddText( m_fLastValidTime );
	AddText( GetDistToFloor() );
//	AddText( m_FloorPlane ); // rplane
	AddText( m_bLand );
	AddText( m_bWallJump );
	AddText( m_nWallJumpDir );
	AddText( m_bJumpUp );
	AddText( m_bJumpDown );
	AddText( m_bWallJump2 );
	AddText( m_bBlast );
	AddText( m_bBlastFall );
	AddText( m_bBlastDrop );
	AddText( m_bBlastStand );
	AddText( m_bBlastAirmove );
	AddText( m_bSpMotion );
//	AddText( m_SpMotion );

	AddText( m_bDynamicLight );
	AddText( m_iDLightType );
	AddText( m_fLightLife );
	AddText( m_vLightColor );
	AddText( m_fTime );
	AddText( m_bLeftShot );
//	AddText( m_bClimb );
// 	AddText( m_ClimbDir );
//	AddText( m_fClimbZ );
//	AddText( m_ClimbPlane );// rplane
//	AddText( m_pshadow );
//	AddText( m_pCloth );
//	AddText( m_BasicHistory );
//	AddText( m_HPHistory );

	AddText( m_TargetDir );
	AddText( m_Position );
	AddText( m_Direction );
	AddText( m_DirectionLower );
	AddText( m_DirectionUpper );
	AddText( m_RealPositionBefore );
//	AddText( GetVelocity() );
	AddText( m_Accel );

	str.AddLine(1);

	// 풀어서 출력~

	// 상체 에니메이션 상태

#define IF_Upper_ENUM(a)		if(a==m_AniState_Upper)			{ AddTextEnum(m_AniState_Upper,a); }
#define ELSE_IF_Upper_ENUM(a)	else if(a==m_AniState_Upper)	{ AddTextEnum(m_AniState_Upper,a); }

#define IF_Lower_ENUM(a)		if(a==m_AniState_Lower)			{ AddTextEnum(m_AniState_Lower,a); }
#define ELSE_IF_Lower_ENUM(a)	else if(a==m_AniState_Lower)	{ AddTextEnum(m_AniState_Lower,a); }

		 IF_Upper_ENUM(ZC_STATE_UPPER_NONE)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_SHOT)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_RELOAD)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_LOAD)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_START)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_IDLE)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_BLOCK1)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_BLOCK1_RET)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_BLOCK2)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_CANCEL)

/*
	 	 if(m_AniState_Upper==ZC_STATE_UPPER_NONE)				{ AddTextEnum(m_AniState_Upper,ZC_STATE_UPPER_NONE); }
	else if(m_AniState_Upper==ZC_STATE_UPPER_SHOT)				{ AddTextEnum(m_AniState_Upper,ZC_STATE_UPPER_SHOT); }
*/
	// 하체 에니메이션 상태

		 IF_Lower_ENUM(ZC_STATE_LOWER_NONE)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_IDLE1)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_IDLE2)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_IDLE3)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_IDLE4)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_FORWARD)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_BACK)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_LEFT)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_RIGHT)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_UP)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_DOWN)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_DIE1)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_DIE2)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_DIE3)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_DIE4)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_LEFT)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_LEFT_DOWN)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_DOWN)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_RIGHT)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_RIGHT_DOWN)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_TUMBLE_FORWARD)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_TUMBLE_BACK)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_TUMBLE_RIGHT)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_TUMBLE_LEFT)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BIND)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_WALL_FORWARD)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_WALL_BACK)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_WALL_LEFT)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_WALL_RIGHT)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK1)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK1_RET)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK2)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK2_RET)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK3)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK3_RET)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK4)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK4_RET)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK5)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMPATTACK)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_UPPERCUT)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_START)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_IDLE)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_BLOCK1)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_BLOCK1_RET)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_BLOCK2)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_CANCEL)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST_FALL)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST_DROP)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST_STAND)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST_AIRMOVE)

	ELSE_IF_Lower_ENUM(ZC_STATE_DAMAGE)
	ELSE_IF_Lower_ENUM(ZC_STATE_DAMAGE2)
	ELSE_IF_Lower_ENUM(ZC_STATE_DAMAGE_DOWN)

	ELSE_IF_Lower_ENUM(ZC_STATE_TAUNT)
	ELSE_IF_Lower_ENUM(ZC_STATE_BOW)
	ELSE_IF_Lower_ENUM(ZC_STATE_WAVE)
	ELSE_IF_Lower_ENUM(ZC_STATE_LAUGH)
	ELSE_IF_Lower_ENUM(ZC_STATE_CRY)
	ELSE_IF_Lower_ENUM(ZC_STATE_DANCE)

/*
	 	 if(m_AniState_Lower==ZC_STATE_LOWER_NONE)				{ AddTextEnum(m_AniState_Lower,ZC_STATE_LOWER_NONE); }
	else if(m_AniState_Lower==ZC_STATE_LOWER_IDLE1)				{ AddTextEnum(m_AniState_Lower,ZC_STATE_LOWER_IDLE1); }
*/

	// 위의 상태와 다를 수 있는지 조사~

	if(m_pAnimationInfo_Upper) {
		AddText(m_pAnimationInfo_Upper->Name);
	}

	if(m_pAnimationInfo_Lower) {
		AddText(m_pAnimationInfo_Lower->Name);
	}

	str.AddLine(1);

		 if(m_nMoveMode==MCMM_WALK)	{ AddTextEnum(m_nMoveMode,MCMM_WALK); }
	else if(m_nMoveMode==MCMM_RUN)	{ AddTextEnum(m_nMoveMode,MCMM_RUN);  }

		 if(m_nMode==MCM_PEACE)		{ AddTextEnum(m_nMode,MCM_PEACE);	  }
	else if(m_nMode==MCM_OFFENSIVE)	{ AddTextEnum(m_nMode,MCM_OFFENSIVE); }

		 if(m_nState==MCS_STAND)	{ AddTextEnum(m_nState,MCS_STAND);	}
	else if(m_nState==MCS_SIT)		{ AddTextEnum(m_nState,MCS_SIT);	}
	else if(m_nState==MCS_DEAD)		{ AddTextEnum(m_nState,MCS_DEAD);	}

	str.AddLine(1);

//	AddText( m_bAdjusted );
//	AddText( m_fLastAdjustedTime );

	/*
	AddText( m_bAutoDir );
	AddText( m_bLeftMoving );
	AddText( m_bRightMoving );

	AddText( m_bForwardMoving );
	*/
	AddText( m_bBackMoving );
	AddText( m_fLastReceivedTime );
//	AddText( m_fLastUpdateTime );

//	AddText( m_fAveragePingTime );
//	AddText( m_nTimeErrorCount );
//	AddText( m_TimeErrors[20] );
//	AddText( m_fAccumulatedTimeError );
	AddText( m_fGlobalHP );
	AddText( m_nReceiveHPCount );

//	AddText( m_dwBackUpTime );
	AddText( m_nSelectSlot );

//	AddText( m_Slot[ZC_SLOT_END] );

	str.PrintLog();

//	m_pVMesh 의 상태도 출력~

	if(m_pVMesh) {
		m_pVMesh->OutputDebugString_CharacterState();
	}
}

void ZCharacter::TestToggleCharacter()
{
	if(m_pVMesh->m_pMesh) {

		RMesh* pMesh = NULL;

		if( strcmp(m_pVMesh->m_pMesh->GetName(),"heroman1")==0 ) {
			pMesh = ZGetMeshMgr()->Get("herowoman1");//원하는 모델을 붙여주기..
			m_pVMesh->m_pMesh = pMesh;
			m_pVMesh->ClearParts();//남녀가 틀려서.
			TestChangePartsAll();
		}
		else {
			pMesh = ZGetMeshMgr()->Get("heroman1");//원하는 모델을 붙여주기..
			m_pVMesh->m_pMesh = pMesh;
			m_pVMesh->ClearParts();
			TestChangePartsAll();
		}
	}
}

void ZCharacter::InitMesh()
{
	RMesh* pMesh;

	char szMeshName[64];
	if (m_Property.nSex == MMS_MALE)
	{
		strcpy_safe(szMeshName, "heroman1");
	}
	else
	{
		strcpy_safe(szMeshName, "herowoman1");
	}
	pMesh = ZGetMeshMgr()->Get(szMeshName);//원하는 모델을 붙여주기..

	if(!pMesh) {
		mlog("AddCharacter 원하는 모델을 찾을수 없음\n");
	}

	int nVMID = g_pGame->m_VisualMeshMgr.Add(pMesh);

	if(nVMID==-1) {
		mlog("AddCharacter 캐릭터 생성 실패\n");
	}

	m_nVMID = nVMID;

	RVisualMesh* pVMesh = g_pGame->m_VisualMeshMgr.GetFast(nVMID);
	SetVisualMesh(pVMesh);

	// low polygon model 붙여주기..
	// 남녀구분없이
/*
	if (m_Property.nSex == MMS_MALE) {
		strcpy_safe(szMeshName, "heroman_low1");
	}
	else {
		strcpy_safe(szMeshName, "heroman_low2");
	}
*/	
}

void ZCharacter::ChangeLowPolyModel()
{
	if(m_pVMesh==NULL)
		return;

	char szMeshName[64];

	bool cloth_model = false;

	if( m_pVMesh ) {
		cloth_model = m_pVMesh->IsClothModel();
	}

	if ( cloth_model ) {
		strcpy_safe(szMeshName, "heroman_low1");
	}
	else {
		strcpy_safe(szMeshName, "heroman_low2");
	}

	RMesh* pLowMesh = ZGetMeshMgr()->Get(szMeshName);//원하는 모델을 붙여주기..

	m_pVMesh->SetLowPolyModel(pLowMesh);
}

bool ZCharacter::IsAdmin()
{
	if(m_InitialInfo.nUGradeID == MMUG_DEVELOPER ||
		m_InitialInfo.nUGradeID == MMUG_ADMIN)
		return true;
	return false;
}

void ZCharacter::InitProperties()
{
	MTD_CharInfo* pCharInfo = &m_InitialInfo;

	m_Property.SetName(pCharInfo->szName);
	m_Property.SetClanName(pCharInfo->szClanName);
	m_Property.nSex = (MMatchSex)pCharInfo->nSex;
	m_Property.nHair = pCharInfo->nHair;
	m_Property.nFace = pCharInfo->nFace;
	m_Property.nLevel = pCharInfo->nLevel;
	m_Property.fMaxHP = pCharInfo->nHP;
	m_Property.fMaxAP = pCharInfo->nAP;

	// AP 재조정
	float fAddedAP = DEFAULT_CHAR_AP;
	for (int i = 0; i < MMCIP_END; i++)
	{
		if (!m_Items.GetItem(MMatchCharItemParts(i))->IsEmpty())
		{
			fAddedAP += m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nAP;
		}
	}
	// HP 재조정
	float fAddedHP = DEFAULT_CHAR_HP;
	for (int i = 0; i < MMCIP_END; i++)
	{
		if (!m_Items.GetItem(MMatchCharItemParts(i))->IsEmpty())
		{
			fAddedHP += m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nHP;
		}
	}

	m_Property.fMaxAP += fAddedAP;
	m_Property.fMaxHP += fAddedHP;

	if(GetUserGrade() == MMUG_DEVELOPER) {
		strcpy_safe(m_szUserName,ZMsg(MSG_WORD_DEVELOPER));
		strcpy_safe(m_szUserAndClanName,ZMsg(MSG_WORD_DEVELOPER));
	}
	else if(GetUserGrade() == MMUG_ADMIN) {
		strcpy_safe(m_szUserName,ZMsg(MSG_WORD_ADMIN));
		strcpy_safe(m_szUserAndClanName,ZMsg(MSG_WORD_ADMIN));
	}
	else {
		strcpy_safe(m_szUserName,m_Property.szName);
		if(m_Property.szClanName[0])
			sprintf_safe(m_szUserAndClanName,"%s(%s)",m_Property.szName,m_Property.szClanName);
		else
			sprintf_safe(m_szUserAndClanName,"%s",m_Property.szName);
	}

	MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(GetUID());
	if (pObjCache && IsAdminGrade(pObjCache->GetUGrade()) && 
		pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide))
		m_bAdminHide = true;
	else
		m_bAdminHide = false;

	auto& MatchSetting = *ZGetGameClient()->GetMatchStageSetting();
	if (MatchSetting.IsForcedHPAP())
	{
		m_Property.fMaxHP = MatchSetting.GetForcedHP();
		m_Property.fMaxAP = MatchSetting.GetForcedAP();
	}
}

bool ZCharacter::Create(const MTD_CharInfo& CharInfo)
{
	_ASSERT(!m_bInitialized);

	memcpy(&m_InitialInfo, &CharInfo, sizeof(MTD_CharInfo));

	// 아이템 세팅
	for (int i = 0; i < MMCIP_END; i++)
	{
		m_Items.EquipItem(MMatchCharItemParts(i), CharInfo.nEquipedItemDesc[i]);
	}


	InitProperties();

	InitMesh();
	m_bInitialized = true;

	SetAnimationLower(ZC_STATE_LOWER_IDLE1);
	SetAnimationUpper(ZC_STATE_UPPER_NONE);

	InitMeshParts();

	CreateShadow();
	
	m_pSoundMaterial[0] = 0;

	if(Enable_Cloth)
	{
		m_pVMesh->ChangeChestCloth(1.f,1);
	}

	ChangeLowPolyModel();

	m_bIsLowModel = false;
	SetVisible(true);	// 여기서 비로소 보여준다

	m_fAttack1Ratio = 1.f;

	ZGetEmblemInterface()->AddClanInfo(GetClanID());

	MMatchItemDesc *pDesc = m_Items.GetDesc(MMCIP_MELEE);
	if (pDesc)
	{
		switch (pDesc->m_nID)
		{
		case 8501:
			g_RGMain->SetSwordColor(GetUID(), 0xFFFFB7D5);
			break;
		case 8502:
			g_RGMain->SetSwordColor(GetUID(), 0xFF00FF00);
			break;
		case 8503:
			g_RGMain->SetSwordColor(GetUID(), 0xFF00FFFF);
			break;
		}
	}

	return true;
}

void ZCharacter::Destroy()
{
	if(m_bInitialized)
		ZGetEmblemInterface()->DeleteClanInfo(GetClanID());

	m_bInitialized = false;

	DestroyShadow();
}

void ZCharacter::InitMeshParts()
{
	RMeshPartsType mesh_parts_type;

	if (m_pVMesh)
	{
		for (int i = 0; i < MMCIP_END;i++)
		{
			switch (MMatchCharItemParts(i))
			{
			case MMCIP_HEAD:
				mesh_parts_type = eq_parts_head;
				break;
			case MMCIP_CHEST:
				mesh_parts_type = eq_parts_chest;
				break;
			case MMCIP_HANDS:
				mesh_parts_type = eq_parts_hands;
				break;
			case MMCIP_LEGS:
				mesh_parts_type = eq_parts_legs;
				break;
			case MMCIP_FEET:
				mesh_parts_type = eq_parts_feet;
				break;
			default:
				continue;
			}

			if (!GetItems()->GetItem(MMatchCharItemParts(i))->IsEmpty())
			{
				m_pVMesh->SetParts(mesh_parts_type, GetItems()->GetItem(MMatchCharItemParts(i))->GetDesc()->m_szMeshName);
			}
			else
			{
				m_pVMesh->SetBaseParts(mesh_parts_type);
			}
		}	// for

		// 머리 아이템이 없으면 머리카락이 입혀진다. 지금은 서로 메쉬를 공유함
		if (GetItems()->GetItem(MMCIP_HEAD)->IsEmpty())
		{
			ChangeCharHair(m_pVMesh, m_Property.nSex, m_Property.nHair);	
		}
		
		ChangeCharFace(m_pVMesh, m_Property.nSex, m_Property.nFace);

	}


	SetAnimationUpper(ZC_STATE_UPPER_NONE);
	SetAnimationLower(ZC_STATE_LOWER_IDLE1);

	// 처음에 쥐는 아이템
	if (!g_pGame->GetMatch()->IsRuleGladiator())
	{
		if (!m_Items.GetItem(MMCIP_PRIMARY)->IsEmpty()) ChangeWeapon(MMCIP_PRIMARY);
		else if (!m_Items.GetItem(MMCIP_SECONDARY)->IsEmpty()) ChangeWeapon(MMCIP_SECONDARY);
		else if (!m_Items.GetItem(MMCIP_MELEE)->IsEmpty()) ChangeWeapon(MMCIP_MELEE);
		else if (!m_Items.GetItem(MMCIP_CUSTOM1)->IsEmpty()) ChangeWeapon(MMCIP_CUSTOM1);
		else if (!m_Items.GetItem(MMCIP_CUSTOM2)->IsEmpty()) ChangeWeapon(MMCIP_CUSTOM2);
		else ChangeWeapon(MMCIP_PRIMARY);
	}
	else
	{
		if (!m_Items.GetItem(MMCIP_MELEE)->IsEmpty()) ChangeWeapon(MMCIP_MELEE);
		else if (!m_Items.GetItem(MMCIP_CUSTOM1)->IsEmpty()) ChangeWeapon(MMCIP_CUSTOM1);
		else if (!m_Items.GetItem(MMCIP_CUSTOM2)->IsEmpty()) ChangeWeapon(MMCIP_CUSTOM2);
		else ChangeWeapon(MMCIP_PRIMARY);
	}
}

void ZCharacter::ChangeWeapon(MMatchCharItemParts nParts)
{
//	_ASSERT(nParts != 7);

	if(m_Items.GetSelectedWeaponParts()==nParts) return;

	if( nParts < 0 || nParts > MMCIP_END )
	{
		return;
	}
	if (m_Items.GetItem(nParts) == NULL) return;
	if (m_Items.GetItem(nParts)->GetDesc() == NULL) return;

	// 원하지 않는 무기 바꾸기 방지
	/*
	ZANIMATIONINFO* pAnimInfo = 0;
	int amode = 0;
	if( m_AniState_Upper != ZC_STATE_UPPER_NONE )
	{
		pAnimInfo	= m_pAnimationInfo_Upper;
		amode	= ani_mode_upper;
	}
	else
	{
		pAnimInfo	= m_pAnimationInfo_Lower;
		amode	= ani_mode_lower;
	}
	if( !pAnimInfo->bEnableCancel )
	{
		if( !m_pVMesh->m_isPlayDone[amode] )
		{
			return;
		}
	}
//*/
	// 글래디에이터일때는 총무기 사용 금지
	if (g_pGame->GetMatch()->IsRuleGladiator() && !IsAdmin())
	{
		if ((nParts == MMCIP_PRIMARY) || (nParts == MMCIP_SECONDARY)) {
			return;
		}
	}

	MMatchCharItemParts BackupParts = m_Items.GetSelectedWeaponParts();

	m_Items.SelectWeapon(nParts);

	if(m_Items.GetSelectedWeapon()==NULL) return;

	MMatchItemDesc* pSelectedItemDesc = m_Items.GetSelectedWeapon()->GetDesc();

	if (pSelectedItemDesc==NULL) {
		m_Items.SelectWeapon(BackupParts);
		mlog("선택된 무기의 데이터가 없다.\n");
		mlog("ZCharacter 무기상태와 RVisualMesh 의 무기상태가 틀려졌다\n");
		return;
	}

	OnChangeWeapon(pSelectedItemDesc->m_szMeshName);

	if(nParts!=MMCIP_MELEE)
		m_bCharged = false;
}

//#define _CHECKVALIDSHOTLOG

bool ZCharacter::CheckValidShotTime(int nItemID, float fTime, ZItem* pItem)
{
#ifdef _CHECKVALIDSHOTLOG
	char szTime[32]; _strtime(szTime);
	char szLog[256];
#endif

	if (GetLastShotItemID() == nItemID) {
		if (fTime - GetLastShotTime() < (float)pItem->GetDesc()->m_nDelay/1000.0f) {
			MMatchWeaponType nWeaponType = pItem->GetDesc()->m_nWeaponType;
			if ( (MWT_DAGGER <= nWeaponType && nWeaponType <= MWT_DOUBLE_KATANA) &&
				(fTime - GetLastShotTime() >= 0.23f) ) 
			{
				// continue Valid... (칼질 정확한 시간측정이 어려워 매직넘버사용.
			} else if ( (nWeaponType==MWT_DOUBLE_KATANA || nWeaponType==MWT_DUAL_DAGGER) &&
				(fTime - GetLastShotTime() >= 0.11f) ) 
			{
				// continue Valid... (칼질 정확한 시간측정이 어려워 매직넘버사용.
			} else {
				// 불법적인 속도인경우
#ifdef _CHECKVALIDSHOTLOG
				sprintf_safe(szLog, "IGNORE>> [%s] (%u:%u) Interval(%0.2f) Delay(%0.2f) \n", 
					szTime, GetUID().High, GetUID().Low, fTime - GetLastShotTime(), (float)pItem->GetDesc()->m_nDelay/1000.0f);
				OutputDebugString(szLog);	
#endif
				return false;
			}
		}
	}

#ifdef _CHECKVALIDSHOTLOG
	sprintf_safe(szLog, "[%s] (%u:%u) %u(%f)\n", 
			szTime, GetUID().High, GetUID().Low, nItemID, fTime);
	OutputDebugString(szLog);
#endif

	return true;
}

bool ZCharacter::IsObserverTarget()
{
	if (ZApplication::GetGameInterface()->GetCombatInterface()->GetObserver()->GetTargetCharacter() == this)
	{
		return true;
	}

	return false;
}

void ZCharacter::OnDamagedAnimation(ZObject *pAttacker,int type)
{
	// 칼을 맞으면 잠시 멈춘다.  누워있을때 빼고..
	if(pAttacker==NULL)
		return;

	if(!m_bBlastDrop)
	{
		rvector dir = m_Position-pAttacker->m_Position;
		Normalize(dir);

		m_bStun = true;
		SetVelocity(0,0,0);

		float fRatio = GetMoveSpeedRatio();

		// 스턴효과 없애려 머리흔들 애니메이션 없앴다. - 버드
		if(type==SEM_WomanSlash5 || type==SEM_ManSlash5)
		{
			AddVelocity( dir * MAX_SPEED * fRatio );
			m_nStunType = ZST_SLASH;	// 머리 흔드는 애니메이션 뺐다

			ZCharacterObject* pCObj = MDynamicCast(ZCharacterObject, pAttacker);

			if(pCObj) {
				ZC_ENCHANT etype = pCObj->GetEnchantType();
				if( etype == ZC_ENCHANT_LIGHTNING )//라이트닝의 경우 데미지 입은 모션..
					m_nStunType = ZST_LIGHTNING;
			}

			// 강베기로 다시 스턴을 부활 - dubble
//			m_nStunType = 1;

		} else {
			AddVelocity( dir * RUN_SPEED * fRatio );
			m_nStunType = (ZSTUNTYPE)((type) %2);
			if(type<=SEM_ManSlash4)
				m_nStunType=(ZSTUNTYPE)(1-m_nStunType);
		}
		
		m_bPlayDone = false;
	}
}

// 피어 Dead메세지에 맞춰 Dead 애니메이션을 연기 
// - 실제로 죽는 것 판정은 서버에서 직접 받아온 메세지를 가지고 따로 처리한다
void ZCharacter::ActDead()
{
	if (m_bInitialized==false)	return;
	if (!IsVisible())			return;

	rvector vDir = m_LastDamageDir;
	vDir.z = 0.f;
	Normalize(vDir);
	vDir.z = 0.6f;//약간위쪽으로
	Normalize(vDir);

	float fForce = 1.f;

	bool bKnockBack = false;

	SetDeadTime(g_pGame->GetTime());

	if ((GetStateLower() != ZC_STATE_LOWER_DIE1) && 
		(GetStateLower() != ZC_STATE_LOWER_DIE2) && 
		(GetStateLower() != ZC_STATE_LOWER_DIE3) && 
		(GetStateLower() != ZC_STATE_LOWER_DIE4) )
	{

		ZC_STATE_LOWER lower_motion;

		float dot = m_LastDamageDot;

		switch(m_LastDamageWeapon) {

		// melee

		case MWT_DAGGER:
		case MWT_DUAL_DAGGER: 
		case MWT_KATANA:
		case MWT_GREAT_SWORD:
		case MWT_DOUBLE_KATANA:

			bKnockBack = false;

			break;

		case MWT_PISTOL:
		case MWT_PISTOLx2:
		case MWT_REVOLVER:
		case MWT_REVOLVERx2:
		case MWT_SMG:
		case MWT_SMGx2:
		case MWT_RIFLE:
		case MWT_SNIFER:

			// 거리가 8m보다 가까우면
			if( m_LastDamageDistance < 800.f )
			{
				// 400 ~ 900
				fForce = 300 + (1.f-(m_LastDamageDistance/800.f)) * 500.f;

				bKnockBack = true;
			}

			break;

		case MWT_SHOTGUN:
		case MWT_SAWED_SHOTGUN:
		case MWT_MACHINEGUN:

			// 거리가 10m보다 가까우면
			if( m_LastDamageDistance < 1000.f )
			{
				// 500 ~ 1000
				fForce = 400 + (1.f-(m_LastDamageDistance/1000.f)) * 500.f;

				bKnockBack = true;
			}

			break;

		case MWT_ROCKET:
		case MWT_FRAGMENTATION:

			// 날라가는 처리필요

			fForce = 600.f;
			bKnockBack = true;

			break;

		default:
			lower_motion = ZC_STATE_LOWER_DIE1;

		}

		if(m_LastDamageType == ZD_BULLET_HEADSHOT) {
			bKnockBack = true;
			fForce = 700.f;
		}

		if(bKnockBack) {
			// zcharacter의 넉백은 없으므로..
			ZObject::OnKnockback(vDir, fForce );
		}

		if(bKnockBack) {

			if(dot<0)	lower_motion = ZC_STATE_LOWER_DIE3;
			else		lower_motion = ZC_STATE_LOWER_DIE4;
		}
		else {

			if(dot<0)	lower_motion = ZC_STATE_LOWER_DIE1;
			else		lower_motion = ZC_STATE_LOWER_DIE2;
		}

		// 나락 에니메이션 
		if (GetPosition().z <= DIE_CRITICAL_LINE)
		{
			lower_motion = ZC_STATE_PIT;
			m_bFallingToNarak = true;
		}
		SetAnimationLower(lower_motion);

//		mlog("dir : %f %f %f / %d : force %f / dist %f \n",vDir.x,vDir.y,vDir.z,lower_motion,fForce,m_LastDamageDistance);
	}

	if (GetStateUpper() != ZC_STATE_UPPER_NONE )
	{
		SetAnimationUpper(ZC_STATE_UPPER_NONE);
	}

	// excellent 판정
#define EXCELLENT_TIME	3.0f
	ZCharacter *pLastAttacker = ZGetCharacterManager()->Find(GetLastAttacker());
	if(pLastAttacker && pLastAttacker!=this)
	{		
		if(g_pGame->GetTime()-pLastAttacker->m_fLastKillTime < EXCELLENT_TIME && ZApplication::GetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL)
		{
			pLastAttacker->GetStatus()->nExcellent++;
			pLastAttacker->AddIcon(ZCI_EXCELLENT);
		}

		pLastAttacker->m_fLastKillTime=g_pGame->GetTime();		


		// fantastic 판정
		if(!m_bLand && GetDistToFloor()>200.f && ZApplication::GetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL)
		{
			pLastAttacker->GetStatus()->nFantastic++;
			pLastAttacker->AddIcon(ZCI_FANTASTIC);
		}

		// unbelievable 판정
		if(pLastAttacker && ZApplication::GetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL)
		{
			pLastAttacker->m_nKillsThisRound++;
			if(pLastAttacker->m_nKillsThisRound==3)
				pLastAttacker->GetStatus()->nUnbelievable++;
			if(pLastAttacker->m_nKillsThisRound>=3)
			{
				pLastAttacker->AddIcon(ZCI_UNBELIEVABLE);
			}
		}
	}
}

void ZCharacter::AddIcon(int nIcon)
{
	if(nIcon<0 || nIcon>=5) return;

	ZGetEffectManager()->AddCharacterIcon(this,nIcon);

	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if(pTargetCharacter == this)
	{
		ZGetScreenEffectManager()->AddPraise(nIcon);
	}

	/*
	float fDelta=g_pGame->GetTime()-m_fIconStartTime[nIcon];
	if(fDelta<CHARACTER_ICON_DELAY) return;	// 나타나 있을때 나왔으면 무시
		
	m_fIconStartTime[nIcon]=g_pGame->GetTime();
	*/
}
/*
float ZCharacter::GetIconStartTime(int nIcon)
{
	if(nIcon<0 || nIcon>=5) return -HP_SCALE;

	return m_fIconStartTime[nIcon];
}
*/

void ZCharacter::ToggleClothSimulation()
{
	if(!m_pVMesh) return;

	if( Enable_Cloth )
		m_pVMesh->ChangeChestCloth(1.f,1);
	else
		m_pVMesh->DestroyCloth();
}

void ZCharacter::Save(ReplayPlayerInfo& rpi)
{
	rpi.IsHero = IsHero();
	rpi.Info = m_InitialInfo;
	rpi.State.UID = m_UID;
	rpi.State.Property = m_Property;
	rpi.State.HP = m_pModule_HPAP->GetHP();
	rpi.State.AP = m_pModule_HPAP->GetAP();
	rpi.State.Status = m_Status;
	m_Items.Save(rpi.State.BulletInfos);
	rpi.State.Position = m_Position;
	rpi.State.Direction = m_Direction;
	rpi.State.Team = m_nTeamID;
	rpi.State.Dead = m_bDie;
	rpi.State.HidingAdmin = m_bAdminHide;
}

void ZCharacter::Load(const ReplayPlayerInfo& rpi)
{
	m_InitialInfo = rpi.Info;
	m_UID = rpi.State.UID;
	m_Property = rpi.State.Property;
	m_pModule_HPAP->SetHP(rpi.State.HP);
	m_pModule_HPAP->SetAP(rpi.State.AP);
	m_Status = rpi.State.Status;
	m_Items.Load(rpi.State.BulletInfos);
	m_Position = rpi.State.Position;
	m_Direction = rpi.State.Direction;
	m_nTeamID = rpi.State.Team;
	m_bDie = rpi.State.Dead;
	m_bAdminHide = rpi.State.HidingAdmin;
}

void ZCharacter::OnLevelDown()
{
	m_Property.nLevel--;
}
void ZCharacter::OnLevelUp()
{
	m_Property.nLevel++;
	ZGetEffectManager()->AddLevelUpEffect(this);
}

void ZCharacter::LevelUp()
{
	OnLevelUp();
}
void ZCharacter::LevelDown()
{
	OnLevelDown();
}


RMesh *ZCharacter::GetWeaponMesh(MMatchCharItemParts parts)
{
	ZItem *pWeapon = m_Items.GetItem(parts);

	if(!pWeapon) return NULL;

	if( pWeapon->GetDesc()==NULL ) return NULL;

	RMesh* pMesh = ZGetWeaponMeshMgr()->Get( pWeapon->GetDesc()->m_szMeshName );
	return pMesh;
}

bool ZCharacter::IsRunWall()
{
	ZC_STATE_LOWER s = m_AniState_Lower;

	if( ( s == ZC_STATE_LOWER_RUN_WALL_LEFT ) || 
		( s == ZC_STATE_LOWER_RUN_WALL_LEFT_DOWN ) || 
		( s == ZC_STATE_LOWER_RUN_WALL ) || 
		( s == ZC_STATE_LOWER_RUN_WALL_DOWN_FORWARD ) || 
		( s == ZC_STATE_LOWER_RUN_WALL_DOWN ) || 
		( s == ZC_STATE_LOWER_RUN_WALL_RIGHT ) || 
		( s == ZC_STATE_LOWER_RUN_WALL_RIGHT_DOWN ) ||
		( s == ZC_STATE_LOWER_JUMP_WALL_FORWARD ) ||
		( s == ZC_STATE_LOWER_JUMP_WALL_BACK ) ||
		( s == ZC_STATE_LOWER_JUMP_WALL_LEFT ) ||
		( s == ZC_STATE_LOWER_JUMP_WALL_RIGHT ) ) {
		return true;
	}
	return false;
}

bool ZCharacter::IsMeleeWeapon()
{
	ZItem* pItem = m_Items.GetSelectedWeapon();

	if(pItem) {
		if(pItem->GetDesc()) {
			if(pItem->GetDesc()->m_nType == MMIT_MELEE) {		
				return true;
			}
		}
	}

	return false;
}

bool ZCharacter::IsCollideable()
{
	if (m_Collision.bCollideable)
	{
		return ((!IsDie() && !m_bBlastDrop));
	}

	return m_Collision.bCollideable;
}

bool ZCharacter::IsAttackable()
{
	// 죽어있으면 공격할 수 없다.
	if (IsDie()) return false;
	return true;
}

float ZCharacter::ColTest(const rvector& pos, const rvector& vec, float radius, rplane* out)
{
	return SweepTest(rsphere(pos, radius), vec, rsphere(m_Position, CHARACTER_COLLISION_DIST), out);
}

// 방어중인가 ?
bool ZCharacter::IsGuard() const
{
	return ((ZC_STATE_LOWER_GUARD_IDLE<=m_AniState_Lower && m_AniState_Lower<=ZC_STATE_LOWER_GUARD_BLOCK2) ||
		(ZC_STATE_UPPER_GUARD_IDLE<=m_AniState_Upper && m_AniState_Upper<=ZC_STATE_UPPER_GUARD_BLOCK2));
}

void ZCharacter::AddMassiveEffect(const rvector &pos, const rvector &dir)
{
	ZGetEffectManager()->AddSwordWaveEffect(GetUID(), pos, dir);
}

void ZCharacter::InitRound()
{
	// 온게임넷의 요청으로 짱 아이콘을 달아준다. initround시에, 난입할때 달아준다
	if(GetUserGrade()==MMUG_STAR) {
		ZGetEffectManager()->AddStarEffect(this);
	}
}

ZOBJECTHITTEST ZCharacter::HitTest(const rvector& origin, const rvector& to,float fTime,rvector *pOutPos)
{
	v3 Head, Foot;
	GetPositions(Head, Foot, fTime);
	return PlayerHitTest(Head, Foot, origin, to, pOutPos);
}

//void ZCharacter::OnDamagedMelee(ZObject* pAttacker, float fDamage, float fPiercingRatio, MMatchWeaponType weaponType, int nMeleeType)
//{
//	if (m_bInitialized==false) return;
//	if (!IsVisible() || IsDie()) return;
//
//	bool bCanAttack = g_pGame->IsAttackable(pAttacker,this);
//
//	rvector dir = GetPosition() - pAttacker->GetPosition();
//	Normalize(dir);
//
//// lastAttacker 쓰지 않고 있다
////	if (bCanAttack) m_LastAttacker= pAttacker->m_UID;
//	m_LastDamageDir = dir;
//	m_LastDamageType = ZD_KATANA;
//	m_LastDamageWeapon = weaponType;
//	m_LastDamageDot = DotProduct( m_Direction,dir );
//	m_LastDamageDistance = Magnitude(GetPosition() - pAttacker->GetPosition());
//
//	// hp, ap 계산
//	if (bCanAttack)
//	{
//		ZObject::OnDamagedMelee(pAttacker,fDamage,fPiercingRatio,weaponType,nMeleeType);
//		
//		if( pAttacker == g_pGame->m_pMyCharacter )
//			g_pGame->m_pMyCharacter->HitSound();
//	}
//
//	OnDamagedAnimation(pAttacker,nMeleeType);
//}
//
//void ZCharacter::OnDamagedRange( ZObject* pAttacker, float fDamage, float fPiercingRatio, MMatchWeaponType weaponType)
//{
//	if (m_bInitialized==false) return;
//	if (!IsVisible() || IsDie()) return;
//
//	bool bCanAttack = g_pGame->IsAttackable(pAttacker,this);
//
//	rvector dir = GetPosition() - pAttacker->GetPosition();
//	Normalize(dir);
//
//// lastAttacker 쓰지 않고 있다
////	if (bCanAttack) m_LastAttacker= pAttacker->m_UID;
//	m_LastDamageDir = dir;
//	m_LastDamageType = (partstype==eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET
//	m_LastDamageWeapon = weaponType;
//	m_LastDamageDot = DotProduct( m_Direction,dir );
//	m_LastDamageDistance = Magnitude(GetPosition() - pAttacker->GetPosition());
//
//	// hp, ap 계산
//	if (bCanAttack)
//	{
//		ZObject::OnDamagedRange(pAttacker,fDamage,fPiercingRatio,weaponType);
//		
//		if( pAttacker == g_pGame->m_pMyCharacter )
//			g_pGame->m_pMyCharacter->HitSound();
//	}
//}
//

void ZCharacter::OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType)
{
	if (m_bInitialized==false) return;
	if (!IsVisible() || IsDie()) return;

	// If this isn't called on MyCharacter, it's unreliable predicted damage. Actual damage for other players is reported in HP/AP info packets.
	if (this != ZGetGame()->m_pMyCharacter)
		return;

	// 자기가 쏜 폭발 데미지 & 나락 데미지는 무조건 먹는다
	bool bCanAttack = g_pGame->IsAttackable(pAttacker,this) || (pAttacker==this && (damageType==ZD_EXPLOSION || damageType==ZD_FALLING));

	if ( damageType != ZD_FALLING)
		bCanAttack &= !isInvincible();

	rvector dir = GetPosition() - srcPos;
	Normalize(dir);

	m_LastDamageDir = dir;
	m_LastDamageType = damageType;
	m_LastDamageWeapon = weaponType;
	m_LastDamageDot = DotProduct( m_Direction,dir );
	m_LastDamageDistance = Magnitude(GetPosition() - srcPos);

	// hp, ap 계산
	if (bCanAttack)
		ZObject::OnDamaged(pAttacker,srcPos,damageType,weaponType,fDamage,fPiercingRatio,nMeleeType);

	if(damageType==ZD_MELEE) OnDamagedAnimation(pAttacker,nMeleeType);

	m_bDamaged = true;
}

void ZCharacter::OnScream()
{
	if(GetProperty()->nSex==MMS_MALE)
		ZGetSoundEngine()->PlaySound("ooh_male",m_Position,IsObserverTarget());
	else			
		ZGetSoundEngine()->PlaySound("ooh_female",m_Position,IsObserverTarget());
}

void ZCharacter::OnMeleeGuardSuccess()
{
	ZGetSoundEngine()->PlaySound("fx_guard",m_Position,IsObserverTarget());
}

void ZCharacter::OnShot()
{
	if (m_bChatEffect) m_bChatEffect = false;
}

bool ZCharacter::isInvincible()
{
	return ((int)timeGetTime() < (m_dwInvincibleStartTime + m_dwInvincibleDuration));
}