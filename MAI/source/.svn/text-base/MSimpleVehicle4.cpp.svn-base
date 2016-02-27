#include "MSimpleVehicle4.h"
#include "MFreeMapModel.h"
#include "MUtil.h"
#include <float.h>


bool MSV4FMCollisionMap::IsCollision(rvector* pRecommendedPos, float* t, rvector& To, rvector& From)
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

	return true;
}

void Clip(rvector* p, float fMaxMagnitude)
{
	float fMagnitude = p->GetMagnitude();
	if(fMagnitude>fMaxMagnitude) (*p) *= (fMaxMagnitude/fMagnitude);
	//if(fMagnitude>fMaxMagnitude) (*p) = (Normalize(*p)*fMaxMagnitude);
}

rvector Clip(rvector& p, float fMaxMagnitude)
{
	float fMagnitude = p.GetMagnitude();
	if(fMagnitude>fMaxMagnitude) return (p*(fMaxMagnitude/fMagnitude));
	//if(fMagnitude>fMaxMagnitude) return (Normalize(p)*fMaxMagnitude);
	else return p;
}

#define CHECKVECTOR_NAN(_v)	{_ASSERT(_isnan(_v.x)==0);_ASSERT(_isnan(_v.y)==0);_ASSERT(_isnan(_v.z)==0);}

rvector MSimpleVehicle4::RotateDir(rvector& TargetDir)
{
	float fDiffAngle2D = GetAngleOfVectors(TargetDir, m_Dir);
	float fRotateAngle = fDiffAngle2D;

	float fRotatePercent = 1.0f;

	// 회전 가/감속
	if(fDiffAngle2D>0){
		m_fAngularVelocity = min(m_Property.fMaxRotationVelocity, m_fAngularVelocity + m_Property.fRotationAcceleration);
		if(fabs(fDiffAngle2D)<m_Property.fMaxRotationVelocity*m_nAgularDecelerationConstant){
			m_fAngularVelocity = min(m_fAngularVelocity, fDiffAngle2D/m_nAgularDecelerationConstant);
		}
	}
	else if(fDiffAngle2D<0){
		m_fAngularVelocity = max(-m_Property.fMaxRotationVelocity, m_fAngularVelocity - m_Property.fRotationAcceleration);
		if(fabs(fDiffAngle2D)<m_Property.fMaxRotationVelocity*m_nAgularDecelerationConstant){
			m_fAngularVelocity = max(m_fAngularVelocity, fDiffAngle2D/m_nAgularDecelerationConstant);
		}
	}

	if(fDiffAngle2D!=0.0f) fRotatePercent = m_fAngularVelocity/fDiffAngle2D;

	rvector NewDir = Normalize(InterpolatedVector(m_Dir, Normalize(TargetDir), fRotatePercent));

	// 결과 값
	//m_fResultRotatePercent = fRotateAngle / m_Property.fMaxRotationVelocity;
	m_fResultRotatePercent = GetAngleOfVectors(NewDir, m_Dir) / m_Property.fMaxRotationVelocity;	// 버그로 인해 실제 계산값을 입력

	return NewDir;
}

