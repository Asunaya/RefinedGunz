#include "stdafx.h"
#include "MWidget.h"
#include "portal.h"
#ifdef PORTAL
#include "ZCamera.h"
#include "ZCombatInterface.h"
#include "ZGame.h"
#include "ZMyCharacter.h"
#include "ZGlobal.h"
#include "RTypes.h"
#include "RMeshUtil.h"
#include "RGMain.h"
#include "ZGameInterface.h"
#include "ZGameInput.h"
#include "ZConfiguration.h"

Portal *g_pPortal = nullptr;

const D3DXVECTOR3 Portal::vDim = rvector(100, 150, 0);
bool Portal::bPortalSetExists = false;

static bool LineOBBIntersection(const D3DXMATRIX &matWorld, const D3DXMATRIX &matInvWorld, const D3DXVECTOR3 &extents, const D3DXVECTOR3 &L1, const D3DXVECTOR3 &L2, D3DXVECTOR3 &Hit);
static bool LineOBBIntersection(const D3DXVECTOR3 &center, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &up, const D3DXVECTOR3 &extents, const D3DXVECTOR3 &L1, const D3DXVECTOR3 &L2, D3DXVECTOR3 &Hit);
static bool LineAABBIntersection(const D3DXVECTOR3 &B1, const D3DXVECTOR3 &B2, const D3DXVECTOR3 &L1, const D3DXVECTOR3 &L2, D3DXVECTOR3 &Hit);
static bool AABBAABBIntersection(const D3DXVECTOR3 mins1, const D3DXVECTOR3 maxs1, const D3DXVECTOR3 mins2, const D3DXVECTOR3 maxs2);
static void MakeOrientationMatrix(D3DXMATRIX &mat, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &up);
static void MakeViewMatrix(D3DXMATRIX &mat, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &up);
static void MakeProjectionMatrix(D3DXMATRIX &mat, float NearZ = 5, float FarZ = 1000000);
static void MakeObliquelyClippingProjectionMatrix(D3DXMATRIX &matProjection, const D3DXMATRIX &matView, const D3DXVECTOR3 &p, const D3DXVECTOR3 &normal);
static void MakePlane(D3DXPLANE &plane, const D3DXVECTOR3 &v, const D3DXVECTOR3 &u, const D3DXVECTOR3 &origin);

Portal::Portal() : ValidPortals(PortalList)
{
	static constexpr int coords[] = { 200, 20,
		287, 50,
		363, 159,
		384, 372,
		290, 555,
		200, 586,
		118, 561,
		19, 388,
		35, 163,
		109, 49,
	};

	static constexpr int NumVertices = sizeof(coords) / sizeof(coords[0]) / 2;

	DWORD dwFVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
	D3DXCreateMeshFVF(4, 4, D3DXMESH_MANAGED, dwFVF, RGetDevice(), &pRectangleMesh);
	D3DXCreateMeshFVF(NumVertices - 2, NumVertices, D3DXMESH_MANAGED, dwFVF, RGetDevice(), &pEdgeMesh);

	struct D3DVERTEX
	{
		D3DXVECTOR3 p;
		D3DXVECTOR3 n;
		FLOAT       tu, tv;
	};

	D3DVERTEX*              pSrc;

	pRectangleMesh->LockVertexBuffer(0, (void**)&pSrc);

	//left bottom point
	pSrc[0].p.x = -100;
	pSrc[0].p.y = -150;
	pSrc[0].p.z = 0;

	//left top point
	pSrc[1].p.x = -100;
	pSrc[1].p.y = 150;
	pSrc[1].p.z = 0;

	//right top point
	pSrc[2].p.x = 100;
	pSrc[2].p.y = 150;
	pSrc[2].p.z = 0;

	//right bottom point
	pSrc[3].p.x = 100;
	pSrc[3].p.y = -150;
	pSrc[3].p.z = 0;


	pSrc[0].tu = 0;
	pSrc[0].tv = 0;

	pSrc[1].tu = 0;
	pSrc[1].tv = 1;

	pSrc[2].tu = 1;
	pSrc[2].tv = 1;

	pSrc[3].tu = 1;
	pSrc[3].tv = 0;

	pRectangleMesh->UnlockVertexBuffer();

	WORD* pIndexBuffer = 0;
	pRectangleMesh->LockIndexBuffer(0, (void**)&pIndexBuffer);

	pIndexBuffer[0] = 0; pIndexBuffer[1] = 1; pIndexBuffer[2] = 2;
	pIndexBuffer[3] = 0; pIndexBuffer[4] = 2; pIndexBuffer[5] = 3;
	pRectangleMesh->UnlockIndexBuffer();

	pEdgeMesh->LockVertexBuffer(0, (void**)&pSrc);

	for (int i = 0; i < NumVertices; i++)
	{
		pSrc[i].p.x = float(coords[i * 2] - 200) / 400 * 200;
		pSrc[i].p.y = float(coords[i * 2 + 1] - 300) / 600 * 300;
		pSrc[i].p.z = 0;
		pSrc[i].tu = float(coords[i * 2]) / 400;
		pSrc[i].tv = float(coords[i * 2 + 1]) / 600;
	}

	pEdgeMesh->UnlockVertexBuffer();

	pIndexBuffer = 0;
	pEdgeMesh->LockIndexBuffer(0, (void**)&pIndexBuffer);

	// fill in the front face index data
	for (int i = 0; i < NumVertices - 2; i++)
	{
		pIndexBuffer[i * 3] = 0;
		pIndexBuffer[i * 3 + 1] = i + 1;
		pIndexBuffer[i * 3 + 2] = i + 2;
	}
	pEdgeMesh->UnlockIndexBuffer();

	// set material
	Mat.Diffuse = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
	Mat.Ambient = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
	Mat.Specular = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
	Mat.Emissive = D3DXCOLOR(0.f, 0.f, 0.f, 0.f);
	Mat.Power = 0.f;

#ifdef PORTAL_USE_RT_TEXTURE
	CreateTextures();

	g_flOnLost.AddMethod(this, &Portal::ReleaseTextures);
	g_flOnReset.AddMethod(this, &Portal::CreateTextures);
#endif

	GenerateTexture(RGetDevice(), &BlackTex.ptr, 0xFF000000);

	for (int i = 0; i < 2; i++)
	{
		char path[128];
		sprintf(path, "Interface/default/portal%d.png", i + 1);
		auto ret = ReadMZFile(path);

		if (!ret.first)
		{
			MLog("Failed to load portal %d texture file %s\n", i + 1, path);
			break;
		}

		if (FAILED(D3DXCreateTextureFromFileInMemory(RGetDevice(), ret.second.data(), ret.second.size(), &PortalEdgeTex[i].ptr)))
		{
			MLog("Failed to create portal %d texture\n", i + 1);
			break;
		}
	}

	bLastLClick = false;
	bLastRClick = false;

	pMyPortalInfo = 0;

	bDontDraw = false;
	bDontDrawChar = false;

	bForceProjection = false;
	bLookingThroughPortal = false;

#ifndef PORTAL_USE_RT_TEXTURE
	if (!RIsStencilBuffer())
		MLog("Portal::Portal() - Your graphics card doesn't support stencil buffers; no portals will be drawn.\n");
#endif
}

Portal::~Portal()
{
}

#ifdef PORTAL_USE_RT_TEXTURE
void Portal::CreateTextures()
{
	const int Width = g_pDraw->GetWidth();
	const int Height = g_pDraw->GetHeight();

	for (int i = 0; i < 2; i++)
	{
		if (FAILED(RGetDevice()->CreateTexture(Width,
			Height,
			1,
			D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8,
			D3DPOOL_MANAGED,
			&pTex[i],
			NULL)))
			MLog("CreateTexture failed.\n");

		if (FAILED(pTex[i]->GetSurfaceLevel(0, &pSurf[i])))
			MLog("GetSurfaceLevel failed.\n");
	}

	if (FAILED(RGetDevice()->CreateTexture(Width,
		Height,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_MANAGED,
		&pDummyTex,
		NULL)))
		MLog("CreateTexture failed.\n");

	if (FAILED(pDummyTex->GetSurfaceLevel(0, &pDummySurf)))
		MLog("GetSurfaceLevel failed.\n");
}

