#pragma once

#include "RNameSpace.h"

_NAMESPACE_REALSPACE2_BEGIN

IDirect3DVertexShader9* CreateVertexShader(const BYTE* Function);
IDirect3DPixelShader9* CreatePixelShader(const BYTE* Function);

IDirect3DVertexShader9* CreateVertexShaderFromFile(const char* Filename);
IDirect3DPixelShader9* CreatePixelShaderFromFile(const char* Filename);

_NAMESPACE_REALSPACE2_END
