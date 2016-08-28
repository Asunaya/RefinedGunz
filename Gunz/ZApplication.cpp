#include "stdafx.h"

#include "ZApplication.h"
#include "ZGameInterface.h"
#include "MCommandLogFrame.h"
#include "ZConsole.h"
#include "ZInterface.h"
#include "Config.h"
#include "MDebug.h"
#include "RMeshMgr.h"
#include "RShadermgr.h"
#include "ZConfiguration.h"
#include "MProfiler.h"
#include "MChattingFilter.h"
#include "ZNetmarble.h"
#include "ZInitialLoading.h"
#include "ZWorldItem.h"
#include "MMatchWorlditemdesc.h"
#include "ZSecurity.h"
#include "ZReplay.h"
#include "ZTestGame.h"
#include "ZGameClient.h"
#include "MRegistry.h"
#include "CGLEncription.h"
#include "ZLocale.h"
#include "ZUtil.h"
#include "ZStringResManager.h"
#include "ZFile.h"
#include "ZActionKey.h"
#include "ZInput.h"
#include "ZOptionInterface.h"
#include "RGMain.h"
#include "MeshManager.h"

#ifdef _QEUST_ITEM_DEBUG
#include "MQuestItem.h"
#endif

#ifdef _ZPROFILER
#include "ZProfiler.h"
#endif

ZApplication*	ZApplication::m_pInstance = NULL;
MZFileSystem	ZApplication::m_FileSystem;    
ZSoundEngine	ZApplication::m_SoundEngine;
RMeshMgr		ZApplication::m_NPCMeshMgr;
RMeshMgr		ZApplication::m_MeshMgr;
RMeshMgr		ZApplication::m_WeaponMeshMgr;
ZTimer			ZApplication::m_Timer;
ZEmblemInterface	ZApplication::m_EmblemInterface;
ZSkillManager	ZApplication::m_SkillManager;				///< 스킬 매니저

MCommandLogFrame* m_pLogFrame = NULL;


ZApplication::ZApplication()
	: Time(timeGetTime())
{
	_ASSERT(m_pInstance==NULL);

	m_nTimerRes = 0;
	m_pInstance = this;


	m_pGameInterface=NULL;
	m_pStageInterface = NULL;

	m_nInitialState = GUNZ_LOGIN;

	m_bLaunchDevelop = false;
	m_bLaunchTest = false;

	SetLaunchMode(ZLAUNCH_MODE_DEBUG);

#ifdef _ZPROFILER
	m_pProfiler = new ZProfiler;
#endif
}

ZApplication::~ZApplication()
{
#ifdef _ZPROFILER
	SAFE_DELETE(m_pProfiler);
#endif

//	OnDestroy();
	m_pInstance = NULL;


}

// szBuffer 에 있는 다음 단어를 얻는다. 단 따옴표가 있으면 따옴표 안의 단어를 얻는다
bool GetNextName(char *szBuffer, int nBufferCount, const char *szSource)
{
	while(*szSource==' ' || *szSource=='\t') szSource++;

	const char *end=NULL;
	if(szSource[0]=='"') 
		end=strchr(szSource+1,'"');
	else
	{
		end=strchr(szSource,' ');
		if(NULL==end) end=strchr(szSource,'\t');
	}

	if(end)
	{
		int nCount=end-szSource-1;
		if(nCount==0 || nCount>=nBufferCount) return false;

		strncpy(szBuffer,szSource+1,nCount);
		szBuffer[nCount]=0;
	}
	else
	{
		int nCount=(int)strlen(szSource);
		if(nCount==0 || nCount>=nBufferCount) return false;

		strcpy_safe(szBuffer, nBufferCount, szSource);
	}

	return true;
}

