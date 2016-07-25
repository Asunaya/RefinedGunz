#pragma once

#include "GlobalTypes.h"
#include "MMath.h"
#include "RTypes.h"
#include <random>
#include "RBspObject.h"

using namespace RealSpace2;

template <typename rngT>
float RandomAngle(rngT& rng)
{
	std::uniform_real_distribution<float> dist(0, TAU);
	return dist(rng);
}

inline void CalcRangeShotControllability(v3& vOutDir, const v3& vSrcDir,
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

#define SHOTGUN_BULLET_COUNT	12
#define SHOTGUN_DIFFUSE_RANGE	0.1f

template <typename rngT>
v3 GetShotgunPelletDir(const v3& dir, rngT& rng)
{
	std::uniform_real_distribution<float> dist(0, SHOTGUN_DIFFUSE_RANGE);

	rvector r, up(0, 0, 1), right;
	D3DXQUATERNION q;
	D3DXMATRIX mat;

	float fAngle = RandomAngle(rng);
	float fForce = dist(rng);

	D3DXVec3Cross(&right, &dir, &up);
	D3DXVec3Normalize(&right, &right);
	D3DXMatrixRotationAxis(&mat, &right, fForce);
	D3DXVec3TransformCoord(&r, &dir, &mat);

	D3DXMatrixRotationAxis(&mat, &dir, fAngle);
	D3DXVec3TransformCoord(&r, &r, &mat);

	return r;
}

inline auto GetShotgunPelletDirGenerator(const v3& orig_dir, u32 seed)
{
	return [&orig_dir, rng = std::mt19937(seed)]() mutable
	{
		return GetShotgunPelletDir(orig_dir, rng);
	};
}

enum ZOBJECTHITTEST {
	ZOH_NONE = 0,
	ZOH_BODY = 1,
	ZOH_HEAD = 2,
	ZOH_LEGS = 3
};

inline ZOBJECTHITTEST PlayerHitTest(const v3& head, const v3& foot,
	const v3& src, const v3& dest, v3* pOutPos = nullptr)
{
	// 적절한 시점의 위치를 얻어낼수없으면 실패..
	rvector footpos, headpos, characterdir;

	footpos = foot;
	headpos = head;

	footpos.z += 5.f;
	headpos.z += 5.f;

	rvector rootpos = (footpos + headpos)*0.5f;

	// headshot 판정
	rvector nearest = GetNearestPoint(headpos, src, dest);
	float fDist = Magnitude(nearest - headpos);
	float fDistToChar = Magnitude(nearest - src);

	rvector ap, cp;

	if (fDist < 15.f)
	{
		if (pOutPos) *pOutPos = nearest;
		return ZOH_HEAD;
	}
	else
	{
		rvector dir = dest - src;
		Normalize(dir);

		// 실린더로 몸통 판정
		rvector rootdir = (rootpos - headpos);
		Normalize(rootdir);
		float fDist = GetDistanceBetweenLineSegment(src, dest, headpos + 20.f*rootdir,
			rootpos - 20.f*rootdir, &ap, &cp);

		if (fDist < 30)		// 상체
		{
			rvector ap2cp = ap - cp;
			float fap2cpsq = D3DXVec3LengthSq(&ap2cp);
			float fdiff = sqrtf(30.f*30.f - fap2cpsq);

			if (pOutPos) *pOutPos = ap - dir*fdiff;;
			return ZOH_BODY;
		}
		else
		{
			float fDist = GetDistanceBetweenLineSegment(src, dest, rootpos - 20.f*rootdir,
				footpos, &ap, &cp);

			if (fDist < 30)	// 하체
			{
				rvector ap2cp = ap - cp;
				float fap2cpsq = D3DXVec3LengthSq(&ap2cp);
				float fdiff = sqrtf(30.f*30.f - fap2cpsq);

				if (pOutPos) *pOutPos = ap - dir*fdiff;
				return ZOH_LEGS;
			}
		}
	}

	return ZOH_NONE;
}

template <typename ObjectT, typename ContainerT, typename PickInfoT>
bool PickHistory(const ObjectT* Exception, const v3& src, const v3& dest,
	RBspObject* BspObject, PickInfoT& pickinfo, const ContainerT& Container,
	double Time, u32 PassFlag = RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY | RM_FLAG_HIDE)
{
	ObjectT* HitObject = nullptr;
	v3 HitPos;
	pickinfo.info.t = 0;

	for (auto& Obj : Container)
	{
		if (Exception == Obj)
			continue;

		v3 TempHitPos;
		auto HitParts = Obj->HitTest(src, dest, Time, &TempHitPos);

		if (HitParts == ZOH_NONE)
			continue;

		if (!Obj->IsAlive())
			continue;

		if (!HitObject || Magnitude(TempHitPos - src) < Magnitude(HitPos - src))
		{
			HitObject = Obj;
			HitPos = TempHitPos;
			switch (HitParts)
			{
			case ZOH_HEAD: pickinfo.info.parts = eq_parts_head; break;
			case ZOH_BODY: pickinfo.info.parts = eq_parts_chest; break;
			case ZOH_LEGS:	pickinfo.info.parts = eq_parts_legs; break;
			}
			pickinfo.info.vOut = TempHitPos;
		}
	}

#define DIDNT_HIT_BSP()					\
	pickinfo.bBspPicked = false;		\
	pickinfo.pObject = HitObject;		\
	return pickinfo.pObject != nullptr;	\


	if (!BspObject)
	{
		DIDNT_HIT_BSP();
	}

	bool HitBsp = BspObject->PickTo(src, dest, &pickinfo.bpi, PassFlag);
	if (!HitBsp)
	{
		DIDNT_HIT_BSP();
	}

	if (HitObject && Magnitude(HitPos - src) < Magnitude(pickinfo.bpi.PickPos - src))
	{
		DIDNT_HIT_BSP();
	}

	pickinfo.bBspPicked = true;
	pickinfo.pObject = nullptr;

	return true;

#undef DIDNT_HIT_BSP
}

template <typename ObjectT, typename ContainerT, typename GetOriginT>
void GrenadeExplosion(const ObjectT& Owner, const ContainerT& Container, const v3& ExplosionPos,
	float fDamage, float fRange, float fMinDamage, float fKnockBack, const GetOriginT& GetOrigin)
{
	float fDist, fDamageRange;

	for (auto* Target : Container)
	{
		if (!Target || !Target->IsAlive())
			continue;

		v3 TargetOrigin;
		GetOrigin(*Target, TargetOrigin);

		fDist = Magnitude(ExplosionPos - (TargetOrigin + v3(0, 0, 80)));
		if (fDist > fRange)
			continue;

		auto dir = ExplosionPos - (TargetOrigin + v3(0, 0, 80));
		Normalize(dir);

		// 몸에 직접 맞았다.
		if (GetDistance(ExplosionPos, TargetOrigin + v3(0, 0, 50), TargetOrigin + v3(0, 0, 130)) < 50)
		{
			fDamageRange = 1.f;
		}
		else
		{
			constexpr auto MAX_DMG_RANGE = 50.f;

			fDamageRange = 1.f - (1.f - fMinDamage)*(max(fDist - MAX_DMG_RANGE, 0) / (fRange - MAX_DMG_RANGE));
		}

		float fActualDamage = fDamage * fDamageRange;
		float fRatio = GetPiercingRatio(MWT_FRAGMENTATION, eq_parts_chest);
		Target->OnDamaged(Owner, ExplosionPos, ZD_EXPLOSION, MWT_FRAGMENTATION, fActualDamage, fRatio);
	}
}