rvector MSimpleVehicle4::GetRecommendedDistance(rvector& Pos, rvector& Distance, float fTargetDistance, bool bSmoothArrival, MSimpleVehicle4CollisionMap* pCM)
{
	rvector NextPos = Pos+m_Velocity;
	rvector RecommendedDistance = Distance;

	float fMapHeight = 0;
	if(pCM!=NULL) fMapHeight = pCM->GetMapHeight(NextPos.x, NextPos.y);

	if(m_nType==MSVT_GROUND || m_nType==MSVT_FLOATING){
		float fHeight = 0;
		if(m_nType==MSVT_GROUND) fHeight = fMapHeight;
		else fHeight = fMapHeight+m_fMaxHeight;

		rvector Diff = Pos+Distance-NextPos;
		Diff.z = 0;
		float fDiff2D = Diff.GetMagnitude();

		//rvector CheckDistance2D(NextPos.x-Pos.x, NextPos.y-Pos.y, 0);
		float fCheckDistance2D = rvector(NextPos.x-Pos.x, NextPos.y-Pos.y, 0).GetMagnitude();
		fCheckDistance2D = max(m_Property.fMaxVelocity/2.0f, fCheckDistance2D);	// fCheckDistance2D가 지나치게 작아서 RecommendedDistance.z가 상대적으로 커지는것을 막는다.
		if(fCheckDistance2D>0){
			// EstimateDistance에 맞는 높이 EstimateDistance.z를 설정한다.
			rvector EstimateDistance2D(Distance.x, Distance.y, 0);
			RecommendedDistance.z = EstimateDistance2D.GetMagnitude()*(fHeight - NextPos.z)/fCheckDistance2D;
		}
		else{
			// 더 나아갈 방향이 없으면, 현재 높이와 같은 높이로 나아감을 의미한다.
			RecommendedDistance.z = 0;
		}
	}
	else if(m_nType==MSVT_ONAIR){
		RecommendedDistance.z = m_fMaxHeight - Pos.z;
	}
	else if(m_nType==MSVT_LANDING){
		float fLandingDistance = m_Property.fMaxVelocity*m_nDecelerationConstant;
		float fReommendedHeight = m_fMaxHeight;
		float fLandingMinHeight = fMapHeight + ((m_fMaxHeight-fMapHeight) * (fTargetDistance/fLandingDistance));

		if(fTargetDistance<fLandingDistance && Pos.z>fLandingMinHeight-float(fabs(fLandingMinHeight)*0.2f) && bSmoothArrival==true){
			fReommendedHeight = fMapHeight;
		}
		else{
			float fMinLandingHeight = m_fMaxHeight*0.2f;
			if(Pos.z-fMapHeight<fMinLandingHeight){
				float fLandingAngle = (pi/4) * min(max((m_fMaxHeight-Pos.z)/m_fMaxHeight, 0), 1);
				fReommendedHeight = (float)tan(fLandingAngle)*fTargetDistance;
			}
			else
				fReommendedHeight = m_fMaxHeight;
			//fReommendedHeight = m_fMaxHeight;
		}

		RecommendedDistance.z = fReommendedHeight - Pos.z;
	}

	return RecommendedDistance;
}

