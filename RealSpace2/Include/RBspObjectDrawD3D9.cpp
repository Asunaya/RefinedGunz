#include "stdafx.h"
#include "RBspObjectDrawD3D9.h"
#include "RealSpace2.h"
#include <algorithm>
#include <numeric>
#include "RBspObject.h"
#include "ShaderGlobals.h"
#include "ShaderUtil.h"
#include "RS2.h"
#include "Sphere.h"

// Shader objects
#include "DeferredVS.h"
#include "DeferredPS.h"
#include "PointLightVS.h"
#include "PointLightPS.h"
#include "DepthCopyVS.h"
#include "DepthCopyPS.h"
#include "VisualizeLinearDepth.h"

_NAMESPACE_REALSPACE2_BEGIN

struct MaterialBatch
{
	int TriangleCount;
	int VertexBase;
	int IndexBase;
};

using namespace rsx;
using IndexType = u32;

template <typename T> constexpr D3DFORMAT GetD3DFormat();
template <>	constexpr D3DFORMAT GetD3DFormat<u16>() { return D3DFMT_INDEX16; }
template <> constexpr D3DFORMAT GetD3DFormat<u32>() { return D3DFMT_INDEX32; }

template <typename VertexType, typename IndexType>
static bool CreateBuffers(size_t VertexCount, size_t IndexCount, u32 FVF,
	D3DPtr<IDirect3DVertexBuffer9>& vb, D3DPtr<IDirect3DIndexBuffer9>& ib)
{
	auto hr = RGetDevice()->CreateVertexBuffer(VertexCount * sizeof(VertexType), 0,
		FVF, D3DPOOL_MANAGED,
		MakeWriteProxy(vb), nullptr);
	if (FAILED(hr))
	{
		MLog("RBspObjectDrawD3D9::Create -- Failed to create vertex buffer\n");
		return false;
	}

	hr = RGetDevice()->CreateIndexBuffer(IndexCount * sizeof(IndexType), 0,
		GetD3DFormat<IndexType>(), D3DPOOL_MANAGED,
		MakeWriteProxy(ib), nullptr);
	if (FAILED(hr))
	{
		MLog("RBspObjectDrawD3D9::Create -- Failed to create index buffer\n");
		return false;
	}

	return true;
}

RBspObjectDrawD3D9::RBspObjectDrawD3D9(RBspObject& bsp) : bsp{ bsp }, dev{ RGetDevice() } {}
RBspObjectDrawD3D9::RBspObjectDrawD3D9(RBspObjectDrawD3D9&&) = default;
RBspObjectDrawD3D9::~RBspObjectDrawD3D9() = default;

void RBspObjectDrawD3D9::OnInvalidate()
{
	ReleaseRTs();
}

void RBspObjectDrawD3D9::OnRestore()
{
	CreateRTs();
}

void RBspObjectDrawD3D9::CreateTextures()
{
	std::unordered_map<std::string, int> TexMap;

	Materials.resize(State.Materials.size());
	for (size_t i{}; i < State.Materials.size(); ++i)
	{
		auto& EluMat = State.Materials[i];

		auto LoadTexture = [&TexMap, this](auto& name) -> int
		{
			if (name.empty())
				return -1;

			// Check if already loaded and use the same index if so.
			{
				auto it = TexMap.find(name);
				if (it != TexMap.end())
					return it->second;
			}

			auto* Tex = RCreateBaseTexture(name.c_str(), RTextureType_Map, true);
			if (!Tex)
			{
				MLog("RBspObjectDrawD3D9::CreateTextures -- Failed to load texture %s\n",
					name.c_str());
				return -1;
			}

			TextureMemory.emplace_back(Tex);

			auto idx = TextureMemory.size() - 1;
			TexMap.emplace(std::make_pair(name, idx));
			return idx;
		};

		auto& Mat = Materials[i];
		Mat.Diffuse = LoadTexture(EluMat.tDiffuse);
		Mat.Normal = LoadTexture(EluMat.tNormal);
		Mat.Specular = LoadTexture(EluMat.tSpecular);
		Mat.Opacity = LoadTexture(EluMat.tOpacity);
		Mat.Emissive = LoadTexture(EluMat.tEmissive);
		Mat.AlphaTestValue = EluMat.AlphaTestValue;
		Mat.TwoSided = EluMat.TwoSided;
	}
}

