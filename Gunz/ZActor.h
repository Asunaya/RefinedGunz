#pragma once

#include "MRTTI.h"
#include "ZCharacterObject.h"
#include "ZCharacter.h"
#include "MUID.h"
#include "RVisualMeshMgr.h"
#include "MObjectTypes.h"
#include "ZStateMachine.h"
#include "ZAI_Base.h"
#include "MBaseQuest.h"
#include "ZBrain.h"
#include "ZActorAnimation.h"
#include "ZTask.h"
#include "ZTaskManager.h"
#include "ZModule_HPAP.h"

#include "SafeString.h"

#include <list>
#include <string>

_USING_NAMESPACE_REALSPACE2

enum ZACTOR_FLAG
{
	AF_NONE				= 0,
	AF_LAND				= 0x1,				// 발이 땅에 닿아 있는지 여부
	AF_BLAST			= 0x2,				// 띄움을 당한 상태인지 여부
	AF_MOVING			= 0x4,				// 이동여부
	AF_DEAD				= 0x8,				// 죽었다
	AF_REQUESTED_DEAD	= 0x10,				// 서버에 죽음 요청을 했는지 여부
	AF_BLAST_DAGGER		= 0x20,				// 띄움을 당한 상태인지 여부	

	AF_MY_CONTROL		= 0x100,			// 내가 컨트롤하는지 여부


	AF_SOUND_WOUNDED	= 0x1000,			// 한번 맞았을때 울었는지 여부
};

// 일정 시간간격으로 NPC정보 계속 주고받는 데이터
struct ZACTOR_BASICINFO {
	float			fTime;
	MUID			uidNPC;
	short			posx,posy,posz;
	short			velx,vely,velz;
	short			dirx,diry,dirz;
	BYTE			anistate;
};

struct MQuestNPCInfo;

class ZActor : public ZCharacterObjectHistory
{
	MDeclareRTTI;

	friend ZBrain;
	friend ZActorAnimation;
private:
	unsigned long int		m_nFlags;
	void UpdateHeight(float fDelta);
	void UpdatePosition(float fDelta);
protected:
	float					m_fTC;
	int						m_nQL;
	MMatchItemDesc			m_ItemDesc;
	MQuestNPCInfo*			m_pNPCInfo;
	ZActorAnimation			m_Animation;
	ZBrain*					m_pBrain;
	ZTaskManager			m_TaskManager;
	float					m_TempBackupTime;
	float					m_fSpeed;
	int						m_nDamageCount;
private:
	void InitFromNPCType(MQUEST_NPC nNPCType, float fTC, int nQL);
	void InitMesh(char* szMeshName);
	void OnTaskFinished(ZTASK_ID nLastID);
	static void OnTaskFinishedCallback(ZActor* pActor, ZTASK_ID nLastID);
	inline static int CalcMaxHP(int nQL, int nSrcHP);
	inline static int CalcMaxAP(int nQL, int nSrcAP);
protected:
	enum ZACTOR_LASTTIME
	{
		ACTOR_LASTTIME_HPINFO		= 0,
		ACTOR_LASTTIME_BASICINFO,
		ACTOR_LASTTIME_MAX
	};
	unsigned long int	m_nLastTime[ACTOR_LASTTIME_MAX];

	ZModule_Skills			*m_pModule_Skills;

	rvector				m_vAddBlastVel;
	float				m_fAddBlastVelTime;
	rvector				m_TargetDir;
	rvector				m_Accel;
	float				m_fDelayTime;

	MQUEST_NPC			m_nNPCType;

protected:
	bool				m_bTestControl;
	void TestControl(float fDelta);

	virtual void InitProperty();
	virtual void InitStatus();

	virtual void OnDraw();
	virtual void OnUpdate(float fDelta);
	virtual bool ProcessMotion(float fDelta);
	virtual void ProcessNetwork(float fDelta);

	// task
	virtual void ProcessAI(float fDelta);
	void ProcessMovement(float fDelta);
	void CheckDead(float fDelta);

	void PostBasicInfo();

public:
	ZActor();
	virtual ~ZActor();
	static ZActor* CreateActor(MQUEST_NPC nNPCType, float fTC, int nQL);
	void InputBasicInfo(ZBasicInfo* pni, BYTE anistate);
	void Input(AI_INPUT_SET nInput);
	void DebugTest();
	void SetMyControl(bool bMyControl);

