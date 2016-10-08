// types for realspace 2 . 2001-10-4 created.
#pragma once

#include <string>
#include <list>
#include "d3dx9math.h"
#include "GlobalTypes.h"
#include "RNameSpace.h"

using rvector = D3DXVECTOR3;
using rmatrix = D3DXMATRIX;
using rplane = D3DXPLANE;
using rvector2 = D3DXVECTOR2;
using rquaternion = D3DXQUATERNION;
using RPIXELFORMAT = D3DFORMAT;
using rmatrixa16 = D3DXMATRIXA16;

_NAMESPACE_REALSPACE2_BEGIN

// awkward, remove sometime
#ifndef pi
#define pi D3DX_PI
#endif

enum rsign { NEGATIVE = -1, ZERO = 0, POSITIVE = 1 };

enum RRESULT {
	R_UNKNOWN = -1,
	R_OK = 0,
	R_NOTREADY = 1,
	R_RESTORED = 2,

	R_ERROR_LOADING = 1000,
};

enum class FullscreenType
{
	Fullscreen,
	Borderless,
	Windowed,
};

struct RMODEPARAMS {
	int nWidth, nHeight;
	FullscreenType FullscreenMode;
	RPIXELFORMAT PixelFormat;
};

#define RM_FLAG_ADDITIVE		0x0001
#define RM_FLAG_USEOPACITY		0x0002
#define RM_FLAG_TWOSIDED		0x0004
#define RM_FLAG_NOTWALKABLE		0x0008
#define RM_FLAG_CASTSHADOW		0x0010
#define RM_FLAG_RECEIVESHADOW	0x0020
#define RM_FLAG_PASSTHROUGH		0x0040
#define RM_FLAG_HIDE			0x0080
#define RM_FLAG_PASSBULLET		0x0100
#define RM_FLAG_PASSROCKET		0x0200
#define RM_FLAG_USEALPHATEST	0x0400
#define RM_FLAG_AI_NAVIGATION	0x1000

struct rboundingbox
{
	rboundingbox() {}

	union {
	struct {
		float minx,miny,minz,maxx,maxy,maxz;
	};
	struct {
		rvector vmin,vmax;
	};
	float m[2][3];
	};
};

using RFPROGRESSCALLBACK = void(*)(void *pUserParams,float fProgress);

_NAMESPACE_REALSPACE2_END