void Portal::ReleaseTextures()
{
	for (int i = 0; i < 2; i++)
	{
		SAFE_RELEASE(pTex[i]);
		SAFE_RELEASE(pSurf[i]);
	}

	SAFE_RELEASE(pDummyTex);
	SAFE_RELEASE(pDummySurf);
}
#endif

void Portal::OnLostDevice()
{
}

void Portal::OnResetDevice()
{
}

void Portal::Transform(D3DXVECTOR3 &v, const PortalInfo &portalinfo)
{
	Transform(v, v, portalinfo);
}

void Portal::Transform(D3DXVECTOR3 &vOut, const D3DXVECTOR3 &vIn, const PortalInfo &portalinfo)
{
	rvector Displacement = vIn - portalinfo.vPos;

	Rotate(Displacement, portalinfo);

	vOut = portalinfo.Other->vPos + Displacement;
}

void Portal::Rotate(D3DXVECTOR3 &v, const PortalInfo &portalinfo)
{
	v = v * portalinfo.matRot;
}

#ifdef PORTAL_USE_RT_TEXTURE
void Portal::RenderWorld()
{
	D3DXMATRIX mOrigView;

	RGetDevice()->GetTransform(D3DTS_VIEW, &mOrigView);

	IDirect3DSurface9 *pOrigRT;

	RGetDevice()->GetRenderTarget(0, &pOrigRT);

	/*D3DVIEWPORT9 OrigVP, VP;

	RGetDevice()->GetViewport(&OrigVP);
	VP = OrigVP;
	VP.Width = 1000;
	VP.Height = 1500;
	RGetDevice()->SetViewport(&VP);*/

	for (int i = 0; i < 2; i++)
	{
		RGetDevice()->SetRenderTarget(0, pSurf[i]);

		RGetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0.0f);

		D3DXVECTOR3 pos = vPos[!i];
		D3DXVECTOR3 camtoportal = vPos[i] - ZGetCamera()->m_Position;
		Normalize(camtoportal);
		D3DXVECTOR3 a = -vNormal[i];
		D3DXVECTOR3 b = vNormal[!i];


		rvector Displacement = ZGetCamera()->m_Position - vPos[i];
		rvector dir = ZGetCamera()->GetCurrentDir();// camtoportal;

		Rotate(Displacement, i);

		Rotate(dir, i);

		/*extern bool g_bForceView;
		extern D3DXMATRIX g_mView;*/

		pos = vPos[!i] + Displacement;
		D3DXVECTOR3 at = pos + dir, up = vUp[!i];

		D3DXMatrixLookAtLH(&matView[i], &pos, &at, &up);

		rmatrix mProjection;

		float fAspect = float(g_pDraw->GetWidth()) / g_pDraw->GetHeight();
		float fovy = atanf(tanf(1.22173048f / 2.0f) / fAspect) * 2.0f;

		rvector vec = pos - vPos[!i];

		extern bool g_bForceProjection;
		extern rmatrix g_mProjection;
		g_bForceProjection = true;

		D3DXMatrixPerspectiveFovLH(&g_mProjection, fovy, fAspect, D3DXVec3Length(&vec) + 10.f, 30000.f);

		/*rmatrix mView, mv, mRot, mIWorld;
		rvector right = rvector(0.f, 1.f, 0.f);
		mv = mOrigView * matWorld[i];
		D3DXMatrixRotationAxis(&mRot, &right, portalinfo);
		mIWorld = matWorld[!i];
		D3DXMatrixInverse(&mIWorld, NULL, &mIWorld);
		mView = mv * mRot * mIWorld;*/

		/*D3DXMATRIX mView, mat;

		D3DXQUATERNION Rot;

		D3DXVECTOR3 curportalpos = vPos[i],
			curportaldir = -vNormal[i],
			otherportalpos = vPos[!i],
			otherportaldir = vNormal[!i];

		D3DXVECTOR3 Axis;
		D3DXVec3Cross(&Axis, &curportaldir, &otherportaldir);
		D3DXVec3Normalize(&Axis, &Axis);
		float Angle = D3DXVec3Dot(&curportaldir, &otherportaldir);

		Rot.x = sin(Angle / 2) * Axis.x;
		Rot.y = sin(Angle / 2) * Axis.y;
		Rot.z = sin(Angle / 2) * Axis.z;
		Rot.w = cos(Angle / 2);

		D3DXVECTOR3 Trans;
		Trans = otherportalpos - curportalpos;

		D3DXMatrixAffineTransformation(&mat, 1.f, &otherportalpos, &Rot, &Trans);

		mView = mOrigView * mat;*/

		RGetDevice()->SetTransform(D3DTS_VIEW, &matView[i]);

		//g_bForceView = true;

		ZGameDraw(ZGetGame());

		g_bForceProjection = false;

		//g_bForceView = false;

		/*RGetDevice()->SetRenderTarget(0, pOrigRT);

		RGetDevice()->SetTransform(D3DTS_VIEW, &mOrigView);

		ZGameDraw(ZGetGame());*/
	}

	//RGetDevice()->SetRenderTarget(0, pDummySurf);

	RGetDevice()->SetTransform(D3DTS_VIEW, &mOrigView);

	//ZGameDraw(ZGetGame());

	//RGetDevice()->SetViewport(&OrigVP);

	RGetDevice()->SetRenderTarget(0, pOrigRT);
	
	/*RECT dest;
	dest.top = 1080 - 300;
	dest.bottom = 1080;
	dest.left = 1920 - 1000;
	dest.right = 1920 - 500;
	RGetDevice()->StretchRect(pSurf[0], 0, pOrigRT, &dest, D3DTEXF_NONE);

	dest.top = 1080 - 300;
	dest.bottom = 1080;
	dest.left = 1920 - 500;
	dest.right = 1920;
	RGetDevice()->StretchRect(pSurf[1], 0, pOrigRT, &dest, D3DTEXF_NONE);*/

	SAFE_RELEASE(pOrigRT);
}
#endif

void Portal::RenderEdge(const PortalInfo& portalinfo)
{
	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	rmatrix matWorld;
	MakeWorldMatrix(&matWorld, portalinfo.vPos + portalinfo.vNormal * 0.5, portalinfo.vNormal, portalinfo.vUp);

	RGetDevice()->SetTransform(D3DTS_WORLD, &matWorld);

	RGetDevice()->SetFVF(pRectangleMesh->GetFVF());
	RGetDevice()->SetVertexShader(NULL);

	RGetDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	RGetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);

	// Use alpha testing to draw only the edge
	RGetDevice()->SetRenderState(D3DRS_ALPHAREF, 0x00000001);
	RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	RGetDevice()->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	RGetDevice()->SetTexture(0, PortalEdgeTex[portalinfo.Index]);

	pRectangleMesh->DrawSubset(0);

	RGetDevice()->SetTexture(0, NULL);

	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

