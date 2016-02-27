#include "stdafx.h"

#include "ZGame.h"
#include <windows.h>
//#include <zmouse.h>

#include "MZFileSystem.h"
#include "RealSpace2.h"
#include "FileInfo.h"
#include "MDebug.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "ZConsole.h"
#include "MCommandLogFrame.h"
#include "ZInterface.h"
#include "ZGameInterface.h"
#include "ZApplication.h"
#include "ZCommandTable.h"
#include "ZPost.h"
#include "ZPostLocal.h"
#include "MStrEx.h"
#include "MMatchItem.h"
#include "ZEffectManager.h"
#include "ZEffectBillboard.h"
#include "Config.h"
#include "MProfiler.h"
#include "MMatchItem.h"
#include "ZScreenEffectManager.h"
#include "RParticleSystem.h"
#include "RDynamicLight.h"
#include "ZConfiguration.h"
#include "ZLoading.h"
#include "Physics.h"
#include "zeffectflashbang.h"
#include "ZInitialLoading.h"
#include "RealSoundEffect.h"
#include "RLenzFlare.h"
#include "ZWorldItem.h"
#include "ZMyInfo.h"
#include "ZNetCharacter.h"
#include "ZSecurity.h"
#include "ZStencilLight.h"
#include "ZMap.h"
#include "ZEffectStaticMesh.h"
#include "ZEffectAniMesh.h"
#include "ZGameAction.h"
#include "ZSkyBox.h"
#include "ZFile.h"
#include "ZObject.h"
#include "ZCharacter.h"
#include "ZMapDesc.h"

#include "MXml.h"
#include "ZGameClient.h"
#include "MUtil.h"
#include "RMeshMgr.h"
#include "RVisualMeshMgr.h"
#include "RMaterialList.h"
#include "RAnimationMgr.h"
#include "ZCamera.h"
#include "Mint4R2.h"
//#include "RParticleSystem.h"
#include "ZItemDesc.h"

#include "MObjectCharacter.h"
#include "MMath.h"
#include "ZQuest.h"
#include "MMatchUtil.h"
#include "ZReplay.h"
#include "ZRuleBerserker.h"
#include "ZApplication.h"
#include "ZGameConst.h"

#include "ZRuleDuel.h"
#include "ZMyCharacter.h"

_USING_NAMESPACE_REALSPACE2

#define ENABLE_CHARACTER_COLLISION		// 캐릭터끼리 충돌체크 
#define ENABLE_ADJUST_MY_DATA			// 투표시스템 동작


// Particle 테스트 ///////////////////////////

struct RSnowParticle : public RParticle , CMemPoolSm<RSnowParticle>
{

	virtual bool Update(float fTimeElapsed);
};


bool RSnowParticle::Update(float fTimeElapsed)
{
	RParticle::Update(fTimeElapsed);

	if (position.z <= -1000.0f) return false;
	return true;
}

class ZSnowTownParticleSystem
{
private:
	RParticles*		m_pParticles[3];
	bool IsSnowTownMap()
	{
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "snow", 4)) return true;

		return false;
	}
public:
	void Update(float fDeltaTime)
	{
		if (!IsSnowTownMap()) return;

#define SNOW_PARTICLE_COUNT_PER_SECOND		400		// 대략 초당 300개쯤 원래 프레임당 14개였음.

		int nSnowParticleCountPerSec = SNOW_PARTICLE_COUNT_PER_SECOND;
		switch (ZGetConfiguration()->GetVideo()->nEffectLevel)
		{
		case Z_VIDEO_EFFECT_HIGH:	break;
		case Z_VIDEO_EFFECT_NORMAL:	nSnowParticleCountPerSec = nSnowParticleCountPerSec / 4; break;
		case Z_VIDEO_EFFECT_LOW:	nSnowParticleCountPerSec = nSnowParticleCountPerSec / 8; break;
		default: nSnowParticleCountPerSec = 0; break;
		}

		int nCount = min(nSnowParticleCountPerSec * fDeltaTime,20);	// 한번에 20개 이상은 안나오도록한다
		for(int i=0;i<nCount;i++)
		{
			RParticle *pp=new RSnowParticle();
			pp->ftime=0;
			pp->position = rvector(RandomNumber(-8000.0f, 8000.0f), RandomNumber(-8000.0f, 8000.0f), 1500.0f);
			pp->velocity = rvector(RandomNumber(-40.0f, 40.0f), RandomNumber(-40.0f, 40.0f), RandomNumber(-150.0f, -250.0f));
			pp->accel=rvector(0,0,-5.f);

			int particle_index = RandomNumber(0, 2);
			if (m_pParticles[particle_index]) m_pParticles[particle_index]->push_back(pp);
		}
	}
	void Create()
	{
		if (!IsSnowTownMap()) return;

		for (int i = 0; i < 3; i++)
		{
			m_pParticles[i] = NULL;
		}

		// 파티클 생성
		m_pParticles[0] = RGetParticleSystem()->AddParticles("sfx/water_splash.bmp", 25.0f);
		m_pParticles[1] = RGetParticleSystem()->AddParticles("sfx/water_splash.bmp", 10.0f);
		m_pParticles[2] = RGetParticleSystem()->AddParticles("sfx/water_splash.bmp", 5.0f);
	}
	void Destroy()
	{
		if (!IsSnowTownMap()) return;

		m_pParticles[0]->Clear();
		m_pParticles[1]->Clear();
		m_pParticles[2]->Clear();
	}
};

static ZSnowTownParticleSystem g_SnowTownParticleSystem;
// Particle 테스트 ///////////////////////////




ZGame*	g_pGame = NULL;
float	g_fFOV = DEFAULT_FOV;
float	g_fFarZ = DEFAULT_FAR_Z;


//RParticleSystem* g_ParticleSystem = 0;
extern sCharacterLight g_CharLightList[NUM_LIGHT_TYPE];

MUID tempUID(0, 0);		// 로컬 테스트용
MUID g_MyChrUID(0, 0);

#define IsKeyDown(key) ((GetAsyncKeyState(key) & 0x8000)!=0)

bool IsMyCharacter(ZObject* pObject)
{
	if ((g_pGame) && ((ZObject*)g_pGame->m_pMyCharacter == pObject)) return true;
	return false;
}

void TestCreateEffect(int nEffIndex)
{
	float fDist = RandomNumber(0.0f, 100.0f);
	rvector vTar = rvector(RandomNumber(0.0f, 100.0f), RandomNumber(0.0f, 100.0f), RandomNumber(0.0f, 100.0f));
	rvector vPos = g_pGame->m_pMyCharacter->GetPosition();
	vPos.x += RandomNumber(0.0f, 100.0f);
	vPos.y += RandomNumber(0.0f, 100.0f);
	vPos.z += RandomNumber(0.0f, 100.0f);

	rvector vTarNormal = -RealSpace2::RCameraDirection;

	vTarNormal = rvector(RandomNumber(0.0f, 100.0f), RandomNumber(0.0f, 100.0f), RandomNumber(0.0f, 100.0f));


	ZCharacter* pCharacter = g_pGame->m_pMyCharacter;
	ZEffectManager* pEM = ZGetEffectManager();

	switch (nEffIndex)
	{
	case 0:
		pEM->AddReBirthEffect(vPos);
		break;
	case 1:
		pEM->AddLandingEffect(vPos, vTarNormal);
		break;
	case 2:
		pEM->AddGrenadeEffect(vPos, vTarNormal);
		break;
	case 3:
		pEM->AddRocketEffect(vPos, vTarNormal);
		break;
	case 4:
		pEM->AddRocketSmokeEffect(vPos);
		break;
	case 5:
		pEM->AddSwordDefenceEffect(vPos,-vTarNormal);
		break;
	case 6:
		pEM->AddSwordWaveEffect(vPos, 200, pCharacter);
		break;
	case 7:
		pEM->AddSwordUppercutDamageEffect(vPos, pCharacter->GetUID());
		break;
	case 8:
		pEM->AddBulletMark(vPos, vTarNormal);
		break;
	case 9:
		pEM->AddShotgunEffect(vPos, vPos, vTar, pCharacter);
		break;
	case 10:
		pEM->AddBloodEffect(vPos, vTarNormal);
		break;
	case 11:
		for (int i = 0; i < SEM_End; i++)
			pEM->AddSlashEffect(vPos, vTarNormal, i);
		break;
	case 12:
		pEM->AddSlashEffectWall(vPos, vTarNormal,0);
		break;
	case 13:
		pEM->AddLightFragment(vPos, vTarNormal);
		break;
	case 14:
		pEM->AddDashEffect(vPos, vTarNormal, pCharacter);
		break;
	case 15:
		pEM->AddSmokeEffect(vPos);
		break;
	case 16:
		pEM->AddSmokeGrenadeEffect(vPos);
		break;
	case 17:
		pEM->AddGrenadeSmokeEffect(vPos, 1.0f, 0.5f, 123);
		break;
	case 18:
		pEM->AddWaterSplashEffect(vPos, vPos);
		break;
	case 19:
		pEM->AddWorldItemEatenEffect(vPos);
		break;
	case 20:
		pEM->AddCharacterIcon(pCharacter, 0);
		break;
	case 21:
		pEM->AddCommanderIcon(pCharacter, 0);
		break;
	case 22:
		pEM->AddChatIcon(pCharacter);
		break;
	case 23:
		pEM->AddLostConIcon(pCharacter);
		break;
	case 24:
		pEM->AddChargingEffect(pCharacter);
		break;
	case 25:
		pEM->AddChargedEffect(pCharacter);
		break;
	case 26:
		pEM->AddTrackFire(vPos);
		pEM->AddTrackFire(vPos);
		pEM->AddTrackFire(vPos);
		pEM->AddTrackFire(vPos);
		break;
	case 27:

		ZEffectWeaponEnchant* pEWE = pEM->GetWeaponEnchant(ZC_ENCHANT_FIRE);

		if(pEWE) {
			//표준 사이즈는 카타나... 100 정도..
			float fSIze = 105.f / 100.f;
			rvector vScale = rvector(0.6f*fSIze,0.6f*fSIze,0.9f*fSIze);// 무기의 크기에 따라서..
			pEWE->SetUid( pCharacter->GetUID() );
			pEWE->SetAlignType(1);
			pEWE->SetScale(vScale);
			pEWE->Draw(timeGetTime());
		}

		break;
	}
}

float CalcActualDamage(ZObject* pAttacker, ZObject* pVictim, float fDamage)
{
	if (g_pGame->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_BERSERKER)
	{
		ZRuleBerserker* pRule = (ZRuleBerserker*)g_pGame->GetMatch()->GetRule();
		if ((pAttacker) && (pAttacker != pVictim) && (pAttacker->GetUID() == pRule->GetBerserkerUID()))
		{
			return fDamage * BERSERKER_DAMAGE_RATIO;			
		}
	}

	return fDamage;
}

void TestCreateEffects()
{
	int nCount = 100;

	for (int i = 0; i < nCount; i++)
	{
		int nEffIndex = RandomNumber(25, 28);
		TestCreateEffect(nEffIndex);
	}
}

ZCharacterManager* ZGetCharacterManager()
{
	if (g_pGame == NULL) return NULL;
	return &g_pGame->m_CharacterManager;
}

ZObjectManager* ZGetObjectManager()
{
	if (g_pGame == NULL) return NULL;
	return &g_pGame->m_ObjectManager;
}

ZGame::ZGame()
{
	g_pGame	= this;

	m_bShowWireframe = false;
	m_pMyCharacter = NULL;

	m_bReserveObserver = false;

	memset(m_nLastTime, 0, sizeof(DWORD) * ZLASTTIME_MAX);

	m_fTime = 0.f;
	m_nReadyState = ZGAME_READYSTATE_INIT;
	m_pParticles = NULL;
	m_render_poly_cnt = 0;
	m_nReservedObserverTime = 0;
	m_nSpawnTime = 0;
	m_t = 0;

	m_bRecording = false;
//	m_pReplayFile = NULL;
	m_pReplayFile = NULL;

	m_bReplaying = false;
	m_bShowReplayInfo = true;
	m_bSpawnRequested = false;

	m_pGameAction = new ZGameAction;

	CancelSuicide();
}

ZGame::~ZGame()
{
	delete m_pGameAction;
	g_SnowTownParticleSystem.Destroy();
	RSnowParticle::Release();
}

bool ZGame::Create(MZFileSystem *pfs, ZLoadingProgress *pLoading )
{
	mlog("ZGame::Create() begin , type = %d\n",ZGetGameClient()->GetMatchStageSetting()->GetGameType());

	SetReadyState(ZGAME_READYSTATE_INIT);	// Sync 맞을때까지 Game Loop 진입않도록

#ifdef _QUEST
	{
		ZGetQuest()->OnGameCreate();
	}
#endif

//	m_ItemDescManager.Create(FILENAME_ZITEM_DESC);	// 나중에 넣어야지

	// world를 세팅
	if (ZGetApplication()->GetLaunchMode()!=ZApplication::ZLAUNCH_MODE_STANDALONE_AI &&
		ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())) {
		for (int i = 0; i < ZGetQuest()->GetGameInfo()->GetMapSectorCount(); i++)
		{
			MQuestMapSectorInfo* pSecInfo = ZGetQuest()->GetSectorInfo(ZGetQuest()->GetGameInfo()->GetMapSectorID(i));
			ZGetWorldManager()->AddWorld(pSecInfo->szTitle);
#ifdef _DEBUG
			mlog("map(%s)\n", pSecInfo ? pSecInfo->szTitle : "null");
#endif
		}
	}else{
		ZGetWorldManager()->AddWorld(ZGetGameClient()->GetMatchStageSetting()->GetMapName());
	}

	if(!ZGetWorldManager()->LoadAll(pLoading))
		return false;

	ZGetWorldManager()->SetCurrent(0);

	//RSetCamera(rvector(-10.f,0.f,150.f),rvector(0.f,0.f,0.f),rvector(0.f,1.f,0.f));
	//RSetProjection(DEFAULT_FOV,DEFAULT_NEAR_Z,DEFAULT_FAR_Z);

	//char szMapFileName[256];
	//char szMapPath[64] = "";
	//ZGetCurrMapPath(szMapPath);

	//sprintf(szMapFileName, "%s%s/%s.rs", 
	//	szMapPath,
	//	ZGetGameClient()->GetMatchStageSetting()->GetMapName(),
	//	ZGetGameClient()->GetMatchStageSetting()->GetMapName());

	//if(!strlen(szMapFileName))
	//	return false;

	mlog("ZGame::Create() :: ReloadAllAnimation Begin \n");
	ZGetMeshMgr()->ReloadAllAnimation();// 읽지 않은 에니메이션이 있다면 로딩
	mlog("ZGame::Create() :: ReloadAllAnimation End \n");

	//ZGetInitialLoading()->SetPercentage( 90.f );
//	ZGetInitialLoading()->SetPercentage( 70.f );
//	ZGetInitialLoading()->Draw( MODE_DEFAULT, 0 , true );
	
	// 난입일때에는 PeerList를 요청한다
	if (ZGetGameClient()->IsForcedEntry())
	{
		ZPostRequestPeerList(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
//		ZPostRequestGameInfo(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
	}

	//if(!GetWorld()->GetBsp()->Open(szMapFileName))
	//{
	//	MLog("error while loading %s \n",szMapFileName);
	//	return false;
	//}

	//mlog("ZGame::Create() GetWorld()->GetBsp()->Open %s \n",szMapFileName);

	//GetWorld()->GetBsp()->OptimizeBoundingBox();

	//ZGetInitialLoading()->SetPercentage( 100.f );

	//GetMapDesc()->Open(&m_bsp);

	g_fFOV = DEFAULT_FOV;

	rvector dir = GetMapDesc()->GetWaitCamDir();
	rvector pos = GetMapDesc()->GetWaitCamPos();
	rvector up(0,0,1);
	RSetCamera(pos, pos+dir, up);



	int nModelID = -1;

	m_Match.Create();
	
	D3DMATERIAL9 mtrl;
	ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );

	mtrl.Diffuse.r = 1.0f;
	mtrl.Diffuse.g = 1.0f;
	mtrl.Diffuse.b = 1.0f;
	mtrl.Diffuse.a = 1.0f;

	mtrl.Ambient.r = 1.0f;
	mtrl.Ambient.g = 1.0f;
	mtrl.Ambient.b = 1.0f;
	mtrl.Ambient.a = 1.0f;

	RGetDevice()->SetMaterial( &mtrl );

	m_fTime=0.f;
	m_bReserveObserver = false;

	
#ifdef _BIRDSOUND
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_BATTLE);
	ZApplication::GetSoundEngine()->PlayMusic();
#else
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_BATTLE, pfs);
	ZApplication::GetSoundEngine()->PlayMusic();
#endif

	m_CharacterManager.Clear();
	m_ObjectManager.Clear();

	mlog("ZGame::Create() m_CharacterManager.Clear done \n");

	m_pMyCharacter = (ZMyCharacter*)m_CharacterManager.Add(ZGetGameClient()->GetPlayerUID(), rvector(0.0f, 0.0f, 0.0f),true);

	
	{
		g_CharLightList[GUN].fLife	= 300;
		g_CharLightList[GUN].fRange = 100;
		g_CharLightList[GUN].iType	= GUN;
		g_CharLightList[GUN].vLightColor.x	= 5.0f;
		g_CharLightList[GUN].vLightColor.y	= 1.0f;
		g_CharLightList[GUN].vLightColor.z	= 1.0f;

		g_CharLightList[SHOTGUN].fLife	= 1000;
		g_CharLightList[SHOTGUN].fRange = 150;
		g_CharLightList[SHOTGUN].iType	= SHOTGUN;
		g_CharLightList[SHOTGUN].vLightColor.x	= 6.0f;
		g_CharLightList[SHOTGUN].vLightColor.y	= 1.3f;
		g_CharLightList[SHOTGUN].vLightColor.z	= 1.3f;

		g_CharLightList[CANNON].fLife	= 1300;
		g_CharLightList[CANNON].fRange	= 200;
		g_CharLightList[CANNON].iType	= CANNON;
		g_CharLightList[CANNON].vLightColor.x	= 7.0f;
		g_CharLightList[CANNON].vLightColor.y	= 1.3f;
		g_CharLightList[CANNON].vLightColor.z	= 1.3f;
	}

	ZGetFlashBangEffect()->SetBuffer();
	ZGetScreenEffectManager()->SetGuageExpFromMyInfo();

#ifdef _BIRDSOUND

#else
	ZGetSoundEngine()->SetEffectVolume( Z_AUDIO_EFFECT_VOLUME );
	ZGetSoundEngine()->SetMusicVolume( Z_AUDIO_BGM_VOLUME );
#endif

	// Net init
	ZApplication::ResetTimer();
	m_GameTimer.Reset();
	ZSetupDataChecker_Game(&m_DataChecker);
	
	ZGetInitialLoading()->SetPercentage( 100.f );
	ZGetInitialLoading()->Draw( MODE_DEFAULT, 0 , true );


#ifdef _BIRDSOUND

#else
	list<AmbSndInfo*> aslist = GetWorld()->GetBsp()->GetAmbSndList();
	for( list<AmbSndInfo*>::iterator iter = aslist.begin(); iter!= aslist.end(); ++iter )
	{
		AmbSndInfo* pAS = *iter;
		if( pAS->itype & AS_AABB)
			ZGetSoundEngine()->SetAmbientSoundBox(pAS->szSoundName, pAS->min, pAS->max, (pAS->itype&AS_2D)?true:false );
		else if( pAS->itype & AS_SPHERE )
			ZGetSoundEngine()->SetAmbientSoundSphere(pAS->szSoundName, pAS->center, pAS->radius, (pAS->itype&AS_2D)?true:false );
	}
#endif

	// 로딩 다 됬어요.. 라고 다른 사람들한테 알린다.
	m_pMyCharacter->GetStatus()->nLoadingPercent = 100;
	ZPostLoadingComplete(ZGetGameClient()->GetPlayerUID(), 100);

	// 게임에 들어갔다고 서알림
	ZPostStageEnterBattle(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());

	char tmpbuf[128];
	_strtime( tmpbuf );

	// 도움말 화면생성..

	mlog("ZGame Created ( %s )\n",tmpbuf);

	ZGetGameInterface()->GetCamera()->SetLookMode(ZCAMERA_DEFAULT);


	g_SnowTownParticleSystem.Create();



	return true;
}

void ZGame::Destroy()
{
	g_SnowTownParticleSystem.Destroy();

	SetClearColor(0);

	mlog("ZGame::Destroy begin\n");

	ZGetGameClient()->AgentDisconnect();

	ZApplication::GetSoundEngine()->StopMusic();
	ZApplication::GetSoundEngine()->CloseMusic();

	mlog("ZGame::Destroy ZApplication::GetSoundEngine()->CloseMusic()\n");

#ifdef _QUEST
	{
		ZGetQuest()->OnGameDestroy();
	}
#endif

	m_Match.Destroy();

	mlog("ZGame::Destroy m_Match.Destroy \n");

	if (ZGetGameClient()->IsForcedEntry())
	{
		ZGetGameClient()->ReleaseForcedEntry();

		ZGetGameInterface()->SerializeStageInterface();
	}

	ZPostStageLeaveBattle(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());

	//SAFE_DELETE( g_ParticleSystem );

	ReleaseFlashBangEffect();

	mlog("ZGame::Destroy SAFE_DELETE(m_pSkyBox) \n");

	RGetLenzFlare()->Clear();

	// 게임이 끝날 때마다 메모리 릴리즈 해줌...
	//ReleaseMemPool(RealSoundEffectFx);
	//ReleaseMemPool(RealSoundEffect);
	//ReleaseMemPool(RealSoundEffectPlay);

//	int e_size = m_EffectManager.m_Effects[0].size();
//	e_size += m_EffectManager.m_Effects[1].size();
//	e_size += m_EffectManager.m_Effects[2].size();
//	int w_size = m_WeaponManager.m_list.size();
//	mlog("ZGame e_size : w_size : %d %d\n",e_size,w_size);

//	ZGetEffectManager()->Clear();
	m_WeaponManager.Clear();
#ifdef _WORLD_ITEM_
	ZGetWorldItemManager()->Clear();
#endif

#ifdef _BIRDSOUND

#else
	ZGetSoundEngine()->ClearAmbientSound();
#endif

	m_ObserverCommandList.Destroy();
	m_ReplayCommandList.Destroy();
	m_DelayedCommandList.Destroy();

	ZGetEffectManager()->Clear();
	ZGetScreenEffectManager()->Clear();

	ZGetWorldManager()->Clear();

	char tmpbuf[128];
	_strtime( tmpbuf );

	mlog("ZGame Destroyed ( %s )\n",tmpbuf);
}

bool ZGame::CreateMyCharacter(MTD_CharInfo* pCharInfo)
{
	if (!m_pMyCharacter) return false;

	m_pMyCharacter->Create(pCharInfo);
	m_pMyCharacter->SetVisible(true);

	ZGetEffectManager()->AddBerserkerIcon(m_pMyCharacter);

	mlog("ZGame::CreateMyCharacter : Name=%s Level=%d \n", pCharInfo->szName, pCharInfo->nLevel);
	return true;
}

bool ZGame::CheckGameReady()
{
	if (GetReadyState() == ZGAME_READYSTATE_RUN) {
		return true;
	} else if (GetReadyState() == ZGAME_READYSTATE_INIT) {
		SetReadyState(ZGAME_READYSTATE_WAITSYNC);
		// 시간 싱크 요청
		ZPostRequestTimeSync(GetTickTime());
		return false;
	} else if (GetReadyState() == ZGAME_READYSTATE_WAITSYNC) {
		return false;
	}
	return false;
}

void ZGame::OnGameResponseTimeSync(unsigned int nLocalTimeStamp, unsigned int nGlobalTimeSync)
{
	ZGameTimer* pTimer = GetGameTimer();
	int nCurrentTick = pTimer->GetGlobalTick();
	int nDelay = (nCurrentTick - nLocalTimeStamp)/2;
	int nOffset = (int)nGlobalTimeSync - (int)nCurrentTick + nDelay;
	
	pTimer->SetGlobalOffset(nOffset);

	SetReadyState(ZGAME_READYSTATE_RUN);
}

