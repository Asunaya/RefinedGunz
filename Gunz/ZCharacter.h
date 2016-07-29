#ifndef _ZCHARACTER_H
#define _ZCHARACTER_H

//#pragma	once

#include "MRTTI.h"
#include "ZCharacterObject.h"
//#include "ZActor.h"
#include "MUID.h"
#include "RTypes.h"
#include "RPathFinder.h"
#include "RVisualMeshMgr.h"

#include "MObjectTypes.h"
#include "MObjectCharacter.h"
#include "ZItem.h"
#include "ZCharacterItem.h"
#include "MMatchObject.h"
#include "RCharCloth.h"
#include "ZFile.h"
#include "Mempool.h"

#include "ZModule_HPAP.h"

#include <list>
#include <string>

#include "ZCharacterStructs.h"
#include "AnimationStuff.h"
#include "BasicInfoHistory.h"
#include "ZReplayStructs.h"

using namespace std;

_USING_NAMESPACE_REALSPACE2

#define MAX_SPEED			1000.f			// 최대속도..
#define RUN_SPEED			630.f			// 달리는 속도
#define BACK_SPEED			450.f			// 뒤나 옆으로 갈때 속도
#define ACCEL_SPEED			7000.f			// 가속도
#define STOP_SPEED			3000.f			// 아무키도 안눌렀을때 감속도..
#define STOP_FORMAX_SPEED	7100.f			// 달리는 속도 이상으로 올라갔을때 빨리 감속하는 속도

#define CHARACTER_RADIUS	35.f		// 캐릭터 충돌 반지름
#define CHARACTER_HEIGHT	180.0f		// 캐릭터 충돌 높이

#define ARRIVAL_TOLER		5.f

class ZShadow;

struct ZFACECOSTUME
{
	char* szMaleMeshName;
	char* szFemaleMeshName;
};


enum ZC_SKILL {

	ZC_SKILL_NONE = 0,

	ZC_SKILL_UPPERCUT,
	ZC_SKILL_SPLASHSHOT,
	ZC_SKILL_DASH,
	ZC_SKILL_CHARGEDSHOT,

	ZC_SKILL_END
};


enum ZC_DIE_ACTION
{
	ZC_DIE_ACTION_RIFLE = 0,
	ZC_DIE_ACTION_KNIFE,
	ZC_DIE_ACTION_SHOTGUN,
	ZC_DIE_ACTION_ROCKET,

	ZC_DIE_ACTION_END
};

enum ZC_SPMOTION_TYPE {

	ZC_SPMOTION_TAUNT = 0,
	ZC_SPMOTION_BOW ,
	ZC_SPMOTION_WAVE ,
	ZC_SPMOTION_LAUGH ,
	ZC_SPMOTION_CRY ,
	ZC_SPMOTION_DANCE ,

	ZC_SPMOTION_END
};

enum ZC_WEAPON_SLOT_TYPE {

	ZC_SLOT_MELEE_WEAPON = 0,
	ZC_SLOT_PRIMARY_WEAPON,
	ZC_SLOT_SECONDARY_WEAPON,
	ZC_SLOT_ITEM1,
	ZC_SLOT_ITEM2,

	ZC_SLOT_END,
};

enum ZSTUNTYPE {
	ZST_NONE	=	-1,
	ZST_DAMAGE1	=	0,
	ZST_DAMAGE2,
	ZST_SLASH,			// 강베기 스턴
	ZST_BLOCKED,		// 칼 막혔을때 스턴
	ZST_LIGHTNING,		// 인챈트중 Lightning
	ZST_LOOP,			// 스킬중 root 속성
};


class ZSlot {
public:
	ZSlot() {
		m_WeaponID = 0;
	}

	int m_WeaponID;
};

// 이것은 캐릭터끼리 주고받는 데이터로 나중에 투표 판정의 근거가 된다.
/*
struct ZHPItem {
	MUID muid;
	float fHP;
};
*/