#ifdef PORTAL_USE_RT_TEXTURE
void Portal::Render()
{
	IDirect3DStateBlock9 *pStateBlock = NULL;
	RGetDevice()->CreateStateBlock(D3DSBT_ALL, &pStateBlock);

	//RenderWorld();

	pStateBlock->Apply();

	for (int i = 0; i < 2; i++)
	{
		D3DXMATRIX mWorld, mView, mProjection;

		RGetDevice()->SetTransform(D3DTS_WORLD, &matWorld[i]);

		/*rvector at = ZGetCamera()->m_Position + ZGetCamera()->GetCurrentDir();
		D3DXMatrixLookAtLH(&mView, &ZGetCamera()->m_Position, &at, &vUp[i]);
		RGetDevice()->SetTransform(D3DTS_VIEW, &mView);*/

		auto WorldToScreen = [this, i](rvector &v){
			rmatrix mViewport;

			int x1 = 0, y1 = 0, x2 = g_pDraw->GetWidth(), y2 = g_pDraw->GetHeight();

			float RSwx = (float)(x2 - x1) / 2;
			float RSwy = (float)(y2 - y1) / 2;
			float RScx = (float)RSwx + x1;
			float RScy = (float)RSwy + y1;

			D3DXMatrixIdentity(&mViewport);
			mViewport._11 = RSwx;
			mViewport._22 = -RSwy;
			mViewport._41 = RScx;
			mViewport._42 = RScy;

			rmatrix mProjection;
			RGetDevice()->GetTransform(D3DTS_PROJECTION, &mProjection);

			rmatrix mat = matView[!i] * mProjection * mViewport;
			D3DXVec3TransformCoord(&v, &v, &mat);
		};

		rvector p[4];
		const int x = 100, y = 150;
		p[0] = rvector(-x, -y, 0) * matWorld[!i];
		p[1] = rvector(-x, y, 0) * matWorld[!i];
		p[2] = rvector(x, y, 0) * matWorld[!i];
		p[3] = rvector(x, -y, 0) * matWorld[!i];


		for (int j = 0; j < 4; j++)
		{
			WorldToScreen(p[j]);

			DrawTextF(5, 200 + g_Settings.nFontSize * (j + i * 4), "%f, %f", p[j].x, p[j].y);
		}

		/*struct D3DVERTEX
		{
			D3DXVECTOR3 p;
			D3DXVECTOR3 n;
			FLOAT       tu, tv;
		};

		D3DVERTEX*              pSrc;

		pMesh->LockVertexBuffer(0, (void**)&pSrc);

		for (int j = 0; j < 4; j++)
		{
			((D3DVERTEX*)pSrc)[j].tu = p[j].x / g_pDraw->GetWidth();// * 0.375;
			((D3DVERTEX*)pSrc)[j].tv = p[j].y / g_pDraw->GetHeight();
		}

		pMesh->UnlockVertexBuffer();*/

		//RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
		RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		//RGetDevice()->SetRenderState(D3DRS_FOGENABLE, FALSE);
		/*RGetDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);
		RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		RGetDevice()->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		RGetDevice()->SetRenderState(D3DRS_STENCILENABLE, FALSE);*/

		RGetDevice()->SetFVF(pMesh->GetFVF());

		RGetDevice()->SetVertexShader(NULL);

		RGetDevice()->SetTexture(0, pTex[i]);
		//for(int i = 1; i < 8; i++)
			//RGetDevice()->SetTexture(i, NULL);
		RGetDevice()->SetMaterial(&Mat);
		pMesh->DrawSubset(0);
	}

	pStateBlock->Apply();
	SAFE_RELEASE(pStateBlock);

	/*D3DXMATRIX mView;
	RGetDevice()->GetTransform(D3DTS_VIEW, &mView);

	auto DrawTextF = [](int x, int y, const char *szFmt, ...){
		char buf[256] = { 0 };
		vsnprintf(buf, sizeof(buf), szFmt, GETVA(szFmt));
		RECT Rectangle = { x, y, x + 1, y + 1 };
		g_pFont->DrawText(0, buf, -1, &Rectangle, DT_TOP | DT_LEFT | DT_NOCLIP, g_Settings.dwColor);
	};

	int itr = 0;

	DrawTextF(5, 200 + (g_Settings.nFontSize + 1) * itr++, "%f %f %f %f", mView._11, mView._12, mView._13, mView._14);
	DrawTextF(5, 200 + (g_Settings.nFontSize + 1) * itr++, "%f %f %f %f", mView._21, mView._22, mView._23, mView._24);
	DrawTextF(5, 200 + (g_Settings.nFontSize + 1) * itr++, "%f %f %f %f", mView._31, mView._32, mView._33, mView._34);
	DrawTextF(5, 200 + (g_Settings.nFontSize + 1) * itr++, "%f %f %f %f", mView._41, mView._42, mView._43, mView._44);
	
	D3DXVECTOR3 pos = ZGetCamera()->m_Position;
	D3DXVECTOR3 at = pos + ZGetGame()->m_pMyCharacter->GetDirection();
	D3DXVECTOR3 up = D3DXVECTOR3(0.f, 0.f, 1.f);
	D3DXMatrixLookAtLH(&mView, &pos, &at, &up);
	DrawTextF(5, 200 + (g_Settings.nFontSize + 1) * itr++, "%f %f %f %f", mView._11, mView._12, mView._13, mView._14);
	DrawTextF(5, 200 + (g_Settings.nFontSize + 1) * itr++, "%f %f %f %f", mView._21, mView._22, mView._23, mView._24);
	DrawTextF(5, 200 + (g_Settings.nFontSize + 1) * itr++, "%f %f %f %f", mView._31, mView._32, mView._33, mView._34);
	DrawTextF(5, 200 + (g_Settings.nFontSize + 1) * itr++, "%f %f %f %f", mView._41, mView._42, mView._43, mView._44);*/
}
#endif

static void GetIntersectedScreenLine(rvector &pos, rvector &dir)
{
	MPOINT Crosshair = ZGetCombatInterface()->GetCrosshairPoint();

	RGetScreenLine(Crosshair.x, Crosshair.y, &pos, &dir);

	rvector mypos = ZGetGame()->m_pMyCharacter->m_Position + rvector(0, 0, 100);
	rplane myplane;
	D3DXPlaneFromPointNormal(&myplane, &mypos, &dir);

	rvector checkpos, checkposto = pos + 100000.f*dir;
	D3DXPlaneIntersectLine(&checkpos, &myplane, &pos, &checkposto);
}

//#define _DEBUG

void Portal::OnShot()
{
	int n = -1;
#ifndef _DEBUG
	bool bLClick = !!(GetAsyncKeyState(VK_LBUTTON) & 0x8000), bRClick = !!(GetAsyncKeyState(VK_RBUTTON) & 0x8000);
#else
	bool bLClick = !!(GetAsyncKeyState('T') & 0x8000), bRClick = !!(GetAsyncKeyState('Y') & 0x8000);
#endif
	if (bLClick && !bLastLClick)
		n = 0;
	else if (bRClick && !bLastRClick)
		n = 1;
	else
	{
		bLastLClick = bLClick;
		bLastRClick = bRClick;
		return;
	}

	bLastLClick = bLClick;
	bLastRClick = bRClick;

	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_TRAINING)
		return;

#ifndef DEBUG
	MMatchItemDesc *pDesc = ZGetGame()->m_pMyCharacter->GetSelectItemDesc();

	if (!pDesc)
		return;

	if (pDesc->m_nID != 8500) // Change this to something saner later
		return;
#endif
	
	D3DXVECTOR3 Pos, Dir;
	GetIntersectedScreenLine(Pos, Dir);

	ZPICKINFO zpi;
	if (!ZGetGame()->Pick(ZGetGame()->m_pMyCharacter, Pos, Dir, &zpi, RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSBULLET) || !zpi.bBspPicked)
		return;

	rvector normal = D3DXVECTOR3(zpi.bpi.pInfo->plane.a, zpi.bpi.pInfo->plane.b, zpi.bpi.pInfo->plane.c);
	rvector pos = zpi.bpi.PickPos + normal * 5;
	rvector up = fabs(normal.z) != 1 ? rvector(0, 0, 1) : rvector(1, 0, 0);

	ZPostPortal(n, pos, normal, up);
}

D3DXVECTOR3 g_v[4];

