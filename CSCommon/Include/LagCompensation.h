#pragma once

#include "RAnimation.h"
#include "RAnimationMgr.h"

using namespace RealSpace2;

enum ZC_STATE_LOWER;

class LagCompManager
{
public:
	bool Create();
	bool LoadAnimations(const char* filename, int Index);

	v3 GetHeadPosition(const v3 & Origin, MMatchSex Sex, ZC_STATE_LOWER v, int Frame);

private:
	RAnimationMgr AniMgrs[2]; // 0 = male, 1 = female
};