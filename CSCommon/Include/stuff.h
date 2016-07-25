#pragma once

#define POINTER_64 __ptr64

#include "targetver.h"

// 여기다 include
#include <afxdb.h>
#include <afxtempl.h>
#include <afxdtctl.h>

#include <Winsock2.h>

#include "GlobalTypes.h"
#include "RMeshUtil.h"
#undef pi
#include "MMatchItem.h"
#define pi D3DX_PI
#include "RBspObject.h"
#include "AnimationStuff.h"

struct BasicInfo {
	v3 position;
	v3 velocity;
	v3 direction;
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
	//	rvector accel;
	v3 direction;
	//	ZC_STATE_UPPER upperstate;
	//	ZC_STATE_LOWER lowerstate;
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

static RWeaponMotionType WeaponTypeToMotionType(MMatchWeaponType WeaponType)
{
	switch (WeaponType)
	{
	case MWT_ROCKET:
		return eq_wd_rlauncher;
	case MWT_SHOTGUN:
		return eq_wd_shotgun;
	case MWT_RIFLE:
		return eq_wd_rifle;
	case MWT_DAGGER:
		return eq_wd_dagger;
	case MWT_KATANA:
		return eq_wd_katana;
	case MWT_PISTOL:
		return eq_ws_pistol;
	case MWT_PISTOLx2:
		return eq_wd_pistol;
	case MWT_SMG:
		return eq_ws_smg;
	case MWT_SMGx2:
		return eq_wd_smg;
	case MWT_MACHINEGUN:
		return eq_wd_shotgun;
	case MWT_MED_KIT:
	case MWT_REPAIR_KIT:
	case MWT_BULLET_KIT:
	case MWT_FLASH_BANG:
	case MWT_FRAGMENTATION:
	case MWT_SMOKE_GRENADE:
	case MWT_FOOD:
	case MWT_SKILL:
		return eq_wd_item;
	case MWT_GREAT_SWORD:
		return eq_wd_sword;
	case MWT_DOUBLE_KATANA:
		return eq_wd_blade;
	};

	return eq_weapon_etc;
}

static float GetPiercingRatio(MMatchWeaponType wtype, RMeshPartsType partstype)
{
	float fRatio = 0.5f;

	bool bHead = false;

	if (partstype == eq_parts_head) { // 헤드샷 구분~ 
		bHead = true;
	}

	switch (wtype) {

	case MWT_DAGGER:		// 단검
	case MWT_DUAL_DAGGER:	// 양손단검
	{
		if (bHead)	fRatio = 0.75f;
		else		fRatio = 0.7f;
	}
	break;
	case MWT_KATANA:		// 카타나
	{
		if (bHead)	fRatio = 0.65f;
		else		fRatio = 0.6f;
	}
	break;
	case MWT_DOUBLE_KATANA:
	{
		if (bHead)	fRatio = 0.65f;
		else		fRatio = 0.6f;
	}
	break;
	case MWT_GREAT_SWORD:
	{
		if (bHead)	fRatio = 0.65f;
		else		fRatio = 0.6f;
	}
	break;

	case MWT_PISTOL:
	case MWT_PISTOLx2:
	{
		if (bHead)	fRatio = 0.7f;
		else		fRatio = 0.5f;
	}
	break;

	case MWT_REVOLVER:
	case MWT_REVOLVERx2:
	{
		if (bHead)	fRatio = 0.9f;
		else		fRatio = 0.7f;
	}
	break;

	case MWT_SMG:
	case MWT_SMGx2:			// 서브머신건
	{
		if (bHead)	fRatio = 0.5f;
		else		fRatio = 0.3f;
	}
	break;
	case MWT_SHOTGUN:
	case MWT_SAWED_SHOTGUN:
	{
		if (bHead)	fRatio = 0.2f;
		else		fRatio = 0.2f;
	}
	break;
	case MWT_MACHINEGUN:	// 머신건
	{
		if (bHead)	fRatio = 0.8f;
		else		fRatio = 0.4f;
	}
	break;
	case MWT_RIFLE:			// 돌격소총
	{
		if (bHead)	fRatio = 0.8f;
		else		fRatio = 0.4f;
	}
	break;
	case MWT_SNIFER:		//우선은 라이플처럼...
	{
		if (bHead)	fRatio = 0.8f;
		else		fRatio = 0.4f;
	}
	break;
	case MWT_FRAGMENTATION:
	case MWT_FLASH_BANG:
	case MWT_SMOKE_GRENADE:
	case MWT_ROCKET:		// 로켓런쳐
	{
		if (bHead)	fRatio = 0.4f;
		else		fRatio = 0.4f;
	}
	break;
	default:
		// case eq_wd_item
		break;
	}
	return fRatio;
}

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

	ZC_WEAPON_SP_ITEMKIT,	// medikit, repairkit, bulletkit 등등

	ZC_WEAPON_SP_END,
};

struct MPICKINFO {
	class MMatchObject*	pObject;
	RPickInfo	info;

	bool bBspPicked;
	RBSPPICKINFO bpi;
};