	inline ZA_ANIM_STATE GetCurrAni();
	inline void SetFlag(unsigned int nFlag, bool bValue);
	inline bool CheckFlag(unsigned int nFlag);
	inline void SetFlags(unsigned int nFlags);
	inline unsigned long GetFlags();
	inline bool IsMyControl();
	inline int GetHP();
	inline int GetAP();
	inline float GetTC();
	inline int GetQL();
	inline float GetHitRate();
	inline int GetActualMaxHP();
	inline int GetActualMaxAP();

	void RunTo(rvector& dir);
	void Stop(bool bWithAniStop=true);
	void RotateTo(const rvector& dir);

	virtual void OnBlast(rvector &dir);
	virtual void OnBlastDagger(rvector &dir,rvector& pos);
	virtual bool IsCollideable();
	virtual bool IsAttackable();
	virtual void Attack_Melee();
	virtual void Attack_Range(rvector& dir);
	virtual void Skill(int nSkill);

	bool isThinkAble();

	ZBrain* GetBrain()					{ return m_pBrain; }
	MQuestNPCInfo* GetNPCInfo()			{ return m_pNPCInfo; }
	ZTaskManager* GetTaskManager()		{ return &m_TaskManager; }
	
	virtual ZOBJECTHITTEST HitTest(const rvector& origin, const rvector& to,
		float fTime, rvector *pOutPos = NULL) override;

	virtual bool IsDie() override;

	virtual MMatchTeam GetTeamID() { return MMT_BLUE; }

	virtual void OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio=1.f, int nMeleeType=-1);

	virtual void OnKnockback(const rvector& dir, float fForce);
	virtual void OnDie();
	virtual void OnPeerDie(MUID& uidKiller);

	bool IsDieAnimationDone();
	bool CanSee(ZObject* pTarget);
	bool CanAttackRange(ZObject* pTarget);
	bool CanAttackMelee(ZObject* pTarget, ZSkillDesc *pSkillDesc=NULL);

	auto& GetLastAttacker() const { return m_pModule_HPAP->GetLastAttacker(); }

	char m_szOwner[64];
	float m_fLastBasicInfo;
	void SetOwner(const char* szOwner) { strcpy_safe(m_szOwner,szOwner); }
};



// inline /////////////////////////////////////////////////////////////////////////////////////////
inline void ZActor::SetFlags(unsigned int nFlags)
{
	if (m_nFlags != nFlags)
	{
		m_nFlags = nFlags;
	}
}

inline void ZActor::SetFlag(unsigned int nFlag, bool bValue)
{
	if (bValue) m_nFlags |= nFlag;
	else m_nFlags &= ~nFlag;
}

inline bool ZActor::CheckFlag(unsigned int nFlag)
{
	return ((m_nFlags & nFlag) != 0);
}

inline unsigned long ZActor::GetFlags() 
{ 
	return m_nFlags; 
}

inline ZA_ANIM_STATE ZActor::GetCurrAni()
{ 
	return m_Animation.GetCurrState();
}

inline int ZActor::GetHP()	
{ 
	return m_pModule_HPAP->GetHP(); 
}

inline int ZActor::GetAP()	
{
	return m_pModule_HPAP->GetAP(); 
}

inline float ZActor::GetTC()
{ 
	return m_fTC; 
}

inline int ZActor::GetQL()
{
	return m_nQL;
}

inline float ZActor::GetHitRate()
{
	return (m_fTC * m_pNPCInfo->fAttackHitRate);
}

inline bool ZActor::IsMyControl()
{
	return CheckFlag(AF_MY_CONTROL);
}

inline int ZActor::GetActualMaxHP()
{
	return ((m_pNPCInfo) ? ((int)(float)m_pNPCInfo->nMaxHP * m_fTC) : 0);
}

inline int ZActor::GetActualMaxAP()
{
	return ((m_pNPCInfo) ? ((int)(float)m_pNPCInfo->nMaxAP * m_fTC) : 0);
}


inline int ZActor::CalcMaxHP(int nQL, int nSrcHP)
{
	return (int)(((float)nQL * 0.2f + 1) * nSrcHP);
}

inline int ZActor::CalcMaxAP(int nQL, int nSrcAP)
{
	return (int)(((float)nQL * 0.2f + 1) * nSrcAP);
}