bool Portal::RedirectPos(D3DXVECTOR3 &from, D3DXVECTOR3 &to)
{
	if (!bPortalSetExists)
		return false;

	for (auto pair : PortalList)
	{
		auto& portalpair = pair.second;

		for (int i = 0; i < 2; i++)
		{
			auto& portalinfo = portalpair[i];
			auto& other = portalpair[!i];

			D3DXVECTOR3 dim = D3DXVECTOR3(.1f, 100.f, 150.f);
			D3DXVECTOR3 v1 = portalinfo.vPos - dim;
			D3DXVECTOR3 v2 = portalinfo.vPos + dim;
			D3DXVECTOR3 hit;

			g_v[0] = from;
			g_v[1] = to;

			/*g_v[2] = v1;
			g_v[3] = v2;*/

			rvector dir = to - from;
			Normalize(dir);

			if (!LinePortalIntersection(from, to, portalinfo, hit))
				continue;

			/*D3DXMATRIX mat;

			D3DXVECTOR3 right, up;

			CrossProduct(&right, vUp[!i], dir);
			Normalize(right);

			CrossProduct(&up, right, dir);
			Normalize(up);

			mat._11 = right.x; mat._12 = right.y; mat._13 = right.z;
			mat._21 = up.x; mat._22 = up.y; mat._23 = up.z;
			mat._31 = dir.x; mat._32 = dir.y; mat._33 = dir.z;

			dir = dir * mat;*/

			rvector Displacement = hit - portalinfo.vPos;

			Rotate(Displacement, portalinfo);

			Rotate(dir, portalinfo);

			from = other.vPos + Displacement;

			/*ZPICKINFO ZPickInfo;
			if (ZGamePick(*pZGame, ZGetGame()->m_pMyCharacter, &from, &dir, &ZPickInfo, RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSBULLET, 0))
			if (ZPickInfo.bBspicked){
			to = ZPickInfo.bpi.PickPos;
			}*/

			to = from + dir * 10000;

			g_v[2] = from;
			g_v[3] = to;

			//cprint("vPos[!i]: %f, %f, %f; Displacement: %f, %f, %f", pi.vPos[!i].x, pi.vPos[!i].y, pi.vPos[!i].z, Displacement.x, Displacement.y, Displacement.z);
			//cprint("From: %f, %f, %f; to: %f, %f, %f", from.x, from.y, from.z, to.x, to.y, to.z);

			return true;
		}
	}

	return false;
}

void Portal::DrawFakeCharacter(ZCharacter *pZChar)
{
	if (!bPortalSetExists || bDontDrawChar)
		return;

	PortalInfo *ppi;

	if (!CheckIntersection(pZChar->GetPosition(), 100, 190, &ppi))
		return;

	
	rvector &pos = pZChar->m_Position; // Change bounding box
	rmatrix &mat = pZChar->GetVisualMesh()->m_WorldMat;

	rvector oldpos = pos;

	rvector newpos = pos * ppi->matTransform;

	rmatrix oldmat = mat;

	mat = mat * ppi->matTransform;

	pos = newpos;


	bDontDrawChar = true;
	pZChar->Draw();
	bDontDrawChar = false;


	pos = oldpos;

	mat = oldmat;
}

bool Portal::Move(ZObject *pObj, D3DXVECTOR3 &diff)
{
	if (!bPortalSetExists)
		return false;

	rvector &origin = pObj->m_Position;
	rvector target = origin + diff;
	rvector hit;

	PortalInfo *ppi;

	if (!CheckIntersection(target, pObj->GetCollRadius(), pObj->GetCollHeight(), &ppi))
		return false;

	if (DotProduct(target, ppi->vNormal) + ppi->d < 0)
	{
		origin = target * ppi->matTransform;
		pObj->SetDirection(pObj->GetDirection() * ppi->matRot);
		pObj->SetVelocity(pObj->GetVelocity() * ppi->matRot);

		ZGetGameInterface()->GetGameInput()->lastanglex = ZGetCamera()->m_fAngleX;
		ZGetGameInterface()->GetGameInput()->lastanglez = ZGetCamera()->m_fAngleZ;

		RCameraPosition *= ppi->matTransform;
		RCameraDirection *= ppi->matRot;
		RUpdateCamera();

		ZCharacter *pChar = MDynamicCast(ZCharacter, pObj);

		if (pChar)
		{
			pChar->m_DirectionLower = pChar->m_Direction;
			pChar->m_DirectionUpper = pChar->m_Direction;
		}
	}
	else
		origin = target;

	return true;
}

bool Portal::Move(ZMovingWeapon & Obj, v3 & diff)
{
	if (!bPortalSetExists)
		return false;

	v3& origin = Obj.m_Position;
	v3 target = origin + diff;

	PortalInfo* pi = LineIntersect(origin, target);

	if (!pi)
		return false;

	if (DotProduct(target, pi->vNormal) + pi->d < 0)
	{
		DMLog("Portal::Move(ZMovingWeapon&, v3&) -- Old vel %f, %f, %f\n", Obj.m_Velocity.x, Obj.m_Velocity.y, Obj.m_Velocity.z);
		origin *= pi->matTransform;
		Obj.m_Dir *= pi->matRot;
		Obj.m_Velocity *= pi->matRot;
		diff *= pi->matRot;
		DMLog("Portal::Move(ZMovingWeapon&, v3&) -- New vel %f, %f, %f\n", Obj.m_Velocity.x, Obj.m_Velocity.y, Obj.m_Velocity.z);
	}

	return true;
}

void Portal::RedirectCamera()
{
	if(!bPortalSetExists)
		return;

	bMakeNearProjectionMatrix = false;
	bLookingThroughPortal = false;

	vCameraPos = RCameraPosition;
	vCameraDir = RCameraDirection;

	rvector charpos = ZGetGame()->m_pMyCharacter->GetPosition(), target, offset;
	rvector forward, right, up;
	forward = vCameraDir;
	up = rvector(0, 0, 1);
	CrossProduct(&right, up, forward);

#define CAMERA_TRACK_OFFSET		rvector(0,0,160)
#define CAMERA_RIGHT_OFFSET		rvector(0,30.f,10.f)

	offset = CAMERA_TRACK_OFFSET;

	offset += CAMERA_RIGHT_OFFSET.x * forward + CAMERA_RIGHT_OFFSET.y * right + CAMERA_RIGHT_OFFSET.z * up;

	target = charpos + offset;

	bool bInsidePortal = false;

	PortalInfo *ppi;
	if (ZObjectPortalIntersection(ZGetGame()->m_pMyCharacter, &ppi))
	{
		if (DotProduct(target - ZGetCamera()->m_fDist * vCameraDir, ppi->vNormal) + ppi->d < 0)
		{
			bInsidePortal = true;
		}
	}

	rvector newpos, newdir;

	if (bInsidePortal)
	{
		newdir = vCameraDir * ppi->matRot;
		newpos = target * ppi->matTransform;
		newpos -= ZGetCamera()->m_fDist * newdir;

		bMakeNearProjectionMatrix = DotProduct(newpos, ppi->Other->vNormal) + ppi->Other->d < 15;
	}
	else
	{
		ZPICKINFO zpi;

		rvector dir = -ZGetCamera()->GetCurrentDir();

		rvector idealpos = target + dir * ZGetCamera()->m_fDist;

		if (!(ZGetGame()->Pick(ZGetGame()->m_pMyCharacter, target, dir, &zpi, RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSBULLET, 0)
			&& zpi.bBspPicked
			&& Magnitude(target - zpi.bpi.PickPos) < ZGetCamera()->m_fDist))
			//{
			//cprint("No hit\n");
			return;
		//}

		rvector &pick = zpi.bpi.PickPos;

		rvector hit;
		bool Found = false;
		for (auto pair : PortalList)
		{
			for (int i = 0; i < 2; i++)
			{
				auto& pi = pair.second[i];

				if (LinePortalIntersection(target, pick, pi, hit))
				{
					ppi = &pi;
					Found = true;
					break;
				}
			}
		}

		if (!Found)
			return;

		int axis = -1;

		for (int i = 0; i < 3; i++)
		{
			if (fabs(hit[i] - idealpos[i]) > 1)
			{
				axis = i;
				break;
			}
		}

		if (axis == -1)
			return;

		rvector diff = idealpos - target;
		float distoutside = (idealpos[axis] - hit[axis]) / (idealpos[axis] - target[axis]) * D3DXVec3Length(&diff);

		rvector disp = hit;
		Transform(disp, *ppi);

		newdir = -dir;
		Rotate(newdir, *ppi);

		newpos = disp - newdir * distoutside;

		bMakeNearProjectionMatrix = distoutside < 15;
	}

	/*pZCam->m_Position = newpos;
	pZCam->SetDirection(-newdir);*/

	D3DXMATRIX mView;

	MakeViewMatrix(mView, newpos, newdir, D3DXVECTOR3(0, 0, 1));
	RGetDevice()->SetTransform(D3DTS_VIEW, &mView);

	vCameraPos = newpos;
	vCameraDir = newdir;

	bLookingThroughPortal = true;

	auto ComputeViewFrustum = [&](rplane *plane, float x, float y, float z)
	{
		/*
		rmatrix RView;
		RGetDevice()->GetTransform(D3DTS_VIEW,&RView);
		*/
		plane->a = mView._11*x + mView._12*y + mView._13*z;
		plane->b = mView._21*x + mView._22*y + mView._23*z;
		plane->c = mView._31*x + mView._32*y + mView._33*z;
		plane->d = -plane->a*vCameraPos.x
			- plane->b*vCameraPos.y
			- plane->c*vCameraPos.z;
	};

	auto ComputeZPlane = [&](rplane *plane, float z, int sign)
	{
		static rvector normal, t;
		t = vCameraPos + z * vCameraDir;
		normal = float(sign) * vCameraDir;
		D3DXVec3Normalize(&normal, &normal);
		plane->a = normal.x; plane->b = normal.y; plane->c = normal.z;
		plane->d = -plane->a*t.x - plane->b*t.y - plane->c*t.z;
	};

	float RFov_horiz = g_fFOV;
	float RFov_vert = atanf(tanf(RFov_horiz / 2.0f) / (float(RGetScreenWidth()) / RGetScreenHeight()))*2.0f;
	float fovh2 = RFov_horiz / 2.0f, fovv2 = RFov_vert / 2.0f;
	float ch = cosf(fovh2), sh = sinf(fovh2);
	float cv = cosf(fovv2), sv = sinf(fovv2);

	ComputeViewFrustum(RViewFrustum + 0, -ch, 0, sh);
	ComputeViewFrustum(RViewFrustum + 1, ch, 0, sh);
	ComputeViewFrustum(RViewFrustum + 2, 0, cv, sv);
	ComputeViewFrustum(RViewFrustum + 3, 0, -cv, sv);
	ComputeZPlane(RViewFrustum + 4, bMakeNearProjectionMatrix ? 0.1 : 5.0, 1);
	ComputeZPlane(RViewFrustum + 5, 1000000.0, -1);

	//cprint("distoutside: %f\n", distoutside);
}

