#include "stdafx.h"
#include "ColorTypes.h"
#include <D2DBaseTypes.h>

color_r32::operator D3DCOLORVALUE() const
{
	return{ r, g, b, a };
}