void ZGame::Update(float fElapsed)
{
	if (CheckGameReady() == false) {
		OnCameraUpdate(fElapsed);
		return;
	}

	GetWorld()->Update(fElapsed);

	ZGetEffectManager()->Update(fElapsed);
	ZGetScreenEffectManager()->UpdateEffects();

	m_GameTimer.UpdateTick(timeGetTime());
	m_fTime+=fElapsed;
//	AdjustGlobalTime();


	// 다른 플레이어들 업데이트
	//m_CharacterManager.Update(fElapsed);
	m_ObjectManager.Update(fElapsed);

	if(m_pMyCharacter && !m_bReplaying)
	{
		PostBasicInfo();
//		PostHPInfo();
		PostHPAPInfo();
		PostPeerPingInfo();
		PostSyncReport();
	}



	CheckMyCharDead(fElapsed);
	if (m_bReserveObserver)
	{
		OnReserveObserver();
	}

	UpdateCombo();	// 이것은 나중에 MyCharacter로 들어갈 예정


	g_SnowTownParticleSystem.Update(fElapsed);

#ifdef _WORLD_ITEM_
	ZGetWorldItemManager()->update();
#endif
	m_Match.Update(fElapsed);

	if(m_bReplaying) 
		OnReplayRun();
	if(ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
		OnObserverRun();

	ProcessDelayedCommand();



#ifdef _QUEST
	{
		ZGetQuest()->OnGameUpdate(fElapsed);
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	//	TODO : 상황에 따라 업데이트 안해줘도 될 때를 구별해 낼수 있을까?
	//////////////////////////////////////////////////////////////////////////
	//g_ParticleSystem->Update(0.05);
	RGetParticleSystem()->Update(fElapsed);

	if(Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->Update();

	OnCameraUpdate(fElapsed);
}


void ZGame::OnCameraUpdate(float Elapsed)
{
	if (m_Match.GetRoundState() == MMATCH_ROUNDSTATE_PREPARE)
	{
		rvector dir = GetMapDesc()->GetWaitCamDir();
		rvector pos = GetMapDesc()->GetWaitCamPos();
		rvector up(0,0,1);

		RSetCamera(pos, pos+dir, up);
	}
	else
	{
		ZGetGameInterface()->GetCamera()->Update(Elapsed);
	}
}

void ZGame::CheckMyCharDead(float fElapsed)
{
	if(!m_pMyCharacter || m_pMyCharacter->IsDie()) return;

	MUID uidAttacker = MUID(0,0);

	// 나락으로 떨어지면 끝..-_-;
	if (m_pMyCharacter->GetPosition().z < DIE_CRITICAL_LINE)
	{
		//m_pMyCharacter->SetVelocity(rvector(0,0,0));
		uidAttacker = m_pMyCharacter->GetLastThrower();

		ZObject *pAttacker = ZGetObjectManager()->GetObject(uidAttacker);
		if(pAttacker==NULL || !IsAttackable(pAttacker,m_pMyCharacter))
		{
			uidAttacker = ZGetMyUID();
			pAttacker = m_pMyCharacter;
		}

		m_pMyCharacter->OnDamaged(pAttacker,m_pMyCharacter->GetPosition(),ZD_FALLING,MWT_NONE,m_pMyCharacter->GetHP());
		ZChatOutput( ZMsg(MSG_GAME_FALL_NARAK) );
	}

	if ((m_pMyCharacter->IsDie() == false) && (m_pMyCharacter->GetHP() <= 0))
	{
		if (uidAttacker == MUID(0,0) && m_pMyCharacter->GetLastAttacker() != MUID(0,0)) 
			uidAttacker = m_pMyCharacter->GetLastAttacker();

		// 다음라운드로 넘어가기 위한 finish 상태에서는 메시지 라우팅을 생략한다
		if(GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_FINISH) {
			// 죽는 척은 한다
			m_pMyCharacter->ActDead();
			m_pMyCharacter->Die();
			return;
		}

		ZPostDie(uidAttacker);		// 피어들에게 보내는 메세지

		// 퀘스트 모드는 죽음 메세지가 다르다.
		if (! ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
		{
			ZPostGameKill(uidAttacker);	// 서버에 보내는 메세지
		}
		else
		{
			ZPostQuestGameKill();
		}
	}
}

void ZGame::OnPreDraw()
{
	__BP(19,"ZGame::sub1");

	RSetProjection(g_fFOV, DEFAULT_NEAR_Z, g_fFarZ);

	bool bTrilinear=RIsTrilinear();

	RGetDevice()->SetSamplerState( 0, D3DSAMP_MAGFILTER , D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState( 0, D3DSAMP_MINFILTER , D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState( 0, D3DSAMP_MIPFILTER , bTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE );
	RGetDevice()->SetSamplerState( 1, D3DSAMP_MAGFILTER , D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState( 1, D3DSAMP_MINFILTER , D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState( 1, D3DSAMP_MIPFILTER , bTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE );

	if(m_bShowWireframe ) {
		RGetDevice()->SetRenderState( D3DRS_FILLMODE , D3DFILL_WIREFRAME );
		RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
		GetWorld()->SetFog(false);
	}
	else {
		RGetDevice()->SetRenderState( D3DRS_FILLMODE ,  D3DFILL_SOLID );
		GetWorld()->SetFog(true);
	}

	GetWorld()->GetBsp()->SetWireframeMode(m_bShowWireframe);

	rmatrix initmat;
	D3DXMatrixIdentity(&initmat);
	RGetDevice()->SetTransform( D3DTS_WORLD, &initmat );
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, false );
	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, true );

	LPDIRECT3DDEVICE9 pd3dDevice=RGetDevice();
	pd3dDevice->SetTexture(0,NULL);
	pd3dDevice->SetTexture(1,NULL);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 );

	pd3dDevice->SetRenderState(D3DRS_CULLMODE  ,D3DCULL_CW);
	pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS , TRUE );

	if(m_pMyCharacter)
	{
		if( ZGetConfiguration()->GetVideo()->bDynamicLight )
		{
			rvector pos = m_pMyCharacter->GetPosition();
			RGetDynamicLightManager()->SetPosition( pos );
		}
	}

	__EP(19);
}

int g_debug_render_mode = 0;

extern MDrawContextR2* g_pDC;

void ZGame::Draw()
{
	__BP(20,"ZGame::Draw");

	RRESULT isOK=RIsReadyToRender();

	if(isOK==R_NOTREADY)
	{
		__EP(20);
		return ;
	}

	// 자살 확인
	if ( m_bSuicide && ( timeGetTime() > m_dwReservedSuicideTime))
		ZGetGameClient()->RequestGameSuicide();


	OnPreDraw();		// Device 상태값 설정은 여기서 하자

//	RRenderNodeMgr::m_bRenderBuffer = true;//------test code
	/*
	else if(isOK==R_RESTORED) {

	} // restore device dependent objects
	*/

	rmatrix _mat;
	RGetDevice()->GetTransform(D3DTS_WORLD, &_mat);

	__BP(21,"ZGame::Draw::DrawWorld");
	GetWorld()->Draw();
	__EP(21);

	// 맵에 기술된 특수 더미 오브젝트중 그려야 할것이 있는 경우

	ZMapDesc* pMapDesc = GetMapDesc();

	if( pMapDesc ) {
		pMapDesc->DrawMapDesc();
	}

	/*
	D3DLIGHT9 light;
	light.Type			= D3DLIGHT_POINT;
	light.Ambient.r		= 0.1f;
	light.Ambient.g		= 0.1f;
	light.Ambient.b		= 0.1f;
	light.Specular.r	= 1.0f;
	light.Specular.g	= 1.0f;
	light.Specular.b	= 1.0f;
	light.Attenuation0	= 0.05f; 
	light.Attenuation1	= 0.002f; 
	light.Attenuation2	= 0.0f; 

	light.Range			= 200.f;
	light.Position		= m_pMyCharacter->GetPosition();

	light.Diffuse.r		= .9f;
	light.Diffuse.g		= .1f;
	light.Diffuse.b		= .1f;

	GetWorld()->GetBsp()->DrawLight(&light);
	//*/
	if (m_Match.GetRoundState() != MMATCH_ROUNDSTATE_PREPARE)
	{
		__BP(22,"ZGame::Draw::DrawCharacters");

		m_ObjectManager.Draw();

		__EP(22);

		m_render_poly_cnt = RealSpace2::g_poly_render_cnt;

//		RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
//		RGetDevice()->SetTexture(0,NULL);
//		RGetDevice()->SetTexture(1,NULL);
//		RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );
	}

	RGetDevice()->SetTransform(D3DTS_WORLD, &_mat);//map_mat

	ZGetWorldItemManager()->Draw(0,GetWorld()->GetWaterHeight(),GetWorld()->IsWaterMap());

	m_WeaponManager.Render();//weapon

	__BP(50,"ZGame::DrawObjects");

	GetWorld()->GetBsp()->DrawObjects();

	__EP(50);

	__BP(17,"ZGame::Draw::Reflection");

	GetWorld()->GetWaters()->Render();

	__EP(17);

	if (m_Match.GetRoundState() != MMATCH_ROUNDSTATE_PREPARE)
	{
		__BP(23,"ZGame::Draw::DrawWeapons and effects");
#ifndef _PUBLISH
//		TestCreateEffects();
#endif

		ZGetEffectManager()->Draw(timeGetTime());

		__EP(23);
	}

#ifdef _WORLD_ITEM_
	__BP(34,"ZGame::Draw::ZGetWorldItemManager");

		ZGetWorldItemManager()->Draw(1,GetWorld()->GetWaterHeight(),GetWorld()->IsWaterMap());

	__EP(34);
#endif


//	RRenderNodeMgr::m_bRenderBuffer = false;//------test code

	/*
	if(m_bCharacterLight)
	GetWorld()->GetBsp()->DrawLight(&light);
	*/
	//	m_render_poly_cnt = RealSpace2::g_poly_render_cnt;	


	__BP(35,"ZGame::Draw::RGetParticleSystem");

	RGetParticleSystem()->Draw();

	__EP(35);

	__BP(36,"ZGame::Draw::LenzFlare");

	if( RReadyLenzFlare())
	{
		RGetLenzFlare()->Render( RCameraPosition, GetWorld()->GetBsp());
	}

	RSetProjection(DEFAULT_FOV,DEFAULT_NEAR_Z, g_fFarZ);
	RSetFog(FALSE);

	__EP(36);

	__BP(37,"ZGame::Draw::FlashBangEffect");

	if( IsActivatedFlashBangEffect() )
	{
		ShowFlashBangEffect();
	}

	__BP(505, "ZGame::Draw::RenderStencilLight");
	if(Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->Render();
	__EP(505);

	__EP(37);

	__BP(38,"ZGame::Draw::DrawGameMessage");

	m_Match.OnDrawGameMessage();

//	m_HelpScreen.DrawHelpScreen();

	__EP(38);

	__EP(20);

//	빨간라인을 그려본다 화면에 보이면 색이 바뀌도록...? 기본은 파랑 체크되면 빨강...
/*
	rvector line1 = rvector(200,163,168);
	rvector line2 = rvector(900,163,168);

	rmatrix m;

	rvector pos = line1;

	rvector dir = rvector(0,0,1);
	rvector up  = rvector(0,1,0);
	rvector max = rvector( 10, 10, 10);
	rvector min = rvector(-10,-10,-10);

	MakeWorldMatrix(&m,pos,dir,up);

	draw_box(&m,max,min,0xffff0000);

	pos = line2;

	MakeWorldMatrix(&m,pos,dir,up);

	draw_box(&m,max,min,0xffff0000);

	//////////////////////////////////////////////////////////////////////

	D3DXMatrixIdentity(&m);

	RGetDevice()->SetTransform( D3DTS_WORLD, &m );

	RGetDevice()->SetTexture(0,NULL);
	RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
	RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );
	RDrawLine(line1,line2,0xffff0000);

	rvector new_line1;
	rvector new_line2;

	D3DXVec3TransformCoord(&new_line1,&line1,&RViewProjection);
	D3DXVec3TransformCoord(&new_line2,&line2,&RViewProjection);

	rvector tmin = rvector(-1.f,-1.f,0.f);
	rvector tmax = rvector( 1.f, 1.f,1.f);

	D3DXMatrixIdentity(&m);
//	MakeWorldMatrix(&m,rvector(0,0,0),dir,up);
	draw_box(&m,tmax*100,tmin*100,0xff00ffff);

	D3DXMatrixIdentity(&m);
	RGetDevice()->SetTransform( D3DTS_WORLD, &m );
	RGetDevice()->SetTexture(0,NULL);
	RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
	RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );
	RDrawLine(new_line1*100,new_line2*100,0xffffffff);

	/////////////////////////////////////////////////////////////////////

	int nPick = 0;

	if(isInViewFrustum(line1,line2, RGetViewFrustum() )) {
		nPick = 1;
	}
	else 
		nPick = 0;

	char szTemp[256];
	sprintf(szTemp, "line1 = %6.3f %6.3f %6.3f  line2 = %6.3f %6.3f %6.3f Pick %d", new_line1.x,new_line1.y,new_line1.z, new_line2.x,new_line2.y,new_line2.z,nPick);
	g_pDC->Text(100,200,szTemp);
*/

}

void ZGame::DrawDebugInfo()
{
	char szTemp[256] = "";
	int n = 20;
	g_pDC->SetColor(MCOLOR(0xFFffffff));

	for (ZCharacterManager::iterator itor = m_CharacterManager.begin(); itor != m_CharacterManager.end(); ++itor)
	{
		ZCharacter* pCharacter = (*itor).second;
		sprintf(szTemp, "Pos = %6.3f %6.3f %6.3f  Dir = %6.3f %6.3f %6.3f", pCharacter->m_Position.x,
				pCharacter->m_Position.y, pCharacter->m_Position.z, 
				pCharacter->m_Direction.x, pCharacter->m_Direction.y, pCharacter->m_Direction.z);
		g_pDC->Text(20,n,szTemp);
		n += 15;

/*
		sprintf(szTemp, "state = %d , %d", (int)(pCharacter->GetState()), (int)(pCharacter->GetStateSub()));
		g_pDC->Text(20, n, szTemp);
		n+= 15;
*/

		RVisualMesh* pVMesh = pCharacter->m_pVMesh;

		AniFrameInfo* pAniLow = pVMesh->GetFrameInfo(ani_mode_lower);
		AniFrameInfo* pAniUp  = pVMesh->GetFrameInfo(ani_mode_upper);

		sprintf(szTemp,"%s frame down %d / %d ",pAniLow->m_pAniSet->GetName() , pAniLow->m_nFrame , pAniLow->m_pAniSet->GetMaxFrame());
		g_pDC->Text(20,n,szTemp);
		n+= 15;

		if( pAniUp->m_pAniSet ) 
		{
			sprintf(szTemp,"%s frame up %d / %d ",pAniUp->m_pAniSet->GetName(),pAniUp->m_nFrame,pAniUp->m_pAniSet->GetMaxFrame());
			g_pDC->Text(20,n,szTemp);
			n+= 15;
		}
	}

/*
	n = 300;
	for (MMatchPeerInfoList::iterator itor = ZGetGameClient()->GetPeers()->begin();
		 itor != ZGetGameClient()->GetPeers()->end(); ++itor)
	{
		MMatchPeerInfo* pPeerInfo = (*itor);
		sprintf(szTemp, "MUID(%d, %d) , IP = %s, port = %d", pPeerInfo->uidChar.High, 
			    pPeerInfo->uidChar.Low, pPeerInfo->szIP, pPeerInfo->nPort);
		g_pDC->Text(20,n,szTemp);
		n+=15;
	}
*/
}


void ZGame::Draw(MDrawContextR2 &dc)
{
	/*	// 패스노드 출력.. for debug
	char buffer[256];
	sprintf(buffer," state: %d , pathnode: %d",m_pMyCharacter->m_State,m_pMyCharacter->m_nPathNodeID);

	dc.SetColor(MCOLOR(0xFFffffff));
	dc.Text(0,20,buffer);
	//*/
}

void ZGame::ParseReservedWord(char* pszDest, const char* pszSrc)
{
	char szSrc[256];
	char szWord[256];

	strcpy(szSrc, pszSrc);

	char szOut[256];	ZeroMemory(szOut, 256);
	int nOutOffset = 0;

	char* pszNext = szSrc;
	while( *pszNext != NULL ) {
		pszNext = MStringCutter::GetOneArg(pszNext, szWord);

		if ( (*szWord == '$') && (stricmp(szWord, "$player")==0) ) {
			sprintf(szWord, "%d %d", m_pMyCharacter->GetUID().High, m_pMyCharacter->GetUID().Low);
		} else if ( (*szWord == '$') && (stricmp(szWord, "$target")==0) ) {
			sprintf(szWord, "%d %d", m_pMyCharacter->GetUID().High, m_pMyCharacter->GetUID().Low);	// Target생기믄 꼭 Target 으로 바꾸기
		}

		strcpy(szOut+nOutOffset, szWord);	nOutOffset += (int)strlen(szWord);
		if (*pszNext) { 
			strcpy(szOut+nOutOffset, " ");
			nOutOffset++;
		}
	}
	strcpy(pszDest, szOut);
}

#include "ZMessages.h"

extern bool g_bProfile;

// observer 모드에서도 딜레이를 거칠 필요없는 커맨드들
bool IsIgnoreObserverCommand(int nID)
{
	switch(nID) {
		case MC_PEER_PING :
		case MC_PEER_PONG :
		case MC_PEER_OPENED	:
		case MC_MATCH_GAME_RESPONSE_TIMESYNC :
			return false;
	}
	return true;
}

void ZGame::OnCommand_Observer(MCommand* pCommand)
{
	if(!IsIgnoreObserverCommand(pCommand->GetID()))
	{
		OnCommand_Immidiate(pCommand);
		return;
	}


	ZObserverCommandItem *pZCommand=new ZObserverCommandItem;
	pZCommand->pCommand=pCommand->Clone();
	pZCommand->fTime=GetTime();
	m_ObserverCommandList.push_back(pZCommand);

	if(pCommand->GetID()==MC_PEER_BASICINFO)
	{
		/*
		ZCharacter *pChar=m_CharacterManager.Find(pCommand->GetSenderUID());
		if(pChar)
		{
			mlog("%s basic info : %3.3f \n",pChar->GetProperty()->szName,pZCommand->fTime);
		}
		*/
		OnPeerBasicInfo(pCommand,true,false);
	}
}

void ZGame::ProcessDelayedCommand()
{
	for(ZObserverCommandList::iterator i = m_DelayedCommandList.begin(); i != m_DelayedCommandList.end();i++)
	{
		ZObserverCommandItem *pItem = *i;

		// 실행할 시간이 지났으면 실행한다
		if(GetTime() > pItem->fTime) 
		{
			OnCommand_Immidiate(pItem->pCommand);
			i = m_DelayedCommandList.erase(i);
			delete pItem->pCommand;
			delete pItem;
		}
	}
}

void ZGame::OnReplayRun()
{
	if(m_ReplayCommandList.size()==0 && m_bReplaying) {
		m_bReplaying=false;
		EndReplay();
		return;
	}

//	static float fLastTime = 0;
	while(m_ReplayCommandList.size())
	{
		ZObserverCommandItem *pItem=*m_ReplayCommandList.begin();

//		_ASSERT(pItem->fTime>=fLastTime);

		if(GetTime() < pItem->fTime)
			return;

//		mlog("curtime = %d ( %3.3f ) time = %3.3f , id %d \n",timeGetTime(),GetTime(),pItem->fTime,pItem->pCommand->GetID());

		m_ReplayCommandList.erase(m_ReplayCommandList.begin());
		
		OnCommand_Observer(pItem->pCommand);

//		fLastTime = pItem->fTime;

		delete pItem->pCommand;
		delete pItem;

	}
}

void ZGame::OnObserverRun()
{
	while(m_ObserverCommandList.begin() != m_ObserverCommandList.end())
	{
		ZObserverCommandItem *pItem=*m_ObserverCommandList.begin();
		if(GetTime()-pItem->fTime < ZGetGameInterface()->GetCombatInterface()->GetObserver()->GetDelay())
			return;

		m_ObserverCommandList.erase(m_ObserverCommandList.begin());

		if(pItem->pCommand->GetID()==MC_PEER_BASICINFO)
			OnPeerBasicInfo(pItem->pCommand,false,true);
		else
			OnCommand_Immidiate(pItem->pCommand);

		delete pItem->pCommand;
		delete pItem;
	}
}

void ZGame::FlushObserverCommands()
{
	while(m_ObserverCommandList.begin() != m_ObserverCommandList.end())
	{
		ZObserverCommandItem *pItem=*m_ObserverCommandList.begin();

		m_ObserverCommandList.erase(m_ObserverCommandList.begin());

		if(pItem->pCommand->GetID()!=MC_PEER_BASICINFO)
			OnCommand_Immidiate(pItem->pCommand);

		delete pItem->pCommand;
		delete pItem;
	}
}

bool ZGame::OnCommand(MCommand* pCommand)
{
	if(m_bRecording)
	{
		ZObserverCommandItem *pItem = new ZObserverCommandItem;
		pItem->fTime = m_fTime;
		pItem->pCommand = pCommand->Clone();

		m_ReplayCommandList.push_back(pItem);

	}

	if(ZGetGameInterface()->GetCombatInterface()->GetObserverMode()){
		OnCommand_Observer(pCommand);
		return true;
	}

	if(FilterDelayedCommand(pCommand))
		return true;

	return OnCommand_Immidiate(pCommand);
}

// 유저 컬러

bool GetUserGradeIDColor(MMatchUserGradeID gid,MCOLOR& UserNameColor,char* sp_name)
{
//		 if(gid == MMUG_FREE)			{ UserNameColor = MCOLOR(200,200,200); return true; }// 무료유저
//	else if(gid == MMUG_REGULAR)		{ UserNameColor = MCOLOR(200,200,200); return true; }// 정액유저
//	else if(gid == MMUG_CRIMINAL)		{ UserNameColor = MCOLOR(200,200,200); return true; }// 전과자
//	else if(gid == MMUG_WARNING_1)		{ UserNameColor = MCOLOR(200,200,200); return true; }// 1차경고
//	else if(gid == MMUG_WARNING_2)		{ UserNameColor = MCOLOR(200,200,200); return true; }// 2차경고
//	else if(gid == MMUG_WARNING_3)		{ UserNameColor = MCOLOR(200,200,200); return true; }// 3차경고
//	else if(gid == MMUG_CHAT_LIMITED)	{ UserNameColor = MCOLOR(200,200,200); return true; }// 채팅 금지
//	else if(gid == MMUG_PENALTY)		{ UserNameColor = MCOLOR(200,200,200); return true; }// 기간 정지
//	else if(gid == MMUG_BLOCKED)		{ UserNameColor = MCOLOR(200,200,200); return true; }// 사용정지

	if(gid == MMUG_DEVELOPER) 
	{ 
		UserNameColor = MCOLOR(255,128, 64); 
		if(sp_name) { 
			strcpy(sp_name,ZMsg(MSG_WORD_DEVELOPER));
		}
		return true; 
	}
	else if(gid == MMUG_ADMIN) {
		UserNameColor = MCOLOR(255,128, 64); 
		if(sp_name) { 
			strcpy(sp_name,ZMsg(MSG_WORD_ADMIN));
		}
		return true; 
	}

	return false;
}

bool ZGame::GetUserNameColor(MUID uid,MCOLOR& UserNameColor,char* sp_name)
{
	MMatchUserGradeID gid = MMUG_FREE;

	if(m_pMyCharacter->GetUID()==uid) {

		if(ZGetMyInfo()) {

			gid = ZGetMyInfo()->GetUGradeID();
//			gid = MMUG_DEVELOPER;//test

		} else {

			mlog("ZGame::GetUserNameColor ZGetMyInfo==NULL \n");

		}
	}
	else {

		MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(uid);

		if(pPeer) {
			 gid = pPeer->CharInfo.nUGradeID;
		}
		
	}

	return GetUserGradeIDColor( gid , UserNameColor ,sp_name );
}

void ZTranslateCommand(MCommand* pCmd, string& strLog)
{
	char szBuf[256]="";

	// 시간
	unsigned long nGlobalClock = ZGetGame()->GetTickTime();
	itoa(nGlobalClock, szBuf, 10);
	strLog = szBuf;
	strLog += ": ";

	// Command
	strLog += pCmd->m_pCommandDesc->GetName();

	// PlayerName
	string strPlayerName;
	MUID uid = pCmd->GetSenderUID();
	ZCharacter* pChar = ZGetCharacterManager()->Find(uid);
	if (pChar)
		strPlayerName = pChar->GetProperty()->szName;
	else
		strPlayerName = "Unknown";

	strLog += " (";
	strLog += strPlayerName;
	strLog += ") ";

	// Params
	string strParams;
	for(int i=0; i<pCmd->GetParameterCount(); i++){
		char szParam[256]="";
		pCmd->GetParameter(i)->GetString(szParam);
		strParams += szParam;
		if (i<pCmd->GetParameterCount()-1)
			strParams += ", ";
	}
	strLog += strParams;
}

void ZLogCommand(MCommand* pCmd)
{
	if (pCmd->GetID() == MC_AGENT_TUNNELING_UDP) {
		return;
	}

	string strCmd;
	ZTranslateCommand(pCmd, strCmd);

	OutputDebugString(strCmd.c_str());
	OutputDebugString("\n");
}

bool ZGame::OnCommand_Immidiate(MCommand* pCommand)
{
	/* rpg 시대의 코드. 필요없는듯하다.
	string test;
	if(TranslateMessage(pCommand,&test))
		ZChatOutput(test.c_str());
	*/

#ifdef _DEBUG
//	ZLogCommand(pCommand);
#endif

	// 먼저 ZGameAction 에서 처리되는 커맨드면 처리한다.

	if(m_pGameAction->OnCommand(pCommand))
		return true;


	if (OnRuleCommand(pCommand)) return true;

	switch (pCommand->GetID())
	{
	case MC_MATCH_STAGE_ENTERBATTLE:
		{	
			unsigned char nParam;
			pCommand->GetParameter(&nParam,		0, MPT_UCHAR);

			MCommandParameter* pParam = pCommand->GetParameter(1);
			if(pParam->GetType()!=MPT_BLOB) break;
			void* pBlob = pParam->GetPointer();

			MTD_PeerListNode* pPeerNode = (MTD_PeerListNode*)MGetBlobArrayElement(pBlob, 0);

			OnStageEnterBattle(MCmdEnterBattleParam(nParam), pPeerNode);
		}
		break;
	case MC_MATCH_STAGE_LEAVEBATTLE:
		{
			MUID uidChar, uidStage;

			pCommand->GetParameter(&uidChar, 0, MPT_UID);
			pCommand->GetParameter(&uidStage, 1, MPT_UID);

			OnStageLeaveBattle(uidChar, uidStage);
		}
		break;
	case MC_MATCH_RESPONSE_PEERLIST:
		{
			MUID uidStage;
			pCommand->GetParameter(&uidStage, 0, MPT_UID);
			MCommandParameter* pParam = pCommand->GetParameter(1);
			if(pParam->GetType()!=MPT_BLOB) break;
			void* pBlob = pParam->GetPointer();
			int nCount = MGetBlobArrayCount(pBlob);
			OnPeerList(uidStage,pBlob, nCount);
		}
		break;
	case MC_MATCH_GAME_ROUNDSTATE:
		{
			MUID uidStage;
			int nRoundState, nRound, nArg;

			pCommand->GetParameter(&uidStage, 0, MPT_UID);
			pCommand->GetParameter(&nRound, 1, MPT_INT);
			pCommand->GetParameter(&nRoundState, 2, MPT_INT);
			pCommand->GetParameter(&nArg, 3, MPT_INT);

			OnGameRoundState(uidStage, nRound, nRoundState, nArg);

			g_pGame->GetMatch()->SetRoundStartTime();
		}
		break;
	case MC_MATCH_GAME_RESPONSE_TIMESYNC: 
		{
			unsigned int nLocalTS, nGlobalTS;
			pCommand->GetParameter(&nLocalTS, 0, MPT_UINT);
			pCommand->GetParameter(&nGlobalTS, 1, MPT_UINT);

			OnGameResponseTimeSync(nLocalTS, nGlobalTS);
		}
		break;
	case MC_MATCH_RESPONSE_SUICIDE:
		{
			int nResult;
			MUID	uidChar;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			pCommand->GetParameter(&uidChar, 1, MPT_UID);

			if (nResult == MOK)
			{
				OnPeerDie(uidChar, uidChar);
				CancelSuicide();
			}
		}
		break;
	case MC_EVENT_UPDATE_JJANG:
		{
			MUID uidChar;
			bool bJjang;

			pCommand->GetParameter(&uidChar, 0, MPT_UID);
			pCommand->GetParameter(&bJjang, 1, MPT_BOOL);

			OnEventUpdateJjang(uidChar, bJjang);
		}
		break;
	case MC_PEER_CHAT:
		{
			int nTeam = MMT_ALL;
			char szMsg[512];
			memset(szMsg, 0, sizeof(szMsg));

			pCommand->GetParameter(&nTeam, 0, MPT_INT);
			pCommand->GetParameter(szMsg, 1, MPT_STR, sizeof(szMsg) );

			MCOLOR ChatColor = MCOLOR(0xFFD0D0D0);
			const MCOLOR TeamChatColor = MCOLOR(109,207,246);

			MUID uid=pCommand->GetSenderUID();
			ZCharacter *pChar=ZGetCharacterManager()->Find(uid);

			MCOLOR UserNameColor = MCOLOR(190,190,0);

			char sp_name[256];
			bool bSpUser = GetUserNameColor(uid,UserNameColor,sp_name);
/*
			bool GetPureUserName(char* name,char* pure_name)
			{
				//^숫자제거

			}
*/
			if(pChar) 
			{
				int nMyTeam = ZApplication::GetGame()->m_pMyCharacter->GetTeamID();

				// 일반 채팅 말 일때...
				if ( (nTeam == MMT_ALL) || (nTeam == MMT_SPECTATOR))
				{
					if ( !ZGetGameClient()->GetRejectNormalChat() || ( strcmp( pChar->GetUserName(), ZGetMyInfo()->GetCharName()) == 0))
					{
						ZGetSoundEngine()->PlaySound("if_error");
						char szTemp[sizeof(szMsg)+64];

						if ( ZApplication::GetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)	// 듀얼 매치일때
						{
							if ( !m_pMyCharacter->IsDie() && pChar->IsDie())
							{
								ChatColor = MCOLOR(0xFF808080);
								strcpy( szMsg, "^0. . . . .");
							}
						}


						if(bSpUser) {
							sprintf(szTemp, "%s : %s", sp_name,szMsg);
							ZChatOutput(UserNameColor, szTemp);
						}
						else {
							sprintf(szTemp, "%s : %s", pChar->GetProperty()->szName,szMsg);
							ZChatOutput(ChatColor, szTemp);
						}
					}
				}

				// 팀 채팅 말 일때...
				else if (nTeam == nMyTeam)
				{
					if ( (!ZGetGameClient()->IsLadderGame() && !ZGetGameClient()->GetRejectTeamChat()) ||
						 ( ZGetGameClient()->IsLadderGame() && !ZGetGameClient()->GetRejectClanChat()) ||
						 ( strcmp( pChar->GetUserName(), ZGetMyInfo()->GetCharName()) == 0))
					{
						ZGetSoundEngine()->PlaySound("if_error");
						char szTemp[512];

						if(bSpUser) {
							sprintf(szTemp, "(Team)%s : %s", sp_name,szMsg);
							ZChatOutput(UserNameColor, szTemp);
						}
						else {
							sprintf(szTemp, "(Team)%s : %s", pChar->GetProperty()->szName,szMsg);
							ZChatOutput(TeamChatColor, szTemp);
						}
					}
				}
			}
		}
		break;

	case MC_PEER_CHAT_ICON:
		{
			bool bShow = false;
			pCommand->GetParameter(&bShow, 0, MPT_BOOL);

			MUID uid=pCommand->GetSenderUID();
			ZCharacter *pChar=ZGetCharacterManager()->Find(uid);
			if(pChar)
			{
				if(bShow)
				{
					if(!pChar->m_bChatEffect)
					{
						pChar->m_bChatEffect=true;
						ZGetEffectManager()->AddChatIcon(pChar);
					}
				}else
					pChar->m_bChatEffect=false;
			}
		}break;

		/*
		case MC_PEER_MOVE:
		{

		rvector pos, dir, velocity;
		pCommand->GetParameter(&pos, 0, MPT_POS);
		pCommand->GetParameter(&dir, 1, MPT_VECTOR);
		pCommand->GetParameter(&velocity, 2, MPT_VECTOR);
		int upper, lower;
		pCommand->GetParameter(&upper, 3, MPT_INT);
		pCommand->GetParameter(&lower, 4, MPT_INT);

		OnPeerMove(pCommand->GetSenderUID(), pos, dir, velocity, ZC_STATE_UPPER(upper), ZC_STATE_LOWER(lower));
		}
		break;
		*/
	case MC_MATCH_OBTAIN_WORLDITEM:
		{
			if (!IsReplay()) break;

			MUID uidPlayer;
			int nItemUID;

			pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
			pCommand->GetParameter(&nItemUID, 1, MPT_INT);

			ZGetGameClient()->OnObtainWorldItem(uidPlayer, nItemUID);
		}
		break;
	case MC_MATCH_SPAWN_WORLDITEM:
		{
			if (!IsReplay()) break;

			MCommandParameter* pParam = pCommand->GetParameter(0);
			if (pParam->GetType()!=MPT_BLOB) break;

			void* pSpawnInfoBlob = pParam->GetPointer();

			ZGetGameClient()->OnSpawnWorldItem(pSpawnInfoBlob);
		}
		break;
	case MC_MATCH_REMOVE_WORLDITEM:
		{
			if (!IsReplay()) break;

			int nItemUID;

			pCommand->GetParameter(&nItemUID, 0, MPT_INT);

			ZGetGameClient()->OnRemoveWorldItem(nItemUID);
		}
		break;




	case MC_PEER_BASICINFO	: OnPeerBasicInfo(pCommand);break;
	case MC_PEER_HPINFO		: OnPeerHPInfo(pCommand);break;
	case MC_PEER_HPAPINFO		: OnPeerHPAPInfo(pCommand);break;
	case MC_PEER_PING		: OnPeerPing(pCommand); break;
	case MC_PEER_PONG		: OnPeerPong(pCommand); break;
	case MC_PEER_OPENED		: OnPeerOpened(pCommand); break;
	case MC_PEER_DASH	: OnPeerDash(pCommand); break;
	case MC_PEER_SHOT:
		{
			MCommandParameter* pParam = pCommand->GetParameter(0);
			if(pParam->GetType()!=MPT_BLOB) break;	// 문제가 있다

			ZPACKEDSHOTINFO *pinfo =(ZPACKEDSHOTINFO*)pParam->GetPointer();

			rvector pos = rvector(pinfo->posx,pinfo->posy,pinfo->posz);
			rvector to = rvector(pinfo->tox,pinfo->toy,pinfo->toz);

			// TODO : 시간판정을 각각해야한다
			OnPeerShot(pCommand->GetSenderUID(), pinfo->fTime, pos, to, (MMatchCharItemParts)pinfo->sel_type);
		}
		break;
	case MC_PEER_SHOT_MELEE:
		{
			float fShotTime;
			rvector pos, dir;

			pCommand->GetParameter(&fShotTime, 0, MPT_FLOAT);
			pCommand->GetParameter(&pos, 1, MPT_POS);

			OnPeerShot(pCommand->GetSenderUID(), fShotTime, pos, pos, MMCIP_MELEE);
		}
		break;

	case MC_PEER_SHOT_SP:
		{
			float fShotTime;
			rvector pos, dir;
			int sel_type,type;

			pCommand->GetParameter(&fShotTime, 0, MPT_FLOAT);
			pCommand->GetParameter(&pos, 1, MPT_POS);
			pCommand->GetParameter(&dir, 2, MPT_VECTOR);
			pCommand->GetParameter(&type, 3, MPT_INT);
			pCommand->GetParameter(&sel_type, 4, MPT_INT);

			// fShotTime 은 무시하고..
			//fShotTime=GetTime()-(float)GetPing(pCommand->GetSenderUID())*0.001f;

			OnPeerShotSp(pCommand->GetSenderUID(), fShotTime, pos, dir,type,(MMatchCharItemParts)sel_type);
		}
		break;

	case MC_PEER_RELOAD:
		{
			OnPeerReload(pCommand->GetSenderUID());
		}
		break;
	case MC_PEER_CHANGECHARACTER:
		{
			OnPeerChangeCharacter(pCommand->GetSenderUID());
		}
		break;

	case MC_PEER_DIE:
		{
			MUID	uid;
			pCommand->GetParameter(&uid , 0, MPT_UID);

			OnPeerDie(pCommand->GetSenderUID(), uid);

		}
		break;
	case MC_MATCH_GAME_DEAD:
		{
			MUID uidAttacker, uidVictim;
			unsigned long int nAttackerArg, nVictimArg;

			pCommand->GetParameter(&uidAttacker, 0, MPT_UID);
			pCommand->GetParameter(&nAttackerArg, 1, MPT_UINT);
			pCommand->GetParameter(&uidVictim, 2, MPT_UID);
			pCommand->GetParameter(&nVictimArg, 3, MPT_UINT);

			OnPeerDead(uidAttacker, nAttackerArg, uidVictim, nVictimArg);
		}
		break;
	case MC_MATCH_GAME_TEAMBONUS:
		{
			MUID uidChar;
			unsigned long int nExpArg;

			pCommand->GetParameter(&uidChar, 0, MPT_UID);
			pCommand->GetParameter(&nExpArg, 1, MPT_UINT);

			OnReceiveTeamBonus(uidChar, nExpArg);
		}
		break;
/*
	case MC_MATCH_ASSIGN_COMMANDER:
		{
			MUID uidRedCommander, uidBlueCommander;

			pCommand->GetParameter(&uidRedCommander, 0, MPT_UID);
			pCommand->GetParameter(&uidBlueCommander, 1, MPT_UID);

			OnAssignCommander(uidRedCommander, uidBlueCommander);
		}
		break;
*/
	case MC_PEER_SPAWN:
		{
			rvector pos, dir;
			pCommand->GetParameter(&pos, 0, MPT_POS);
			pCommand->GetParameter(&dir, 1, MPT_DIR);

			OnPeerSpawn(pCommand->GetSenderUID(), pos, dir);
		}
		break;
	case MC_MATCH_GAME_RESPONSE_SPAWN:
		{
			MUID uidChar;
			MShortVector s_pos, s_dir;

			pCommand->GetParameter(&uidChar, 0, MPT_UID);
			pCommand->GetParameter(&s_pos, 1, MPT_SVECTOR);
			pCommand->GetParameter(&s_dir, 2, MPT_SVECTOR);

			rvector pos, dir;
			pos = rvector((float)s_pos.x, (float)s_pos.y, (float)s_pos.z);
			dir = rvector(ShortToDirElement(s_dir.x), ShortToDirElement(s_dir.y), ShortToDirElement(s_dir.z));

			OnPeerSpawn(uidChar, pos, dir);
		}
		break;
	case MC_MATCH_SET_OBSERVER:
		{
			MUID uidChar;

			pCommand->GetParameter(&uidChar, 0, MPT_UID);

			OnSetObserver(uidChar);
		}
		break;
	case MC_PEER_CHANGE_WEAPON:
		{
			int nWeaponID;

			pCommand->GetParameter(&nWeaponID, 0, MPT_INT);

			OnChangeWeapon(pCommand->GetSenderUID(),MMatchCharItemParts(nWeaponID));
		}

		break;

	case MC_PEER_SPMOTION:
		{
			int nMotionType;

			pCommand->GetParameter(&nMotionType, 0, MPT_INT);

			OnPeerSpMotion(pCommand->GetSenderUID(),nMotionType);
		}
		break;

	case MC_PEER_CHANGE_PARTS:
		{
			int PartsType;
			int PartsID;

			pCommand->GetParameter(&PartsType, 0, MPT_INT);
			pCommand->GetParameter(&PartsID, 1, MPT_INT);

			OnChangeParts(pCommand->GetSenderUID(),PartsType,PartsID);
		}
		break;

	case MC_PEER_ATTACK:
		{
			int		type;
			rvector pos;

			pCommand->GetParameter(&type, 0, MPT_INT);
			pCommand->GetParameter(&pos , 1, MPT_POS);

			OnAttack( pCommand->GetSenderUID(), type, pos);
		}
		break;

	case MC_PEER_DAMAGE:
		{
			MUID	tuid;
			int		damage;

			pCommand->GetParameter(&tuid   , 0, MPT_UID);
			pCommand->GetParameter(&damage , 1, MPT_INT);

			OnDamage( pCommand->GetSenderUID(), tuid, damage);
		}
		break;
	case MC_MATCH_RESET_TEAM_MEMBERS:
		{
			OnResetTeamMembers(pCommand);
		}
		break;

	case MC_REQUEST_XTRAP_HASHVALUE :
		{
			char szNewRandomValue[ 256 ] = {0,};

			pCommand->GetParameter( szNewRandomValue, 0, MPT_STR, 255 );

			ZApplication::GetGameInterface()->OnRequestNewHashValue( szNewRandomValue );
		}
		break;

	case MC_MATCH_DISCONNMSG :
		{
			DWORD dwMsgID;
			pCommand->GetParameter( &dwMsgID, 0, MPT_UINT );

			ZApplication::GetGameInterface()->OnDisconnectMsg( dwMsgID );
		}
		break;

		/*
	case MC_PEER_SKILL:
		{
			float fTime;
			int nSkill,sel_type;

			pCommand->GetParameter(&fTime, 0, MPT_FLOAT);
			pCommand->GetParameter(&nSkill, 1, MPT_INT);
			pCommand->GetParameter(&sel_type, 2, MPT_INT);

			OnPeerSkill( pCommand->GetSenderUID(), fTime, nSkill, sel_type );
		}
		break;
		*/

	case ZC_TEST_INFO:
		{
			OutputToConsole("Sync : %u", ZGetGameClient()->GetGlobalClockCount());

			rvector v;
			//				int iQueueCount;
			v = m_pMyCharacter->m_Position;
			//				iQueueCount = (int)m_pMyCharacter->m_PathQueue.size();
			OutputToConsole("My Pos = %.2f %.2f %.2f", v.x, v.y, v.z);

			/*
			for (ZCharacterItor itor = m_OtherCharacters.begin(); itor != m_OtherCharacters.end(); )
			{
			ZCharacter* pCharacter = (*itor).second;
			v = pCharacter->m_Position;
			iQueueCount = (int)pCharacter->m_PathQueue.size();
			OutputToConsole("Other Pos(%d) = %.2f %.2f %.2f", iQueueCount, v.x, v.y, v.z);
			++itor;
			}
			*/
		}
		break;
	case ZC_BEGIN_PROFILE:	
		g_bProfile=true;	
		break;
	case ZC_END_PROFILE:	
		g_bProfile=false;	
		break;
	case ZC_EVENT_OPTAIN_SPECIAL_WORLDITEM:
		{
			OnLocalOptainSpecialWorldItem(pCommand);
		}
		break;
	}

	return true;
}

rvector ZGame::GetMyCharacterFirePosition(void)
{
	rvector p = g_pGame->m_pMyCharacter->GetPosition();
	p.z += 160.f;
	return p;
}

// 옵저버 때에는 이 펑션의 역할이 분리된다 
// 즉, 미리 history에 더해지고 적절한 타이밍에 실행된다.
void ZGame::OnPeerBasicInfo(MCommand *pCommand,bool bAddHistory,bool bUpdate)
{
	MCommandParameter* pParam = pCommand->GetParameter(0);
	if(pParam->GetType()!=MPT_BLOB) return;

	ZPACKEDBASICINFO* ppbi= (ZPACKEDBASICINFO*)pParam->GetPointer();
	
	ZBasicInfo bi;
	bi.position = rvector(Roundf(ppbi->posx),Roundf(ppbi->posy),Roundf(ppbi->posz));
	bi.velocity = rvector(ppbi->velx,ppbi->vely,ppbi->velz);
	bi.direction = 1.f/32000.f * rvector(ppbi->dirx,ppbi->diry,ppbi->dirz);
	
	MUID uid = pCommand->GetSenderUID();

	MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(uid);
	if (pPeer) {
		if (pPeer->IsOpened() == false) {
			MCommand* pCmd = ZGetGameClient()->CreateCommand(MC_PEER_OPENED, ZGetGameClient()->GetPlayerUID());
			pCmd->AddParameter(new MCmdParamUID(pPeer->uidChar));
			ZGetGameClient()->Post(pCmd);

			pPeer->SetOpened(true);
		}
	}

	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
	if (!pCharacter) return;

	// 캐릭터의 현재시간을 업데이트한다
	// 캐릭터의 현재시간 추정치
	float fCurrentLocalTime = pCharacter->m_fTimeOffset + GetTime() ;

	// 캐릭터가 보내온 시간이 내가 추정한 시간과 3초 이상 차이가 나면 내가 알고있는 시간을 고친다.
	float fTimeError = ppbi->fTime - fCurrentLocalTime;
	if(fabs(fTimeError)>3.f) {
		pCharacter->m_fTimeOffset = ppbi->fTime - GetTime();
		pCharacter->m_fAccumulatedTimeError = 0;
		pCharacter->m_nTimeErrorCount = 0;
	}else
	{
		// 차이가 3초 이내이고 일정시간 합했다가 조금(차이의 반)씩 조절한다
		pCharacter->m_fAccumulatedTimeError += fTimeError;
		pCharacter->m_nTimeErrorCount ++;
		if(pCharacter->m_nTimeErrorCount > 10) {
			pCharacter->m_fTimeOffset += .5f*pCharacter->m_fAccumulatedTimeError/10.f;
			pCharacter->m_fAccumulatedTimeError = 0;
			pCharacter->m_nTimeErrorCount = 0;
		}
	}

	// 현재시간을 마지막 데이터 받은시간으로 기록.
	pCharacter->m_fLastReceivedTime = GetTime();


	// 나중에 판정을 위해 histroy 에 보관한다.
	if(bAddHistory)
	{
		ZBasicInfoItem *pitem=new ZBasicInfoItem;
		CopyMemory(&pitem->info, &bi, sizeof(ZBasicInfo));

		pitem->fReceivedTime=GetTime();

		pitem->fSendTime= ppbi->fTime - pCharacter->m_fTimeOffset;	// 내 기준으로 변환
		
		pCharacter->m_BasicHistory.push_back(pitem);

		while(pCharacter->m_BasicHistory.size()>CHARACTER_HISTROY_COUNT)
		{
			delete *pCharacter->m_BasicHistory.begin();
			pCharacter->m_BasicHistory.erase(pCharacter->m_BasicHistory.begin());
		}
	}

	if(bUpdate)
	{
		// 리플레이때를 제외하고 내 캐릭터는 모션이나 속도등등을 업데이트 할 필요가 없다.
		if(!IsReplay() && pCharacter->IsHero()) return;

		/*
		// 보낸이와의 network delay 시간을 계산한다.
		float fDelay=(GetTime()-pci->fSendTime)*.5f;
		pCharacter->m_PingData.push_back(fDelay);
		while(pCharacter->m_PingData.size()>10)
			pCharacter->m_PingData.erase(pCharacter->m_PingData.begin());

		float fDelaySum=0;
		for(list<float>::iterator i=pCharacter->m_PingData.begin();i!=pCharacter->m_PingData.end();i++)
		{
			fDelaySum+=*i;
		}

		pCharacter->m_fAveragePingTime=fDelaySum/(float)pCharacter->m_PingData.size();
		*/

		// 움직임및 애니메이션등의 처리

		// 데미지 판정때 과거의 위치로 판정하기때문에 현재의 위치를 예측할 필요가 없다.
		// 따라서 아래의 예측대신 단순한코드로 대체..
/*
		rvector nextPos;
		pCharacter->SetPeerPosition(pci->position);
		pCharacter->SetVelocity(pci->velocity);
		pCharacter->SetAccel(pci->accel);
		pCharacter->m_TargetDir = pci->direction;
		pCharacter->m_fLastValidTime = GetTime();
*/

		((ZNetCharacter*)(pCharacter))->SetNetPosition(bi.position, bi.velocity, bi.direction);

		pCharacter->SetAnimationLower((ZC_STATE_LOWER)ppbi->lowerstate);
		pCharacter->SetAnimationUpper((ZC_STATE_UPPER)ppbi->upperstate);

		// 들고있는 무기가 다르면 바꿔준다
		if(pCharacter->GetItems()->GetSelectedWeaponParts()!=ppbi->selweapon) {
			pCharacter->ChangeWeapon((MMatchCharItemParts)ppbi->selweapon);
		}
	}
}

void ZGame::OnPeerHPInfo(MCommand *pCommand)
{
	MUID uid = pCommand->GetSenderUID();
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
	if (!pCharacter) return;

	float fHP=0.0f;
	pCommand->GetParameter(&fHP, 0, MPT_FLOAT);

	// 옵저브 하고 있을때는 보여주기 위해 hp 정보를 갱신한다.
	if(ZGetGameInterface()->GetCombatInterface()->GetObserverMode()) {
		pCharacter->SetHP(fHP);
	}
}


void ZGame::OnPeerHPAPInfo(MCommand *pCommand)
{
	MUID uid = pCommand->GetSenderUID();
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
	if (!pCharacter) return;

	float fHP=0.0f;
	pCommand->GetParameter(&fHP, 0, MPT_FLOAT);
	float fAP=0.0f;
	pCommand->GetParameter(&fAP, 1, MPT_FLOAT);

	// 옵저브 하고 있을때는 보여주기 위해 hp 정보를 갱신한다.
	if(ZGetGameInterface()->GetCombatInterface()->GetObserverMode()) {
		pCharacter->SetHP(fHP);
		pCharacter->SetAP(fAP);
	}
}

#ifdef _DEBUG
	static int g_nPingCount=0;
	static int g_nPongCount=0;
#endif

void ZGame::OnPeerPing(MCommand *pCommand)
{
	if(m_bReplaying) return;

	unsigned int nTimeStamp;
	pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT);
	
	// PONG 으로 응답한다
	MCommandManager* MCmdMgr = ZGetGameClient()->GetCommandManager();
	MCommand* pCmd = new MCommand(MCmdMgr->GetCommandDescByID(MC_PEER_PONG), 
								  pCommand->GetSenderUID(), ZGetGameClient()->GetUID());	
	pCmd->AddParameter(new MCmdParamUInt(nTimeStamp));
	ZGetGameClient()->Post(pCmd);
}

void ZGame::OnPeerPong(MCommand *pCommand)
{
	MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(pCommand->GetSenderUID());
	if (pPeer == NULL)
		return;

	unsigned int nTimeStamp;
	pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT);

	int nPing = (GetTickTime() - nTimeStamp)/2;
    pPeer->UpdatePing(GetTickTime(), nPing);

/*
	if (pPeer->IsOpened() == false) {
		MCommand* pCmd = ZGetGameClient()->CreateCommand(MC_PEER_OPENED, ZGetGameClient()->GetPlayerUID());
		pCmd->AddParameter(new MCmdParamUID(pPeer->uidChar));
		ZGetGameClient()->Post(pCmd);

		pPeer->SetOpened(true);
	}
*/
	#ifdef _DEBUG
		g_nPongCount++;
	#endif
}

void ZGame::OnPeerOpened(MCommand *pCommand)
{
	MUID uidChar;
	pCommand->GetParameter(&uidChar, 0, MPT_UID);

	//// Show Character ////////////////////////////////////////
	ZCharacter* pCharacter = m_CharacterManager.Find(uidChar);
	if (pCharacter && pCharacter->IsDie()==false) {
		pCharacter->SetVisible(true);

		// 신입 캐릭터에게 자신의 무기를 알린다...
		ZCharacter* pMyCharacter = g_pGame->m_pMyCharacter;
		if(pMyCharacter)
		{
			int nParts = g_pGame->m_pMyCharacter->GetItems()->GetSelectedWeaponParts();
			g_pGame->m_pMyCharacter->m_bSpMotion = false;
			ZPostChangeWeapon(nParts);
		}
	}

#ifdef _DEBUG
	//// PeerOpened Debug log //////////////////////////////////
	char* pszName = "Unknown";
	char* pszNAT = "NoNAT";
	MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(uidChar);
	if (pPeer) {
		pszName = pPeer->CharInfo.szName;
		if (pPeer->GetUDPTestResult() == false) pszNAT = "NAT";
	}

	char szBuf[64];
	sprintf(szBuf, "PEER_OPENED(%s) : %s(%d%d) \n", pszNAT, pszName, uidChar.High, uidChar.Low);
	OutputDebugString(szBuf);
#endif
}

void ZGame::OnChangeWeapon(MUID& uid, MMatchCharItemParts parts)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
//	if (uid == ZGetGameClient()->GetUID()) pCharacter = m_pMyCharacter;
//	_ASSERT(pCharacter != NULL);

	if (pCharacter && pCharacter!=m_pMyCharacter)		// 내 캐릭터는 이미 바꿨다.
	{
		pCharacter->ChangeWeapon(parts);
	}
}

void ZGame::OnChangeParts(MUID& uid,int partstype,int PartsID)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
//	if (uid == ZGetGameClient()->GetUID()) pCharacter = m_pMyCharacter;

	if ( pCharacter ) {
		pCharacter->OnChangeParts( (RMeshPartsType)partstype , PartsID );
	}
}

void ZGame::OnAttack(MUID& uid,int type,rvector& pos)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
//	if (uid == ZGetGameClient()->GetUID()) pCharacter = m_pMyCharacter;

	// 우선 모션만..
	if ( pCharacter ) {

	}
}

