#ifndef _ZSOUNDENGINE_H
#define _ZSOUNDENGINE_H

class ZLoadingProgress;

#ifdef _BIRDSOUND

#include "RealSound2.h"
#include "MZFileSystem.h"

#include <map>
#include <string>
using namespace std;

struct MMatchItemDesc;
struct FSOUND_SAMPLE;

#pragma comment(lib, "dxguid.lib")

class ZSoundEngine : public RealSound2
{
private:
protected:
	virtual void OnCreate();
	virtual void OnUpdate();
	bool LoadResource( char* pFileName);
	void GetSoundName(const char* szSrcSoundName, char* out);
public:
	ZSoundEngine() { }
	virtual ~ZSoundEngine() { }

	// 정식 편의 함수
	bool OpenMusic(int nBgmIndex);
	int PlaySoundCharacter(const char* szSoundName, rvector& pos, bool bHero, int nPriority=128);
	//int PlaySound(const char* szSoundName, rvector& pos, int nPriority=128, DWORD dwDelay=0);
	void PlaySound(char* Name,rvector& pos,bool bHero=false, bool bLoop = false, DWORD dwDelay = 0 );

	virtual int PlaySound(const char* szSoundName, int nPriority=128);
	virtual int PlaySound(const char* szSoundName, float* pos, float* vel, int nPriority=128);


	void SetEffectVolume(float fVolume);
	void SetEffectMute(bool bMute);



	// 편의함수
	int PlaySE( FSOUND_SAMPLE* pFS, rvector& pos, int Priority , bool bPlayer = false, bool bLoop = false ) { }
	//void PlaySound(char* Name,rvector& pos,bool bHero=false, bool bLoop = false, DWORD dwDelay = 0 ) { }
	void PlaySoundElseDefault(char* Name,char* NameDefault,rvector& pos,bool bHero=false, bool bLoop = false, DWORD dwDelay = 0 ) { }
	//void PlaySound(char* Name, bool bLoop=false, DWORD dwDelay = 0 ) { }

	void PlaySEFire(MMatchItemDesc *pDesc, float x, float y, float z, bool bHero=false);
	void PlaySEDryFire(MMatchItemDesc *pDesc, float x, float y, float z, bool bHero=false);
	void PlaySEReload(MMatchItemDesc *pDesc, float x, float y, float z, bool bHero=false);

	// 총알 튕기는 소리
	void PlaySERicochet(float x, float y, float z);
	void PlaySEHitObject( float x, float y, float z, RBSPPICKINFO& info_ );

	// 총알이 몸에 맞는 소리
	void PlaySEHitBody(float x, float y, float z);
	bool Get3DSoundUpdate() const { return false; }
};


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
#else

#include "MZFileSystem.h"
#include "ZActorSound.h"
#include "MQuestNPC.h"

#include <map>
#include <string>
using namespace std;

#pragma comment(lib, "dxguid.lib")

struct MMatchItemDesc;
struct FSOUND_SAMPLE;

struct SoundSource
{
	FSOUND_SAMPLE* pFS;
	float fMaxDistance; // to Culling
	unsigned long int	nLastPlayedTime;
	SoundSource()
	{
		pFS = 0;
		fMaxDistance = 1000000000.0f; // 2d
		nLastPlayedTime = 0;
	}
};

struct DelaySound
{
	SoundSource* pSS;
	DWORD dwDelay;
	rvector pos;
	int priority;
	bool bPlayer;
	DelaySound()
	{
		pSS = 0;
		dwDelay = 0;
		priority = 0;
		bPlayer = false;
	}
};

struct AmbSound
{
	int type;
	SoundSource* pSS;
	rvector pos[2];		//사각형(min, max)
	float radius;
	rvector center;
	int iChannel;
	float dx, dy, dz;
	char szSoundName[64];	// 사운드 이름.. reset할때 필요함
	AmbSound()
	{
		type = 0;
		pSS = NULL;
		iChannel = -1;
		dx = dy = dz = 0;
		szSoundName[0] = 0;
	}
};

typedef map<string, SoundSource*> SESMAP;
typedef list<DelaySound> DSLIST;
typedef list<AmbSound> ASLIST;