static D3DPtr<IDirect3DVertexDeclaration9> CreateLitVertexDecl()
{
	D3DVERTEXELEMENT9 Decl[] =
	{
		{ 0, 0,     D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 3 * 4, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
		{ 0, 6 * 4, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 8 * 4, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,  0 },
		D3DDECL_END()
	};

	D3DPtr<IDirect3DVertexDeclaration9> ptr;
	RGetDevice()->CreateVertexDeclaration(Decl, MakeWriteProxy(ptr));
	return ptr;
}

void RBspObjectDrawD3D9::CreateShaderStuff()
{
	SupportsDynamicLighting = SupportsVertexShaderVersion(3, 0) &&
		SupportsPixelShaderVersion(3, 0) &&
		GetDeviceCaps().NumSimultaneousRTs >= 4;
	if (!SupportsDynamicLighting)
		return;

	LitVertexDecl = CreateLitVertexDecl();
	DeferredVS = CreateVertexShader(DeferredVSData);
	DeferredPS = CreatePixelShader(DeferredPSData);
	PointLightVS = CreateVertexShader(PointLightVSData);
	PointLightPS = CreatePixelShader(PointLightPSData);
	DepthCopyVS = CreateVertexShader(DepthCopyVSData);
	DepthCopyPS = CreatePixelShader(DepthCopyPSData);
	VisualizeLinearDepthPS = CreatePixelShader(VisualizeLinearDepthData);

	CreateRTs();
}

void RBspObjectDrawD3D9::CreateBatches()
{
	struct DrawPropStuff
	{
		int Mesh;
		u32 IndexBase;
		u32 Count;
	};

	struct MaterialStuff
	{
		int Data;
		std::vector<DrawPropStuff> DrawProps;
	};

	std::vector<MaterialStuff> MaterialProps;
	MaterialProps.resize(State.Materials.size());

	// Sort draw props per material
	for (size_t DataIndex{}; DataIndex < State.ObjectData.size(); ++DataIndex)
	{
		auto& Data = State.ObjectData[DataIndex];
		for (size_t MeshIndex{}; MeshIndex < Data.Meshes.size(); ++MeshIndex)
		{
			auto& Mesh = Data.Meshes[MeshIndex];
			for (auto& dp : Mesh.DrawProps)
			{
				auto& Mat = MaterialProps[Data.MaterialStart + dp.material];
				Mat.Data = DataIndex;

				Mat.DrawProps.push_back({
					static_cast<int>(MeshIndex),
					dp.indexBase,
					dp.count });

			}
		}
	}

	bool ret{};
	if (UsingLighting())
	{
		ret = CreateBuffers<LitVertex, IndexType>(
			State.TotalVertexCount, State.TotalIndexCount,
			GetFVF(), VertexBuffer, IndexBuffer);
	}
	else
	{
		ret = CreateBuffers<SimpleVertex, IndexType>(
			State.TotalVertexCount, State.TotalIndexCount,
			GetFVF(), VertexBuffer, IndexBuffer);
	}

	if (!ret)
	{
		MLog("RBspObjectDrawD3D9::Create -- Failed to create vertex and index buffers\n");
		return;
	}

	DMLog("Create vertex buffer with %d vertices, index buffer with %d indices\n",
		State.TotalVertexCount, State.TotalIndexCount);

	// Lock vertex and index buffers.
	auto Lock = [](auto& Buffer, auto*& Pointer)
	{
		if (FAILED(Buffer->Lock(0, 0, reinterpret_cast<void**>(&Pointer), 0)))
		{
			MLog("Oh no!\n");
			return false;
		}
		return true;
	};
	char* Verts{};        if (!Lock(VertexBuffer, Verts))  return;
	IndexType* Indices{}; if (!Lock(IndexBuffer, Indices)) return;

	// Insert transformed vertices for each object into the vertex buffer
	// and keep a list of the offset of each mesh's vertices.

	// Dimensions: [Data][Object][Mesh]
	// TODO: Change this terrible type
	std::vector<std::vector<std::vector<int>>> MeshVertexOffsets;
	MeshVertexOffsets.resize(State.Objects.size());
	int CumulativeVertexBase{};
	for (auto& pair : State.ObjectMap)
	{
		auto& Data = State.ObjectData[pair.first];
		auto& DataOffsets = MeshVertexOffsets[pair.first];
		auto ObjectIndexCount = pair.second.size();
		DataOffsets.resize(ObjectIndexCount);
		for (size_t ObjectListIndex{}; ObjectListIndex < ObjectIndexCount; ++ObjectListIndex)
		{
			auto ObjectIndex = pair.second[ObjectListIndex];
			auto& Object = State.Objects[ObjectIndex];

			auto& MeshOffsets = DataOffsets[ObjectListIndex];
			auto MeshCount = Data.Meshes.size();
			MeshOffsets.resize(MeshCount);
			for (size_t MeshIndex{}; MeshIndex < MeshCount; ++MeshIndex)
			{
				auto& Mesh = Data.Meshes[MeshIndex];
				MeshOffsets[MeshIndex] = CumulativeVertexBase;

				for (size_t i{}; i < Mesh.VertexCount; ++i)
				{
					auto AddVertexData = [&](auto&& val) {
						memcpy(Verts, &val, sizeof(val));
						Verts += sizeof(val);
					};
					if (!UsingLighting())
					{
						AddVertexData(Mesh.Positions[i] * Mesh.World * Object.World);
						AddVertexData(Mesh.TexCoords[i]);
					}
					else
					{
						AddVertexData(Mesh.Positions[i] * Mesh.World * Object.World);
						AddVertexData(Mesh.Normals[i]);
						AddVertexData(Mesh.TexCoords[i]);
						AddVertexData(Mesh.Tangents[i]);
					}
				}
				CumulativeVertexBase += Mesh.VertexCount;
			}
		}
	}

	// Insert indices for each object
	MaterialBatches.resize(State.Materials.size());
	int CumulativeIndexBase{};
	for (size_t MaterialIndex{}; MaterialIndex < Materials.size(); ++MaterialIndex)
	{
		auto& Mat = MaterialProps[MaterialIndex];
		auto& Batch = MaterialBatches[MaterialIndex];

		Batch.TriangleCount = 0;
		Batch.VertexBase = 0;
		Batch.IndexBase = CumulativeIndexBase;

		auto& Data = State.ObjectData[Mat.Data];
		auto& ObjectIndices = State.ObjectMap[Mat.Data];
		assert(!ObjectIndices.empty());

		DMLog("Create indices for material %d\n", MaterialIndex);

		for (size_t ObjectListIndex{}; ObjectListIndex < ObjectIndices.size(); ++ObjectListIndex)
		{
			for (auto& dp : Mat.DrawProps)
			{
				for (size_t i{}; i < dp.Count; ++i)
				{
					for (size_t j{}; j < 3; ++j)
					{
						auto Index = Data.Meshes[dp.Mesh].Indices[dp.IndexBase + i * 3 + j];
						auto VertexOffset = MeshVertexOffsets[Mat.Data][ObjectListIndex][dp.Mesh];
						*Indices = VertexOffset + Index;
						++Indices;
					}
				}

				CumulativeIndexBase += dp.Count * 3;
				Batch.TriangleCount += dp.Count;
			}
		}

		DMLog("Batch: TriangleCount = %d, VertexBase = %d, IndexBase = %d\n",
			Batch.TriangleCount, Batch.VertexBase, Batch.IndexBase);
	}

	DMLog("Added %d vertices, %d indices\n", CumulativeVertexBase, CumulativeIndexBase);

	// Unlock vertex and index buffers.
	VertexBuffer->Unlock();
	IndexBuffer->Unlock();

	// Sort materials by state changes.
	auto NumMaterials = MaterialBatches.size();
	NormalMaterialsEnd = OpacityMaterialsEnd = AlphaTestMaterialsEnd = NumMaterials;

	auto SortMaterials = [&](auto Start, auto& End, auto& Pred)
	{
		for (auto i = Start; i < End; ++i)
		{
			if (!Pred(Materials[i]))
				continue;

			--End;
			auto DestIndex = End;
			std::swap(MaterialBatches[i], MaterialBatches[End]);
			std::swap(Materials[i], Materials[End]);
			--i;
		}
	};

	SortMaterials(0, NormalMaterialsEnd, [](auto& Mat) { return Mat.HasOpacity(); });
	SortMaterials(NormalMaterialsEnd, OpacityMaterialsEnd, [](auto& Mat) { return Mat.HasAlphaTest(); });

	DMLog("NormalMaterialsEnd = %d\n"
		"OpacityMaterialsEnd = %d\n"
		"AlphaTestMaterialsEnd = %d\n",
		NormalMaterialsEnd, OpacityMaterialsEnd, AlphaTestMaterialsEnd);
}

void RBspObjectDrawD3D9::CreateRTs()
{
	if (!UsingLighting())
		return;

	for (size_t i{}; i < ArraySize(RTs); ++i)
	{
		auto& RT = RTs[i];

		D3DFORMAT Format = i == static_cast<size_t>(RTType::LinearDepth) ? D3DFMT_R32F : D3DFMT_X8R8G8B8;

		if (FAILED(dev->CreateTexture(
			RGetScreenWidth(), RGetScreenHeight(),
			1, D3DUSAGE_RENDERTARGET,
			Format,
			D3DPOOL_DEFAULT,
			MakeWriteProxy(RT),
			nullptr)))
		{
			MLog("RBspObjectDrawD3D9::CreateShaderStuff -- Failed to create render target\n");
		}
	}

	if (FAILED(RGetDevice()->CreateDepthStencilSurface(
		RGetScreenWidth(), RGetScreenHeight(),
		GetDepthStencilFormat(),
		D3DMULTISAMPLE_NONE,
		0,
		FALSE,
		MakeWriteProxy(DummyDepthSurface),
		nullptr)))
	{
		MLog("RBspObjectDrawD3D9::CreateShaderStuff -- Failed to create depth surface\n");
	}
}

void RBspObjectDrawD3D9::ReleaseRTs()
{
	for (auto& RT : RTs)
		SAFE_RELEASE(RT);
}

void RBspObjectDrawD3D9::Create(rsx::LoaderState && srcState)
{
	Materials.clear();
	TextureMemory.clear();
	MaterialBatches.clear();
	VertexBuffer.reset();
	IndexBuffer.reset();
	NormalMaterialsEnd = 0;
	OpacityMaterialsEnd = 0;
	AlphaTestMaterialsEnd = 0;

	State = std::move(srcState);

	CreateTextures();
	CreateShaderStuff();
	CreateBatches();
	
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

static void DrawBatch(const MaterialBatch& Mat)
{
	auto hr = RGetDevice()->DrawIndexedPrimitive(
		D3DPT_TRIANGLELIST,    // Primitive type
		Mat.VertexBase,        // BaseVertexIndex
		0,                     // MinVertexIndex
		Mat.TriangleCount * 3, // NumVertices
		Mat.IndexBase,         // Start index
		Mat.TriangleCount);    // Primitive count

	assert(SUCCEEDED(hr));
}

template <RBspObjectDrawD3D9::MaterialType Type>
void RBspObjectDrawD3D9::SetMaterial(Material& Mat)
{
	if (!UsingLighting())
	{
		dev->SetTexture(0, GetTexture(Mat.Diffuse));
		if (Type > Normal)
			dev->SetTexture(1, GetTexture(Mat.Opacity));
		if (Type > Opacity)
			dev->SetRenderState(D3DRS_ALPHAREF, Mat.AlphaTestValue);
	}
	else
	{
		int TexIndices[] = { Mat.Diffuse, Mat.Normal, Mat.Specular, Mat.Opacity, Mat.Emissive };
		for (size_t i{}; i < ArraySize(TexIndices); ++i)
			dev->SetTexture(i, GetTexture(TexIndices[i]));
		if (Type > Opacity)
			dev->SetRenderState(D3DRS_ALPHAREF, Mat.AlphaTestValue);
		SetPSFloat(DeferredShaderConstant::Opacity, Type <= Normal);
	}
}

template <RBspObjectDrawD3D9::MaterialType Type>
void RBspObjectDrawD3D9::SetPrerenderStates()
{
	switch (Type)
	{
	case Normal:
		dev->SetRenderState(D3DRS_ZWRITEENABLE, true);
		dev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

		dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
		break;
	case Opacity:
		dev->SetRenderState(D3DRS_ZWRITEENABLE, false);
		dev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

		dev->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
		dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		break;
	case AlphaTest:
		dev->SetRenderState(D3DRS_ZWRITEENABLE, true);
		dev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
		dev->SetRenderState(D3DRS_ALPHATESTENABLE, true);
		dev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

		dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		break;
	}
}

template <RBspObjectDrawD3D9::MaterialType Type>
void RBspObjectDrawD3D9::RenderMaterials(int StartIndex, int EndIndex)
{
	if (StartIndex == EndIndex)
		return;

	SetPrerenderStates<Type>();

	for (int i = StartIndex; i < EndIndex; ++i)
	{
		auto& Batch = MaterialBatches[i];
		auto& Mat = Materials[i];

		SetMaterial<Type>(Mat);

		if (Mat.TwoSided)
		{
			dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			DrawBatch(Batch);
			dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		}
		else
			DrawBatch(Batch);
	}
}

LPDIRECT3DTEXTURE9 RBspObjectDrawD3D9::GetTexture(int Index) const
{
	if (Index == static_cast<u16>(-1))
		return nullptr;

	return TextureMemory[Index].get()->GetTexture();
}

u32 RBspObjectDrawD3D9::GetFVF() const
{
	return D3DFVF_XYZ | D3DFVF_TEX1;
}

size_t RBspObjectDrawD3D9::GetStride() const
{
	return UsingLighting() ? sizeof(LitVertex) : sizeof(SimpleVertex);
}

bool RBspObjectDrawD3D9::UsingLighting() const
{
	return SupportsDynamicLighting && bsp.m_bisDrawLightMap;
}

static void DrawQuad(const v2& p1, const v2& p2)
{
	struct VertexType
	{
		float x, y, z, rhw;
		float u, v;
	};

	VertexType Vertices[] = {
		{ p1.x - 0.5f, p1.y - 0.5f, 0, 1, 0, 0 },
		{ p1.x - 0.5f, p2.y - 0.5f, 0, 1, 0, 1 },
		{ p2.x - 0.5f, p2.y - 0.5f, 0, 1, 1, 1 },
		{ p2.x - 0.5f, p1.y - 0.5f, 0, 1, 1, 0 },
	};

	RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	RGetDevice()->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

	RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &Vertices, sizeof(VertexType));
}

static void DrawFullscreenQuad()
{
	auto Width = (float)RGetScreenWidth();
	auto Height = (float)RGetScreenHeight();
	DrawQuad({ 0, 0 }, { Width, Height });
}

static void DrawAmbient(LPDIRECT3DTEXTURE9 Ambient)
{
	RGetDevice()->SetTexture(0, Ambient);
	DrawFullscreenQuad();
}

void RBspObjectDrawD3D9::ShowRTs()
{
	auto ShowRT = [&](auto& RT, auto&& p1, auto&& p2) {
		RGetDevice()->SetTexture(0, RT.get());
		DrawQuad(p1, p2);
	};

	auto ShowDepth = [&](auto& RT, auto&& p1, auto&& p2) {
		RGetDevice()->SetPixelShader(VisualizeLinearDepthPS.get());
		GetRenderer().SetScreenSpaceVertexDeclaration();
		ShowRT(RT, p1, p2);
		RGetDevice()->SetPixelShader(nullptr);
	};

	for (size_t i{}; i < ArraySize(RTs); ++i)
	{
		auto Width = float(RGetScreenWidth());
		auto Height = float(RGetScreenHeight());
		v2 p1{ Width / 2 + (i % 2) * Width / 4, Height / 2 + (i >= 2) * Height / 4 };
		v2 p2{ p1 + v2{Width / 4, Height / 4} };
		if (i == static_cast<size_t>(RTType::LinearDepth))
			ShowDepth(RTs[i], p1, p2);
		else
			ShowRT(RTs[i], p1, p2);
	}
}

static bool IsSphereInsideFrustum(const v3& Center, float Radius, const rplane(&Frustum)[6])
{
	for (auto& Plane : Frustum)
		if (DotProduct(Center, v3{ EXPAND_VECTOR(Plane) }) + Plane.d < -Radius)
			return false;
	return true;
}

void RBspObjectDrawD3D9::DrawLighting()
{
	dev->SetRenderTarget(0, OrigRT.get());
	OrigRT.reset();
	for (size_t i = 1; i < ArraySize(RTs); ++i)
		dev->SetRenderTarget(i, nullptr);

	dev->SetVertexShader(nullptr);
	dev->SetPixelShader(nullptr);

	// Enable additive blending
	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	dev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	// Disable depth writing and testing
	dev->SetRenderState(D3DRS_ZWRITEENABLE, false);
	dev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

	DrawAmbient(GetRT(RTType::Ambient).get());

	dev->SetVertexShader(PointLightVS.get());
	dev->SetPixelShader(PointLightPS.get());

	for (size_t i{}; i < ArraySize(RTs); ++i)
		dev->SetTexture(i, RTs[i].get());

	SetShaderMatrix(LightingShaderConstant::Proj, Transpose(RProjection));
	v2 InvRes{ 1.0f / RGetScreenWidth(), 1.0f / RGetScreenHeight() };
	SetShaderVector2(LightingShaderConstant::InvRes, InvRes);
	v2 InvFocalLen{ 1.0f / RProjection._11, 1.0f / RProjection._22 };
	SetShaderVector2(LightingShaderConstant::InvFocalLen, InvFocalLen);
	SetShaderFloat(LightingShaderConstant::Near, GetRenderer().GetNearZ());
	SetShaderFloat(LightingShaderConstant::Far, GetRenderer().GetFarZ());

	dev->SetFVF(D3DFVF_XYZ);

	for (auto& Light : bsp.StaticMapLightList)
	{
		if (!IsSphereInsideFrustum(Light.Position, Light.fAttnEnd, RViewFrustum))
			continue;

		auto& pos = Light.Position * RView;
		SetShaderVector4(LightingShaderConstant::Light, { EXPAND_VECTOR(pos), Light.fAttnEnd });
		SetShaderVector3(LightingShaderConstant::LightColor, Light.Color);

		if (Magnitude(pos) < Light.fAttnEnd * 1.3 + GetRenderer().GetNearZ())
		{
			dev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
			dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		}
		else
		{
			dev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
			dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		}

		dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, data::nvert / 3, data::sphere, 3 * sizeof(float));
	}

	// Reset states
	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	dev->SetRenderState(D3DRS_ZWRITEENABLE, true);
	dev->SetRenderState(D3DRS_ZENABLE, true);

	dev->SetVertexShader(nullptr);
	dev->SetPixelShader(nullptr);

	if (ShowRTsEnabled)
		ShowRTs();
}

