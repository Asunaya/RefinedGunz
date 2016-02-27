#ifndef MSimpleVehicle4_H
#define MSimpleVehicle4_H

#include "rutils.h"

struct MSimpleVehicle4PROPERTY{
	float	fMaxVelocity;			// Max Velocity
	float	fAcceleration;			// Acceleration
	float	fMaxRotationVelocity;	// Max Rotation Velocity ( Radian )
	float	fRotationAcceleration;	// Rotation Acceleration ( Radian )
};

// Abstract Collision Map
class MSimpleVehicle4CollisionMap{
public:
	virtual bool IsCollision(rvector* pRecommendedPos, float* t, rvector& To, rvector& From) = 0;
	virtual float GetMapHeight(float x, float y){ return 0; }
};

class MFreeMapModel;
class MSV4FMCollisionMap : public MSimpleVehicle4CollisionMap{
	MFreeMapModel*	m_pFM;
public:
	MSV4FMCollisionMap(MFreeMapModel* pFM){
		m_pFM = pFM;
	}
	virtual bool IsCollision(rvector* pRecommendedPos, float* t, rvector& To, rvector& From);
};

class MSimpleVehicle4{
public:
	// Property
	MSimpleVehicle4PROPERTY	m_Property;

	// Values
	rvector		m_Pos;
	rvector		m_Dir;
	rvector		m_Velocity;
	float		m_fAngularVelocity;

	// Attribute
	int			m_nDecelerationConstant;		// 감속할 거리 상수
	float		m_fDirectionSettingConstant;	// 목표 방향을 맞추기 시작하는 거리
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

	enum MType{
		MSVT_GROUND,
		MSVT_FLOATING,
		MSVT_ONAIR,
		MSVT_LANDING,
	};

	MType		m_nType;					// 유닛 타입
	float		m_fMaxHeight;				// 타입에 따른 최대 높이

	bool		m_bUseRecommendedHeight;	// 여기서 제공하는 고도를 사용할 경우(TargetPos.z는 무시된다.)

protected:
	rvector RotateDir(rvector& TargetDir);
	rvector GetRecommendedDistance(rvector& Pos, rvector& Distance, float fTargetDistance, bool bSmoothArrival, MSimpleVehicle4CollisionMap* pCM);
	bool ProcMove(rvector* pNewPos, rvector* pNewDir, rvector* pNewVelocity, rvector* pDir, rvector& Distance, rvector& RealDistance, bool bSmoothArrival, MSimpleVehicle4CollisionMap* pCM);

public:
	MSimpleVehicle4(bool bDirectionSpeed=false);
	virtual ~MSimpleVehicle4(void);

	bool Move(rvector* pDir, rvector& TargetPos, MSimpleVehicle4CollisionMap* pCM, bool bSmoothArrival);
	bool Move(MSimpleVehicle4CollisionMap* pCM);

	void SetMaxVelocity(float v);
	void SetAcceleration(float v);
	void SetMaxRotationVelocity(float v);
	void SetRotationAcceleration(float v);

	// Helper Functions.......
	float AdjustMaxVeloctiy(float fDistance, float fDecConstant=2.0f);		// 거리에 따른 Max Velocity 조정
};


#define DEFAULT_DECELERATION_CONSTANT		50
#define DEFAULT_ANGULARDECELERATION_CONTANT	5
#define DEFAULT_ARRIVAL_CONTANT				0.2f
#define DEFAULT_DECELERATION_CURVE_CONSTANT	1.1f
#define DEFAULT_DIRECTIONAL_SPEED_CONSTANT	1.0f
#define DEFAULT_DIRECTION_SETTING_CONSTANT	50

#endif
