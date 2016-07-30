#include "stdafx.h"
#include "HitRegistration.h"

void CalcRangeShotControllability(v3& vOutDir, const v3& vSrcDir,
	int nControllability, u32 seed, float CtrlFactor)
{
	v3 up(0, 0, 1);
	v3 right;
	D3DXMATRIX mat;

	std::mt19937 rng;
	rng.seed(seed);

	float fAngle = RandomAngle(rng);

	float fControl;
	if (nControllability <= 0)
	{
		fControl = 0;
	}
	else
	{
		std::uniform_real_distribution<float> dist(0, nControllability * 100);
		fControl = dist(rng);
		fControl = fControl * CtrlFactor;
	}

	float fForce = fControl / 100000.0f;

	D3DXVec3Cross(&right, &vSrcDir, &up);
	D3DXVec3Normalize(&right, &right);
	D3DXMatrixRotationAxis(&mat, &right, fForce);
	D3DXVec3TransformCoord(&vOutDir, &vSrcDir, &mat);

	D3DXMatrixRotationAxis(&mat, &vSrcDir, fAngle);
	D3DXVec3TransformCoord(&vOutDir, &vOutDir, &mat);

	Normalize(vOutDir);
}

ZOBJECTHITTEST PlayerHitTest(const v3& head, const v3& foot,
	const v3& src, const v3& dest, v3* pOutPos = nullptr)
{
	// 적절한 시점의 위치를 얻어낼수없으면 실패..
	v3 footpos, headpos, characterdir;

	footpos = foot;
	headpos = head;

	footpos.z += 5.f;
	headpos.z += 5.f;

	auto rootpos = (footpos + headpos)*0.5f;

	auto nearest = GetNearestPoint(headpos, src, dest);
	float fDist = Magnitude(nearest - headpos);
	float fDistToChar = Magnitude(nearest - src);

	v3 ap, cp;

	// Head
	if (fDist < 15.f)
	{
		if (pOutPos) *pOutPos = nearest;
		return ZOH_HEAD;
	}

	auto dir = dest - src;
	Normalize(dir);

	auto rootdir = (rootpos - headpos);
	Normalize(rootdir);
	float fDist = GetDistanceBetweenLineSegment(src, dest, headpos + 20.f*rootdir,
		rootpos - 20.f*rootdir, &ap, &cp);

	// Body
	if (fDist < 30)
	{
		auto ap2cp = ap - cp;
		float fap2cpsq = D3DXVec3LengthSq(&ap2cp);
		float fdiff = sqrtf(30.f*30.f - fap2cpsq);

		if (pOutPos) *pOutPos = ap - dir*fdiff;
		return ZOH_BODY;
	}

	float fDist = GetDistanceBetweenLineSegment(src, dest, rootpos - 20.f*rootdir,
		footpos, &ap, &cp);

	// Legs
	if (fDist < 30)
	{
		auto ap2cp = ap - cp;
		float fap2cpsq = D3DXVec3LengthSq(&ap2cp);
		float fdiff = sqrtf(30.f*30.f - fap2cpsq);

		if (pOutPos) *pOutPos = ap - dir*fdiff;
		return ZOH_LEGS;
	}

	return ZOH_NONE;
}