//struct ZHPInfoItem : public CMemPoolSm<ZHPInfoItem>{
//	ZHPInfoItem()	{ pHPTable=NULL; }
//	~ZHPInfoItem()	{ if(pHPTable) delete pHPTable; }
//	
//	int		nCount;
//	ZHPItem *pHPTable;
//};


//class ZHPInfoHistory : public list<ZHPInfoItem*> {
//};

#define CHARACTER_ICON_DELAY		2.f
#define CHARACTER_ICON_FADE_DELAY	.2f
#define CHARACTER_ICON_SIZE			32.f		// 아이콘 크기 (640x480기준)

class ZModule_HPAP;
class ZModule_QuestStatus;

/// 캐릭터 클래스
class ZCharacter : public ZCharacterObject
{
	MDeclareRTTI;
	//friend class ZCharacterManager;
private:
protected:

	// 모듈들. 한번 생성되고 소멸될때 같이 지운다
	ZModule_HPAP			*m_pModule_HPAP;
	ZModule_QuestStatus		*m_pModule_QuestStatus;
	ZModule_Resistance		*m_pModule_Resistance;
	ZModule_FireDamage		*m_pModule_FireDamage;
	ZModule_ColdDamage		*m_pModule_ColdDamage;
	ZModule_PoisonDamage	*m_pModule_PoisonDamage;
	ZModule_LightningDamage	*m_pModule_LightningDamage;
	

	ZCharacterProperty		m_Property;		///< HP 등의 캐릭터 속성
	ZCharacterStatus		m_Status;		///< 플레이어 상태값

	MTD_CharInfo			m_InitialInfo;	///< 초기정보

	char	m_szUserName[MATCHOBJECT_NAME_LENGTH];			///< 유져이름(운영자는 '운영자')
	char	m_szUserAndClanName[MATCHOBJECT_NAME_LENGTH];	///< 유저이름(클랜이름)


	// 패킹
	union {
		struct {
			bool	m_bAdminHide:1;					///< admin hide 되어있는지..
			bool	m_bDie:1;						///< 죽었는지 체크
			bool	m_bStylishShoted:1;				///< 마지막으로 쏜게 스타일리쉬 했는지 체크
			bool	m_bFallingToNarak:1;			///< 나락으로 떨어지고 있는지 여부
			bool	m_bStun:1;						///< stun ..움직일수없게된상태.
			bool	m_bDamaged:1;					///< 데미지 여부
			
			bool	m_bPlayDone:1;				// 애니메이션 플레이가 다 되었는지. 다음동작으로 넘어가는 기준
			bool	m_bPlayDone_upper:1;		// 상체 애니메이션 플레이가 다 되었는지. 다음동작으로 넘어가는 기준
			bool	m_bIsLowModel:1;
			bool	m_bTagger:1;					///< 술래

		};
		DWORD dwFlagsProtected;
	}; // 패킹 끝


	ZSTUNTYPE	m_nStunType;				///< 맞는 애니메이션 종류.. 2:마지막타격,4:lightning,5:루프

	int			m_nKillsThisRound;	///< 이번라운드에서의 kills ( unbelievable 판정)
	float		m_fLastKillTime;	///< 마지막에 죽인 시간 (excellent)를 표시하기 위함
	ZDAMAGETYPE	m_LastDamageType;	///< 마지막 데미지 타입		
	MMatchWeaponType m_LastDamageWeapon;///< 마지막 데미지 무기..
	rvector		m_LastDamageDir;	///< 마지막 데미지 방향 ( 죽는 모션을 결정 )
	float		m_LastDamageDot;
	float		m_LastDamageDistance;

	MUID		m_LastThrower;		///< 마지막 띄운 사람
	float		m_tmLastThrowClear;	///< 마지막 띄운 사람 잊어도 되는시간

	int			m_nWhichFootSound;	///< 발소리를 번갈아 내기위해 어느 발인지 저장한 변수

	DWORD		m_dwInvincibleStartTime;		// 무적의 시작 시간
	DWORD		m_dwInvincibleDuration;			// 무적의 지속시간

	virtual void UpdateSound();