void ZGame::OnDamage(MUID& uid,MUID& tuid,int damage)
{
/*
	ZCharacter* pSender = NULL;		
	ZCharacter* pTarget = NULL;		

	pSender = m_CharacterManager.Find(uid);
	pTarget = m_CharacterManager.Find(tuid);

	pTarget->OnSimpleDamaged(NULL,damage,0.5f);
*/
}

void ZGame::OnPeerShotSp(MUID& uid, float fShotTime, rvector& pos, rvector& dir,int type,MMatchCharItemParts sel_type)
{
	ZCharacter* pOwnerCharacter = NULL;		// 총 쏜 사람

	pOwnerCharacter = m_CharacterManager.Find(uid);
//	if (uid == ZGetGameClient()->GetUID()) pOwnerCharacter = m_pMyCharacter;

//	_ASSERT(pOwnerCharacter != NULL);
	if (pOwnerCharacter == NULL) return;
	if(!pOwnerCharacter->GetInitialized()) return;
	if(!pOwnerCharacter->IsVisible()) return;

	////////////////////////////////////////////////////////////////////////////////
	MMatchCharItemParts parts = (MMatchCharItemParts)sel_type;

	if( parts != pOwnerCharacter->GetItems()->GetSelectedWeaponParts()) {
		// 지금 들고 있는 무기와 보내진 무기가 틀리다면 보내진 무기로 바꿔준다..
		OnChangeWeapon(uid,parts);
	}
	/////////////////////////////////////////////////////////////////////////////////

	ZItem *pItem = pOwnerCharacter->GetItems()->GetItem(sel_type);
	if(!pItem) return;

	// 비정상적인 발사속도를 무시한다.
	if (pOwnerCharacter->CheckValidShotTime(pItem->GetDescID(), fShotTime, pItem)) {
		pOwnerCharacter->UpdateValidShotTime(pItem->GetDescID(), fShotTime);
	} else {
		return;
	}

	if (uid == ZGetMyUID()) {
		//// 루프중 MEMORYHACK있었나 검사
//		ZItem* pItem = pOwnerCharacter->GetItems()->GetSelectedWeapon();	// Check Target
		MDataChecker* pChecker = ZApplication::GetGame()->GetDataChecker();
		MDataCheckNode* pCheckNode = pChecker->FindCheck((BYTE*)pItem->GetAMagazinePointer());
		if (pCheckNode && (pCheckNode->UpdateChecksum()==false)) {
			pChecker->BringError();	//// MEMORYHACK 감지. Checksum 모두중단하고 끝장낸다.
		} else {
			int nAMagazine = pItem->GetBulletAMagazine();

			if (!pItem->Shot()) return;	//// SHOT ////

			//// MEMORYHACK 없었으면 리뉴한다.
			if (pItem->GetBulletAMagazine() < nAMagazine)	// Shot에서 총알 줄어야만 정상이다
				pChecker->RenewCheck((BYTE*)pItem->GetAMagazinePointer(), sizeof(int));
			else
				if(sel_type != MMCIP_MELEE)
					pChecker->BringError();
		}
	} else {
		if (!pItem->Shot()) return;	//// SHOT ////
	}


	rvector velocity;
	rvector _pos;

	bool dLight = true;
	bool bSpend = false;	// 사용하면 없어지는 아이템이다

//	ZGetEffectManager()->AddHealEffect(pOwnerCharacter->GetPosition()); // test
//	ZGetEffectManager()->AddRepireEffect(pOwnerCharacter->GetPosition());

	switch(type)
	{
	case ZC_WEAPON_SP_GRENADE : {
		//static RealSoundEffectSource* pSES	= ZGetSoundEngine()->GetSES("we_grenade_fire");
		//if( pSES != NULL )
		//{
		//	ZGetSoundEngine()->PlaySE( pSES, pos.x, pos.y, pos.z, pOwnerCharacter == m_pMyCharacter );
		//}
		bSpend = true;

		velocity	= pOwnerCharacter->GetVelocity()+pOwnerCharacter->m_TargetDir*1200.f;
		velocity.z	+= 300.f;
		m_WeaponManager.AddGrenade(pos,velocity,pOwnerCharacter);
		//m_WeaponManager.AddFlashBang( pos - rvector(10,10,10), velocity, pOwnerCharacter );
		//m_WeaponManager.AddSmokeGrenade( pos + rvector(10,10,10), velocity, pOwnerCharacter );
		}break;

	case ZC_WEAPON_SP_ROCKET : {
		//static RealSoundEffectSource* pSES = ZApplication::GetSoundEngine()->GetSES("rocket_fire");
		//static RealSoundEffectSource* pSES = ZApplication::GetSoundEngine()->GetSES("we_rocket_fire");
		//if(pSES!=NULL) ZApplication::GetSoundEngine()->PlaySE(pSES, pos.x, pos.y, pos.z ,pOwnerCharacter==m_pMyCharacter);

			m_WeaponManager.AddRocket(pos,dir,pOwnerCharacter);
//			m_WeaponManager.AddFireBall(pos,dir,pOwnerCharacter);
//			m_WeaponManager.AddIceMissile(pos,dir,pOwnerCharacter);
//			m_WeaponManager.AddMagicMissile(pos,dir,pOwnerCharacter);

			//if (pOwnerCharacter->m_UID == g_pGame->m_pMyCharacter->m_UID) {
			//	ZItem* pWeapon = pOwnerCharacter->GetItems()->GetSelectedWeapon();
			//	if ( (pWeapon->GetBulletAMagazine() <= 0) && (pWeapon->GetBullet()>0) ) {
			//		ZPostReload();
			//	}
			//}
			if(Z_VIDEO_DYNAMICLIGHT)
				ZGetStencilLight()->AddLightSource( pos, 2.0f, 100 );
		}break;

	case  ZC_WEAPON_SP_FLASHBANG:
		bSpend = true; 

		velocity	= pOwnerCharacter->GetVelocity() + pOwnerCharacter->m_TargetDir*1200.f;
		velocity.z	+= 300.0f;
		m_WeaponManager.AddFlashBang(pos,velocity,pOwnerCharacter);
		dLight	= false;
		break;

	case  ZC_WEAPON_SP_SMOKE:
		bSpend = true;

		velocity	= pOwnerCharacter->GetVelocity() + pOwnerCharacter->m_TargetDir*1200.f;
		velocity.z	+= 300.0f;
		m_WeaponManager.AddSmokeGrenade(pos,velocity,pOwnerCharacter);
		dLight	= false;
		break;

	case  ZC_WEAPON_SP_TEAR_GAS:
		bSpend = true;

		dLight	= false;
		break;

	case  ZC_WEAPON_SP_ITEMKIT: 
		{
			int nLinkedWorldItem = ZGetWorldItemManager()->GetLinkedWorldItemID(pItem->GetDesc());

			velocity	= dir;
			_pos = pos;

			m_WeaponManager.AddKit(_pos,velocity,pOwnerCharacter,0.2f,pItem->GetDesc()->m_szMeshName,nLinkedWorldItem);
			dLight	= false;
		}
		break;
	}

	ZApplication::GetSoundEngine()->PlaySEFire(pItem->GetDesc(), pos.x, pos.y, pos.z, (pOwnerCharacter==m_pMyCharacter));
	
	if( dLight )
	{
		// 총 쏠때 라이트 추가
		ZCharacter* pChar;

		if( ZGetConfiguration()->GetVideo()->bDynamicLight && pOwnerCharacter != NULL )	{

			pChar = pOwnerCharacter;

			if( pChar->m_bDynamicLight ) {

				pChar->m_vLightColor = g_CharLightList[CANNON].vLightColor;
				pChar->m_fLightLife = g_CharLightList[CANNON].fLife;

			} else {

				pChar->m_bDynamicLight = true;
				pChar->m_vLightColor = g_CharLightList[CANNON].vLightColor;
				pChar->m_vLightColor.x = 1.0f;
				pChar->m_iDLightType = CANNON;
				pChar->m_fLightLife = g_CharLightList[CANNON].fLife;
			}

			if( pOwnerCharacter->IsHero() )
			{
				RGetDynamicLightManager()->AddLight( GUNFIRE, pos );
			}
		}
	}
}

