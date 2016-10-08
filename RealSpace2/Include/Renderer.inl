#pragma once

#include "Renderer.h"
#include "RMath.h"
#include "ShaderGlobals.h"
#include "defer.h"

#define V(expr) if (FAILED(expr)) return false

#ifdef SHADOW_TEST
template <typename T>
bool Renderer::DrawShadowMap(T& func)
{
	D3DPtr<IDirect3DSurface9> OldZSurface;
	V(RGetDevice()->GetDepthStencilSurface(MakeWriteProxy(OldZSurface)));
	V(RGetDevice()->SetDepthStencilSurface(ShadowDepthSurface));
	DEFER(RGetDevice()->SetDepthStencilSurface(OldZSurface););

	SetTransform(TransformType::Projection, LightProjection);
	CommitChanges();

	RGetDevice()->SetVertexShader(ShadowVS);
	RGetDevice()->SetPixelShader(AlphaBlending ? ShadowAlphaPS : ShadowPS);

	const v3 Dirs[6][2] = {
		{ { 1, 0, 0 },{ 0, 1, 0 } },
		{ { -1, 0, 0 },{ 0, 1, 0 } },
		{ { 0, 1, 0 },{ 0, 0, -1 } },
		{ { 0, -1, 0 },{ 0, 0, 1 } },
		{ { 0, 0, 1 },{ 0, 1, 0 } },
		{ { 0, 0, -1 },{ 0, 1, 0 } },
	};

	for (int i{}; i < 6; ++i)
	{
		LightDir = { 0, 0, 0 };
		LightDir[i / 2] = i % 2 ? 1 : -1;
		LightDir = Dirs[i][0];
		auto LightView = ViewMatrix(LightPos, LightDir, Dirs[i][1]);
		SetTransform(TransformType::View, LightView);
		CommitChanges();

		D3DPtr<IDirect3DSurface9> ShadowRTSurface;
		V(ShadowCubeTexture->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(i), 0,
			MakeWriteProxy(ShadowRTSurface)));
		V(RGetDevice()->SetRenderTarget(0, ShadowRTSurface));

		V(RGetDevice()->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1, 0));

		func();
	}

	SetTransform(TransformType::View, RView);
	SetTransform(TransformType::Projection, RProjection);
	CommitChanges();

	return true;
}

template <typename T>
bool Renderer::DrawDiffuseMap(T& func)
{
	D3DPtr<IDirect3DSurface9> DiffuseSurface;
	V(DiffuseMap->GetSurfaceLevel(0, MakeWriteProxy(DiffuseSurface)));
	V(RGetDevice()->SetRenderTarget(0, DiffuseSurface));
	V(RGetDevice()->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1, 0));

	V(RGetDevice()->SetTexture(ShaderSampler::Shadow0 + 2, ShadowCubeTexture.get()));

	RGetDevice()->SetVertexShader(MergeShadowMapsVS);
	RGetDevice()->SetPixelShader(MergeShadowMapsPS);

	func();

	V(RGetDevice()->SetTexture(ShaderSampler::Shadow0 + 2, nullptr));

	RGetDevice()->SetVertexShader(nullptr);
	RGetDevice()->SetPixelShader(nullptr);

	V(RGetDevice()->Clear(0, nullptr, D3DCLEAR_ZBUFFER, 0x00000000, 1, 0));

	return true;
}
#endif

template <typename T>
bool Renderer::DrawShadows(T& func)
{
#ifdef SHADOW_TEST
	D3DPtr<IDirect3DSurface9> OldRT;
	V(RGetDevice()->GetRenderTarget(0, MakeWriteProxy(OldRT)));
	DEFER(RGetDevice()->SetRenderTarget(0, OldRT););

	SetDrawShadows(true);
	if (!DrawShadowMap(func)) return false;
	if (!DrawDiffuseMap(func)) return false;
	SetDrawShadows(false);
	
	return true;
#else
	return true;
#endif
}

#undef V
