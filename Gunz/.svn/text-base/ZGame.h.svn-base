#ifndef _ZGAME_H
#define _ZGAME_H

//#pragma once

#include "ZPrerequisites.h"

#include "MMatchClient.h"
#include "MDataChecker.h"

#include "RTypes.h"
#include "RBspObject.h"
#include "ZMatch.h"
#include "ZGameTimer.h"
#include "ZWater.h"
#include "ZClothEmblem.h"
#include "ZEffectManager.h"
#include "ZWeaponMgr.h"
#include "ZHelpScreen.h"
#include "ZCharacterManager.h"
#include "ZObjectManager.h"
#include "ZWorld.h"

_USING_NAMESPACE_REALSPACE2

class MZFileSystem;

class ZLoading;
class ZGameAction;
class ZSkyBox;
class ZFile;
class ZObject;
class ZCharacter;
class ZMyCharacter;
class ZMiniMap;
class ZMsgBox;
class ZInterfaceBackground;
class ZCharacterSelectView;
class ZScreenEffectManager;
class ZPlayerMenu;
class ZGameClient;
class ZMapDesc;
class ZReplayLoader;


// Game Loop 시작하기 전의 초기화및 싱크완료 검사에 쓰임
enum ZGAME_READYSTATE {
	ZGAME_READYSTATE_INIT,
	ZGAME_READYSTATE_WAITSYNC,
	ZGAME_READYSTATE_RUN
};

// game 에서 pick 되어져 나온 결과.. pCharacter(캐릭터) 혹은 pNode(맵)둘중에 하나의 결과만 나온다.
struct ZPICKINFO {

	// 캐릭터가 결과인 경우
//	ZCharacter *pCharacter;
	ZObject*	pObject;
	RPickInfo	info;

	// 맵이 결과인경우.
	bool bBspPicked;
	RBSPPICKINFO bpi;
};

struct ZObserverCommandItem {
	float fTime;
	MCommand *pCommand;
};

class ZObserverCommandList : public list<ZObserverCommandItem*> {
public:
	~ZObserverCommandList() { Destroy(); }
	void Destroy() {
		while(!empty())
		{
			ZObserverCommandItem *pItem=*begin();
			delete pItem->pCommand;
			delete pItem;
			erase(begin());
		}
	}
};

class ZGame {
protected:
	enum ZGAME_LASTTIME
	{
		ZLASTTIME_HPINFO		= 0,
		ZLASTTIME_BASICINFO,
		ZLASTTIME_PEERPINGINFO,
		ZLASTTIME_SYNC_REPORT,
		ZLASTTIME_MAX
	};

//	ZWorld				*m_pWorld;
	ZGameAction			*m_pGameAction;
	MDataChecker		m_DataChecker;
	ZGameTimer			m_GameTimer;
	float				m_fTime;
	
	DWORD				m_nLastTime[ZLASTTIME_MAX];
	
	
	ZGAME_READYSTATE	m_nReadyState;

	void OnPreDraw();
	bool OnRuleCommand(MCommand* pCommand);
public:

	RParticles			*m_pParticles;

	ZMyCharacter*		m_pMyCharacter;
	ZCharacterManager	m_CharacterManager;
	ZObjectManager		m_ObjectManager;

	RVisualMeshMgr		m_VisualMeshMgr;

	ZEffectManager*		m_pEffectManager;
	ZWeaponMgr			m_WeaponManager;
	
	int					m_render_poly_cnt;

	ZHelpScreen	m_HelpScreen;
public:

	ZGame();
	virtual ~ZGame();

	bool Create(MZFileSystem *pfs, ZLoadingProgress *pLoading);

	void Draw();
	void Draw(MDrawContextR2 &dc);
	void Update(float fElapsed);
	void Destroy();

	void OnCameraUpdate(float fElapsed);
	void OnInvalidate();
	void OnRestore();

	void ParseReservedWord(char* pszDest, const char* pszSrc);

	bool m_bShowWireframe;

	void ShowReplayInfo( bool bShow);

	void OnExplosionGrenade(MUID uidOwner,rvector pos,float fDamage,float fRange,float fMinDamage,float fKnockBack,MMatchTeam nTeamID);
	void OnExplosionMagic(ZWeaponMagic *pWeapon, MUID uidOwner,rvector pos,float fMinDamage,float fKnockBack,MMatchTeam nTeamID,bool bSkipNpc);
	void OnExplosionMagicNonSplash(ZWeaponMagic *pWeapon, MUID uidOwner, MUID uidTarget, rvector pos, float fKnockBack);
	void OnReloadComplete(ZCharacter *pCharacter);
	void OnPeerShotSp(MUID& uid, float fShotTime, rvector& pos, rvector& dir,int type,MMatchCharItemParts sel_type);
	void OnChangeWeapon(MUID& uid, MMatchCharItemParts parts);

	rvector GetMyCharacterFirePosition(void);

	void CheckMyCharDead(float fElapsed);