PortalInfo* Portal::LineIntersect(const v3& l1, const v3& l2)
{
	for (auto& pi : ValidPortals)
	{
		v3 hit;
		if (LinePortalIntersection(l1, l2, pi, hit))
		{
			return &pi;
		}
	}

	return nullptr;
}

bool Portal::CheckIntersection(const D3DXVECTOR3 &target, float fRadius, float fHeight, PortalInfo **pppi)
{
	rvector mins = target - rvector(fRadius, fRadius, 0);
	rvector maxs = target + rvector(fRadius, fRadius, fHeight);

	for (auto &pair : PortalList)
	{
		for (int i = 0; i < 2; i++)
		{
			auto& portalinfo = pair.second[i];

			if (AABBPortalIntersection(mins, maxs, portalinfo))
			{
				if(pppi)
					*pppi = &portalinfo;

				return true;
			}
		}
	}

	return false;
}

bool Portal::ZObjectPortalIntersection(const ZObject *pObj, PortalInfo **retppi)
{
	return CheckIntersection(pObj->m_Position, pObj->GetCollRadius(), pObj->GetCollHeight(), retppi);
}

bool Portal::LinePortalIntersection(const D3DXVECTOR3 &L1, const D3DXVECTOR3 &L2, const PortalInfo &portalinfo, D3DXVECTOR3 &Hit)
{
	return LineOBBIntersection(portalinfo.matWorld, portalinfo.matInvWorld, vDim, L1, L2, Hit);
}

static bool LineOBBIntersection(const D3DXMATRIX &matWorld, const D3DXMATRIX &matInvWorld, const D3DXVECTOR3 &extents, const D3DXVECTOR3 &L1, const D3DXVECTOR3 &L2, D3DXVECTOR3 &Hit)
{
	rvector mins, maxs, l1, l2;

	mins = -extents;
	maxs = extents;

	l1 = L1 * matInvWorld;
	l2 = L2 * matInvWorld;

	bool bRet = LineAABBIntersection(mins, maxs, l1, l2, Hit);

	if (!bRet)
		return false;

	Hit = Hit * matWorld;

	return true;
}

static bool LineOBBIntersection(const D3DXVECTOR3 &center, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &up, const D3DXVECTOR3 &extents, const D3DXVECTOR3 &L1, const D3DXVECTOR3 &L2, D3DXVECTOR3 &Hit)
{
	rvector mins, maxs, l1, l2;

	mins = -extents;
	maxs = extents;

	rmatrix mat, imat;

	MakeWorldMatrix(&mat, center, dir, up);

	D3DXMatrixInverse(&imat, NULL, &mat);

	l1 = L1 * imat;
	l2 = L2 * imat;

	bool bRet = LineAABBIntersection(mins, maxs, l1, l2, Hit);

	if (!bRet)
		return false;

	Hit = Hit * mat;

	return true;
}

static bool LineAABBIntersection(const D3DXVECTOR3 &B1, const D3DXVECTOR3 &B2, const D3DXVECTOR3 &L1, const D3DXVECTOR3 &L2, D3DXVECTOR3 &Hit)
{
	auto GetIntersection = [](float fDst1, float fDst2, const D3DXVECTOR3 &P1, const D3DXVECTOR3 &P2, D3DXVECTOR3 &Hit) -> bool {
		if ((fDst1 * fDst2) >= 0.0f) return 0;
		if (fDst1 == fDst2) return 0;
		Hit = P1 + (P2 - P1) * (-fDst1 / (fDst2 - fDst1));
		return 1;
	};

	auto InBox = [](const D3DXVECTOR3 &Hit, const D3DXVECTOR3 &B1, const D3DXVECTOR3 &B2, const int Axis) -> bool {
		if (Axis == 1 && Hit.z > B1.z && Hit.z < B2.z && Hit.y > B1.y && Hit.y < B2.y) return 1;
		if (Axis == 2 && Hit.z > B1.z && Hit.z < B2.z && Hit.x > B1.x && Hit.x < B2.x) return 1;
		if (Axis == 3 && Hit.x > B1.x && Hit.x < B2.x && Hit.y > B1.y && Hit.y < B2.y) return 1;
		return 0;
	};

	if (L2.x < B1.x && L1.x < B1.x) return false;
	if (L2.x > B2.x && L1.x > B2.x) return false;
	if (L2.y < B1.y && L1.y < B1.y) return false;
	if (L2.y > B2.y && L1.y > B2.y) return false;
	if (L2.z < B1.z && L1.z < B1.z) return false;
	if (L2.z > B2.z && L1.z > B2.z) return false;
	if (L1.x > B1.x && L1.x < B2.x &&
		L1.y > B1.y && L1.y < B2.y &&
		L1.z > B1.z && L1.z < B2.z)
	{
		Hit = L1;
		return true;
	}

	if ((GetIntersection(L1.x - B1.x, L2.x - B1.x, L1, L2, Hit) && InBox(Hit, B1, B2, 1))
		|| (GetIntersection(L1.y - B1.y, L2.y - B1.y, L1, L2, Hit) && InBox(Hit, B1, B2, 2))
		|| (GetIntersection(L1.z - B1.z, L2.z - B1.z, L1, L2, Hit) && InBox(Hit, B1, B2, 3))
		|| (GetIntersection(L1.x - B2.x, L2.x - B2.x, L1, L2, Hit) && InBox(Hit, B1, B2, 1))
		|| (GetIntersection(L1.y - B2.y, L2.y - B2.y, L1, L2, Hit) && InBox(Hit, B1, B2, 2))
		|| (GetIntersection(L1.z - B2.z, L2.z - B2.z, L1, L2, Hit) && InBox(Hit, B1, B2, 3)))
		return true;

	return false;
}

bool Portal::AABBPortalIntersection(const D3DXVECTOR3 &B1, const D3DXVECTOR3 &B2, const PortalInfo &ppi)
{
	return AABBAABBIntersection(ppi.bb.vmin, ppi.bb.vmax, B1, B2);
}