bool ZGame::CheckWall(ZObject* pObj1,ZObject* pObj2)
{
	if( (pObj1==NULL) || (pObj2==NULL) )
		return false;

	if( (pObj1->GetVisualMesh()==NULL) || (pObj2->GetVisualMesh()==NULL) )
		return false;

	// 에니메이션 때문에 벽을 뚫고 들어가는 경우도 있어서..
	rvector p1 = pObj1->GetPosition() + rvector(0.f,0.f,100.f);
	rvector p2 = pObj2->GetPosition() + rvector(0.f,0.f,100.f);

	rvector dir = p2 - p1;

	Normalize(dir);

	ZPICKINFO pickinfo;

	if( Pick( pObj1 , p1 , dir, &pickinfo ) ) {//벽이라면
		if(pickinfo.bBspPicked)//맵이 걸린경우
			return true;
	}

	return false;
}

void ZGame::OnExplosionGrenade(MUID uidOwner,rvector pos,float fDamage,float fRange,float fMinDamage,float fKnockBack,MMatchTeam nTeamID)
{
	ZObject* pTarget = NULL;

	float fDist,fDamageRange;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor) {

		pTarget = (*itor).second;

		if(pTarget && !pTarget->IsDie()) {

			fDist = Magnitude(pos-(pTarget->GetPosition()+rvector(0,0,80)));
			if(fDist < fRange) {

				// 두 캐릭터사이에 장애물이 없어야한다~ 
//				if( CheckWall(pos,pTarget) == false )
				{

					rvector dir=pos-(pTarget->GetPosition()+rvector(0,0,80));
					Normalize(dir);

					// 몸에 직접 맞았다.
					if(GetDistance(pos,pTarget->GetPosition()+rvector(0,0,50),pTarget->GetPosition()+rvector(0,0,130))<50)
					{
						fDamageRange = 1.f;
					}else
					{
#define MAX_DMG_RANGE	50.f	// 반경이만큼 까지는 최대 데미지를 다 먹는다
//#define MIN_DMG			0.4f	// 최소 기본 데미지는 이정도.

						fDamageRange = 1.f - (1.f-fMinDamage)*( max(fDist-MAX_DMG_RANGE,0) / (fRange-MAX_DMG_RANGE));
					}

					// 수류탄을 맞으면 반동으로 튀어나간다.

					ZActor* pATarget = MDynamicCast(ZActor,pTarget);

					bool bPushSkip = false;

					if(pATarget) {
						bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
					}

					if(bPushSkip==false) {
						pTarget->AddVelocity(fKnockBack*7.f*(fRange-fDist)*-dir);
					}
					else {
						ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
					}

					ZCharacter* pOwnerCharacter = g_pGame->m_CharacterManager.Find( uidOwner );
					if(pOwnerCharacter) {
						CheckCombo(pOwnerCharacter, pTarget,!bPushSkip);
						CheckStylishAction(pOwnerCharacter);
					}


					float fActualDamage = fDamage * fDamageRange;
					float fRatio = ZItem::GetPiercingRatio( MWT_FRAGMENTATION , eq_parts_chest );//수류탄과 로켓 구분없다..
					pTarget->OnDamaged(pOwnerCharacter,pos,ZD_EXPLOSION,MWT_FRAGMENTATION,fActualDamage,fRatio);
//					pTarget->OnDamagedGrenade( uidOwner, dir, fDamage * fDamageRange, nTeamID);
				}
			}
		}
	}

#define SHOCK_RANGE		1500.f			// 10미터까지 흔들린다

	ZCharacter *pTargetCharacter=ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	float fPower= (SHOCK_RANGE-Magnitude(pTargetCharacter->GetPosition()+rvector(0,0,50) - pos))/SHOCK_RANGE;

	if(fPower>0)
		ZGetGameInterface()->GetCamera()->Shock(fPower*500.f, .5f, rvector(0.0f, 0.0f, -1.0f));

	GetWorld()->GetWaters()->CheckSpearing( pos, pos + rvector(0,0,MAX_WATER_DEEP), 500, 0.8f );

//	static RealSoundEffectSource* pSES= ZApplication::GetSoundEngine()->GetSES("explosion");
//	ZApplication::GetSoundEngine()->PlaySE(pSES,pos.x,pos.y,pos.z);
}

// 매직류의 무기의 데미지를 준다
void ZGame::OnExplosionMagic(ZWeaponMagic *pWeapon, MUID uidOwner,rvector pos,float fMinDamage,float fKnockBack,MMatchTeam nTeamID,bool bSkipNpc)
{
	ZObject* pTarget = NULL;

	float fRange = 100.f * pWeapon->GetDesc()->fEffectArea;
	float fDist,fDamageRange;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor) {

		pTarget = (*itor).second;

		// 범위공격이 아니라면 타겟만 검사하면된다.
		if(pTarget->IsNPC()) continue;

		if(!pWeapon->GetDesc()->IsAreaTarget() && pWeapon->GetTarget()!=pTarget->GetUID()) continue;

		if(pTarget && !pTarget->IsDie()) {

			// 두 캐릭터사이에 장애물이 없어야한다~ 
//				if( CheckWall(pos,pTarget) == false )
			{
				fDist = Magnitude(pos-(pTarget->GetPosition()+rvector(0,0,80)));
				if(pWeapon->GetDesc()->IsAreaTarget())	// 범위공격이면 거리에 따른 데미지를 계산한다
				{
					if(fDist > fRange) continue;	// 범위를 벗어났다

					// 몸에 직접 맞았다.
					if(GetDistance(pos,pTarget->GetPosition()+rvector(0,0,50),pTarget->GetPosition()+rvector(0,0,130))<50)
					{
						fDamageRange = 1.f;
					}else
					{
#define MAX_DMG_RANGE	50.f	// 반경이만큼 까지는 최대 데미지를 다 먹는다

						fDamageRange = 1.f - (1.f-fMinDamage)*( max(fDist-MAX_DMG_RANGE,0) / (fRange-MAX_DMG_RANGE));
					}
				}else {
					fDamageRange = 1.f;
				}

				// resist 를 체크한다
				float fDamage = pWeapon->GetDesc()->nModDamage;
				if(pWeapon->GetDesc()->CheckResist(pTarget,&fDamage)) 
				{
					ZCharacter* pOwnerCharacter = g_pGame->m_CharacterManager.Find( uidOwner );
					if(pOwnerCharacter) {
						CheckCombo(pOwnerCharacter, pTarget,true);
						CheckStylishAction(pOwnerCharacter);
					}

					// 수류탄을 맞으면 반동으로 튀어나간다.
					rvector dir=pos-(pTarget->GetPosition()+rvector(0,0,80));
					Normalize(dir);
					pTarget->AddVelocity(fKnockBack*7.f*(fRange-fDist)*-dir);

					float fActualDamage = fDamage * fDamageRange;
					float fRatio = ZItem::GetPiercingRatio( MWT_FRAGMENTATION , eq_parts_chest );//수류탄과 로켓 구분없다..
					pTarget->OnDamaged(pOwnerCharacter,pos,ZD_MAGIC,MWT_FRAGMENTATION,fActualDamage,fRatio);
				} else {
					// TODO: 저항에 성공했으니 이펙트를 보여주자.
				}
			}
		}
	}

/*
#define SHOCK_RANGE		1500.f			// 10미터까지 흔들린다

	ZCharacter *pTargetCharacter=ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	float fPower= (SHOCK_RANGE-Magnitude(pTargetCharacter->GetPosition()+rvector(0,0,50) - pos))/SHOCK_RANGE;

	if ((fPower>0) && (pWeapon->GetDesc()->bCameraShock))
		ZGetGameInterface()->GetCamera()->Shock(fPower*500.f, .5f, rvector(0.0f, 0.0f, -1.0f));
*/

	if (pWeapon->GetDesc()->bCameraShock)
	{
		ZCharacter *pTargetCharacter=ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
		const float fDefaultPower = 500.0f;
		float fShockRange = pWeapon->GetDesc()->fCameraRange;
		float fDuration = pWeapon->GetDesc()->fCameraDuration;
		float fPower= (fShockRange-Magnitude(pTargetCharacter->GetPosition()+rvector(0,0,50) - pos))/fShockRange;
		fPower *= pWeapon->GetDesc()->fCameraPower;
		
		if (fPower>0)
		{
			ZGetGameInterface()->GetCamera()->Shock(fPower*fDefaultPower, fDuration, rvector(0.0f, 0.0f, -1.0f));
		}
	}

	GetWorld()->GetWaters()->CheckSpearing( pos, pos + rvector(0,0,MAX_WATER_DEEP), 500, 0.8f );
}

void ZGame::OnExplosionMagicNonSplash(ZWeaponMagic *pWeapon, MUID uidOwner, MUID uidTarget, rvector pos, float fKnockBack)
{
	ZObject* pTarget = m_CharacterManager.Find( uidTarget );
	if (pTarget == NULL) return;

	// 범위공격이 아니라면 타겟만 검사하면된다.
	if(pTarget->IsNPC()) return;

	if(pTarget && !pTarget->IsDie()) {

		// resist 를 체크한다
		float fDamage = pWeapon->GetDesc()->nModDamage;
		if(pWeapon->GetDesc()->CheckResist(pTarget,&fDamage)) 
		{
			ZCharacter* pOwnerCharacter = g_pGame->m_CharacterManager.Find( uidOwner );
			if(pOwnerCharacter) {
				CheckCombo(pOwnerCharacter, pTarget,true);
				CheckStylishAction(pOwnerCharacter);
			}

			// 반동으로 튀어나간다.
			rvector dir=pos-(pTarget->GetPosition()+rvector(0,0,80));
			Normalize(dir);
			pTarget->AddVelocity(fKnockBack*7.f*-dir);

			float fRatio = ZItem::GetPiercingRatio( MWT_FRAGMENTATION , eq_parts_chest );//수류탄과 로켓 구분없다..
			pTarget->OnDamaged(pOwnerCharacter,pos,ZD_MAGIC,MWT_FRAGMENTATION,fDamage,fRatio);
		} else {
			// TODO: 저항에 성공했으니 이펙트를 보여주자.
		}
	}
}

int ZGame::SelectSlashEffectMotion(ZCharacter* pCharacter)
{
	if(pCharacter==NULL) return SEM_None;

	// 남녀가 같아졌지만 혹시 또 바뀔지 모르니 놔둔다~~

//	MWT_DAGGER
//	MWT_DUAL_DAGGER
//	MWT_KATANA
//	MWT_GREAT_SWORD
//	MWT_DOUBLE_KATANA

	ZC_STATE_LOWER lower = pCharacter->m_AniState_Lower;

	int nAdd = 0;
	int ret = 0;

	MMatchWeaponType nType = pCharacter->GetSelectItemDesc()->m_nWeaponType;

	if(pCharacter->IsMan()) {

			 if(lower == ZC_STATE_LOWER_ATTACK1) {	nAdd = 0;	}
		else if(lower == ZC_STATE_LOWER_ATTACK2) {	nAdd = 1;	}
		else if(lower == ZC_STATE_LOWER_ATTACK3) {	nAdd = 2;	}
		else if(lower == ZC_STATE_LOWER_ATTACK4) {	nAdd = 3;	}
		else if(lower == ZC_STATE_LOWER_ATTACK5) {	nAdd = 4;	}
		else if(lower == ZC_STATE_LOWER_UPPERCUT) {	return SEM_ManUppercut;	}

			 if(nType == MWT_KATANA )		return SEM_ManSlash1 + nAdd;
		else if(nType == MWT_DOUBLE_KATANA)	return SEM_ManDoubleSlash1 + nAdd;
		else if(nType == MWT_GREAT_SWORD)	return SEM_ManGreatSwordSlash1 + nAdd;

	}
	else {

			 if(lower == ZC_STATE_LOWER_ATTACK1) {	nAdd = 0;	}
		else if(lower == ZC_STATE_LOWER_ATTACK2) {	nAdd = 1;	}
		else if(lower == ZC_STATE_LOWER_ATTACK3) {	nAdd = 2;	}
		else if(lower == ZC_STATE_LOWER_ATTACK4) {	nAdd = 3;	}
		else if(lower == ZC_STATE_LOWER_ATTACK5 ) {	nAdd = 4;	}
		else if(lower == ZC_STATE_LOWER_UPPERCUT) {	return SEM_WomanUppercut;	}

			 if(nType == MWT_KATANA )		return SEM_WomanSlash1 + nAdd;
		else if(nType == MWT_DOUBLE_KATANA)	return SEM_WomanDoubleSlash1 + nAdd;
		else if(nType == MWT_GREAT_SWORD)	return SEM_WomanGreatSwordSlash1 + nAdd;
	}

	return SEM_None;
}

// shot 이 너무 커서 분리..
void ZGame::OnPeerShot_Melee(const MUID& uidOwner, float fShotTime)
{
	ZObject *pOwner = m_ObjectManager.GetObject(uidOwner);
	if(!pOwner) return;

	ZItem *pItem = pOwner->GetItems()->GetItem(MMCIP_MELEE);
	if(!pItem) return;
	MMatchItemDesc *pDesc = pItem->GetDesc();
	if(!pDesc) { _ASSERT(FALSE); return; }

	ZGetSoundEngine()->PlaySound("blade_swing",pOwner->m_Position, 
		uidOwner==ZGetGameInterface()->GetCombatInterface()->GetTargetUID());

	RPickInfo info;

	bool bCheck = false;

	fShotTime=GetTime();

	rvector OwnerPosition,OwnerDir;
	OwnerPosition = pOwner->GetPosition();
	OwnerDir = pOwner->m_Direction;
	OwnerDir.z=0; 
	Normalize(OwnerDir);

	float fMeleeRange = pDesc->m_nRange;
	if (fMeleeRange == 0) {
		_ASSERT(FALSE);	// Unknown WeaponRange
		fMeleeRange = 150.f;
	}


	// 남녀 칼 휘두르는 방향을 구해놓는다
	int cm = 0;
	
{
	ZCharacter *pOwnerCharacter = m_CharacterManager.Find(uidOwner);
	if(pOwnerCharacter) {
		cm = SelectSlashEffectMotion(pOwnerCharacter);
	}
}

	for (ZObjectManager::iterator itor = m_ObjectManager.begin();
		itor != m_ObjectManager.end(); ++itor)
	{
		ZObject* pTar = (*itor).second;
		if (pOwner == pTar) continue;

		rvector TargetPosition,TargetDir,TargetDirLow;

		if(pTar->IsDie()) continue;
		// 적절한 위치를 얻어낼수 없으면 다음으로~
		if( !pTar->GetHistory(&TargetPosition,&TargetDir,fShotTime)) continue;

//		float fDist = Magnitude(OwnerPosition + OwnerDir*100.f - TargetPosition);
		// 키가 각각 다른 object들에 대해서 칼의 위치 ~ object의 실린더 중심축선분과의 거리를 구한다
		rvector swordPos = OwnerPosition + OwnerDir*100.f;
		swordPos.z += pOwner->GetCollHeight() * .5f;
		float fDist = GetDistanceLineSegment(swordPos,TargetPosition,TargetPosition+rvector(0,0,pTar->GetCollHeight()));

		if (fDist < fMeleeRange) {

			if( IsAttackable(pOwner,pTar) ) {
				
				rvector fTarDir = TargetPosition - (OwnerPosition - OwnerDir*50.f);
				Normalize(fTarDir);
				float fDot = D3DXVec3Dot(&OwnerDir, &fTarDir);

				if (fDot > 0.5f) {

					if(CheckWall(pOwner,pTar)==false) //중간에 벽이 막고 있는가?
					{
						if(pTar->IsGuard() && DotProduct(pTar->m_Direction,OwnerDir)<0 ) {	// 막고있으며 방향이 맞으면
							rvector t_pos = pTar->GetPosition();
							t_pos.z += 120.f;
							ZGetEffectManager()->AddSwordDefenceEffect(t_pos+(pTar->m_Direction*50.f),pTar->m_Direction);
							pTar->OnMeleeGuardSuccess();
						}else
						{
							rvector tpos = pTar->GetPosition();

							tpos.z += 130.f;
							tpos -= pOwner->m_Direction * 50.f;

							ZGetEffectManager()->AddBloodEffect( tpos , -fTarDir);
							ZGetEffectManager()->AddSlashEffect( tpos , -fTarDir , cm);

							float fActualDamage = CalcActualDamage(pOwner, pTar, (float)pDesc->m_nDamage);
							float fRatio = pItem->GetPiercingRatio( pDesc->m_nWeaponType , eq_parts_chest );
							pTar->OnDamaged(pOwner, pOwner->GetPosition(), ZD_MELEE, pDesc->m_nWeaponType, fActualDamage, fRatio, cm);

							ZActor* pATarget = MDynamicCast(ZActor,pTar);

							bool bPushSkip = false;

							if(pATarget) {
								bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
							}

							float fKnockbackForce = pItem->GetKnockbackForce();

							if(bPushSkip) {
								ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
								fKnockbackForce = 1.0f;
							}

							pTar->OnKnockback( pOwner->m_Direction, fKnockbackForce );

							ZGetSoundEngine()->PlaySound("blade_damage", tpos );

							bCheck = true;

							if(pOwner == m_pMyCharacter) {
								CheckCombo(m_pMyCharacter, pTar,!bPushSkip);
								CheckStylishAction(m_pMyCharacter);
							}
						}
					}
				}
			}//isTeamAttackable
		}
	}

	// 아무일도 안하는 코드
//	int cm = SelectSlashEffectMotion(pOwnerCharacter);//남녀 칼 휘두르는 방향
//	if( (cm==SEM_ManSlash5) || (cm==SEM_WomanSlash5) ) {
//
//		rvector _vp	  = pOwnerCharacter->GetPosition();
//		rvector _vdir = pOwnerCharacter->m_Direction;
//		_vdir.z = 0;
//		Normalize(_vdir);
//
//		if(cm==SEM_ManSlash5) {
//			_vdir.z = 0.f;
//			_vp += _vdir * 200.f;
////			_vp.z -= 460.f;
////			m_EffectManager.AddSwordWaveEffect(_vp,0,pOwnerCharacter);
//		}
//		else if(cm==SEM_WomanSlash5) {
//			_vdir.z = 0.f;
//			_vp += _vdir * 200.f;
////			_vp.z -= 50.f;
////			m_EffectManager.AddSwordWaveEffect(_vp,0,pOwnerCharacter);
//		}
//	}


	// 캐릭터를 때리지 못했다면...
	// test 방향으로 벽과 충돌할경우 스파크..
	if(!bCheck){

		rvector vPos = pOwner->GetPosition();
		rvector vDir = OwnerDir;

		vPos.z += 130.f;

		RBSPPICKINFO bpi;

		if(GetWorld()->GetBsp()->Pick(vPos, vDir, &bpi)) {

			float fDist = Magnitude(vPos - bpi.PickPos);

			if (fDist < fMeleeRange) {

				rplane r = bpi.pInfo->plane;
				rvector vWallDir = rvector( r.a, r.b, r.c );
				Normalize(vWallDir);

//				int cm = SelectSlashEffectMotion(pOwnerCharacter);//남녀 칼 휘두르는 방향

				ZGetEffectManager()->AddSlashEffectWall( bpi.PickPos - (vDir*5.f) , vWallDir ,cm);

				rvector pos = bpi.PickPos;

				ZGetSoundEngine()->PlaySound("blade_concrete", pos );

				//test 용..
				//					int cm = SelectSlashEffectMotion(pOwnerCharacter);//남녀 칼 휘두르는 방향
				//					m_EffectManager.AddSlashEffect(bpi.PickPos - (vDir*5.f), -vDir,cm);
			}
		}
	}

	return;
}

