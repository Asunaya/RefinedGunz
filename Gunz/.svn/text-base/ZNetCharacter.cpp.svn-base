#include "stdafx.h"

#include "ZNetCharacter.h"
#include "ZGameInterface.h"
#include "ZGame.h"
#include "ZApplication.h"

MImplementRTTI(ZNetCharacter, ZCharacter);

ZNetCharacter::ZNetCharacter() : ZCharacter()
{

}

ZNetCharacter::~ZNetCharacter()
{

}

void  ZNetCharacter::OnUpdate(float fDelta)
{
	if (m_bInitialized==false) return;
	if (!IsVisible()) return;

	float fTimeLapse = g_pGame->GetTime() - m_fLastValidTime;
	m_fLastValidTime = g_pGame->GetTime();

	ZCharacter::OnUpdate(fDelta);

	// 실제 움직이는 부분은 module_movable 로 옮겨갔다

	if(IsMoveAnimation())		// 애니메이션에 움직임이 포함된넘은 움직여줘야한다.
	{
		rvector origdiff=fDelta*GetVelocity();

		rvector diff = m_AnimationPositionDiff;
		diff.z+=origdiff.z;
		//		rvector diff=m_RealPositionBefore-m_RealPosition;	// mesh 의 위치 변화로 움직임판정
		if(GetDistToFloor()<0 && diff.z<0) diff.z=-GetDistToFloor();

		Move(diff);

		// debug
		/*
		mlog("%d pos : %3.3f %3.3f %3.3f      ani-move : %3.3f %3.3f %3.3f\n",
			m_AniState_Lower,
			m_Position.x,m_Position.y,m_Position.z,diff.x,diff.y,diff.z);
		*/
	}

	UpdateHeight(fDelta);
}

void ZNetCharacter::OnDraw()
{
	ZCharacter::OnDraw();
}

void ZNetCharacter::SetNetPosition(rvector& pos)
{
	// TODO : 이것때문에 발이 묻히는경우가 종종 있다
	if (Magnitude(pos - m_Position) > 20.0f)
	{
		m_Position = pos;
	}
}

void ZNetCharacter::SetNetPosition(rvector& position, rvector& velocity, rvector& dir)
{

	SetNetPosition(position);
	SetVelocity(velocity);
	SetAccel(rvector(0.0f, 0.0f, 0.0f));

/*
	float fT = 0.1f;
	//rvector pretar = position + velocity * fT + (0.5f * accel * fT * fT);
	rvector pretar = position + velocity * fT;
	rvector v = (pretar - m_Position) / fT;

	SetNetPosition(position);
	SetVelocity(v);
	SetAccel(rvector(0.0f,0.0f,0.0f));
*/



	m_TargetDir = dir;
	m_fLastValidTime = ZApplication::GetGame()->GetTime();
}