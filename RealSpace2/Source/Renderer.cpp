#include "stdafx.h"
#include "Renderer.h"
#include "ShaderGlobals.h"
#include <fstream>
#include "ShaderUtil.h"

// Shader object files
#include "ShadowVS.h"
#include "ShadowPS.h"
#include "MergeShadowMapsVS.h"
#include "MergeShadowMapsPS.h"

constexpr float LightFov = TAU / 4;
constexpr float LightTheta = LightFov / 2;
const float LightCosTheta = cos(LightTheta);
constexpr float AttenuationStart = 300;
constexpr float AttenuationEnd = 2000;
const v4 AttenuationValues{ Square(AttenuationStart), Square(AttenuationEnd), Square(AttenuationEnd) - Square(AttenuationStart), 0 };

static auto* CreateVertexDeclaration(bool Normal)
{
	IDirect3DVertexDeclaration9* Decl;

	D3DVERTEXELEMENT9 Elements[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, sizeof(float) * 3, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, sizeof(float) * 5, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		D3DDECL_END()
	};

	D3DVERTEXELEMENT9 ElementsNormal[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, sizeof(float) * 3, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, sizeof(float) * 6, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, sizeof(float) * 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		D3DDECL_END()
	};

	RGetDevice()->CreateVertexDeclaration(Normal ? ElementsNormal : Elements, &Decl);

	return Decl;
}

Renderer::Renderer()
	: LightPos{ 0, 0, 100 }
{
	SetTransform(TransformType::World, GetIdentityMatrix());

#ifdef SHADOW_TEST
	CreateShaders();
	CreateTextures();

	void* Pointers[] = { 
		VertexDeclaration,
		ShadowVS, ShadowPS,
		ShadowTexture, ShadowDepthSurface,
		MergeShadowMapsVS, MergeShadowMapsPS};
	for (auto* Pointer : Pointers)
	{
		if (!Pointer)
		{
			MLog("Renderer::Renderer -- Something failed, can't render with shader\n");
			assert(false);
			return;
		}
	}

	CanRenderWithShader = true;
#endif
}

void Renderer::CreateTextures()
{
#ifdef SHADOW_TEST
	RGetDevice()->CreateCubeTexture(
		ShadowMapSize,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_R32F,
		D3DPOOL_DEFAULT,
		MakeWriteProxy(ShadowCubeTexture),
		nullptr);

	RGetDevice()->CreateTexture(
		ShadowMapSize, ShadowMapSize,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_R32F,
		D3DPOOL_DEFAULT,
		MakeWriteProxy(ShadowTexture),
		nullptr);

	RGetDevice()->CreateDepthStencilSurface(
		ShadowMapSize, ShadowMapSize,
		GetDepthStencilFormat(),
		D3DMULTISAMPLE_NONE,
		0,
		TRUE,
		MakeWriteProxy(ShadowDepthSurface),
		nullptr);

	RGetDevice()->CreateTexture(
		RGetScreenWidth(), RGetScreenHeight(),
		0, D3DUSAGE_RENDERTARGET,
		D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT,
		MakeWriteProxy(DiffuseMap),
		nullptr);
#endif
}

void Renderer::CreateShaders()
{
#ifdef SHADOW_TEST
	VertexDeclaration = CreateVertexDeclaration(true);
	ShadowVS = CreateVertexShader(ShadowVSData);
	ShadowPS = CreatePixelShader(ShadowPSData);
	MergeShadowMapsVS = CreateVertexShader(MergeShadowMapsVSData);
	MergeShadowMapsPS = CreatePixelShader(MergeShadowMapsPSData);
#endif
}

void Renderer::OnInvalidate()
{
#ifdef SHADOW_TEST
	SAFE_RELEASE(ShadowTexture);
	SAFE_RELEASE(ShadowCubeTexture);
	SAFE_RELEASE(ShadowDepthSurface);
	SAFE_RELEASE(DiffuseMap);
#endif
}

void Renderer::OnRestore()
{
	CreateTextures();
}

void Renderer::SetAlphaBlending(bool b)
{
	if (AlphaBlending == b)
		return;

	AlphaBlending = b;

#ifdef SHADOW_TEST
	if (DrawingShadows)
	{
		if (!AlphaBlending)
		{
			RGetDevice()->SetVertexShader(ShadowVS);
			RGetDevice()->SetPixelShader(ShadowPS);
		}
		else
		{
			RGetDevice()->SetVertexShader(nullptr);
			RGetDevice()->SetPixelShader(nullptr);
		}
	}
#endif
}

