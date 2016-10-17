#include "stdafx.h"
// RBspObject.cpp: 99.2.9 by dubble

#include <crtdbg.h>
#include <map>
#include <array>
#include "MXml.h"
#include "MZFileSystem.h"
#include "RBspObject.h"
#include "RMaterialList.h"
#include "RealSpace2.h"
#include "RBaseTexture.h"
#include "MDebug.h"
#include "RVersions.h"
#include "RMaterialList.h"
#include "RVisualMeshMgr.h"
#include "FileInfo.h"
#include "ROcclusionList.h"
#include "MProfiler.h"
#include "RShaderMgr.h"
#include "RLenzFlare.h"
#include <dxerr9.h>
#include "RNavigationNode.h"
#include <fstream>
#include "ShaderGlobals.h"
#include "RS2.h"

#undef pi

#ifndef _PUBLISH

#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);

#else

#define __BP(i,n) ;
#define __EP(i) ;

#endif

#define TOLERENCE 0.001f
#define SIGN(x) ( (x)<-TOLERENCE ? -1 : (x)>TOLERENCE ? 1 : 0 )

#define MAX_LIGHTMAP_SIZE		1024
#define MAX_LEVEL_COUNT			10

#define DEFAULT_BUFFER_SIZE	1000

_NAMESPACE_REALSPACE2_BEGIN

static LPDIRECT3DTEXTURE9 g_pShademap = nullptr;

static int nsplitcount, nleafcount;
static int g_nFrameNumber;

#ifdef _DEBUG
int g_nPoly, g_nCall;
int g_nPickCheckPolygon, g_nRealPickCheckPolygon;
#endif

struct OpenNodesState
{
	BSPVERTEX* Vertices;
	RSBspNode* Node;
	RPOLYGONINFO* Info;
	BSPNORMALVERTEX* Normals;
	int PolygonID;
};

void DrawBoundingBox(rboundingbox *bb, DWORD color)
{
	int i, j;

	int ind[8][3] = { {0,0,0},{1,0,0},{1,1,0},{0,1,0}, {0,0,1},{1,0,1},{1,1,1},{0,1,1} };
	int lines[12][2] = { {0,1},{1,5},{5,4},{4,0},{5,6},{1,2},{0,3},{4,7},{7,6},{6,2},{2,3},{3,7} };

	for (i = 0; i < 12; i++)
	{
		rvector a, b;
		for (j = 0; j < 3; j++)
		{
			a[j] = ind[lines[i][0]][j] ? bb->vmax[j] : bb->vmin[j];
			b[j] = ind[lines[i][1]][j] ? bb->vmax[j] : bb->vmin[j];
		}

		RDrawLine(a, b, color);
	}
}

// TODO: Remove this piece of global state
static bool m_bisDrawLightMap = true;

RSBspNode::RSBspNode()
{
	m_pPositive = m_pNegative = NULL;
	nPolygon = 0;
	pInfo = NULL;

	nFrameCount = -1;
	pDrawInfo = NULL;
}

RSBspNode::~RSBspNode()
{
	SAFE_DELETE_ARRAY(pDrawInfo);
}

void RSBspNode::DrawBoundingBox(DWORD color)
{
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	RealSpace2::DrawBoundingBox(&bbTree, color);

	if (m_pNegative) m_pNegative->DrawBoundingBox(color);
	if (m_pPositive) m_pPositive->DrawBoundingBox(color);
}

void RSBspNode::DrawWireFrame(int nPolygon, DWORD color)
{
	RPOLYGONINFO *info = &pInfo[nPolygon];

	for (int i = 0; i < info->nVertices; i++)
		RDrawLine(*info->pVertices[i].Coord(), *info->pVertices[(i + 1) % info->nVertices].Coord(), color);
}

RSBspNode* RSBspNode::GetLeafNode(const rvector &pos)
{
	if (nPolygon) return this;
	if (plane.a*pos.x + plane.b*pos.y + plane.c*pos.z + plane.d>0)
		return m_pPositive->GetLeafNode(pos);
	else
		return m_pNegative->GetLeafNode(pos);
}

RBspLightmapManager::RBspLightmapManager()
{
	m_nSize = MAX_LIGHTMAP_SIZE;
	m_pData = new DWORD[MAX_LIGHTMAP_SIZE*MAX_LIGHTMAP_SIZE];
	m_pFreeList = new RFREEBLOCKLIST[MAX_LEVEL_COUNT + 1];

	POINT p = { 0,0 };
	m_pFreeList[MAX_LEVEL_COUNT].push_back(p);
}

RBspLightmapManager::~RBspLightmapManager()
{
	Destroy();
}

void RBspLightmapManager::Destroy()
{
	SAFE_DELETE(m_pData);
	if (m_pFreeList) {
		delete[]m_pFreeList;
		m_pFreeList = NULL;
	}
}

float RBspLightmapManager::CalcUnused()
{
	float fUnused = 0.f;

	for (int i = 0; i <= MAX_LEVEL_COUNT; i++) {
		float fThisLevelSize = pow(0.25, (MAX_LEVEL_COUNT - i));
		fUnused += (float)m_pFreeList[i].size()*fThisLevelSize;
	}

	return fUnused;
}

bool RBspLightmapManager::GetFreeRect(int nLevel, POINT *pt)
{
	if (nLevel > MAX_LEVEL_COUNT) return false;

	if (!m_pFreeList[nLevel].size())
	{
		POINT point;
		if (!GetFreeRect(nLevel + 1, &point))
			return false;

		int nSize = 1 << nLevel;

		POINT newpoint;

		newpoint.x = point.x + nSize; newpoint.y = point.y;
		m_pFreeList[nLevel].push_back(newpoint);

		newpoint.x = point.x; newpoint.y = point.y + nSize;
		m_pFreeList[nLevel].push_back(newpoint);

		newpoint.x = point.x + nSize; newpoint.y = point.y + nSize;
		m_pFreeList[nLevel].push_back(newpoint);

		*pt = point;

	}
	else
	{
		*pt = *m_pFreeList[nLevel].begin();
		m_pFreeList[nLevel].erase(m_pFreeList[nLevel].begin());
	}

	return true;
}

bool RBspLightmapManager::Add(DWORD *data, int nSize, POINT *retpoint)
{
	int nLevel = 0, nTemp = 1;
	while (nSize > nTemp)
	{
		nTemp = nTemp << 1;
		nLevel++;
	}
	_ASSERT(nSize == nTemp);

	POINT pt;
	if (!GetFreeRect(nLevel, &pt))
		return false;

	for (int y = 0; y < nSize; y++)
	{
		for (int x = 0; x < nSize; x++)
		{
			m_pData[(y + pt.y)*GetSize() + (x + pt.x)] = data[y*nSize + x];
		}
	}
	*retpoint = pt;
	return true;
}

void RBspLightmapManager::Save(const char *filename)
{
	RSaveAsBmp(GetSize(), GetSize(), m_pData, filename);
}

RMapObjectList::~RMapObjectList()
{
	while (size())
	{
		Delete(begin());
	}
}

RMapObjectList::iterator RMapObjectList::Delete(iterator i)
{
	ROBJECTINFO *info = *i;
	delete info->pVisualMesh;
	delete info;
	return erase(i);
}

////////////////////////////
// RBspObject
////////////////////////////

RBspObject::RBspObject(bool PhysOnly)
	: PhysOnly(PhysOnly), DrawObj{ VulkanTag{}, *this }
{
	m_MeshList.SetMtrlAutoLoad(true);
	m_MeshList.SetMapObject(true);

#ifdef SHADOW_TEST
	RenderWithNormal = true;
#endif
}

void RBspObject::ClearLightmaps()
{
	m_nLightmap = 0;
	LightmapTextures.clear();
}

void RBspObject::LightMapOnOff(bool bDraw)
{
	if (m_bisDrawLightMap == bDraw)
		return;

	m_bisDrawLightMap = bDraw;

	if (bDraw)
	{
		OpenLightmap();
		Sort_Nodes(OcRoot.data());
		if (!RIsHardwareTNL())
			CreatePolygonTable();
		else
			UpdateVertexBuffer();
	}
	else
	{
		ClearLightmaps();
		Sort_Nodes(OcRoot.data());
		if (!RIsHardwareTNL())
			CreatePolygonTable();
	}
}

void RBspObject::SetDrawLightMap(bool b) {
	m_bisDrawLightMap = b;
}

void RBspObject::DrawNormal(int nIndex, float fSize)
{
	RCONVEXPOLYGONINFO *pInfo = &ConvexPolygons[nIndex];

	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	for (int j = 0; j < pInfo->nVertices; j++)
	{
		// Draw edge
		RDrawLine(pInfo->pVertices[j], pInfo->pVertices[(j + 1) % pInfo->nVertices], 0xff808080);

		// Draw normal
		RDrawLine(pInfo->pVertices[j], pInfo->pVertices[j] + fSize*pInfo->pNormals[j], 0xff00ff);
	}
}

void RBspObject::SetDiffuseMap(int nMaterial)
{
	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();

	RBaseTexture *pTex = Materials[nMaterial].texture;
	if (pTex)
	{
		pd3dDevice->SetTexture(0, pTex->GetTexture());
		pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	}
	else
	{
		auto Diffuse = VECTOR2RGB24(Materials[nMaterial].Diffuse);
		pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, Diffuse);
		pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	}
}

// TODO: This whole setup seems like an awful way of drawing anything. Fix.
template <typename T>
static void DrawImpl(RSBspNode& Node, int Material, T& DrawFunc)
{
#ifndef SHADOW_TEST
	// nFrameCount is updated to the current frame number
	// only for nodes that aren't occluded.
	if (Node.nFrameCount != g_nFrameNumber)
		return;
#endif

	// Leaf node
	if (Node.nPolygon)
	{
		auto TriangleCount = Node.pDrawInfo[Material].nTriangleCount;
		if (TriangleCount)
		{
#ifdef _DEBUG
			g_nCall++;
			g_nPoly += TriangleCount;
#endif
			DrawFunc(Node, TriangleCount);
		}
		return;
	}

	// Branch node
	auto DrawNode = [&](auto Branch) {
		if (Node.*Branch)
			DrawImpl(*(Node.*Branch), Material, DrawFunc);
	};

	DrawNode(&RSBspNode::m_pPositive);
	DrawNode(&RSBspNode::m_pNegative);
}

void RBspObject::UpdateUBO()
{
	DrawObj.Get<RBspObjectDrawVulkan>().UpdateUniformBuffers();
}

void RBspObject::Draw(RSBspNode* Node, int Material)
{
	DrawImpl(*Node, Material, [&](auto& Node, auto Count)
	{
		auto hr = RGetDevice()->DrawIndexedPrimitive(
			D3DPT_TRIANGLELIST,
			0, 0, OcVertices.size(),
			Node.pDrawInfo[Material].nIndicesOffset, Count);
		assert(SUCCEEDED(hr));
	});
}

void RBspObject::DrawNoTNL(RSBspNode *Node, int Material)
{
	DrawImpl(*Node, Material, [&](auto& Node, int Count)
	{
		int index = Node.pDrawInfo[Material].nIndicesOffset;
		auto hr = RGetDevice()->DrawIndexedPrimitiveUP(
			D3DPT_TRIANGLELIST, 0,
			Node.pDrawInfo[Material].nVertice, Count,
			OcIndices.data() + index, D3DFMT_INDEX16,
			Node.pDrawInfo[Material].pVertices, sizeof(BSPVERTEX));
		assert(SUCCEEDED(hr));
	});
}

template <u32 Flags, bool ShouldHaveFlags, bool SetAlphaTestFlags>
void RBspObject::DrawNodes(int LoopCount)
{
	if (GetRenderer().IsDrawingShadows())
		DrawNodesImpl<Flags, ShouldHaveFlags, SetAlphaTestFlags, false>(LoopCount);
	else
		DrawNodesImpl<Flags, ShouldHaveFlags, SetAlphaTestFlags, true>(LoopCount);
}

template <u32 Flags, bool ShouldHaveFlags, bool SetAlphaTestFlags, bool SetTextures>
void RBspObject::DrawNodesImpl(int LoopCount)
{
	auto dev = RGetDevice();

	// Materials 0 is always empty, don't render
	for (int i = 1; i < LoopCount; i++)
	{
		auto MaterialIndex = i % m_nMaterial;
		if ((Materials[MaterialIndex].dwFlags & Flags) != (ShouldHaveFlags ? Flags : 0))
			continue;

		// Cull back faces on materials that aren't two-sided
		if ((Materials[MaterialIndex].dwFlags & RM_FLAG_TWOSIDED) == 0)
			dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		else
			dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		if (SetTextures)
		{
			SetDiffuseMap(MaterialIndex);

			if (!LightmapTextures.empty())
				dev->SetTexture(static_cast<DWORD>(ShaderSampler::Lightmap), LightmapTextures[i / m_nMaterial].get());
		}

		if (SetAlphaTestFlags)
		{
			if ((Materials[MaterialIndex].dwFlags & RM_FLAG_USEALPHATEST) != 0) {
				dev->SetRenderState(D3DRS_ZWRITEENABLE, true);
				dev->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x80808080);
				dev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
				dev->SetRenderState(D3DRS_ALPHATESTENABLE, true);

				dev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
				dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
				dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			}
			else {
				dev->SetRenderState(D3DRS_ZWRITEENABLE, false);
				dev->SetRenderState(D3DRS_ALPHATESTENABLE, false);
				dev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
				dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

				dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

				dev->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
				dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			}
		}

		if (RIsHardwareTNL())
			Draw(&OcRoot[0], i);
		else
			DrawNoTNL(&OcRoot[0], i);

		break;
	}
}