static bool AABBAABBIntersection(const D3DXVECTOR3 mins1, const D3DXVECTOR3 maxs1, const D3DXVECTOR3 mins2, const D3DXVECTOR3 maxs2)
{
	if (mins1.x > maxs2.x)
		return false;

	if (maxs1.x < mins2.x)
		return false;

	if (mins1.y > maxs2.y)
		return false;

	if (maxs1.y < mins2.y)
		return false;

	if (mins1.z > maxs2.z)
		return false;

	if (maxs1.z < mins2.z)
		return false;

	return true;
}

static void MakeOrientationMatrix(D3DXMATRIX &mat, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &up)
{
	rvector right, upn;

	D3DXVec3Cross(&right, &up, &dir);
	D3DXVec3Normalize(&right, &right);

	D3DXVec3Cross(&upn, &right, &dir);
	D3DXVec3Normalize(&upn, &upn);

	D3DXMatrixIdentity(&mat);
	mat._11 = right.x; mat._12 = right.y; mat._13 = right.z;
	mat._21 = upn.x; mat._22 = upn.y; mat._23 = upn.z;
	mat._31 = -dir.x; mat._32 = -dir.y; mat._33 = -dir.z;
}

static void MakeViewMatrix(D3DXMATRIX &mat, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &up)
{
	rvector at = pos + dir;

	D3DXMatrixLookAtLH(&mat, &pos, &at, &up);
}

static void MakeProjectionMatrix(D3DXMATRIX &mat, float NearZ, float FarZ)
{
	float fAspect = float(RGetScreenWidth()) / RGetScreenHeight();
	float fovy = atanf(tanf(g_fFOV / 2.0f) / fAspect) * 2.0f;

	D3DXMatrixPerspectiveFovLH(&mat, fovy, fAspect, NearZ, FarZ);
}

static float sgn(float a)
{
	if (a > 0.0F) return (1.0F);
	if (a < 0.0F) return (-1.0F);
	return (0.0F);
}

static void MakeObliquelyClippingProjectionMatrix(D3DXMATRIX &matProjection, const D3DXMATRIX &matView, const D3DXVECTOR3 &p, const D3DXVECTOR3 &normal)
{
	//MakeProjectionMatrix(matProjection);

	rplane plane;

	D3DXPlaneFromPointNormal(&plane, &p, &normal);

	D3DXMATRIX WorldToView = matView;

	D3DXMatrixInverse(&WorldToView, nullptr, &WorldToView);
	D3DXMatrixTranspose(&WorldToView, &WorldToView);

	D3DXVECTOR4 projClipPlane;
	v4 vectorplane{ plane };

	// Transform clip plane into view space
	D3DXVec4Transform(&projClipPlane, &vectorplane, &WorldToView);

	//cprint("%f\n", projClipPlane.w);

	/*if (fabs(projClipPlane.w) < 15)
	{
		MakeProjectionMatrix(matProjection, 10.f, 10000.f);
		//cprint("returning\n");
		return;
	}*/
	
	MakeProjectionMatrix(matProjection);

	if (projClipPlane.w > 0)
	{
		D3DXVECTOR4 flippedPlane(-plane.a, -plane.b, -plane.c, -plane.d);

		D3DXVec4Transform(&projClipPlane, &flippedPlane, &WorldToView);
	}

	D3DXVECTOR4 q;

	q.x = (sgn(projClipPlane.x) + matProjection(2, 0)) / matProjection(0, 0);
	q.y = (sgn(projClipPlane.y) + matProjection(2, 1)) / matProjection(1, 1);
	q.z = -1.0F;
	q.w = (1.0F + matProjection(2, 2)) / matProjection(3, 2);

	D3DXVECTOR4 c = projClipPlane * (1.0F / D3DXVec4Dot(&projClipPlane, &q));

	matProjection(0, 2) = c.x;
	matProjection(1, 2) = c.y;
	matProjection(2, 2) = c.z;
	matProjection(3, 2) = c.w;
}

static void MakeObliquelyClippingProjectionMatrixNVIDIA(D3DXMATRIX &matProjection, const D3DXMATRIX &matView, const D3DXVECTOR3 &p, const D3DXVECTOR3 &normal)
{
	rplane plane;

	D3DXPlaneFromPointNormal(&plane, &p, &normal);

	MakeProjectionMatrix(matProjection);

	D3DXMATRIX WorldToProjection = matView * matProjection;

	D3DXMatrixInverse(&WorldToProjection, NULL, &WorldToProjection);
	D3DXMatrixTranspose(&WorldToProjection, &WorldToProjection);

	D3DXVECTOR4 clipPlane(plane.a, plane.b, plane.c, plane.d);
	D3DXVECTOR4 projClipPlane;

	// Transform clip plane into projection space
	D3DXVec4Transform(&projClipPlane, &clipPlane, &WorldToProjection);

	if (projClipPlane.w > 0)
	{
		// Flip!
		D3DXVECTOR4 clipPlane(-plane.a, -plane.b, -plane.c, -plane.d);

		D3DXVec4Transform(&projClipPlane, &clipPlane, &WorldToProjection);
	}

	matrix mat;
	D3DXMatrixIdentity(&mat);
	mat(0, 2) = projClipPlane.x;
	mat(1, 2) = projClipPlane.y;
	mat(2, 2) = projClipPlane.z;
	mat(3, 2) = projClipPlane.w;

	matProjection = matProjection * mat;
}

void Portal::DeletePlayer(ZCharacter *pZChar)
{
	auto it = PortalList.find(pZChar);
	if(it != PortalList.end())
		PortalList.erase(it);
}

static void MakePlane(D3DXPLANE &plane, const D3DXVECTOR3 &v, const D3DXVECTOR3 &u, const D3DXVECTOR3 &origin)
{
	D3DXVECTOR3 a = v - origin, b = u - origin, normal;
	CrossProduct(&normal, a, b);
	Normalize(normal);
	plane.a = normal.x;
	plane.b = normal.y;
	plane.c = normal.z;
	plane.d = -DotProduct(origin, normal);
}

static void MakePortalFrustum(plane (&Frustum)[6], const PortalInfo& pi, const v3& CamPos)
{
	MakePlane(Frustum[0], pi.topleft, pi.topright, CamPos);
	MakePlane(Frustum[1], pi.bottomright, pi.bottomleft, CamPos);
	MakePlane(Frustum[2], pi.topright, pi.bottomright, CamPos);
	MakePlane(Frustum[3], pi.bottomleft, pi.topleft, CamPos);
	memcpy(Frustum[4], pi.Near, sizeof(Frustum[0]));
	memcpy(Frustum[5], pi.Far, sizeof(Frustum[0]));
}

void Portal::Update(RecursionContext& rc)
{
	auto& Contexts = rc.Portals;

	auto Run = [&](PortalContext& Context)
	{
		const auto& pi = Context.pi;

		Context.IsVisible = isInViewFrustum(&pi.bb, rc.Cam.Frustum) && DotProduct(pi.vNormal, rc.Cam.Pos) + pi.d > 0;

		MakePortalFrustum(Context.Frustum, pi, rc.Cam.Pos * pi.Other->matTransform);
	};

	ForEachPortal(rc, Run);
}

