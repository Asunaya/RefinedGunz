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
#include "Config.h"
#include "ZPickInfo.h"

Portal *g_pPortal = nullptr;

const rvector Portal::vDim = rvector(100, 150, 0);
bool Portal::bPortalSetExists = false;

static bool LineOBBIntersection(const rmatrix &matWorld, const rmatrix &matInvWorld, const rvector &extents, const rvector &L1, const rvector &L2, rvector &Hit);
static bool LineOBBIntersection(const rvector &center, const rvector &dir, const rvector &up, const rvector &extents, const rvector &L1, const rvector &L2, rvector &Hit);
static bool LineAABBIntersection(const rvector &B1, const rvector &B2, const rvector &L1, const rvector &L2, rvector &Hit);
static bool AABBAABBIntersection(const rvector mins1, const rvector maxs1, const rvector mins2, const rvector maxs2);
static void MakeOrientationMatrix(rmatrix &mat, const rvector &dir, const rvector &up);
static void MakeObliquelyClippingProjectionMatrix(rmatrix &matProjection, const rmatrix &matView,
	const rvector &p, const rvector &normal);
static void MakePlane(D3DXPLANE &plane, const rvector &v, const rvector &u, const rvector &origin);
static rmatrix GetDefaultProjectionMatrix(float Near = DEFAULT_NEAR_Z, float Far = DEFAULT_FAR_Z) {
	return PerspectiveProjectionMatrixViewport(
		RGetScreenWidth(), RGetScreenHeight(),
		FIXED_FOV(g_fFOV),
		Near, Far);
}

struct D3DVERTEX
{
	v3 pos;
	float tu, tv;
};

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

	static constexpr int NumVertices = ArraySize(coords) / 2;

	D3DVERTEX RectangleVertices[] = {
		{{-100, -150, 0}, 0, 0 },
		{{-100, 150, 0}, 0, 1},
		{{100, 150, 0}, 1, 1},
		{{100, -150, 0}, 1, 0},
	};

	u16 RectangleIndices[] = {
		0, 1, 2,
		0, 2, 3,
	};

	RectangleMesh.Create(RectangleVertices, RectangleIndices);

	D3DVERTEX EdgeVertices[NumVertices];
	for (int i = 0; i < NumVertices; i++)
	{
		EdgeVertices[i].pos.x = float(coords[i * 2] - 200) / 400 * 200;
		EdgeVertices[i].pos.y = float(coords[i * 2 + 1] - 300) / 600 * 300;
		EdgeVertices[i].pos.z = 0;
		EdgeVertices[i].tu = float(coords[i * 2]) / 400;
		EdgeVertices[i].tv = float(coords[i * 2 + 1]) / 600;
	}

	u16 EdgeIndices[(NumVertices - 2) * 3];
	for (int i = 0; i < NumVertices - 2; i++)
	{
		EdgeIndices[i * 3] = 0;
		EdgeIndices[i * 3 + 1] = i + 1;
		EdgeIndices[i * 3 + 2] = i + 2;
	}

	EdgeMesh.Create(EdgeVertices, EdgeIndices);

	// set material
	Mat.Diffuse = { 1, 1, 1, 1 };
	Mat.Ambient = { 1, 1, 1, 1 };
	Mat.Specular = { 1, 1, 1, 1 };
	Mat.Emissive = { 0, 0, 0, 0 };
	Mat.Power = 0;

#ifdef PORTAL_USE_RT_TEXTURE
	CreateTextures();

	g_flOnLost.AddMethod(this, &Portal::ReleaseTextures);
	g_flOnReset.AddMethod(this, &Portal::CreateTextures);