void RBspObjectDrawD3D9::DepthCopy()
{
	dev->SetRenderState(D3DRS_COLORWRITEENABLE, 0);

	SetShaderFloat(DepthCopyShaderConstant::Near, GetRenderer().GetNearZ());
	SetShaderFloat(DepthCopyShaderConstant::Far, GetRenderer().GetFarZ());

	dev->SetTexture(0, GetRT(RTType::LinearDepth).get());

	dev->SetVertexShader(DepthCopyVS.get());
	dev->SetPixelShader(DepthCopyPS.get());

	DrawFullscreenQuad();

	dev->SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);
}

void RBspObjectDrawD3D9::SetPrologueStates()
{
	if (bsp.m_bWireframe)
		SetWireframeStates();
	else
		SetDefaultStates();

	// Enable Z-buffer testing and writing.
	RSetWBuffer(true);
	dev->SetRenderState(D3DRS_ZWRITEENABLE, true);
	dev->SetRenderState(D3DRS_LIGHTING, FALSE);
	dev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	// Map triangles are clockwise, so cull counter-clockwise faces.
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	dev->SetStreamSource(0, VertexBuffer.get(), 0, GetStride());
	dev->SetIndices(IndexBuffer.get());

	if (!UsingLighting())
	{
		dev->SetFVF(GetFVF());
	}
	else
	{
		dev->SetVertexDeclaration(LitVertexDecl.get());
		dev->SetVertexShader(DeferredVS.get());
		dev->SetPixelShader(DeferredPS.get());

		const auto& WorldView = RView;
		SetShaderMatrix(DeferredShaderConstant::WorldView, Transpose(WorldView));
		const auto& Projection = RProjection;
		auto WorldViewProjection = WorldView * RProjection;
		SetShaderMatrix(DeferredShaderConstant::WorldViewProjection, Transpose(WorldViewProjection));

		SetShaderFloat(DeferredShaderConstant::Near, GetRenderer().GetNearZ());
		SetShaderFloat(DeferredShaderConstant::Far, GetRenderer().GetFarZ());

		dev->GetRenderTarget(0, MakeWriteProxy(OrigRT));
		for (size_t i{}; i < ArraySize(RTs); ++i)
		{
			D3DPtr<IDirect3DSurface9> Surface;
			RTs[i]->GetSurfaceLevel(0, MakeWriteProxy(Surface));
			dev->SetRenderTarget(i, Surface.get());
		}
	}
}

void RBspObjectDrawD3D9::SetEpilogueStates()
{
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

	// Disable shaders
	dev->SetVertexShader(nullptr);
	dev->SetPixelShader(nullptr);
}

void RBspObjectDrawD3D9::Draw()
{
	SetPrologueStates();

	RenderMaterials<Normal>(0, NormalMaterialsEnd);
	RenderMaterials<Opacity>(NormalMaterialsEnd, OpacityMaterialsEnd);
	RenderMaterials<AlphaTest>(OpacityMaterialsEnd, AlphaTestMaterialsEnd);

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
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	if (UsingLighting())
		DrawLighting();

	SetEpilogueStates();
}

_NAMESPACE_REALSPACE2_END