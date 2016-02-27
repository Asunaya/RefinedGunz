#pragma once
//#ifdef DEBUG
//#define PORTAL
//#endif
#ifdef PORTAL
#include "rtypes.h"
#include "ZCharacter.h"
#include <map>

//#define PORTAL_USE_RT_TEXTURE

struct PortalInfo
{
	D3DXVECTOR3 vPos[2], vNormal[2], vUp[2], vOBB[2];
	float d[2];
	D3DXMATRIX matWorld[2], matInvWorld[2], matView[2], matRot[2], matTransform[2];
	D3DXPLANE Frustum[2][6]; // Other portal
	D3DXVECTOR3 topright[2], topleft[2], bottomright[2], bottomleft[2];
	rboundingbox bb[2];
	bool bIsVisible[2];
	bool bSet[2];

	PortalInfo()
	{
		ZeroMemory(this, sizeof(*this));
	}

	bool IsValid() const
	{
		return bSet[0] && bSet[1];
	}
};

typedef std::map<ZCharacter *, PortalInfo>::iterator PortalListIt;

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
	IDirect3DTexture9 *pBlackTex;
	IDirect3DTexture9 *pPortalEdgeTex[2];

	std::map<ZCharacter *, PortalInfo> PortalList;

	PortalInfo *pMyPortalInfo;

	bool bTeleported;

	rmatrix matNearProjection, matObliqueNearProjection;

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

#ifdef PORTAL_USE_RT_TEXTURE
	void CreateTextures();
	void ReleaseTextures();
#endif


#ifdef PORTAL_USE_RT_TEXTURE
	void Render();
	void RenderWorld();
#else
	void RenderWorldStencil();
#endif
	void RenderEdge();

	bool ZObjectPortalIntersection(const ZObject *pObj, PortalInfo **retppi, int *piPortal);
	bool LinePortalIntersection(const D3DXVECTOR3 &L1, const D3DXVECTOR3 &L2, const PortalInfo &ppi, int iPortal, D3DXVECTOR3 &Hit);
	bool AABBPortalIntersection(const D3DXVECTOR3 &B1, const D3DXVECTOR3 &B2, const PortalInfo &ppi, int iPortal);
	static bool LineOBBIntersection(const D3DXMATRIX &matWorld, const D3DXMATRIX &matInvWorld, const D3DXVECTOR3 &extents, const D3DXVECTOR3 &L1, const D3DXVECTOR3 &L2, D3DXVECTOR3 &Hit);
	static bool LineOBBIntersection(const D3DXVECTOR3 &center, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &up, const D3DXVECTOR3 &extents, const D3DXVECTOR3 &L1, const D3DXVECTOR3 &L2, D3DXVECTOR3 &Hit);
	static bool LineAABBIntersection(const D3DXVECTOR3 &B1, const D3DXVECTOR3 &B2, const D3DXVECTOR3 &L1, const D3DXVECTOR3 &L2, D3DXVECTOR3 &Hit);
	static bool AABBAABBIntersection(const D3DXVECTOR3 mins1, const D3DXVECTOR3 maxs1, const D3DXVECTOR3 mins2, const D3DXVECTOR3 maxs2);

	bool CheckIntersection(const D3DXVECTOR3 &target, float fRadius, float fHeight, PortalInfo **retppi, int *piPortal);

	static void MakeOrientationMatrix(D3DXMATRIX &mat, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &up);
	static void MakeViewMatrix(D3DXMATRIX &mat, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &up);
	static void MakeProjectionMatrix(D3DXMATRIX &mat, float NearZ = 5, float FarZ = 1000000);
	static void MakeObliquelyClippingProjectionMatrix(D3DXMATRIX &matProjection, const D3DXMATRIX &matView, const D3DXVECTOR3 &p, const D3DXVECTOR3 &normal);
	static void MakePlane(D3DXPLANE &plane, const D3DXVECTOR3 &v, const D3DXVECTOR3 &u, const D3DXVECTOR3 &origin);

	void Transform(D3DXVECTOR3 &v, const PortalInfo &ppi, int iPortal);
	void Transform(D3DXVECTOR3 &vOut, const D3DXVECTOR3 &vIn, const PortalInfo &ppi, int iPortal);
	void Rotate(D3DXVECTOR3 &v, const PortalInfo &ppi, int iPortal);
	void RotateReflect(D3DXVECTOR3 &v, const PortalInfo &ppi, int iPortal);

	void DrawTextF(int x, int y, const char *szFmt, ...);

public:
	Portal();

	void OnInvalidate();
	void OnRestore();

	void OnShot();
	bool RedirectPos(D3DXVECTOR3 &from, D3DXVECTOR3 &to);
	void RedirectCamera();

	void AddPlayer(ZCharacter *pZChar);
	void DeletePlayer(ZCharacter *pZChar);

	void DrawFakeCharacter(ZCharacter *pZChar);

	bool Move(ZObject *pObj, D3DXVECTOR3 &diff);

	void Update();

	void PreDraw();
	void PostDraw();
	void PostCameraUpdate();

	void CreatePortal(ZCharacter *pZChar, int iPortal, const D3DXVECTOR3 &vPos, const D3DXVECTOR3 &vNormal, const D3DXVECTOR3 &vUp);

	bool ForceProjection(){
		return bForceProjection;
	}
};

extern Portal *g_pPortal;
#endif