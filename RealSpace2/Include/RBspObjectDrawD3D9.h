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

	void Create(rsx::LoaderState&&);
	void Draw();

private:
	struct SimpleVertex
	{
		v3 Pos;
		v2 Tex;
	};

	struct LitVertex
	{
		v3 Pos;
		v3 Nor;
		v2 Tex;
		v4 Tan;
	};

	void CreateTextures();
	void CreateShaderStuff();
	void CreateBatches();

	void SetPrologueStates();
	void SetEpilogueStates();

	LPDIRECT3DTEXTURE9 GetTexture(int Index);

	u32 GetFVF() const;
	size_t GetStride() const;

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

	D3DPtr<IDirect3DVertexBuffer9> VertexBuffer;
	D3DPtr<IDirect3DIndexBuffer9> IndexBuffer;

	D3DPtr<IDirect3DVertexDeclaration9> LitVertexDecl;

	D3DPtr<IDirect3DVertexShader9> DeferredVS;
	D3DPtr<IDirect3DPixelShader9> DeferredPS;

	// Indices map to State.Materials.
	std::vector<MaterialBatch> MaterialBatches;
	int NormalMaterialsEnd{};
	int OpacityMaterialsEnd{};
	int AlphaTestMaterialsEnd{};

	IDirect3DDevice9* dev{};
};

_NAMESPACE_REALSPACE2_END