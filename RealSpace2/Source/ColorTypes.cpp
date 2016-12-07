#include "stdafx.h"
#include "ColorTypes.h"
#include <D2DBaseTypes.h>

color_r32::operator D3DCOLORVALUE() const
{
	return{ r, g, b, a };
}

color_r32::operator u32() const
{
	return ARGB(a * 255, r * 255, g * 255, b * 255);
}
