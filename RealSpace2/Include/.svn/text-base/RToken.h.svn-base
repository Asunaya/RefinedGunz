#ifndef _RTOKEN_H
#define _RTOKEN_H

//#pragma once

#include "RTypes.h"

#define RTOK_NAME			"name"

#define RTOK_MATERIALLIST	"MATERIALLIST"
#define RTOK_MATERIAL		"MATERIAL"
#define RTOK_AMBIENT		"AMBIENT"
#define RTOK_DIFFUSE		"DIFFUSE"
#define RTOK_SPECULAR		"SPECULAR"
#define RTOK_POWER			"POWER"
#define RTOK_DIFFUSEMAP		"DIFFUSEMAP"
#define RTOK_ADDITIVE		"ADDITIVE"
#define RTOK_TWOSIDED		"TWOSIDED"
#define RTOK_USEOPACITY		"USEOPACITY"
#define RTOK_USEALPHATEST	"USEALPHATEST"

#define RTOK_LIGHTLIST		"LIGHTLIST"
#define RTOK_LIGHT			"LIGHT"
#define RTOK_POSITION		"POSITION"
#define RTOK_DIRECTION		"DIRECTION"
#define RTOK_COLOR			"COLOR"
#define RTOK_INTENSITY		"INTENSITY"
#define RTOK_ATTNSTART		"ATTENUATIONSTART"
#define RTOK_ATTNEND		"ATTENUATIONEND"
#define RTOK_CASTSHADOW		"CASTSHADOW"
#define RTOK_RECEIVESHADOW	"RECEIVESHADOW"

#define RTOK_OBJECTLIST		"OBJECTLIST"
#define RTOK_OBJECT			"OBJECT"

#define RTOK_SPAWNPOSITIONLIST	"SPAWNPOSITIONLIST"
#define RTOK_SPAWNPOSITION		"SPAWNPOSITION"

#define RTOK_OCCLUSIONLIST	"OCCLUSIONLIST"
#define RTOK_OCCLUSION		"OCCLUSION"

#define RTOK_DUMMYLIST		"DUMMYLIST"
#define RTOK_DUMMY			"DUMMY"

#define RTOK_FLAG					"FLAG"
#define RTOK_FLAG_NAME				"NAME"
#define RTOK_FLAG_DIRECTION			"DIRECTION"		
#define RTOK_FLAG_POWER				"POWER"
#define RTOK_RESTRICTION			"RESTRICTION"
#define RTOK_RESTRICTION_AXIS		"AXIS"
#define RTOK_RESTRICTION_POSITION	"POSITION"
#define RTOK_RESTRICTION_COMPARE	"COMPARE"
#define RTOK_WINDTYPE				"TYPE"
#define RTOK_WINDDELAY				"DELAY"

#define RTOK_SMOKE					"SMOKE"
#define RTOK_SMOKE_NAME				"NAME"
#define RTOK_SMOKE_DIRECTION		"DIRECTION"
#define RTOK_SMOKE_POWER			"POWER"
#define RTOK_SMOKE_DELAY			"DELAY"
#define RTOK_SMOKE_SIZE				"SIZE"
#define RTOK_SMOKE_COLOR			"COLOR"
#define RTOK_SMOKE_LIFE				"LIFE"
#define RTOK_SMOKE_TOGMINTIME		"TOGGLEMINTIME"

#define RTOK_FOG		"FOG"
#define RTOK_GLOBAL		"GLOBAL"

#define FORMAT_FLOAT	"%.7f"

char *Format(char *buffer,rvector &v);
char *Format(char *buffer,float f);
char *Format(char *buffer,DWORD dw);


//////////////// 맵 디자인 규칙

#define RTOK_MAX_OBJLIGHT		"obj_"			// object 에만 영향을 주는 광원의 이름은 이렇게 시작한다.
#define RTOK_MAX_NOPATH			"nopath_"		// 갈수없는 곳은 이렇게 시작한다.
#define RTOK_MAX_EXPORTOBJ		"obj_"			// elu 로 export 하는 오브젝트들은 이렇게 이름이 시작한다.. (애니메이션이 있으면 자동)
#define RTOK_MAX_SPAWNPOINT		"spawn_"		// spawn position 임을 나타낸다. ( dummy object 로 놓아야 한다 )
#define RTOK_MAX_PASSTHROUGH	"pass_"			// 충돌체크를 통과한다.
#define RTOK_MAX_HIDE			"hide_"			// 이런 이름을 가진 오브젝트는 렌더링되지 않는다.
#define RTOK_MAX_OCCLUSION		"wall_"			// 벽.. (culling을 위한 추가정보)
#define RTOK_MAX_PASSBULLET		"passb_"		// 총알통과
#define RTOK_MAX_PASSROCKET		"passr_"		// 로켓및 수류탄 통과
#define RTOK_MAX_OCCLUSION		"wall_"			// 벽.. (culling을 위한 추가정보)
#define RTOK_MAX_PARTITION		"partition_"	// 공간 분할을 위한 평면 partition_0 partition_1 ... 이런 식으로 쓴다
#define RTOK_MAX_NAVIGATION		"hnav_"			// 인공지능 길찾기 정보 메쉬 - hide와 마찬가지로 렌더링되지 않는다.
#define RTOK_MAX_SMOKE_SS		"smk_ss_"		// 굴뚝의 연기 더미..

#endif