static void SetWireframeStates()
{
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

// Shows each pixel of the lightmap on a white map
static void SetShowLightmapStates()
{
	RGetDevice()->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

	RGetDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
	RGetDevice()->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	RGetDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE4X);
	RGetDevice()->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TFACTOR);
}

static void SetDefaultStates(bool Lightmap)
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

	if (!Lightmap)
	{
		RGetDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	}
	else
	{
		RGetDevice()->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		RGetDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE4X);
		RGetDevice()->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	}
}

bool RBspObject::Draw()
{
	if (GetRS2().UsingVulkan())
	{
		DrawObj.DrawStatic<RBspObjectDrawVulkan>();
		return true;
	}
	
	g_nFrameNumber++;
#ifdef _DEBUG
	g_nPoly = 0;
	g_nCall = 0;
#endif

	if (RIsHardwareTNL() &&
		(!VertexBuffer || !IndexBuffer)) return false;

	auto dev = RGetDevice();

	// Disable alpha blending
	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

	dev->SetFVF(GetFVF());

	// Set vertex and index buffers
	dev->SetStreamSource(0, VertexBuffer, 0, GetStride());
	dev->SetIndices(IndexBuffer);

	if (m_bWireframe)
		SetWireframeStates();
	else if (m_bShowLightmap)
		SetShowLightmapStates();
	else
		SetDefaultStates(!LightmapTextures.empty());

	if (!GetRenderer().IsDrawingShadows())
	{
		dev->SetTexture(0, nullptr);
		dev->SetTexture(1, nullptr);
	}
	dev->SetRenderState(D3DRS_LIGHTING, FALSE);

	// Disable texture stage 2
	dev->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	dev->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	// Update occlusion list and local view frustum
	{
		rmatrix World;
		dev->GetTransform(D3DTS_WORLD, &World);

		m_OcclusionList.UpdateCamera(World, RCameraPosition);

		rmatrix trWorld = Transpose(World);

		for (int i = 0; i < 6; i++)
			m_localViewFrustum[i] = Transform(RGetViewFrustum()[i], trWorld);
	}

	_BP("ChooseNodes"); ChooseNodes(OcRoot.data()); _EP("ChooseNodes");

	// Disable alpha blending
	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

	// Enable Z-buffer testing and writing
	RSetWBuffer(true);
	dev->SetRenderState(D3DRS_ZWRITEENABLE, true);

	int nCount = m_nMaterial * max(1, m_nLightmap);

	// Draw non-additive and opaque materials
	DrawNodes<RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY, false, false>(nCount);

	if (!GetRenderer().IsDrawingShadows())
	{
		// Draw transparent materials
		DrawNodes<RM_FLAG_USEOPACITY, true, true>(nCount);

		// Draw additive materials
		dev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		dev->SetRenderState(D3DRS_ZWRITEENABLE, false);

		DrawNodes<RM_FLAG_ADDITIVE, true, false>(nCount);
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
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);

	return true;
}

void RBspObject::SetObjectLight(const rvector& vPos)
{
	float fIntensityFirst = FLT_MIN;

	float fdistance = 0.f;
	float fIntensity = 0.f;

	RLIGHT *first = nullptr;

	D3DLIGHT9 light{};

	light.Type = D3DLIGHT_POINT;

	light.Attenuation0 = 0.f;
	light.Attenuation1 = 0.0010f;
	light.Attenuation2 = 0.f;

	for (auto& Light : GetObjectLightList())
	{
		fdistance = Magnitude(Light.Position - vPos);
		fIntensity = (fdistance - Light.fAttnStart) / (Light.fAttnEnd - Light.fAttnStart);

		fIntensity = min(max(1.0f - fIntensity, 0.f), 1.f);
		fIntensity *= Light.fIntensity;

		fIntensity = min(max(fIntensity, 0.f), 1.f);

		if (fIntensityFirst < fIntensity) {
			fIntensityFirst = fIntensity;
			first = &Light;
		}
	}

	if (first)
	{
		light.Position = first->Position;

		light.Ambient.r = min(first->Color.x*first->fIntensity * 0.25f, 1.f);
		light.Ambient.g = min(first->Color.y*first->fIntensity * 0.25f, 1.f);
		light.Ambient.b = min(first->Color.z*first->fIntensity * 0.25f, 1.f);

		light.Diffuse.r = min(first->Color.x*first->fIntensity * 0.25f, 1.f);
		light.Diffuse.g = min(first->Color.y*first->fIntensity * 0.25f, 1.f);
		light.Diffuse.b = min(first->Color.z*first->fIntensity * 0.25f, 1.f);

		light.Specular.r = 1.f;
		light.Specular.g = 1.f;
		light.Specular.b = 1.f;

		light.Range = first->fAttnEnd*1.0f;

		RGetDevice()->SetLight(0, &light);
		RGetDevice()->LightEnable(0, TRUE);

		RShaderMgr::getShaderMgr()->setLight(0, &light);
	}
}

void RBspObject::DrawObjects()
{
	m_DebugInfo.nMapObjectFrustumCulled = 0;
	m_DebugInfo.nMapObjectOcclusionCulled = 0;

	RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	RGetDevice()->SetRenderState(D3DRS_LIGHTING, TRUE);

	rmatrix world;
	RGetDevice()->GetTransform(D3DTS_WORLD, &world);

	rvector v_add = GetTransPos(world);

	rvector camera_pos = RCameraPosition;
	rvector t_vec;

	for (list<ROBJECTINFO*>::iterator i = m_ObjectList.begin(); i != m_ObjectList.end(); i++) {

		ROBJECTINFO *pInfo = *i;
		if (!pInfo->pVisualMesh) continue;

		if (pInfo) {
			t_vec = GetTransPos(pInfo->pVisualMesh->m_WorldMat);
			t_vec = camera_pos - t_vec;
			pInfo->fDist = Magnitude(t_vec);
		}
	}

	// Sort by distance
	m_ObjectList.sort([&](auto* a, auto* b) { return a->fDist < b->fDist; });

	for (auto* pInfo : m_ObjectList)
	{
		if (!pInfo->pVisualMesh) continue;

		rboundingbox bb;
		bb.vmax = pInfo->pVisualMesh->m_vBMax;
		bb.vmin = pInfo->pVisualMesh->m_vBMin;

		if (!m_bNotOcclusion) {
			if (!IsVisible(bb)) {
				m_DebugInfo.nMapObjectOcclusionCulled++;
				continue;
			}
		}
		else {
			pInfo->pVisualMesh->m_bCheckViewFrustum = false;
		}

		bool bLight = true;

		if (pInfo->pVisualMesh && pInfo->pVisualMesh->m_pMesh)
			bLight = !pInfo->pVisualMesh->m_pMesh->m_LitVertexModel;

		if (pInfo->pLight && bLight)
		{
			D3DLIGHT9 light{};

			light.Type = D3DLIGHT_POINT;

			light.Attenuation0 = 0.f;
			light.Attenuation1 = 0.0001f;
			light.Attenuation2 = 0.f;

			light.Position = pInfo->pLight->Position + v_add;

			rvector lightmapcolor(1, 1, 1);

			light.Diffuse.r = pInfo->pLight->Color.x*pInfo->pLight->fIntensity;
			light.Diffuse.g = pInfo->pLight->Color.y*pInfo->pLight->fIntensity;
			light.Diffuse.b = pInfo->pLight->Color.z*pInfo->pLight->fIntensity;

			light.Range = pInfo->pLight->fAttnEnd;

			RGetDevice()->SetLight(0, &light);
			RGetDevice()->LightEnable(0, TRUE);
			RGetDevice()->LightEnable(1, FALSE);
		}

		pInfo->pVisualMesh->SetWorldMatrix(world);
		pInfo->pVisualMesh->Frame();
		pInfo->pVisualMesh->Render(&m_OcclusionList);

		if (!pInfo->pVisualMesh->m_bIsRender) m_DebugInfo.nMapObjectFrustumCulled++;
	}
}

void RBspObject::DrawBoundingBox()
{
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetTexture(1, NULL);
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	OcRoot[0].DrawBoundingBox(0xffffff);
}

void RBspObject::DrawOcclusions()
{
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetTexture(1, NULL);
	RGetDevice()->SetRenderState(D3DRS_ZENABLE, false);
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	for (ROcclusionList::iterator i = m_OcclusionList.begin(); i != m_OcclusionList.end(); i++)
	{
		ROcclusion *poc = *i;

		for (int j = 0; j < poc->nCount; j++)
		{
			RDrawLine(poc->pVertices[j], poc->pVertices[(j + 1) % poc->nCount], 0xffff00ff);
		}
	}

	RSetWBuffer(true);
}

#ifndef _PUBLISH
void RBspObject::DrawColNodePolygon(const rvector &pos)
{
	ColRoot[0].DrawPos(pos);
}

void RBspObject::DrawCollision_Polygon()
{
	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();

	pd3dDevice->SetFVF(D3DFVF_XYZ);
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0x406fa867);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	ColRoot[0].DrawPolygon();

	RSetWBuffer(true);
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ffffff);
	ColRoot[0].DrawPolygonWireframe();

	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ff00ff);
	ColRoot[0].DrawPolygonNormal();

	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

}

void RBspObject::DrawCollision_Solid()
{
	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();

	pd3dDevice->SetFVF(D3DFVF_XYZ);
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40808080);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	ColRoot[0].DrawSolidPolygon();

	RSetWBuffer(true);
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ffffff);
	ColRoot[0].DrawSolidPolygonWireframe();

	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ff00ff);
	ColRoot[0].DrawSolidPolygonNormal();

	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

}

void RBspObject::DrawNavi_Polygon()
{
	m_NavigationMesh.Render();
}

void RBspObject::DrawNavi_Links()
{
	m_NavigationMesh.RenderLinks();
}

#else
void RBspObject::DrawColNodePolygon(const rvector &pos) {}
void RBspObject::DrawCollision_Polygon() {}
void RBspObject::DrawCollision_Solid() {}
void RBspObject::DrawNavi_Polygon() {}
void RBspObject::DrawNavi_Links() {}
#endif

bool isInViewFrustumWithFarZ(rboundingbox *bb, rplane *plane)
{
	if (isInPlane(bb, plane) && isInPlane(bb, plane + 1) &&
		isInPlane(bb, plane + 2) && isInPlane(bb, plane + 3) &&
		isInPlane(bb, plane + 5)) return true;
	return false;
}

void RBspObject::ChooseNodes(RSBspNode *bspNode)
{
	if (isInViewFrustumWithFarZ(&bspNode->bbTree, m_localViewFrustum))
	{
		if (bspNode->nPolygon)
		{
			if (!IsVisible(bspNode->bbTree)) return;
		}

		bspNode->nFrameCount = g_nFrameNumber;
		if (bspNode->m_pNegative)
			ChooseNodes(bspNode->m_pNegative);
		if (bspNode->m_pPositive)
			ChooseNodes(bspNode->m_pPositive);
	}
}

int RBspObject::ChooseNodes(RSBspNode *bspNode, const rvector &center, float fRadius)
{
	if (bspNode == NULL) return 0;

	if (IsInSphere(bspNode->bbTree, center, fRadius))
		bspNode->nFrameCount = g_nFrameNumber;

	if (!bspNode->nPolygon)
	{
		int nm_pNegative = ChooseNodes(bspNode->m_pNegative, center, fRadius);
		int nm_pPositive = ChooseNodes(bspNode->m_pPositive, center, fRadius);

		return 1 + nm_pNegative + nm_pPositive;
	}
	return 0;
}

bool RBspObject::ReadString(MZFile *pfile, char *buffer, int nBufferSize)
{
	int nCount = 0;
	do {
		pfile->Read(buffer, 1);
		nCount++;
		buffer++;
		if (nCount >= nBufferSize)
			return false;
	} while ((*(buffer - 1)) != 0);

	return true;
}

