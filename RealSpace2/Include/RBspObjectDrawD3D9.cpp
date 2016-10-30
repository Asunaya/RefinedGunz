#include "stdafx.h"
#include "RBspObjectDrawD3D9.h"
#include "RealSpace2.h"
#include <algorithm>
#include <numeric>

using namespace rsx;
using namespace RealSpace2;
using IndexType = u32;

template <typename VertexType, typename IndexType>
static bool CreateBuffers(size_t VertexCount, size_t IndexCount, u32 FVF,
	std::pair<D3DPtr<IDirect3DVertexBuffer9>, D3DPtr<IDirect3DIndexBuffer9>>& p)
{
	auto hr = RGetDevice()->CreateVertexBuffer(VertexCount * sizeof(VertexType), 0,
		FVF, D3DPOOL_MANAGED,
		MakeWriteProxy(p.first), nullptr);
	if (FAILED(hr))
	{
		MLog("RBspObjectDrawD3D9::Create -- Failed to create vertex buffer\n");
		return false;
	}

	hr = RGetDevice()->CreateIndexBuffer(IndexCount * sizeof(IndexType), 0,
		D3DFMT_INDEX16, D3DPOOL_MANAGED,
		MakeWriteProxy(p.second), nullptr);
	if (FAILED(hr))
	{
		MLog("RBspObjectDrawD3D9::Create -- Failed to create index buffer\n");
		return false;
	}

	return true;
}

void RBspObjectDrawD3D9::Create(rsx::LoaderState && srcState)
{
	State = std::move(srcState);

	auto GetMeshCount = [&](auto& data, auto Member) {
		return std::accumulate(data.Meshes.begin(), data.Meshes.end(), 0,
			[&](auto counter, auto& mesh) {
				return counter + mesh.*Member; });
	};

	auto GetCount = [&](auto Member) {
		return std::accumulate(State.ObjectData.begin(), State.ObjectData.end(), 0,
			[&](auto counter, auto& data) {
				return counter + GetMeshCount(data, Member); });
	};

	int TotalVertexCount = GetCount(&EluMesh::VertexCount);
	int TotalIndexCount = GetCount(&EluMesh::IndexCount);

	auto hr = RGetDevice()->CreateVertexBuffer(TotalVertexCount * sizeof(Vertex), 0,
		GetFVF(), D3DPOOL_MANAGED,
		MakeWriteProxy(VertexBuffer), nullptr);
	if (FAILED(hr))
	{
		MLog("RBspObjectDrawD3D9::Create -- Failed to create vertex buffer\n");
		return;
	}

	hr = RGetDevice()->CreateIndexBuffer(TotalIndexCount * sizeof(IndexType), 0,
		D3DFMT_INDEX16, D3DPOOL_MANAGED,
		MakeWriteProxy(IndexBuffer), nullptr);
	if (FAILED(hr))
	{
		MLog("RBspObjectDrawD3D9::Create -- Failed to create index buffer\n");
		return;
	}

	std::unordered_map<std::string, int> TexMap;

	Textures.resize(State.Materials.size());
	for (size_t i{}; i < State.Materials.size(); ++i)
	{
		auto& Mat = State.Materials[i];

		auto LoadTexture = [&](auto& name) -> int
		{
			if (name.empty())
				return -1;

			{
				auto it = TexMap.find(name);
				if (it != TexMap.end())
					return it->second;
			}

			TextureMemory.emplace_back();
			auto hr = D3DXCreateTextureFromFile(RGetDevice(), name.c_str(),
				MakeWriteProxy(TextureMemory.back()));
			if (FAILED(hr))
			{
				DMLog("Failed to load texture %s\n", name);
				TextureMemory.pop_back();
				return -1;
			}
			auto idx = TextureMemory.size() - 1;
			TexMap.emplace(std::make_pair(name, idx));
			return idx;
		};

		auto& Tex = Textures[i];
		Tex.Diffuse = LoadTexture(Mat.tDiffuse);
		Tex.Opacity = LoadTexture(Mat.tOpacity);
	}

	for (size_t i{}; i < State.ObjectData.size(); ++i)
	{
		auto& Data = State.ObjectData[i];
		Buffers.emplace_back();
		auto& buf = Buffers.back();
		auto TotesVertexCount = GetMeshCount(Data, &EluMesh::VertexCount);
		auto TotesIndexCount = GetMeshCount(Data, &EluMesh::IndexCount);
		DMLog("mesh %d, %s: %d, %d\n", i, Data.Name.c_str(), TotesVertexCount, TotesIndexCount);
		if (!CreateBuffers<Vertex, IndexType>(
			TotesVertexCount,
			TotesIndexCount,
			GetFVF(),
			buf))
		{
			MLog("Oh no!\n");
			return;
		}

		char* Verts{};
		if (FAILED(buf.first->Lock(0, 0, reinterpret_cast<void**>(&Verts), 0)))
		{
			MLog("Oh no!\n");
			return;
		}
		u16* Indices{};
		if (FAILED(buf.second->Lock(0, 0, reinterpret_cast<void**>(&Indices), 0)))
		{
			MLog("Oh no!\n");
			return;
		}

		for (auto& Mesh : Data.Meshes)
		{
			for (size_t j{}; j < Mesh.VertexCount; ++j)
			{
				auto AddVertexData = [&](auto& val) {
					memcpy(Verts, &val, sizeof(val));
					Verts += sizeof(val);
				};
				AddVertexData(Mesh.Positions[j]);
				AddVertexData(Mesh.TexCoords[j]);
			}
			memcpy(Indices, Mesh.Indices.get(), Mesh.IndexCount * sizeof(Mesh.Indices[0]));
			Indices += Mesh.IndexCount;
		}
		buf.first->Unlock();
		buf.second->Unlock();
	}
	
	DMLog("RBspObjectDrawD3D9 created\n");
}

