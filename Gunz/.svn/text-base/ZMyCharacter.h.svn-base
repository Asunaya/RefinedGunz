#ifndef _ZMYCHARACTER_H
#define _ZMYCHARACTER_H


//#pragma once

#include "MRTTI.h"
#include "ZCharacter.h"

enum ZChangeWeaponType
{
	ZCWT_NONE = 0,
	ZCWT_PREV,
	ZCWT_NEXT,
	ZCWT_MELEE,
	ZCWT_PRIMARY,
	ZCWT_SECONDARY,
	ZCWT_CUSTOM1,
	ZCWT_CUSTOM2,
	ZCWT_END,
};

enum ZUsingStamina{
	ZUS_Tumble = 0,// + dash
	ZUS_Jump,
};

enum ZDELAYEDWORK {
	ZDW_SHOT,
	ZDW_UPPERCUT,	// 장검 띄우기
	ZDW_DASH,		// 단검 특수기
	ZDW_SLASH,		// 모아서 칼질
};

struct ZDELAYEDWORKITEM {
	float fTime;		// 실행될 시간
	ZDELAYEDWORK nWork;	// 실행될 사건        
};

typedef list<ZDELAYEDWORKITEM*> ZDELAYEDWORKLIST;


class ZMyCharacter : public ZCharacter {
	MDeclareRTTI;
protected:
	virtual void OnDraw();
public:
#ifdef _DEBUG
	bool m_bGuardTest;
#endif

	float	m_fDeadTime;		// 죽은 시간

	float	m_fNextShotTimeType[MMCIP_END];
	float	m_fNextShotTime;	// 다음번 무기 발사 가능 시간
	float	m_fWallJumpTime;

	int		m_nTumbleDir;

	// 패킹되는 플래그들
	union {
		struct {
			bool	m_bWallHang:1;		// 벽에 매달린 상태일때 true

			bool	m_bLimitJump:1;
			bool	m_bLimitTumble:1;
			bool	m_bLimitWall:1;

			bool	m_bMoveLimit:1;
			bool	m_bMoving:1;

			bool	m_bReleasedJump:1;				// 점프를 한번 하면 떼었다가 점프를 해야한다.
			bool	m_bJumpQueued:1;
			bool	m_bWallJumpQueued:1;
			bool	m_bHangSuccess:1;
			bool	m_bSniferMode:1;

			bool	m_bEnterCharge:1;				// 첫번째 칼질이후 계속 누르고있었는지.

			bool	m_bJumpShot:1;
			bool	m_bShot:1;						// 칼질중일때.
			bool	m_bShotReturn:1;				// 칼질하다 idle 되돌아오는 동작중

			bool	m_bSkill:1;						// 여러가지 오른쪽 스킬
			bool	m_b1ShotSended:1;				// 첫번째 칼질은 딜레이가 있다

			bool	m_bSplashShot:1;				// 마지막 스플래시 칼질 해야 하는지..
			bool	m_bGuard:1;
			bool	m_bGuardBlock_ret:1;
			bool	m_bGuardStart:1;
			bool	m_bGuardCancel:1;
			bool	m_bGuardKey:1;
			bool	m_bGuardByKey:1;
			bool	m_bDrop:1;
			bool	m_bSlash:1;				// 힘모아 베기
			bool	m_bJumpSlash:1;			// 공중에서 힘모아 베기
			bool	m_bJumpSlashLanding:1;	// 이후 착지동작
			bool	m_bReserveDashAttacked:1;

			bool	m_bLButtonPressed:1;			// LButton 눌려져있는지.
			bool	m_bLButtonFirstPressed:1;		// LButton 이 처음으로 눌려진건지
			bool	m_bLButtonQueued:1;				// LButton 예약되어있는지

			bool	m_bRButtonPressed:1;
			bool	m_bRButtonFirstPressed:1;		// RButton 이 처음으로 눌려진건지
			bool	m_bRButtonFirstReleased:1;		// RButton 이 눌려져있다가 떼졌을때
		};

		DWORD dwFlags[2];	// 2005.12.9 현재 35개
	}; // 패킹 끝.

	float	m_fSplashShotTime;

	float	m_fLastJumpPressedTime;			// 점프 예약 (약간 미리 눌러도 뛴다)
	float	m_fJump2Time;					// 이중점프 한 시간
	float	m_fHangTime;
	rvector m_HangPos;

	int		m_nWallJump2Dir;

// 버튼 상태 관리	: 필요하면 더 만든다. UpdateButtonState() 에서 관리
	float	m_fLastLButtonPressedTime;		// LButton 이 처음으로 눌려진 시간
	float	m_fLastRButtonPressedTime;		// RButton 이 눌려진 시간

