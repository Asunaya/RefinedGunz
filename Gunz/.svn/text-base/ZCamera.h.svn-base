#ifndef _ZCAMERA_H
#define _ZCAMERA_H

//#pragma once

#include "RTypes.h"

_USING_NAMESPACE_REALSPACE2

#define CAMERA_DEFAULT_DISTANCE		290.f


#define CAMERA_DEFAULT_ANGLEX		(pi/2.f)
#define CAMERA_DEFAULT_ANGLEZ		(pi/4.f)

#define CAMERA_ANGLEX_MIN	0.01f
#define CAMERA_ANGLEX_MAX	(pi-0.01f)

#define CAMERA_DIST_MAX		1000.f
#define CAMERA_DIST_MIN		150.f


enum ZCAMERALOOKMODE
{
	ZCAMERA_DEFAULT		= 0,
	ZCAMERA_FREEANGLE	= 1,
	ZCAMERA_FREELOOK	= 2,
	ZCAMERA_MINIMAP		= 3,
	
	ZCAMERA_END
};

class ZCamera {
public:
	ZCamera();

	void Update(float fElapsed);

	float m_fAngleX;
	float m_fAngleZ;
	float m_fDist;

	bool  m_bAutoAiming;

	rvector m_Target;				// 타겟이 최종적으로 가야할 위치.
	rvector	m_vCameraTrackOffset;	// 카메라가 위치 잡는 캐릭터의 offset

	void RecoilAngle(float fRecoilAngleX, float fRecoilAngleZ);		// 총쏠때 반동
	void SetMaxRecoilAngleX(float fMax);

	rvector GetPosition()	{ return m_Position; }
	void SetPosition(rvector &pos)		{ m_Position = pos; }
	void SetDirection(rvector& dir);
//	rvector GetDirection()	{ return m_Direction;}


	// 카메라 흔들리는 이펙트
	bool m_bShocked;
	rvector m_CameraShockOffset;
	rvector m_CameraShockVelocity;
	float	m_fShockStartTime;
	float	m_fShockPower,m_fShockDuration;

	void Shock(float fPower, float fDuration,rvector& vDir);
	void StopShock();
	void Init();

	ZCAMERALOOKMODE	GetLookMode()			{ return m_nLookMode; }
	void SetLookMode(ZCAMERALOOKMODE mode);
	void SetNextLookMode();


private:
	ZCAMERALOOKMODE		m_nLookMode;

	float m_fRecoilAngleX;
	float m_fRecoilAngleZ;
	float m_fMaxRecoilAngleX;


	float m_fCurrentAngleX;
	float m_fCurrentAngleZ;


	rvector m_Position;		// 현재 카메라의 위치
//	rvector m_Direction;	// 현재 카메라의 방향

	rvector m_CurrentTarget;

	bool CheckCollisionWall(float &fRealDist, rvector& pos, rvector& dir);
	void CalcMaxPayneCameraZ(float &fRealDist, float& fAddedZ, float fAngleX);
	rvector GetCurrentDir();		// 카메라가 바라보는 방향을 얻어낸다

public:
	float m_fCurrentDist;
	bool m_bCollision[4];
	float m_fPH;
};

#endif