bool ZApplication::ParseArguments(const char* pszArgs)
{
	strcpy_safe(m_szCmdLine, pszArgs);

	// 파라미터가 리플레이 파일명인지 확인한다
	{
		size_t nLength;

		// 따옴표 있으면 제거한다
		if(pszArgs[0]=='"') 
		{
			strcpy_safe(m_szFileName,pszArgs+1);

			nLength = strlen(m_szFileName);
			if(m_szFileName[nLength-1]=='"')
			{
				m_szFileName[nLength-1]=0;
				nLength--;
			}
		}
		else
		{
			strcpy_safe(m_szFileName,pszArgs);
			nLength = strlen(m_szFileName);
		}

		if(_stricmp(m_szFileName+nLength-strlen(GUNZ_REC_FILE_EXT),GUNZ_REC_FILE_EXT)==0){
			SetLaunchMode(ZLAUNCH_MODE_STANDALONE_REPLAY);
			m_nInitialState = GUNZ_GAME;
			ZGetLocale()->SetTeenMode(false);
			return true;
		}
	}


	// '/launchdevelop' 모드
	if ( pszArgs[0] == '/')
	{
#ifndef _PUBLISH
		if ( strstr( pszArgs, "launchdevelop") != NULL)
		{
			SetLaunchMode( ZLAUNCH_MODE_STANDALONE_DEVELOP);
			m_bLaunchDevelop = true;
			ZGetLocale()->SetTeenMode( false);

			return true;
		} 
		// '/launch' 모드
		else if ( strstr( pszArgs, "launch") != NULL)
		{
			SetLaunchMode(ZLAUNCH_MODE_STANDALONE);
			return true;
		}
#endif
	}

	// TODO: 일본넷마블 테스트하느라 주석처리 - bird
	// 디버그버전은 파라메타가 없어도 launch로 실행하도록 변경
#ifndef _PUBLISH
	{
		SetLaunchMode(ZLAUNCH_MODE_STANDALONE_DEVELOP);
		m_bLaunchDevelop=true;
		return true;
	}
#endif


/*	// RAON DEBUG ////////////////////////
	ZNetmarbleAuthInfo* pMNInfo = ZNetmarbleAuthInfo::GetInstance();
	pMNInfo->SetServerIP("192.168.0.30");
	pMNInfo->SetServerPort(6000);
	pMNInfo->SetAuthCookie("");
	pMNInfo->SetDataCookie("");
	pMNInfo->SetCpCookie("Certificate=4f3d7e1cf8d27bd0&Sex=f1553a2f8bd18a59&Name=018a1751ea0eaf54&UniID=25c1272f61aaa6ec8d769f14137cf298&Age=1489fa5ce12aeab7&UserID=e23616614f162e03");
	pMNInfo->SetSpareParam("Age=15");

	SetLaunchMode(ZLAUNCH_MODE_NETMARBLE);
	return true;
	//////////////////////////////////////
	*/		

	switch(ZGetLocale()->GetCountry()) {
	case MC_JAPAN:
	case MC_KOREA:
		{
			return ZGetLocale()->ParseArguments(pszArgs);
		}
		break;
	case MC_US:
	case MC_BRAZIL:
	case MC_INDIA:
		{
			SetLaunchMode( ZLAUNCH_MODE_STANDALONE);
			ZGetLocale()->SetTeenMode(false);
			return true;
			// 암호 코드 구하기
			CGLEncription cEncription;
			int nMode = cEncription.Decription();

			// launch 모드
			if ( nMode == GLE_LAUNCH_INTERNATIONAL)
			{
				SetLaunchMode( ZLAUNCH_MODE_STANDALONE);
				ZGetLocale()->SetTeenMode(false);
				return true;
			}
			// launchdevelop 모드
			else if ( nMode == GLE_LAUNCH_DEVELOP)
			{
				SetLaunchMode( ZLAUNCH_MODE_STANDALONE_DEVELOP);
				m_bLaunchDevelop = true;
				ZGetLocale()->SetTeenMode(false);
				return true;
			}
			// Test 모드
			else if ( nMode == GLE_LAUNCH_TEST)
			{
				SetLaunchMode( ZLAUNCH_MODE_STANDALONE_DEVELOP);
				m_bLaunchDevelop = true;
				ZGetLocale()->SetTeenMode(false);
				m_bLaunchTest = true;
				return true;
			}
		}
		break;
	case MC_INVALID:
	default:
		{
			mlog("Invalid Locale \n");
			return false;
		}
		break;
	};

	return false;
}

