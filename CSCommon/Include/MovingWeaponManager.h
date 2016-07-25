#pragma once

#include "GlobalTypes.h"
#include <vector>
#include "MMatchItem.h"
#include "MultiVector.h"

class MovingWeaponManager;

struct MovingWeapon
{
	v3 Pos;
	v3 Dir;
	v3 Vel;
};

template <typename T>
struct ItemKit : MovingWeapon
{
	MMatchItemDesc* ItemDesc;

	ItemKit(const v3& Pos, const v3& Dir, const v3& Vel, MMatchItemDesc* ItemDesc)
		: MovingWeapon({ Pos, Dir, Vel }), ItemDesc(ItemDesc)
	{ }

	void OnCollision(MovingWeaponManager& Mgr, const v3& ColPos, const MPICKINFO& mpi)
	{
		Vel = { 0, 0, 0 };
	}

	void Update(float Elapsed)
	{
	}
};

struct MedKit : ItemKit<MedKit>
{
	using ItemKit::ItemKit;

	void OnHitPlayer(MMatchObject& Obj)
	{
	}
};

struct Rocket : MovingWeapon
{
	MMatchItemDesc* ItemDesc;
	MMatchObject* Owner;

	Rocket(const v3& Pos, const v3& Dir, const v3& Vel, MMatchItemDesc* ItemDesc, MMatchObject* Owner)
		: MovingWeapon({ Pos, Dir, Vel }), ItemDesc(ItemDesc), Owner(Owner)
	{ }

	void OnCollision(MovingWeaponManager& Mgr, const v3& ColPos, const MPICKINFO&);
};

class MovingWeaponManager
{
public:
	MovingWeaponManager(MMatchStage& Stage)
		: Stage(&Stage)
	{ }
	void Update(float Elapsed);
	void AddRocket(MMatchObject* Owner, MMatchItemDesc* ItemDesc, const v3& Pos, const v3& Dir);
	void AddItemKit(MMatchObject* Owner, MMatchItemDesc* ItemDesc, const v3& Pos, const v3& Dir);

	class MMatchStage* Stage;

private:
	MultiVector<MedKit, Rocket> Weapons;
};