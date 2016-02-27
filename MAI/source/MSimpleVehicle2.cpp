#include "MSimpleVehicle2.h"


void MSimpleVehicle2::ProcMove(rvector* pNewPos, rvector* pNewDir, float* pNewVelocity, rvector& Velocity)
{
	if(Velocity.x==0.0f && Velocity.y==0.0f && Velocity.z==0.0f) return;	// 같은 위치면 도착

	// 회전
	/////////////////////////////////////////////////////////////
	float fDiffAngle = GetAngleOfVectors(Velocity, m_Dir);
	float fRotateAngle = fDiffAngle;

	float fRotatePercent = 1.0f;

	m_fAngularVelocity = min(m_Property.fMaxRotationVelocity, m_fAngularVelocity + m_Property.fRotationAcceleration);	// Angular Acceleration
#define ANGULAR_DECCELRATION_CONTANT	2.0f
	if(fabs(fDiffAngle)<m_Property.fMaxRotationVelocity*ANGULAR_DECCELRATION_CONTANT){
		m_fAngularVelocity = min(m_fAngularVelocity, m_Property.fMaxRotationVelocity*ANGULAR_DECCELRATION_CONTANT/(ANGULAR_DECCELRATION_CONTANT+1));
	}

	if(fabs(fDiffAngle)>m_fAngularVelocity){	// 가려는 방향까지 할당된 RotationDegree만큼 회전각 구하기
		if(fDiffAngle>=0) fRotateAngle = m_fAngularVelocity;
		else fRotateAngle = -m_fAngularVelocity;
	}

	if(fDiffAngle!=0.0f) fRotatePercent = fRotateAngle/fDiffAngle;

	rvector NewDir = Normalize(InterpolatedVector(m_Dir, Velocity, fRotatePercent));
	//rvector NewDir = Normalize(MInterpolateVector(m_Dir, Velocity, fRotatePercent));
	// InterpolatedVector()가 정반대의 두각일때 버그가 있다.

	// 결과 값
	//m_fResultRotatePercent = fRotateAngle / m_Property.fMaxRotationVelocity;
	m_fResultRotatePercent = GetAngleOfVectors(NewDir, m_Dir) / m_Property.fMaxRotationVelocity;	// 버그로 인해 실제 계산값을 입력

	// 가속 & 이동
	////////////////////////////////////////////////////////////
	float fDesireVelocity = Velocity.GetSafeMagnitude();
	float fNewVelocity = fDesireVelocity;
	if(fDesireVelocity>m_Property.fMaxVelocity)
		fNewVelocity = min(m_Property.fMaxVelocity, m_fVelocity + m_Property.fAcceleration); // 현재 가능 속도 계산

#define DIRECTIONAL_SPEED_CONSTANT	1.0f
	if(fNewVelocity>=0.0f){
		if(m_bDirectionSpeed==true){	// 방향에 따른 가감속
			fNewVelocity -= (fNewVelocity*(1.0f-fRotatePercent)*DIRECTIONAL_SPEED_CONSTANT);
			//fNewVelocity -= (fNewVelocity*(m_fAngularVelocity/m_Property.fMaxRotationVelocity)*DIRECTIONAL_SPEED_CONSTANT);
		}
	}

	// New Values
	*pNewVelocity = fNewVelocity;
	*pNewDir = NewDir;
	*pNewPos = m_Pos+(fNewVelocity*NewDir);
}

MSimpleVehicle2::MSimpleVehicle2(bool bDirectionSpeed)
: m_Pos(0, 0, 0), m_Dir(1, 0, 0), m_fVelocity(0), m_fAngularVelocity(0), m_bDirectionSpeed(bDirectionSpeed)
{
}

MSimpleVehicle2::~MSimpleVehicle2(void)
{
}

void MSimpleVehicle2::Move(rvector& Dir, MCollisionMap2* pCM)
{
	// 이동
	rvector NewPos, NewDir;
	float fNewVelocity;
	ProcMove(&NewPos, &NewDir, &fNewVelocity, Dir);

	// 이동에 따른 충돌 체크
	rvector RecommendedPos;
#define ESTIMATE_CONSTANT	4
	if(pCM!=NULL && pCM->IsCollision(&RecommendedPos, m_Pos+NewDir*fNewVelocity*ESTIMATE_CONSTANT)==true){
		ProcMove(&NewPos, &NewDir, &fNewVelocity, RecommendedPos-m_Pos);
		fNewVelocity = fNewVelocity * ESTIMATE_CONSTANT / (ESTIMATE_CONSTANT+1);
	}
		/*
		float t = 1.0f;
	#define EXPECT_CONSTANT	10.0f
		if(pCM!=NULL && pCM->IsIntersect(&t, m_Pos, NewDir, fNewVelocity*EXPECT_CONSTANT)==true && (t>0.0f && t<=1.0f)){
			//fNewVelocity = fNewVelocity * t * fRotatePercent / EXPECT_CONSTANT;		// 정밀하게 체크하는 경우 ( 모서리 부분에서 멈출 수 있다. ㅠ.ㅠ )
			fNewVelocity = fNewVelocity * fRotatePercent * fRotatePercent;				// 겹침을 허용하지만 부드럽게 체크 ( 충돌 가능시에 방향에 비례해서 속도를 줄임 )
			//fNewVelocity = fNewVelocity * max(t, 0.2f) * fRotatePercent / EXPECT_CONSTANT;		// 정밀하게 체크하는 경우 ( 모서리 부분에서 멈출 수 있다. ㅠ.ㅠ )
		}
		*/

	m_fVelocity = fNewVelocity;
	m_Dir = NewDir;
	m_Pos = NewPos;
}

void MSimpleVehicle2::SetMaxVelocity(float v)
{
	m_Property.fMaxVelocity = v;
}

void MSimpleVehicle2::SetAcceleration(float v)
{
	m_Property.fAcceleration = v;
}

void MSimpleVehicle2::SetMaxRotationVelocity(float v)
{
	m_Property.fMaxRotationVelocity = v;
}

void MSimpleVehicle2::SetRotationAcceleration(float v)
{
	m_Property.fRotationAcceleration = v;
}

float MSimpleVehicle2::AdjustMaxVeloctiy(float fDistance, float fDecConstant)		// 거리에 따른 Max Velocity 조정
{
	m_Property.fMaxVelocity = min(m_Property.fMaxVelocity, m_Property.fMaxVelocity * fDistance / fDecConstant);
	return m_Property.fMaxVelocity;
}
