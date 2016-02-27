#ifndef _ZRANGEWEAPONHITDICE_H
#define _ZRANGEWEAPONHITDICE_H

#include "RTypes.h"

// 몇가지 팩터를 가지고 Actor가 쏠 방향을 정하는 빌더 클래스
class ZRangeWeaponHitDice
{
private:
	float		m_fHitProbability;		// 명중 확률

	float		m_fGlobalFactor;
	float		m_fDistance;
	float		m_fTargetHeight;
	float		m_fTargetWidth;
	float		m_fTargetSpeed;
	rvector		m_TargetPosition;
	rvector		m_SourcePosition;

	void MakeHitProbability();
public:
	ZRangeWeaponHitDice();
	void BuildTargetBounds(float fWidth, float fHeight);	// 대상의 크기
	void BuildTargetSpeed(float fSpeed);					// 대상의 속도
	void BuildTargetPosition(rvector& vPosition);			// 대상 위치
	void BuildSourcePosition(rvector& vPosition);			// 쏘는 사람 위치
	void BuildGlobalFactor(float fGlobalFactor);			// 최종적으로 미치는 팩터
	
	rvector ReturnShotDir();
	float GetHitProbability()		{ return m_fHitProbability; }
};







#endif