#endif

	GenerateTexture(RGetDevice(), MakeWriteProxy(BlackTex), 0xFF000000);

	for (int i = 0; i < 2; i++)
	{
		char path[128];
		sprintf_safe(path, "Interface/default/portal%d.png", i + 1);
		PortalEdgeTex[i] = RBaseTexturePtr{ RCreateBaseTexture(path, RTextureType::Map) };
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
		MLog("Portal::Portal() - Your graphics card doesn't support stencil buffers;"
			"no portals will be drawn.\n");
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

void Portal::Transform(rvector &v, const PortalInfo &portalinfo)
{
	Transform(v, v, portalinfo);
}

void Portal::Transform(rvector &vOut, const rvector &vIn, const PortalInfo &portalinfo)
{
	rvector Displacement = vIn - portalinfo.vPos;

	Rotate(Displacement, portalinfo);

	vOut = portalinfo.Other->vPos + Displacement;
}

void Portal::Rotate(rvector &v, const PortalInfo &portalinfo)
{
	v = v * portalinfo.matRot;
}

#ifdef PORTAL_USE_RT_TEXTURE
void Portal::RenderWorld()
{
	rmatrix mOrigView;

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

		rvector pos = vPos[!i];
		rvector camtoportal = vPos[i] - ZGetCamera()->m_Position;
		Normalize(camtoportal);
		rvector a = -vNormal[i];
		rvector b = vNormal[!i];


		rvector Displacement = ZGetCamera()->m_Position - vPos[i];
		rvector dir = ZGetCamera()->GetCurrentDir();// camtoportal;

		Rotate(Displacement, i);

		Rotate(dir, i);

		/*extern bool g_bForceView;
		extern rmatrix g_mView;*/

		pos = vPos[!i] + Displacement;
		rvector at = pos + dir, up = vUp[!i];

		rmatrixLookAtLH(&matView[i], &pos, &at, &up);

		rmatrix mProjection;

		float fAspect = float(g_pDraw->GetWidth()) / g_pDraw->GetHeight();
		float fovy = atanf(tanf(1.22173048f / 2.0f) / fAspect) * 2.0f;

		rvector vec = pos - vPos[!i];

		extern bool g_bForceProjection;
		extern rmatrix g_mProjection;
		g_bForceProjection = true;

		rmatrixPerspectiveFovLH(&g_mProjection, fovy, fAspect, D3DXVec3Length(&vec) + 10.f, 30000.f);

		/*rmatrix mView, mv, mRot, mIWorld;
		rvector right = rvector(0.f, 1.f, 0.f);
		mv = mOrigView * matWorld[i];
		rmatrixRotationAxis(&mRot, &right, portalinfo);
		mIWorld = matWorld[!i];
		rmatrixInverse(&mIWorld, NULL, &mIWorld);
		mView = mv * mRot * mIWorld;*/

		/*rmatrix mView, mat;

		D3DXQUATERNION Rot;

		rvector curportalpos = vPos[i],
			curportaldir = -vNormal[i],
			otherportalpos = vPos[!i],
			otherportaldir = vNormal[!i];

		rvector Axis;
		D3DXVec3Cross(&Axis, &curportaldir, &otherportaldir);
		D3DXVec3Normalize(&Axis, &Axis);
		float Angle = D3DXVec3Dot(&curportaldir, &otherportaldir);

		Rot.x = sin(Angle / 2) * Axis.x;
		Rot.y = sin(Angle / 2) * Axis.y;
		Rot.z = sin(Angle / 2) * Axis.z;
		Rot.w = cos(Angle / 2);

		rvector Trans;
		Trans = otherportalpos - curportalpos;

		rmatrixAffineTransformation(&mat, 1.f, &otherportalpos, &Rot, &Trans);

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

	RSetTransform(D3DTS_WORLD, matWorld);

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

	RGetDevice()->SetTexture(0, PortalEdgeTex[portalinfo.Index].get()->GetTexture());

	RectangleMesh.Draw();

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
		rmatrix mWorld, mView, mProjection;

		RGetDevice()->SetTransform(D3DTS_WORLD, &matWorld[i]);

		/*rvector at = ZGetCamera()->m_Position + ZGetCamera()->GetCurrentDir();
		rmatrixLookAtLH(&mView, &ZGetCamera()->m_Position, &at, &vUp[i]);
		RGetDevice()->SetTransform(D3DTS_VIEW, &mView);*/

		auto WorldToScreen = [this, i](rvector &v){
			rmatrix mViewport;

			int x1 = 0, y1 = 0, x2 = g_pDraw->GetWidth(), y2 = g_pDraw->GetHeight();

			float RSwx = (float)(x2 - x1) / 2;
			float RSwy = (float)(y2 - y1) / 2;
			float RScx = (float)RSwx + x1;
			float RScy = (float)RSwy + y1;

			GetIdentityMatrix(mViewport);
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
			rvector p;
			rvector n;
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

		RGetDevice()->SetTexture(0, pTex[i]);
		//for(int i = 1; i < 8; i++)
			//RGetDevice()->SetTexture(i, NULL);
		RGetDevice()->SetMaterial(&Mat);
		pMesh->DrawSubset(0);
	}

	pStateBlock->Apply();
	SAFE_RELEASE(pStateBlock);

	/*rmatrix mView;
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
	
	rvector pos = ZGetCamera()->m_Position;
	rvector at = pos + ZGetGame()->m_pMyCharacter->GetDirection();
	rvector up = rvector(0.f, 0.f, 1.f);
	rmatrixLookAtLH(&mView, &pos, &at, &up);
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
	rplane myplane = PlaneFromPointNormal(mypos, dir);

	rvector checkpos, checkposto = pos + 100000.f*dir;
	IntersectLineSegmentPlane(&checkpos, myplane, pos, checkposto);
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

#ifndef _DEBUG
	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_TRAINING)
		return;

	MMatchItemDesc *pDesc = ZGetGame()->m_pMyCharacter->GetSelectItemDesc();

	if (!pDesc)
		return;

	if (pDesc->m_nID != 8500) // Change this to something saner later
		return;
#endif
	
	rvector Pos, Dir;
	GetIntersectedScreenLine(Pos, Dir);

	ZPICKINFO zpi;
	if (!ZGetGame()->Pick(ZGetGame()->m_pMyCharacter, Pos, Dir, &zpi, RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSBULLET) || !zpi.bBspPicked)
		return;

	rvector normal = rvector(zpi.bpi.pInfo->plane.a, zpi.bpi.pInfo->plane.b, zpi.bpi.pInfo->plane.c);
	rvector pos = zpi.bpi.PickPos + normal * 5;
	rvector up = fabs(normal.z) != 1 ? rvector(0, 0, 1) : rvector(1, 0, 0);

	ZPostPortal(n, pos, normal, up);
}

rvector g_v[4];

bool Portal::RedirectPos(rvector &from, rvector &to)
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

			rvector dim = rvector(.1f, 100.f, 150.f);
			rvector v1 = portalinfo.vPos - dim;
			rvector v2 = portalinfo.vPos + dim;
			rvector hit;

			g_v[0] = from;
			g_v[1] = to;

			/*g_v[2] = v1;
			g_v[3] = v2;*/

			rvector dir = to - from;
			Normalize(dir);

			if (!LinePortalIntersection(from, to, portalinfo, hit))
				continue;

			/*rmatrix mat;

			rvector right, up;

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

bool Portal::Move(ZObject *pObj, rvector &diff)
{
	if (!bPortalSetExists)
		return false;

	rvector &origin = pObj->m_Position;
	rvector target = origin + diff;

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

		if (!(ZGetGame()->Pick(ZGetGame()->m_pMyCharacter, target, dir, &zpi,
			RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSBULLET, 0)
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
		float distoutside = (idealpos[axis] - hit[axis]) / (idealpos[axis] - target[axis]) * Magnitude(diff);

		rvector disp = hit;
		Transform(disp, *ppi);

		newdir = -dir;
		Rotate(newdir, *ppi);

		newpos = disp - newdir * distoutside;

		bMakeNearProjectionMatrix = distoutside < 15;
	}

	/*pZCam->m_Position = newpos;
	pZCam->SetDirection(-newdir);*/

	rmatrix mView;

	mView = ViewMatrix(newpos, newdir, rvector(0, 0, 1));
	RSetTransform(D3DTS_VIEW, mView);

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
		normal = Normalized(float(sign) * vCameraDir);
		plane->a = normal.x; plane->b = normal.y; plane->c = normal.z;
		plane->d = -plane->a*t.x - plane->b*t.y - plane->c*t.z;
	};

	float RFov_horiz = FIXED_FOV(g_fFOV);
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

bool Portal::CheckIntersection(const rvector &target, float fRadius, float fHeight, PortalInfo **pppi)
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

bool Portal::LinePortalIntersection(const rvector &L1, const rvector &L2, const PortalInfo &portalinfo, rvector &Hit)
{
	return LineOBBIntersection(portalinfo.matWorld, portalinfo.matInvWorld, vDim, L1, L2, Hit);
}

static bool LineOBBIntersection(const rmatrix &matWorld, const rmatrix &matInvWorld, const rvector &extents, const rvector &L1, const rvector &L2, rvector &Hit)
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

static bool LineOBBIntersection(const rvector &center, const rvector &dir, const rvector &up, const rvector &extents, const rvector &L1, const rvector &L2, rvector &Hit)
{
	rvector mins, maxs, l1, l2;

	mins = -extents;
	maxs = extents;

	rmatrix mat, imat;

	MakeWorldMatrix(&mat, center, dir, up);

	imat = Inverse(mat);

	l1 = L1 * imat;
	l2 = L2 * imat;

	bool bRet = LineAABBIntersection(mins, maxs, l1, l2, Hit);

	if (!bRet)
		return false;

	Hit = Hit * mat;

	return true;
}

static bool LineAABBIntersection(const rvector &B1, const rvector &B2, const rvector &L1, const rvector &L2, rvector &Hit)
{
	auto GetIntersection = [](float fDst1, float fDst2, const rvector &P1, const rvector &P2, rvector &Hit) -> bool {
		if ((fDst1 * fDst2) >= 0.0f) return 0;
		if (fDst1 == fDst2) return 0;
		Hit = P1 + (P2 - P1) * (-fDst1 / (fDst2 - fDst1));
		return 1;
	};

	auto InBox = [](const rvector &Hit, const rvector &B1, const rvector &B2, const int Axis) -> bool {
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

bool Portal::AABBPortalIntersection(const rvector &B1, const rvector &B2, const PortalInfo &ppi)
{
	return AABBAABBIntersection(ppi.bb.vmin, ppi.bb.vmax, B1, B2);
}

static bool AABBAABBIntersection(const rvector mins1, const rvector maxs1, const rvector mins2, const rvector maxs2)
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

static void MakeOrientationMatrix(rmatrix &mat, const rvector &dir, const rvector &up)
{
	auto right = Normalized(CrossProduct(up, dir));
	auto upn = Normalized(CrossProduct(right, dir));

	GetIdentityMatrix(mat);
	mat._11 = right.x; mat._12 = right.y; mat._13 = right.z;
	mat._21 = upn.x; mat._22 = upn.y; mat._23 = upn.z;
	mat._31 = -dir.x; mat._32 = -dir.y; mat._33 = -dir.z;
}

static void MakeObliquelyClippingProjectionMatrix(rmatrix &matProjection, const rmatrix &matView, const rvector &p, const rvector &normal)
{
	//MakeProjectionMatrix(matProjection);

	rplane plane = PlaneFromPointNormal(p, normal);

	rmatrix WorldToView = Transpose(Inverse(matView));

	v4 projClipPlane;
	v4 vectorplane{ plane.a, plane.b, plane.c, plane.d };

	// Transform clip plane into view space
	projClipPlane = Transform(vectorplane, WorldToView);

	//cprint("%f\n", projClipPlane.w);

	/*if (fabs(projClipPlane.w) < 15)
	{
		MakeProjectionMatrix(matProjection, 10.f, 10000.f);
		//cprint("returning\n");
		return;
	}*/
	
	matProjection = GetDefaultProjectionMatrix();

	if (projClipPlane.w > 0)
	{
		v4 flippedPlane(-plane.a, -plane.b, -plane.c, -plane.d);

		projClipPlane = Transform(flippedPlane, WorldToView);
	}

	v4 q;

	q.x = (sgn(projClipPlane.x) + matProjection(2, 0)) / matProjection(0, 0);
	q.y = (sgn(projClipPlane.y) + matProjection(2, 1)) / matProjection(1, 1);
	q.z = -1.0F;
	q.w = (1.0F + matProjection(2, 2)) / matProjection(3, 2);

	D3DXVECTOR4 c = projClipPlane * (1.0F / DotProduct(projClipPlane, q));

	matProjection(0, 2) = c.x;
	matProjection(1, 2) = c.y;
	matProjection(2, 2) = c.z;
	matProjection(3, 2) = c.w;
}

static void MakeObliquelyClippingProjectionMatrixNVIDIA(rmatrix &matProjection, const rmatrix &matView, const rvector &p, const rvector &normal)
{
	rplane plane = PlaneFromPointNormal(p, normal);

	matProjection = GetDefaultProjectionMatrix();

	rmatrix WorldToProjection = Transpose(Inverse(matView * matProjection));

	v4 clipPlane(plane.a, plane.b, plane.c, plane.d);
	v4 projClipPlane;

	// Transform clip plane into projection space
	projClipPlane = Transform(clipPlane, WorldToProjection);

	if (projClipPlane.w > 0)
	{
		// Flip!
		v4 clipPlane(-plane.a, -plane.b, -plane.c, -plane.d);

		projClipPlane = Transform(clipPlane, WorldToProjection);
	}

	rmatrix mat;
	GetIdentityMatrix(mat);
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

static void MakePlane(rplane &plane, const rvector &v, const rvector &u, const rvector &origin)
{
	rvector a = v - origin, b = u - origin, normal;
	CrossProduct(&normal, a, b);
	Normalize(normal);
	plane.a = normal.x;
	plane.b = normal.y;
	plane.c = normal.z;
	plane.d = -DotProduct(origin, normal);
}

static void MakePortalFrustum(rplane (&Frustum)[6], const PortalInfo& pi, const v3& CamPos)
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
	rmatrix mOrigWorld, mOrigView, mOrigProjection;
	rplane OrigViewFrustum[6];

	mOrigWorld = RGetTransform(D3DTS_WORLD);
	mOrigView = RGetTransform(D3DTS_VIEW);
	mOrigProjection = RGetTransform(D3DTS_PROJECTION);

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

			RSetTransform(D3DTS_WORLD, portalinfo.matWorld);

			RGetDevice()->SetTexture(0, BlackTex.get());

			RGetDevice()->SetMaterial(&Mat);

			EdgeMesh.Draw();

			RSetTransform(D3DTS_WORLD, portalinfo.matWorld);

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

			EdgeMesh.Draw();

			RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
			RGetDevice()->SetRenderState(D3DRS_STENCILREF, 1);
			RGetDevice()->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);
			RGetDevice()->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
			RGetDevice()->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
			RGetDevice()->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
			RGetDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);

			rmatrix mView;

			rvector pos = rc.Cam.Pos * portalinfo.matTransform,
				dir = rc.Cam.Dir * portalinfo.matRot;

			rvector up = portalinfo.Other->vUp;

			mView = ViewMatrix(pos, dir, up);

			RSetTransform(D3DTS_VIEW, mView);

			rmatrix mProjection;
			MakeObliquelyClippingProjectionMatrix(mProjection, mView, portalinfo.Other->vPos, portalinfo.Other->vNormal);
			RSetTransform(D3DTS_PROJECTION, mProjection);

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

		RSetTransform(D3DTS_VIEW, mOrigView);
		RSetTransform(D3DTS_PROJECTION, mOrigProjection);

		//RGetDevice()->Clear(0, nullptr, D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);

		RenderPortal(!PortalIndex);
	}

	RGetDevice()->SetRenderState(D3DRS_STENCILENABLE, FALSE);

	RSetTransform(D3DTS_WORLD, mOrigWorld);
	RSetTransform(D3DTS_VIEW, mOrigView);
	RSetTransform(D3DTS_PROJECTION, mOrigProjection);

	memcpy(RViewFrustum, OrigViewFrustum, sizeof(RViewFrustum));
}