bool Renderer::SetDrawShadows(bool b)
{
	if (DrawingShadows == b)
		return true;

	DrawingShadows = b;

/*#define V(expr) if (FAILED(expr)) return false
	
	auto* dev = RGetDevice();
	if (DrawingShadows)
	{
		V(dev->GetRenderTarget(0, MakeWriteProxy(OldRT)));
		V(dev->GetDepthStencilSurface(MakeWriteProxy(OldZSurface)));

		D3DPtr<IDirect3DSurface9> ShadowRTSurface;
		V(ShadowTexture->GetSurfaceLevel(0, MakeWriteProxy(ShadowRTSurface)));
		V(dev->SetRenderTarget(0, ShadowRTSurface));
		V(dev->SetDepthStencilSurface(ShadowDepthSurface));

		V(dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1, 0));

		SetTransform(TransformType::View, LightView);
		SetTransform(TransformType::Projection, LightProjection);
		CommitChanges();

		RGetDevice()->SetVertexShader(ShadowVS);
		RGetDevice()->SetPixelShader(AlphaBlending ? ShadowAlphaPS : ShadowPS);
	}
	else
	{
		V(dev->SetRenderTarget(0, OldRT));
		V(dev->SetDepthStencilSurface(OldZSurface));

		SetTransform(TransformType::View, RView);
		SetTransform(TransformType::Projection, RProjection);
		CommitChanges();
	}*/

	return true;
}

void Renderer::SetDynamicLights(bool b)
{
	DynamicLights = b;

	/*if (DynamicLights)
	{
		//RGetDevice()->SetTexture(static_cast<DWORD>(ShaderSampler::Shadow), ShadowCubeTexture.get());
		RGetDevice()->SetTexture(ShaderSampler::Diffuse, DiffuseMap.get());
		RGetDevice()->SetVertexShader(SceneVS);
		RGetDevice()->SetPixelShader(ScenePointLightPS);
	}
	else*/
	{
		//RGetDevice()->SetTexture(static_cast<DWORD>(ShaderSampler::Shadow), nullptr);
		RGetDevice()->SetTexture(ShaderSampler::Diffuse, nullptr);
		RGetDevice()->SetVertexShader(nullptr);
		RGetDevice()->SetPixelShader(nullptr);
	}
}

void Renderer::CommitChanges()
{
#ifdef SHADOW_TEST
	auto& World = GetTransform(TransformType::World);
	auto& View = GetTransform(TransformType::View);
	auto& Projection = GetTransform(TransformType::Projection);

	auto WorldView = World * View;
	auto trWorldViewProj = Transpose(WorldView * Projection);
	auto trWorldView = Transpose(WorldView);
	auto trProj = Transpose(Projection);

	SetVSMatrix(ShaderConstant::WorldViewProjection, trWorldViewProj);
	SetVSMatrix(ShaderConstant::WorldView, trWorldView);  SetPSMatrix(ShaderConstant::WorldView, trWorldView);
	SetVSMatrix(ShaderConstant::Projection, trProj);      SetPSMatrix(ShaderConstant::Projection, trProj);
	SetVSMatrix(ShaderConstant::World, Transpose(World));

	SetPSVector3(ShaderConstant::LightPos0, LightPos);
	SetPSVector3(ShaderConstant::LightDir0, LightDir);

	LightProjection = PerspectiveProjectionMatrix(1, LightFov, NearZ, FarZ);
#endif
}

void Renderer::SetPrerenderStates()
{
#ifdef SHADOW_TEST
	RGetDevice()->SetTexture(static_cast<DWORD>(ShaderSampler::Shadow0), nullptr);

	rmatrixa16 LightView = ViewMatrix(LightPos, LightDir, { 0, 0, 1 });
	rmatrixa16 LightViewProjection = Transpose(LightView * LightProjection);

	SetVSMatrix(ShaderConstant::LightViewProjection, LightViewProjection);
	SetPSFloat(ShaderConstant::CosTheta, LightCosTheta);
	SetPSVector4(ShaderConstant::LightDiffuse, v4{ 1, 1, 1, 1 });
	SetPSVector4(ShaderConstant::LightAmbient, v4{ 0.3f, 0.3f, 0.3f, 1 });
	SetPSFloat(ShaderConstant::ShadowMapSize, ShadowMapSize);
	SetPSVector4(ShaderConstant::AttenuationValues, AttenuationValues);
#endif
}

void Renderer::ApplyLighting()
{
#ifdef SHADOW_TEST
	RGetDevice()->SetVertexShader(nullptr);
	RGetDevice()->SetPixelShader(nullptr);
	RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	RGetDevice()->SetStreamSource(0, nullptr, 0, 0);
	RGetDevice()->SetIndices(nullptr);

	RGetDevice()->SetTexture(0, DiffuseMap.get());
	RGetDevice()->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, false);
	
	struct VertexType
	{
		float x, y, z, rhw;
		float u, v;
	};

	auto Width = (float)RGetScreenWidth();
	auto Height = (float)RGetScreenHeight();
	VertexType Vertices[] = {
		{0, 0, 0, 1, 0, 0},
		{ 0, Height, 0, 1, 0, 1 },
		{Width, Height, 0, 1, 1, 1},
		{Width, 0, 0, 1, 1, 0},
	};

	RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &Vertices, sizeof(VertexType));

	/*D3DPtr<IDirect3DSurface9> DiffuseSurface;
	DiffuseMap->GetSurfaceLevel(0, MakeWriteProxy(DiffuseSurface));
	D3DPtr<IDirect3DSurface9> RT;
	RGetDevice()->GetRenderTarget(0, MakeWriteProxy(RT));
	RECT r;
	r.left = 1080;
	r.right = 1920;
	r.top = 720;
	r.bottom = 1080;
	RGetDevice()->StretchRect(DiffuseSurface, nullptr, RT, &r, D3DTEXF_NONE);*/
#endif
}