#pragma once

#include "AnimationStuff.h"

static RAnimationMgr* AniMgrs[2];

RAnimationMgr * GetAnimationMgr(MMatchSex Sex)
{
	return AniMgrs[Sex];
}

void SetAnimationMgr(MMatchSex Sex, RAnimationMgr* AniMgr)
{
	AniMgrs[Sex] = AniMgr;
}