	void CheckStylishAction(ZCharacter* pCharacter);
	void CheckCombo( ZCharacter *pOwnerCharacter , ZObject *pHitObject ,bool bPlaySound);
	void UpdateCombo(bool bShot = false );
	//void AssignCommander(const MUID& uidRedCommander, const MUID& uidBlueCommander);
	//void SetGameRuleInfo(const MUID& uidRedCommander, const MUID& uidBlueCommander);

	void PostBasicInfo();
	void PostHPInfo();
	void PostHPAPInfo();
	void PostPeerPingInfo();
	void PostSyncReport();

	int  SelectSlashEffectMotion(ZCharacter* pCharacter);
	bool CheckWall(ZObject* pObj1,ZObject* pObj2);

	void InitRound();
	void AddEffectRoundState(MMATCH_ROUNDSTATE nRoundState, int nArg);

	bool CreateMyCharacter(MTD_CharInfo* pCharInfo);
	void DeleteCharacter(const MUID& uid);
	void RefreshCharacters();
	void ConfigureCharacter(const MUID& uidChar, MMatchTeam nTeam, unsigned char nPlayerFlags);

	bool OnCommand(MCommand* pCommand);
	bool OnCommand_Immidiate(MCommand* pCommand);

	// 녹화 & 재생
	void ToggleRecording();
	void StartRecording();
	void StopRecording();

	bool OnLoadReplay(ZReplayLoader* pLoader);
	bool IsReplay() { return m_bReplaying; }
	bool IsShowReplayInfo() { return m_bShowReplayInfo; }
	void EndReplay();

	void OnReplayRun();


	// 옵저버 
	void OnObserverRun();
	void OnCommand_Observer(MCommand* pCommand);
	void FlushObserverCommands();
	int	GetObserverCommandListCount() { return (int)m_ObserverCommandList.size(); }

	void ReserveObserver();
	void ReleaseObserver();

	// 외부에서 참조할만한 것들
	ZMatch* GetMatch()				{ return &m_Match; }
	ZMapDesc* GetMapDesc()			{ return GetWorld() ? GetWorld()->GetDesc() : NULL; }
	ZWorld* GetWorld()				{ return ZGetWorldManager()->GetCurrent(); }

	ZGameTimer* GetGameTimer()		{ return &m_GameTimer; }
	unsigned long GetTickTime()		{ return m_GameTimer.GetGlobalTick(); }
	float GetTime()					{ return m_fTime; }

	int GetPing(MUID& uid);

	MDataChecker* GetDataChecker()	{ return &m_DataChecker; }

	rvector GetFloor(rvector pos,rplane *pimpactplane=NULL);

	bool Pick(ZObject *pOwnerObject,rvector &origin,rvector &dir,ZPICKINFO *pickinfo,DWORD dwPassFlag=RM_FLAG_ADDITIVE | RM_FLAG_HIDE,bool bMyChar=false);
	bool PickTo(ZObject *pOwnerObject,rvector &origin,rvector &to,ZPICKINFO *pickinfo,DWORD dwPassFlag=RM_FLAG_ADDITIVE | RM_FLAG_HIDE,bool bMyChar=false);
	bool PickHistory(ZObject *pOwnerObject,float fTime,rvector &origin,rvector &to,ZPICKINFO *pickinfo,DWORD dwPassFlag,bool bMyChar=false);
	bool ObjectColTest(ZObject* pOwner, rvector& origin, rvector& to, float fRadius, ZObject** poutTarget);

	char* GetSndNameFromBsp(const char* szSrcSndName, RMATERIAL* pMaterial);

	bool CheckGameReady();
	ZGAME_READYSTATE GetReadyState()			{ return m_nReadyState; }
	void SetReadyState(ZGAME_READYSTATE nState)	{ m_nReadyState = nState; }

	bool GetSpawnRequested()			{ return m_bSpawnRequested; }
	void SetSpawnRequested(bool bVal)	{ m_bSpawnRequested = bVal; }

	bool GetUserNameColor(MUID uid,MCOLOR& color,char* sp_name);
	bool IsAttackable(ZObject *pAttacker, ZObject *pTarget);

protected:
	int	m_nGunzReplayNumber;	// 메시지출력을 위한 변수
	ZFile *m_pReplayFile;
//	FILE *m_pRecordingFile;
	bool m_bReplaying;
	bool m_bShowReplayInfo;

	bool m_bRecording;
	bool m_bReserveObserver;

	bool m_bSuicide;
	DWORD m_dwReservedSuicideTime;

	unsigned long int m_nReservedObserverTime;
	int m_t;
	ZMatch				m_Match;
	unsigned long int	m_nSpawnTime;
	bool				m_bSpawnRequested;

	ZObserverCommandList m_ObserverCommandList;		// observer 상태일때 command 를 저장해두는 곳
	ZObserverCommandList m_ReplayCommandList;		// replay 상태일때 command 를 저장해두는 곳, 또 녹화할때 저장하는 곳.

	ZObserverCommandList m_DelayedCommandList;		// 지연시간을 가지는 command 들이다. ex) 띄우기,칼질

	void CheckKillSound(ZCharacter* pAttacker);
	
