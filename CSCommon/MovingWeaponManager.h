#pragma once

#include "GlobalTypes.h"
#include <vector>
#include "MMatchItem.h"
#include "MultiVector.h"

class MovingWeaponMgr;

struct MovingWeapon
{
	v3 Pos;
	v3 Dir;
	v3 Vel;
};

struct MedKit : MovingWeapon
{
	MMatchItemDesc* ItemDesc;

	void OnCollision(MovingWeaponMgr& Mgr, const v3& ColPos, const MPICKINFO&);
};

struct Rocket : MovingWeapon
{
	MMatchObject* Owner;
	MMatchItemDesc* ItemDesc;

	void OnCollision(MovingWeaponMgr& Mgr, const v3& ColPos, const MPICKINFO&);
};

class MovingWeaponMgr
{
public:
	void Update(float Elapsed);
	class MMatchStage* Stage;

private:
	MultiVector<MedKit, Rocket> Weapons;
};