bool MSimpleVehicle4::ProcMove(rvector* pNewPos, rvector* pNewDir, rvector* pNewVelocity, rvector* pDir, rvector& Distance, rvector& RealDistance, bool bSmoothArrival, MSimpleVehicle4CollisionMap* pCM)
{
	//if(Velocity.x==0.0f && Velocity.y==0.0f && Velocity.z==0.0f) return true;	// 같은 위치면 도착

	float fDistance = Distance.GetMagnitude();
	rvector Distance2D(Distance.x, Distance.y, 0);
	float fDistance2D = Distance2D.GetMagnitude();

	rvector TargetDir2D(m_Dir);
	rvector TargetDir(m_Dir);
	if(Distance.x!=0.0f || Distance.y!=0.0f){
		TargetDir2D = Distance;
		TargetDir2D.z = 0;
		TargetDir2D.Normalize();
	}
	if(fDistance!=0){
		TargetDir = Distance;
		TargetDir.Normalize();
	}
	if(pDir!=NULL){
		_ASSERT(pDir->z==0.0f);
		// 방향 설정은 보정된 Distance가 아닌 RealDistance에 의존한다.
		float fRealDistance = RealDistance.GetMagnitude();
		if(fRealDistance<=m_Property.fMaxVelocity*m_fDirectionSettingConstant){
			float fDirectionWeightConstant = max(min((1-fRealDistance/(m_Property.fMaxVelocity*m_fDirectionSettingConstant)), 1), 0);
			TargetDir2D = Normalize(InterpolatedVector(TargetDir2D, Normalize(*pDir), fDirectionWeightConstant));
		}
	}

	// 회전
	////////////////////////////////////////////////////////////
	rvector NewDir = RotateDir(TargetDir2D);
	/*
	rvector NewDir;
	if(m_Velocity.IsZero()==true)
		NewDir = RotateDir(TargetDir2D);
	else
		NewDir = RotateDir(Normalize(m_Velocity));
	*/

	// 가속 & 이동
	////////////////////////////////////////////////////////////

	//만약 bSmoothArrival==false인 경우 목표 방향으로 최고 속도를 내서 가야 함
	rvector EstimateDistance = Distance;
	if(bSmoothArrival==false && fDistance<m_Property.fMaxVelocity && EstimateDistance.IsZero()==false) EstimateDistance = Normalize(Distance)*m_Property.fMaxVelocity*10;

	// 유닛 타입에 따른 높이를 재설정
	if(m_bUseRecommendedHeight==true || m_nType==MSVT_GROUND || m_nType==MSVT_FLOATING || m_nType==MSVT_ONAIR){	// MSVT_GROUND, MSVT_FLOATING, MSVT_ONAIR이면 항상 Recommended Height를 써야 한다.
		EstimateDistance = GetRecommendedDistance(m_Pos, EstimateDistance, fDistance2D, bSmoothArrival, pCM);
		fDistance = EstimateDistance.GetMagnitude();	// 재설정된 위치에 따른 거리 재 측정
	}
	else{
		// 특정 위치로 갈때 최대 고도를 벗어나선 안된다.
		rvector Target = m_Pos+EstimateDistance;
		if(Target.z>m_fMaxHeight){
			EstimateDistance.z = m_fMaxHeight - m_Pos.z;
			fDistance = EstimateDistance.GetMagnitude();	// 재설정된 위치에 따른 거리 재 측정
		}
	}

	// 현재 가능 가속도 계산
	rvector Acceleration = Clip(EstimateDistance-m_Velocity, m_Property.fAcceleration);
	float fAcceleration = Acceleration.GetMagnitude();

	// 현재 가능 속도 계산
	rvector NewVelocity = m_Velocity + Acceleration;
	//float fNewVelocity = NewVelocity.GetMagnitude();
	Clip(&NewVelocity, m_Property.fMaxVelocity);	// Clip
	float fNewVelocity = NewVelocity.GetMagnitude();

	CHECKVECTOR_NAN(NewVelocity);

	if(fNewVelocity>=0.0f){
		if(m_bDirectionSpeed==true){	// 방향에 따른 가감속
			float fDiffAngle2D = GetAngleOfVectors(TargetDir2D, m_Dir);
			fNewVelocity -= (fNewVelocity*max(min((float)fabs(fDiffAngle2D)/pi, 1), 0)*m_nDirectionalSpeedContant);
			if(fNewVelocity==0) NewVelocity = rvector(0,0,0);	// -0 이 되는 경우(?)가 있어서 따로 뺌
			else NewVelocity = Normalize(NewVelocity)*fNewVelocity;
		}
	}

	CHECKVECTOR_NAN(NewVelocity);

	bool bArrive = false;
	// 거리에 따른 속도 조절
	if(bSmoothArrival==true && fDistance<=m_Property.fMaxVelocity*m_nDecelerationConstant){
		float t = fDistance/(m_Property.fMaxVelocity*m_nDecelerationConstant);
		//_ASSERT(m_nDecelerationCurveConstant>=1.0f);
		fNewVelocity = min(fNewVelocity, m_Property.fMaxVelocity*(float)pow((t), m_nDecelerationCurveConstant));
		if(fNewVelocity==0) NewVelocity = rvector(0,0,0);	// -0 이 되는 경우(?)가 있어서 따로 뺌
		else NewVelocity = Normalize(NewVelocity)*fNewVelocity;
		if(fDistance-fNewVelocity<=m_Property.fMaxVelocity*m_fArrivalConstant) bArrive = true;
	}
	else if(fNewVelocity>=fDistance2D)
		bArrive = true;


	CHECKVECTOR_NAN(NewVelocity);
	CHECKVECTOR_NAN(NewDir);

	// New Values
	*pNewVelocity = NewVelocity;
	*pNewDir = NewDir;
	*pNewPos = m_Pos+NewVelocity;

	return bArrive;
}

