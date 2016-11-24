#pragma once

#include "RealSpace2.h"

_NAMESPACE_REALSPACE2_BEGIN

//#define SHADOW_TEST

enum class TransformType
{
	World,
	View,
	Projection,
	End,
};

class Renderer
{
public:
	Renderer();

	void OnInvalidate();
	void OnRestore();

	void SetNearZ(float f) { NearZ = f; }
	void SetFarZ(float f) { FarZ = f; }

	float GetNearZ() const { return NearZ; }
	float GetFarZ() const { return FarZ; }

	void SetAlphaBlending(bool b);
	auto IsAlphaBlendingEnabled() const { return AlphaBlending; }

	bool SetDrawShadows(bool b);
	auto IsDrawingShadows() const { return DrawingShadows; }

	void SetDynamicLights(bool b);
	bool GetDynamicLights() const { return DynamicLights; }

	void SetTransform(TransformType t, const rmatrix& mat) { GetTransform(t) = mat; }
	void CommitChanges();

	void SetPrerenderStates();

	template <typename T>
	bool DrawShadows(T& func);

	void ApplyLighting();

	void SetScreenSpaceVertexDeclaration();

private:
	void CreateTextures();
	void CreateShaders();

	rmatrix& GetTransform(TransformType t) { return Transforms[static_cast<size_t>(t)]; }

	template <typename T>
	bool DrawShadowMap(T& func);
	template <typename T>
	bool DrawDiffuseMap(T& func);

	bool AlphaBlending{};	
	bool DrawingShadows{};
	bool CanRenderWithShader{};
	bool DynamicLights{};

	float NearZ{}, FarZ{};

	rmatrix Transforms[static_cast<size_t>(TransformType::End)];
	v3 LightPos;
	v3 LightDir;

	D3DPtr<IDirect3DVertexDeclaration9> VertexDeclaration;
	D3DPtr<IDirect3DVertexDeclaration9> ScreenSpaceVertexDeclaration;

#ifdef SHADOW_TEST
	D3DPtr<IDirect3DVertexShader9> SceneVS;
	D3DPtr<IDirect3DPixelShader9> SceneDirLightPS;
	D3DPtr<IDirect3DPixelShader9> ScenePointLightPS;
	D3DPtr<IDirect3DVertexShader9> ShadowVS;
	D3DPtr<IDirect3DPixelShader9> ShadowPS;
	D3DPtr<IDirect3DPixelShader9> ShadowAlphaPS;
	D3DPtr<IDirect3DVertexShader9> MergeShadowMapsVS;
	D3DPtr<IDirect3DPixelShader9> MergeShadowMapsPS;

	D3DPtr<IDirect3DTexture9> ShadowTexture;
	D3DPtr<IDirect3DCubeTexture9> ShadowCubeTexture;
	D3DPtr<IDirect3DSurface9> ShadowDepthSurface;
	D3DPtr<IDirect3DTexture9> DiffuseMap;

	rmatrix LightProjection;

	int ShadowMapSize = 1024;
#endif
};

_NAMESPACE_REALSPACE2_END