void ZGame::OnPeerShot_Range(MMatchCharItemParts sel_type, const MUID& uidOwner, float fShotTime, rvector& pos, rvector& to)
{
	ZObject *pOwner = m_ObjectManager.GetObject(uidOwner);
	if(!pOwner) return;

	ZItem *pItem = pOwner->GetItems()->GetItem(sel_type);
	if(!pItem) return;
	MMatchItemDesc *pDesc = pItem->GetDesc();
	if(!pDesc) { _ASSERT(FALSE); return; }

	rvector dir = to - pos;

	Normalize(dir);

	rvector v1, v2;
	rvector BulletMarkNormal;
	bool bBulletMark = false;
	ZTargetType nTargetType = ZTT_OBJECT;

	ZCharacter* pCharacter = NULL;

	ZPICKINFO pickinfo;

	memset(&pickinfo,0,sizeof(ZPICKINFO));

	// 총알은 로켓이 통과하는곳도 통과한다
	const DWORD dwPickPassFlag=RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;

	// 쏘는 캐릭터 흔들어 주기..
	pOwner->Tremble(8.f, 50, 30);

/*
	if(pOwner->m_pVMesh)
	{
		float fMaxValue = 8.f;// 흔들 강도 +- 가능

		RFrameTime* ft = &pOwner->m_pVMesh->m_FrameTime;
		if(ft && !ft->m_bActive)
			ft->Start(fMaxValue,50,30);// 강도 , 최대시간 , 복귀시간...
	}
*/

	if(g_pGame->PickHistory(pOwner,fShotTime,pos,to,&pickinfo,dwPickPassFlag))
	{
		/* 땜빵 -bird */
		
		if(pickinfo.pObject)
		{
			ZObject *pObject = pickinfo.pObject;
			bool bGuard = pObject->IsGuard() && (pickinfo.info.parts!=eq_parts_legs) &&		// 다리는 막을수없다
							DotProduct(dir,pObject->GetDirection())<0;

			if(bGuard) {
				nTargetType = ZTT_CHARACTER_GUARD;
				// 막았다
				rvector t_pos = pObject->GetPosition();
				t_pos.z += 100.f;
				ZGetEffectManager()->AddSwordDefenceEffect(t_pos+(-dir*50.f),-dir);
				pObject->OnGuardSuccess();

			}else {
				nTargetType = ZTT_CHARACTER;

				ZActor* pATarget = MDynamicCast(ZActor,pickinfo.pObject);

				bool bPushSkip = false;

				if(pATarget) {
					bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
				}

				float fKnockbackForce = pItem->GetKnockbackForce();

				if(bPushSkip) 
				{
//					ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
					rvector vPos = pOwner->GetPosition() + (pickinfo.pObject->GetPosition() - pOwner->GetPosition()) * 0.1f; 
					ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met", vPos );
					fKnockbackForce = 1.0f;
				}

				pickinfo.pObject->OnKnockback( pOwner->m_Direction, fKnockbackForce );

				float fActualDamage = CalcActualDamage(pOwner, pickinfo.pObject, (float)pDesc->m_nDamage);
				float fRatio = pItem->GetPiercingRatio( pDesc->m_nWeaponType
 , pickinfo.info.parts );
				ZDAMAGETYPE dt = (pickinfo.info.parts==eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET;
				pickinfo.pObject->OnDamaged(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType, fActualDamage, fRatio );

				if(pOwner == m_pMyCharacter) {
					CheckCombo(m_pMyCharacter,pickinfo.pObject,!bPushSkip);
					CheckStylishAction(m_pMyCharacter);
				}
			}

			v1 = pos;
			v2 = pickinfo.info.vOut;

		}else
			if(pickinfo.bBspPicked)
			{
				nTargetType = ZTT_OBJECT;

				v1 = pos;
				v2 = pickinfo.bpi.PickPos;

				// 총탄 흔적
				BulletMarkNormal.x = pickinfo.bpi.pInfo->plane.a;
				BulletMarkNormal.y = pickinfo.bpi.pInfo->plane.b;
				BulletMarkNormal.z = pickinfo.bpi.pInfo->plane.c;
				Normalize(BulletMarkNormal);
				bBulletMark = true;

			}
			else {
				_ASSERT(false);
				return;
			}
	}
	else {
		v1 = pos;
		v2 = pos+dir*10000.f;
		nTargetType	= ZTT_NOTHING;
	}

	bool bPlayer = false;
	//bool b3D = (pOwnerCharacter!=m_pMyCharacter);	// 자기가 내는 사운드는 2D로 출력한다.
	//rvector Pos = pOwnerCharacter->GetPosition();
	rvector Pos = v1;
	if(pOwner==m_pMyCharacter)
	{

		Pos = RCameraPosition;
		bPlayer = true;
	}

	ZCharacter *pTargetCharacter=ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	ZApplication::GetSoundEngine()->PlaySEFire(pDesc, Pos.x, Pos.y, Pos.z, bPlayer);
	//if(nTargetType == ZTT_OBJECT) { ZApplication::GetSoundEngine()->PlaySERicochet(v2.x, v2.y, v2.z); }
#define SOUND_CULL_DISTANCE 1500.0F
	if( D3DXVec3LengthSq(&(v2 - pTargetCharacter->m_Position)) < (SOUND_CULL_DISTANCE * SOUND_CULL_DISTANCE) )
	{
		if(nTargetType == ZTT_OBJECT) { 
			ZGetSoundEngine()->PlaySEHitObject( v2.x, v2.y, v2.z, pickinfo.bpi ); 
		}

		if(nTargetType == ZTT_CHARACTER) { 
			ZGetSoundEngine()->PlaySEHitBody(v2.x, v2.y, v2.z); 
		}
	}

	//// 보이지 않으면 이펙트를 그릴필요는 없다 - 정확한 컬링을 요망.. by bird
	//if(!pOwner->IsRendered()) return;


	// 쏘는곳 반경 100cm 가 화면에 들어가는지 체크한다
	bool bDrawFireEffects = isInViewFrustum(v1,100.f,RGetViewFrustum());

	if(!isInViewFrustum(v1,v2,RGetViewFrustum()) // 쏘는곳에서 맞는곳의 라인이 보이는지..
		&& !bDrawFireEffects) return;					// 쏘는곳에서도 그릴게 없는지..

	bool bDrawTargetEffects = isInViewFrustum(v2,100.f,RGetViewFrustum());



	/////////////////////// 이후는 이펙트 추가

	// 물튀는 이펙트 체크
	GetWorld()->GetWaters()->CheckSpearing( v1, v2, 250, 0.3 );



	// TODO: NPC 의 총구위치 인터페이스가 확정되면 마저 추가하자.
//	ZCharacter *pOwnerCharacter = m_CharacterManager.Find(uidOwner);

	
	ZCharacterObject* pCOwnerObject = MDynamicCast(ZCharacterObject, pOwner);

	if(pCOwnerObject) {

		// 총구 화염이펙트
		rvector pdir = v2-v1;
		Normalize(pdir);

		int size = 3;

		rvector v[6];

//		size = GetWeapondummyPos(pOwnerCharacter,v);
		if(pCOwnerObject->IsRendered())
			size = pCOwnerObject->GetWeapondummyPos(v);
		else
		{
			size = 6;
			v[0] = v[1] = v[2] = v1;
			v[3] = v[4] = v[5] = v[0];
		}


		MMatchWeaponType wtype = pDesc->m_nWeaponType;

		// Effect
		if(bBulletMark==false) BulletMarkNormal = -pdir;

		ZGetEffectManager()->AddShotEffect( v , size , v2, BulletMarkNormal, nTargetType, wtype,pCOwnerObject,bDrawFireEffects,bDrawTargetEffects);

		// 총 쏠때 라이트 추가
		ZCharacterObject* pChar;

		if( ZGetConfiguration()->GetVideo()->bDynamicLight && pCOwnerObject != NULL )
		{
			pChar = pCOwnerObject;
		
			if( pChar->m_bDynamicLight )
			{
				pChar->m_vLightColor = g_CharLightList[GUN].vLightColor;
				pChar->m_fLightLife = g_CharLightList[GUN].fLife;
			}
			else
			{
				pChar->m_bDynamicLight = true;
				pChar->m_vLightColor = g_CharLightList[GUN].vLightColor;
				pChar->m_vLightColor.x = 1.0f;
				pChar->m_iDLightType = GUN;
				pChar->m_fLightLife = g_CharLightList[GUN].fLife;
			}
		}
	}
	
	// 깃발에 힘 적용			   p
	GetWorld()->GetFlags()->CheckSpearing( v1, v2, BULLET_SPEAR_EMBLEM_POWER );
	if(Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->AddLightSource( v1, 2.0f, 75 );
}

void ZGame::OnPeerShot_Shotgun(ZItem *pItem, ZCharacter* pOwnerCharacter, float fShotTime, rvector& pos, rvector& to)
{
	// 내 캐릭터 혹은 내가 보고있는 캐릭터
	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if(!pTargetCharacter) return;

	//// 디버그용 덤프
	//{
	//	if(strcmp("폭주천사다크",pTargetCharacter->GetProperty()->szName)==0) {
	//		mlog("shot : %3.3f \n",fShotTime);
	//	}
	//}


	MMatchItemDesc *pDesc = pItem->GetDesc();
	if(!pDesc) { _ASSERT(false); return; }

#define SHOTGUN_BULLET_COUNT	12
#define SHOTGUN_DIFFUSE_RANGE	0.1f

	bool waterSound = false;

	if (pOwnerCharacter == NULL) return;

	// 모든사람이 같은 random seed 를 갖도록 같은값으로 초기화 해준다
	int *seed=(int*)&fShotTime;
	srand(*seed);

	bool bHitGuard=false,bHitBody=false,bHitGround=false,bHitEnemy=false;
	rvector GuardPos,BodyPos,GroundPos;

	rvector v1, v2;

	rvector origdir = to - pos;
	Normalize(origdir);

	int nHitCount = 0;

	for(int i=0;i<SHOTGUN_BULLET_COUNT;i++)
	{
		rvector dir = origdir;

		{
			// 오차값 - 반동대신 시범삼아 넣음
			rvector r, up(0,0,1), right;
			D3DXQUATERNION q;
			D3DXMATRIX mat;

			float fAngle = (rand() % (31415 * 2)) / 1000.0f;
			float fForce = RANDOMFLOAT*SHOTGUN_DIFFUSE_RANGE;

			D3DXVec3Cross(&right,&dir,&up);
			D3DXVec3Normalize(&right,&right);
			D3DXMatrixRotationAxis(&mat, &right, fForce);
			D3DXVec3TransformCoord(&r, &dir, &mat);

			D3DXQuaternionRotationAxis(&q, &dir, fAngle);
			D3DXMatrixRotationQuaternion(&mat, &q);
			D3DXVec3TransformCoord(&r, &r, &mat);

			dir=r;
		}

		rvector BulletMarkNormal;
		bool bBulletMark = false;
		ZTargetType nTargetType = ZTT_OBJECT;

		ZPICKINFO pickinfo;

		memset(&pickinfo,0,sizeof(ZPICKINFO));

		// 총알은 로켓이 통과하는곳도 통과한다
		const DWORD dwPickPassFlag=RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;

		if(g_pGame->PickHistory(pOwnerCharacter,fShotTime,pos,pos+10000.f*dir,&pickinfo,dwPickPassFlag))
		{
			ZObject *pObject = pickinfo.pObject;
			if(pObject)
			{
				bool bGuard = pObject->IsGuard() && (pickinfo.info.parts!=eq_parts_legs) &&		// 다리는 막을수없다
								DotProduct(dir,pObject->GetDirection())<0;

				if(bGuard) {
					rvector t_pos = pObject->GetPosition();
					t_pos.z += 100.f;
					ZGetEffectManager()->AddSwordDefenceEffect(t_pos+(-dir*50.f),-dir);
					pObject->OnGuardSuccess();

					nTargetType = ZTT_CHARACTER_GUARD;
					bHitGuard=true;

				} else {

					ZActor* pATarget = MDynamicCast(ZActor,pObject);

					bool bPushSkip = false;

					if(pATarget) {
						bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
					}

					float fKnockbackForce = pItem->GetKnockbackForce() / (.5f*float(SHOTGUN_BULLET_COUNT));

					if(bPushSkip) {
//						ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
						rvector vPos = pOwnerCharacter->GetPosition() + (pObject->GetPosition() - pOwnerCharacter->GetPosition()) * 0.1f; 
						ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met", vPos );
						fKnockbackForce = 1.0;
					}

					pObject->OnKnockback( dir, fKnockbackForce );

					float fActualDamage = CalcActualDamage(pOwnerCharacter, pObject, (float)pDesc->m_nDamage);
					float fRatio = ZItem::GetPiercingRatio( pDesc->m_nWeaponType , pickinfo.info.parts );
					ZDAMAGETYPE dt = (pickinfo.info.parts==eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET;
					pObject->OnDamaged(pOwnerCharacter, pOwnerCharacter->GetPosition(), dt, pDesc->m_nWeaponType, fActualDamage, fRatio );

					nTargetType = ZTT_CHARACTER;
					bHitBody=true;

					// 우리편 때린것은 combo 에 포함되지 않음
					if(!m_Match.IsTeamPlay() || (pTargetCharacter->GetTeamID()!=pObject->GetTeamID()))
					{
						bHitEnemy=true;
					}
				}

				v1 = pos;
				v2 = pickinfo.info.vOut;

			}else
				if(pickinfo.bBspPicked)
		//		if(pickinfo.bBspPicked || (pCharacter == pOwnerCharacter))
				{
					bHitGround=true;
					nTargetType = ZTT_OBJECT;

					v1 = pos;
					v2 = pickinfo.bpi.PickPos;

					// 총탄 흔적
					BulletMarkNormal.x = pickinfo.bpi.pInfo->plane.a;
					BulletMarkNormal.y = pickinfo.bpi.pInfo->plane.b;
					BulletMarkNormal.z = pickinfo.bpi.pInfo->plane.c;
					Normalize(BulletMarkNormal);
					bBulletMark = true;

					// 맞는곳 반경 20cm 가 화면에 들어오면 그린다
					bool bDrawTargetEffects = isInViewFrustum(v2,20.f,RGetViewFrustum());
					if(bDrawTargetEffects)
						ZGetEffectManager()->AddBulletMark(v2,BulletMarkNormal);

/*
///////////////////////////////////////////////////////////////////////////////////////////////////////
// by 베니
// 샷건이 이펙트가 제대로 나오지 않아서 수정
#define TARGET_SMOKE_MAX_SCALE		50.0f
#define TARGET_SMOKE_MIN_SCALE		40.0f
#define TARGET_SMOKE_LIFE_TIME		0.9f
#define TARGET_SMOKE_VELOCITY		0.2f				// meter/sec
#define TARGET_SMOKE_ACCEL			rvector(0,0,100.f)	// meter/sec

	int max_cnt = 0;

	if(GetEffectLevel()==0)	max_cnt = 5;
	else if(GetEffectLevel()==1)	max_cnt = 3;
	else if(GetEffectLevel()==2)	max_cnt = 1;

	if(max_cnt) {
	//m_EffectManager.AddShotEffect(v,size, v2, BulletMarkNormal, nTargetType, NULL,ring_draw,wtype,pOwnerCharacter);	
		for(int i=0; i<max_cnt; i++) {
			rvector p = v2+BulletMarkNormal*TARGET_SMOKE_MIN_SCALE*float(i)*0.5f + rvector(fmod((float)rand(), TARGET_SMOKE_MIN_SCALE), fmod((float)rand(), TARGET_SMOKE_MIN_SCALE), fmod((float)rand(), TARGET_SMOKE_MIN_SCALE));
			float fSize = 1.0f+float(rand()%100)/100.0f;
			m_EffectManager.AddSmokeEffect(m_EffectManager.m_pEBSSmokes[rand()%SMOKE_COUNT], p, BulletMarkNormal*TARGET_SMOKE_VELOCITY,rvector(0,100.f,0), TARGET_SMOKE_MIN_SCALE*fSize, TARGET_SMOKE_MAX_SCALE*fSize, TARGET_SMOKE_LIFE_TIME);
		}
		m_EffectManager.AddLightFragment(v2,BulletMarkNormal);

	}
///////////////////////////////////////////////////////////////////////////////////////////////////////
*/
				}
				else {
					_ASSERT(false);
					return;
				}
		}
		else {
			v1 = pos;
			v2 = pos+dir*10000.f;
			nTargetType	= ZTT_NOTHING;
		}

		waterSound = GetWorld()->GetWaters()->CheckSpearing( v1, v2, 250, 0.3, !waterSound );
	}

	
	/////////////////////////////////////////////////////
	if(bHitEnemy) {
		CheckStylishAction(pOwnerCharacter);
		CheckCombo(pOwnerCharacter, NULL,true);
	}


	ZApplication::GetSoundEngine()->PlaySEFire(pItem->GetDesc(), pos.x, pos.y, pos.z, (pOwnerCharacter==m_pMyCharacter));

	// 보이지 않으면 이펙트를 그릴필요는 없다
	if(!pOwnerCharacter->IsRendered()) return;

	rvector v[6];

	int _size = pOwnerCharacter->GetWeapondummyPos(v);

	rvector dir = to - pos;
	Normalize(dir);

	ZGetEffectManager()->AddShotgunEffect(pos,v[1],dir,pOwnerCharacter);

	// 총 쏠때 라이트 추가
	ZCharacter* pChar;
	if( ZGetConfiguration()->GetVideo()->bDynamicLight && pOwnerCharacter != NULL )
	{
		pChar = pOwnerCharacter;

		if( pChar->m_bDynamicLight )
		{
			pChar->m_vLightColor = g_CharLightList[SHOTGUN].vLightColor;
			pChar->m_fLightLife = g_CharLightList[SHOTGUN].fLife;
		}
		else
		{
			pChar->m_bDynamicLight = true;
			pChar->m_vLightColor = g_CharLightList[SHOTGUN].vLightColor;
			pChar->m_vLightColor.x = 1.0f;
			pChar->m_iDLightType = SHOTGUN;
			pChar->m_fLightLife = g_CharLightList[SHOTGUN].fLife;
		}
	}

//	m_flags.CheckSpearing( v1, v2, SHOTGUN_SPEAR_EMBLEM_POWER );
	if(Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->AddLightSource(v1, 2.0f, 200 );
}

// shot 을 shot_range, shot_melee, shot_shotgun 으로 command 를 각각 분리하는것도 방법이 좋을듯.
void ZGame::OnPeerShot(MUID& uid, float fShotTime, rvector& pos, rvector& to,MMatchCharItemParts sel_type)
{
	ZCharacter* pOwnerCharacter = NULL;		// 총 쏜 사람

	pOwnerCharacter = m_CharacterManager.Find(uid);

	if (pOwnerCharacter == NULL) return;
	if(!pOwnerCharacter->IsVisible()) return;

	pOwnerCharacter->OnShot();

	// fShotTime 이 그 캐릭터의 로컬 시간이므로 내 시간으로 변환해준다
	fShotTime-=pOwnerCharacter->m_fTimeOffset;

	ZItem *pItem = pOwnerCharacter->GetItems()->GetItem(sel_type);
	if(!pItem || !pItem->GetDesc()) return;

	// 비정상적인 발사속도를 무시한다.
	if (pOwnerCharacter->CheckValidShotTime(pItem->GetDescID(), fShotTime, pItem)) {
		pOwnerCharacter->UpdateValidShotTime(pItem->GetDescID(), fShotTime);
	} else {
		return;
	}

	//// 루프중 MEMORYHACK있었나 검사
	if (uid == ZGetMyUID()) {
		MDataChecker* pChecker = ZApplication::GetGame()->GetDataChecker();
		MDataCheckNode* pCheckNode = pChecker->FindCheck((BYTE*)pItem->GetAMagazinePointer());
		if (pCheckNode && (pCheckNode->UpdateChecksum()==false)) {
			pChecker->BringError();	//// MEMORYHACK 감지. Checksum 모두중단하고 끝장낸다.
		} else {
			int nAMagazine = pItem->GetBulletAMagazine();

			// 실제로 무기를 소비
			if (!pItem->Shot()) return;	//// SHOT ////

			//// MEMORYHACK 없었으면 리뉴한다.
			if (pItem->GetBulletAMagazine() < nAMagazine)	// Shot에서 총알 줄어야만 정상이다
				pChecker->RenewCheck((BYTE*)pItem->GetAMagazinePointer(), sizeof(int));
			else
				if(sel_type != MMCIP_MELEE)
					pChecker->BringError();
		}
	} else {
		// 실제로 무기를 소비
		if (!pItem->Shot()) {
//			_ASSERT(FALSE);	// 문제가있다, 치팅 ?
			return;	// SHOT
		}
	}

	// MELEE일 경우
	if (sel_type == MMCIP_MELEE)
	{
		OnPeerShot_Melee(uid,fShotTime);
		return;
	}

	if ((sel_type != MMCIP_PRIMARY) &&
		(sel_type != MMCIP_SECONDARY) &&
		(sel_type != MMCIP_CUSTOM1 )) return;


	if(!pItem->GetDesc()) return;
	MMatchWeaponType wtype = pItem->GetDesc()->m_nWeaponType;

	if(wtype == MWT_SHOTGUN)
	{
		OnPeerShot_Shotgun(pItem,pOwnerCharacter,fShotTime,pos,to);
		return;
	}

	OnPeerShot_Range(sel_type,uid,fShotTime,pos,to);

	rvector position;
	pOwnerCharacter->GetWeaponTypePos( weapon_dummy_muzzle_flash , &position );
	if( ZGetConfiguration()->GetVideo()->bDynamicLight )
	{
		RGetDynamicLightManager()->AddLight( GUNFIRE, position );
	}
}

void ZGame::OnPeerDie(MUID& uidVictim, MUID& uidAttacker)
{
	ZCharacter* pVictim = m_CharacterManager.Find(uidVictim);
	if (pVictim == NULL) return;

	pVictim->ActDead();

	if (pVictim == m_pMyCharacter)	
	{
		pVictim->Die();		// 여기서 실제로 죽는다. 나 자신은 실제로도 여기서 죽는것 처리

		// 팀플시 또는 퀘스트모드시 죽으면 옵져버 모드
		if (m_Match.IsWaitForRoundEnd())
		{
			if (m_CharacterManager.GetCount() > 2)
			{
				if (GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL)
					ReserveObserver();
			}
		}
#ifdef _QUEST
		else if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
		{
			if (m_CharacterManager.GetCount() >= 2)
			{
				ReserveObserver();
			}
		}
#endif

		CancelSuicide();
	}


	ZCharacter* pAttacker = m_CharacterManager.Find(uidAttacker);
	if (pAttacker == NULL) return;	// 죽인 사람이 누구인지 모를 경우는 그냥 리턴
	if(pAttacker!=pVictim)	// 자살이 아니면 이펙트 보여줄거 보여주자
	{
		if (ZApplication::GetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
		{
			if (pAttacker->GetKils() + 1 == 5)
			{
				pAttacker->GetStatus()->nFantastic++;
				pAttacker->AddIcon(ZCI_FANTASTIC);
			}
			else if (pAttacker->GetKils() + 1 == 15)
			{
				pAttacker->GetStatus()->nExcellent++;
				pAttacker->AddIcon(ZCI_EXCELLENT);
			}
			else if (pAttacker->GetKils() + 1 == 30)
			{
				pAttacker->GetStatus()->nUnbelievable++;
				pAttacker->AddIcon(ZCI_UNBELIEVABLE);
			}
		}
		else
		{
			if (pAttacker->GetStatus()->nKills>=3)
			{
				pAttacker->GetStatus()->nFantastic++;
				pAttacker->AddIcon(ZCI_FANTASTIC);
			}
		}

		if(pVictim->GetLastDamageType()==ZD_BULLET_HEADSHOT)
		{
			pAttacker->GetStatus()->nHeadShot++;
			pAttacker->AddIcon(ZCI_HEADSHOT);
		}
	}
}

// 서버로부터 직접 날라오는 Dead메세지
void ZGame::OnPeerDead(const MUID& uidAttacker, const unsigned long int nAttackerArg, 
					   const MUID& uidVictim, const unsigned long int nVictimArg)
{
	ZCharacter* pVictim = m_CharacterManager.Find(uidVictim);
	ZCharacter* pAttacker = m_CharacterManager.Find(uidAttacker);

	bool bSuicide = false;
	if (uidAttacker == uidVictim) bSuicide = true;

	int nAttackerExp = 0, nVictimExp = 0;

	nAttackerExp = GetExpFromTransData(nAttackerArg);
	nVictimExp = -GetExpFromTransData(nVictimArg);

	if(pAttacker)
	{
		ZCharacterStatus* pAttackerCS = NULL;
		pAttackerCS = pAttacker->GetStatus();
		pAttackerCS->AddExp(nAttackerExp);

		if (!bSuicide) pAttackerCS->AddKills();
	}

	if(pVictim)
	{
		if (pVictim != m_pMyCharacter)
		{
			pVictim->Die();		// 여기서 실제로 죽는다
		}

		ZCharacterStatus* pVictimCS = NULL;
		pVictimCS = pVictim->GetStatus();
		pVictimCS->AddExp(nVictimExp);
		pVictimCS->AddDeaths();
		if (pVictimCS->nLife > 0) pVictimCS->nLife--;
	}

	// 화면 경험치 이펙트 표시
	if(bSuicide && (ZGetCharacterManager()->Find(uidAttacker)==g_pGame->m_pMyCharacter)) 
	{
		// 자살
		ZGetScreenEffectManager()->AddExpEffect(nVictimExp);
		int nExpPercent = GetExpPercentFromTransData(nVictimArg);
		ZGetMyInfo()->SetLevelPercent(nExpPercent);

		ZGetScreenEffectManager()->SetGuageExpFromMyInfo();
	}
	else if(ZGetCharacterManager()->Find(uidAttacker)==m_pMyCharacter) 
	{
		// 내가 attacker 일때
		ZGetScreenEffectManager()->AddExpEffect(nAttackerExp);

		int nExpPercent = GetExpPercentFromTransData(nAttackerArg);
		ZGetMyInfo()->SetLevelPercent(nExpPercent);
		ZGetScreenEffectManager()->SetGuageExpFromMyInfo();
	}
	else if(ZGetCharacterManager()->Find(uidVictim)==m_pMyCharacter) 
	{
		// 내가 victim 일때
		ZGetScreenEffectManager()->AddExpEffect(nVictimExp);

		int nExpPercent = GetExpPercentFromTransData(nVictimArg);
		ZGetMyInfo()->SetLevelPercent(nExpPercent);
		ZGetScreenEffectManager()->SetGuageExpFromMyInfo();
	}

	m_Match.AddRoundKills();

	CheckKillSound(pAttacker);
	OnPeerDieMessage(pVictim, pAttacker);
}

void ZGame::CheckKillSound(ZCharacter* pAttacker)
{
	if ((!pAttacker) || (pAttacker != m_pMyCharacter)) return;

	if (m_Match.GetRoundKills() == 1)
	{
		ZApplication::GetSoundEngine()->PlayVoiceSound(VOICE_FIRST_KILL);
	}
}

void ZGame::OnReceiveTeamBonus(const MUID& uidChar, const unsigned long int nExpArg)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uidChar);
	if (pCharacter == NULL) return;

	int nExp = 0;

	nExp = GetExpFromTransData(nExpArg);

	if(pCharacter)
	{
		pCharacter->GetStatus()->AddExp(nExp);
	}

	// 화면 경험치 이펙트 표시
	if(pCharacter==m_pMyCharacter) 
	{
#ifdef _DEBUG
		char szTemp[128];
		sprintf(szTemp, "TeamBonus = %d\n", nExp);
		OutputDebugString(szTemp);
#endif

		// 내가 attacker 일때
		ZGetScreenEffectManager()->AddExpEffect(nExp);

		
		int nExpPercent = GetExpPercentFromTransData(nExpArg);
		ZGetMyInfo()->SetLevelPercent(nExpPercent);
		ZGetScreenEffectManager()->SetGuageExpFromMyInfo();
	}
}

void ZGame::OnPeerDieMessage(ZCharacter* pVictim, ZCharacter* pAttacker)
{
	const char *testdeathnametable[ZD_END+1] = { "에러", "총", "칼", "추락", "폭발", "HEADSHOT", "마지막칼질" };
	char szMsg[256] = "";

	const char *szAnonymous = "?아무개?";

	char szVictim[256];
	strcpy(szVictim, pVictim ? pVictim->GetUserAndClanName() : szAnonymous);

	char szAttacker[256];
	strcpy(szAttacker, pAttacker ? pAttacker->GetUserAndClanName() : szAnonymous);

	// 자살일 경우
	if(pAttacker==pVictim)
	{
		if (pVictim == m_pMyCharacter)
		{
			if(m_pMyCharacter->GetLastDamageType()==ZD_EXPLOSION) {
//				sprintf(szMsg, "당신은 자신의 폭탄으로 인하여 패배 하였습니다.");
				sprintf( szMsg, ZMsg(MSG_GAME_LOSE_BY_MY_BOMB) );
			}
			else {
//				sprintf(szMsg, "당신은 스스로 패배하였습니다.");
				sprintf( szMsg, ZMsg(MSG_GAME_LOSE_MYSELF) );
			}

			ZChatOutput(MCOLOR(0xFFCF2020), szMsg);
		}
		else
		{
//			sprintf(szMsg, "%s님이 스스로 패배하였습니다.", szAttacker);
			ZTransMsg( szMsg, MSG_GAME_WHO_LOSE_SELF, 1, szAttacker );
			ZChatOutput(MCOLOR(0xFF707070), szMsg);

			// Admin Grade
			if (ZGetMyInfo()->IsAdminGrade()) {
				MMatchObjCache* pCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
				if (pCache && pCache->CheckFlag(MTD_PlayerFlags_AdminHide))
				{
					sprintf( szMsg, "^%d%s^9 스스로 패배",
									(pAttacker->GetTeamID() == MMT_BLUE) ? 3 : 1,
									pAttacker->GetProperty()->szName);
					ZGetGameInterface()->GetCombatInterface()->m_AdminMsg.OutputChatMsg( szMsg);
				}
			}
		}

	}

	// 내가 죽였을 때
	else if (pAttacker == m_pMyCharacter)
	{
//		sprintf(szMsg, "당신은 %s님으로부터 승리하였습니다.", szVictim );
		ZTransMsg( szMsg, MSG_GAME_WIN_FROM_WHO, 1, szVictim );
		ZChatOutput(MCOLOR(0xFF80FFFF), szMsg);
	}

	// 내가 죽었을 때
	else if (pVictim == m_pMyCharacter)
	{
//		sprintf(szMsg, "당신은 %s님에게 패배하였습니다.", szAttacker );
		ZTransMsg( szMsg, MSG_GAME_LOSE_FROM_WHO, 1, szAttacker );
		ZChatOutput(MCOLOR(0xFFCF2020), szMsg);
	}

	// 다른 사람이 다른 사람 죽였을때
	else
	{
// 		sprintf(szMsg, "%s님이 %s님으로부터 승리하였습니다.", szAttacker, szVictim );
		ZTransMsg( szMsg, MSG_GAME_WHO_WIN_FROM_OTHER, 2, szAttacker, szVictim );
		ZChatOutput(MCOLOR(0xFF707070), szMsg);
		
		// Admin Grade
		if (ZGetMyInfo()->IsAdminGrade()) {
			MMatchObjCache* pCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
			if (pCache && pCache->CheckFlag(MTD_PlayerFlags_AdminHide))
			{
				sprintf( szMsg, "^%d%s^9 승리,  ^%d%s^9 패배",
							(pAttacker->GetTeamID() == MMT_BLUE) ? 3 : 1, pAttacker->GetProperty()->szName,
							(pVictim->GetTeamID() == MMT_BLUE) ? 3 : 1,   pVictim->GetProperty()->szName);
				ZGetGameInterface()->GetCombatInterface()->m_AdminMsg.OutputChatMsg( szMsg);
			}
		}
	}
}


void ZGame::OnReloadComplete(ZCharacter *pCharacter)
{
	ZItem* pItem = pCharacter->GetItems()->GetSelectedWeapon();
	//// 루프중 MEMORYHACK있었나 검사
	if (pCharacter->GetUID() == ZGetMyUID() && pItem!=NULL) {
		MDataChecker* pChecker = ZApplication::GetGame()->GetDataChecker();
		MDataCheckNode* pCheckNodeA = pChecker->FindCheck((BYTE*)pItem->GetBulletPointer());
		MDataCheckNode* pCheckNodeB = pChecker->FindCheck((BYTE*)pItem->GetAMagazinePointer());
		if ( (pCheckNodeA && (pCheckNodeA->UpdateChecksum()==false)) ||
		 	 (pCheckNodeB && (pCheckNodeB->UpdateChecksum()==false)) ) 
		{
			pChecker->BringError();	//// MEMORYHACK 감지. Checksum 모두중단하고 끝장낸다.
		} else {
			bool bResult = pCharacter->GetItems()->Reload();	//// RELOAD ////

			//// MEMORYHACK 없었으면 리뉴한다.
			pChecker->RenewCheck((BYTE*)pItem->GetBulletPointer(), sizeof(int));
			pChecker->RenewCheck((BYTE*)pItem->GetAMagazinePointer(), sizeof(int));
		}
	} else {
		bool bResult = pCharacter->GetItems()->Reload();	//// RELOAD ////
	}

	if(pCharacter==m_pMyCharacter) {
		ZApplication::GetSoundEngine()->PlaySound("we_weapon_rdy");
	}
	return;
}

void ZGame::OnPeerSpMotion(MUID& uid,int nMotionType)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);

	if (pCharacter == NULL) return;

	pCharacter->m_bSpMotion = true;

	ZC_STATE_LOWER zsl = ZC_STATE_TAUNT;

		 if(nMotionType == ZC_SPMOTION_TAUNT) zsl = ZC_STATE_TAUNT;
	else if(nMotionType == ZC_SPMOTION_BOW  ) zsl = ZC_STATE_BOW;
	else if(nMotionType == ZC_SPMOTION_WAVE ) zsl = ZC_STATE_WAVE;
	else if(nMotionType == ZC_SPMOTION_LAUGH) zsl = ZC_STATE_LAUGH;
	else if(nMotionType == ZC_SPMOTION_CRY  ) zsl = ZC_STATE_CRY;
	else if(nMotionType == ZC_SPMOTION_DANCE) zsl = ZC_STATE_DANCE;

	pCharacter->m_SpMotion = zsl;

	pCharacter->SetAnimationLower( zsl );
}

void ZGame::OnPeerReload(MUID& uid)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
	//	if (uid == ZGetGameClient()->GetUID()) pCharacter = m_pMyCharacter;

	if (pCharacter == NULL || pCharacter->IsDie() ) return;

	// 임시.. 뭔가 상체애니메이션이 있으면 캔슬.. 리로드/샷 따위
	//	if(pCharacter->m_pVMesh->m_pAniSet[ani_mode_upper]!=NULL) return;

	// 내 캐릭터는 애니메이션이 끝날때에 실제로 리로드 시킨다.
	if(pCharacter==m_pMyCharacter)
		m_pMyCharacter->Animation_Reload();
	else
		OnReloadComplete(pCharacter);

	// Sound Effect
	if(pCharacter->GetItems()->GetSelectedWeapon()!=NULL) {
		rvector p = pCharacter->m_Position+rvector(0,0,160.f);
		ZApplication::GetSoundEngine()->PlaySEReload(pCharacter->GetItems()->GetSelectedWeapon()->GetDesc(), p.x, p.y, p.z, (pCharacter==m_pMyCharacter));
	}
}