	void InitMesh();	///< 캐릭터 파츠 등의 메쉬정보 세팅. InitCharInfo에서 호출	
	void InitProperties();

//	float m_fIconStartTime[ZCI_END];	///< 머리위에 뜨는 아이콘들

	void CheckLostConn();
	virtual void OnLevelDown();
	virtual void OnLevelUp();
	virtual void OnDraw();
//	virtual void RegisterModules();
	virtual void	OnDie();

public:
	BasicInfoHistoryManager BasicInfoHistory;

	bool GetHistory(rvector *pos, rvector *direction, float fTime) override;
	void GetPositions(v3* Head, v3* Foot, double Time);

	float	m_fLastValidTime;		// Dead Reckoning에 필요한 변수 -> 지금 코드에서 필요없어보임
//	float	m_fDistToFloor;			// 바닥까지의 거리
//	rplane	m_FloorPlane;			// 바닥 평면의 방정식
//	float	m_fFallHeight;			// 낙하가 시작된 시점

	// 1bit 패킹
	union {
		struct {
			bool	m_bLand:1;				// 지금 발을 땅에 대고있는지..
			bool	m_bWallJump:1;			// 벽점프 중인지
			bool	m_bJumpUp:1;			// 점프올라가는중
			bool	m_bJumpDown:1;			// 내려가는중
			bool	m_bWallJump2:1;			// 이건 walljump 후에 착지시 두번째 튕겨져 나오는 점프..
			bool	m_bTumble:1;			// 덤블링 중
			bool	m_bBlast:1;				// 띄워짐당할때 ( 올라갈때 )
			bool	m_bBlastFall:1;			// 띄워져서 떨어질때
			bool	m_bBlastDrop:1;			// 떨어지다 땅에 튕길때
			bool	m_bBlastStand:1;		// 일어날때
			bool	m_bBlastAirmove:1;		// 공중회전후 착지
			bool	m_bSpMotion:1;
			bool	m_bCommander:1;			///< 대장
			bool	m_bCharging:1;			// 힘모으고 있는중
			bool	m_bCharged:1;			// 힘모인상태
			bool	m_bLostConEffect:1;		// 네트웍 응답이 없을때 머리에 뜨는 이펙트가 나와야 하는지.
			bool	m_bChatEffect:1;		// 채팅시 머리에 뜨는 이펙트가 나와야 하는지.
			bool	m_bBackMoving:1;		// 뒤로 이동할때
		};
		DWORD dwFlagsPublic;
	}; // 패킹 끝


	float	m_fChargedFreeTime;		// 힘모인게 풀리는 시간
	int		m_nWallJumpDir;			// 벽점프하는 방향
	int		m_nBlastType;			// 단도계열추가~


	ZC_STATE_LOWER	m_SpMotion;

	
	/*
	bool	m_bClimb;				// 턱등에 올라가고있는 경우
	rvector	m_ClimbDir;				// 올라가는 방향
	float	m_fClimbZ;				// 올라가기 시작한 높이
	rplane	m_ClimbPlane;
	*/

//	bool	m_bRendered;				///< 이전프레임에서 화면에 나왔는지

	/////// 네트웍에서 임시로 옷 갈아 입기 위한 현재 선택된 파츠정보
	int m_t_parts[6];	//남자
	int m_t_parts2[6];	//여자
	
	
	rvector		m_vProxyPosition, m_vProxyDirection;
	
//	ZHPInfoHistory		m_HPHistory;		///< 투표를 위해 몇초간의 데이터를 가지고있는다

	ZCharacter();
	virtual ~ZCharacter();

	virtual bool Create(const MTD_CharInfo& pCharInfo);
	virtual void Destroy();
	
	void InitMeshParts();
	
	void EmptyHistory();

	void Draw() {
		OnDraw();
	}

	rvector m_TargetDir;
	rvector m_DirectionLower,m_DirectionUpper;
	rvector m_RealPositionBefore;			// 애니메이션의 움직임을 추적하기 위한 변수
	rvector m_AnimationPositionDiff;
	rvector m_Accel;


