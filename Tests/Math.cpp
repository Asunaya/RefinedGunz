#include "RMath.h"
#include "MMath.h"
#define GLM_GTC_quaternion 
#include "../sdk/glm/gtc/quaternion.hpp"
#include "../sdk/glm/gtx/quaternion.hpp"
#include "LogMatrix.h"
#include "TestAssert.h"

using namespace RealSpace2;

#if _WIN32 && !defined(TEST_COMPARE_TO_D3DX)
#define TEST_COMPARE_TO_D3DX 1
#endif

#if TEST_COMPARE_TO_D3DX && defined(_MSC_VER)
#pragma comment(lib, "d3dx9.lib")
#include <d3dx9.h>
#include "MWindows.h"
#endif

static void TestIntersectLineSegmentPlane()
{
	rplane p{ 1, 0, 0, -500 };

	v3 l0{ 0, 0, 0 };
	v3 l1{ 600, 0, 0 };
	v3 hit;
	auto result = IntersectLineSegmentPlane(&hit, p, l0, l1);

	TestAssert(result);
	TestAssert(hit.x == 500 && hit.y == 0 && hit.z == 0);

	std::swap(l0, l1);

	result = IntersectLineSegmentPlane(&hit, p, l0, l1);

	TestAssert(result);
	TestAssert(hit.x == 500 && hit.y == 0 && hit.z == 0);

	l0 = { 0, 0, 0 };
	l1 = { 499, 0, 0 };

	result = IntersectLineSegmentPlane(&hit, p, l0, l1);
	TestAssert(!result);

	p = { -1.000000f, 0.000000f, 0.000000f, 4104.648438f };
	l0 = { 4013.892578f, -943.908447f, 612.525452f };
	l1 = { 10194.750000f, 6517.653809f, -98.953125f };
	result = IntersectLineSegmentPlane(&hit, p, l0, l1);
	TestAssert(result);
	TestAssert(Equals(hit, v3(4104.648438f, -834.347534f, 602.078552f)));
}