	int		m_nShot;						// 칼질 연속기중 몇번째 인지
	int		m_nJumpShotType;
	float	m_f1ShotTime;					// 첫번째 칼질 시작한 시간

	float	m_fSkillTime;					// 발동한 시간

	/*
	bool	m_bSkillSended;					// 어퍼컷도 보낼때 딜레이가 있다
	*/

	float	m_fLastShotTime;				// 마지막 총에 맞은 시간
	int		m_nGuardBlock;
	float	m_fGuardStartTime;				// 가드 발동시간

	float	m_fDropTime;			// 누운시간.

	float	m_bJumpSlashTime;		// 공중 강베기 시작시간

	rvector	m_vReserveDashAttackedDir;
	float	m_fReserveDashAttackedTime;
	MUID	m_uidReserveDashAttacker;

	float	m_fStunEndTime;				///< stun 풀리는 시간, 5번타입을 위함


	void WallJump2();

	ZMyCharacter();
	~ZMyCharacter();

	void InitSpawn();//솔로에서는 라운드에 해당..

	void ProcessInput(float fDelta);

	bool CheckWall(rvector& Pos);

//	bool CheckStamina(int v);	// 보류
//	bool UsingStamina(int mode);

//	virtual void UpdateStamina(float fTime);
	virtual void UpdateAnimation();
	virtual void OnUpdate(float fTime);

	virtual void UpdateLimit();
	
	virtual void OnChangeWeapon(char* WeaponModelName);

	void Animation_Reload();

	void OnTumble(int nDir);
	virtual void OnBlast(rvector &dir);
	void OnRecoil(int nControlability);
	void OnDashAttacked(rvector &dir);
	void ReserveDashAttacked(MUID uid, float time,rvector &dir);

	virtual void InitBullet();
	virtual void InitStatus();
	virtual void InitRound();
	
	virtual void SetDirection(rvector& dir);
	virtual void OnDamagedAnimation(ZObject *pAttacker,int type);

	void OutputDebugString_CharacterState();

	float GetCAFactor() { return m_fCAFactor; }
	bool IsGuard();

	void ShotBlocked();			// 칼질이 막혔을때 스턴 먹는다

	void ReleaseButtonState();

private:
	float m_fCAFactor;		// Controllability Factor
	float m_fElapsedCAFactorTime;

	// 일정 시간 뒤에 처리해야 하는 일들이다
	ZDELAYEDWORKLIST m_DelayedWorkList;

	void OnDelayedWork(ZDELAYEDWORKITEM *pItem);
	void AddDelayedWork(float fTime,ZDELAYEDWORK nWork);
	void ProcessDelayedWork();

	virtual void	OnDie();
//	void UpdatePosition(float fDelta);	// 속도에 따라 실제 위치를 이동
	void CalcRangeShotControllability(rvector& vOutDir, rvector& vSrcDir, int nControllability);
	float GetControllabilityFactor();
	void UpdateCAFactor(float fDelta);
	void ReleaseLButtonQueue();

	void UpdateButtonState();

	void ProcessShot();
	void ProcessGadget();
	void ProcessGuard();

	//	void OnGadget_Katana(bool bFirst);
	void OnGadget_Hanging();
	void OnGadget_Snifer();

	void OnShotCustom();
	void OnShotItem();
	void OnShotRocket();
	void OnShotMelee();
	void OnShotRange();

	void Charged();				// 기 모았다
	void EnterCharge();			// 기모으기로 들어간다
	void Discharged();			// 기모으기 풀렸을때
	void ChargedShot();			// 강베기
	void JumpChargedShot();		// 공중 강베기

//	bool IsCounterAttackable();		// 반격기가 나갈수 있는 타이밍인가

	float GetGravityConst();		// 중력의 영향을 얼만큼 받는지..
public:
	virtual void OnGuardSuccess();
	virtual void OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio=1.f, int nMeleeType=-1);
	// knockback을 적용받아야한다
	virtual void OnKnockback(rvector& dir, float fForce);
	virtual void OnMeleeGuardSuccess();

	virtual void OnStun(float fTime);
};



#ifndef _PUBLISH
// 더미 캐릭터 - 테스트로 사용한다.
class ZDummyCharacter : public ZMyCharacter
{
private:
	float m_fNextAniTime;
	float m_fElapsedTime;

	float m_fNextShotTime;
	float m_fShotElapsedTime;

	float m_fShotDelayElapsedTime;

	bool m_bShotting;
	bool m_bShotEnable;
	virtual void  OnUpdate(float fDelta);
public:
	ZDummyCharacter();
	virtual ~ZDummyCharacter();
	

	void SetShotEnable(bool bEnable) { m_bShotEnable = bEnable; }
};
#endif // #ifndef _PUBLISH

#endif