#ifndef MSimpleVehicle3_H
#define MSimpleVehicle3_H

#include "rutils.h"

struct MSIMPLEVEHICLE3PROPERTY{
	float	fMaxVelocity;			// Max Velocity
	float	fAcceleration;			// Acceleration
	float	fMaxRotationVelocity;	// Max Rotation Velocity ( Radian )
	float	fRotationAcceleration;	// Rotation Acceleration ( Radian )
};

// Abstract Collision Map
class MSimpleVehicle3CollisionMap{
public:
	virtual bool IsCollision(rvector* pRecommendedPos, float* t, rvector& To, rvector& From) = 0;
};

class MFreeMapModel;
class MSV3FMCollisionMap : public MSimpleVehicle3CollisionMap{
	MFreeMapModel*	m_pFM;
public:
	MSV3FMCollisionMap(MFreeMapModel* pFM){
		m_pFM = pFM;
	}
	virtual bool IsCollision(rvector* pRecommendedPos, float* t, rvector& To, rvector& From);
};

class MSimpleVehicle3{
public:
	// Property
	MSIMPLEVEHICLE3PROPERTY	m_Property;

	// Values
	rvector		m_Pos;
	rvector		m_Dir;
	float		m_fVelocity;
	float		m_fAngularVelocity;

	// Attribute
	int			m_nDecelerationConstant;		// 감속할 거리 상수
	int			m_nAgularDecelerationConstant;	// Angular Speed에서 감속할 각도 상도
	bool		m_bDirectionSpeed;				// 방향에 따른 가감속 지원 ( Default = false )
	float		m_fArrivalConstant;				// 도착 상수 ( 0 ~ 1.0f ), MaxVelocity*m_fArrivalConstant에 들어오면 도착
	float		m_nDecelerationCurveConstant;	// 감속 커브 상수 t^m_nDecelerationCurveConstant
	float		m_nDirectionalSpeedContant;		// 목표지점과의 각도 차이에 의해 속도 조절이 되는 정도 (0:없음~1.0f:정지)

	// 결과값들
	float		m_fResultRotatePercent;			// 실제로 회전한 값 ( -1.0 ~ 1.0 )

	// Debug
	rvector		m_DebugRecommendedPos;
	rvector		m_DebugTowardPos;

protected:
	bool ProcMove(rvector* pNewPos, rvector* pNewDir, float* pNewVelocity, rvector& Dir, float fDistance, bool bSmoothArrival);

public:
	MSimpleVehicle3(bool bDirectionSpeed=false);
	virtual ~MSimpleVehicle3(void);

	bool Move(rvector& Dir, float fDistance, MSimpleVehicle3CollisionMap* pCM, bool bSmoothArrival);

	void SetMaxVelocity(float v);
	void SetAcceleration(float v);
	void SetMaxRotationVelocity(float v);
	void SetRotationAcceleration(float v);

	// Helper Functions.......
	float AdjustMaxVeloctiy(float fDistance, float fDecConstant=2.0f);		// 거리에 따른 Max Velocity 조정
};


#define DEFAULT_DECELERATION_CONSTANT		20
#define DEFAULT_ANGULARDECELERATION_CONTANT	5
#define DEFAULT_ARRIVAL_CONTANT				0.2f
#define DEFAULT_DECELERATION_CURVE_CONSTANT	1.1f
#define DEFAULT_DIRECTIONAL_SPEED_CONSTANT	1.0f

#endif