void ZApplication::CheckSound()
{
#ifdef _BIRDSOUND

#else
	int size = m_MeshMgr.m_id_last;
	int ani_size = 0;

	RMesh* pMesh = NULL;
	RAnimationMgr* pAniMgr = NULL;
	RAnimation* pAni = NULL;

	for(int i=0;i<size;i++) {
		pMesh = m_MeshMgr.GetFast(i);
		if(pMesh) {
			pAniMgr = &pMesh->m_ani_mgr;
			if(pAniMgr){
				ani_size = pAniMgr->m_id_last;
				for(int j=0;j<ani_size;j++) {
					pAni = pAniMgr->m_node_table[j];
					if(pAni) {

						if(m_SoundEngine.isPlayAbleMtrl(pAni->m_sound_name)==false) {
							MLog("ClearSoundFile %s %s\n", pAni->GetName(), pAni->m_sound_name);
							pAni->ClearSoundFile();
						}
						else {
							int ok = 0;
						}
					}
				}
			}
		}
	}
#endif
}

void RegisterForbidKey()
{
	ZActionKey::RegisterForbidKey(0x3b);// f1
	ZActionKey::RegisterForbidKey(0x3c);
	ZActionKey::RegisterForbidKey(0x3d);
	ZActionKey::RegisterForbidKey(0x3e);
	ZActionKey::RegisterForbidKey(0x3f);
	ZActionKey::RegisterForbidKey(0x40);
	ZActionKey::RegisterForbidKey(0x41);
	ZActionKey::RegisterForbidKey(0x42);// f8

	ZActionKey::RegisterForbidKey(0x35);// /
	ZActionKey::RegisterForbidKey(0x1c);// enter
}

void ZProgressCallBack(void *pUserParam,float fProgress)
{
	ZLoadingProgress *pLoadingProgress = (ZLoadingProgress*)pUserParam;
	pLoadingProgress->UpdateAndDraw(fProgress);
}

static void ListSoundDevices()
{
	for (int i = 0; i < ZGetSoundEngine()->GetEnumDeviceCount(); ++i)
		mlog("Sound Device %d = %s\n", i, ZGetSoundEngine()->GetDeviceDescription(i));
}

namespace RealSpace2
{
	extern bool DynamicResourceLoading;
}

#define STARTUP_CACHE_FILES

bool ZApplication::OnCreate(ZLoadingProgress *pLoadingProgress)
{
	MInitProfile();

	TIMECAPS tc;

	mlog("ZApplication::OnCreate : begin\n");

	ZLoadingProgress InitialLoading("Initializing", pLoadingProgress, 0.05f);
	InitialLoading.UpdateAndDraw(0);

#ifdef STARTUP_CACHE_FILES
	__BP(1999, "Cache archives");
	const char* CachedFileNames[] = { "system", "model", "sfx",
		"interface/default", "interface/loadable", "interface/Login",
		"sound/bgm", "sound/effect", };
	constexpr int NumCachedFiles = ArraySize(CachedFileNames);
	int CacheIndices[NumCachedFiles];
	for (int i = 0; i < NumCachedFiles; i++)
		CacheIndices[i] = m_FileSystem.CacheArchive(CachedFileNames[i]);
	__EP(1999);
#endif

	InitialLoading.UpdateAndDraw(0.5f);

	//ListSoundDevices();

	[&]
	{
		if (!IsDynamicResourceLoad())
			return;

		auto Fail = [&]()
		{
			MLog("Failed to load parts index! Turning off dynamic resource loading\n");
			RealSpace2::DynamicResourceLoading = false;
		};

		auto ret = ReadMZFile("system/parts_index.xml");
		if (!ret.first)
			return Fail();

		ret.second.emplace_back(0);
		if (!GetMeshManager()->LoadParts(ret.second))
			return Fail();
	}();

	__BP(2000, "ZApplication::OnCreate");

	g_RGMain->OnAppCreate();

	InitialLoading.UpdateAndDraw(1);

#define MMTIMER_RESOLUTION	1
	if (TIMERR_NOERROR == timeGetDevCaps(&tc, sizeof(TIMECAPS)))
	{
		m_nTimerRes = min(max(tc.wPeriodMin, MMTIMER_RESOLUTION), tc.wPeriodMax);
		timeBeginPeriod(m_nTimerRes);
	}

	if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_NETMARBLE)
		m_nInitialState = GUNZ_NETMARBLELOGIN;

	DWORD _begin_time,_end_time;
#define BEGIN_ { _begin_time = GetGlobalTimeMS(); }
#define END_(x) { _end_time = GetGlobalTimeMS(); float f_time = (_end_time - _begin_time) / 1000.f; \
	mlog("\n-------------------> %s : %f \n\n", x,f_time ); }

	__BP(2001,"m_SoundEngine.Create");

	ZLoadingProgress soundLoading("Sound", pLoadingProgress, 0.12f);
	BEGIN_;
