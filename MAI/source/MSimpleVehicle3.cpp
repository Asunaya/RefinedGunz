#include "MSimpleVehicle3.h"
#include "MFreeMapModel.h"
#include "MUtil.h"


bool MSV3FMCollisionMap::IsCollision(rvector* pRecommendedPos, float* t, rvector& To, rvector& From)
{
	MFMPoint* p1;
	MFMPoint* p2;
	float pt;

	MFMPolygon* pPolygon = m_pFM->IsInnerClosedPolygon(From.x, From.y);
	if(pPolygon!=NULL){
		rvector NewFrom = From - (To-From);
		rvector NewTo = From;
		if(pPolygon->IsIntersect(&p1, &p2, t, NewFrom.x, NewFrom.y, NewTo.x, NewTo.y)==false) return false;

		//*t = 1 - *t;
		MGetPointFromPointToLine(pRecommendedPos, &pt, To, rvector(p2->GetX(), p2->GetY(), 0), rvector(p1->GetX(), p1->GetY(), 0));
		return true;
	}

	if(m_pFM->IsIntersect(&p1, &p2, t, From.x, From.y, To.x, To.y)==false) return false;

	MGetPointFromPointToLine(pRecommendedPos, &pt, To, rvector(p1->GetX(), p1->GetY(), 0), rvector(p2->GetX(), p2->GetY(), 0));
	
	*pRecommendedPos = To+(*pRecommendedPos-To)*1.1f;

	return true;
}

bool MSimpleVehicle3::ProcMove(rvector* pNewPos, rvector* pNewDir, float* pNewVelocity, rvector& Dir, float fDistance, bool bSmoothArrival)
{
	//if(Velocity.x==0.0f && Velocity.y==0.0f && Velocity.z==0.0f) return true;	// 같은 위치면 도착

	// 회전
	////////////////////////////////////////////////////////////
	float fDiffAngle = GetAngleOfVectors(Normalize(Dir), m_Dir);
	float fRotateAngle = fDiffAngle;

	float fRotatePercent = 1.0f;

	// 회전 가/감속
	if(fDiffAngle>0){
		m_fAngularVelocity = min(m_Property.fMaxRotationVelocity, m_fAngularVelocity + m_Property.fRotationAcceleration);
		if(fabs(fDiffAngle)<m_Property.fMaxRotationVelocity*m_nAgularDecelerationConstant){
			m_fAngularVelocity = min(m_fAngularVelocity, fDiffAngle/m_nAgularDecelerationConstant);
		}
	}
	else if(fDiffAngle<0){
		m_fAngularVelocity = max(-m_Property.fMaxRotationVelocity, m_fAngularVelocity - m_Property.fRotationAcceleration);
		if(fabs(fDiffAngle)<m_Property.fMaxRotationVelocity*m_nAgularDecelerationConstant){
			m_fAngularVelocity = max(m_fAngularVelocity, fDiffAngle/m_nAgularDecelerationConstant);
		}
	}

	if(fDiffAngle!=0.0f) fRotatePercent = m_fAngularVelocity/fDiffAngle;

	rvector NewDir = Normalize(InterpolatedVector(m_Dir, Normalize(Dir), fRotatePercent));

	// 결과 값
	//m_fResultRotatePercent = fRotateAngle / m_Property.fMaxRotationVelocity;
	m_fResultRotatePercent = GetAngleOfVectors(NewDir, m_Dir) / m_Property.fMaxRotationVelocity;	// 버그로 인해 실제 계산값을 입력

	bool bArriveAngle = false;
	if(fRotatePercent==1.0f) bArriveAngle = true;

	// 회전만 요구할 경우
	if(fDistance==0.0f){
		*pNewVelocity = 0;
		*pNewDir = NewDir;
		*pNewPos = m_Pos;
		return bArriveAngle;
	}

	// 가속 & 이동
	////////////////////////////////////////////////////////////
	float fDesireVelocity = fDistance;
	float fNewVelocity = min(m_Property.fMaxVelocity, m_fVelocity + m_Property.fAcceleration); // 현재 가능 속도 계산

	bool bArrive = false;
	// 거리에 따른 속도 조절
	if(bSmoothArrival==true && fDesireVelocity<=m_Property.fMaxVelocity*m_nDecelerationConstant){
		float t = fDesireVelocity/(m_Property.fMaxVelocity*m_nDecelerationConstant);
		//_ASSERT(m_nDecelerationCurveConstant>=1.0f);
		fNewVelocity = min(fNewVelocity, m_Property.fMaxVelocity*(float)pow((t), m_nDecelerationCurveConstant));
		if(fNewVelocity<=m_Property.fMaxVelocity*m_fArrivalConstant) bArrive = true;
	}
	else if(fNewVelocity>=fDesireVelocity) bArrive = true;


	if(fNewVelocity>=0.0f){
		if(m_bDirectionSpeed==true){	// 방향에 따른 가감속
			fNewVelocity -= (fNewVelocity*max(min((float)fabs(fDiffAngle)/pi, 1), 0)*m_nDirectionalSpeedContant);
		}
	}

	// New Values
	*pNewVelocity = fNewVelocity;
	*pNewDir = NewDir;
	*pNewPos = m_Pos+(fNewVelocity*NewDir);

	return bArrive;
}