	void OnReserveObserver();
	void DrawDebugInfo();

	void OnStageEnterBattle(MCmdEnterBattleParam nParam, MTD_PeerListNode* pPeerNode);
	void OnStageLeaveBattle(const MUID& uidChar, const MUID& uidStage);
	void OnPeerList(const MUID& uidStage, void* pBlob, int nCount);
	void OnAddPeer(const MUID& uidChar, DWORD dwIP, const int nPort =	
		MATCHCLIENT_DEFAULT_UDP_PORT, MTD_PeerListNode* pNode = NULL);
	void OnGameRoundState(const MUID& uidStage, int nRound, int nRoundState, int nArg);

	void OnGameResponseTimeSync(unsigned int nLocalTimeStamp, unsigned int nGlobalTimeSync);
	void OnEventUpdateJjang(const MUID& uidChar, bool bJjang);

	// 사라진듯.
//	void OnPeerShot_Item(ZCharacter* pOwnerCharacter,float fShotTime, rvector& pos, rvector& dir,int type);

	void OnPeerDead(const MUID& uidAttacker, const unsigned long int nAttackerArg, 
					const MUID& uidVictim, const unsigned long int nVictimArg);
	void OnReceiveTeamBonus(const MUID& uidChar, const unsigned long int nExpArg);
	void OnPeerDie(MUID& uidVictim, MUID& uidAttacker);
	void OnPeerDieMessage(ZCharacter* pVictim, ZCharacter* pAttacker);
	void OnChangeParts(MUID& uid,int partstype,int PartsID);
	//void OnAssignCommander(const MUID& uidRedCommander, const MUID& uidBlueCommander);
	void OnAttack(MUID& uid,int type,rvector& pos);
	void OnDamage(MUID& uid,MUID& tuid,int damage);
	void OnPeerReload(MUID& uid);
	void OnPeerSpMotion(MUID& uid,int nMotionType);
	void OnPeerChangeCharacter(MUID& uid);
	void OnPeerSpawn(MUID& uid, rvector& pos, rvector& dir);

	void OnSetObserver(MUID& uid);
	

//	void OnPeerAdd();
	void OnPeerBasicInfo(MCommand *pCommand,bool bAddHistory=true,bool bUpdate=true);
	void OnPeerHPInfo(MCommand *pCommand);
	void OnPeerHPAPInfo(MCommand *pCommand);
	void OnPeerPing(MCommand *pCommand);
	void OnPeerPong(MCommand *pCommand);
	void OnPeerOpened(MCommand *pCommand);
	void OnPeerDash(MCommand* pCommand);

		
	bool FilterDelayedCommand(MCommand *pCommand);
	void ProcessDelayedCommand();

	// 투표는 봉인
	//	void AdjustGlobalTime();
	//	void AdjustMyData();

	void OnLocalOptainSpecialWorldItem(MCommand* pCommand);
	void OnResetTeamMembers(MCommand* pCommand);
public:

	void AutoAiming();

	void OnPeerShot(MUID& uid, float fShotTime, rvector& pos, rvector& to,MMatchCharItemParts sel_type);

	void PostSpMotion(ZC_SPMOTION_TYPE type);

	// peershot이 너무 길어져서 분리
	void OnPeerShot_Melee(const MUID& uidOwner, float fShotTime);
	void OnPeerShot_Range(MMatchCharItemParts sel_type, const MUID& uidOwner, float fShotTime, rvector& pos, rvector& to);
	void OnPeerShot_Shotgun(ZItem *pItem, ZCharacter* pOwnerCharacter, float fShotTime, rvector& pos, rvector& to);

    void ReserveSuicide( void);
	bool IsReservedSuicide( void)		{ return m_bSuicide; }
	void CancelSuicide( void)			{ m_bSuicide = false; }
	ZObserverCommandList* GetReplayCommandList()  { return &m_ReplayCommandList;} 
};


extern ZGame* g_pGame;
extern MUID g_MyChrUID;
extern float g_fFOV;

ZCharacterManager*	ZGetCharacterManager();
ZObjectManager*		ZGetObjectManager();
bool IsMyCharacter(ZObject* pObject);

/*
// Damage 계산에 필요한 사항
#define DAMAGE_MELEE_HEAD	0.6f
#define DAMAGE_MELEE_CHEST	0.6f
#define DAMAGE_MELEE_HANDS	0.6f
#define DAMAGE_MELEE_LEGS	0.6f
#define DAMAGE_MELEE_FEET	0.6f

#define DAMAGE_RANGE_HEAD	0.8f
#define DAMAGE_RANGE_CHEST	0.5f
#define DAMAGE_RANGE_HANDS	0.5f
#define DAMAGE_RANGE_LEGS	0.5f
#define DAMAGE_RANGE_FEET	0.5f
*/
#define MAX_COMBO 99

#define PEERMOVE_TICK			100		// 0.1초 마다 이동메세지를 보낸다 (초당 10회)
#define PEERMOVE_AGENT_TICK		100		// Agent를 통하면 초당 10번 메시지를 보낸다.


#endif