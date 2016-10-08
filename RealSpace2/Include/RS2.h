#pragma once

#include "Renderer.h"

_NAMESPACE_REALSPACE2_BEGIN

class RS2
{
public:
	RS2();
	RS2(const RS2&) = delete;

	void OnInvalidate();
	void OnRestore();

	static RS2& Get();

	LPDIRECT3DDEVICE9 Direct3DDevice9;
	Renderer Render;
};

inline RS2& GetRS2() { return RS2::Get(); }
inline Renderer& GetRenderer() { return GetRS2().Render; }

_NAMESPACE_REALSPACE2_END
