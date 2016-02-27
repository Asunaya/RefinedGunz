#include "stdafx.h"
#include "RCollisionDetection.h"

_NAMESPACE_REALSPACE2_BEGIN

float SweepTest(const rsphere& sphere, const rvector& vec, const rplane &pln, rplane* out)
{
	float d1, d2;
	rvector pln_normal = rvector(pln.a, pln.b, pln.c);

	d1 = DotProduct(vec, pln_normal);

	if (d1 < 0.0f) {
		d2 = GetDistance(sphere.Pos(), pln) - sphere.radius;

		if (d2 >= 0 && d2 < -d1) {
			float t = d2 / (-d1);

			if (out) {
				*out = rplane(pln.a, pln.b, pln.c, pln.d-sphere.radius);
			}
			return t;
		}
	}

	return 1.0f;
}

static float _RayIntersect(const rvector& pivot, const rvector& pos, const rvector& vec, float radius)
{
	rvector dv;
	float dvl;

	dv = pivot - pos;
	dvl = Magnitude(dv);

//	겹쳐 있으면 충돌체크가 불가능하다
//
	if (dvl < radius)
		return 1.0f;

	float dotp = DotProduct(dv, vec);

//	도달 불가능
//
	if (dotp / dvl < dvl-radius)
		return 1.0f;

//	무한히 가도 닿을 수 없는 위치
//
	float vecl = Magnitude(vec);
	float l2 = dotp / vecl;
	float db = (float) sqrt(dvl*dvl - l2*l2);

	if (db > radius)
		return 1.0f;

//	충돌 지점의 거리 구하기
//
	float dc = (float) sqrt(radius*radius - db*db);
	float dd = l2 - dc;

	if (dd < vecl)
		return dd / vecl;

	return 1.0f;
}


float SweepTest(const rsphere & sphere, const rvector& vec, const rsphere& body, rplane* out)
{
	rvector dv, p;
	float radius = sphere.radius + body.radius, t;
	int i;

//	aabb 체크먼저 한다
	for(i=0; i<3; i++) {
		if (((float*)&vec)[i] > 0.0f) {
			if (((float*)&sphere.x)[i] - radius > ((float*)&body.x)[i] || ((float*)&sphere.x)[i] + ((float*)&vec)[i] + radius < ((float*)&body.x)[i])
				return 1.0f;
		} else {
			if (((float*)&sphere.x)[i] + radius < ((float*)&body.x)[i] || ((float*)&sphere.x)[i] + ((float*)&vec)[i] - radius > ((float*)&body.x)[i])
				return 1.0f;
		}
	}

//	충돌체크
	t = _RayIntersect(body.Pos(), sphere.Pos(), vec, radius);

	if (t < 1.0f) {
		if (out) {
			rvector norm = (sphere.Pos() + vec * t) - body.Pos();
			Normalize(norm);
			rvector plane_pos = body.Pos() + (norm * body.radius);
			float d = -DotProduct(norm, plane_pos);

			*out = rplane(norm.x, norm.y, norm.z, d);
		}
		return t;
	}

	return 1.0f;
}

float SweepTest(const rsphere& sphere, const rvector& vec, const rcapsule &capsule, rplane* out)
{
//	먼저 러프하게 aabb 체크해봄

//	충돌 불가능한 위치 체크
//
	float d1 = DotProduct(capsule.lerp, vec);
	float d2 = DotProduct(capsule.lerp, sphere.Pos() - capsule.pivot);

	if (d1 < 0) {
		if (d2+d1 >= capsule.length+(sphere.radius+capsule.radius))
			return 1.0f;
	}	else {
		if (d2 < -(sphere.radius+capsule.radius))
			return 1.0f;
	}

//	capsule.pivot 성분을 제거하여 capsule.pivot 을 지나고 capsule.pivot 과 직각인 평면위의 성분으로 만든다
//
	rvector p1 = sphere.Pos() - d2 * capsule.lerp;
	rvector v1 = vec - d1 * capsule.lerp;

//	p1, v1 을 이용하여 구의 직선의 충돌 체크를 한다
//
	float t = _RayIntersect(capsule.pivot, p1, v1, sphere.radius + capsule.radius);

	if (t < 1.0f) {
//		유효한 위치인지 확인
//
		rvector p = sphere.Pos() + vec * t;
		float v = DotProduct(capsule.lerp, p - capsule.pivot);

		if (v > 0 && v < capsule.length) {
			if (out) {
				rvector norm = p1 + v1 * t - capsule.pivot;
				Normalize(norm);
				rvector plane_pos = p - norm * sphere.radius;
				float d = -DotProduct(norm, plane_pos);
				*out = rplane(norm.x, norm.y, norm.z, d);
			}

			return t;
		}
	}

//	뚜껑과 비교
//
	if (d1 > 0) { // 윗뚜껑
		if (d2 < 0) {
			t = _RayIntersect(capsule.pivot, sphere.Pos(), vec, sphere.radius + capsule.radius);

			if (t < 1.0f) {
				if (out) {
					rvector norm = sphere.Pos() + vec * t - capsule.pivot;
					Normalize(norm);
					rvector plane_pos = capsule.pivot + norm * capsule.radius;
					float d = -DotProduct(norm, plane_pos);
					*out = rplane(norm.x, norm.y, norm.z, d);
				}
				return t;
			}
		}
	}	else {
		if (d2 > capsule.length) {
			rvector v2 = capsule.pivot + capsule.lerp;
			t = _RayIntersect(v2, sphere.Pos(), vec, sphere.radius + capsule.radius);

			if (t < 1.0f) {
				if (out) {
					rvector norm = sphere.Pos() + vec * t - v2;
					Normalize(norm);
					rvector plane_pos = v2 + norm * capsule.radius;
					float d = -DotProduct(norm, plane_pos);
					*out = rplane(norm.x, norm.y, norm.z, d);
				}
				return t;
			}
		}	}

	return 1.0f;
}



_NAMESPACE_REALSPACE2_END