void Portal::WriteDepth(const RecursionContext& rc)
{
	RGetDevice()->Clear(0, nullptr, D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0.0f);

	auto mOrigWorld = RGetTransform(D3DTS_WORLD);
	auto mOrigView = RGetTransform(D3DTS_VIEW);
	auto mOrigProjection = RGetTransform(D3DTS_PROJECTION);

	for (const auto& pair : rc.Portals)
	{
		for (int i = 0; i < 2; i++)
		{
			const auto& pc = pair[i];

			if (!pc.IsVisible)
				continue;

			const auto& pi = pc.pi;

			RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

			RSetTransform(D3DTS_WORLD, pi.matWorld);

			RGetDevice()->SetTexture(0, nullptr);

			RGetDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, 0);

			EdgeMesh.Draw();

			RGetDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		}

		for (int i = 0; i < 2; i++)
		{
			if (pair[i].IsVisible)
				RenderEdge(pair[i].pi);
		}
	}

	RSetTransform(D3DTS_WORLD, mOrigWorld);
	RSetTransform(D3DTS_VIEW, mOrigView);
	RSetTransform(D3DTS_PROJECTION, mOrigProjection);
}

void Portal::CreatePortal(ZCharacter *pZChar, int iPortal, const rvector &vPos,
	const rvector &vNormal,const rvector &vUp)
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

	rmatrix matTranslation;

	auto right = Normalized(CrossProduct(vUp, Dir));
	auto local_up = Normalized(CrossProduct(right, Dir));

	rvector dir = Dir;

	rmatrix mat;
	GetIdentityMatrix(mat);
	mat._11 = right.x; mat._12 = right.y; mat._13 = right.z;
	mat._21 = local_up.x; mat._22 = local_up.y; mat._23 = local_up.z;
	mat._31 = -dir.x; mat._32 = -dir.y; mat._33 = -dir.z;

	matTranslation = TranslationMatrix(Pos);

	portalinfo.matWorld = mat * matTranslation;

	portalinfo.matInvWorld = Inverse(portalinfo.matWorld);

	for (int i = 0; i < 2; i++)
	{
		auto& pi = portalpair[i];

		rmatrix mat1, mat2;

		MakeOrientationMatrix(mat1, pi.vNormal, pi.vUp);
		MakeOrientationMatrix(mat2, -pi.Other->vNormal, pi.Other->vUp);
		mat1 = Inverse(mat1);

		pi.matRot = mat1 * mat2;

		mat1 = TranslationMatrix(-pi.vPos);
		mat2 = TranslationMatrix(pi.Other->vPos);

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

static void MakeFrustum(rplane (&Frustum)[6], rmatrix View, rvector Pos, rvector Dir, float Near = 5)
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
		normal = Normalized(float(sign) * Dir);
		plane->a = normal.x; plane->b = normal.y; plane->c = normal.z;
		plane->d = -plane->a*t.x - plane->b*t.y - plane->c*t.z;
	};

	float RFov_horiz = FIXED_FOV(g_fFOV);
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

			OrigView = RGetTransform(D3DTS_VIEW);

			rvector pos = rc.Cam.Pos,
				dir = rc.Cam.Dir;

			rvector up = rc.ViewingPortal->vUp;

			View = ViewMatrix(pos, dir, up);
		}

		Update(rc);

		UpdateAndRender(&rc);

		if (rc.Depth > 1)
		{
			RSetTransform(D3DTS_VIEW, View);
		}

		RenderPortals(rc);

		WriteDepth(rc);

		if (rc.Depth > 1)
		{
			RSetTransform(D3DTS_VIEW, OrigView);

			memcpy(RViewFrustum, OrigFrustum, sizeof(RViewFrustum));
		}

		/*for (auto& pair : rc.Portals)
		{
			for (auto& portal : pair)
			{
				v3 CamPos = rc.Cam.Pos * portal.pi.matTransform;

				rmatrix world;
				GetIdentityMatrix(world);
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
		rmatrix mProjection = GetDefaultProjectionMatrix(0.01);
		RSetTransform(D3DTS_PROJECTION, mProjection);
		bForceProjection = true;
	}
	else
	{
		if (bLookingThroughPortal)
		{
			rmatrix mProjection = GetDefaultProjectionMatrix();
			RSetTransform(D3DTS_PROJECTION, mProjection);
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