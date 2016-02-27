#ifndef MVehicle_H
#define MVehicle_H

#include "MAStar.h"
#include "rutils.h"

struct MVehiclePROPERTY{
	float	fMaxVelocity;			// Max Velocity
	float	fAcceleration;			// Acceleration
	float	fMaxRotationVelocity;	// Max Rotation Velocity ( Radian )
	float	fRotationAcceleration;	// Rotation Acceleration ( Radian )
};

// Abstract Collision Map
class MVehicleCollisionMap{
public:
	virtual bool IsCollision(rvector* pRecommendedPos, float* t, rvector& To, rvector& From) = 0;
	virtual float GetMapHeight(float x, float y){ return 0; }
};

class MVehicle : public MNodeModel{
public:
	// Property
	MVehiclePROPERTY	m_Property;

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

protected:
	bool ProcMove(rvector* pNewPos, rvector* pNewDir, rvector* pNewVelocity, rvector* pDir, rvector& Distance, rvector& RealDistance, bool bSmoothArrival);

public:
	MVehicle(bool bDirectionSpeed=false);
	virtual ~MVehicle(void);

	bool Move(rvector* pDir, rvector& TargetPos, MVehicleCollisionMap* pCM, bool bSmoothArrival);
	bool Move(MVehicleCollisionMap* pCM);

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


#define MAX_TEST_POS	8	// 다음 예상 위치 개수

class MVehicleNode : public MNodeModel{
protected:
	rvector		m_Pos;		// Current Position
	rvector		m_Dir;		// Current Direction
	rvector		m_Velocity;

	// Static Properties
	float		m_fMaxVelocity;		// Max Velocity
	float		m_fAcceleration;	// Acceleration

	MVehicleNode*	m_Successors[MAX_TEST_POS];

protected:
	void GetNextTestPos(rvector* pNewPos, rvector* pNewVelocity, int i);
public:
	MVehicleNode(rvector& Pos, rvector& Dir, rvector& Velocity, float fMaxVelocity, float fAcceleration);
	//MVehicleNode(void);
	virtual ~MVehicleNode(void);

	rvector GetPos(void){ return m_Pos; }
	rvector GetDir(void){ return m_Dir; }
	rvector GetVelocity(void){ return m_Velocity; }

	// MNodeModel Virtual Functions
	virtual int GetSuccessorCount(void);
	virtual MNodeModel* GetSuccessor(int i);
	virtual float GetSuccessorCost(MNodeModel* pSuccessor);
	virtual float GetHeuristicCost(MNodeModel* pNode);

	//virtual float GetTotalCostPriority(void){ return m_fCostToGoal; }		// Goal까지의 Cost만을 고려한다.
};

#endif