void ZGame::OnPeerChangeCharacter(MUID& uid)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);

	//	if (uid == ZGetGameClient()->GetUID()) pCharacter = m_pMyCharacter;

	if (pCharacter == NULL) return;

	pCharacter->TestToggleCharacter();
}

/*
void ZGame::OnAssignCommander(const MUID& uidRedCommander, const MUID& uidBlueCommander)
{
	AssignCommander(uidRedCommander, uidBlueCommander);
}

void ZGame::AssignCommander(const MUID& uidRedCommander, const MUID& uidBlueCommander)
{
	ZCharacter* pRedChar = m_CharacterManager.Find(uidRedCommander);
	ZCharacter* pBlueChar = m_CharacterManager.Find(uidBlueCommander);

	if(pRedChar) {
		ZGetEffectManager()->AddCommanderIcon(pRedChar,0);
		pRedChar->m_bCommander = true;
	}
	if(pBlueChar) {
		ZGetEffectManager()->AddCommanderIcon(pBlueChar,1);
		pBlueChar->m_bCommander = true;
	}

#ifdef _DEBUG
	//// DEBUG LOG ////
	const char *szUnknown = "unknown";
	char szBuf[128];
	sprintf(szBuf, "RedCMDER=%s , BlueCMDER=%s \n", 
		pRedChar ? pRedChar->GetProperty()->szName : szUnknown , 
		pBlueChar ? pBlueChar->GetProperty()->szName : szUnknown );
	OutputDebugString(szBuf);
	///////////////////
#endif
}
*/
void ZGame::OnSetObserver(MUID& uid)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
	if (pCharacter == NULL) return;

	if(pCharacter==m_pMyCharacter)
	{
		ZApplication::GetGameInterface()->GetCombatInterface()->SetObserverMode(true);
	}
	pCharacter->SetVisible(false);
	pCharacter->ForceDie();
}

void ZGame::OnPeerSpawn(MUID& uid, rvector& pos, rvector& dir)
{
	m_nSpawnTime = timeGetTime();
	SetSpawnRequested(false);

	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
	if (pCharacter == NULL) return;

	bool isRespawn	= ( pCharacter->IsDie() == true ) ? true : false;

	//	dir = rvector(-1.f,0,0);
	pCharacter->SetVisible(true);
	pCharacter->Revival();
	pCharacter->SetPosition(pos);
	pCharacter->SetDirection(dir);
	pCharacter->SetSpawnTime(GetTime());

	ZGetEffectManager()->AddReBirthEffect(pos);

	if(pCharacter==m_pMyCharacter)
	{
		m_pMyCharacter->InitSpawn();

		if( isRespawn )
		{
			ZGetSoundEngine()->PlaySound("fx_respawn");
		}
		else
		{
			ZGetSoundEngine()->PlaySound("fx_whoosh02");
		}

		ZGetScreenEffectManager()->ReSetHpPanel();// hppanel_ani
	}

#ifndef _PUBLISH
	char szLog[128];
	sprintf(szLog, "ZGame::OnPeerSpawn() - %s(%u) Spawned \n", 
		pCharacter->GetProperty()->szName, pCharacter->GetUID().Low);
	OutputDebugString(szLog);
#endif

	if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DEATHMATCH_TEAM2)
		pCharacter->SetInvincibleTime( 5000);
}

void ZGame::OnPeerDash(MCommand* pCommand)
{
	MCommandParameter* pParam = pCommand->GetParameter(0);
	if(pParam->GetType()!=MPT_BLOB) return;

	MUID uid = pCommand->GetSenderUID();
	ZPACKEDDASHINFO* ppdi= (ZPACKEDDASHINFO*)pParam->GetPointer();

	rvector pos, dir;
	int sel_type;
	

	pos = rvector(Roundf(ppdi->posx),Roundf(ppdi->posy),Roundf(ppdi->posz));
	dir = 1.f/32000.f * rvector(ppdi->dirx,ppdi->diry,ppdi->dirz);
	sel_type = (int)ppdi->seltype;


	ZCharacter* pCharacter = m_CharacterManager.Find(uid);

	if (pCharacter == NULL) return;

	MMatchCharItemParts parts = (MMatchCharItemParts)sel_type;

	if( parts != pCharacter->GetItems()->GetSelectedWeaponParts()) {
		// 지금 들고 있는 무기와 보내진 무기가 틀리다면 보내진 무기로 바꿔준다..
		OnChangeWeapon(uid,parts);
	}

	ZGetEffectManager()->AddDashEffect(pos,dir,pCharacter);

	/*
	// 내가 아닌 경우에 애니메이션이 씹히는 현상이 있으므로 다시 세팅해준다
	if(pCharacter!=m_pMyCharacter)
	{	
		// 프로토콜을 바꿀수 없으므로 이미 있는 방향으로 판단한다
		rvector charDir = pCharacter->GetDirection();
		rvector charRight;
		CrossProduct(&charRight,dir,rvector(0,0,1));

		float fDotMax = 0.f , fDot;

		int nTumbleDir = 0;
		fDot=DotProduct(dir,-charDir);
		if(fDot>fDotMax) {	nTumbleDir = 1; fDotMax = fDot; }
		fDot=DotProduct(dir,charRight);
		if(fDot>fDotMax) {	nTumbleDir = 2; fDotMax = fDot; }
		fDot=DotProduct(dir,-charRight);
		if(fDot>fDotMax) {	nTumbleDir = 3; fDotMax = fDot; }

		switch(nTumbleDir)
		{
		case 0 :pCharacter->SetAnimationLower(ZC_STATE_LOWER_TUMBLE_FORWARD);break;
		case 1 :pCharacter->SetAnimationLower(ZC_STATE_LOWER_TUMBLE_BACK);break;
		case 2 :pCharacter->SetAnimationLower(ZC_STATE_LOWER_TUMBLE_RIGHT);break;
		case 3 :pCharacter->SetAnimationLower(ZC_STATE_LOWER_TUMBLE_LEFT);break;
		}
	}
	*/

}


//#define CHAR_COLLISION_HEIGHT	170.f


rvector ZGame::GetFloor(rvector pos,rplane *pimpactplane)
{
	rvector floor=g_pGame->GetWorld()->GetBsp()->GetFloor(pos+rvector(0,0,120),CHARACTER_RADIUS-1.1f,58.f,pimpactplane);

#ifdef ENABLE_CHARACTER_COLLISION
	for (ZObjectManager::iterator itor = m_ObjectManager.begin();
		itor != m_ObjectManager.end(); ++itor)
	{
		ZObject* pObject = (*itor).second;
		if (pObject->IsCollideable())
//		if(!pCharacter->IsDie() && !pCharacter->m_bBlastDrop)
		{
			rvector diff=pObject->m_Position-pos;
			diff.z=0;

			// 나중에 radius상수값으로 된것 Object의 멤버변수로 고치자
			if(Magnitude(diff)<CHARACTER_RADIUS && pos.z>pObject->m_Position.z)
			{
				rvector newfloor=pObject->m_Position+rvector(0,0,pObject->GetCollHeight());
				if(floor.z<newfloor.z)
				{
					floor=newfloor;
					if(pimpactplane)
					{
						rvector up=rvector(0,0,1);
						D3DXPlaneFromPointNormal(pimpactplane,&floor,&up);
					}
				}
			}
		}
	}
#endif

	return floor;
}

/*
rvector ZGame::GetCeiling(rvector pos)
{
rvector ceiling=g_pGame->GetWorld()->GetBsp()->GetCeiling(pos+rvector(0,0,130),CHARACTER_RADIUS-0.1f);

#ifdef ENABLE_CHARACTER_COLLISION
for (ZCharacterManager::iterator itor = m_CharacterManager.begin();
itor != m_CharacterManager.end(); ++itor)
{
ZCharacter* pCharacter = (*itor).second;
if(pCharacter!=m_pMyCharacter && !pCharacter->IsDie() && !pCharacter->m_bBlastDrop)
{
rvector diff=pCharacter->m_Position-m_pMyCharacter->m_Position;
diff.z=0;

if(Magnitude(diff)<CHARACTER_RADIUS && pos.z+CHAR_COLLISION_HEIGHT<pCharacter->m_Position.z)
{
rvector newceiling=pCharacter->m_Position;
if(ceiling.z<newceiling.z)
ceiling=newceiling;
}
}
}
#endif

return ceiling;
}
*/

bool ZGame::Pick(ZObject *pOwnerObject,rvector &origin,rvector &dir,ZPICKINFO *pickinfo,DWORD dwPassFlag,bool bMyChar)
{
	return PickHistory(pOwnerObject,GetTime(),origin,origin+10000.f*dir,pickinfo,dwPassFlag,bMyChar);
}

bool ZGame::PickTo(ZObject *pOwnerObject,rvector &origin,rvector &to,ZPICKINFO *pickinfo,DWORD dwPassFlag,bool bMyChar)
{
	return PickHistory(pOwnerObject,GetTime(),origin,to,pickinfo,dwPassFlag,bMyChar);
}

// fTime 시간의 캐릭터 위치로 pick 한다.. 캐릭터는 실린더 판정.
bool ZGame::PickHistory(ZObject *pOwnerObject,float fTime,rvector &origin,rvector &to,ZPICKINFO *pickinfo,DWORD dwPassFlag,bool bMyChar)
{
	pickinfo->pObject=NULL;
	pickinfo->bBspPicked=false;

	RPickInfo info;
	memset(&info,0,sizeof(RPickInfo));

	ZObject *pObject=NULL;

	bool bCheck = false;

	float fCharacterDist=FLT_MAX;			// 캐릭터들 사이의 최소거리지점을 찾는다
	for(ZObjectManager::iterator i=m_ObjectManager.begin();i!=m_ObjectManager.end();i++)
	{
		ZObject *pc=i->second;

		bCheck = false;

		if(bMyChar) {
			if(pc==pOwnerObject && pc->IsVisible()) {
				bCheck = true;
			}
		}
		else {
			if( pc!=pOwnerObject && pc->IsVisible() ) {
				bCheck = true;
			}
		}

		if( pc->IsDie() )//죽은넘이 몸빵한다고 해서~
			bCheck = false;


		if(bCheck)
		{
			rvector hitPos;
			ZOBJECTHITTEST ht = pc->HitTest(origin,to,fTime,&hitPos);
			if(ht!=ZOH_NONE) {
				float fDistToChar=Magnitude(hitPos-origin);
				if(fDistToChar<fCharacterDist) {
					pObject=pc;
					fCharacterDist=fDistToChar;
					info.vOut=hitPos;								
					switch(ht) {
						case ZOH_HEAD : info.parts=eq_parts_head;break;
						case ZOH_BODY : info.parts=eq_parts_chest;break;
						case ZOH_LEGS :	info.parts=eq_parts_legs;break;
					}
				}
			}
		}
	}

	RBSPPICKINFO bpi;
	bool bBspPicked = GetWorld()->GetBsp()->PickTo(origin, to, &bpi, dwPassFlag);

	int nCase=0;

	if(pObject && bBspPicked)		// 둘다 맞았을때는 거리가 가까운쪽을 택한다.
	{
		if(Magnitude(info.vOut-origin)>Magnitude(bpi.PickPos-origin))
			nCase=1;
		else
			nCase=2;
	}else
		if(bBspPicked)				// 둘중 하나만 맞았으면 맞은걸 택하면 된다.
			nCase=1;
		else
			if(pObject)
				nCase=2;

	if(nCase==0) return false;

	switch(nCase)
	{
	case 1 :						// 맵에 맞은경우
		pickinfo->bBspPicked=true;
		pickinfo->bpi=bpi;
		break;
	case 2 :						// 사람에 맞은경우.
		pickinfo->pObject=pObject;
		pickinfo->info=info;
		break;
	}
	return true;
}

bool ZGame::ObjectColTest(ZObject* pOwner, rvector& origin, rvector& to, float fRadius, ZObject** poutTarget)
{
	// 맵에 맞는것은 체크하지 않는다.

	for(ZObjectManager::iterator i=m_ObjectManager.begin();i!=m_ObjectManager.end();i++)
	{
		ZObject *pc=i->second;

		bool bCheck = true;

		if(pc == pOwner || !pc->IsVisible())  bCheck = false;
		if( pc->IsDie() ) bCheck = false;

		if(bCheck)
		{
			if (pc->ColTest(origin, to, fRadius, GetTime()))
			{
				*poutTarget = pc;
				return true;
			}
		}
	}

	return false;
}

char* ZGame::GetSndNameFromBsp(const char* szSrcSndName, RMATERIAL* pMaterial)
{
	char szMaterial[256] = "";
	static char szRealSndName[256] = "";
	szRealSndName[0] = 0;

	if (pMaterial == NULL) return "";

	strcpy(szMaterial, pMaterial->Name.c_str());

	size_t nLen = strlen(szMaterial);

#define ZMETERIAL_SNDNAME_LEN 7

	if ((nLen > ZMETERIAL_SNDNAME_LEN) && 
		(!strnicmp(&szMaterial[nLen-ZMETERIAL_SNDNAME_LEN+1], "mt", 2)))
	{
		strcpy(szRealSndName, szSrcSndName);
		strcat(szRealSndName, "_");
		strcat(szRealSndName, &szMaterial[nLen-ZMETERIAL_SNDNAME_LEN+1]);
	}
	else
	{
		strcpy(szRealSndName, szSrcSndName);
	}


	return szRealSndName;
}