MSimpleVehicle3::MSimpleVehicle3(bool bDirectionSpeed)
: m_Pos(0, 0, 0), m_Dir(1, 0, 0), m_fVelocity(0), m_fAngularVelocity(0),
m_bDirectionSpeed(bDirectionSpeed),
m_nDecelerationConstant(DEFAULT_DECELERATION_CONSTANT),
m_nAgularDecelerationConstant(DEFAULT_ANGULARDECELERATION_CONTANT),
m_fArrivalConstant(DEFAULT_ARRIVAL_CONTANT),
m_nDecelerationCurveConstant(DEFAULT_DECELERATION_CURVE_CONSTANT),
m_nDirectionalSpeedContant(DEFAULT_DIRECTIONAL_SPEED_CONSTANT)
{
}

MSimpleVehicle3::~MSimpleVehicle3(void)
{
}

/*
bool MSimpleVehicle3::Move(rvector& Target, MSimpleVehicle3CollisionMap* pCM, bool bSmoothArrival)
{
	rvector Velocity = Target - m_Pos;
	float fDiff = Velocity.GetSafeMagnitude();

	//bool bArrive = ProcMove(&NewPos, &NewDir, &fNewVelocity, Velocity);

	float t = 0;
	rvector RecommendedPos = Target;

	rvector TowardPos = m_Pos+min(m_Property.fMaxVelocity*m_nDecelerationConstant, fDiff)*Normalize(Target-m_Pos);

	m_DebugTowardPos = TowardPos;

	m_DebugRecommendedPos = m_Pos;

	if(pCM!=NULL && pCM->IsCollision(&RecommendedPos, &t, TowardPos, m_Pos)==true){
		m_DebugRecommendedPos = RecommendedPos;
	}

	rvector NewPos, NewDir;
	float fNewVelocity;

	bool bArrive = ProcMove(&NewPos, &NewDir, &fNewVelocity, fDiff*Normalize(RecommendedPos-m_Pos), bSmoothArrival);

	if(pCM!=NULL && pCM->IsCollision(&RecommendedPos, &t, m_Pos+min(fNewVelocity*m_nDecelerationConstant, fDiff)*NewDir, m_Pos)==true){
		//fNewVelocity = min(fNewVelocity, fNewVelocity*(t-0.01f));
		//t = max(0.1f, t);
		fNewVelocity = min(fNewVelocity, m_Property.fMaxVelocity*pow((t*fNewVelocity*m_nDecelerationConstant/(m_Property.fMaxVelocity*m_nDecelerationConstant)), m_nDecelerationCurveConstant));
		//fNewVelocity = min(fNewVelocity, fNewVelocity*t*m_nDecelerationConstant/(m_nDecelerationConstant+1));
		NewPos = m_Pos + fNewVelocity*NewDir;
		//m_DebugRecommendedPos = m_Pos+fNewVelocity*Normalize(RecommendedPos-m_Pos);
	}

	m_fVelocity = fNewVelocity;
	m_Dir = NewDir;
	m_Pos = NewPos;

	return bArrive;
}
*/
bool MSimpleVehicle3::Move(rvector& Dir, float fDistance, MSimpleVehicle3CollisionMap* pCM, bool bSmoothArrival)
{
	float t = 0;
	rvector RecommendedPos = m_Pos + Dir*fDistance;

	rvector TowardPos = m_Pos+min(m_Property.fMaxVelocity*m_nDecelerationConstant, fDistance)*Normalize(Dir);

	m_DebugTowardPos = TowardPos;

	m_DebugRecommendedPos = m_Pos;

	if(fDistance>0 && pCM!=NULL && pCM->IsCollision(&RecommendedPos, &t, TowardPos, m_Pos)==true){
		m_DebugRecommendedPos = RecommendedPos;
		Dir = Normalize(RecommendedPos - m_Pos);
	}

	rvector NewPos, NewDir;
	float fNewVelocity;

	bool bArrive = ProcMove(&NewPos, &NewDir, &fNewVelocity, Dir, fDistance, bSmoothArrival);

	if(fDistance>0 && pCM!=NULL && pCM->IsCollision(&RecommendedPos, &t, m_Pos+min(fNewVelocity*m_nDecelerationConstant, fDistance)*NewDir, m_Pos)==true){
		fNewVelocity = min(fNewVelocity, m_Property.fMaxVelocity*(float)pow((t*fNewVelocity*m_nDecelerationConstant/(m_Property.fMaxVelocity*m_nDecelerationConstant)), m_nDecelerationCurveConstant));
		NewPos = m_Pos + fNewVelocity*NewDir;
	}

	m_fVelocity = fNewVelocity;
	m_Dir = NewDir;
	m_Pos = NewPos;

	return bArrive;
}


void MSimpleVehicle3::SetMaxVelocity(float v)
{
	m_Property.fMaxVelocity = v;
}

void MSimpleVehicle3::SetAcceleration(float v)
{
	m_Property.fAcceleration = v;
}

void MSimpleVehicle3::SetMaxRotationVelocity(float v)
{
	m_Property.fMaxRotationVelocity = v;
}

void MSimpleVehicle3::SetRotationAcceleration(float v)
{
	m_Property.fRotationAcceleration = v;
}

float MSimpleVehicle3::AdjustMaxVeloctiy(float fDistance, float fDecConstant)		// 거리에 따른 Max Velocity 조정
{
	m_Property.fMaxVelocity = min(m_Property.fMaxVelocity, m_Property.fMaxVelocity * fDistance / fDecConstant);
	return m_Property.fMaxVelocity;
}
