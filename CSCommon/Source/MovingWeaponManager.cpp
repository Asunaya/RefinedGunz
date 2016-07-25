#include "stdafx.h"
#include "MovingWeaponManager.h"
#include "HitRegistration.h"
#include "MMatchStage.h"
#undef pi

void MovingWeaponManager::Update(float Elapsed)
{
	Weapons.apply([&](auto& Obj)
	{
		auto diff = Obj.Vel * Elapsed;
		auto dir = diff;
		Normalize(dir);
		auto dist = Magnitude(diff);

		constexpr u32 PickFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

		v3 pickpos;
		MPICKINFO pi;
		bool bPicked = PickHistory<MMatchObject>(nullptr, Obj.Pos, Obj.Pos + diff, Stage->BspObject, pi,
			Stage->GetObjectList(), MGetMatchServer()->GetGlobalClockCount() / 1000.0, PickFlag);
		if (bPicked)
		{
			if (pi.bBspPicked)
			{
				pickpos = pi.bpi.PickPos;
			}
			else if (pi.pObject)
			{
#ifdef REFLECT_ROCKETS
				if (zpi.pObject->IsGuard() && DotProduct(zpi.pObject->GetDirection(), m_Dir) < 0)
				{
					auto ReflectedDir = GetReflectionVector(m_Dir, zpi.pObject->GetDirection());
					auto ReflectedVel = GetReflectionVector(m_Velocity, zpi.pObject->GetDirection());

					m_Dir = ReflectedDir;
					m_Velocity = ReflectedVel;

					diff = m_Velocity * fElapsedTime;
					dir = diff;
					Normalize(dir);

					bPicked = false;
				}
#endif

				pickpos = pi.info.vOut;
			}
		}

		if (bPicked && Magnitude(pickpos - Obj.Pos) < dist)
		{
			Obj.OnCollision(*this, pickpos, pi);
			return false;
		}
		else
		{
			Obj.Pos += diff;
		}

		return true;
	});
}

void MovingWeaponManager::AddRocket(MMatchObject* Owner, MMatchItemDesc* ItemDesc,
	const v3 & Pos, const v3 & Dir)
{
	Weapons.emplace<Rocket>(Pos, Dir, Dir * 2700, ItemDesc, Owner);
}

void MovingWeaponManager::AddItemKit(MMatchObject * Owner, MMatchItemDesc * ItemDesc,
	const v3 & Pos, const v3 & Dir)
{
	Weapons.emplace<MedKit>(Pos, Dir, v3{ 0, 0, 0 }, ItemDesc);
}

void Rocket::OnCollision(MovingWeaponManager& Mgr, const v3& ColPos, const MPICKINFO& pi)
{
	if (Mgr.Stage->GetStageSetting()->GetGameType() == MMATCH_GAMETYPE_SKILLMAP)
		return;

	constexpr auto ROCKET_SPLASH_RANGE = 350.f;
	constexpr auto ROCKET_MINIMUM_DAMAGE = .3f;
	constexpr auto ROCKET_KNOCKBACK_CONST = .5f;
	auto GetOrigin = [&](const auto& Obj, auto& Origin)
	{
		Obj.GetPositions(nullptr, &Origin, MGetMatchServer()->GetGlobalClockCount() - Owner->GetPing() / 1000.f);
	};

	GrenadeExplosion(*Owner, Mgr.Stage->GetObjectList(), ColPos, ItemDesc->m_nDamage,
		ROCKET_SPLASH_RANGE, ROCKET_MINIMUM_DAMAGE, ROCKET_KNOCKBACK_CONST,
		GetOrigin);
}