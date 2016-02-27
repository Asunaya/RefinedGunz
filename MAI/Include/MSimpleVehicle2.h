#ifndef MSimpleVehicle2_H
#define MSimpleVehicle2_H

#include "rutils.h"

struct MSIMPLEVEHICLE2PROPERTY{
	float	fMaxVelocity;			// Max Velocity
	float	fAcceleration;			// Acceleration
	float	fMaxRotationVelocity;	// Max Rotation Velocity ( Radian )
	float	fRotationAcceleration;	// Rotation Acceleration ( Radian )
};

// Abstract Collision Map
class MCollisionMap2{
public:
	virtual bool IsCollision(rvector* pRecommendedPos, rvector& Pos) = 0;
};

class MSimpleVehicle2{
public:
	MSIMPLEVEHICLE2PROPERTY	m_Property;
	rvector		m_Pos;
	rvector		m_Dir;
	float		m_fVelocity;
	float		m_fAngularVelocity;
	bool		m_bDirectionSpeed;		// 방향에 따른 가감속 지원 ( Default = false )

	// 결과값들
	float		m_fResultRotatePercent;	// 실제로 회전한 값 ( -1.0 ~ 1.0 )

protected:
	void ProcMove(rvector* pNewPos, rvector* pNewDir, float* pNewVelocity, rvector& Velocity);

public:
	MSimpleVehicle2(bool bDirectionSpeed=false);
	virtual ~MSimpleVehicle2(void);

	void Move(rvector& Dir, MCollisionMap2* pCM);

	void SetMaxVelocity(float v);
	void SetAcceleration(float v);
	void SetMaxRotationVelocity(float v);
	void SetRotationAcceleration(float v);

	// Helper Functions.......
	float AdjustMaxVeloctiy(float fDistance, float fDecConstant=2.0f);		// 거리에 따른 Max Velocity 조정
};

#endif