class ZSoundEngine
{
private:
	char					m_SoundFileName[256];	// for Reload
	ZActorSoundManager		m_ASManager;			// NPC용 사운드 관리 클래스
protected:
	char*			m_pMusicBuffer;
	char			m_szOpenedMusicName[256];

	SESMAP			m_SoundEffectSource;
	SESMAP			m_SoundEffectSource2D;
	DSLIST			m_DelaySoundList; // 지연 사운드 리스트
	ASLIST			m_AmbientSoundList; // 환경 사운드 리스트

	float			m_fEffectVolume;
	float			m_fMusicVolume;

	bool			m_bEffectMute;
	bool			m_bMusicMute;
	bool			m_bSoundEnable;
	bool			m_b3DSound;
	bool			m_b3DSoundUpdate;
	rvector			m_ListenerPos;
	bool			m_bInverse;
	bool			m_b8Bits;
	bool			m_bHWMixing;

	DWORD	m_Time;
	DWORD	m_DelayTime;

	bool			m_bEffectVolControl;	
	float			m_fEffectVolFactor;
	float			m_fEffectVolEnd;
	
	bool			m_bBGMVolControl;
	float			m_fBGMVolFactor;
	float			m_fBGMVolEnd;

	MZFileSystem*	m_pfs;
	bool			m_bBattleMusic;
	const char*		GetBGMFileName(int nBgmIndex);
	bool OpenMusic(const char* szFileName, MZFileSystem* pfs);
	bool CheckCulling(const char* szName, SoundSource* pSS, const rvector& vSoundPos, bool bHero, int* pnoutPriority=NULL);
	static void MusicEndCallback(void* pCallbackContext);
public:
	ZSoundEngine();
	virtual ~ZSoundEngine();
	bool Create(HWND hwnd, bool bHWMixing = false, ZLoadingProgress *pLoading = NULL);
	bool Reset(HWND hwnd, bool bHWMixing );
	void Destroy();
	bool LoadResource( char* pFileName_ ,ZLoadingProgress *pLoading = NULL );
	bool Reload();

	// BGM
	bool OpenMusic(int nBgmIndex, MZFileSystem* pfs);
	void CloseMusic();
	void PlayMusic(bool bLoop = true);
	void StopMusic();
	void SetMusicVolume(float fVolume);
	float GetMusicVolume( void);
	void SetMusicMute(bool b);

	/// Sound Effect Source
	void SetEffectVolume(float fVolume);
	void SetEffectVolume( int iChannel, float fVolume );
	void StopLoopSound();
	void StopSound( int iChannel );
	void SetEffectMute(bool b);
	bool SetSamplingBits( bool b8bit );
	void SetInverseSound( bool bInverse )	{ m_bInverse = bInverse; }

	// Game-Specified Sound Effect
	// 이 함수 계열을 사용할 경우 max distance culling, Priority 셋팅이 안됨
	//int PlaySE( FSOUND_SAMPLE* pFS, rvector& pos, bool bPlayer = false, bool bLoop = false );
	//int PlaySE( FSOUND_SAMPLE* pFS, bool bLoop = false );
	int PlaySE( FSOUND_SAMPLE* pFS, const rvector& pos, int Priority , bool bPlayer = false, bool bLoop = false );

	/**
		WEAPON SOUND
	**/
	void PlaySEFire(MMatchItemDesc *pDesc, float x, float y, float z, bool bPlayer=false);
	void PlaySEDryFire(MMatchItemDesc *pDesc, float x, float y, float z, bool bPlayer=false);
	void PlaySEReload(MMatchItemDesc *pDesc, float x, float y, float z, bool bPlayer=false);

	// 총알 튕기는 소리
	void PlaySERicochet(float x, float y, float z);
	void PlaySEHitObject( float x, float y, float z, RBSPPICKINFO& info_ );

	// 총알이 몸에 맞는 소리
	void PlaySEHitBody(float x, float y, float z);

	int PlaySound(const char* Name, const rvector& pos, bool bHero=false, bool bLoop = false, DWORD dwDelay = 0 );
	void PlaySoundElseDefault(char* Name,char* NameDefault,rvector& pos,bool bHero=false, bool bLoop = false, DWORD dwDelay = 0 );
	int PlaySound(const char* Name, bool bLoop=false, DWORD dwDelay = 0 ); // only 2D sound
	
