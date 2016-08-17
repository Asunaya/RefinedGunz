#ifndef _ZMODULE_MOVABLE_H
#define _ZMODULE_MOVABLE_H

#include "ZModule.h"
#include "ZModuleID.h"

class ZModule_Movable : public ZModule {
private:
	// 실린더의 반지름과 높이
//	float	m_fRadius;
//	float	m_fHeight;

	float	m_fMaxSpeed;	// 최고속도
	bool	m_bGravity;		// 중력의 영향을 받는가?

	float	m_fDistToFloor;	/// 바닥까지의 거리
	rplane	m_FloorPlane;	/// 바닥 평면의 방정식

	rvector m_lastMove;		// 마지막으로 움직인 거리

	bool	m_bFalling;		// 낙하중이다
	float	m_fFallHeight;	// 낙하가 시작된 시점

	bool	m_bLanding;		// 이번에 착지했나
	bool	m_bAdjusted;	// 마지막 움직임이 (벽때문에) 비벼졌나
	float	m_fLastAdjustedTime;	// 마지막으로 비빈 시간

	rvector m_Velocity;		// 곧 private 으로 간다

	bool	m_bRestrict;		// 이동제한
	float	m_fRestrictTime;	// 제한이 걸린 시간
	float	m_fRestrictDuration;// 제한의 지속시간
	float	m_fMoveSpeedRatio;	// 움직임의 느려짐 비율

protected:
	void OnAdd();

public:

	DECLARE_ID(ZMID_MOVABLE)
	ZModule_Movable();

	virtual bool Update(float fElapsed);
	virtual void InitStatus();

	const rvector &GetVelocity() { return m_Velocity; }
	void SetVelocity(const rvector &vel) { m_Velocity=vel; }
	void SetVelocity(float x,float y,float z) { m_Velocity=rvector(x,y,z); }

	const rvector &GetLastMove() { return m_lastMove; }

	bool Move(rvector &diff);

	void UpdateGravity(float fDelta);

	float GetDistToFloor() { return m_fDistToFloor; }

	float GetFallHeight() { return m_fFallHeight; }
	bool isLanding() { return m_bLanding; }
	bool isAdjusted() { return m_bAdjusted; }
	float GetAdjustedTime() { return m_fLastAdjustedTime; }

	float GetMoveSpeedRatio() { return m_fMoveSpeedRatio; }
	void SetMoveSpeedRatio(float fRatio, float fDuration);

//	void SetCollision(float fRadius, float fHeight) { m_fRadius = fRadius; m_fHeight = fHeight; }
	//void SetRadius(float fRadius) { m_fRadius = fRadius; }
//	float GetRadius()			{ return m_fRadius; }
//	float GetHeight()			{ return m_fHeight; }

protected:
	void UpdatePosition(float fDelta);

};

#endif