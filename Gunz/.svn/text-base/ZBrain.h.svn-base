#ifndef _ZBRAIN_H
#define _ZBRAIN_H

class ZActor;

#include "ZActorAnimation.h"
#include "ZTask.h"
#include "ZTaskManager.h"
#include "ZBehavior.h"
#include "ZTimer.h"

class ZBrain_GoblinKing;

///< Actor의 AI주체
class ZBrain
{
	friend ZActor;
	friend ZActorAnimation;
private:
	// 몸체에서의 이벤트 발생 인터페이스
	virtual void OnBody_AnimEnter(ZA_ANIM_STATE nAnimState);
	virtual void OnBody_AnimExit(ZA_ANIM_STATE nAnimState);
	virtual void OnBody_CollisionWall();
	virtual void OnBody_OnTaskFinished(ZTASK_ID nLastID);

	// task 관련

	// 업데이트 관련
	ZUpdateTimer		m_PathFindingTimer;
	ZUpdateTimer		m_AttackTimer;
	ZUpdateTimer		m_DefaultAttackTimer;
protected:
	ZActor*				m_pBody;			///< 몸체
	ZBehavior			m_Behavior;
	MUID				m_uidTarget;		///< 타게팅하고 있는 적 UID
	

	// 길찾기 관련
	list<rvector>		m_WayPointList;
	bool BuildPath(rvector& vTarPos);
	void DrawDebugPath();
	void PushPathTask();

	
	MQUEST_NPC_ATTACK CheckAttackable();
	
	bool CheckSkillUsable(int *pnSkill, MUID *pTarget, rvector *pTargetPosition);		///< 스킬이 사용가능한지 체크후 가능하면 true를 반환

	bool FindTarget();
	void ProcessAttack(float fDelta);
	void ProcessBuildPath(float fDelta);
	void DefaultAttack(MQUEST_NPC_ATTACK nNpcAttackType);
	void UseSkill(int nSkill, MUID& uidTarget, rvector& vTargetPos);

	float MakePathFindingUpdateTime(char nIntelligence);
	float MakeAttackUpdateTime(char nAgility);
	float MakeDefaultAttackCoolTime();

	virtual bool CheckEnableTargetting(ZCharacter* pCharacter);
public:
	ZBrain();
	virtual ~ZBrain();
	void Init(ZActor* pBody);
	void Think(float fDelta);
	ZActor* GetBody()		{ return m_pBody; }
	ZObject* GetTarget();
	void DebugTest();

	static float MakeSpeed(float fSpeed);
	static ZBrain* CreateBrain(MQUEST_NPC nNPCType);
};



class ZBrain_GoblinKing : public ZBrain
{
private:
protected:
public:
	ZBrain_GoblinKing();
	virtual ~ZBrain_GoblinKing();
};




#endif