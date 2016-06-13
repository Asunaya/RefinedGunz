#pragma once

#include "RAnimation.h"
#include "RAnimationMgr.h"
#include "stuff.h"
#include "RBspObject.h"
#include <unordered_map>

using namespace RealSpace2;

enum MMatchSex;

class LagCompManager
{
public:
	bool Create();

	RBspObject* GetBspObject(const char* MapName);

private:
	bool LoadAnimations(const char* filename, int Index);

	RAnimationMgr AniMgrs[2]; // 0 = male, 1 = female
	std::unordered_map<std::string, RBspObject> Maps;
};