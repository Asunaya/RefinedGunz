// 물리학에 관련된 함수들
//
// 단위계
// 거리 : 미터
// 시간 : 초
//
#ifndef PHYSICS_H
#define PHYSICS_H

#include "RTypes.h"

/// 중력 가속도
#define GRAVITY_ACCELERATION 9.8f

/// 포물선 운동
rvector ParabolicMotion(rvector& InitialVelocity, float fSec);

#endif