#ifdef _BIRDSOUND
	m_SoundEngine.Create(RealSpace2::g_hWnd, 44100, Z_AUDIO_HWMIXING, GetFileSystem());
#else
	m_SoundEngine.Create(RealSpace2::g_hWnd, Z_AUDIO_HWMIXING, &soundLoading );
#endif
	END_("Sound Engine Create");
	soundLoading.UpdateAndDraw(1.f);

	__EP(2001);

	RegisterForbidKey();

	__BP(2002,"m_pInterface->OnCreate()");

	ZLoadingProgress giLoading("GameInterface",pLoadingProgress,.35f);

	BEGIN_;
	m_pGameInterface = new ZGameInterface("GameInterface",
		Mint::GetInstance()->GetMainFrame(), Mint::GetInstance()->GetMainFrame());
	m_pGameInterface->m_nInitialState = m_nInitialState;
	if(!m_pGameInterface->OnCreate(&giLoading))
	{
		mlog("Failed: ZGameInterface OnCreate\n");
		SAFE_DELETE(m_pGameInterface);
		return false;
	}

	m_pGameInterface->SetBounds(0,0,MGetWorkspaceWidth(),MGetWorkspaceHeight());
	END_("GameInterface Create");
	mlog("ZApplication::OnCreate : GameInterface Created\n");

	giLoading.UpdateAndDraw(1.f);

	__BP(1998, "ZStageInterface/ZOptionInterface constructor");
	m_pStageInterface = new ZStageInterface;
	m_pOptionInterface = new ZOptionInterface;
	__EP(1998);

	mlog("ZApplication::OnCreate : m_pInterface->OnCreate() \n");

	__EP(2002);

#ifdef _BIRDTEST
	goto BirdGo;
#endif

	__BP(2003,"Character Loading");

	ZLoadingProgress meshLoading("Mesh",pLoadingProgress,.41f);
	BEGIN_;

	if(m_MeshMgr.LoadXmlList("model/character.xml",ZProgressCallBack,&meshLoading)==-1)
		return false;

	SetAnimationMgr(MMS_MALE, &m_MeshMgr.Get("heroman1")->m_ani_mgr);
	SetAnimationMgr(MMS_FEMALE, &m_MeshMgr.Get("herowoman1")->m_ani_mgr);

	mlog("ZApplication::OnCreate : m_MeshMgr.LoadXmlList(character.xml) \n");

	END_("Character Loading");
	meshLoading.UpdateAndDraw(1.f);

	__EP(2003);

	__BP(1985, "Quest NPC loading");
#ifdef _QUEST
	if(m_NPCMeshMgr.LoadXmlList("model/npc.xml") == -1)
		return false;
#endif
	__EP(1985);

	/*__BP(1986, "CheckSound");
	CheckSound();
	__EP(1986);*/

	__BP(2004,"WeaponMesh Loading");

	BEGIN_;

	if(m_WeaponMeshMgr.LoadXmlList("model/weapon.xml")==-1) 
		return false;

	END_("WeaponMesh Loading");

	__EP(2004);

	__BP(2005,"Worlditem Loading");

	ZLoadingProgress etcLoading("etc",pLoadingProgress,.02f);
	BEGIN_;

#ifdef	_WORLD_ITEM_
	m_MeshMgr.LoadXmlList("system/worlditem.xml");
#endif

	mlog("ZApplication::OnCreate : m_WeaponMeshMgr.LoadXmlList(weapon.xml) \n");

	END_("Worlditem Loading");
	__EP(2005);

