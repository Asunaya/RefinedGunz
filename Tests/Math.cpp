#include "TestAssert.h"
#include "RMath.h"
#include "MMath.h"

using namespace RealSpace2;

static bool TestIntersectLineSegmentPlane()
{
	rplane p{ 1, 0, 0, 500 };

	v3 l0{ 0, 0, 0 };
	v3 l1{ 600, 0, 0 };
	v3 hit;
	auto result = IntersectLineSegmentPlane(&hit, p, l0, l1);

	assert(result);
	assert(hit.x == 500 && hit.y == 0 && hit.z == 0);

	std::swap(l0, l1);

	result = IntersectLineSegmentPlane(&hit, p, l0, l1);

	assert(result);
	assert(hit.x == 500 && hit.y == 0 && hit.z == 0);

	l0 = { 0, 0, 0 };
	l1 = { 499, 0, 0 };

	result = IntersectLineSegmentPlane(&hit, p, l0, l1);
	assert(!result);

	return true;
}

static bool TestRotationMatrix()
{
	v3 axis{ 0, 0, 1 };
	v3 v{ 1, 0, 0 };
	v3 result;

	auto Check = [&](const v3& vec) {
		assert(IS_EQ(vec.x, result.x) && IS_EQ(vec.y, result.y) && IS_EQ(vec.z, result.z));
	};

	rmatrix rot = RotationMatrix(axis, TAU / 4);
	result = v * rot;
	Check({ 0, 1, 0 });

	auto inv = Inverse(rot);
	result = result * inv;
	Check(v);

	rot = RotationMatrix(axis, TAU / 8);
	result = v * rot;
	Check({ sqrt(2.0f) / 2, sqrt(2.0f) / 2, 0 });

	inv = Inverse(rot);
	result = result * inv;
	Check(v);

	return true;
}

bool TestMath()
{
	auto ret = true;

	ret &= TestIntersectLineSegmentPlane();
	ret &= TestRotationMatrix();

	return ret; 
}