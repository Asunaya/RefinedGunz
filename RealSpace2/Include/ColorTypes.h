#pragma once

#include "GlobalTypes.h"

typedef struct _D3DCOLORVALUE D3DCOLORVALUE;

struct color_r32
{
	float r, g, b, a;

	color_r32() = default;
	color_r32(u32 argb) :
		a{ ((argb & 0xFF000000) >> 24) * (1.0f / 255.0f) },
		r{ ((argb & 0xFF0000) >> 16) * (1.0f / 255.0f) },
		g{ ((argb & 0xFF00) >> 8) * (1.0f / 255.0f) },
		b{ (argb & 0xFF) * (1.0f / 255.0f) }
	{}
	color_r32(const float(&arr)[4]) :
		a{ arr[0] },
		r{ arr[1] },
		g{ arr[2] },
		b{ arr[3] }
	{}

	operator D3DCOLORVALUE() const;
};