/*
void ZGame::AdjustGlobalTime()
{
// 카운트 다운할때만 시간을 싱크한다
//	if(GetMatch()->GetRoundState()!=MMATCH_ROUNDSTATE_COUNTDOWN) return;
static DWORD nLastTime = GetTickTime();
DWORD nNowTime = GetTickTime();
if((nNowTime - nLastTime) < 100) return;	// 100밀리세컨드마다 체크

nLastTime = nNowTime;

float fAverageTime=0.f;

int nValidCount=0;

ZCharacterManager::iterator i;
for(i=m_CharacterManager.begin();i!=m_CharacterManager.end();i++)
{
ZCharacter *pCharacter=i->second;
if(pCharacter->m_BasicHistory.size()==0) continue;		// 유효하지 않다

// 캐릭터가 마지막으로 보내온 정보를 참조
ZBasicInfoHistory::iterator infoi=pCharacter->m_BasicHistory.end();
infoi--;
ZBasicInfoItem *pInfo=*infoi;

// 마지막 데이터 받은지 3초 이상이면 문제가 있다고 판정. 유효하지 않다
if(GetTime()-pInfo->fReceivedTime > 3.f) continue;

float fCharacterTime=pInfo->info.fSendTime+(GetTime()-pInfo->fReceivedTime);

nValidCount++;
fAverageTime+=fCharacterTime;
}

fAverageTime/=(float)nValidCount;
fAverageTime=max(fAverageTime,0);	// 0보다 작을수는 없다.

// 글로벌 시간과의 차이를 누적한다.
for(i=m_CharacterManager.begin();i!=m_CharacterManager.end();i++)
{
ZCharacter *pCharacter=i->second;
if(pCharacter->m_BasicHistory.size()==0) continue;		// 유효하지 않다

ZBasicInfoHistory::iterator infoi=pCharacter->m_BasicHistory.end();
infoi--;
ZBasicInfoItem *pInfo=*infoi;
float fCharacterTime=pInfo->info.fSendTime+(GetTime()-pInfo->fReceivedTime);

pCharacter->m_TimeErrors[pCharacter->m_nTimeErrorCount++]=fAverageTime-fCharacterTime;
if( TIME_ERROR_CORRECTION_PERIOD == pCharacter->m_nTimeErrorCount )
{
pCharacter->m_nTimeErrorCount=0;
float fAvrTimeError=0;
for(int j=0;j<TIME_ERROR_CORRECTION_PERIOD;j++)
fAvrTimeError+=pCharacter->m_TimeErrors[j];
fAvrTimeError/=(float)TIME_ERROR_CORRECTION_PERIOD;

pCharacter->m_fAccumulatedTimeError+=fAvrTimeError*.5f;
if(fabs(pCharacter->m_fAccumulatedTimeError)>10.f)
{
#ifndef _PUBLISH
char szTemp[256];
sprintf(szTemp, "%s님이 스피드핵 ? %3.1f", pCharacter->GetProperty()->szName,pCharacter->m_fAccumulatedTimeError);
ZGetGameInterface()->OutputChatMsg(MCOLOR(0xFFFF0000), szTemp);
#endif

pCharacter->m_fAccumulatedTimeError=0;
}

if(pCharacter==m_pMyCharacter)
{
m_fTime+=fAvrTimeError*.5f;
}
}
}

}
*/

// 취소

void ZGame::AutoAiming()
{
#ifdef _PUBLISH
	return;
#endif
/*
	ZCamera* pCamera = ZGetGameInterface()->GetCamera();

	rvector vMyPos = m_pMyCharacter->GetPosition();

	//거리 계산하고 중간에 벽하고 충돌하는 캐릭인가 계산하고..

	if(ZGetMyInfo()==NULL) 
	return;

	//	개발자나 관리자라면..

	//	MMatchUserGradeID gid = MMUG_FREE;
	//	gid = ZGetMyInfo()->GetUGradeID();
	//	if((gid == MMUG_DEVELOPER)||(gid==MMUG_ADMIN))

	//	우선 테스트 모드에서 알지5만..

	if( strcmp(g_pGame->m_pMyCharacter->GetProperty()->szName,"알지5")!=0 )
		return;

	ZCharacter *pCharacter = NULL;
	rvector pos;
	rvector dir;

	// 주변의 캐릭터 중 가까우면서 맵이 중간에 있지 않은 경우..나중에 코드 부활시..처리..

	for(ZCharacterManager::iterator i = m_CharacterManager.begin();i != m_CharacterManager.end();i++) 
	{
		pCharacter = i->second;

		if(pCharacter != m_pMyCharacter) {

			if(pCharacter->IsDie()==false) {
				pos = pCharacter->GetPosition();
				pos.z += 140.f;
				dir = pos - RCameraPosition;
				Normalize(dir);
				pCamera->SetDirection(dir);
			}
			else {
				pCamera->m_bAutoAiming = false;
			}
		}
	}
*/
}

/* 
실제 투표를 행하는 곳이다. 만약 내가 남의 데이터와 비교해서 많이 다른 데이터를 가지고 있으면
나의 데이터를 보정해야 한다.

*/

/*
void ZGame::AdjustMyData()
{
	for(ZCharacterManager::iterator i=m_CharacterManager.begin();i!=m_CharacterManager.end();i++)
	{
		ZCharacter *pCharacter=i->second;
		pCharacter->m_fGlobalHP=0.f;
		pCharacter->m_nReceiveHPCount=0;
	}

	for(i=m_CharacterManager.begin();i!=m_CharacterManager.end();i++)
	{
		ZCharacter *pCharacter=i->second;
		if(pCharacter->m_HPHistory.size()==0) return;	// 한넘이라도 데이터가 없으면 좆치않다.

		// 가장 최근의 데이터를 선택한다
		ZHPInfoHistory::iterator hpi=pCharacter->m_HPHistory.end();
		hpi--;
		ZHPInfoItem *pItem=*hpi;

		for(int j=0;j<pItem->nCount;j++)
		{
			MUID uid=pItem->pHPTable[j].muid;
			ZCharacter *pchar=m_CharacterManager.Find(uid);
			if(pchar)
			{
				pchar->m_fGlobalHP+=pItem->pHPTable[j].fHP;
				pchar->m_nReceiveHPCount++;
			}
		}
	}

	for(i=m_CharacterManager.begin();i!=m_CharacterManager.end();i++)
	{
		ZCharacter *pCharacter=i->second;
		// spawn 된 직후에는 투표 보류
		if(pCharacter->m_nReceiveHPCount && (GetTime()-pCharacter->GetSpawnTime())>3.f)
		{
			pCharacter->m_fGlobalHP/=(float)pCharacter->m_nReceiveHPCount;

			if(pCharacter==m_pMyCharacter)	// 내 캐릭터는 피를 올리지는 않는다.
			{
				pCharacter->SetHP(
					min(pCharacter->GetStatus()->GetHP(),
					(pCharacter->GetStatus()->GetHP()+pCharacter->m_fGlobalHP)/2));
			}else							// 넘의 캐릭터는 평균값에 근접하도록 간다.
			{
				pCharacter->SetHP((pCharacter->GetStatus()->GetHP()+pCharacter->m_fGlobalHP)/2);
			}
		}
	}
}
*/

#define MAX_PLAYERS		64


// 투표는 제거 되었으므로 내 피정보만 보낸다
void ZGame::PostHPAPInfo()
{
	DWORD nNowTime = GetTickTime();

	if (m_pMyCharacter->GetInitialized() == false) return;

	if ((nNowTime - m_nLastTime[ZLASTTIME_HPINFO]) >= PEER_HP_TICK)
	{
		m_nLastTime[ZLASTTIME_HPINFO] = nNowTime;

		ZPostHPAPInfo(m_pMyCharacter->GetHP(), m_pMyCharacter->GetAP());
	}

#ifdef ENABLE_ADJUST_MY_DATA
	//	AdjustMyData();
#endif
}

// 투표는 제거 되었으므로 내 피정보만 보낸다
void ZGame::PostHPInfo()
{
	DWORD nNowTime = GetTickTime();

	if (m_pMyCharacter->GetInitialized() == false) return;

	if ((nNowTime - m_nLastTime[ZLASTTIME_HPINFO]) >= PEER_HP_TICK)
	{
		m_nLastTime[ZLASTTIME_HPINFO] = nNowTime;

		ZPostHPInfo(m_pMyCharacter->GetHP());
	}

#ifdef ENABLE_ADJUST_MY_DATA
	//	AdjustMyData();
#endif
}

void ZGame::PostBasicInfo()
{
	DWORD nNowTime = timeGetTime();

	if (m_pMyCharacter->GetInitialized() == false) return;

	// 죽고나서 5초가 지나면 basicinfo를 보내지 않는다.
	if(m_pMyCharacter->IsDie() && GetTime()-m_pMyCharacter->m_fDeadTime>5.f) return;

	// 난입한 직후에도 보내지 않는다 ( global time 이 세팅되지 않았기 때문에 )
	//	if(m_bForceEntry) return;

	int nMoveTick = (ZGetGameClient()->GetAllowTunneling() == false) ? PEERMOVE_TICK : PEERMOVE_AGENT_TICK;

	if ((int)(nNowTime - m_nLastTime[ZLASTTIME_BASICINFO]) >= nMoveTick)
	{
		m_nLastTime[ZLASTTIME_BASICINFO] = nNowTime;

		ZPACKEDBASICINFO pbi;
		pbi.fTime = GetTime();

		pbi.posx = m_pMyCharacter->m_Position.x;
		pbi.posy = m_pMyCharacter->m_Position.y;
		pbi.posz = m_pMyCharacter->m_Position.z;

		pbi.velx = m_pMyCharacter->GetVelocity().x;
		pbi.vely = m_pMyCharacter->GetVelocity().y;
		pbi.velz = m_pMyCharacter->GetVelocity().z;

		pbi.dirx = m_pMyCharacter->m_TargetDir.x*32000;
		pbi.diry = m_pMyCharacter->m_TargetDir.y*32000;
		pbi.dirz = m_pMyCharacter->m_TargetDir.z*32000;

		pbi.upperstate = m_pMyCharacter->GetStateUpper();
		pbi.lowerstate = m_pMyCharacter->GetStateLower();
		pbi.selweapon = m_pMyCharacter->GetItems()->GetSelectedWeaponParts();


		ZPOSTCMD1(MC_PEER_BASICINFO,MCommandParameterBlob(&pbi,sizeof(ZPACKEDBASICINFO)));
	}
}

void ZGame::PostPeerPingInfo()
{
	if (!ZGetGameInterface()->GetCombatInterface()->IsShowScoreBoard()) return;

	DWORD nNowTime = GetTickTime();

	if ((nNowTime - m_nLastTime[ZLASTTIME_PEERPINGINFO]) >= PEER_PING_TICK) {
		m_nLastTime[ZLASTTIME_PEERPINGINFO] = nNowTime;

		unsigned long nTimeStamp = GetTickTime();
		MMatchPeerInfoList* pPeers = ZGetGameClient()->GetPeers();
		for (MMatchPeerInfoList::iterator itor = pPeers->begin(); itor != pPeers->end(); ++itor) {
			MMatchPeerInfo* pPeerInfo = (*itor).second;
			if (pPeerInfo->uidChar != ZGetGameClient()->GetPlayerUID()) {
				_ASSERT(pPeerInfo->uidChar != MUID(0,0));

				MCommandManager* MCmdMgr = ZGetGameClient()->GetCommandManager();
				MCommand* pCmd = new MCommand(MCmdMgr->GetCommandDescByID(MC_PEER_PING), 
					pPeerInfo->uidChar, ZGetGameClient()->GetUID());	
				pCmd->AddParameter(new MCmdParamUInt(nTimeStamp));
				ZGetGameClient()->Post(pCmd);

#ifdef _DEBUG
				g_nPingCount++;
#endif
				pPeerInfo->SetLastPingTime(nTimeStamp);
			}
		}
	}
}

void ZGame::PostSyncReport()
{
	DWORD nNowTime = GetTickTime();

#ifdef _PUBLISH
	if ((nNowTime - m_nLastTime[ZLASTTIME_SYNC_REPORT]) >= MATCH_CYCLE_CHECK_SPEEDHACK) {
#else
	if ((nNowTime - m_nLastTime[ZLASTTIME_SYNC_REPORT]) >= 1000/*MATCH_CYCLE_CHECK_SPEEDHACK*/) {
#endif
		m_nLastTime[ZLASTTIME_SYNC_REPORT] = nNowTime;
		int nDataChecksum = 0;
		if (m_DataChecker.UpdateChecksum() == false) {
			nDataChecksum = m_DataChecker.GetChecksum();
			ZGetApplication()->Exit();
		}
/* 2006년 6월 27일 삭제 - 필요 없다고 판단됨
		if (ZCheckHackProcess() == true) {
			nDataChecksum = 4444;
			ZPostLocalMessage(MSG_HACKING_DETECTED);
		}
*/
		ZPOSTCMD2(MC_MATCH_GAME_REPORT_TIMESYNC, MCmdParamUInt(nNowTime), MCmdParamUInt(nDataChecksum));
	}
}

// pOwner / pTarget = 쏜캐릭터 / 맞은 캐릭터
void ZGame::CheckCombo( ZCharacter *pOwnerCharacter , ZObject *pHitObject,bool bPlaySound)
{
	// 자기가 자기 맞춘건 체크하지 않음
	if(pOwnerCharacter==pHitObject) return;

	// 내 캐릭터 혹은 내가 보고있는 캐릭터
	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if(!pTargetCharacter) return;

	if(pTargetCharacter!=pOwnerCharacter) return;	// 내가 보고있는 캐릭터가 아니면 체크하지 않음

	if(pHitObject)	// NULL 이면 무조건 올린다
	{
		if(pHitObject->IsDie()) return;		// 시체를 쏴도 체크되지 않음.
	}	

	if (IsPlayerObject(pHitObject))
	{
		// 우리편 때린것은 combo 에 포함되지 않음
		if(m_Match.IsTeamPlay() && (pTargetCharacter->GetTeamID()==((ZCharacter*)(pHitObject))->GetTeamID()))
			return;

		// 퀘스트일때도 우리편은 포함하지 않는다.
		if (m_Match.IsQuestDrived()) return;
	}

	UpdateCombo(true);

	// npc는 hit 소리를 안나도록 한다.
	if (Z_AUDIO_HITSOUND )
	{
//		bool bHitObjectIsNPC = ((pHitObject) && (pHitObject->IsNPC()));

//		if (!bHitObjectIsNPC)
//		{
#ifdef _BIRDSOUND
		ZGetSoundEngine()->PlaySound("fx_myhit", 128);
#else
		if(bPlaySound)
			if( ZGetSoundEngine()->Get3DSoundUpdate() )
				ZGetSoundEngine()->PlaySound("fx_myhit");
#endif
//		}
	}
}

void ZGame::UpdateCombo(bool bShot)
{
	// 내 캐릭터 혹은 내가 보고있는 캐릭터
	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if(!pTargetCharacter) return;

	// test
	static DWORD nLastShotTime = timeGetTime();
	DWORD nNowTime = timeGetTime();

	if (bShot) 
	{
		if(pTargetCharacter->GetStatus()->nCombo<2) {
			// hit 이펙트
			ZGetScreenEffectManager()->AddHit();
		}

		if ((nNowTime - nLastShotTime) < 700)
		{
			pTargetCharacter->GetStatus()->nCombo++;
			if (pTargetCharacter->GetStatus()->nCombo > MAX_COMBO) 
				pTargetCharacter->GetStatus()->nCombo = 1;
		}
		nLastShotTime = nNowTime;
	}
	else
	{
		if ((pTargetCharacter->GetStatus()->nCombo > 0) && ((nNowTime - nLastShotTime) > 1000))
		{
			pTargetCharacter->GetStatus()->nCombo = 0;
		}
	}
}


void ZGame::CheckStylishAction(ZCharacter* pCharacter)
{
	if (pCharacter->GetStylishShoted())
	{
		if (pCharacter == m_pMyCharacter)
		{
			ZGetScreenEffectManager()->AddCool();
		}

		// 점수 계산 넣어줘야한다.
	}
}

#define RESERVED_OBSERVER_TIME	5000

void ZGame::OnReserveObserver()
{
	unsigned long int currentTime = timeGetTime();

	if (currentTime - m_nReservedObserverTime > RESERVED_OBSERVER_TIME)
	{

		if ((m_Match.GetRoundState() == MMATCH_ROUNDSTATE_PLAY) ||
			(m_Match.IsWaitForRoundEnd() && ZGetGameClient()->IsForcedEntry())
			)
		{
			ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);
			m_bReserveObserver = false;
		}
		else
		{
			m_bReserveObserver = false;
		}

	}
}

void ZGame::ReserveObserver()
{
	m_bReserveObserver = true;
	m_nReservedObserverTime = timeGetTime();
}

void ZGame::ReleaseObserver()
{
	if(!m_bReplaying)
	{
		m_bReserveObserver = false;
		ZGetGameInterface()->GetCombatInterface()->SetObserverMode(false);

		FlushObserverCommands();
	}
}

void ZGame::OnInvalidate()
{
	GetWorld()->OnInvalidate();
	ZGetFlashBangEffect()->OnInvalidate();
	m_CharacterManager.OnInvalidate();
}

void ZGame::OnRestore()
{
	GetWorld()->OnRestore();
	ZGetFlashBangEffect()->OnRestore();
	m_CharacterManager.OnRestore();
}

void ZGame::InitRound()
{
//	m_fTime=0;
	SetSpawnRequested(false);
	ZGetGameInterface()->GetCamera()->StopShock();

	ZGetFlashBangEffect()->End();

	ZGetEffectManager()->Clear();
	m_WeaponManager.Clear();

#ifdef _WORLD_ITEM_
	//ZGetWorldItemManager()->Reset();
#endif

	ZGetCharacterManager()->InitRound();
}

void ZGame::AddEffectRoundState(MMATCH_ROUNDSTATE nRoundState, int nArg)
{

	switch(nRoundState) 
	{

	case MMATCH_ROUNDSTATE_COUNTDOWN : 
		{
			// 이팩트 출력 - 팀플일 경우만 RoundStart이팩트 출력한다. 듀얼은 라운드 기다리지만 제외.
			if (m_Match.IsWaitForRoundEnd() && m_Match.GetMatchType() != MMATCH_GAMETYPE_DUEL)
			{
				if(m_Match.GetCurrRound()+1==m_Match.GetRoundCount())
				{
					ZGetScreenEffectManager()->AddFinalRoundStart();
				}
				else
				{
					// m_nCurrRound 가 0이면 1라운드이다.
					ZGetScreenEffectManager()->AddRoundStart(m_Match.GetCurrRound()+1);
				}
			}
		}
		break;
	case MMATCH_ROUNDSTATE_PLAY:
		{
			ZGetScreenEffectManager()->AddRock();
		}
		break;
	case MMATCH_ROUNDSTATE_FINISH:
		{
			if (m_Match.IsTeamPlay())
			{
				int nRedTeam, nBlueTeam;
				m_Match.GetTeamAliveCount(&nRedTeam, &nBlueTeam);

				if (nArg == MMATCH_ROUNDRESULT_DRAW)
				{
					if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DEATHMATCH_TEAM2)
					{
						MMatchTeam nMyTeam = (MMatchTeam)m_pMyCharacter->GetTeamID();
						MMatchTeam nEnemyTeam = (nMyTeam == MMT_BLUE ? MMT_RED : MMT_BLUE);

						int nMyScore = GetMatch()->GetTeamKills(nMyTeam);
						int nEnemyScore = GetMatch()->GetTeamKills(nEnemyTeam);

						if (nMyScore > nEnemyScore)
							ZGetScreenEffectManager()->AddWin();
						else if (nMyScore < nEnemyScore)
							ZGetScreenEffectManager()->AddLose();
						else
							ZGetScreenEffectManager()->AddDraw();
					}
					else
						ZGetScreenEffectManager()->AddDraw();
				}
				else 
				{
					if (nArg == MMATCH_ROUNDRESULT_DRAW) {
						// Do nothing
					} else {
						MMatchTeam nMyTeam = (MMatchTeam)m_pMyCharacter->GetTeamID();
						MMatchTeam nTeamWon = (nArg == MMATCH_ROUNDRESULT_REDWON ? MMT_RED : MMT_BLUE);

						// 만약 강제로 팀이 바껴진 경우에는 반대편
						if (ZGetMyInfo()->GetGameInfo()->bForcedChangeTeam)
						{
							nMyTeam = NegativeTeam(nMyTeam);
						}

						// Spectator 일경우 처리
						if (ZGetGameInterface()->GetCombatInterface()->GetObserver()->IsVisible()) {
							ZCharacter* pTarget = ZGetGameInterface()->GetCombatInterface()->GetObserver()->GetTargetCharacter();
							if (pTarget)
								nMyTeam = (MMatchTeam)pTarget->GetTeamID();
						}

						if (nTeamWon == nMyTeam)
						{
							ZGetScreenEffectManager()->AddWin();
						} else {
							ZGetScreenEffectManager()->AddLose();
						}
					}
				}

				int nTeam = 0;

				// all kill 판정
				for(int j=0;j<2;j++)
				{
					bool bAllKill=true;
					ZCharacter *pAllKillPlayer=NULL;

					for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();
						itor != ZGetCharacterManager()->end(); ++itor)
					{
						ZCharacter* pCharacter = (*itor).second;
						if (pCharacter == NULL) return;
						
						if(j==0) {	
							nTeam = MMT_RED;
						} 
						else if(j==1) { 
							nTeam = MMT_BLUE;	
						}	

						if(pCharacter->GetTeamID() != nTeam) 
							continue;

						if(pCharacter->IsDie())
						{
							ZCharacter *pKiller = ZGetCharacterManager()->Find(pCharacter->GetLastAttacker());
							if(pAllKillPlayer==NULL)
							{
								if(!pKiller || pKiller->GetTeamID()==nTeam)			// 같은편한테 죽었으면 꽝
								{
									bAllKill=false;
									break;
								}

								pAllKillPlayer=pKiller;
							}
							else
								if(pAllKillPlayer!=pKiller)	// 여러명이 나눠 죽였으면 꽝
								{
									bAllKill=false;
									break;
								}
						}else
						{
							bAllKill=false;											// 살아있는 넘이 있어도 꽝
							break;
						}
					}

					if((bAllKill) && (pAllKillPlayer))
					{
						pAllKillPlayer->GetStatus()->nAllKill++;
						pAllKillPlayer->AddIcon(ZCI_ALLKILL);
					}
				}
			}


			// 듀얼 모드일 경우
			else if ( ZApplication::GetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
			{
				ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
				if ( pDuel)
				{
					bool bAddWin = false;
					bool bAddLose = false;
					int nCount = 0;				// 챔피언과 도전자 모두 게임중이였는지 체크하기 위해서...


					// 옵져버 모드일때
					MUID uidTarget;
					ZObserver* pObserver = ZGetGameInterface()->GetCombatInterface()->GetObserver();
					if ( pObserver && pObserver->IsVisible())
						uidTarget = pObserver->GetTargetCharacter()->GetUID();

					// 옵져버 모드가 아닐때
					else
						uidTarget = m_pMyCharacter->GetUID();


					for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin(); itor != ZGetCharacterManager()->end(); ++itor)
					{
						ZCharacter* pCharacter = (*itor).second;

						// Is champion or challenger
						if ( (pCharacter->GetUID() == pDuel->QInfo.m_uidChampion) || (pCharacter->GetUID() == pDuel->QInfo.m_uidChallenger))
						{
							if ( uidTarget == pCharacter->GetUID())
							{
								if ( pCharacter->IsDie())
									bAddLose |= true;
								else
									bAddWin |= true;
							}
							else
							{
								if ( pCharacter->IsDie())
									bAddWin |= true;
								else
									bAddLose |= true;
							}


							// 챔피온과 도전자 수를 모두 더해서 2가 되어야 한다
							nCount++;
						}
					}


					// Draw
					if ( (nCount < 2) || (bAddWin == bAddLose))
						ZGetScreenEffectManager()->AddDraw();
					
					// Win
					else if ( bAddWin)
						ZGetScreenEffectManager()->AddWin();

					// Lose
					else
						ZGetScreenEffectManager()->AddLose();
				}
			}
		}
		break;
	};

}

void ZGame::StartRecording()
{
	int nsscount=0;

	char replayfilename[_MAX_PATH];
	char replayfoldername[_MAX_PATH];

	TCHAR szPath[MAX_PATH];
	if(GetMyDocumentsPath(szPath)) {
		strcpy(replayfoldername,szPath);
		strcat(replayfoldername,GUNZ_FOLDER);
		CreatePath( replayfoldername );
		strcat(replayfoldername,REPLAY_FOLDER);
		CreatePath( replayfoldername );
	}

	do {
		sprintf(replayfilename,"%s/Gunz%03d."GUNZ_REC_FILE_EXT , replayfoldername , nsscount);
		m_nGunzReplayNumber = nsscount;
		nsscount++;
	}
	while( IsExist(replayfilename) && nsscount<1000);

	if(nsscount==1000) goto RECORDING_FAIL;

	m_pReplayFile = zfopen(replayfilename,true);
	if(!m_pReplayFile) goto RECORDING_FAIL;

	int nWritten;

	DWORD header;
	header=GUNZ_REC_FILE_ID;
	nWritten = zfwrite(&header,sizeof(header),1,m_pReplayFile);
	if(nWritten==0) goto RECORDING_FAIL;

	header=GUNZ_REC_FILE_VERSION;
	nWritten = zfwrite(&header,sizeof(header),1,m_pReplayFile);
	if(nWritten==0) goto RECORDING_FAIL;

	nWritten = zfwrite(ZGetGameClient()->GetMatchStageSetting()->GetStageSetting(),sizeof(MSTAGE_SETTING_NODE),1,m_pReplayFile);
	if(nWritten==0) goto RECORDING_FAIL;

	// duel 모드일때 상태 추가 저장
	if(ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUEL)
	{
		ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		nWritten = zfwrite(&pDuel->QInfo,sizeof(MTD_DuelQueueInfo),1,m_pReplayFile);
		if(nWritten==0) goto RECORDING_FAIL;
	}

	int nCharacterCount= (int)m_CharacterManager.size();
	nWritten = zfwrite(&nCharacterCount,sizeof(nCharacterCount),1,m_pReplayFile);
	if(nWritten==0) goto RECORDING_FAIL;

	for (ZCharacterManager::iterator itor = m_CharacterManager.begin(); itor != m_CharacterManager.end(); ++itor)
	{
		ZCharacter* pCharacter = (*itor).second;
		if(!pCharacter->Save(m_pReplayFile)) goto RECORDING_FAIL;
	}	

	nWritten = zfwrite(&m_fTime,sizeof(m_fTime),1,m_pReplayFile);
	if(nWritten==0) goto RECORDING_FAIL;


	m_bRecording=true;
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), 
		ZMsg(MSG_RECORD_STARTING));
	return;