	ZC_STATE_UPPER	m_AniState_Upper;		// 상체 애니메이션 상태
	ZC_STATE_LOWER	m_AniState_Lower;		// 하체 애니메이션 상태 (기본)
	ZANIMATIONINFO *m_pAnimationInfo_Upper,*m_pAnimationInfo_Lower;

	void AddIcon(int nIcon);
//	float GetIconStartTime(int nIcon);

	int		m_nVMID;	// VisualMesh ID
	//MUID	m_UID;		// 서버에서 부여한 캐릭터의 UID
	MMatchTeam		m_nTeamID;	// Team ID

	MCharacterMoveMode		m_nMoveMode;
	MCharacterMode			m_nMode;
	MCharacterState			m_nState;

//	RVisualMesh*			m_pVMesh;

//	float	m_fLastAdjustedTime;
	float	m_fAttack1Ratio;//칼질등의 경우 첫번째비율을 나중타에도 적용한다..
//	rvector m_AdjustedNormal;

	/*
	bool	m_bAutoDir;

	bool	m_bLeftMoving,m_bRightMoving;
	bool	m_bForwardMoving;
	*/

//	float	m_fLastUpdateTime;
	float	m_fLastReceivedTime;	// basicinfo 데이터를 마지막 받은 시간

	float	m_fTimeOffset;				// 나와 이 캐릭터의 시간차이
	float	m_fAccumulatedTimeError;	// 현재시간의 누적된 오차
	int		m_nTimeErrorCount;			// 현재시간의 오차가 누적된 회수


	float	m_fGlobalHP;			// 다른사람들이 볼때 이캐릭터의 HP의 평균.. 투표를 위함.
	int		m_nReceiveHPCount;		// 평균내기위한...

	/*
	float	m_fAveragePingTime;				// 일정시간 평균 네트웍 지연 시간

	// 글로벌 시간과의 차이를 누적한다. 이 숫자가 점점 커지는 유저는 스피드핵이 의심된다.
	#define TIME_ERROR_CORRECTION_PERIOD	20

	int		m_nTimeErrorCount;
	float	m_TimeErrors[TIME_ERROR_CORRECTION_PERIOD];
	float	m_fAccumulatedTimeError;

	list<float> m_PingData;			// 몇개의 핑 타임을 가지고 평균을 낸다.
	*/

//	DWORD m_dwBackUpTime;

	// 무기 발사속도의 이상유무를 검출한다.
	int		m_nLastShotItemID;
	float	m_fLastShotTime;

	void	SetInvincibleTime(int nDuration)
	{
		m_dwInvincibleStartTime = GetGlobalTimeMS();
		m_dwInvincibleDuration = nDuration;
	}

	bool	isInvincible();

	bool IsMan();

	virtual void  OnUpdate(float fDelta);
	virtual void  UpdateSpeed();
	virtual float GetMoveSpeedRatio();

	virtual void UpdateVelocity(float fDelta);	// 적당한 속도로 감속
	virtual void UpdateHeight(float fDelta);		// 높이와 폴링 데미지를 검사.
	virtual void UpdateMotion(float fDelta=0.f);	// 허리돌리기등의 애니메이션 상태관련
	virtual void UpdateAnimation();

	void UpdateLoadAnimation();

	void Stop();
	//void DrawForce(bool bDrawShadow);	

	void CheckDrawWeaponTrack();
	void UpdateSpWeapon();

	void SetAnimation(char *AnimationName,bool bEnableCancel,int tick);
	void SetAnimation(RAniMode mode,char *AnimationName,bool bEnableCancel,int tick);

	void SetAnimationLower(ZC_STATE_LOWER nAni);
	void SetAnimationUpper(ZC_STATE_UPPER nAni);
	
	ZC_STATE_LOWER GetStateLower() { return m_AniState_Lower; }
	ZC_STATE_UPPER GetStateUpper() { return m_AniState_Upper; }

	bool IsUpperPlayDone()	{ return m_bPlayDone_upper; }