#ifdef _BIRDTEST
BirdGo:
#endif

	__BP(2006,"ETC .. XML");

	BEGIN_;
	CreateConsole(ZGetGameClient()->GetCommandManager());

	mlog("ZApplication::OnCreate : CreateConsole \n");

	m_pLogFrame = new MCommandLogFrame("Command Log", Mint::GetInstance()->GetMainFrame(),
		Mint::GetInstance()->GetMainFrame());
	int nHeight = MGetWorkspaceHeight()/3;
	m_pLogFrame->SetBounds(0, MGetWorkspaceHeight()-nHeight-1, MGetWorkspaceWidth()-1, nHeight);
	m_pLogFrame->Show(false);

	m_pGameInterface->SetFocusEnable(true);
	m_pGameInterface->SetFocus();
	m_pGameInterface->Show(true);

	if (!MGetMatchItemDescMgr()->ReadXml(GetFileSystem(), FILENAME_ZITEM_DESC))
	{
		MLog("Error while Read Item Descriptor %s\n", FILENAME_ZITEM_DESC);
	}
	mlog("ZApplication::OnCreate : MGetMatchItemDescMgr()->ReadXml \n");

	if (!MGetMatchItemEffectDescMgr()->ReadXml(GetFileSystem(), FILENAME_ZITEMEFFECT_DESC))
	{
		MLog("Error while Read Item Descriptor %s\n", FILENAME_ZITEMEFFECT_DESC);
	}
	mlog("ZApplication::OnCreate : MGetMatchItemEffectDescMgr()->ReadXml \n");

	if (!MGetMatchWorldItemDescMgr()->ReadXml(GetFileSystem(), "system/worlditem.xml"))
	{
		MLog("Error while Read Item Descriptor %s\n", "system/worlditem.xml");
	}
	mlog("ZApplication::OnCreate : MGetMatchWorldItemDescMgr()->ReadXml \n");

	if (!ZGetChannelRuleMgr()->ReadXml(GetFileSystem(), "system/channelrule.xml"))
	{
		MLog("Error while Read Item Descriptor %s\n", "system/channelrule.xml");
	}
	mlog("ZApplication::OnCreate : ZGetChannelRuleMgr()->ReadXml \n");

	if (!MGetChattingFilter()->LoadFromFile(GetFileSystem(), "system/abuse.txt"))
	{
		MLog("Error while Read Abuse Filter %s\n", "system/abuse.xml");
	}

#ifdef _QUEST_ITEM
	if( !GetQuestItemDescMgr().ReadXml(GetFileSystem(), FILENAME_QUESTITEM_DESC) )
	{
		MLog( "Error while read quest tiem descrition xml file.\n" );
	}
#endif

	mlog("ZApplication::OnCreate : MGetChattingFilter()->Create \n");

	if(!m_SkillManager.Create()) {
		MLog("Error while create skill manager\n");
	}

	END_("ETC ..");

#ifndef _BIRDTEST
	etcLoading.UpdateAndDraw(1.f);
#endif

	ZGetEmblemInterface()->Create();

	__EP(2006);

	__EP(2000);

	__SAVEPROFILE("profile_loading.txt");

	if (ZCheckFileHack() == true)
	{
		MLog("File Check Failed\n");
		return false;
	}

	ZSetupDataChecker_Global(&m_GlobalDataChecker);

#ifdef STARTUP_CACHE_FILES
	for (int i = 0; i < NumCachedFiles; i++)
		m_FileSystem.ReleaseArchive(CacheIndices[i]);
#endif

	return true;
}

void ZApplication::OnDestroy()
{
	m_WorldManager.Destroy();
	ZGetEmblemInterface()->Destroy();

	MGetMatchWorldItemDescMgr()->Clear();

	m_SoundEngine.Destroy();
	DestroyConsole();

	mlog("ZApplication::OnDestroy : DestroyConsole() \n");

	SAFE_DELETE(m_pLogFrame);
	SAFE_DELETE(m_pGameInterface);
	SAFE_DELETE(m_pStageInterface);
	SAFE_DELETE(m_pOptionInterface);

	m_NPCMeshMgr.DelAll();

	m_MeshMgr.DelAll();
	mlog("ZApplication::OnDestroy : m_MeshMgr.DelAll() \n");

	m_WeaponMeshMgr.DelAll();
	mlog("ZApplication::OnDestroy : m_WeaponMeshMgr.DelAll() \n");

	if (m_nTimerRes != 0)
	{
		timeEndPeriod(m_nTimerRes);
		m_nTimerRes = 0;
	}

	RGetParticleSystem()->Destroy();		

	mlog("ZApplication::OnDestroy done \n");
}

void ZApplication::ResetTimer()
{
	m_Timer.ResetFrame();
}