RECORDING_FAIL:	// 실패

	if(m_pReplayFile)
	{
		zfclose(m_pReplayFile);
		m_pReplayFile = NULL;
	}

	ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), ZMsg(MSG_RECORD_CANT_SAVE));
}

void ZGame::StopRecording()
{
	if(!m_bRecording) return;

	bool bError = false;

	m_bRecording=false;

	ZObserverCommandList::iterator itr = m_ReplayCommandList.begin();
	for(size_t i=0;i<m_ReplayCommandList.size();i++)
	{
		ZObserverCommandItem *pItem = *itr;
		MCommand *pCommand = pItem->pCommand;

		const BUF_SIZE = 1024;
		char CommandBuffer[BUF_SIZE];
		int nSize = pCommand->GetData(CommandBuffer, BUF_SIZE);

		int nWritten;
		nWritten = zfwrite(&pItem->fTime,sizeof(pItem->fTime),1,m_pReplayFile);
		if(nWritten==0) { bError=true; break; }
		nWritten = zfwrite(&pCommand->m_Sender,sizeof(pCommand->m_Sender),1,m_pReplayFile);
		if(nWritten==0) { bError=true; break; }
		nWritten = zfwrite(&nSize,sizeof(nSize),1,m_pReplayFile);
		if(nWritten==0) { bError=true; break; }
		nWritten = zfwrite(CommandBuffer,nSize,1,m_pReplayFile);
		if(nWritten==0) { bError=true; break; }

		itr++;
	}

	while(m_ReplayCommandList.size())
	{
		ZObserverCommandItem *pItem = *m_ReplayCommandList.begin();
		delete pItem->pCommand;
		delete pItem;
		m_ReplayCommandList.pop_front();
	}

	if(!zfclose(m_pReplayFile))
		bError = true;

	if(bError)
	{
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), ZMsg(MSG_RECORD_CANT_SAVE));
	}
	else
	{
		char szOutputFilename[256];
		sprintf(szOutputFilename,GUNZ_FOLDER REPLAY_FOLDER"/Gunz%03d."GUNZ_REC_FILE_EXT , m_nGunzReplayNumber );

		char szOutput[256];
		// ZTranslateMessage(szOutput,MSG_RECORD_SAVED,1,szOutputFilename);
		ZTransMsg(szOutput,MSG_RECORD_SAVED,1,szOutputFilename);
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szOutput);
	}

}



void ZGame::ToggleRecording()
{
	if(m_bReplaying) return;	// 재생중 녹화불가 -_-;

	// 퀘스트는 녹화되지 않는다
	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
		return;

	if(!m_bRecording)
		StartRecording();
	else
		StopRecording();
}

DWORD dwReplayStartTime;

bool ZGame::OnLoadReplay(ZReplayLoader* pLoader)
{
	m_fTime = pLoader->GetGameTime();

	m_bReplaying = true;
	SetReadyState(ZGAME_READYSTATE_RUN);
	GetMatch()->SetRoundState(MMATCH_ROUNDSTATE_FREE);
	ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);
	ZGetGameInterface()->GetCombatInterface()->GetObserver()->SetTarget(g_pGame->m_pMyCharacter->GetUID());
	g_bProfile=true;	
	dwReplayStartTime=timeGetTime();





/*
	size_t n;

	m_bReplaying=true;

	int nCharacterCount;
	zfread(&nCharacterCount,sizeof(nCharacterCount),1,file);

	ZGetCharacterManager()->Clear();
	m_ObjectManager.Clear();

	for(int i=0;i<nCharacterCount;i++)
	{
		bool bHero;
		n=zfread(&bHero,sizeof(bHero),1,file);
		if(n!=1) return false;

		MTD_CharInfo info;

		if(nVersion<2) {
			n=zfread(&info,sizeof(info)-4,1,file);
			if(n!=1) return false;
			info.nClanCLID = 0;
		}
		else {
			n=zfread(&info,sizeof(info),1,file);
			if(n!=1) return false;
		}

		ZCharacter *pChar=NULL;
		if(bHero)
		{
			m_pMyCharacter=new ZMyCharacter;
			CreateMyCharacter(&info);
			pChar=m_pMyCharacter;
			pChar->Load(file,nVersion);
		}else
		{
			pChar=new ZNetCharacter;
			pChar->Load(file,nVersion);
			pChar->Create(&info);
		}

		ZGetCharacterManager()->Add(pChar);
		mlog("%s : %d %d\n",pChar->GetProperty()->szName,pChar->GetUID().High,pChar->GetUID().Low);

		pChar->SetVisible(true);
	}

	float fGameTime;
	zfread(&fGameTime,sizeof(fGameTime),1,file);
	m_fTime=fGameTime;

	int nCommandCount=0;

	int nSize;
	float fTime;
	while( zfread(&fTime,sizeof(fTime),1,file) )
	{
		nCommandCount++;

		char CommandBuffer[1024];

		MUID uidSender;
		zfread(&uidSender,sizeof(uidSender),1,file);
		zfread(&nSize,sizeof(nSize),1,file);
		if(nSize<0 || nSize>sizeof(CommandBuffer)) {
			m_bReplaying=false;
			ShowReplayInfo( true);
			return false;
		}
		zfread(CommandBuffer,nSize,1,file);

		ZObserverCommandItem *pZCommand=new ZObserverCommandItem;
		pZCommand->pCommand=new MCommand;
		pZCommand->pCommand->SetData(CommandBuffer,ZGetGameClient()->GetCommandManager());
		pZCommand->pCommand->m_Sender=uidSender;
		pZCommand->fTime=fTime;
		m_ReplayCommandList.push_back(pZCommand);

	}

	SetReadyState(ZGAME_READYSTATE_RUN);
	GetMatch()->SetRoundState(MMATCH_ROUNDSTATE_FREE);
	ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);

	ZGetGameInterface()->GetCombatInterface()->GetObserver()->SetTarget(m_pMyCharacter->GetUID());

	g_bProfile=true;	

	dwReplayStartTime=timeGetTime();

	return true;
*/
	return true;
}

void ZGame::EndReplay()
{
	g_bProfile=false;

	DWORD dwReplayEndTime=timeGetTime();
	
	mlog("replay end. profile saved. playtime = %3.3f seconds , average fps = %3.3f \n", 
		float(dwReplayEndTime-dwReplayStartTime)/1000.f,
		1000.f*g_nFrameCount/float(dwReplayEndTime-dwReplayStartTime));


	// 리플레이가 다 끝나면 다시 처음부터 돌려보자. - (버드)
	ZChangeGameState(GUNZ_LOBBY);
}

void ZGame::ConfigureCharacter(const MUID& uidChar, MMatchTeam nTeam, unsigned char nPlayerFlags)
{
	ZCharacterManager* pCharMgr = ZGetCharacterManager();
	ZCharacter* pChar = pCharMgr->Find(uidChar);
	if (pChar == NULL) return;

	pChar->SetAdminHide((nPlayerFlags & MTD_PlayerFlags_AdminHide) !=0);
	pChar->SetTeamID(nTeam);
	pChar->InitStatus();
	pChar->InitRound();

	ZGetCombatInterface()->OnAddCharacter(pChar);
}

void ZGame::RefreshCharacters()
{
	for (MMatchPeerInfoList::iterator itor = ZGetGameClient()->GetPeers()->begin();
		itor != ZGetGameClient()->GetPeers()->end(); ++itor)
	{
		MMatchPeerInfo* pPeerInfo = (*itor).second;
		ZCharacter* pCharacter = m_CharacterManager.Find(pPeerInfo->uidChar);

		if (pCharacter == NULL) {

			pCharacter = m_CharacterManager.Add(pPeerInfo->uidChar, rvector(0.0f, 0.0f, 0.0f));
			pCharacter->Create(&pPeerInfo->CharInfo);

			if (m_Match.GetRoundState() == MMATCH_ROUNDSTATE_PREPARE)
			{
				if (m_Match.IsTeamPlay())
				{
//					pCharacter->SetVisible(true);		// RAONHAJE: PeerOpened TEST
				}
			}


			/*
			// TODO: AGENT 통한 peer들에게는 씹힌다. 수정요망.

			//신입 캐릭터에게 자신의 무기를 알린다...

			ZCharacter* pMyCharacter = g_pGame->m_pMyCharacter;
			if(pMyCharacter)
			{
				//			if(pMyCharacter != pCharacter) { // 자신이 새로 생긴 캐릭이 아니라면
				int nParts = g_pGame->m_pMyCharacter->GetItems()->GetSelectedWeaponParts();
				ZPostChangeWeapon(nParts);
				//			}
			}
			*/
		}
	}
}

void ZGame::DeleteCharacter(const MUID& uid)
{
	bool bObserverDel = false;
	ZCharacter* pCharacter = ZGetCharacterManager()->Find(uid);

	// 옵져버 타겟인 경우 다른 타겟으로 바꿔준다.
	ZObserver* pObserver = ZGetGameInterface()->GetCombatInterface()->GetObserver();
	if (pObserver->IsVisible())
	{
		if ((pCharacter != NULL) && (pCharacter == pObserver->GetTargetCharacter()))
		{
			bObserverDel = true;				
		}
	}

	m_CharacterManager.Delete(uid);


	if (bObserverDel)
	{
		if (pObserver) pObserver->SetFirstTarget();
	}
}


void ZGame::OnStageEnterBattle(MCmdEnterBattleParam nParam, MTD_PeerListNode* pPeerNode)
{
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) return;

	MUID uidChar = pPeerNode->uidChar;

	if (uidChar == ZGetMyUID())		// enter한사람이 나자신일 경우
	{
		if (g_pGame->CreateMyCharacter(&pPeerNode->CharInfo) == true)
		{
			ConfigureCharacter(uidChar, (MMatchTeam)pPeerNode->ExtendInfo.nTeam, pPeerNode->ExtendInfo.nPlayerFlags);	// Player Character 포함
		}
	}
	else		// enter한사람이 나 자신이 아닐경우
	{
		OnAddPeer(pPeerNode->uidChar, pPeerNode->dwIP, pPeerNode->nPort, pPeerNode);
	}


	if (nParam == MCEP_FORCED)
	{
		ZCharacter* pChar = ZGetCharacterManager()->Find(uidChar);
		GetMatch()->OnForcedEntry(pChar);

		char temp[256] = "";
		if((pPeerNode->ExtendInfo.nPlayerFlags & MTD_PlayerFlags_AdminHide)==0) {
			ZTransMsg(temp, MSG_GAME_JOIN_BATTLE, 1, pChar->GetUserAndClanName());
			ZChatOutput(MCOLOR(ZCOLOR_GAME_INFO), temp);
		}
	}

	ZGetGameClient()->OnStageEnterBattle(uidChar,nParam,pPeerNode);
}

void ZGame::OnStageLeaveBattle(const MUID& uidChar, const MUID& uidStage)
{
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) return;

	if (uidChar != ZGetMyUID()) {

		ZCharacter* pChar = ZGetCharacterManager()->Find(uidChar);

		if(pChar && !pChar->IsAdminHide()) {
			char temp[256] = "";
			ZTransMsg(temp, MSG_GAME_LEAVE_BATTLE, 1, pChar->GetUserAndClanName());
			ZChatOutput(MCOLOR(ZCOLOR_GAME_INFO), temp);
		}

		ZGetGameClient()->DeletePeer(uidChar);
		if (ZApplication::GetGameInterface()->GetState() == GUNZ_GAME) {
			DeleteCharacter(uidChar);
		}

		ZGetGameClient()->SetVoteInProgress( false );
		ZGetGameClient()->SetCanVote( false );
	}
}

void ZGame::OnAddPeer(const MUID& uidChar, DWORD dwIP, const int nPort, MTD_PeerListNode* pNode)
{
	if ((ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) || (g_pGame == NULL)) return;

	/*
	//// UDPTEST LOG ////////////////////////////////
	char szLog[256];
	sprintf(szLog, "[%d:%d] ADDPEER: Char(%d:%d) IP:%s, Port:%d \n", 
	GetPlayerUID().High, GetPlayerUID().Low, uidChar.High, uidChar.Low, szIP, nPort);
	mlog(szLog);
	/////////////////////////////////////////////////
	*/

	// 난입한 사람이 자기 자신이 아니면
	if (uidChar != ZGetMyUID())
	{
		if (pNode == NULL)
		{
			/* // -_-?
			MTD_CharInfo charinfo;
			pCharInfo = &charinfo;
			pCharInfo->szName[0] = 0;
			memset(pCharInfo->nEquipedItemDesc, 0, sizeof(pCharInfo->nEquipedItemDesc));
			*/
			_ASSERT(0);
		}

		ZGetGameClient()->DeletePeer(uidChar);	// Delete exist info

		MMatchPeerInfo* pNewPeerInfo = new MMatchPeerInfo;

		if (uidChar == MUID(0,0)) pNewPeerInfo->uidChar = MUID(0, nPort);	// 로컬테스트를 위해서
		else pNewPeerInfo->uidChar = uidChar;

		pNewPeerInfo->dwIP = dwIP;
		in_addr addr;
		addr.s_addr = dwIP;
		char* pszIP = inet_ntoa(addr);
		strcpy(pNewPeerInfo->szIP, pszIP);

		pNewPeerInfo->nPort = nPort;
		memcpy(&pNewPeerInfo->CharInfo, &(pNode->CharInfo), sizeof(MTD_CharInfo));
		memcpy(&pNewPeerInfo->ExtendInfo, &(pNode->ExtendInfo), sizeof(MTD_ExtendInfo));

		ZGetGameClient()->AddPeer(pNewPeerInfo);	

		RefreshCharacters();
	}

	ConfigureCharacter(uidChar, (MMatchTeam)pNode->ExtendInfo.nTeam, pNode->ExtendInfo.nPlayerFlags);	// Player Character 포함
}

void ZGame::OnPeerList(const MUID& uidStage, void* pBlob, int nCount)
{
	if (ZGetGameClient()->GetStageUID() != uidStage) return;
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) return;
	if ((ZApplication::GetGame() == NULL) || (ZGetCharacterManager() == NULL)) return;

	for(int i=0; i<nCount; i++) {
		MTD_PeerListNode* pNode = (MTD_PeerListNode*)MGetBlobArrayElement(pBlob, i);
		OnAddPeer(pNode->uidChar, pNode->dwIP, pNode->nPort, pNode);

		ZCharacter* pChar = ZGetCharacterManager()->Find(pNode->uidChar);
		if (pChar)
		{
			pChar->SetVisible(false);
		}
	}
}

void ZGame::OnGameRoundState(const MUID& uidStage, int nRound, int nRoundState, int nArg)
{
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) return;
	ZMatch* pMatch = GetMatch();
	if (pMatch == NULL) return;

	MMATCH_ROUNDSTATE RoundState = MMATCH_ROUNDSTATE(nRoundState);

	// 프리모드일경우
	if ((RoundState == MMATCH_ROUNDSTATE_FREE) && (pMatch->GetRoundState() != RoundState))
	{
		pMatch->InitCharactersPosition();
		m_pMyCharacter->SetVisible(true);
		m_pMyCharacter->Revival();
		ReleaseObserver();
	}

	pMatch->SetRound(nRound);
	pMatch->SetRoundState(RoundState, nArg);
	AddEffectRoundState(RoundState, nArg);

	if (RoundState == MMATCH_ROUNDSTATE_FINISH)
	{
		ZGetMyInfo()->GetGameInfo()->InitRound();	// 제일 마지막에 불려져야 한다.
	}
}


bool ZGame::FilterDelayedCommand(MCommand *pCommand)
{
	bool bFiltered = true;
	float fDelayTime = 0;

	MUID uid=pCommand->GetSenderUID();
	ZCharacter *pChar=ZGetCharacterManager()->Find(uid);
	if(!pChar) return false;

	switch (pCommand->GetID())
	{
		case MC_PEER_SKILL:
			{
				int nSkill;
				pCommand->GetParameter(&nSkill, 0, MPT_INT);
				fDelayTime = .15f;
				switch(nSkill)	{
					case ZC_SKILL_UPPERCUT		: 
						if(pChar!=m_pMyCharacter) pChar->SetAnimationLower(ZC_STATE_LOWER_UPPERCUT);
						break;
					case ZC_SKILL_SPLASHSHOT	: break;
					case ZC_SKILL_DASH			: break;
				}

				////////////////////////////////////////////////////////////////////
				int sel_type;
				pCommand->GetParameter(&sel_type, 2, MPT_INT);
				MMatchCharItemParts parts = (MMatchCharItemParts)sel_type;
				if( parts != pChar->GetItems()->GetSelectedWeaponParts()) {
					// 지금 들고 있는 무기와 보내진 무기가 틀리다면 보내진 무기로 바꿔준다..
					OnChangeWeapon(uid,parts);
				}
				////////////////////////////////////////////////////////////////////


			}break;

		case MC_PEER_SHOT:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if(pParam->GetType()!=MPT_BLOB) break;	// 문제가 있다
				ZPACKEDSHOTINFO *pinfo =(ZPACKEDSHOTINFO*)pParam->GetPointer();

				// 칼질만 딜레이가 있다
				if(pinfo->sel_type!=MMCIP_MELEE) return false;

				if(pChar!=m_pMyCharacter &&
					( pChar->m_pVMesh->m_SelectWeaponMotionType==eq_wd_dagger ||
					pChar->m_pVMesh->m_SelectWeaponMotionType==eq_ws_dagger )) { // dagger
						pChar->SetAnimationUpper(ZC_STATE_UPPER_SHOT);
					}

				fDelayTime = .15f;

				////////////////////////////////////////////////////////////////////
				MMatchCharItemParts parts = (MMatchCharItemParts)pinfo->sel_type;
				if( parts != pChar->GetItems()->GetSelectedWeaponParts()) {
					// 지금 들고 있는 무기와 보내진 무기가 틀리다면 보내진 무기로 바꿔준다..
					OnChangeWeapon(uid,parts);
				}
				///////////////////////////////////////////////////////////////////////////////
			}
			break;

		// 새로 추가된 근접공격 커맨드
		case MC_PEER_SHOT_MELEE:
			{
				float fShotTime;
				rvector pos;
				int nShot;

				pCommand->GetParameter(&fShotTime, 0, MPT_FLOAT);
				pCommand->GetParameter(&pos, 1, MPT_POS);
				pCommand->GetParameter(&nShot, 2, MPT_INT);

				if(pChar!=m_pMyCharacter &&
					( pChar->m_pVMesh->m_SelectWeaponMotionType==eq_wd_dagger ||
					pChar->m_pVMesh->m_SelectWeaponMotionType==eq_ws_dagger )) { // dagger
						pChar->SetAnimationUpper(ZC_STATE_UPPER_SHOT);
					}

				fDelayTime = .1f;
				switch(nShot) {
					case 1 : fDelayTime = .10f;break;
					case 2 : fDelayTime = .15f;break;
					case 3 : fDelayTime = .2f;break;
					case 4 : fDelayTime = .25f;break;
				}
			}
			break;

// 퀘스트 커맨드들
		case MC_QUEST_PEER_NPC_ATTACK_MELEE :	// npc 칼질
			ZGetQuest()->OnPrePeerNPCAttackMelee(pCommand);
			fDelayTime = .4f;break;


		// 이것들 이외의 것들은 
		default:
			bFiltered = false;
			break;
	}

	
	if(bFiltered)
	{
		ZObserverCommandItem *pZCommand=new ZObserverCommandItem;
		pZCommand->pCommand=pCommand->Clone();
		pZCommand->fTime=GetTime()+fDelayTime;
		m_DelayedCommandList.push_back(pZCommand);
		return true;
	}

	return false;
}

void ZGame::PostSpMotion(ZC_SPMOTION_TYPE mtype)
{
	if(m_pMyCharacter==NULL) return;

	if( (m_pMyCharacter->m_AniState_Lower == ZC_STATE_LOWER_IDLE1) || 
		(m_pMyCharacter->m_AniState_Lower == ZC_STATE_LOWER_IDLE2) || 
		(m_pMyCharacter->m_AniState_Lower == ZC_STATE_LOWER_IDLE3) || 
		(m_pMyCharacter->m_AniState_Lower == ZC_STATE_LOWER_IDLE4) ) 
	{

		MMatchWeaponType type = MWT_NONE;

		ZItem* pSItem = m_pMyCharacter->GetItems()->GetSelectedWeapon();

		if( pSItem && pSItem->GetDesc() ) {
			type = pSItem->GetDesc()->m_nWeaponType;
		}

		if( mtype == ZC_SPMOTION_TAUNT ) // taunt 일 경우 모션이 없어서...
			if( (type == MWT_MED_KIT) || 
				(type == MWT_REPAIR_KIT) || 
				(type == MWT_FOOD) ||
				(type == MWT_BULLET_KIT)) 
			{
				return;
			}

		ZPostSpMotion(mtype);
	}
}

void ZGame::OnEventUpdateJjang(const MUID& uidChar, bool bJjang)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uidChar);
	if (pCharacter == NULL) return;

	if (bJjang) 
		ZGetEffectManager()->AddStarEffect(pCharacter);        
}


bool ZGame::IsAttackable(ZObject *pAttacker, ZObject *pTarget)
{
	if(GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_PLAY) return false;
	if(pAttacker==NULL) return true;

	if ( GetMatch()->IsTeamPlay() ) {
		if (pAttacker->GetTeamID() == pTarget->GetTeamID()) {
			if (!GetMatch()->GetTeamKillEnabled()) 
				return false;
		}
	}

#ifdef _QUEST
	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
	{
		if (pAttacker->GetTeamID() == pTarget->GetTeamID())
		{
			return false;
		}
	}

#endif
	return true;
}

void ZGame::ShowReplayInfo( bool bShow)
{
	MWidget* pWidget = ZGetGameInterface()->GetIDLResource()->FindWidget( "CombatChatOutput");
	if ( pWidget)
		pWidget->Show( bShow);

	m_bShowReplayInfo = bShow;
}

void ZGame::OnLocalOptainSpecialWorldItem(MCommand* pCommand)
{
	int nWorldItemID;
	pCommand->GetParameter(&nWorldItemID   , 0, MPT_INT);

	switch (nWorldItemID)
	{
	case WORLDITEM_PORTAL_ID:
		{
			if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST) break;

			// 서버에 포탈로 이동한다고 전송
			char nCurrSectorIndex = ZGetQuest()->GetGameInfo()->GetCurrSectorIndex();
			ZPostQuestRequestMovetoPortal(nCurrSectorIndex);
		}
		break;
	};
}


void ZGame::ReserveSuicide( void)
{
	m_dwReservedSuicideTime = timeGetTime() + 10000;
	m_bSuicide = true;
}


bool ZGame::OnRuleCommand(MCommand* pCommand)
{
#ifdef _QUEST
	if (ZGetQuest()->OnGameCommand(pCommand)) return true;
#endif

	switch (pCommand->GetID())
	{
	case MC_MATCH_ASSIGN_COMMANDER:
	case MC_MATCH_ASSIGN_BERSERKER:
	case MC_MATCH_GAME_DEAD:
	case MC_MATCH_DUEL_QUEUEINFO:
		{
			if (m_Match.OnCommand(pCommand)) return true;
		};
	};

	return false;
}

void ZGame::OnResetTeamMembers(MCommand* pCommand)
{
	if (!m_Match.IsTeamPlay()) return;

	ZChatOutput( MCOLOR(ZCOLOR_GAME_INFO), ZMsg(MSG_GAME_MAKE_AUTO_BALANCED_TEAM) );

	MCommandParameter* pParam = pCommand->GetParameter(0);
	if(pParam->GetType()!=MPT_BLOB) return;
	void* pBlob = pParam->GetPointer();
	int nCount = MGetBlobArrayCount(pBlob);

	ZCharacterManager* pCharMgr = ZGetCharacterManager();

	for (int i = 0; i < nCount; i++)
	{
		MTD_ResetTeamMembersData* pDataNode = (MTD_ResetTeamMembersData*)MGetBlobArrayElement(pBlob, i);

		ZCharacter* pChar = pCharMgr->Find(pDataNode->m_uidPlayer);
		if (pChar == NULL) continue;

		if (pChar->GetTeamID() != ( (MMatchTeam)pDataNode->nTeam) )
		{
			// 만약 나자신이 팀변경이 되었으면 팀변경되었는지를 남긴다.
			if (pDataNode->m_uidPlayer == ZGetMyUID())
			{
				ZGetMyInfo()->GetGameInfo()->bForcedChangeTeam = true;
			}

			pChar->SetTeamID((MMatchTeam)pDataNode->nTeam);
		}

	}
}