	bool IsMoveAnimation();		// 움직임이 포함된 애니메이션인가 ?

//	bool IsRendered()		{ return m_bRendered; }

	bool IsTeam(ZCharacter* pChar);

	bool IsRunWall();
	bool IsMeleeWeapon();
	virtual bool IsCollideable();

//	void SetAnimationForce(ZC_STATE nState, ZC_STATE_SUB nStateSub) {}

	void SetTargetDir(rvector vDir); 

//	bool Pick(int x,int y,RPickInfo* pInfo);
//	bool Pick(int x,int y,rvector* v,float* f);
	virtual bool Pick(rvector& pos,rvector& dir, RPickInfo* pInfo = NULL);

//	bool Move(rvector &diff);

	virtual void OnChangeWeapon(char* WeaponModelName);
	void OnChangeParts(RMeshPartsType type,int PartsID);
	void OnAttack(int type,rvector& pos);
//	void OnHeal(ZCharacter* pAttacter,int type,int heal);
	void OnShot();

	void ChangeWeapon(MMatchCharItemParts nParts);

	int GetLastShotItemID()	{ return m_nLastShotItemID; }
	float GetLastShotTime()						{ return m_fLastShotTime; }
	bool CheckValidShotTime(int nItemID, float fTime, ZItem* pItem);
	void UpdateValidShotTime(int nItemID, float fTime) 
	{ 
		m_nLastShotItemID = nItemID;
		m_fLastShotTime = fTime;
	}

	bool IsDie() { return m_bDie; }
	bool IsAlive() const { return !m_bDie; }
	void ForceDie() { SetHP(0); m_bDie = true; }		// 이것은 그냥 죽은 상태로 만들기 위할때 사용

	void SetAccel(rvector& accel) { m_Accel = accel; }
	virtual void SetDirection(rvector& dir);

	int		m_nSelectSlot;
	ZSlot	m_Slot[ZC_SLOT_END];

	ZCharacterStatus* GetStatus()	{ return &m_Status; }

	// Character Property
	ZCharacterProperty* GetProperty() { return &m_Property; }

	MMatchUserGradeID GetUserGrade()	{ return m_InitialInfo.nUGradeID; }
	unsigned int GetClanID()	{ return m_InitialInfo.nClanCLID; }

	void SetName(const char* szName) { strcpy_safe(m_Property.szName, szName); }

	const char *GetUserName()			{ return m_szUserName;	}		// 운영자는 처리됨
	const char *GetUserAndClanName()	{ return m_szUserAndClanName; }	// 운영자는 클랜표시 안함
	bool IsAdmin();
	bool IsAdminHide()			{ return m_bAdminHide;	}
	void SetAdminHide(bool bHide) { m_bAdminHide = bHide; }

	int GetHP();
	int GetAP();
	void SetHP(int nHP);
	void SetAP(int nAP);
//	void SetMoveSpeed(int nMoveSpeed) { m_Property.nMoveSpeed = nMoveSpeed; }
//	void SetWeight(int nWeight) { m_Property.nWeight = nWeight; }
//	void SetMaxWeight(int nMaxWeight) { m_Property.nMaxWeight = nMaxWeight; }
//	void SetSafeFall(int nSafeFall) { m_Property.nSafeFall = nSafeFall; }

	int GetKils() { return GetStatus()->nKills; }

	bool CheckDrawGrenade();

//	int GetWeaponEffectType();
//	float GetCurrentWeaponRange();
//	float GetMeleeWeaponRange();

	bool GetStylishShoted() { return m_bStylishShoted; }
	void UpdateStylishShoted();
	
	MUID GetLastAttacker() { return m_pModule_HPAP->GetLastAttacker(); }
	void SetLastAttacker(MUID uid) { m_pModule_HPAP->SetLastAttacker(uid); }
	ZDAMAGETYPE GetLastDamageType() { return m_LastDamageType; }
	void SetLastDamageType(ZDAMAGETYPE type) { m_LastDamageType = type; }