void ZApplication::OnUpdate()
{
	__BP(0,"ZApplication::OnUpdate");

	[&]
	{
		static u64 LastRealTime = timeGetTime();
		auto CurRealTime = timeGetTime();
		if (Timescale == 1.f)
		{
			Time += CurRealTime - LastRealTime;
		}
		else
		{
			auto Delta = double(CurRealTime - LastRealTime);
			Time += Delta * Timescale;
		}
		LastRealTime = CurRealTime;
	}();

	auto fElapsed = ZApplication::m_Timer.UpdateFrame();

	if (Timescale != 1.f)
		fElapsed *= Timescale;

	g_RGMain->OnUpdate(fElapsed);

	__BP(1,"ZApplication::OnUpdate::m_pInterface->Update");
	if (m_pGameInterface) m_pGameInterface->Update(fElapsed);
	__EP(1);

	__BP(2,"ZApplication::OnUpdate::SoundEngineRun");

#ifdef _BIRDSOUND
	m_SoundEngine.Update();
#else
	m_SoundEngine.Run();
#endif

	__EP(2);

	//// ANTIHACK ////
	{
		static DWORD dwLastAntiHackTick = 0;
		if (GetGlobalTimeMS() - dwLastAntiHackTick > 10000) {
			dwLastAntiHackTick = GetGlobalTimeMS();
			if (m_GlobalDataChecker.UpdateChecksum() == false) {
				Exit();
			}
		}
	}

	if(ZIsActionKeyPressed(ZACTION_SCREENSHOT)) {
		if(m_pGameInterface)
			m_pGameInterface->SaveScreenShot();
	}

	__EP(0);
}

bool g_bProfile=false;

#define PROFILE_FILENAME	"profile.txt"

bool ZApplication::OnDraw()
{
	static bool currentprofile=false;
	if(g_bProfile && !currentprofile)
	{
		currentprofile=true;
        MInitProfile();
	}

	if(!g_bProfile && currentprofile)
	{
		currentprofile=false;
		MSaveProfile(PROFILE_FILENAME);
	}


	__BP(3,"ZApplication::Draw");

		__BP(4,"ZApplication::Draw::Mint::Run");
			if(ZGetGameInterface()->GetState()!=GUNZ_GAME)	// 게임안에서는 막는다
			{
				Mint::GetInstance()->Run();
			}
		__EP(4);

		__BP(5,"ZApplication::Draw::Mint::Draw");

			Mint::GetInstance()->Draw();

		__EP(5);

	__EP(3);

#ifdef _ZPROFILER
	// profiler
	m_pProfiler->Update();
	m_pProfiler->Render();
#endif

	return m_pGameInterface->IsDone();
}

ZApplication* ZApplication::GetInstance(void)
{
	return m_pInstance;
}
ZGameInterface* ZApplication::GetGameInterface(void)
{
	ZApplication* pApp = GetInstance();
	if(pApp==NULL) return NULL;
	return pApp->m_pGameInterface;
}
ZStageInterface* ZApplication::GetStageInterface(void)
{
	ZApplication* pApp = GetInstance();
	if(pApp==NULL) return NULL;
	return pApp->m_pStageInterface;
}
ZOptionInterface* ZApplication::GetOptionInterface(void)
{
	ZApplication* pApp = GetInstance();
	if(pApp==NULL) return NULL;
	return pApp->m_pOptionInterface;
}
MZFileSystem* ZApplication::GetFileSystem(void)
{
	return &m_FileSystem;
}

ZGameClient* ZApplication::GetGameClient(void)
{
	return (GetGameInterface()->GetGameClient());
}

ZGame* ZApplication::GetGame(void)
{
	return (GetGameInterface()->GetGame());
}

ZTimer* ZApplication::GetTimer(void)
{
	return &m_Timer;
}

ZSoundEngine* ZApplication::GetSoundEngine(void)
{
	return &m_SoundEngine;
}

void ZApplication::OnInvalidate()
{
	RGetShaderMgr()->Release();
	if(m_pGameInterface)
		m_pGameInterface->OnInvalidate();
}

void ZApplication::OnRestore()
{
	if(m_pGameInterface)
		m_pGameInterface->OnRestore();
	if( ZGetConfiguration()->GetVideo()->bShader )
	{
		RMesh::mHardwareAccellated		= true;
		if( !RGetShaderMgr()->SetEnable() )
		{
			RGetShaderMgr()->SetDisable();
		}
	}

	g_RGMain->OnReset();
}

void ZApplication::Exit()
{
	PostMessage(g_hWnd,WM_CLOSE,0,0);
}