static void DeleteVoidNodes(RSBspNode *pNode)
{
	if (pNode->m_pPositive)
		DeleteVoidNodes(pNode->m_pPositive);

	if (pNode->m_pNegative)
		DeleteVoidNodes(pNode->m_pNegative);

	if (pNode->m_pPositive && !pNode->m_pPositive->nPolygon &&
		!pNode->m_pPositive->m_pPositive && !pNode->m_pPositive->m_pNegative)
		SAFE_DELETE(pNode->m_pPositive);

	if (pNode->m_pNegative && !pNode->m_pNegative->nPolygon &&
		!pNode->m_pNegative->m_pPositive && !pNode->m_pNegative->m_pNegative)
		SAFE_DELETE(pNode->m_pNegative);
}

static void RecalcBoundingBox(RSBspNode *pNode)
{
	if (pNode->nPolygon)
	{
		rboundingbox *bb = &pNode->bbTree;
		bb->vmin.x = bb->vmin.y = bb->vmin.z = FLT_MAX;
		bb->vmax.x = bb->vmax.y = bb->vmax.z = -FLT_MAX;
		for (int i = 0; i < pNode->nPolygon; i++)
		{
			RPOLYGONINFO *pInfo = &pNode->pInfo[i];

			for (int j = 0; j < pInfo->nVertices; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					bb->vmin[k] = min(bb->vmin[k], (*pInfo->pVertices[j].Coord())[k]);
					bb->vmax[k] = max(bb->vmax[k], (*pInfo->pVertices[j].Coord())[k]);
				}
			}
		}
	}
	else
	{
		if (pNode->m_pPositive)
		{
			RecalcBoundingBox(pNode->m_pPositive);
			memcpy(&pNode->bbTree, &pNode->m_pPositive->bbTree, sizeof(rboundingbox));
		}
		if (pNode->m_pNegative)
		{
			RecalcBoundingBox(pNode->m_pNegative);
			memcpy(&pNode->bbTree, &pNode->m_pNegative->bbTree, sizeof(rboundingbox));
		}
		if (pNode->m_pPositive) MergeBoundingBox(&pNode->bbTree, &pNode->m_pPositive->bbTree);
		if (pNode->m_pNegative) MergeBoundingBox(&pNode->bbTree, &pNode->m_pNegative->bbTree);
	}
}

bool RBspObject::Open(const char *filename, ROpenMode nOpenFlag, RFPROGRESSCALLBACK pfnProgressCallback,
	void *CallbackParam, bool PhysOnly)
{
	this->PhysOnly = PhysOnly;
	m_OpenMode = nOpenFlag;
	m_filename = filename;

	char xmlname[_MAX_PATH];
	sprintf_safe(xmlname, "%s.xml", filename);

	if (!OpenDescription(xmlname))
	{
		MLog("Error while loading %s\n", xmlname);
		return false;
	}
	if (pfnProgressCallback) pfnProgressCallback(CallbackParam, .3f);

	if (!OpenRs(filename))
	{
		MLog("Error while loading %s\n", filename);
		return false;
	}

	if (pfnProgressCallback) pfnProgressCallback(CallbackParam, .6f);

	char bspname[_MAX_PATH];
	sprintf_safe(bspname, "%s.bsp", filename);
	if (!OpenBsp(bspname))
	{
		MLog("Error while loading %s\n", bspname);
		return false;
	}

	if (pfnProgressCallback) pfnProgressCallback(CallbackParam, .8f);

	char colfilename[_MAX_PATH];
	sprintf_safe(colfilename, "%s.col", filename);
	if (!OpenCol(colfilename))
	{
		MLog("Error while loading %s\n", colfilename);
		return false;
	}

	char navfilename[_MAX_PATH];
	sprintf_safe(navfilename, "%s.nav", filename);
	if (!OpenNav(navfilename))
	{
	}

	if (!PhysOnly && GetRS2().UsingD3D9())
	{
		if (RIsHardwareTNL())
		{
			if (!CreateVertexBuffer())
				mlog("Error while Creating VB\n");
		}

		if (m_bisDrawLightMap)
			OpenLightmap();
	}

	Sort_Nodes(OcRoot.data());

	if (RIsHardwareTNL() && !PhysOnly && GetRS2().UsingD3D9())
	{
		UpdateVertexBuffer();
	}

	CreatePolygonTable();

	if (RIsHardwareTNL() && !PhysOnly && GetRS2().UsingD3D9())
	{
		if (!CreateIndexBuffer())
			mlog("Error while Creating IB\n");

		UpdateIndexBuffer();
	}

	if (pfnProgressCallback) pfnProgressCallback(CallbackParam, 1.f);

	if (!PhysOnly && GetRS2().UsingVulkan())
	{
		DrawObj.InitStatic<RBspObjectDrawVulkan>();
	}

	return true;
}

void RBspObject::OptimizeBoundingBox()
{
	DeleteVoidNodes(OcRoot.data());
	RecalcBoundingBox(OcRoot.data());
}

bool RBspObject::CreateIndexBuffer()
{
	SAFE_RELEASE(IndexBuffer);

	HRESULT hr = RGetDevice()->CreateIndexBuffer(sizeof(OcIndices[0]) * OcIndices.size(), D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, MakeWriteProxy(IndexBuffer), NULL);
	if (FAILED(hr)) {
		mlog("RBspObject::CreateIndexBuffer failed\n");
		return false;
	}

	return true;
}

bool RBspObject::UpdateIndexBuffer()
{
	if (!IndexBuffer) return false;

	void* pIndices = nullptr;
	auto hr = IndexBuffer->Lock(0, 0, &pIndices, 0);
	if (FAILED(hr)) goto fail;
	memcpy(pIndices, OcIndices.data(), sizeof(OcIndices[0]) * OcIndices.size());
	hr = IndexBuffer->Unlock();
	if (FAILED(hr)) goto fail;

	return true;

fail:
	MLog("RBspObject::UpdateIndexBuffer failed\n");
	return false;
}

void RBspObject::OnInvalidate()
{
	InvalidateDynamicLightVertexBuffer();
}

void RBspObject::OnRestore()
{
	if (!m_bisDrawLightMap)
		Sort_Nodes(OcRoot.data());
}

bool RBspObject::Open_MaterialList(MXmlElement *pElement)
{
	RMaterialList ml;
	ml.Open(pElement);

	m_nMaterial = ml.size();
	m_nMaterial = m_nMaterial + 1;

	Materials.resize(m_nMaterial);

	Materials[0].texture = NULL;
	Materials[0].Diffuse = rvector(1, 1, 1);
	Materials[0].dwFlags = 0;

	string strBase = m_filename;
	string::size_type nPos = strBase.find_last_of("\\"), nothing = -1;
	if (nPos == nothing)
		nPos = strBase.find_last_of("/");

	if (nPos == nothing)
		strBase = "";
	else
		strBase = strBase.substr(0, nPos) + "/";



	RMaterialList::iterator itor;
	itor = ml.begin();

	for (int i = 1; i < m_nMaterial; i++)
	{
		RMATERIAL *mat = *itor;

		Materials[i].dwFlags = mat->dwFlags;
		Materials[i].Diffuse = mat->Diffuse;
		Materials[i].Specular = mat->Specular;
		Materials[i].Ambient = mat->Ambient;
		Materials[i].Name = mat->Name;
		Materials[i].DiffuseMap = mat->DiffuseMap;

		string DiffuseMapName = strBase + mat->DiffuseMap;
		char szMapName[256];
		GetRefineFilename(szMapName, DiffuseMapName.c_str());

		if (!PhysOnly && szMapName[0])
		{
			if (GetRS2().UsingD3D9())
				Materials[i].texture = RCreateBaseTexture(szMapName, RTextureType_Map, true);
			else
			{
				Materials[i].VkMaterial.Texture = GetRS2Vulkan().LoadTexture(szMapName);
			}
		}

		itor++;
	}

	return true;
}


bool RBspObject::Open_LightList(MXmlElement *pElement)
{
	RLightList llist;
	llist.Open(pElement);

	for (auto& Light : llist)
	{
		if (_strnicmp(Light.Name.c_str(), RTOK_MAX_OBJLIGHT, strlen(RTOK_MAX_OBJLIGHT)) == 0) {
			m_StaticObjectLightList.push_back(Light);
		}
		else {
			m_StaticMapLightList.push_back(Light);

			if (_strnicmp(Light.Name.c_str(), "sun_omni", 8) == 0)
			{
				RLIGHT Sunlight;
				Sunlight.Name = Light.Name;
				Sunlight.dwFlags = Light.dwFlags;
				Sunlight.fAttnEnd = Light.fAttnEnd;
				Sunlight.fAttnStart = Light.fAttnStart;
				Sunlight.fIntensity = Light.fIntensity;
				Sunlight.Position = Light.Position;
				Sunlight.Color = Light.Color;
				m_StaticSunLightList.push_back(Sunlight);
			}
		}
	}

	return true;
}

bool RBspObject::Open_OcclusionList(MXmlElement *pElement)
{
	m_OcclusionList.Open(pElement);

	for (ROcclusionList::iterator i = m_OcclusionList.begin(); i != m_OcclusionList.end(); i++)
	{
		ROcclusion *poc = *i;
		poc->CalcPlane();
		poc->pPlanes = new rplane[poc->nCount + 1];
	}

	return true;
}

bool RBspObject::Open_ObjectList(MXmlElement *pElement)
{
	return true;
	int i;

	MXmlElement	aObjectNode, aChild;
	int nCount = pElement->GetChildNodeCount();

	char szTagName[256], szContents[256];
	for (i = 0; i < nCount; i++)
	{
		aObjectNode = pElement->GetChildNode(i);
		aObjectNode.GetTagName(szTagName);

		if (_stricmp(szTagName, RTOK_OBJECT) == 0)
		{
			ROBJECTINFO *pInfo = new ROBJECTINFO;
			aObjectNode.GetAttribute(szContents, RTOK_NAME);
			pInfo->name = szContents;

			char fname[_MAX_PATH];
			GetPurePath(fname, m_descfilename.c_str());
			strcat(fname, szContents);

			m_MeshList.SetMtrlAutoLoad(true);
			m_MeshList.SetMapObject(true);

			pInfo->nMeshID = m_MeshList.Add(fname);
			RMesh *pmesh = m_MeshList.GetFast(pInfo->nMeshID);

			if (pmesh)
			{
				auto* pName = pmesh->GetFileName();
				while (pName[0] != '\0')
				{
					if (pName[0] == 'o' && pName[1] == 'b') break;
					++pName;
				}

				if (strncmp(pName, "obj_water", 9) != 0 && strncmp(pName, "obj_flag", 8) != 0)
				{
					strcat(fname, ".ani");

					m_AniList.Add(fname, fname, i);
					RAnimation* pAni = m_AniList.GetAnimation(fname);

					pmesh->SetAnimation(pAni);

					pInfo->pVisualMesh = new RVisualMesh;
					pInfo->pVisualMesh->Create(pmesh);
					pInfo->pVisualMesh->SetAnimation(pAni);
				}
				else
				{
					pInfo->pVisualMesh = 0;
				}

				m_ObjectList.push_back(pInfo);
				pInfo->pLight = NULL;
			}
			else
				delete pInfo;
		}
	}

	for (auto* pInfo : m_ObjectList)
	{
		if (pInfo == NULL) {

			mlog("RBspObject::Open_ObjectList : pInfo == NULL pVisualMesh->CalcBox\n");
			continue;
		}
		else {
			if (pInfo->pVisualMesh == NULL) {
				mlog("RBspObject::Open_ObjectList : pInfo->pVisualMesh == NULL \n");
				continue;
			}
		}

		pInfo->pVisualMesh->CalcBox();

		rvector center = (pInfo->pVisualMesh->m_vBMax + pInfo->pVisualMesh->m_vBMin)*.5f;

		auto& LightList = GetObjectLightList();

		float fIntensityFirst = FLT_MIN;
		for (auto& Light : LightList)
		{
			float fdistance = Magnitude(Light.Position - center);

			float fDist = Light.fAttnEnd - Light.fAttnStart;
			float fIntensity;
			if (fDist == 0) fIntensity = 0;
			else fIntensity = (fdistance - Light.fAttnStart) / (Light.fAttnEnd - Light.fAttnStart);

			fIntensity = min(max(1.0f - fIntensity, 0.f), 1.f);
			fIntensity *= Light.fIntensity;
			fIntensity = min(max(fIntensity, 0.f), 1.f);

			if (fIntensityFirst < fIntensity) {
				fIntensityFirst = fIntensity;
				pInfo->pLight = &Light;
			}
		}
	}

	return true;
}

bool RBspObject::Make_LenzFalreList()
{
	for (RDummyList::iterator itor = m_DummyList.begin(); itor != m_DummyList.end(); ++itor)
	{
		RDummy* pDummy = *itor;

		if (_stricmp(pDummy->szName.c_str(), "sun_dummy") == 0)
		{
			if (!RGetLenzFlare()->SetLight(pDummy->Position))
			{
				mlog("Fail to Set LenzFlare Position...\n");
			}

			return true;
		}
	}

	return true;
}

