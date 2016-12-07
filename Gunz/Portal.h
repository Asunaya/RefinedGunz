#pragma once
#include "Config.h"
#ifdef PORTAL
#include "rtypes.h"
#include "ZCharacter.h"
#include <unordered_map>
#include "ArrayView.h"
#include "SimpleMesh.h"

//#define PORTAL_USE_RT_TEXTURE

struct PortalInfo
{
	rvector vPos, vNormal, vUp, vOBB;
	float d;
	rmatrix matWorld, matInvWorld, matView, matRot, matTransform;
	rplane Near;
	rplane Far;
	rvector topright, topleft, bottomright, bottomleft;
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

struct PortalContextPair
{
	PortalContextPair(typename std::unordered_map<ZCharacter *, PortalPair>::iterator it)
		: p{ it->second[0], it->second[1] }
	{
		p[0].Other = &p[1];
		p[1].Other = &p[2];
	}

	auto& operator[](size_t idx)
	{
		return p[idx];
	}

	auto& operator[](size_t idx) const
	{
		return p[idx];
	}

	PortalContext p[2];
};

struct CameraContext
{
	rvector Pos;
	rvector Dir;
	rplane Frustum[6];
};

struct RecursionContext
{
	int Depth = 0;
	PortalInfo* ViewingPortal;
	ArrayView<PortalContextPair> Portals;
	CameraContext Cam;

	RecursionContext(int d, const ArrayView<PortalContextPair>& p) : Depth(d), Portals(p) {}
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

class ValidPortalIterator
{
public:
	ValidPortalIterator(const std::unordered_map<ZCharacter *, PortalPair>& pl, const std::unordered_map<ZCharacter *, PortalPair>::iterator& it_) : PortalList(pl), it(it_)
	{
	}

	ValidPortalIterator& operator++(int)
	{
		auto prev_it = *this;

		++*this;

		return prev_it;
	}

	ValidPortalIterator& operator++()
	{
		if (index == 0)
		{
			index = 1;
			return *this;
		}

		index = 0;

		do
		{
			it++;
		} while (it != PortalList.end() && !it->second.IsValid());

		return *this;
	}

	bool operator!=(const ValidPortalIterator& rhs) const
	{
		return it != rhs.it;
	}

	PortalInfo& operator*()
	{
		return it->second[index];
	}

private:
	std::unordered_map<ZCharacter *, PortalPair>::iterator it;
	int index = 0;
	const std::unordered_map<ZCharacter *, PortalPair>& PortalList;
};

class ValidPortalAdapter
{
public:
	ValidPortalAdapter(std::unordered_map<ZCharacter *, PortalPair>& pl) : PortalList(pl)
	{
	}

	auto begin() const
	{
		return ValidPortalIterator(PortalList, PortalList.begin());
	}

	auto end() const
	{
		return ValidPortalIterator(PortalList, PortalList.end());
	}

private:
	std::unordered_map<ZCharacter *, PortalPair>& PortalList;
};

class Portal
{
public:
	Portal();
	~Portal();

	void OnLostDevice();
	void OnResetDevice();

	void OnShot();
	bool RedirectPos(rvector &from, rvector &to);

	void DeletePlayer(ZCharacter *pZChar);

	void DrawFakeCharacter(ZCharacter *pZChar);

	bool Move(ZObject *pObj, rvector &diff);
	bool Move(ZMovingWeapon& Obj, v3& diff);

	void PreDraw();
	void PostDraw();

	void CreatePortal(ZCharacter *pZChar, int iPortal, const rvector &vPos, const rvector &vNormal, const rvector &vUp);

	bool ForceProjection() const
	{
		return bForceProjection;
	}

	bool IsDrawingFakeChar() const
	{
		return bDontDrawChar;
	}

private:
	SimpleMesh<struct D3DVERTEX, u16, D3DFVF_XYZ | D3DFVF_TEX1> RectangleMesh, EdgeMesh;
	D3DMATERIAL9 Mat;
#ifdef PORTAL_USE_RT_TEXTURE
	IDirect3DTexture9 *pTex[2];
	IDirect3DSurface9 *pSurf[2];
	IDirect3DTexture9 *pDummyTex;
	IDirect3DSurface9 *pDummySurf;
#endif
	D3DPtr<IDirect3DTexture9> BlackTex;
	RBaseTexturePtr PortalEdgeTex[2];

	std::unordered_map<ZCharacter *, PortalPair> PortalList;
	ValidPortalAdapter ValidPortals;

	PortalInfo *pMyPortalInfo;

	bool bTeleported;

	rmatrix matNearProjection;

	bool bMakeNearProjectionMatrix;
	bool bLookingThroughPortal;

	rplane NewFrustum[6];

	static bool bPortalSetExists;

	rvector vCameraPos, vCameraDir;

	bool bLastLClick;
	bool bLastRClick;

	static const rvector vDim;

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
	bool LinePortalIntersection(const rvector &L1, const rvector &L2, const PortalInfo &ppi, rvector &Hit);
	bool AABBPortalIntersection(const rvector &B1, const rvector &B2, const PortalInfo &ppi);

	PortalInfo* LineIntersect(const v3& l1, const v3& l2);
	bool CheckIntersection(const rvector &target, float fRadius, float fHeight, PortalInfo **retppi);

	void Transform(rvector &v, const PortalInfo &ppi);
	void Transform(rvector &vOut, const rvector &vIn, const PortalInfo &ppi);
	void Rotate(rvector &v, const PortalInfo &ppi);

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
};

extern Portal *g_pPortal;
#endif