#pragma once

#include "RNameSpace.h"
#include "EluLoader.h"

_NAMESPACE_REALSPACE2_BEGIN

class RBspObject;

struct MaterialBatch
{
	int TriangleCount;
	int VertexBase;
	int IndexBase;
};

class RBspObjectDrawD3D9
{
public:
	RBspObjectDrawD3D9(RBspObject& bsp) : bsp(bsp) {}
	RBspObjectDrawD3D9(RBspObjectDrawD3D9&&) = default;

	void Create(rsx::LoaderState&&);
	void Draw();

private:
	struct Vertex
	{
		v3 Pos;
		v2 Tex;
	};

	void CreateTextures();
	void CreateBatches();

	u32 GetFVF() const { return D3DFVF_XYZ | D3DFVF_TEX1; }
	size_t GetStride() const { return sizeof(Vertex); }

	RBspObject& bsp;
	rsx::LoaderState State;
	struct TextureData
	{
		int Diffuse = -1;
		int Opacity = -1;
		int AlphaTestValue = -1;
	};
	// Indices map to State.Materials. Contains indices into TextureMemory.
	std::vector<TextureData> Textures;
	std::vector<D3DPtr<IDirect3DTexture9>> TextureMemory;

	D3DPtr<IDirect3DVertexBuffer9> VertexBuffer;
	D3DPtr<IDirect3DIndexBuffer9> IndexBuffer;

	// Indices map to State.Materials.
	std::vector<MaterialBatch> MaterialBatches;
	int NumNormalMaterials{};
	int NumOpacityMaterials{};
};

_NAMESPACE_REALSPACE2_END