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

enum ZC_STATE_UPPER {

	ZC_STATE_UPPER_NONE = 0,

	ZC_STATE_UPPER_SHOT,
	ZC_STATE_UPPER_RELOAD,
	ZC_STATE_UPPER_LOAD,

	ZC_STATE_UPPER_GUARD_START,
	ZC_STATE_UPPER_GUARD_IDLE,
	ZC_STATE_UPPER_GUARD_BLOCK1,
	ZC_STATE_UPPER_GUARD_BLOCK1_RET,
	ZC_STATE_UPPER_GUARD_BLOCK2,
	ZC_STATE_UPPER_GUARD_CANCEL,

	ZC_STATE_UPPER_END
};

enum ZC_STATE_LOWER {

	ZC_STATE_LOWER_NONE = 0,

	ZC_STATE_LOWER_IDLE1,
	ZC_STATE_LOWER_IDLE2,
	ZC_STATE_LOWER_IDLE3,
	ZC_STATE_LOWER_IDLE4,

	ZC_STATE_LOWER_RUN_FORWARD,
	ZC_STATE_LOWER_RUN_BACK,
	ZC_STATE_LOWER_RUN_LEFT,
	ZC_STATE_LOWER_RUN_RIGHT,

	ZC_STATE_LOWER_JUMP_UP,
	ZC_STATE_LOWER_JUMP_DOWN,

	ZC_STATE_LOWER_DIE1,
	ZC_STATE_LOWER_DIE2,
	ZC_STATE_LOWER_DIE3,
	ZC_STATE_LOWER_DIE4,

	ZC_STATE_LOWER_RUN_WALL_LEFT,
	ZC_STATE_LOWER_RUN_WALL_LEFT_DOWN,
	ZC_STATE_LOWER_RUN_WALL,
	ZC_STATE_LOWER_RUN_WALL_DOWN_FORWARD,
	ZC_STATE_LOWER_RUN_WALL_DOWN,
	ZC_STATE_LOWER_RUN_WALL_RIGHT,
	ZC_STATE_LOWER_RUN_WALL_RIGHT_DOWN,

	ZC_STATE_LOWER_TUMBLE_FORWARD,
	ZC_STATE_LOWER_TUMBLE_BACK,
	ZC_STATE_LOWER_TUMBLE_RIGHT,
	ZC_STATE_LOWER_TUMBLE_LEFT,

	ZC_STATE_LOWER_BIND,

	ZC_STATE_LOWER_JUMP_WALL_FORWARD,
	ZC_STATE_LOWER_JUMP_WALL_BACK,
	ZC_STATE_LOWER_JUMP_WALL_LEFT,
	ZC_STATE_LOWER_JUMP_WALL_RIGHT,

	ZC_STATE_LOWER_ATTACK1,
	ZC_STATE_LOWER_ATTACK1_RET,
	ZC_STATE_LOWER_ATTACK2,
	ZC_STATE_LOWER_ATTACK2_RET,
	ZC_STATE_LOWER_ATTACK3,
	ZC_STATE_LOWER_ATTACK3_RET,
	ZC_STATE_LOWER_ATTACK4,
	ZC_STATE_LOWER_ATTACK4_RET,
	ZC_STATE_LOWER_ATTACK5,

	ZC_STATE_LOWER_JUMPATTACK,			// 공중에서의 칼질.
	ZC_STATE_LOWER_UPPERCUT,			// 띄우기

	ZC_STATE_LOWER_GUARD_START,
	ZC_STATE_LOWER_GUARD_IDLE,
	ZC_STATE_LOWER_GUARD_BLOCK1,
	ZC_STATE_LOWER_GUARD_BLOCK1_RET,
	ZC_STATE_LOWER_GUARD_BLOCK2,
	ZC_STATE_LOWER_GUARD_CANCEL,

	ZC_STATE_LOWER_BLAST,
	ZC_STATE_LOWER_BLAST_FALL,
	ZC_STATE_LOWER_BLAST_DROP,
	ZC_STATE_LOWER_BLAST_STAND,
	ZC_STATE_LOWER_BLAST_AIRMOVE,

	ZC_STATE_LOWER_BLAST_DAGGER,
	ZC_STATE_LOWER_BLAST_DROP_DAGGER,

	ZC_STATE_DAMAGE,
	ZC_STATE_DAMAGE2,
	ZC_STATE_DAMAGE_DOWN,

	// 기타 특수모션들

	ZC_STATE_TAUNT,
	ZC_STATE_BOW,
	ZC_STATE_WAVE,
	ZC_STATE_LAUGH,
	ZC_STATE_CRY,
	ZC_STATE_DANCE,

	ZC_STATE_DAMAGE_BLOCKED,
	ZC_STATE_CHARGE,
	ZC_STATE_SLASH,
	ZC_STATE_JUMP_SLASH1,
	ZC_STATE_JUMP_SLASH2,

	ZC_STATE_DAMAGE_LIGHTNING,
	ZC_STATE_DAMAGE_STUN,

	ZC_STATE_PIT,

	ZC_STATE_LOWER_END
};

struct BasicInfo {
	v3 position;
	v3 velocity;
	//	rvector accel;
	v3 direction;
	ZC_STATE_UPPER upperstate;
	ZC_STATE_LOWER lowerstate;
	MMatchCharItemParts SelectedSlot;

	float SentTime;
	u32 RecvTime;

	void Validate()
	{
		if (SelectedSlot < MMCIP_PRIMARY || SelectedSlot > MMCIP_CUSTOM2)
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

struct MPICKINFO {
	class MMatchObject*	Object;
	RPickInfo	info;

	bool bBspPicked;
	RBSPPICKINFO bpi;
};