static void SetWireframeStates()
{
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	RGetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
}

static void SetDefaultStates()
{
	bool Trilinear = RIsTrilinear();

	RGetDevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, Trilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MIPFILTER, Trilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);

	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

	RGetDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

void RBspObjectDrawD3D9::Draw()
{
	auto dev = RGetDevice();

	if (true)
		SetDefaultStates();
	else
		SetWireframeStates();

	// Enable Z-buffer testing and writing
	RSetWBuffer(true);
	dev->SetRenderState(D3DRS_ZWRITEENABLE, true);
	dev->SetRenderState(D3DRS_LIGHTING, FALSE);
	dev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	RGetDevice()->SetFVF(GetFVF());
	RGetDevice()->SetStreamSource(0, VertexBuffer.get(), 0, GetStride());
	RGetDevice()->SetIndices(IndexBuffer.get());

	//int i{};
	for (size_t i{}; i < State.Objects.size(); ++i)
	{
		auto& Object = State.Objects[i];
		int vb{}, ib{};
		auto& buf = Buffers[Object.Data];
		auto& Data = State.ObjectData[Object.Data];
		RGetDevice()->SetStreamSource(0, buf.first.get(), 0, GetStride());
		RGetDevice()->SetIndices(buf.second.get());
		//DMLog("%d -> %d\n", i, Object.Data); ++i;
		for (size_t MeshIndex{}; MeshIndex < Data.Meshes.size(); ++MeshIndex)
		{
			auto& Mesh = Data.Meshes[MeshIndex];
			auto World = Mesh.World * Object.World;
			RGetDevice()->SetTransform(D3DTS_WORLD, &World);

			for (auto& dp : Mesh.DrawProps)
			{
				auto& Tex = Textures[Data.MaterialStart + dp.material];

				if (Tex.Diffuse == -1)
					continue;

				if (Tex.Opacity != -1)
				{
					//dev->SetRenderState(D3DRS_ZWRITEENABLE, false);
					dev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
					dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
					dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
					dev->SetTexture(1, TextureMemory[Tex.Opacity].get());
					dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
					dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
					dev->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
					dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
				}
				else
				{
					dev->SetRenderState(D3DRS_ZWRITEENABLE, true);
					dev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
					dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
					dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
					RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
				}

				RGetDevice()->SetTexture(0, TextureMemory[Tex.Diffuse].get());

				RGetDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
					dp.vertexBase + vb, 0, Mesh.VertexCount, dp.indexBase + ib, dp.count);
			}

			vb += Mesh.VertexCount;
			ib += Mesh.IndexCount;
		}
	}

	// Disable alpha testing
	dev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);
	dev->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	// Disable alpha blending
	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	dev->SetRenderState(D3DRS_ZWRITEENABLE, true);

	// Reset all textures and texture states
	dev->SetTexture(0, nullptr);
	dev->SetTexture(1, nullptr);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
}