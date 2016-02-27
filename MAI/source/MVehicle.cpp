#include "MVehicle.h"
#include "MFreeMapModel.h"
#include "MUtil.h"


static void Clip(rvector* p, float fMaxMagnitude)
{
	float fMagnitude = p->GetMagnitude();
	if(fMagnitude>fMaxMagnitude) (*p) *= (fMaxMagnitude/fMagnitude);
	//if(fMagnitude>fMaxMagnitude) (*p) = (Normalize(*p)*fMaxMagnitude);
}

static rvector Clip(rvector& p, float fMaxMagnitude)
{
	float fMagnitude = p.GetMagnitude();
	if(fMagnitude>fMaxMagnitude) return (p*(fMaxMagnitude/fMagnitude));
	//if(fMagnitude>fMaxMagnitude) return (Normalize(p)*fMaxMagnitude);
	else return p;
}

bool MVehicle::ProcMove(rvector* pNewPos, rvector* pNewDir, rvector* pNewVelocity, rvector* pDir, rvector& Distance, rvector& RealDistance, bool bSmoothArrival)
{
	//if(Velocity.x==0.0f && Velocity.y==0.0f && Velocity.z==0.0f) return true;	// 같은 위치면 도착

	float fDistance = Distance.GetMagnitude();

	rvector TargetDir2D(m_Dir);
	if(Distance.x!=0.0f || Distance.y!=0.0f || Distance.z!=0.0f){
		TargetDir2D = Distance;
		TargetDir2D.z = 0;
		TargetDir2D.Normalize();
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
	float fDiffAngle = GetAngleOfVectors(TargetDir2D, m_Dir);
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

	rvector NewDir = Normalize(InterpolatedVector(m_Dir, Normalize(TargetDir2D), fRotatePercent));

	// 결과 값
	//m_fResultRotatePercent = fRotateAngle / m_Property.fMaxRotationVelocity;
	m_fResultRotatePercent = GetAngleOfVectors(NewDir, m_Dir) / m_Property.fMaxRotationVelocity;	// 버그로 인해 실제 계산값을 입력

	bool bArriveAngle = false;
	if(fRotatePercent==1.0f) bArriveAngle = true;

	// 가속 & 이동
	////////////////////////////////////////////////////////////

	// 현재 가능 가속도 계산
	rvector Acceleration = Clip(Distance-m_Velocity, m_Property.fAcceleration);
	float fAcceleration = Acceleration.GetMagnitude();
	if(fAcceleration>=0.0f){
		if(m_bDirectionSpeed==true){	// 방향에 따른 가감속
			fAcceleration -= (fAcceleration*(float)max(min(fabs(fDiffAngle)/pi, 1), 0)*m_nDirectionalSpeedContant);
			Acceleration = Normalize(Acceleration)*fAcceleration;
		}
	}

	// 현재 가능 속도 계산
	rvector NewVelocity = m_Velocity + Acceleration;
	//float fNewVelocity = NewVelocity.GetMagnitude();
	Clip(&NewVelocity, m_Property.fMaxVelocity);	// Clip

	bool bArrive = false;
	// 거리에 따른 속도 조절
	float fNewVelocity = NewVelocity.GetMagnitude();
	if(bSmoothArrival==true && fDistance<=m_Property.fMaxVelocity*m_nDecelerationConstant){
		float t = fDistance/(m_Property.fMaxVelocity*m_nDecelerationConstant);
		//_ASSERT(m_nDecelerationCurveConstant>=1.0f);
		fNewVelocity = min(fNewVelocity, m_Property.fMaxVelocity*(float)pow((t), m_nDecelerationCurveConstant));
		NewVelocity = Normalize(NewVelocity)*fNewVelocity;
		if(fNewVelocity<=m_Property.fMaxVelocity*m_fArrivalConstant) bArrive = true;
	}
	else if(fNewVelocity>=fDistance) bArrive = true;

	// New Values
	*pNewVelocity = NewVelocity;
	*pNewDir = NewDir;
	*pNewPos = m_Pos+NewVelocity;

	return bArrive;
}

MVehicle::MVehicle(bool bDirectionSpeed)
: m_Pos(0, 0, 0), m_Dir(1, 0, 0), m_Velocity(0, 0, 0), m_fAngularVelocity(0),
m_bDirectionSpeed(bDirectionSpeed),
m_nDecelerationConstant(DEFAULT_DECELERATION_CONSTANT),
m_nAgularDecelerationConstant(DEFAULT_ANGULARDECELERATION_CONTANT),
m_fArrivalConstant(DEFAULT_ARRIVAL_CONTANT),
m_nDecelerationCurveConstant(DEFAULT_DECELERATION_CURVE_CONSTANT),
m_nDirectionalSpeedContant(DEFAULT_DIRECTIONAL_SPEED_CONSTANT),
m_fDirectionSettingConstant(DEFAULT_DIRECTION_SETTING_CONSTANT)
{
}

MVehicle::~MVehicle(void)
{
}

bool MVehicle::Move(rvector* pDir, rvector& TargetPos, MVehicleCollisionMap* pCM, bool bSmoothArrival)
{
	float t = 0;
	rvector RecommendedPos = TargetPos;
	rvector Distance = TargetPos - m_Pos;
	float fDistance = Distance.GetMagnitude();
	rvector RealDistance = Distance;

	rvector TowardPos = m_Pos+min(m_Property.fMaxVelocity*m_nDecelerationConstant, fDistance)*Normalize(Distance);

	m_DebugTowardPos = TowardPos;

	m_DebugRecommendedPos = m_Pos;

	if(fDistance>0 && pCM!=NULL && pCM->IsCollision(&RecommendedPos, &t, TowardPos, m_Pos)==true){
		m_DebugRecommendedPos = RecommendedPos;
		Distance = RecommendedPos - m_Pos;
		fDistance = Distance.GetMagnitude();
	}

	rvector NewPos, NewDir;
	rvector NewVelocity;

	//bool bArrive = ProcMove(&NewPos, &NewDir, &NewVelocity, Dir, Distance, bSmoothArrival);
	bool bArrive = ProcMove(&NewPos, &NewDir, &NewVelocity, pDir, Distance, RealDistance, bSmoothArrival);

	if(fDistance>0 && pCM!=NULL && pCM->IsCollision(&RecommendedPos, &t, m_Pos+NewVelocity, m_Pos)==true){
		float fNewVelocity = NewVelocity.GetMagnitude();
		fNewVelocity = min(fNewVelocity, m_Property.fMaxVelocity*(float)pow((t*fNewVelocity*m_nDecelerationConstant/(m_Property.fMaxVelocity*m_nDecelerationConstant)), m_nDecelerationCurveConstant));
		NewVelocity = Normalize(NewVelocity)*fNewVelocity;
		NewPos = m_Pos + NewVelocity;
	}

	m_Velocity = NewVelocity;
	m_Dir = NewDir;
	m_Pos = NewPos;

	return bArrive;
}

void MVehicle::SetMaxVelocity(float v)
{
	m_Property.fMaxVelocity = v;
}

void MVehicle::SetAcceleration(float v)
{
	m_Property.fAcceleration = v;
}

void MVehicle::SetMaxRotationVelocity(float v)
{
	m_Property.fMaxRotationVelocity = v;
}

void MVehicle::SetRotationAcceleration(float v)
{
	m_Property.fRotationAcceleration = v;
}

float MVehicle::AdjustMaxVeloctiy(float fDistance, float fDecConstant)		// 거리에 따른 Max Velocity 조정
{
	m_Property.fMaxVelocity = min(m_Property.fMaxVelocity, m_Property.fMaxVelocity * fDistance / fDecConstant);
	return m_Property.fMaxVelocity;
}


void MVehicleNode::GetNextTestPos(rvector* pNewPos, rvector* pNewVelocity, int i)
{
	_ASSERT(i<MAX_TEST_POS);
	rvector AccDir(0, 0, 0);
	rvector Dir(m_Dir);
	if(m_Velocity.IsZero()==false){
		Dir = Normalize(m_Velocity);
	}
	if(i==0){		// Toward
		AccDir = Dir;
	}
	else if(i==1){	// Right
		AccDir= TransformVector(Dir, RotateZMatrix(-pi/2));
	}
	else if(i==2){	// Left
		AccDir= TransformVector(Dir, RotateZMatrix(pi/2));
	}
	else if(i==3){	// Up
		AccDir= rvector(0, 0, 1);
	}
	else if(i==4){	// Down
		AccDir= rvector(0, 0, -1);
	}
	else if(i==5){	// Back
		AccDir = -Dir;
	}
	else if(i==6){	// Back
		AccDir= TransformVector(Dir, RotateZMatrix(-pi/4));
	}
	else if(i==7){	// Back
		AccDir= TransformVector(Dir, RotateZMatrix(pi/4));
	}
	else if(i==8){	// Back
		AccDir= TransformVector(Dir, RotateZMatrix(-pi*3/4));
	}
	else if(i==9){	// Back
		AccDir= TransformVector(Dir, RotateZMatrix(pi*3/4));
	}

	// 거리에 따른 속도 조절 코드 필요
	// 방향 조절 코드 필요

	*pNewVelocity = Clip(m_Velocity+AccDir*m_fAcceleration, m_fMaxVelocity);
	*pNewPos = m_Pos+(*pNewVelocity);
}

MVehicleNode::MVehicleNode(rvector& Pos, rvector& Dir, rvector& Velocity, float fMaxVelocity, float fAcceleration)
: m_Pos(Pos), m_Dir(Dir), m_Velocity(Velocity), m_fMaxVelocity(fMaxVelocity), m_fAcceleration(fAcceleration)
{
	for(int i=0; i<MAX_TEST_POS; i++) m_Successors[i] = NULL;
}

/*
MVehicleNode::MVehicleNode(void)
: m_Pos(0, 0, 0), m_Dir(0, 1, 0), m_Velocity(0, 0, 0)
{
}
*/
MVehicleNode::~MVehicleNode(void)
{
	for(int i=0; i<MAX_TEST_POS; i++){
		if(m_Successors[i]!=NULL){
			delete m_Successors[i];
			m_Successors[i] = NULL;
		}
	}
}

int MVehicleNode::GetSuccessorCount(void)
{
	return MAX_TEST_POS;
}

MNodeModel* MVehicleNode::GetSuccessor(int i)
{
	if(m_Successors[i]==NULL){
		rvector NewPos, NewVelocity;
		GetNextTestPos(&NewPos, &NewVelocity, i);
		rvector NewDir;
		if(NewVelocity.IsZero()==true) NewDir = m_Dir;
		else NewDir = Normalize(NewVelocity);
		m_Successors[i] = new MVehicleNode(NewPos, NewDir, NewVelocity, m_fMaxVelocity, m_fAcceleration);
	}

	return m_Successors[i];
}

float MVehicleNode::GetSuccessorCost(MNodeModel* pSuccessor)
{
	return GetHeuristicCost(pSuccessor);
}

float MVehicleNode::GetHeuristicCost(MNodeModel* pNode)
{
	MVehicleNode* pVehicleNode = (MVehicleNode*)pNode;
	rvector Diff = GetPos() - pVehicleNode->GetPos();
	float fDiff = Diff.GetMagnitude();
	return fDiff;
}