	bool isPlayAble(char* name);
	bool isPlayAbleMtrl(char* name);//특수목적용~

	void Run(void);
	void UpdateAmbSound(rvector& Pos, rvector& Ori);
	float GetArea( rvector& Pos, AmbSound& a );

	int GetEnumDeviceCount();
	const char* GetDeviceDescription( int index );

	FSOUND_SAMPLE* GetFS( const char* szName, bool bHero = false );
	SoundSource* GetSoundSource( const char* szName, bool bHero );

	/*
	 *	for Sound Debugging
	 */
	void SetFramePerSecond( int n ) { m_DelayTime = 1000 / n; }
	void Set3DSoundUpdate(bool b);
	bool Get3DSoundUpdate() const { return m_b3DSoundUpdate; }

	// AmbientSound
	void SetAmbientSoundBox( char* Name, rvector& pos1, rvector& pos2, bool b2d = true );
	void SetAmbientSoundSphere( char* Name, rvector& pos, float radius, bool b2d = true );
	void ClearAmbientSound();

	// Special Functions
	void PlayVoiceSound(char* szName);
    
	// 일정시간에 걸쳐 시작 볼륨에서부터 끝 볼륨까지 조절해 주는 함수
	void SetVolumeControlwithDuration( float fStartPercent, float fEndPercent, DWORD dwDuration, bool bEffect, bool bBGM );

	// NPC Sound 관련 함수
	bool LoadNPCResource(MQUEST_NPC nNPC, ZLoadingProgress* pLoading = NULL);
	void ReleaseNPCResources();
	void PlayNPCSound(MQUEST_NPC nNPC, MQUEST_NPC_SOUND nSound, rvector& pos, bool bMyKill=true);
};

#endif	// _BIRDSOUND

#define BGMID_INTRO				0
#define BGMID_LOBBY				1
#define BGMID_BATTLE			2
#define BGMID_FIN				9

#define MAX_BGM	10




#define VOICE_COOL					"voice_cool"		// 적용됨
#define VOICE_NICE					"voice_nice"		// 적용됨
#define VOICE_GREAT					"voice_great"		// 적용됨
#define VOICE_WONDERFUL				"voice_wonderful"	// 적용됨
#define VOICE_KILLEDALL				"voice_perfect"		// 적용됨
#define VOICE_HEADSHOT				"voice_headshot"	// 적용됨
#define VOICE_FANTASTIC				"voice_fantastic"	// 적용됨
#define VOICE_EXCELLENT				""
#define VOICE_UNBELIEVABLE			""
#define VOICE_GET_READY_TO_FIGHT	""
#define VOICE_LETS_ROCK				""
#define VOICE_YOU_WON				""
#define VOICE_YOU_LOSE				"voice_youlose"
#define VOICE_RED_TEAM_WON			"voice_redteamwon"
#define VOICE_BLUE_TEAM_WON			"voice_blueteamwon"
#define VOICE_DRAW_GAME				"voice_drawgame"
#define VOICE_GET_READY				"voice_getready"
#define VOICE_FIGHT					"voice_fight"		// 적용됨
#define VOICE_NEW_INTRUDER			""
#define VOICE_NEW_CHALLENGER		"voice_newchallenger"
#define VOICE_FIRST_KILL			"voice_firstkill"
#define VOICE_DOUBLE_KILL			"voice_doublekill"
#define VOICE_MULTI_KILL			"voice_multikill"
#define VOICE_GOGOGO				""
#define VOICE_GO_LEFT				""
#define VOICE_GO_RIGHT				""
#define VOICE_GO_BACK				""
#define VOICE_FOLLOW_ME				""
#define VOICE_BACK_ME_UP			""
#define VOICE_COVER_ME				""
#define VOICE_ENEMY_IN_SIGHT		""
#define VOICE_THANK_YOU				""
#define VOICE_SORRY					""
#define VOICE_HAHAHA				""
#define VOICE_OOPS					""
#define VOICE_UH_OH					""
#define VOICE_YES					""
#define VOICE_NO					""
#define VOICE_HEAL_ME_PLEASE		""
#define VOICE_MEDIC					""
#define VOICE_HOLD					""
#define VOICE_DEFEND				""
#define VOICE_NEGATIVE				""
#define VOICE_AFFIRMATIVE			""

#define VOICE_MAX					45






















#endif