MSimpleVehicle4::MSimpleVehicle4(bool bDirectionSpeed)
: m_Pos(0, 0, 0), m_Dir(1, 0, 0), m_Velocity(0, 0, 0), m_fAngularVelocity(0),
m_bDirectionSpeed(bDirectionSpeed),
m_nDecelerationConstant(DEFAULT_DECELERATION_CONSTANT),
m_nAgularDecelerationConstant(DEFAULT_ANGULARDECELERATION_CONTANT),
m_fArrivalConstant(DEFAULT_ARRIVAL_CONTANT),
m_nDecelerationCurveConstant(DEFAULT_DECELERATION_CURVE_CONSTANT),
m_nDirectionalSpeedContant(DEFAULT_DIRECTIONAL_SPEED_CONSTANT),
m_fDirectionSettingConstant(DEFAULT_DIRECTION_SETTING_CONSTANT),
m_fResultRotatePercent(0),
m_nType(MSVT_GROUND),
m_fMaxHeight(0),
m_bUseRecommendedHeight(true)
{
}

MSimpleVehicle4::~MSimpleVehicle4(void)
{
}

bool MSimpleVehicle4::Move(rvector* pDir, rvector& TargetPos, MSimpleVehicle4CollisionMap* pCM, bool bSmoothArrival)
{
	float t = 0;
	rvector RecommendedPos = TargetPos;
	rvector Distance = TargetPos - m_Pos;
	float fDistance = Distance.GetMagnitude();
	rvector RealDistance = Distance;

	rvector TowardPos = m_Pos;
	if(Distance.IsZero()==false) TowardPos = m_Pos+min(m_Property.fMaxVelocity*m_nDecelerationConstant, fDistance)*Normalize(Distance);

	m_DebugTowardPos = TowardPos;

	m_DebugRecommendedPos = m_Pos;

	if(fDistance>0 && pCM!=NULL && pCM->IsCollision(&RecommendedPos, &t, TowardPos, m_Pos)==true){
		m_DebugRecommendedPos = RecommendedPos;
		Distance = RecommendedPos - m_Pos;
		float fNewDistance = Distance.GetMagnitude();
	}

	rvector NewPos, NewDir;
	rvector NewVelocity;

	bool bArrive = ProcMove(&NewPos, &NewDir, &NewVelocity, pDir, Distance, RealDistance, bSmoothArrival, pCM);

	// RecommendedPos에 의해 보정된 목적지가 도착인지 아닌지를 판별(아니면 중간에 서버리므로...)
	rvector Diff = TargetPos - NewPos;
	Diff.z = 0;
	if(Diff.GetMagnitude()>m_Property.fMaxVelocity*3) bArrive = false;	// 타겟과 비슷한 지점이여야 도착이다.

	m_Velocity = NewVelocity;
	m_Dir = NewDir;
	m_Pos = NewPos;

	return bArrive;
}

bool MSimpleVehicle4::Move(MSimpleVehicle4CollisionMap* pCM)
{
	// 현재 속도로 정지할때까지 움직인다.
	rvector NewTarget = m_Pos+m_Velocity;
	return Move(NULL, NewTarget, NULL, true);
}

void MSimpleVehicle4::SetMaxVelocity(float v)
{
	m_Property.fMaxVelocity = v;
}

void MSimpleVehicle4::SetAcceleration(float v)
{
	m_Property.fAcceleration = v;
}

void MSimpleVehicle4::SetMaxRotationVelocity(float v)
{
	m_Property.fMaxRotationVelocity = v;
}

void MSimpleVehicle4::SetRotationAcceleration(float v)
{
	m_Property.fRotationAcceleration = v;
}

float MSimpleVehicle4::AdjustMaxVeloctiy(float fDistance, float fDecConstant)		// 거리에 따른 Max Velocity 조정
{
	m_Property.fMaxVelocity = min(m_Property.fMaxVelocity, m_Property.fMaxVelocity * fDistance / fDecConstant);
	return m_Property.fMaxVelocity;
}