#define ZTOKEN_GAME				"game"
#define ZTOKEN_REPLAY			"replay"
#define ZTOKEN_GAME_CHARDUMMY	"dummy"
#define ZTOKEN_GAME_AI			"ai"
#define ZTOKEN_QUEST			"quest"
#define ZTOKEN_FAST_LOADING		"fast"

// 맵 테스트 : /launchdevelop game 맵이름
// 더미테스트: /launchdevelop dummy 맵이름 더미숫자 이팩트출력여부
// (ex: /launchdevelop test_a 16 1) or (ex: /launchdevelop manstion 8 0)
// ai 테스트 : /launchdevelop 맵이름 AI숫자

// 리소스 로딩전에 실행된다..

void ZApplication::PreCheckArguments()
{
	char *str;

	str=strstr(m_szCmdLine,ZTOKEN_FAST_LOADING);

	if(str != NULL) {
		RMesh::SetPartsMeshLoadingSkip(1);
	}
}

void ZApplication::ParseStandAloneArguments(const char* pszArgs)
{
	char buffer[256];

	const char *str;
	str=strstr(pszArgs, ZTOKEN_GAME);
	if ( str != NULL) {
		ZApplication::GetInstance()->m_nInitialState = GUNZ_GAME;
		if(GetNextName(buffer,sizeof(buffer),str+strlen(ZTOKEN_GAME)))
		{
			strcpy_safe(m_szFileName,buffer);
			CreateTestGame(buffer);
			SetLaunchMode(ZLAUNCH_MODE_STANDALONE_GAME);
			return;
		}
	}

	str=strstr(pszArgs, ZTOKEN_GAME_CHARDUMMY);
	if ( str != NULL) {
		ZApplication::GetInstance()->m_nInitialState = GUNZ_GAME;
		char szTemp[256], szMap[256];
		int nDummyCount = 0, nShotEnable = 0;
		sscanf(str, "%s %s %d %d", szTemp, szMap, &nDummyCount, &nShotEnable);
		bool bShotEnable = false;
		if (nShotEnable != 0) bShotEnable = true;

		SetLaunchMode(ZLAUNCH_MODE_STANDALONE_GAME);
		CreateTestGame(szMap, nDummyCount, bShotEnable);
		return;
	}

	str=strstr(pszArgs, ZTOKEN_QUEST);
	if ( str != NULL) {
		SetLaunchMode(ZLAUNCH_MODE_STANDALONE_QUEST);
		return;
	}

#ifndef _PUBLISH

	#ifdef _QUEST
		str=strstr(pszArgs, ZTOKEN_GAME_AI);
		if ( str != NULL) {
			SetLaunchMode(ZLAUNCH_MODE_STANDALONE_AI);

			ZApplication::GetInstance()->m_nInitialState = GUNZ_GAME;
			char szTemp[256], szMap[256];
			sscanf(str, "%s %s", szTemp, szMap);

			ZGetGameClient()->GetMatchStageSetting()->SetGameType(MMATCH_GAMETYPE_QUEST);
			
			CreateTestGame(szMap, 0, false, true, 0);
			return;
		}
	#endif

#endif

}

void ZApplication::SetInitialState()
{
	if(GetLaunchMode()==ZLAUNCH_MODE_STANDALONE_REPLAY) {
		CreateReplayGame(m_szFileName);
		return;
	}

	ParseStandAloneArguments(m_szCmdLine);

	ZGetGameInterface()->SetState(m_nInitialState);
}


bool ZApplication::InitLocale()
{
	ZGetLocale()->Init(GetCountryID(ZGetConfiguration()->GetLocale()->strCountry.c_str()) );

	ZGetStringResManager()->Init("system/", ZGetLocale()->GetLanguage(), GetFileSystem());


	return true;
}

bool ZApplication::GetSystemValue(const char* szField, char* szData, int maxlen)
{
	return MRegistry::Read(HKEY_CURRENT_USER, szField, szData, maxlen);
}

void ZApplication::SetSystemValue(const char* szField, const char* szData)
{
	MRegistry::Write(HKEY_CURRENT_USER, szField, szData);
}



void ZApplication::InitFileSystem()
{
	m_FileSystem.Create("./","update");

#ifdef _PUBLISH
	m_fileCheckList.Open(FILENAME_FILELIST,&m_FileSystem);
	m_FileSystem.SetFileCheckList(&m_fileCheckList);
#endif

	RSetFileSystem(ZApplication::GetFileSystem());
}