bool RBspObject::Open_DummyList(MXmlElement *pElement)
{
	m_DummyList.Open(pElement);

	Make_LenzFalreList();
	return true;
}

bool RBspObject::Set_Fog(MXmlElement *pElement)
{
	MXmlElement childElem;
	DWORD dwColor = 0;
	int color;
	char name[8];

	m_FogInfo.bFogEnable = true;
	pElement->GetAttribute(&m_FogInfo.fNear, "min");
	pElement->GetAttribute(&m_FogInfo.fFar, "max");
	int nChild = pElement->GetChildNodeCount();
	for (int i = 0; i < nChild; ++i)
	{
		childElem = pElement->GetChildNode(i);
		childElem.GetContents(&color);
		childElem.GetNodeName(name);
		if (name[0] == 'R')	dwColor |= (color << 16);
		else if (name[0] == 'G') dwColor |= (color << 8);
		else if (name[0] == 'B') dwColor |= (color);
	}
	m_FogInfo.dwFogColor = dwColor;
	return true;
}

bool RBspObject::Set_AmbSound(MXmlElement *pElement)
{
	MXmlElement childElem, contentElem;
	char szBuffer[128];

	int nChild = pElement->GetChildNodeCount();
	for (int i = 0; i < nChild; ++i)
	{
		childElem = pElement->GetChildNode(i);
		childElem.GetTagName(szBuffer);
		if (_stricmp(szBuffer, "AMBIENTSOUND") == 0)
		{
			AmbSndInfo asinfo;
			asinfo.itype = 0;


			childElem.GetAttribute(szBuffer, "type");
			if (szBuffer[0] == 'a')
				asinfo.itype |= AS_2D;
			else if (szBuffer[0] == 'b')
				asinfo.itype |= AS_3D;

			if (szBuffer[1] == '0')
				asinfo.itype |= AS_AABB;
			else if (szBuffer[1] == '1')
				asinfo.itype |= AS_SPHERE;

			childElem.GetAttribute(asinfo.szSoundName, "filename");

			int nContents = childElem.GetChildNodeCount();
			for (int j = 0; j < nContents; ++j)
			{
				char* token = NULL;
				contentElem = childElem.GetChildNode(j);
				contentElem.GetNodeName(szBuffer);
				if (_stricmp("MIN_POSITION", szBuffer) == 0)
				{
					contentElem.GetContents(szBuffer);
					token = strtok(szBuffer, " ");
					if (token != NULL) asinfo.min.x = atof(token);
					token = strtok(NULL, " ");
					if (token != NULL) asinfo.min.y = atof(token);
					token = strtok(NULL, " ");
					if (token != NULL) asinfo.min.z = atof(token);
				}
				else if (_stricmp("MAX_POSITION", szBuffer) == 0)
				{
					contentElem.GetContents(szBuffer);
					token = strtok(szBuffer, " ");
					if (token != NULL) asinfo.max.x = atof(token);
					token = strtok(NULL, " ");
					if (token != NULL) asinfo.max.y = atof(token);
					token = strtok(NULL, " ");
					if (token != NULL) asinfo.max.z = atof(token);
				}
				else if (_stricmp("RADIUS", szBuffer) == 0)
				{
					contentElem.GetContents(szBuffer);
					asinfo.radius = atof(szBuffer);
				}
				else if (_stricmp("CENTER", szBuffer) == 0)
				{
					contentElem.GetContents(szBuffer);
					token = strtok(szBuffer, " ");
					if (token != NULL) asinfo.center.x = atof(token);
					token = strtok(NULL, " ");
					if (token != NULL) asinfo.center.y = atof(token);
					token = strtok(NULL, " ");
					if (token != NULL) asinfo.center.z = atof(token);
				}
			}
			AmbSndInfoList.push_back(asinfo);
		}
	}

	return true;
}

bool RBspObject::OpenDescription(const char *filename)
{
	MZFile mzf;
	if (!mzf.Open(filename, g_pFileSystem))
		return false;

	m_descfilename = filename;

	char *buffer;
	buffer = new char[mzf.GetLength() + 1];
	mzf.Read(buffer, mzf.GetLength());
	buffer[mzf.GetLength()] = 0;

	MXmlDocument aXml;
	aXml.Create();
	if (!aXml.LoadFromMemory(buffer))
	{
		delete buffer;
		return false;
	}

	int iCount;
	MXmlElement aParent, aChild;
	aParent = aXml.GetDocumentElement();
	iCount = aParent.GetChildNodeCount();

	char szTagName[256];
	for (int i = 0; i < iCount; i++)
	{
		aChild = aParent.GetChildNode(i);
		aChild.GetTagName(szTagName);
		if (_stricmp(szTagName, RTOK_MATERIALLIST) == 0)
			Open_MaterialList(&aChild);
		else if (!PhysOnly)
		{
			if (_stricmp(szTagName, RTOK_LIGHTLIST) == 0)
				Open_LightList(&aChild);
			else if (_stricmp(szTagName, RTOK_OBJECTLIST) == 0)
				Open_ObjectList(&aChild);
			else if (_stricmp(szTagName, RTOK_OCCLUSIONLIST) == 0)
				Open_OcclusionList(&aChild);
			else if (_stricmp(szTagName, RTOK_DUMMYLIST) == 0)
				Open_DummyList(&aChild);
			else if (_stricmp(szTagName, RTOK_FOG) == 0)
				Set_Fog(&aChild);
			else if (_stricmp(szTagName, "AMBIENTSOUNDLIST") == 0)
				Set_AmbSound(&aChild);
		}
	}

	delete buffer;
	mzf.Close();

	return true;
}

bool RBspObject::OpenRs(const char *filename)
{
	MZFile file;
	if (!file.Open(filename, g_pFileSystem))
		return false;

	RHEADER header;
	file.Read(&header, sizeof(RHEADER));
	if (header.dwID != RS_ID || header.dwVersion != RS_VERSION)
	{
		mlog("%s : %d , %d version required.\n", filename, header.dwVersion, RS_VERSION);
		file.Close();
		return false;
	}

	// read material indices
	int nMaterial;
	file.Read(&nMaterial, sizeof(int));

	if (!PhysOnly)
	{
		if (m_nMaterial - 1 != nMaterial)
			return false;
	}
	else
	{
		m_nMaterial = nMaterial + 1;
	}

	for (int i = 1; i < m_nMaterial; i++)
	{
		char buf[256];
		if (!ReadString(&file, buf, sizeof(buf)))
			return false;
	}

	Open_ConvexPolygons(&file);

	file.Read(&m_nBspNodeCount, sizeof(int));
	file.Read(&m_nBspPolygon, sizeof(int));
	file.Read(&m_nBspVertices, sizeof(int));
	file.Read(&m_nBspIndices, sizeof(int));

	file.Read(&m_nNodeCount, sizeof(int));
	file.Read(&m_nPolygon, sizeof(int));
	int NumVertices = 0;
	file.Read(&NumVertices, sizeof(int));
	int NumIndices = 0;
	file.Read(&NumIndices, sizeof(int));

	OcRoot.resize(m_nNodeCount);
	OcInfo.resize(m_nPolygon);
	OcVertices.resize(NumVertices);
	OcIndices.resize(NumIndices);
	OcNormalVertices.resize(NumVertices);

	auto ret = Open_Nodes(OcRoot.data(), &file,
		{ OcVertices.data(), OcRoot.data(), OcInfo.data(),
		RenderWithNormal ? OcNormalVertices.data() : nullptr });

	assert(ret.Vertices <= OcVertices.data() + OcVertices.size());
	assert(ret.Node <= OcRoot.data() + OcRoot.size());
	assert(ret.Info <= OcInfo.data() + OcInfo.size());

	return true;
}

bool RBspObject::OpenBsp(const char *filename)
{
	MZFile file;
	if (!file.Open(filename, g_pFileSystem))
		return false;

	RHEADER header;
	file.Read(&header, sizeof(RHEADER));
	if (header.dwID != RBSP_ID || header.dwVersion != RBSP_VERSION)
	{
		file.Close();
		return false;
	}

	int nBspNodeCount, nBspPolygon, nBspVertices, nBspIndices;
	// Read tree information
	file.Read(&nBspNodeCount, sizeof(int));
	file.Read(&nBspPolygon, sizeof(int));
	file.Read(&nBspVertices, sizeof(int));
	file.Read(&nBspIndices, sizeof(int));

	if (m_nBspNodeCount != nBspNodeCount || m_nBspPolygon != nBspPolygon ||
		m_nBspVertices != nBspVertices || m_nBspIndices != nBspIndices)
	{
		file.Close();
		return false;
	}

	BspVertices.resize(m_nBspVertices);
	BspRoot.resize(m_nBspNodeCount);
	BspInfo.resize(m_nBspPolygon);

	auto ret = Open_Nodes(BspRoot.data(), &file,
		{ BspVertices.data(), BspRoot.data(), BspInfo.data() });

	assert(ret.Vertices <= BspVertices.data() + BspVertices.size());
	assert(ret.Node <= BspRoot.data() + BspRoot.size());
	assert(ret.Info <= BspInfo.data() + BspInfo.size());

	file.Close();
	return true;
}

int RBspObject::Open_ColNodes(RSolidBspNode *pNode, MZFile *pfile, int Depth)
{
	pfile->Read(&pNode->m_Plane, sizeof(rplane));
	pfile->Read(&pNode->m_bSolid, sizeof(bool));

	int OrigDepth = Depth;

	auto Open = [&](auto Branch)
	{
		bool flag;
		pfile->Read(&flag, sizeof(bool));
		if (!flag) return;
		++Depth;
		pNode->*Branch = &ColRoot[Depth];
		Depth += Open_ColNodes(pNode->*Branch, pfile, Depth);
	};

	Open(&RSolidBspNode::m_pPositive);
	Open(&RSolidBspNode::m_pNegative);

	int nPolygon;
	pfile->Read(&nPolygon, sizeof(int));

#ifndef _PUBLISH
	pNode->nPolygon = nPolygon;
	if (pNode->nPolygon)
	{
		pNode->pVertices = &ColVertices[Depth * 3];
		pNode->pNormals = new rvector[pNode->nPolygon];
		for (int i = 0; i < pNode->nPolygon; i++)
		{
			pfile->Read(pNode->pVertices + i * 3, sizeof(rvector) * 3);
			pfile->Read(pNode->pNormals + i, sizeof(rvector));
		}
	}
#else
	pfile->Seek(nPolygon * 4 * sizeof(rvector), MZFile::current);
#endif

	return Depth - OrigDepth;
}

bool RBspObject::OpenCol(const char *filename)
{
	MZFile file;
	if (!file.Open(filename, g_pFileSystem))
		return false;

	RHEADER header;
	file.Read(&header, sizeof(RHEADER));
	if (header.dwID != R_COL_ID || header.dwVersion != R_COL_VERSION)
	{
		file.Close();
		return false;
	}

	int nBspNodeCount, nBspPolygon;
	// read tree information
	file.Read(&nBspNodeCount, sizeof(int));
	file.Read(&nBspPolygon, sizeof(int));

	ColRoot.resize(nBspNodeCount);
	ColVertices.resize(nBspPolygon * 3);
	auto ret = Open_ColNodes(ColRoot.data(), &file);
	assert(nBspNodeCount == ret + 1);

	file.Close();
#ifndef _PUBLISH	
	ColRoot[0].ConstructBoundingBox();
#endif
	return true;
}

bool RBspObject::OpenNav(const char *filename)
{
	return m_NavigationMesh.Open(filename, g_pFileSystem);
}

bool SaveMemoryBmp(int x, int y, void *data, void **retmemory, int *nsize);

bool RBspObject::OpenLightmap()
{
	char lightmapinfofilename[_MAX_PATH];
	sprintf_safe(lightmapinfofilename, "%s.lm", m_filename.c_str());

	MZFile file;
	if (!file.Open(lightmapinfofilename, g_pFileSystem)) return false;

	RHEADER header;
	file.Read(&header, sizeof(RHEADER));
	if (header.dwID != R_LM_ID || header.dwVersion != R_LM_VERSION)
	{
		file.Close();
		return false;
	}

	int nSourcePolygon, nNodeCount;

	file.Read(&nSourcePolygon, sizeof(int));
	file.Read(&nNodeCount, sizeof(int));

	if (nSourcePolygon != m_nConvexPolygon || m_nNodeCount != nNodeCount)
	{
		file.Close();
		return false;
	}

	file.Read(&m_nLightmap, sizeof(int));
	LightmapTextures.resize(m_nLightmap);

	std::vector<u8> bmpmemory;
	for (int i = 0; i < m_nLightmap; i++)
	{
		int nBmpSize;
		file.Read(&nBmpSize, sizeof(int));

		bmpmemory.resize(nBmpSize);
		file.Read(&bmpmemory[0], nBmpSize);

		HRESULT hr = D3DXCreateTextureFromFileInMemoryEx(
			RGetDevice(), &bmpmemory[0], nBmpSize,
			D3DX_DEFAULT, D3DX_DEFAULT,
			D3DX_DEFAULT,
			0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
			D3DX_FILTER_TRIANGLE | D3DX_FILTER_MIRROR,
			D3DX_FILTER_TRIANGLE | D3DX_FILTER_MIRROR,
			0, NULL, NULL, MakeWriteProxy(LightmapTextures[i]));

		if (FAILED(hr))
			mlog("Failed to load lightmap texture! Error code = %d, error message = %s\n",
				hr, DXGetErrorString9(hr));
	}

	for (int i = 0; i < m_nPolygon; i++)
	{
		int a;
		file.Read(&a, sizeof(int));
	}

	for (int i = 0; i < m_nPolygon; i++)
		file.Read(&OcInfo[i].nLightmapTexture, sizeof(int));

	for (size_t i = 0; i < OcVertices.size(); i++)
		file.Read(&OcVertices[i].tu2, sizeof(float) * 2);

	file.Close();

	return true;
}