static void TestIntersectTriangle()
{
	v3 v0{ 0, 0, 0 }, v1{ 1, 0, 0 }, v2{ 1, 1, 0 };
	v3 Origin{ 0.4f, 0.4f, -1 }, Dir{ 0, 0, 1 };
	float Dist{};

	auto Test = [&](bool ExpectedSuccess, float ExpectedDist = 0)
	{
		auto TestImpl = [&](auto&... tris)
		{
			auto success = IntersectTriangle(tris..., Origin, Dir, &Dist);
			TestAssert(success == ExpectedSuccess);
			if (ExpectedSuccess)
				TestAssert(IS_EQ(Dist, ExpectedDist));
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
}

static void TestRotationMatrix()
{
	v3 axis{ 0, 0, 1 };
	v3 v{ 1, 0, 0 };
	v3 result;

	auto Check = [&](const v3& vec) {
		TestAssert(IS_EQ(vec.x, result.x) && IS_EQ(vec.y, result.y) && IS_EQ(vec.z, result.z));
	};

	auto angle = static_cast<float>(TAU / 4);
	rmatrix rot = RotationMatrix(axis, angle);
	result = v * rot;
	Check({ 0, 1, 0 });

#ifdef TEST_COMPARE_TO_D3DX
	D3DXMATRIX d3dx_mat;
	D3DXVECTOR3 d3dx_axis = axis;
	D3DXMatrixRotationAxis(&d3dx_mat, &d3dx_axis, angle);
	D3DXVECTOR3 d3dx_result;
	D3DXVECTOR3 d3dx_v = v;
	D3DXVec3TransformCoord(&d3dx_result, &d3dx_v, &d3dx_mat);
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
}

static void TestPlaneFromPointNormal()
{
	v3 normal{ 1, 0, 0 };
	rplane plane = PlaneFromPointNormal({ 2, 1, 0 }, normal);
	TestAssert(GetPlaneNormal(plane) == normal && IS_EQ(plane.d, -2));

	normal = { -1, 0, 0 };
	plane = PlaneFromPointNormal({ 2, 1, 0 }, normal);
	TestAssert(GetPlaneNormal(plane) == normal && IS_EQ(plane.d, 2));
}

static void TestIntersectLineAABB()
{
	rboundingbox bbox;
	bbox.vmin = { 0, 0, 0 };
	bbox.vmax = { 200, 100, 10 };

	v3 origin{ 50, 50, 30 }, dir{ 0, 0, -1 };

	float t;
	auto success = IntersectLineAABB(origin, dir, bbox, &t);
	TestAssert(success && IS_EQ(t, 20));

	auto end = origin + dir * 100;
	auto norm = Normalized(end - origin);
	auto dirfrac = 1 / norm;
	success = IntersectLineSegmentAABB(origin, end, bbox, &t);
	TestAssert(success && IS_EQ(t, 20));

	success = IntersectLineSegmentAABB(origin, origin + dir * 10, bbox, &t);
	TestAssert(!success);

	dir = { 1, 0, 0 };
	success = IntersectLineAABB(origin, dir, bbox, &t);
	TestAssert(!success);
}

static void TestDotAndCross()
{
	v3 a{ 1, 0, 0 }, b{ 0, 1, 0 };

	auto dot = DotProduct(a, b);
	TestAssert(dot == 0);

	auto cross = CrossProduct(a, b);
	TestAssert(cross == v3(0, 0, 1));
}

static void TestNormalize()
{
	v3 v{ 100, 100, 100 };
	auto n = Normalized(v);
	TestAssert(n == v3(sqrt(3.0f) / 3, sqrt(3.0f) / 3, sqrt(3.0f) / 3));

	v = { 0, 0, 0 };
	n = Normalized(v);
	TestAssert(n == v);
}

static bool Equals(const rplane& a, const rplane& b)
{
	return IS_EQ(a.a, b.a) &&
		IS_EQ(a.b, b.b) &&
		IS_EQ(a.c, b.c) &&
		IS_EQ(a.d, b.d);
}

static void TestTransform()
{
	rplane p{ 1, 0, 0, -100 };
	auto mat = RotationMatrix({ 0, 1, 0 }, static_cast<float>(TAU / 4));
	mat._43 = -100;
	mat = Transpose(Inverse(mat));
	auto result = p * mat;
	rplane expected_result{ 0, 0, -1, -200 };
	TestAssert(Equals(expected_result, result));
}

inline v3 operator *(const v3& vec, const rquaternion& quat) {
	v3 quat_vec{ EXPAND_VECTOR(quat) };
	auto uv = CrossProduct(quat_vec, vec);
	auto uuv = CrossProduct(quat_vec, uv);
	return vec + ((uv * quat.w) + uuv) * 2;
}

static void TestQuaternionAxisAngle(const v3& axis, float angle)
{
	auto test_vec = Normalized(v3{ 1, 1, 1 });
	auto ref_mat = RotationMatrix(axis, angle);

	auto quat = Normalized(AngleAxisToQuaternion(axis, angle));
	auto quat_mat = QuaternionToMatrix(quat);

	auto expected = test_vec * ref_mat;
	auto actual = test_vec * quat_mat;

	TestAssert(Equals(expected, actual));

#ifdef TEST_COMPARE_TO_D3DX
	D3DXQUATERNION d3dx_quat;

	D3DXVECTOR3 d3dx_axis = axis;
	D3DXQuaternionRotationAxis(&d3dx_quat, &d3dx_axis, angle);
	expected = test_vec * d3dx_quat;
	actual = test_vec * quat;
#endif

	TestAssert(Equals(expected, actual));

	quat = MatrixToQuaternion(quat_mat);
	auto norm_quat = Normalized(quat);
	quat_mat = QuaternionToMatrix(norm_quat);

	expected = test_vec * ref_mat;
	actual = test_vec * quat_mat;

	TestAssert(Equals(expected, actual));

#ifdef TEST_COMPARE_TO_D3DX
	D3DXMATRIX d3dx_mat;

	d3dx_axis = axis;
	D3DXMatrixRotationAxis(&d3dx_mat, &d3dx_axis, angle);
	TestAssert(Equals(d3dx_mat, quat_mat));
	TestAssert(Equals(d3dx_mat, ref_mat));

	D3DXQUATERNION d3dx_norm_quat = norm_quat;
	D3DXMatrixRotationQuaternion(&d3dx_mat, &d3dx_norm_quat);

	TestAssert(Equals(d3dx_mat, quat_mat));
#endif
}

static v3 test_vecs[] = {
	{ 1, 0, 0 },
	{ 0, 1, 0 },
	{ 0, 0, 1 },
	{ 1, 1, 0 },
	{ 1, 1, 1 },
	{ 1, 0.5, 0.333f },
};

static void TestSlerp(const rquaternion& a, const rquaternion& b)
{
	for (auto&& test_vec : test_vecs)
	{
		for (int i = 1; i < 50; ++i)
		{
#ifdef TEST_COMPARE_TO_D3DX
			auto t = sin(TAU_FLOAT / i);

			auto actual_quat = Slerp(a, b, t);
			D3DXQUATERNION expected_quat;
			D3DXQUATERNION d3dx_quat = a;
			D3DXQUATERNION d3dx_quat2 = b;
			D3DXQuaternionSlerp(&expected_quat, &d3dx_quat, &d3dx_quat2, t);

			auto expected = test_vec * expected_quat;
			auto actual = test_vec * actual_quat;
			TestAssert(Equals(expected, actual));
#endif
		}
	}
}

static void TestQuaternions()
{
	auto ret = true;
	rquaternion last = Normalized(rquaternion{ 1, 1, 1, 1 });

	auto TestAxisAngle = [&](const v3& axis, float angle) {
		TestQuaternionAxisAngle(Normalized(axis), angle);
		
		auto quat = AngleAxisToQuaternion(axis, angle);
		TestSlerp(quat, last);
		last = quat;
	};

	for (int i = 1; i < 16; ++i)
	{
		auto Test = [&](const v3& axis) {
			TestAxisAngle(axis, TAU_FLOAT / i);
		};

		Test({ 1, 0, 0 });
		Test({ 0, 1, 0 });
		Test({ 0, 0, 1 });
		Test({ 1, 1, 0 });
		Test({ 1, 1, 1 });
		Test({ 1, 0.5, 0.333f });
	}

	rquaternion a{ 0.814870715f, -0.239277035f, 0.0268819593f, 0.527266204f };
	rquaternion b{ -0.839663744f, 0.264600188f, -0.0283754189f, -0.473441035f };
	TestSlerp(a, b);
}

static void TestInterpolation()
{
	v3 p{ 1, 2, 3 };
	auto expected = p;
	auto actual = Lerp(p, p, 0.5);
	TestAssert(expected == actual);

	actual = Slerp(p, p, 0.5);
	TestAssert(expected == actual);
}

void TestMath()
{
	TestIntersectLineSegmentPlane();
	TestIntersectTriangle();
	TestRotationMatrix();
	TestPlaneFromPointNormal();
	TestIntersectLineAABB();
	TestDotAndCross();
	TestNormalize();
	TestTransform();
	TestQuaternions();
	TestInterpolation();
}