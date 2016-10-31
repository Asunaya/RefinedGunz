#pragma once

#include "RNameSpace.h"

_NAMESPACE_REALSPACE2_BEGIN

struct RCONVEXPOLYGONINFO;
struct RLightList;
class RBspObject;

using RGENERATELIGHTMAPCALLBACK = bool(*)(float fProgress);

struct LightmapGenerator
{
	const char* filename{};
	int MaxLightmapSize{}, MinLightmapSize{};
	int Supersample{};
	float Tolerance{};
	v3 AmbientLight{ 0, 0, 0 };

	RGENERATELIGHTMAPCALLBACK pProgressFn = [](float) { return true; };

	RBspObject& bsp;

	LightmapGenerator(RBspObject& bsp) : bsp(bsp) {}

	bool Generate();
};

void GetNormal(RCONVEXPOLYGONINFO *poly, const rvector &position,
	rvector *normal, int au, int av);

_NAMESPACE_REALSPACE2_END
