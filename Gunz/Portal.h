#pragma once
#include "Extensions.h"
#ifdef PORTAL
#include "rtypes.h"
#include "ZCharacter.h"
#include <unordered_map>
#undef pi
#include "ArrayView.h"

//#define PORTAL_USE_RT_TEXTURE

struct PortalInfo
{
	D3DXVECTOR3 vPos, vNormal, vUp, vOBB;
	float d;
	D3DXMATRIX matWorld, matInvWorld, matView, matRot, matTransform;
	//D3DXPLANE Frustum[6];
	D3DXPLANE Near;
	D3DXPLANE Far;
	D3DXVECTOR3 topright, topleft, bottomright, bottomleft;
	rboundingbox bb;
	bool bSet;
	int Index;
	PortalInfo* Other;

	PortalInfo()
	{
		ZeroMemory(this, sizeof(*this));
	}
};

class PortalPair
{
public:
	PortalPair()
	{
		for (int i = 0; i < 2; i++)
		{
			pi[i].Other = &pi[!i];
			pi[i].Index = i;
		}
	}

	PortalInfo& operator [](size_t Index)
	{
		return pi[Index];
	}

	bool IsValid() const
	{
		return pi[0].bSet && pi[1].bSet;
	}

private:
	PortalInfo pi[2];
};

class PortalPairRef
{
public:
	PortalPairRef(PortalInfo& a, PortalInfo& b) : pi{ a, b }
	{
	}

	PortalInfo& operator [](size_t Index)
	{
		return pi[Index];
	}

	bool IsValid() const
	{
		return pi[0].get().bSet && pi[1].get().bSet;
	}

private:
	std::reference_wrapper<PortalInfo> pi[2];
};

struct PortalContext
{
	bool IsVisible;
	rplane Frustum[6];
	PortalInfo& pi;
	PortalContext* Other;

	PortalContext(PortalInfo &a) : pi(a)
	{
	}
};

struct CameraContext
{
	D3DXVECTOR3 Pos;
	D3DXVECTOR3 Dir;
	D3DXPLANE Frustum[6];
};

struct RecursionContext
{
	int Depth = 0;
	PortalInfo* ViewingPortal;
	ArrayView<PortalContext[2]> Portals;
	CameraContext Cam;

	RecursionContext(int d, const ArrayView<PortalContext[2]>& p) : Depth(d), Portals(p)
	{
	}
};

class PortalDraw
{
public:
	virtual void DrawPortals(const RecursionContext& rc);
};

class PortalDrawDX9 : PortalDraw
{
public:
	PortalDrawDX9();

	virtual void DrawPortals(const RecursionContext& rc) override final;
};

class Portal
{
private:
	ID3DXMesh *pEdgeMesh;
	ID3DXMesh *pRectangleMesh;
	D3DMATERIAL9 Mat;
#ifdef PORTAL_USE_RT_TEXTURE
	IDirect3DTexture9 *pTex[2];
	IDirect3DSurface9 *pSurf[2];
	IDirect3DTexture9 *pDummyTex;
	IDirect3DSurface9 *pDummySurf;
#endif
	D3DPtr<IDirect3DTexture9> BlackTex;
	D3DPtr<IDirect3DTexture9> PortalEdgeTex[2];

	std::unordered_map<ZCharacter *, PortalPair> PortalList;

	PortalInfo *pMyPortalInfo;

	bool bTeleported;

	rmatrix matNearProjection;

	bool bMakeNearProjectionMatrix;
	bool bLookingThroughPortal;

	rplane NewFrustum[6];

	static bool bPortalSetExists;

	D3DXVECTOR3 vCameraPos, vCameraDir;

	bool bLastLClick;
	bool bLastRClick;

	static const D3DXVECTOR3 vDim;

	bool bDontDraw;
	bool bDontDrawChar;
	bool bForceProjection;

	int RecursionCount = 0;
	int LastFrameCount = 0;

	PortalDraw* Draw = nullptr;

#ifdef PORTAL_USE_RT_TEXTURE
	void CreateTextures();
	void ReleaseTextures();
#endif

#ifdef PORTAL_USE_RT_TEXTURE
	void Render();
	void RenderWorld();
#else
	void RenderWorldStencil(const PortalContext* Contexts, size_t NumContexts);
#endif

	void UpdateAndRender(const RecursionContext* PrevContext = nullptr);

	void Update(RecursionContext& rc);
	void RenderPortals(const RecursionContext& rc);

	void WriteDepth(const RecursionContext& rc);

	void RenderEdge(const PortalInfo& portalinfo);

	void RedirectCamera();

	bool ZObjectPortalIntersection(const ZObject *pObj, PortalInfo **retppi);
	bool LinePortalIntersection(const D3DXVECTOR3 &L1, const D3DXVECTOR3 &L2, const PortalInfo &ppi, D3DXVECTOR3 &Hit);
	bool AABBPortalIntersection(const D3DXVECTOR3 &B1, const D3DXVECTOR3 &B2, const PortalInfo &ppi);

	bool CheckIntersection(const D3DXVECTOR3 &target, float fRadius, float fHeight, PortalInfo **retppi);

	void Transform(D3DXVECTOR3 &v, const PortalInfo &ppi);
	void Transform(D3DXVECTOR3 &vOut, const D3DXVECTOR3 &vIn, const PortalInfo &ppi);
	void Rotate(D3DXVECTOR3 &v, const PortalInfo &ppi);

	template <typename T>
	void ForEachPortal(T fn)
	{
		for (auto pair : PortalList)
		{
			if (!pair.second[0].bSet || !pair.second[1].bSet)
				continue;

			for (int i = 0; i < 2; i++)
			{
				fn(pair.second[i]);
			}
		}
	}

	template <typename T>
	void ForEachPortal(RecursionContext& rc, T fn)
	{
		for (auto p : rc.Portals)
		{
			for (int i = 0; i < 2; i++)
			{
				fn(p[i]);
			}
		}
	}

public:
	Portal();

	void OnInvalidate();
	void OnRestore();

	void OnShot();
	bool RedirectPos(D3DXVECTOR3 &from, D3DXVECTOR3 &to);

	void DeletePlayer(ZCharacter *pZChar);

	void DrawFakeCharacter(ZCharacter *pZChar);

	bool Move(ZObject *pObj, D3DXVECTOR3 &diff);

	void PreDraw();
	void PostDraw();

	void CreatePortal(ZCharacter *pZChar, int iPortal, const D3DXVECTOR3 &vPos, const D3DXVECTOR3 &vNormal, const D3DXVECTOR3 &vUp);

	bool ForceProjection(){
		return bForceProjection;
	}

	bool IsDrawingFakeChar() const
	{
		return bDontDrawChar;
	}
};

extern Portal *g_pPortal;
#endif