bool RBspObject::Open_ConvexPolygons(MZFile *pfile)
{
	int nConvexVertices;

	pfile->Read(&m_nConvexPolygon, sizeof(int));
	pfile->Read(&nConvexVertices, sizeof(int));

	if (m_OpenMode == ROpenMode::Runtime) {
		for (int i = 0; i < m_nConvexPolygon; i++)
		{
			pfile->Seek(sizeof(int) + sizeof(DWORD) + sizeof(rplane) + sizeof(float), MZFile::current);

			int nVertices;
			pfile->Read(&nVertices, sizeof(int));

			pfile->Seek(nVertices * 2 * sizeof(rvector), MZFile::current);
		}
		return true;
	}

	ConvexPolygons.resize(m_nConvexPolygon);
	ConvexVertices.resize(nConvexVertices);
	ConvexNormals.resize(nConvexVertices);

	rvector *pLoadingVertex = ConvexVertices.data();
	rvector *pLoadingNormal = ConvexNormals.data();

	for (int i = 0; i < m_nConvexPolygon; i++)
	{
		pfile->Read(&ConvexPolygons[i].nMaterial, sizeof(int));
		ConvexPolygons[i].nMaterial += 2;
		pfile->Read(&ConvexPolygons[i].dwFlags, sizeof(DWORD));
		pfile->Read(&ConvexPolygons[i].plane, sizeof(rplane));
		pfile->Read(&ConvexPolygons[i].fArea, sizeof(float));
		pfile->Read(&ConvexPolygons[i].nVertices, sizeof(int));

		ConvexPolygons[i].pVertices = pLoadingVertex;
		for (int j = 0; j < ConvexPolygons[i].nVertices; j++)
		{
			pfile->Read(pLoadingVertex, sizeof(rvector));
			pLoadingVertex++;
		}
		ConvexPolygons[i].pNormals = pLoadingNormal;
		for (int j = 0; j < ConvexPolygons[i].nVertices; j++)
		{
			pfile->Read(pLoadingNormal, sizeof(rvector));
			pLoadingNormal++;
		}
	}
	return true;
}

void RBspObject::CreatePolygonTable()
{
	auto* Indices = OcIndices.data();
	CreatePolygonTable(OcRoot.data(), &Indices);
}

void RBspObject::CreatePolygonTable(RSBspNode *pNode, u16** Indices)
{
	if (pNode->m_pPositive)
		CreatePolygonTable(pNode->m_pPositive, Indices);

	if (pNode->m_pNegative)
		CreatePolygonTable(pNode->m_pNegative, Indices);

	if (pNode->nPolygon)
	{
		for (int i = 0; i < pNode->nPolygon; i++)
		{
			auto* pInd = *Indices;
			RPOLYGONINFO *pInfo = &pNode->pInfo[i];

			pInfo->nIndicesPos = *Indices - OcIndices.data();
			if (RIsHardwareTNL())
			{
				WORD base = pInfo->pVertices - OcVertices.data();
				for (int j = 0; j < pInfo->nVertices - 2; j++)
				{
					*pInd++ = base + 0;
					*pInd++ = base + j + 1;
					*pInd++ = base + j + 2;
				}
			}
			*Indices += (pInfo->nVertices - 2) * 3;
		}

		int nCount = m_nMaterial*max(1, m_nLightmap);

		SAFE_DELETE_ARRAY(pNode->pDrawInfo);
		pNode->pDrawInfo = new RDrawInfo[nCount];

		int lastmatind = pNode->pInfo[0].nIndicesPos;
		int lastmat = pNode->pInfo[0].nMaterial + pNode->pInfo[0].nLightmapTexture*m_nMaterial;
		int nTriangle = pNode->pInfo[0].nVertices - 2;
		int lastj = 0;

		for (int j = 1; j < pNode->nPolygon + 1; j++)
		{
			int nMatIndex = (j == pNode->nPolygon) ? -999 :
				pNode->pInfo[j].nMaterial + pNode->pInfo[j].nLightmapTexture*m_nMaterial;

			if (nMatIndex != lastmat)
			{
				RDrawInfo *pdi = &pNode->pDrawInfo[lastmat];

				if (lastmat != -1 && lastmat >= 0 && lastmat < nCount)
				{
					pdi->nIndicesOffset = lastmatind;
					pdi->nTriangleCount = nTriangle;
					pdi->pPlanes = new rplane[nTriangle];
					pdi->pUAxis = new rvector[nTriangle];
					pdi->pVAxis = new rvector[nTriangle];

					int nPlaneIndex = 0;
					for (int k = lastj; k < j; k++)
					{
						for (int l = 0; l < pNode->pInfo[k].nVertices - 2; l++)
						{
							rplane *pPlane = &pNode->pInfo[k].plane;
							pdi->pPlanes[nPlaneIndex] = *pPlane;
							rvector normal = rvector(pPlane->a, pPlane->b, pPlane->c);

							rvector up;
							if (fabs(DotProduct(normal, rvector(1, 0, 0))) < 0.01) {
								up = rvector(1, 0, 0);
							}
							else
								up = rvector(0, 1, 0);

							rvector au;
							CrossProduct(&au, up, normal);
							Normalize(au);
							rvector av;
							CrossProduct(&av, au, normal);
							Normalize(av);

							pdi->pUAxis[nPlaneIndex] = au;
							pdi->pVAxis[nPlaneIndex] = av;

							nPlaneIndex++;
						}
					}
					_ASSERT(nPlaneIndex == pdi->nTriangleCount);

					if (!RIsHardwareTNL())
					{
						for (int k = lastj; k < j; k++)
						{
							pdi->nVertice += pNode->pInfo[k].nVertices;
						}

						pdi->pVertices = new BSPVERTEX[pdi->nVertice];

						WORD base = 0;
						for (int k = lastj; k < j; k++)
						{
							memcpy(pdi->pVertices + base, pNode->pInfo[k].pVertices, sizeof(BSPVERTEX)*pNode->pInfo[k].nVertices);

							WORD *pInd = OcIndices.data() + pNode->pInfo[k].nIndicesPos;
							for (int l = 0; l < pNode->pInfo[k].nVertices - 2; l++)
							{
								*pInd++ = base + 0;
								*pInd++ = base + l + 1;
								*pInd++ = base + l + 2;
							}
							base += pNode->pInfo[k].nVertices;
						}
					}

					if (j == pNode->nPolygon) break;

				}
				else {
					_ASSERT(FALSE);
				}
				lastmat = nMatIndex;
				lastmatind = pNode->pInfo[j].nIndicesPos;
				nTriangle = 0;
				lastj = j;
			}
			nTriangle += pNode->pInfo[j].nVertices - 2;
		}
	}
}

void RBspObject::Sort_Nodes(RSBspNode *pNode)
{
	if (pNode->m_pPositive)
		Sort_Nodes(pNode->m_pPositive);

	if (pNode->m_pNegative)
		Sort_Nodes(pNode->m_pNegative);

	if (pNode->nPolygon)
	{
		for (int j = 0; j<pNode->nPolygon - 1; j++)
		{
			for (int k = j + 1; k < pNode->nPolygon; k++)
			{
				RPOLYGONINFO *pj = pNode->pInfo + j, *pk = pNode->pInfo + k;

				if (pj->nLightmapTexture > pk->nLightmapTexture
					|| (pj->nLightmapTexture == pk->nLightmapTexture
						&& pNode->pInfo[j].nMaterial > pNode->pInfo[k].nMaterial))
				{
					RPOLYGONINFO ttemp;
					memcpy(&ttemp, pNode->pInfo + j, sizeof(ttemp));
					memcpy(pNode->pInfo + j, pNode->pInfo + k, sizeof(ttemp));
					memcpy(pNode->pInfo + k, &ttemp, sizeof(ttemp));
				}
			}
		}
	}
}

OpenNodesState RBspObject::Open_Nodes(RSBspNode *pNode, MZFile *pfile, OpenNodesState State)
{
	int OrigPolygonID = 0;

	pfile->Read(&pNode->bbTree, sizeof(rboundingbox));
	pfile->Read(&pNode->plane, sizeof(rplane));

	auto Open = [&](auto Branch)
	{
		bool flag;
		pfile->Read(&flag, sizeof(bool));
		if (!flag) return;
		++State.Node;
		pNode->*Branch = State.Node;
		State = Open_Nodes(pNode->*Branch, pfile, State);
	};

	Open(&RSBspNode::m_pPositive);
	Open(&RSBspNode::m_pNegative);

	pfile->Read(&pNode->nPolygon, sizeof(int));

	if (pNode->nPolygon)
	{
		pNode->pInfo = State.Info; State.Info += pNode->nPolygon;

		RPOLYGONINFO *pInfo = pNode->pInfo;

		int i;
		for (i = 0; i < pNode->nPolygon; i++)
		{
			int mat;

			rvector c1, c2, c3, nor;

			pfile->Read(&mat, sizeof(int));
			pfile->Read(&pInfo->nConvexPolygon, sizeof(int));
			pfile->Read(&pInfo->dwFlags, sizeof(DWORD));
			pfile->Read(&pInfo->nVertices, sizeof(int));

			BSPVERTEX *pVertex = pInfo->pVertices = State.Vertices;

			for (int j = 0; j < pInfo->nVertices; j++)
			{
				rvector normal;
				pfile->Read(&pVertex->x, sizeof(float) * 3);
				pfile->Read(&normal, sizeof(float) * 3);
				pfile->Read(&pVertex->tu1, sizeof(float) * 4);
				if (State.Normals)
				{
					*State.Normals = BSPNORMALVERTEX{
						v3{pVertex->x, pVertex->y, pVertex->z},
						normal,
						v2{pVertex->tu1, pVertex->tv1},
						v2{pVertex->tu2, pVertex->tv2},
					};
					State.Normals++;
				}
				pVertex++;
			}

			State.Vertices += pInfo->nVertices;

			pfile->Read(&nor, sizeof(rvector));
			pInfo->plane.a = nor.x;
			pInfo->plane.b = nor.y;
			pInfo->plane.c = nor.z;
			pInfo->plane.d = -DotProduct(nor, *pInfo->pVertices[0].Coord());

			if ((pInfo->dwFlags & RM_FLAG_HIDE) != 0)
				pInfo->nMaterial = -1;
			else
			{
				int nMaterial = mat + 1;

				if (nMaterial < 0 || nMaterial >= m_nMaterial) nMaterial = 0;

				pInfo->nMaterial = nMaterial;
				pInfo->dwFlags |= Materials[nMaterial].dwFlags;
			}
			_ASSERT(pInfo->nMaterial < m_nMaterial);
			pInfo->nPolygonID = State.PolygonID;
			pInfo->nLightmapTexture = 0;

			pInfo++;
			State.PolygonID++;
		}
	}

	return State;
}

bool RBspObject::CreateVertexBuffer()
{
	if (m_nPolygon * 3 > 65530 || m_nPolygon == 0) return false;

	SAFE_RELEASE(VertexBuffer);

	HRESULT hr = RGetDevice()->CreateVertexBuffer(GetStride() * OcVertices.size(),
		D3DUSAGE_WRITEONLY, GetFVF(),
		D3DPOOL_MANAGED, MakeWriteProxy(VertexBuffer),
		nullptr);
	_ASSERT(hr == D3D_OK);
	if (hr != D3D_OK) return false;

	return true;
}

bool RBspObject::UpdateVertexBuffer()
{
	if (!VertexBuffer) return false;

	LPBYTE pVer = nullptr;
	HRESULT hr = VertexBuffer->Lock(0, 0, (VOID**)&pVer, 0);
	_ASSERT(hr == D3D_OK);
	if (hr != D3D_OK)
		return false;
	auto* Src = RenderWithNormal ? static_cast<void*>(OcNormalVertices.data()) : OcVertices.data();
	memcpy(pVer, Src, GetStride() * OcVertices.size());
	hr = VertexBuffer->Unlock();
	_ASSERT(hr == D3D_OK);
	if (hr != D3D_OK)
		return false;

	return true;
}

