#include "TestAssert.h"
#include "RMath.h"
#include "MMath.h"
#include <d3dx9.h>

using namespace RealSpace2;

#define TEST_COMPARE_TO_D3DX

static bool TestIntersectLineSegmentPlane()
{
	rplane p{ 1, 0, 0, -500 };

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

	p = { -1.000000f, 0.000000f, 0.000000f, 4104.648438f };
	l0 = { 4013.892578f, -943.908447f, 612.525452f };
	l1 = { 10194.750000f, 6517.653809f, -98.953125f };
	result = IntersectLineSegmentPlane(&hit, p, l0, l1);
	assert(result);
	assert(hit == v3(4104.648438f, -834.347534f, 602.078552f));

	return true;
}

static bool TestIntersectTriangle()
{
	v3 v0{ 0, 0, 0 }, v1{ 1, 0, 0 }, v2{ 1, 1, 0 };
	v3 Origin{ 0.4f, 0.4f, -1 }, Dir{ 0, 0, 1 };
	float Dist{};

	auto Test = [&](bool ExpectedSuccess, float ExpectedDist = 0)
	{
		auto TestImpl = [&](auto&... tris)
		{
			auto success = IntersectTriangle(tris..., Origin, Dir, &Dist);
			assert(success == ExpectedSuccess);
			if (ExpectedSuccess)
				assert(IS_EQ(Dist, ExpectedDist));
		};
		TestImpl(v0, v1, v2);
		TestImpl(v1, v0, v2);
		TestImpl(v2, v1, v0);
		TestImpl(v0, v2, v1);
		TestImpl(v1, v2, v0);
		TestImpl(v2, v0, v1);
	};

	Test(true, 1);

	Origin = { 0.5, 0.5, -100 };
	Test(true, 100);

	Origin = { 100, 100, -1 };
	Test(false);

	v2 = { 1, 0, 1 };
	Origin = { 0.4f, -1, 0.4f };
	Dir = { 0, 1, 0 };
	Test(true, 1);

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

	auto angle = static_cast<float>(TAU / 4);
	rmatrix rot = RotationMatrix(axis, angle);
	result = v * rot;
	Check({ 0, 1, 0 });

#ifdef TEST_COMPARE_TO_D3DX
	D3DXMATRIX d3dx_mat;
	D3DXMatrixRotationAxis(&d3dx_mat, &axis, angle);
	auto d3dx_result = v * d3dx_mat;
	Check(d3dx_result);
#endif

	auto inv = Inverse(rot);
	result = result * inv;
	Check(v);

	rot = RotationMatrix(axis, static_cast<float>(TAU / 8));
	result = v * rot;
	Check({ sqrt(2.0f) / 2, sqrt(2.0f) / 2, 0 });

	inv = Inverse(rot);
	result = result * inv;
	Check(v);

	return true;
}

static bool TestPlaneFromPointNormal()
{
	v3 normal{ 1, 0, 0 };
	rplane plane = PlaneFromPointNormal({ 2, 1, 0 }, normal);
	assert(GetPlaneNormal(plane) == normal && IS_EQ(plane.d, -2));

	normal = { -1, 0, 0 };
	plane = PlaneFromPointNormal({ 2, 1, 0 }, normal);
	assert(GetPlaneNormal(plane) == normal && IS_EQ(plane.d, 2));
	
	return true;
}

static bool TestIntersectLineAABB()
{
	rboundingbox bbox;
	bbox.vmin = { 0, 0, 0 };
	bbox.vmax = { 200, 100, 10 };

	v3 origin{ 50, 50, 30 }, dir{ 0, 0, -1 };

	float t;
	auto success = IntersectLineAABB(t, origin, dir, bbox);
	assert(success && IS_EQ(t, 20));

	success = IntersectLineSegmentAABB(t, origin, origin + dir * 100, bbox);
	assert(success && IS_EQ(t, 20));

	success = IntersectLineSegmentAABB(t, origin, origin + dir * 10, bbox);
	assert(!success);

	dir = { 1, 0, 0 };
	success = IntersectLineAABB(t, origin, dir, bbox);
	assert(!success);

	return true;
}

static bool TestDotAndCross()
{
	v3 a{ 1, 0, 0 }, b{ 0, 1, 0 };

	auto dot = DotProduct(a, b);
	assert(dot == 0);

	auto cross = CrossProduct(a, b);
	assert(cross == v3(0, 0, 1));

	return true;
}

static bool TestNormalize()
{
	v3 v{ 100, 100, 100 };
	auto n = Normalized(v);
	assert(n == v3(sqrt(3.0f) / 3, sqrt(3.0f) / 3, sqrt(3.0f) / 3));

	v = { 0, 0, 0 };
	n = Normalized(v);
	assert(n == v);

	return true;
}

static bool Equals(const rplane& a, const rplane& b)
{
	return IS_EQ(a.a, b.a) &&
		IS_EQ(a.b, b.b) &&
		IS_EQ(a.c, b.c) &&
		IS_EQ(a.d, b.d);
}

static bool TestTransform()
{
	rplane p{ 1, 0, 0, -100 };
	auto mat = RotationMatrix({ 0, 1, 0 }, static_cast<float>(TAU / 4));
	mat._43 = -100;
	mat = Transpose(Inverse(mat));
	auto result = p * mat;
	rplane expected_result{ 0, 0, -1, -200 };
	assert(Equals(expected_result, result));

	return true;
}

static bool TestQuaternions()
{
	v3 axis{ 0, 0, 1 };
	float angle = 90;

	auto reference_mat = RotationMatrix(axis, angle);

	auto quat = AngleAxisToQuaternion(axis, angle);
	auto quat_mat = QuaternionToMatrix(quat);

	assert(Equals(reference_mat, quat_mat));

	quat = MatrixToQuaternion(quat_mat);
	quat_mat = QuaternionToMatrix(quat);

	assert(Equals(reference_mat, quat_mat));

	return true;
}

bool TestMath()
{
	auto ret = true;

	ret &= TestIntersectLineSegmentPlane();
	ret &= TestIntersectTriangle();
	ret &= TestRotationMatrix();
	ret &= TestPlaneFromPointNormal();
	ret &= TestIntersectLineAABB();
	ret &= TestDotAndCross();
	ret &= TestNormalize();
	ret &= TestTransform();
	ret &= TestQuaternions();

	return ret;
}