#pragma once

#include "RTypes.h"
using namespace RealSpace2;

class ZShadow
{
public:
	bool SetMatrices(RVisualMesh& VisualMesh, RBspObject& Map, float Size = DefaultSize);
	void Draw();

	static constexpr float DefaultSize = 100;

private:
	struct ShadowType {
		bool Visible{};
		float DistanceToFloorSquared{};
		rmatrix World = IdentityMatrix();
	} Shadows[2]; // 0 = left, 1 = right
};