void RBspObject::GetNormal(RCONVEXPOLYGONINFO *poly, const rvector &position,
	rvector *normal, int au, int av)
{
	int nSelPolygon = -1, nSelEdge = -1;
	float fMinDist = FLT_MAX;

	if (poly->nVertices == 3)
		nSelPolygon = 0;
	else
	{
		rvector pnormal(poly->plane.a, poly->plane.b, poly->plane.c);

		for (int i = 0; i < poly->nVertices - 2; i++)
		{
			float t;
			rvector *a = &poly->pVertices[0];
			rvector *b = &poly->pVertices[i + 1];
			rvector *c = &poly->pVertices[i + 2];


			if (IntersectTriangle(*a, *b, *c, position + pnormal, -pnormal, &t))
			{
				nSelPolygon = i;
				nSelEdge = -1;
				break;
			}
			else
			{
				float dist = GetDistance(position, *a, *b);
				if (dist < fMinDist) { fMinDist = dist; nSelPolygon = i; nSelEdge = 0; }
				dist = GetDistance(position, *b, *c);
				if (dist < fMinDist) { fMinDist = dist; nSelPolygon = i; nSelEdge = 1; }
				dist = GetDistance(position, *c, *a);
				if (dist < fMinDist) { fMinDist = dist; nSelPolygon = i; nSelEdge = 2; }
			}
		}
	}

	rvector *v0 = &poly->pVertices[0];
	rvector *v1 = &poly->pVertices[nSelPolygon + 1];
	rvector *v2 = &poly->pVertices[nSelPolygon + 2];

	rvector *n0 = &poly->pNormals[0];
	rvector *n1 = &poly->pNormals[nSelPolygon + 1];
	rvector *n2 = &poly->pNormals[nSelPolygon + 2];

	rvector pos;
	if (nSelEdge != -1)
	{
		rvector *e0 = nSelEdge == 0 ? v0 : nSelEdge == 1 ? v1 : v2;
		rvector *e1 = nSelEdge == 0 ? v1 : nSelEdge == 1 ? v2 : v0;

		rvector dir = *e1 - *e0;
		Normalize(dir);

		pos = *e0 + DotProduct(dir, position - *e0)*dir;
	}
	else
		pos = position;

	rvector a, b, x, tem;

	a = *v1 - *v0;
	b = *v2 - *v1;
	x = pos - *v0;

	float f = b[au] * x[av] - b[av] * x[au];
	if (IS_ZERO(f))
	{
		*normal = *n0;
		return;
	}
	float t = (a[av] * x[au] - a[au] * x[av]) / f;

	tem = Slerp(*n1, *n2, t);


	rvector inter = a + t*b;

	int axisfors;
	if (fabs(inter.x) > fabs(inter.y) && fabs(inter.x) > fabs(inter.z)) axisfors = 0;
	else
		if (fabs(inter.y) > fabs(inter.z)) axisfors = 1;
		else axisfors = 2;

		float s = x[axisfors] / inter[axisfors];
		*normal = Slerp(*n0, tem, s);
}

// This whole thing is absolutely terrible but nothing is using it so whatever.
bool RBspObject::GenerateLightmap(const char *filename, int nMaxlightmapsize, int nMinLightmapSize,
	int nSuperSample, float fToler, RGENERATELIGHTMAPCALLBACK pProgressFn)
{
	bool bReturnValue = true;

	ClearLightmaps();

	int i, j, k, l;

	float fMaximumArea = 0;

	for (i = 0; i < m_nConvexPolygon; i++)
	{
		fMaximumArea = max(fMaximumArea, ConvexPolygons[i].fArea);
	}

	int nConstCount = 0;
	int nLight;
	RLIGHT **pplight;
	pplight = new RLIGHT*[m_StaticMapLightList.size()];
	rvector *lightmap = new rvector[nMaxlightmapsize*nMaxlightmapsize];
	DWORD	*lightmapdata = new DWORD[nMaxlightmapsize*nMaxlightmapsize];
	bool *isshadow = new bool[(nMaxlightmapsize + 1)*(nMaxlightmapsize + 1)];
	int	*pSourceLightmap = new int[m_nConvexPolygon];
	std::map<DWORD, int> ConstmapTable;

	std::vector<RLIGHTMAPTEXTURE*> sourcelightmaplist;
	std::vector<RBspLightmapManager> LightmapList;

	RHEADER header(R_LM_ID, R_LM_VERSION);

	for (i = 0; i < m_nConvexPolygon; i++)
	{
		RCONVEXPOLYGONINFO *poly = &ConvexPolygons[i];

		if (pProgressFn)
		{
			bool bContinue = pProgressFn((float)i / (float)m_nConvexPolygon);
			if (!bContinue)
			{
				bReturnValue = false;
				goto clearandexit;
			}
		}

		rboundingbox bbox;

		bbox.vmin = bbox.vmax = poly->pVertices[0];
		for (j = 1; j < poly->nVertices; j++)
		{
			for (k = 0; k < 3; k++)
			{
				bbox.vmin[k] = min(bbox.vmin[k], poly->pVertices[j][k]);
				bbox.vmax[k] = max(bbox.vmax[k], poly->pVertices[j][k]);
			}
		}

		int lightmapsize;

		{
			lightmapsize = nMaxlightmapsize;

			float targetarea = fMaximumArea / 4.f;
			while (poly->fArea<targetarea && lightmapsize>nMinLightmapSize)
			{
				targetarea /= 4.f;
				lightmapsize /= 2;
			}

			rvector diff = float(lightmapsize) / float(lightmapsize - 1)*(bbox.vmax - bbox.vmin);

			// 1 texel
			for (k = 0; k<3; k++)
			{
				bbox.vmin[k] -= .5f / float(lightmapsize)*diff[k];
				bbox.vmax[k] += .5f / float(lightmapsize)*diff[k];
			}

			rvector pnormal = rvector(poly->plane.a, poly->plane.b, poly->plane.c);

			RBSPMATERIAL *pMaterial = &Materials[ConvexPolygons[i].nMaterial];

			rvector ambient = pMaterial->Ambient;

			nLight = 0;

			for (auto& Light : m_StaticMapLightList)
			{
				if (GetDistance(Light.Position, poly->plane)>Light.fAttnEnd) continue;

				for (int iv = 0; iv < poly->nVertices; iv++)
				{
					if (DotProduct(Light.Position - poly->pVertices[iv], poly->pNormals[iv])>0) {
						pplight[nLight++] = &Light;
						break;
					}

				}
			}


			int au, av, ax;

			if (fabs(poly->plane.a) > fabs(poly->plane.b) && fabs(poly->plane.a) > fabs(poly->plane.c))
				ax = 0; // yz
			else if (fabs(poly->plane.b) > fabs(poly->plane.c))
				ax = 1;	// xz
			else
				ax = 2;	// xy

			au = (ax + 1) % 3;
			av = (ax + 2) % 3;

			for (j = 0; j < lightmapsize; j++)			// v 
			{
				for (k = 0; k < lightmapsize; k++)		// u
				{
					lightmap[j*lightmapsize + k] = m_AmbientLight;
				}
			}

			for (l = 0; l < nLight; l++)
			{
				RLIGHT *plight = pplight[l];

				for (j = 0; j < lightmapsize + 1; j++)			// v 
				{
					for (k = 0; k < lightmapsize + 1; k++)		// u
					{
						isshadow[k*(lightmapsize + 1) + j] = false;
						if ((plight->dwFlags & RM_FLAG_CASTSHADOW) == 0 ||
							(poly->dwFlags & RM_FLAG_RECEIVESHADOW) == 0) continue;
						_ASSERT(plight->dwFlags == 16);

						rvector position;
						position[au] = bbox.vmin[au] + ((float)k / (float)lightmapsize)*diff[au];
						position[av] = bbox.vmin[av] + ((float)j / (float)lightmapsize)*diff[av];
						position[ax] = (-poly->plane.d - pnormal[au] * position[au] - pnormal[av] * position[av]) / pnormal[ax];

						float fDistanceToPolygon = Magnitude(position - plight->Position);

						RBSPPICKINFO bpi;
						if (PickShadow(plight->Position, position, &bpi))
						{
							float fDistanceToPickPos = Magnitude(bpi.PickPos - plight->Position);

							if (fDistanceToPolygon > fDistanceToPickPos + fToler)
								isshadow[k*(lightmapsize + 1) + j] = true;
						}

						{
							for (RMapObjectList::iterator i = m_ObjectList.begin(); i != m_ObjectList.end(); i++)
							{
								ROBJECTINFO *poi = *i;

								if (!poi->pVisualMesh) continue;
								float t;

								rmatrix inv = Inverse(poi->pVisualMesh->m_WorldMat);

								rvector origin = plight->Position * inv;
								rvector target = position * inv;

								rvector dir = target - origin;
								rvector dirorigin = position - plight->Position;

								rvector vOut;

								rboundingbox bbox;
								bbox.vmin = poi->pVisualMesh->m_vBMin;
								bbox.vmax = poi->pVisualMesh->m_vBMax;
								auto bBBTest = IntersectLineAABB(t, origin, dir, bbox);
								if (bBBTest && poi->pVisualMesh->Pick(plight->Position, dirorigin, &vOut, &t))
								{
									rvector PickPos = plight->Position + vOut*t;
									isshadow[k*(lightmapsize + 1) + j] = true;
								}
							}
						}
					}
				}


				for (j = 0; j < lightmapsize; j++)
				{
					for (k = 0; k < lightmapsize; k++)
					{
						rvector color = rvector(0, 0, 0);

						int nShadowCount = 0;

						for (int m = 0; m < 4; m++)
						{
							if (isshadow[(k + m % 2)*(lightmapsize + 1) + j + m / 2])
								nShadowCount++;
						}


						if (nShadowCount < 4)
						{
							if (nShadowCount > 0)
							{
								int m, n;
								rvector tempcolor = rvector(0, 0, 0);

								for (m = 0; m < nSuperSample; m++)
								{
									for (n = 0; n < nSuperSample; n++)
									{
										rvector position;
										position[au] = bbox.vmin[au] + (((float)k + ((float)n + .5f) / (float)nSuperSample) / (float)lightmapsize)*diff[au];
										position[av] = bbox.vmin[av] + (((float)j + ((float)m + .5f) / (float)nSuperSample) / (float)lightmapsize)*diff[av];
										position[ax] = (-poly->plane.d - pnormal[au] * position[au] - pnormal[av] * position[av]) / pnormal[ax];

										bool bShadow = false;

										float fDistanceToPolygon = Magnitude(position - plight->Position);

										RBSPPICKINFO bpi;
										if (PickShadow(plight->Position, position, &bpi))
										{
											float fDistanceToPickPos = Magnitude(bpi.PickPos - plight->Position);
											if (fDistanceToPolygon > fDistanceToPickPos + fToler)
												bShadow = true;
										}

										if (!bShadow)
										{
											rvector dpos = plight->Position - position;
											float fdistance = Magnitude(dpos);
											float fIntensity = (fdistance - plight->fAttnStart) / (plight->fAttnEnd - plight->fAttnStart);
											fIntensity = min(max(1.0f - fIntensity, 0.f), 1.f);
											Normalize(dpos);

											rvector normal;
											GetNormal(poly, position, &normal, au, av);

											float fDot;
											fDot = DotProduct(dpos, normal);
											fDot = max(0.f, fDot);

											tempcolor += fIntensity*plight->fIntensity*fDot*plight->Color;
										}
									}
								}
								tempcolor *= 1.f / (nSuperSample*nSuperSample);
								color += tempcolor;
							}
							else
							{
								rvector position;
								position[au] = bbox.vmin[au] + (((float)k + .5f) / (float)lightmapsize)*diff[au];
								position[av] = bbox.vmin[av] + (((float)j + .5f) / (float)lightmapsize)*diff[av];
								position[ax] = (-poly->plane.d - pnormal[au] * position[au] - pnormal[av] * position[av]) / pnormal[ax];

								rvector dpos = plight->Position - position;
								float fdistance = Magnitude(dpos);
								float fIntensity = (fdistance - plight->fAttnStart) / (plight->fAttnEnd - plight->fAttnStart);
								fIntensity = min(max(1.0f - fIntensity, 0.f), 1.f);
								Normalize(dpos);

								rvector normal;
								GetNormal(poly, position, &normal, au, av);

								float fDot;
								fDot = DotProduct(dpos, normal);
								fDot = max(0.f, fDot);

								color += fIntensity*plight->fIntensity*fDot*plight->Color;
							}
						}

						lightmap[j*lightmapsize + k] += color;
					}
				}
			}

			for (j = 0; j < lightmapsize*lightmapsize; j++)
			{
				rvector color = lightmap[j];

				color *= 0.25f;
				color.x = min(color.x, 1.f);
				color.y = min(color.y, 1.f);
				color.z = min(color.z, 1.f);
				lightmap[j] = color;
				lightmapdata[j] = ((DWORD)(color.x * 255)) << 16 | ((DWORD)(color.y * 255)) << 8 | ((DWORD)(color.z * 255));
			}
		}

		bool bConstmap = true;
		for (j = 0; j < lightmapsize*lightmapsize; j++)
		{
			if (lightmapdata[j] != lightmapdata[0])
			{
				bConstmap = false;
				nConstCount++;
				break;
			}
		}

		bool bNeedInsert = true;
		if (bConstmap)
		{
			lightmapsize = 2;

			map<DWORD, int>::iterator it = ConstmapTable.find(lightmapdata[0]);
			if (it != ConstmapTable.end())
			{
				pSourceLightmap[i] = (*it).second;
				bNeedInsert = false;
			}
		}

		if (bNeedInsert)
		{
			int nLightmap = sourcelightmaplist.size();

			pSourceLightmap[i] = nLightmap;
			if (bConstmap)
				ConstmapTable.insert(map<DWORD, int>::value_type(lightmapdata[0], nLightmap));

			RLIGHTMAPTEXTURE *pnew = new RLIGHTMAPTEXTURE;
			pnew->bLoaded = false;
			pnew->nSize = lightmapsize;
			pnew->data = new DWORD[lightmapsize*lightmapsize];
			memcpy(pnew->data, lightmapdata, lightmapsize*lightmapsize*sizeof(DWORD));
			sourcelightmaplist.push_back(pnew);
		}
	}

	CalcLightmapUV(BspRoot.data(), pSourceLightmap, sourcelightmaplist, LightmapList);
	CalcLightmapUV(OcRoot.data(), pSourceLightmap, sourcelightmaplist, LightmapList);

	FILE *file = nullptr;
	fopen_s(&file, filename, "wb+");
	if (!file) {
		bReturnValue = false;
		goto clearandexit;
	}

	fwrite(&header, sizeof(RHEADER), 1, file);

	fwrite(&m_nConvexPolygon, sizeof(int), 1, file);
	fwrite(&m_nNodeCount, sizeof(int), 1, file);

	m_nLightmap = LightmapList.size();
	fwrite(&m_nLightmap, sizeof(int), 1, file);
	for (size_t i = 0; i < LightmapList.size(); i++)
	{
		char lightfilename[256];
		sprintf_safe(lightfilename, "%s.light%d.bmp", filename, i);
		RSaveAsBmp(LightmapList[i].GetSize(), LightmapList[i].GetSize(),
			LightmapList[i].GetData(), lightfilename);

		void *memory;
		int nSize;
		bool bSaved = SaveMemoryBmp(LightmapList[i].GetSize(), LightmapList[i].GetSize(),
			LightmapList[i].GetData(), &memory, &nSize);
		_ASSERT(bSaved);
		fwrite(&nSize, sizeof(int), 1, file);
		fwrite(memory, nSize, 1, file);
		delete memory;
	}


	Sort_Nodes(OcRoot.data());

	for (int i = 0; i < m_nPolygon; i++)
		fwrite(&OcInfo[i].nPolygonID, sizeof(int), 1, file);

	for (int i = 0; i < m_nPolygon; i++)
		fwrite(&OcInfo[i].nLightmapTexture, sizeof(int), 1, file);

	for (size_t i = 0; i < OcVertices.size(); i++)
		fwrite(&OcVertices[i].tu2, sizeof(float), 2, file);

	fclose(file);

clearandexit:

	delete[]pplight;
	delete[]lightmap;
	delete[]lightmapdata;
	delete[]isshadow;

	delete pSourceLightmap;
	while (sourcelightmaplist.size())
	{
		delete (*sourcelightmaplist.begin())->data;
		delete *sourcelightmaplist.begin();
		sourcelightmaplist.erase(sourcelightmaplist.begin());
	}

	return bReturnValue;
}