	bool DoingStylishMotion();
	
	bool IsObserverTarget();

	MMatchTeam GetTeamID() { return m_nTeamID; }
	void SetTeamID(MMatchTeam nTeamID) { m_nTeamID = nTeamID; }
	bool IsSameTeam(ZCharacter* pCharacter) 
	{ 
		if (pCharacter->GetTeamID() == -1) return false;
		if (pCharacter->GetTeamID() == GetTeamID()) return true; return false; 
	}
	bool IsTagger() { return m_bTagger; }
	void SetTagger(bool bTagger) { m_bTagger = bTagger; }

	void SetLastThrower(MUID uid, float fTime) { m_LastThrower = uid; m_tmLastThrowClear = fTime; }
	const MUID& GetLastThrower() { return m_LastThrower; }
	float GetLastThrowClearTime() { return m_tmLastThrowClear; }

	// 동작이나 이벤트에 관한 것들.
	//void Damaged(ZCharacter* pAttacker, rvector& dir, RMeshPartsType partstype,MMatchCharItemParts wtype,int nCount=-1);
	//void DamagedGrenade(MUID uidOwner, rvector& dir, float fDamage,int nTeamID);
	//void DamagedFalling(float fDamage);
	//void DamagedKatanaSplash(ZCharacter* pAttacker,float fDamageRange);

	void Revival();
	void Die();
	void ActDead();
	virtual void InitHPAP();
	virtual void InitBullet();
	virtual void InitStatus();
	virtual void InitRound();


	void TestChangePartsAll();
	void TestToggleCharacter();

	virtual void OutputDebugString_CharacterState();

	void ToggleClothSimulation();
	void ChangeLowPolyModel();
	bool IsFallingToNarak() { return m_bFallingToNarak; }

	MMatchItemDesc* GetSelectItemDesc() {
		if(GetItems())
			if(GetItems()->GetSelectedWeapon())
				return GetItems()->GetSelectedWeapon()->GetDesc();
		return NULL;
	}

	void LevelUp();
	void LevelDown();

	void Save(ReplayPlayerInfo& Info);
	void Load(const ReplayPlayerInfo& Info);

	RMesh *GetWeaponMesh(MMatchCharItemParts parts);

	virtual float ColTest(const rvector& pos, const rvector& vec, float radius, rplane* out=0) override;
	virtual bool IsAttackable() override;

	virtual bool IsGuard() const override;
	virtual bool IsGuardCustom() const override {
		return IsGuard();
	}
	virtual void OnMeleeGuardSuccess() override;

	void AddMassiveEffect(const rvector &pos, const rvector &dir);


	virtual void OnDamagedAnimation(ZObject *pAttacker, int type) override;

	// ZObject에 맞게 만든 동작이나 이벤트에 관한 것들.
	virtual ZOBJECTHITTEST HitTest(const rvector& origin, const rvector& to, float fTime, rvector *pOutPos = NULL) override;

	virtual void OnKnockback(const rvector& dir, float fForce) override;
	virtual void OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType,
		float fDamage, float fPiercingRatio=1.f, int nMeleeType=-1) override;
	virtual void OnScream() override;
};

void ZChangeCharParts(RVisualMesh* pVMesh, MMatchSex nSex, int nHair, int nFace, unsigned long int* pItemID);
void ZChangeCharWeaponMesh(RVisualMesh* pVMesh, unsigned long int nWeaponID);
bool CheckTeenVersionMesh(RMesh** ppMesh);

//////////////////////////////////////////////////////////////////////////
//	ENUM
//////////////////////////////////////////////////////////////////////////
enum CHARACTER_LIGHT_TYPE
{
	GUN,
	SHOTGUN,
	CANNON,
	NUM_LIGHT_TYPE,
};

//////////////////////////////////////////////////////////////////////////
//	STRUCT
//////////////////////////////////////////////////////////////////////////
typedef struct
{
	int			iType;
	float		fLife;
	rvector		vLightColor;
	float		fRange;
}	sCharacterLight;





#endif