void Portal::RenderPortals(const RecursionContext& rc)
{
	D3DXMATRIX mOrigWorld, mOrigView, mOrigProjection;
	rplane OrigViewFrustum[6];

	RGetDevice()->GetTransform(D3DTS_WORLD, &mOrigWorld);
	RGetDevice()->GetTransform(D3DTS_VIEW, &mOrigView);
	RGetDevice()->GetTransform(D3DTS_PROJECTION, &mOrigProjection);

	memcpy(OrigViewFrustum, RViewFrustum, sizeof(OrigViewFrustum));

	auto& Contexts = rc.Portals;

	for (size_t ctx = 0; ctx < Contexts.size(); ctx++)
	{
		auto RenderPortal = [&](int i)
		{
			const auto& Context = Contexts[ctx][i];
			const auto& portalinfo = Context.pi;

			if (!Context.IsVisible)
				return;

			if (rc.Depth > 1)
			{
				MLog("Depth %d, %d visible\n", rc.Depth, i);
			}

			// Draw black texture
			RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			RGetDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
			RGetDevice()->SetRenderState(D3DRS_STENCILENABLE, FALSE);

			RGetDevice()->SetTransform(D3DTS_WORLD, &portalinfo.matWorld);

			RGetDevice()->SetFVF(pEdgeMesh->GetFVF());
			RGetDevice()->SetVertexShader(nullptr);
			RGetDevice()->SetTexture(0, BlackTex);

			RGetDevice()->SetMaterial(&Mat);

			pEdgeMesh->DrawSubset(0);

			RGetDevice()->SetTransform(D3DTS_WORLD, &portalinfo.matWorld);

			// Write portal area to depth buffer, but not color buffer
			// Don't write to anywhere but the portal area
			RGetDevice()->SetRenderState(D3DRS_STENCILENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_STENCILREF, 0);
			RGetDevice()->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_NEVER);
			RGetDevice()->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
			RGetDevice()->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_INCR);
			RGetDevice()->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
			RGetDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
			RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

			pEdgeMesh->DrawSubset(0);

			RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
			RGetDevice()->SetRenderState(D3DRS_STENCILREF, 1);
			RGetDevice()->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);
			RGetDevice()->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
			RGetDevice()->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
			RGetDevice()->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
			RGetDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);

			rmatrix mView;

			D3DXVECTOR3 pos = rc.Cam.Pos * portalinfo.matTransform,
				dir = rc.Cam.Dir * portalinfo.matRot;

			D3DXVECTOR3 at = pos + dir, up = portalinfo.Other->vUp;

			D3DXMatrixLookAtLH(&mView, &pos, &at, &up);

			RGetDevice()->SetTransform(D3DTS_VIEW, &mView);

			rmatrix mProjection;
			MakeObliquelyClippingProjectionMatrix(mProjection, mView, portalinfo.Other->vPos, portalinfo.Other->vNormal);
			RGetDevice()->SetTransform(D3DTS_PROJECTION, &mProjection);

			memcpy(RViewFrustum, Context.Other->Frustum, sizeof(RViewFrustum));

			bForceProjection = true;
			bDontDraw = true;

			ZGetGame()->Draw();

			bDontDraw = false;
			bForceProjection = false;

			RGetDevice()->Clear(0, nullptr, D3DCLEAR_STENCIL, 0x00000000, 1.0f, 0);
		};

		int PortalIndex = 0;

		/*if (Magnitude(Contexts[ctx][1].pi.vPos - rc.Cam.Pos) > Magnitude(Contexts[ctx][0].pi.vPos - rc.Cam.Pos))
			PortalIndex = 1;*/

		RenderPortal(PortalIndex);

		RGetDevice()->SetTransform(D3DTS_VIEW, &mOrigView);
		RGetDevice()->SetTransform(D3DTS_PROJECTION, &mOrigProjection);

		//RGetDevice()->Clear(0, nullptr, D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);

		RenderPortal(!PortalIndex);
	}

	RGetDevice()->SetRenderState(D3DRS_STENCILENABLE, FALSE);

	RGetDevice()->SetTransform(D3DTS_WORLD, &mOrigWorld);
	RGetDevice()->SetTransform(D3DTS_VIEW, &mOrigView);
	RGetDevice()->SetTransform(D3DTS_PROJECTION, &mOrigProjection);

	memcpy(RViewFrustum, OrigViewFrustum, sizeof(RViewFrustum));
}

void Portal::WriteDepth(const RecursionContext& rc)
{
	RGetDevice()->Clear(0, nullptr, D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0.0f);

	D3DXMATRIX mOrigWorld, mOrigView, mOrigProjection;

	RGetDevice()->GetTransform(D3DTS_WORLD, &mOrigWorld);
	RGetDevice()->GetTransform(D3DTS_VIEW, &mOrigView);
	RGetDevice()->GetTransform(D3DTS_PROJECTION, &mOrigProjection);

	for (const auto& pair : rc.Portals)
	{
		for (int i = 0; i < 2; i++)
		{
			const auto& pc = pair[i];

			if (!pc.IsVisible)
				continue;

			const auto& pi = pc.pi;

			RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

			RGetDevice()->SetTransform(D3DTS_WORLD, &pi.matWorld);

			RGetDevice()->SetFVF(pRectangleMesh->GetFVF());
			RGetDevice()->SetVertexShader(nullptr);
			RGetDevice()->SetTexture(0, nullptr);

			RGetDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, 0);

			pEdgeMesh->DrawSubset(0);

			RGetDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		}

		for (int i = 0; i < 2; i++)
		{
			if (pair[i].IsVisible)
				RenderEdge(pair[i].pi);
		}
	}

	RGetDevice()->SetTransform(D3DTS_WORLD, &mOrigWorld);
	RGetDevice()->SetTransform(D3DTS_VIEW, &mOrigView);
	RGetDevice()->SetTransform(D3DTS_PROJECTION, &mOrigProjection);
}

void Portal::CreatePortal(ZCharacter *pZChar, int iPortal, const D3DXVECTOR3 &vPos,
	const D3DXVECTOR3 &vNormal,const D3DXVECTOR3 &vUp)
{
	auto& portalpair = PortalList[pZChar];
	auto& portalinfo = portalpair[iPortal];

	int n = iPortal;

	portalinfo.vPos = vPos;
	portalinfo.vNormal = vNormal;
	portalinfo.vUp = vUp;

	portalinfo.d = -DotProduct(portalinfo.vNormal, portalinfo.vPos);

	rvector Pos = portalinfo.vPos;
	rvector Dir = -portalinfo.vNormal;
	D3DXVECTOR3 local_up,
		right;

	rmatrix matTranslation;

	D3DXVec3Cross(&right, &vUp, &Dir);
	D3DXVec3Normalize(&right, &right);

	D3DXVec3Cross(&local_up, &right, &Dir);
	D3DXVec3Normalize(&local_up, &local_up);

	rvector dir = Dir;

	rmatrix mat;
	D3DXMatrixIdentity(&mat);
	mat._11 = right.x; mat._12 = right.y; mat._13 = right.z;
	mat._21 = local_up.x; mat._22 = local_up.y; mat._23 = local_up.z;
	mat._31 = -dir.x; mat._32 = -dir.y; mat._33 = -dir.z;

	D3DXMatrixTranslation(&matTranslation, Pos.x, Pos.y, Pos.z);

	portalinfo.matWorld = mat * matTranslation;

	D3DXMatrixInverse(&portalinfo.matInvWorld, NULL, &portalinfo.matWorld);

	for (int i = 0; i < 2; i++)
	{
		auto& pi = portalpair[i];

		rmatrix mat1, mat2;

		MakeOrientationMatrix(mat1, pi.vNormal, pi.vUp);
		MakeOrientationMatrix(mat2, -pi.Other->vNormal, pi.Other->vUp);
		D3DXMatrixInverse(&mat1, NULL, &mat1);

		pi.matRot = mat1 * mat2;

		D3DXMatrixTranslation(&mat1, -pi.vPos.x, -pi.vPos.y, -pi.vPos.z);
		D3DXMatrixTranslation(&mat2, pi.Other->vPos.x, pi.Other->vPos.y, pi.Other->vPos.z);

		pi.matTransform = mat1 * pi.matRot * mat2;
	}

	portalinfo.bSet = true;

	if (portalinfo.bSet && portalinfo.Other->bSet)
		bPortalSetExists = true;

	portalinfo.topright = vDim * portalinfo.matWorld;
	portalinfo.topleft = rvector(vDim.x, -vDim.y, vDim.z) * portalinfo.matWorld;
	portalinfo.bottomright = rvector(-vDim.x, vDim.y, vDim.z) * portalinfo.matWorld;
	portalinfo.bottomleft = rvector(-vDim.x, -vDim.y, vDim.z) * portalinfo.matWorld;

	portalinfo.bb.vmin = portalinfo.topright;
	portalinfo.bb.vmax = portalinfo.topright;

	for (int i = 0; i < 3; i++)
	{
		portalinfo.bb.vmin[i] = min(portalinfo.bb.vmin[i], portalinfo.topright[i]);
		portalinfo.bb.vmin[i] = min(portalinfo.bb.vmin[i], portalinfo.topleft[i]);
		portalinfo.bb.vmin[i] = min(portalinfo.bb.vmin[i], portalinfo.bottomright[i]);
		portalinfo.bb.vmin[i] = min(portalinfo.bb.vmin[i], portalinfo.bottomleft[i]);
		portalinfo.bb.vmax[i] = max(portalinfo.bb.vmax[i], portalinfo.topright[i]);
		portalinfo.bb.vmax[i] = max(portalinfo.bb.vmax[i], portalinfo.topleft[i]);
		portalinfo.bb.vmax[i] = max(portalinfo.bb.vmax[i], portalinfo.bottomright[i]);
		portalinfo.bb.vmax[i] = max(portalinfo.bb.vmax[i], portalinfo.bottomleft[i]);
	}

	// Near
	rvector normal = portalinfo.vNormal;
	portalinfo.Near.a = normal.x;
	portalinfo.Near.b = normal.y;
	portalinfo.Near.c = normal.z;
	portalinfo.Near.d = -DotProduct(normal, portalinfo.vPos);

	// Far
	normal = -portalinfo.vNormal;
	portalinfo.Far.a = normal.x;
	portalinfo.Far.b = normal.y;
	portalinfo.Far.c = normal.z;
	portalinfo.Far.d = -DotProduct(normal, portalinfo.vPos - normal * 100000);
}

