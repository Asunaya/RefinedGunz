#pragma once

#include "RNameSpace.h"
#include "EluLoader.h"

_NAMESPACE_REALSPACE2_BEGIN

class RBspObject;
struct MaterialBatch;

class RBspObjectDrawD3D9
{
public:
	RBspObjectDrawD3D9(RBspObject& bsp);
	RBspObjectDrawD3D9(RBspObjectDrawD3D9&&);
	~RBspObjectDrawD3D9();

	void OnInvalidate();
	void OnRestore();

	void Create(rsx::LoaderState&&);
	void Draw();

	bool ShowRTsEnabled{};

	void SetLighting(bool);

private:
	struct SimpleVertex
	{
		v3 Pos;
		v2 Tex;
	};

	void CreateTextures();
	void CreateShaderStuff();
	void CreateBatches();
	bool CreateBuffers();

	void CreateRTs();
	void ReleaseRTs();

	void SetPrologueStates();
	void SetEpilogueStates();
	void DrawLighting();
	void DepthCopy();
	void ShowRTs();

	LPDIRECT3DTEXTURE9 GetTexture(int Index) const;

	u32 GetFVF() const;
	bool UsingLighting() const;

	RBspObject& bsp;
	rsx::LoaderState State;
	struct Material
	{
		u16 Diffuse = -1;
		u16 Normal = -1;
		u16 Specular = -1;
		u16 Opacity = -1;
		u16 Emissive = -1;
		u8 AlphaTestValue = -1;
		float Shininess = 0;
		bool TwoSided{};

		bool HasDiffuse() const { return Diffuse != static_cast<decltype(Diffuse)>(-1); }
		bool HasNormal() const { return Normal != static_cast<decltype(Normal)>(-1); }
		bool HasSpecular() const { return Specular != static_cast<decltype(Specular)>(-1); }
		bool HasOpacity() const { return Opacity != static_cast<decltype(Opacity)>(-1); }
		bool HasEmissive() const { return Emissive != static_cast<decltype(Emissive)>(-1); }
		bool HasAlphaTest() const { return AlphaTestValue != static_cast<decltype(AlphaTestValue)>(-1); }
	};
	// Indices map to State.Materials. Contains indices into TextureMemory.
	std::vector<Material> Materials;
	std::vector<RBaseTexturePtr> TextureMemory;

	enum MaterialType
	{
		Normal,
		Opacity,
		AlphaTest,
	};
	template <MaterialType Type>
	void SetMaterial(Material& Mat);
	template <MaterialType Type>
	void SetPrerenderStates();
	template <MaterialType Type>
	void RenderMaterials(int StartIndex, int EndIndex);

	struct {
		// If SupportsDynamicLighting is true, each of these contain a vertex buffer with their component.
		// Otherwise, Positions is the only valid pointer, and it holds an interleaved stream of
		// position and texcoord components.
		D3DPtr<IDirect3DVertexBuffer9> Positions, TexCoords, Normals, Tangents;
	} VBs;

	D3DPtr<IDirect3DIndexBuffer9> IndexBuffer;

	D3DPtr<IDirect3DVertexDeclaration9> LitVertexDecl;
	D3DPtr<IDirect3DVertexDeclaration9> UnlitVertexDecl;

	D3DPtr<IDirect3DVertexShader9> DeferredVS;
	D3DPtr<IDirect3DPixelShader9> DeferredPS;
	D3DPtr<IDirect3DVertexShader9> PointLightVS;
	D3DPtr<IDirect3DPixelShader9> PointLightPS;
	D3DPtr<IDirect3DVertexShader9> DepthCopyVS;
	D3DPtr<IDirect3DPixelShader9> DepthCopyPS;
	D3DPtr<IDirect3DPixelShader9> VisualizeLinearDepthPS;

	enum class RTType
	{
		Color,
		Normal,
		LinearDepth,
		Ambient,
		Max,
	};
	D3DPtr<IDirect3DTexture9> RTs[static_cast<size_t>(RTType::Max)];
	auto& GetRT(RTType Index) const { return RTs[static_cast<size_t>(Index)]; }
	D3DPtr<IDirect3DSurface9> DummyDepthSurface;

	D3DPtr<IDirect3DSurface9> OrigRT;
	D3DPtr<IDirect3DSurface9> OrigDepthSurface;

	// Indices map to State.Materials.
	std::vector<MaterialBatch> MaterialBatches;
	int NormalMaterialsEnd{};
	int OpacityMaterialsEnd{};
	int AlphaTestMaterialsEnd{};

	bool SupportsDynamicLighting{};

	IDirect3DDevice9* dev{};
};

_NAMESPACE_REALSPACE2_END