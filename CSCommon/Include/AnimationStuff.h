#pragma once

#include "GlobalTypes.h"
#include "RAnimation.h"
#include "RAnimationNode.h"
#include "RAnimationMgr.h"
#include "RTypes.h"
#include "MUtil.h"

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

struct ZANIMATIONINFO {
	char *Name;
	bool bEnableCancel;		// 캔슬 가능한지
	bool bLoop;				// 반복 되는 동작
	bool bMove;				// 움직임이 포함된 애니메이션
	bool bContinuos;		// 포함된 움직임이 시작부터 연결되어있는지.
};

static ZANIMATIONINFO g_AnimationInfoTableLower[ZC_STATE_LOWER_END] = {
	{ ""				,true	,true	,false 	,false },

	{ "idle"			,true	,true	,false 	,false },
	{ "idle2"			,true	,true	,false 	,false },
	{ "idle3"			,true	,true	,false 	,false },
	{ "idle4"			,true	,true	,false 	,false },

	{ "run"				,true	,true	,false 	,false },
	{ "runB"			,true	,true	,false 	,false },
	{ "runL"			,true	,true	,false 	,false },
	{ "runR"			,true	,true	,false 	,false },

	{ "jumpU"			,true	,false	,false 	,false },
	{ "jumpD"			,true	,false	,false 	,false },

	{ "die" 			,true	,false	,false 	,false },
	{ "die2" 			,true	,false	,false 	,false },
	{ "die3" 			,true	,false	,false 	,false },
	{ "die4"			,true	,false	,false 	,false },

	{ "runLW"			,false	,true 	,false	,false },
	{ "runLW_down"		,false	,false	,false	,false },
	{ "runW" 			,false	,false	,true	,false },
	{ "runW_downF"		,false	,false	,false	,false },
	{ "runW_downB"		,false	,false	,false	,false },
	{ "runRW" 			,false	,true 	,false	,false },
	{ "runRW_down"		,false	,false 	,false	,false },

	{ "tumbleF"			,false	,false	,false	,false },
	{ "tumbleB"			,false	,false	,false	,false },
	{ "tumbleR"			,false	,false	,false	,false },
	{ "tumbleL"			,false	,false	,false	,false },

	{ "bind"			,false	,false	,false	,false },

	{ "jumpwallF"		,false	,false 	,false	,false },
	{ "jumpwallB"		,false	,false 	,false	,false },
	{ "jumpwallL"		,false	,false 	,false	,false },
	{ "jumpwallR"		,false	,false 	,false	,false },

	{ "attack1"			,false	,false 	,true  	,false },
	{ "attack1_ret"		,false	,false 	,true  	,true },
	{ "attack2"			,false	,false 	,true  	,false },
	{ "attack2_ret"		,false	,false 	,true  	,true },
	{ "attack3"			,false	,false 	,true  	,false },
	{ "attack3_ret"		,false	,false 	,true  	,true },
	{ "attack4"			,false	,false 	,true  	,false },
	{ "attack4_ret"		,false	,false 	,true  	,true },
	{ "attack5"			,false	,false 	,true  	,false },

	{ "attack_Jump"		,false	,false 	,false	,false },
	{ "uppercut"		,false	,false 	,true	,false },

	{ "guard_start"		,false	,false 	,true 	,false },
	{ "guard_idle"		,false	,false 	,false	,false },
	{ "guard_block1"	,false	,false 	,false	,false },
	{ "guard_block1_ret",false	,false 	,false	,false },
	{ "guard_block2"	,false	,false 	,false	,false },
	{ "guard_cancel"	,false	,false 	,false	,false },

	{ "blast"			,false	,false 	,false 	,false },
	{ "blast_fall"		,false	,false 	,false 	,false },
	{ "blast_drop"		,false	,false 	,false 	,false },
	{ "blast_stand"		,false	,false 	,false 	,false },
	{ "blast_airmove"	,false	,false 	,false 	,false },

	{ "blast_dagger"	 ,false ,false 	,false 	,false },
	{ "blast_drop_dagger",false ,false 	,false 	,false },

	{ "damage"			,false	,false 	,false 	,false },
	{ "damage2"			,false	,false 	,false 	,false },
	{ "damage_down"		,false	,false 	,false 	,false },

	{ "taunt"			,true	,false	,false	,false },
	{ "bow"				,true	,false	,false	,false },
	{ "wave"			,true	,false	,false	,false },
	{ "laugh"			,true	,false	,false	,false },
	{ "cry"				,true	,false	,false	,false },
	{ "dance"			,true	,false	,false	,false },

	{ "cancel"			,false	,false 	,false 	,false },
	{ "charge"			,false	,false 	,true  	,true },
	{ "slash"			,false	,false 	,true  	,false },
	{ "jump_slash1"		,false	,false 	,false	,false },
	{ "jump_slash2"		,false	,false 	,false	,false },

	{ "lightning"		,false	,false 	,false	,false },
	{ "stun"			,false	,false 	,false	,false },	// 루프되는 스턴

	{ "pit"				,false	,false 	,false	,false },	// 나락에서 떨어지는 거
};

static ZANIMATIONINFO g_AnimationInfoTableUpper[ZC_STATE_UPPER_END] = {
	{ ""				,true	,true	,false	,false },

	{ "attackS"			,false	,false	,false	,false },
	{ "reload"			,false	,false	,false	,false },
	{ "load"			,false	,false	,false	,false },

	{ "guard_start"		,false	,false 	,false	,false },
	{ "guard_idle"		,false	,false 	,false	,false },
	{ "guard_block1"	,false	,false 	,false	,false },
	{ "guard_block1_ret",false	,false 	,false	,false },
	{ "guard_block2"	,false	,false 	,false	,false },
	{ "guard_cancel"	,false	,false 	,false	,false },
};

using namespace RealSpace2;

RAnimationMgr* GetAnimationMgr(enum MMatchSex Sex);
void SetAnimationMgr(enum MMatchSex Sex, RAnimationMgr* AniMgr);

v3 GetHeadPosition(RAnimation* LowerAni, RAnimation* UpperAni,
	int LowerFrame, int UpperFrame, float y, float tremble);
v3 GetFootPosition(RAnimation* LowerAni, int Frame);
int GetSelectWeaponDelay(struct MMatchItemDesc* pSelectItemDesc);
int GetFrame(RAnimation& Ani, ZC_STATE_LOWER LowerState, struct MMatchItemDesc* ItemDesc, float Time);
bool GetNodeMatrix(matrix& mat, const char* Name, const matrix* parent_base_inv,
	RAnimation* Ani, int Frame, float y, float tremble);
bool GetUpperSpine1(matrix& mat, RAnimation* Ani, int Frame, float y, float tremble);