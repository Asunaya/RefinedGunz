#pragma once

#include "RAnimation.h"
#include "RAnimationMgr.h"
#include "stuff.h"

using namespace RealSpace2;

enum MMatchSex;

class LagCompManager
{
public:
	bool Create();
	bool LoadAnimations(const char* filename, int Index);

	v3 GetHeadPosition(const matrix& World, float y, MMatchSex Sex, ZC_STATE_LOWER LowerState, int Frame, RWeaponMotionType MotionType);

private:
	RAnimationMgr AniMgrs[2]; // 0 = male, 1 = female
};