#pragma once

#define POINTER_64 __ptr64

#include "targetver.h"

#ifdef MFC
#include <afxdb.h>
#include <afxtempl.h>
#include <afxdtctl.h>
#endif

#include <Winsock2.h>

#include "GlobalTypes.h"
#undef pi
#include "MMatchItem.h"
#define pi D3DX_PI
#include "AnimationStuff.h"
#include "RMesh.h"
#include "RBspObject.h"
#include "MMatchUtil.h"

struct BasicInfo {
	v3 position;
	v3 velocity;
	v3 direction;
	v3 cameradir;
	ZC_STATE_UPPER upperstate;
	ZC_STATE_LOWER lowerstate;
	MMatchCharItemParts SelectedSlot;

	void Validate()
	{
		if (SelectedSlot < MMCIP_MELEE || SelectedSlot > MMCIP_CUSTOM2)
			SelectedSlot = MMCIP_PRIMARY;
	}
};

struct ZBasicInfo {
	v3 position;
	v3 velocity;
	v3 direction;
	v3 targetdir;
};

#pragma pack(push, 1)

struct ZPACKEDBASICINFO {
	float	fTime;
	short	posx, posy, posz;
	short	velx, vely, velz;
	short	dirx, diry, dirz;
	u8	upperstate;
	u8	lowerstate;
	u8	selweapon;

	void Unpack(ZBasicInfo& bi)
	{
		bi.position = v3(posx, posy, posz);
		bi.velocity = v3(velx, vely, velz);
		bi.direction = 1.f / 32000.f * v3(dirx, diry, dirz);
	}

	void Pack(const ZBasicInfo& bi)
	{
		posx = short(bi.position.x);
		posy = short(bi.position.y);
		posz = short(bi.position.z);

		velx = short(bi.velocity.x);
		vely = short(bi.velocity.y);
		velz = short(bi.velocity.z);

		dirx = short(bi.direction.x * 32000);
		diry = short(bi.direction.y * 32000);
		dirz = short(bi.direction.z * 32000);
	}

	void Unpack(BasicInfo& bi)
	{
		bi.position = v3(posx, posy, posz);
		bi.velocity = v3(velx, vely, velz);
		bi.direction = 1.f / 32000.f * v3(dirx, diry, dirz);
		bi.upperstate = ZC_STATE_UPPER(upperstate);
		bi.lowerstate = ZC_STATE_LOWER(lowerstate);
		bi.SelectedSlot = MMatchCharItemParts(selweapon);
		bi.Validate();
	}

	void Pack(const BasicInfo& bi)
	{
		posx = short(bi.position.x);
		posy = short(bi.position.y);
		posz = short(bi.position.z);

		velx = short(bi.velocity.x);
		vely = short(bi.velocity.y);
		velz = short(bi.velocity.z);

		dirx = short(bi.direction.x * 32000);
		diry = short(bi.direction.y * 32000);
		dirz = short(bi.direction.z * 32000);

		upperstate = bi.upperstate;
		lowerstate = bi.lowerstate;
	}
};

#pragma pack(pop)

enum MMatchWeaponType;

typedef enum _RMeshPartsType RMeshPartsType;
enum RWeaponMotionType;

RWeaponMotionType WeaponTypeToMotionType(MMatchWeaponType WeaponType);

float GetPiercingRatio(MMatchWeaponType wtype, RMeshPartsType partstype);

enum ZDAMAGETYPE {
	ZD_NONE = -1,
	ZD_BULLET,
	ZD_MELEE,
	ZD_FALLING,
	ZD_EXPLOSION,
	ZD_BULLET_HEADSHOT,
	ZD_KATANA_SPLASH,
	ZD_HEAL,
	ZD_REPAIR,
	ZD_MAGIC,

	ZD_END
};

enum ZC_SHOT_SP_TYPE {
	ZC_WEAPON_SP_NONE = 0,

	// grenade type

	ZC_WEAPON_SP_GRENADE,
	ZC_WEAPON_SP_ROCKET,
	ZC_WEAPON_SP_FLASHBANG,
	ZC_WEAPON_SP_SMOKE,
	ZC_WEAPON_SP_TEAR_GAS,

	// item type

	ZC_WEAPON_SP_ITEMKIT,	// medikit, repairkit, bulletkit

	ZC_WEAPON_SP_END,
};

struct MPICKINFO {
	class MMatchObject*	pObject;
	RPickInfo	info;

	bool bBspPicked;
	RBSPPICKINFO bpi;
};