void RBspObject::CalcLightmapUV(RSBspNode *pNode, int *pSourceLightmap,
	std::vector<RLIGHTMAPTEXTURE*>& SourceLightmaps, std::vector<RBspLightmapManager>& LightmapList)
{
	if (pNode->nPolygon)
	{
		int i, j, k;
		for (i = 0; i < pNode->nPolygon; i++)
		{
			int is = pNode->pInfo[i].nConvexPolygon;
			int nSI = pSourceLightmap[is];

			RCONVEXPOLYGONINFO *poly = &ConvexPolygons[is];

			rboundingbox bbox;

			bbox.vmin = bbox.vmax = poly->pVertices[0];
			for (j = 1; j < poly->nVertices; j++)
			{
				for (k = 0; k < 3; k++)
				{
					bbox.vmin[k] = min(bbox.vmin[k], poly->pVertices[j][k]);
					bbox.vmax[k] = max(bbox.vmax[k], poly->pVertices[j][k]);
				}
			}

			RLIGHTMAPTEXTURE* pDestLightmap = SourceLightmaps[nSI];

			int lightmapsize = pDestLightmap->nSize;

			rvector diff = float(lightmapsize) / float(lightmapsize - 1)*(bbox.vmax - bbox.vmin);

			for (k = 0; k<3; k++)
			{
				bbox.vmin[k] -= .5f / float(lightmapsize)*diff[k];
				bbox.vmax[k] += .5f / float(lightmapsize)*diff[k];
			}

			int au, av, ax;

			if (fabs(poly->plane.a)>fabs(poly->plane.b) && fabs(poly->plane.a) > fabs(poly->plane.c))
				ax = 0; // yz
			else if (fabs(poly->plane.b) > fabs(poly->plane.c))
				ax = 1;	// xz
			else
				ax = 2;	// xy

			au = (ax + 1) % 3;
			av = (ax + 2) % 3;

			RPOLYGONINFO *pInfo = &pNode->pInfo[i];
			for (j = 0; j < pInfo->nVertices; j++)
			{
				pInfo->pVertices[j].tu2 = ((*pInfo->pVertices[j].Coord())[au] - bbox.vmin[au]) / diff[au];
				pInfo->pVertices[j].tv2 = ((*pInfo->pVertices[j].Coord())[av] - bbox.vmin[av]) / diff[av];
			}

			auto* CurrentLightmap = LightmapList.size() ? &LightmapList[LightmapList.size() - 1] : NULL;

			if (!pDestLightmap->bLoaded)
			{
				POINT pt;

				while (!CurrentLightmap || !CurrentLightmap->Add(pDestLightmap->data, pDestLightmap->nSize, &pt))
				{
					LightmapList.emplace_back();
					CurrentLightmap = &LightmapList.back();
				}
				pDestLightmap->bLoaded = true;
				pDestLightmap->position = pt;
				pDestLightmap->nLightmapIndex = LightmapList.size() - 1;
			}

			pNode->pInfo[i].nLightmapTexture = pDestLightmap->nLightmapIndex;

			float fScaleFactor = (float)pDestLightmap->nSize / (float)CurrentLightmap->GetSize();
			for (j = 0; j < pInfo->nVertices; j++)
			{
				pInfo->pVertices[j].tu2 =
					pInfo->pVertices[j].tu2 * fScaleFactor +
					(float)pDestLightmap->position.x / (float)CurrentLightmap->GetSize();
				pInfo->pVertices[j].tv2 =
					pInfo->pVertices[j].tv2 * fScaleFactor +
					(float)pDestLightmap->position.y / (float)CurrentLightmap->GetSize();
			}
		}
	}

	if (pNode->m_pPositive) CalcLightmapUV(pNode->m_pPositive, pSourceLightmap, SourceLightmaps, LightmapList);
	if (pNode->m_pNegative) CalcLightmapUV(pNode->m_pNegative, pSourceLightmap, SourceLightmaps, LightmapList);
}

DWORD RBspObject::GetLightmap(rvector &Pos, RSBspNode *pNode, int nIndex)
{
	return 0xffffffff;
}

rvector RBspObject::GetDimension() const
{
	if (OcRoot.empty())
		return v3{0, 0, 0};

	return OcRoot[0].bbTree.vmax - OcRoot[0].bbTree.vmin;
}

void RBspObject::DrawSolid()
{
#ifndef _PUBLISH

	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();

	pd3dDevice->SetFVF(D3DFVF_XYZ);
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0x406fa867);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	ColRoot[0].DrawSolidPolygon();

	RSetWBuffer(true);
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ffffff);
	ColRoot[0].DrawSolidPolygonWireframe();

	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ff00ff);
	ColRoot[0].DrawSolidPolygonNormal();

	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ff0000);
#endif
}

void RBspObject::DrawSolidNode()
{
	if (!m_DebugInfo.pLastColNode) return;
#ifndef _PUBLISH
	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();

	pd3dDevice->SetFVF(D3DFVF_XYZ);
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40808080);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	m_DebugInfo.pLastColNode->DrawSolidPolygon();

	RSetWBuffer(true);
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ffffff);
	m_DebugInfo.pLastColNode->DrawSolidPolygonWireframe();

	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ff00ff);
	m_DebugInfo.pLastColNode->DrawSolidPolygonNormal();

	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ff0000);

	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ffffff);

#endif
}

bool RBspObject::CheckWall(const rvector &origin, rvector &targetpos, float fRadius, float fHeight,
	RCOLLISIONMETHOD method, int nDepth, rplane *pimpactplane)
{
	return RSolidBspNode::CheckWall(ColRoot.data(), origin, targetpos, fRadius, fHeight, method, nDepth, pimpactplane);
}

bool RBspObject::CheckSolid(const rvector &pos, float fRadius, float fHeight, RCOLLISIONMETHOD method)
{
	RImpactPlanes impactPlanes;
	if (method == RCW_SPHERE)
		return ColRoot[0].GetColPlanes_Sphere(&impactPlanes, pos, pos, fRadius);
	else
		return ColRoot[0].GetColPlanes_Cylinder(&impactPlanes, pos, pos, fRadius, fHeight);
}

rvector RBspObject::GetFloor(const rvector &origin, float fRadius, float fHeight, rplane *pimpactplane)
{
	rvector targetpos = origin + rvector(0, 0, -10000);

	RImpactPlanes impactPlanes;
	bool bIntersect = ColRoot[0].GetColPlanes_Cylinder(&impactPlanes, origin, targetpos, fRadius, fHeight);
	if (!bIntersect)
		return targetpos;

	rvector floor = ColRoot[0].GetLastColPos();
	floor.z -= fHeight;
	if (pimpactplane)
		*pimpactplane = ColRoot[0].GetLastColPlane();

	return floor;
}

RBSPMATERIAL *RBspObject::GetMaterial(int nIndex)
{
	_ASSERT(nIndex < m_nMaterial);
	return &Materials[nIndex];
}

RBaseTexture* RBspObject::GetBaseTexture(int n) {
	if (n >= 0 && n < m_nMaterial)
		return Materials[n].texture;
	return nullptr;
}

bool RBspObject::IsVisible(const rboundingbox &bb) const
{
	return m_OcclusionList.IsVisible(bb);
}

bool RBspObject::GetShadowPosition(const rvector& pos_, const rvector& dir_, rvector* outNormal_, rvector* outPos_)
{
	RBSPPICKINFO pick_info;
	if (!Pick(pos_, dir_, &pick_info))
		return false;
	*outPos_ = pick_info.PickPos;
	*outNormal_ = rvector(pick_info.pInfo[pick_info.nIndex].plane.a,
		pick_info.pInfo[pick_info.nIndex].plane.b, pick_info.pInfo[pick_info.nIndex].plane.c);
	return true;
}

struct PickInfo
{
	v3 From;
	v3 To;
	float LengthSquared;
	v3 Dir;
	v3 InverseDir;
	RBSPPICKINFO* Out;
	v3 ColPos;
	plane Plane;
	float Dist;
	u32 PassFlag;
};

template <bool Shadow>
bool RBspObject::Pick(std::vector<RSBspNode>& Nodes,
	const v3& src, const v3& dest, const v3& dir,
	u32 PassFlag, RBSPPICKINFO* Out)
{
	// I don't know how many parts of the code can input invalid
	// directions to this function so need to leave this out and
	// normalize for now.
	//assert(IS_EQ(MagnitudeSq(dir), 1));
	auto MagSq = MagnitudeSq(dir);
	if (MagSq == 0)
		return false;

	if (Nodes.empty())
		return false;

	PickInfo pi;
	pi.From = src;
	pi.To = dest;
	pi.Dir = dir;
	if (!IS_EQ(MagSq, 1))
		Normalize(pi.Dir);
	pi.PassFlag = PassFlag;
	pi.Out = Out;
	
	pi.LengthSquared = MagnitudeSq(pi.To - pi.From);
	pi.InverseDir = 1 / dir;
	pi.Plane = PlaneFromPointNormal(pi.From, pi.Dir);
	pi.Dist = FLT_MAX;

	return Pick(Nodes.data(), src, dest, pi);
}

