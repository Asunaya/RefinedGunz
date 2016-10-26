#pragma once

#include "RNameSpace.h"
#include "EluLoader.h"

_NAMESPACE_REALSPACE2_BEGIN

class RBspObject;

struct MaterialBatch
{
	int MaterialIndex;
	int NumTriangles;
	int IndicesOffset;
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

	u32 GetFVF() const { return D3DFVF_XYZ | D3DFVF_TEX1; }
	size_t GetStride() const { return sizeof(Vertex); }

	RBspObject& bsp;
	rsx::LoaderState State;
	D3DPtr<IDirect3DVertexBuffer9> VertexBuffer;
	D3DPtr<IDirect3DIndexBuffer9> IndexBuffer;
	std::vector<D3DPtr<IDirect3DTexture9>> DiffuseTextures;
	std::vector<std::pair<D3DPtr<IDirect3DVertexBuffer9>, D3DPtr<IDirect3DIndexBuffer9>>> Buffers;
};

_NAMESPACE_REALSPACE2_END