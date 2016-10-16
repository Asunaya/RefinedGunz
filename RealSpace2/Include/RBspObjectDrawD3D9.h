#pragma once

#include "RNameSpace.h"

_NAMESPACE_REALSPACE2_BEGIN

class RBspObject;

class RBspObjectDrawD3D9
{
public:
	RBspObjectDrawD3D9(RBspObject& bsp) : bsp(bsp) {}

	void Draw() {}

private:
	RBspObject& bsp;
};

_NAMESPACE_REALSPACE2_END