bool RBspObject::Pick(const rvector &pos, const rvector &dir, RBSPPICKINFO *Out, u32 PassFlag) {
	return Pick(BspRoot, pos, pos + dir * 10000.f, dir, PassFlag, Out);
}
bool RBspObject::PickTo(const rvector &pos, const rvector &to, RBSPPICKINFO *Out, u32 PassFlag) {
	return Pick(BspRoot, pos, to, Normalized(to - pos), PassFlag, Out);
}
bool RBspObject::PickOcTree(const rvector &pos, const rvector &dir, RBSPPICKINFO *Out, u32 PassFlag) {
	return Pick(OcRoot, pos, pos + dir * 10000.f, dir, PassFlag, Out);
}
bool RBspObject::PickShadow(const rvector &pos, const rvector &to, RBSPPICKINFO *Out) {
	return Pick<true>(BspRoot, pos, to, Normalized(to - pos), DefaultPassFlag, Out);
}

#define PICK_TOLERENCE 0.01f
#define PICK_SIGN(x) ( (x)<-PICK_TOLERENCE ? -1 : (x)>PICK_TOLERENCE ? 1 : 0 )

static bool pick_checkplane(int side, const rplane &plane, const rvector &v0, const rvector &v1, rvector *w0, rvector *w1)
{
	float dotv0 = DotProduct(plane, v0);
	float dotv1 = DotProduct(plane, v1);

	int signv0 = PICK_SIGN(dotv0), signv1 = PICK_SIGN(dotv1);

	if (signv0 != -side) {
		*w0 = v0;

		if (signv1 != -side)
			*w1 = v1;
		else
		{
			rvector intersect;
			if (IntersectLineSegmentPlane(&intersect, plane, v0, v1))
				*w1 = intersect;
			else
				*w1 = v1;
		}
		return true;
	}

	if (signv1 != -side) {
		*w1 = v1;

		if (signv0 != -side)
			*w0 = v0;
		else
		{
			rvector intersect;
			if (IntersectLineSegmentPlane(&intersect, plane, v0, v1))
				*w0 = intersect;
			else
				*w0 = v0;
		}
		return true;
	}

	return false;
}

template <bool Shadow>
bool RBspObject::CheckLeafNode(RSBspNode* pNode, const v3& v0, const v3& v1, PickInfo& pi)
{
	bool Picked = false;

	for (int i = 0; i < pNode->nPolygon; i++)
	{
		RPOLYGONINFO *pInfo = &pNode->pInfo[i];

		if ((pInfo->dwFlags & pi.PassFlag) != 0)
			continue;

		if (Shadow && (pInfo->dwFlags & RM_FLAG_CASTSHADOW) == 0)
			continue;

		// If the ray is coming from behind the triangle, it can't be intersecting.
		if (DotProduct(pInfo->plane, pi.From) < 0)
			continue;

		// Check each triangle
		for (int j = 0; j < pInfo->nVertices - 2; j++)
		{
			float TriDist;
			if (IntersectTriangle(*pInfo->pVertices[0].Coord(),
				*pInfo->pVertices[j + 1].Coord(),
				*pInfo->pVertices[j + 2].Coord(),
				pi.From, pi.Dir,
				&TriDist) &&
				TriDist < pi.Dist)
			{
				pi.Dist = TriDist;
				pi.Out->PickPos = TriDist * pi.Dir + pi.From;
				pi.Out->pNode = pNode;
				pi.Out->nIndex = i;
				pi.Out->pInfo = &pNode->pInfo[i];
				Picked = true;
			}
		}
	}

	return Picked;
}

template <bool Shadow>
bool RBspObject::CheckBranches(RSBspNode* pNode, const v3& v0, const v3& v1, PickInfo& pi)
{
	rvector w0, w1;
	float t;
	auto CheckBranch = [&](int side, auto* branch) {
		return IntersectLineAABB(t, pi.From, pi.Dir, pNode->bbTree, pi.InverseDir) &&
			Square(t) < pi.LengthSquared &&
			pick_checkplane(side, pNode->plane, v0, v1, &w0, &w1) &&
			Pick<Shadow>(branch, w0, w1, pi);
	};
	auto CheckPositive = [&] { return CheckBranch(1, pNode->m_pPositive); };
	auto CheckNegative = [&] { return CheckBranch(-1, pNode->m_pNegative); };

	bool bHit = false;
	if (DotPlaneNormal(pNode->plane, pi.Dir) > 0)
		return CheckNegative() || CheckPositive();
	else
		return CheckPositive() || CheckNegative();
}

template <bool Shadow>
bool RBspObject::Pick(RSBspNode *pNode, const rvector &v0, const rvector &v1, PickInfo& pi)
{
	if (!pNode)
		return false;

	if (pNode->nPolygon)
		return CheckLeafNode<Shadow>(pNode, v0, v1, pi);

	return CheckBranches<Shadow>(pNode, v0, v1, pi);
}

void RBspObject::GetNormal(int nConvexPolygon, const rvector &position, rvector *normal)
{
	RCONVEXPOLYGONINFO *poly = &ConvexPolygons[nConvexPolygon];
	int au, av, ax;

	if (fabs(poly->plane.a) > fabs(poly->plane.b) && fabs(poly->plane.a) > fabs(poly->plane.c))
		ax = 0; // yz
	else if (fabs(poly->plane.b) > fabs(poly->plane.c))
		ax = 1;	// xz
	else
		ax = 2;	// xy

	au = (ax + 1) % 3;
	av = (ax + 2) % 3;

	GetNormal(poly, position, normal, au, av);
}

RBaseTexture *RBspObject::m_pShadeMap;

bool RBspObject::CreateShadeMap(const char *szShadeMap)
{
	if (m_pShadeMap)
		DestroyShadeMap();
	m_pShadeMap = RCreateBaseTexture(szShadeMap, RTextureType_Etc, false);
	return true;
}

void RBspObject::DestroyShadeMap()
{
	RDestroyBaseTexture(m_pShadeMap);
	m_pShadeMap = NULL;
}

struct LIGHTBSPVERTEX {
	rvector coord;
	DWORD dwColor;
	float tu1, tv1;
	float tu2, tv2;
};

#define LIGHTVERTEXBUFFER_SIZE	1024

DWORD m_dwLightVBBase = 0;
LIGHTBSPVERTEX *m_pLightVertex;

bool RBspObject::CreateDynamicLightVertexBuffer()
{
	InvalidateDynamicLightVertexBuffer();
	HRESULT hr = RGetDevice()->CreateVertexBuffer(
		sizeof(LIGHTBSPVERTEX) * LIGHTVERTEXBUFFER_SIZE * 3,
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		LIGHT_BSP_FVF,
		D3DPOOL_DEFAULT,
		MakeWriteProxy(DynLightVertexBuffer),
		nullptr);

	return true;
}

void RBspObject::InvalidateDynamicLightVertexBuffer()
{
	SAFE_RELEASE(DynLightVertexBuffer);
}

bool RBspObject::FlushLightVB()
{
	DynLightVertexBuffer->Unlock();
	if (m_dwLightVBBase == 0) return true;

#ifdef _DEBUG
	g_nCall++;
#endif
	HRESULT hr = RGetDevice()->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_dwLightVBBase);
	_ASSERT(hr == D3D_OK);
	return true;
}

bool RBspObject::LockLightVB()
{
	HRESULT hr = DynLightVertexBuffer->Lock(0,
		LIGHTVERTEXBUFFER_SIZE * sizeof(LIGHTBSPVERTEX),
		(LPVOID*)&m_pLightVertex, D3DLOCK_DISCARD);
	if (FAILED(hr))
	{
		return false;
	}
	m_dwLightVBBase = 0;

	return true;
}

D3DLIGHT9* g_pTargetLight;
DWORD g_dwTargetLightColor;

bool RBspObject::DrawLight(RSBspNode *pNode, int nMaterial)
{
	if (pNode->nFrameCount != g_nFrameNumber) return true;

	if (pNode->nPolygon)
	{
		int nCount = pNode->pDrawInfo[nMaterial].nTriangleCount;
		int nIndCount = nCount * 3;
		if (nCount)
		{
#ifdef _DEBUG
			g_nPoly += nCount;
#endif

			RDrawInfo *pdi = &pNode->pDrawInfo[nMaterial];
			int index = pdi->nIndicesOffset;

			int i;
			for (i = 0; i < nCount; i++)
			{

				rplane *pPlane = &pdi->pPlanes[i];

				WORD indices[3] = { OcIndices[index++], OcIndices[index++], OcIndices[index++] };

				float fPlaneDotCoord = pPlane->a * g_pTargetLight->Position.x + pPlane->b * g_pTargetLight->Position.y +
					pPlane->c * g_pTargetLight->Position.z + pPlane->d;

				if (fPlaneDotCoord > g_pTargetLight->Range) continue;

				constexpr auto BACK_FACE_DISTANCE = 200.f;
				if (fPlaneDotCoord < -BACK_FACE_DISTANCE) continue;
				if (fPlaneDotCoord < 0) fPlaneDotCoord = -fPlaneDotCoord / BACK_FACE_DISTANCE * g_pTargetLight->Range;

				LIGHTBSPVERTEX *v = m_pLightVertex + m_dwLightVBBase * 3;

				for (int j = 0; j < 3; j++) {
					BSPVERTEX *pv = &OcVertices[indices[j]];
					v[j].coord = *pv->Coord();
					v[j].tu2 = pv->tu1;
					v[j].tv2 = pv->tv1;
				}

				rvector t;
				t = v[0].coord - g_pTargetLight->Position;

				for (int j = 0; j < 3; j++)
				{
					rvector l = v[j].coord - g_pTargetLight->Position;
					l *= 1.f / g_pTargetLight->Range;

					v[j].tu1 = -DotProduct(pdi->pUAxis[i], l) * .5 + .5;
					v[j].tv1 = -DotProduct(pdi->pVAxis[i], l) * .5 + .5;

					float fIntensity = min(1.f, max(0.f, 1.f - fPlaneDotCoord / g_pTargetLight->Range));

					v[j].dwColor = DWORD(fIntensity * 255) << 24 | g_dwTargetLightColor;

				}

				m_dwLightVBBase++;
				if (m_dwLightVBBase == LIGHTVERTEXBUFFER_SIZE)
				{
					FlushLightVB();
					LockLightVB();
				}
			}
		}
	}
	else
	{
		bool bOk = true;
		if (pNode->m_pNegative) {
			if (!DrawLight(pNode->m_pNegative, nMaterial))
				bOk = false;
		}
		if (pNode->m_pPositive) {
			if (!DrawLight(pNode->m_pPositive, nMaterial))
				bOk = false;
		}
		return bOk;
	}
	return true;
}

void RBspObject::DrawLight(D3DLIGHT9 *pLight)
{
	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();
	if (!VertexBuffer)
		return;

	if (!DynLightVertexBuffer)
		CreateDynamicLightVertexBuffer();

	pd3dDevice->SetTexture(0, m_pShadeMap->GetTexture());

	RGetDevice()->SetStreamSource(0, DynLightVertexBuffer, 0, sizeof(LIGHTBSPVERTEX));

	g_pTargetLight = pLight;
	g_dwTargetLightColor = FLOAT2RGB24(
		min(1.f, max(0.f, g_pTargetLight->Diffuse.r)),
		min(1.f, max(0.f, g_pTargetLight->Diffuse.g)),
		min(1.f, max(0.f, g_pTargetLight->Diffuse.b)));

#ifdef _DEBUG
	g_nPoly = 0;
	g_nCall = 0;
#endif
	g_nFrameNumber++;

	int nChosen = ChooseNodes(OcRoot.data(), rvector(pLight->Position), pLight->Range);

	for (int i = 0; i < m_nMaterial; i++)
	{
		if ((Materials[i % m_nMaterial].dwFlags & RM_FLAG_TWOSIDED) == 0)
			RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		else
			RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		if ((Materials[i % m_nMaterial].dwFlags & RM_FLAG_ADDITIVE) == 0)
		{
			int nMaterial = i % m_nMaterial;
			RBaseTexture *pTex = Materials[nMaterial].texture;
			if (pTex)
			{
				pd3dDevice->SetTexture(1, pTex->GetTexture());
				pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			}
			else
			{
				DWORD dwDiffuse = VECTOR2RGB24(Materials[nMaterial].Diffuse);
				pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, dwDiffuse);
				pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TFACTOR);
			}

			LockLightVB();
			DrawLight(OcRoot.data(), i);
			FlushLightVB();
		}
	}

	pd3dDevice->SetStreamSource(0, NULL, 0, 0);
}

_NAMESPACE_REALSPACE2_END