static void MakeFrustum(D3DXPLANE (&Frustum)[6], D3DXMATRIX View, D3DXVECTOR3 Pos, D3DXVECTOR3 Dir, float Near = 5)
{
	auto ComputeViewFrustum = [&](rplane *plane, float x, float y, float z)
	{
		plane->a = View._11*x + View._12*y + View._13*z;
		plane->b = View._21*x + View._22*y + View._23*z;
		plane->c = View._31*x + View._32*y + View._33*z;
		plane->d = -plane->a*Pos.x
			- plane->b*Pos.y
			- plane->c*Pos.z;
	};

	auto ComputeZPlane = [&](rplane *plane, float z, int sign)
	{
		static rvector normal, t;
		t = Pos + z * Dir;
		normal = float(sign) * Dir;
		D3DXVec3Normalize(&normal, &normal);
		plane->a = normal.x; plane->b = normal.y; plane->c = normal.z;
		plane->d = -plane->a*t.x - plane->b*t.y - plane->c*t.z;
	};

	float RFov_horiz = g_fFOV;
	float RFov_vert = atanf(tanf(RFov_horiz / 2.0f) / (float(RGetScreenWidth()) / RGetScreenHeight()))*2.0f;
	float fovh2 = RFov_horiz / 2.0f, fovv2 = RFov_vert / 2.0f;
	float ch = cosf(fovh2), sh = sinf(fovh2);
	float cv = cosf(fovv2), sv = sinf(fovv2);

	ComputeViewFrustum(Frustum + 0, -ch, 0, sh);
	ComputeViewFrustum(Frustum + 1, ch, 0, sh);
	ComputeViewFrustum(Frustum + 2, 0, cv, sv);
	ComputeViewFrustum(Frustum + 3, 0, -cv, sv);
	ComputeZPlane(Frustum + 4, Near, 1);
	ComputeZPlane(Frustum + 5, 1000000.0, -1);
}

void Portal::UpdateAndRender(const RecursionContext* PrevContext)
{
	if (!PrevContext)
		RecursionCount = 0;

	if (RecursionCount >= 1)
		return;

	RecursionCount++;

	auto NumContexts = PortalList.size();

	auto it = PortalList.begin();
	auto Contexts = MAKE_STACK_ARRAY(PortalContextPair, NumContexts,
		[&](PortalContextPair* p, size_t pos) {
			new (p) PortalContextPair(it);
			it++;
	});

	RecursionContext rc(RecursionCount, ArrayView<PortalContextPair>{ Contexts.get(), NumContexts });

	CameraContext CurCamera;
	if (!PrevContext)
	{
		CurCamera.Pos = vCameraPos;
		CurCamera.Dir = vCameraDir;
		memcpy(CurCamera.Frustum, RViewFrustum, sizeof(CurCamera.Frustum));
	}
	else
	{
		CurCamera.Pos = PrevContext->Cam.Pos;
		CurCamera.Dir = PrevContext->Cam.Dir;
	}

	rc.Cam = CurCamera;

	auto Run = [this](RecursionContext& rc)
	{
		rmatrix OrigView;
		rmatrix View;
		rplane OrigFrustum[6];
		rplane Frustum[6];

		if (rc.Depth > 1)
		{
			memcpy(OrigFrustum, RViewFrustum, sizeof(OrigFrustum));

			RGetDevice()->GetTransform(D3DTS_VIEW, &OrigView);

			D3DXVECTOR3 pos = rc.Cam.Pos,
				dir = rc.Cam.Dir;

			D3DXVECTOR3 at = pos + dir, up = rc.ViewingPortal->vUp;

			D3DXMatrixLookAtLH(&View, &pos, &at, &up);
		}

		Update(rc);

		UpdateAndRender(&rc);

		if (rc.Depth > 1)
		{
			RGetDevice()->SetTransform(D3DTS_VIEW, &View);
		}

		RenderPortals(rc);

		WriteDepth(rc);

		if (rc.Depth > 1)
		{
			RGetDevice()->SetTransform(D3DTS_VIEW, &OrigView);

			memcpy(RViewFrustum, OrigFrustum, sizeof(RViewFrustum));
		}

		/*for (auto& pair : rc.Portals)
		{
			for (auto& portal : pair)
			{
				v3 CamPos = rc.Cam.Pos * portal.pi.matTransform;

				rmatrix world;
				D3DXMatrixIdentity(&world);
				RGetDevice()->SetTransform(D3DTS_WORLD, &world);
				RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
				RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
				RGetDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);
				RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
				RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
				RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
				RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

				auto Draw = [&](const v3& vec)
				{
					v3 Dir = vec - CamPos;
					Normalize(Dir);
					RDrawLine(CamPos, CamPos + Dir * 10000, 0xFFFF0000);
				};

				Draw(portal.pi.Other->topleft);
				Draw(portal.pi.Other->topright);
				Draw(portal.pi.Other->bottomleft);
				Draw(portal.pi.Other->bottomright);
			}
		}*/
	};

	if (!PrevContext)
	{
		Run(rc);
		return;
	}

	for (size_t i = 0; i < Contexts.size(); i++)
	{
		for (int PortalIndex = 0; PortalIndex < 2; PortalIndex++)
		{
			rc.ViewingPortal = &Contexts[i][PortalIndex].pi;

			if (!rc.Portals[i][PortalIndex].IsVisible)
				continue;

			rc.Cam.Pos = CurCamera.Pos * rc.ViewingPortal->matTransform;
			rc.Cam.Dir = CurCamera.Dir * rc.ViewingPortal->matRot;

			MakePortalFrustum(rc.Cam.Frustum, *rc.ViewingPortal->Other, rc.Cam.Pos);

			Run(rc);
		}
	}
}

void Portal::PreDraw()
{
	if (!bPortalSetExists)
		return;

	if (bDontDraw)
		return;

	//MLog("RecursionCount: %d, frames: %d\n", RecursionCount, g_nFrameCount);

#ifndef PORTAL_USE_RT_TEXTURE
	if (!RIsStencilBuffer())
		return;
#endif

	RedirectCamera();

	UpdateAndRender();

	if (bMakeNearProjectionMatrix)
	{
		rmatrix mProjection;
		MakeProjectionMatrix(mProjection, 0.01);
		RGetDevice()->SetTransform(D3DTS_PROJECTION, &mProjection);
		bForceProjection = true;
	}
	else
	{
		if (bLookingThroughPortal)
		{
			rmatrix mProjection;
			MakeProjectionMatrix(mProjection);
			RGetDevice()->SetTransform(D3DTS_PROJECTION, &mProjection);
			bForceProjection = true; // Setting the projection again would reset the frustum
		}
		else
		{
			bForceProjection = false;
		}
	}
}

void Portal::PostDraw()
{
	if (bDontDraw || !bPortalSetExists || !RIsStencilBuffer())
		return;

#ifndef PORTAL_USE_RT_TEXTURE
	if (!RIsStencilBuffer())
		return;
#endif
}

#endif