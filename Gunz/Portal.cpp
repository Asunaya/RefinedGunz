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

Portal *g_pPortal = 0;

const D3DXVECTOR3 Portal::vDim = rvector(100, 150, 0);
bool Portal::bPortalSetExists = false;

Portal::Portal()
{
	static const int coords[] = { 200, 20,
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

	static const int NumVertices = sizeof(coords) / sizeof(coords[0]) / 2;

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

	/*
	//left bottom point
	((D3DVERTEX*)pSrc)[0].p.x = -1;
	((D3DVERTEX*)pSrc)[0].p.y = -1;
	((D3DVERTEX*)pSrc)[0].p.z = 0;

	//left top point
	((D3DVERTEX*)pSrc)[1].p.x = -1;
	((D3DVERTEX*)pSrc)[1].p.y = 1;
	((D3DVERTEX*)pSrc)[1].p.z = 0;

	//right top point
	((D3DVERTEX*)pSrc)[2].p.x = 1;
	((D3DVERTEX*)pSrc)[2].p.y = 1;
	((D3DVERTEX*)pSrc)[2].p.z = 0;

	//right bottom point
	((D3DVERTEX*)pSrc)[3].p.x = 1;
	((D3DVERTEX*)pSrc)[3].p.y = -1;
	((D3DVERTEX*)pSrc)[3].p.z = 0;
	*/

	//left bottom point
	((D3DVERTEX*)pSrc)[0].p.x = -100;
	((D3DVERTEX*)pSrc)[0].p.y = -150;
	((D3DVERTEX*)pSrc)[0].p.z = 0;

	//left top point
	((D3DVERTEX*)pSrc)[1].p.x = -100;
	((D3DVERTEX*)pSrc)[1].p.y = 150;
	((D3DVERTEX*)pSrc)[1].p.z = 0;

	//right top point
	((D3DVERTEX*)pSrc)[2].p.x = 100;
	((D3DVERTEX*)pSrc)[2].p.y = 150;
	((D3DVERTEX*)pSrc)[2].p.z = 0;

	//right bottom point
	((D3DVERTEX*)pSrc)[3].p.x = 100;
	((D3DVERTEX*)pSrc)[3].p.y = -150;
	((D3DVERTEX*)pSrc)[3].p.z = 0;


	((D3DVERTEX*)pSrc)[0].tu = 0;
	((D3DVERTEX*)pSrc)[0].tv = 0;

	((D3DVERTEX*)pSrc)[1].tu = 0;
	((D3DVERTEX*)pSrc)[1].tv = 1;

	((D3DVERTEX*)pSrc)[2].tu = 1;
	((D3DVERTEX*)pSrc)[2].tv = 1;

	((D3DVERTEX*)pSrc)[3].tu = 1;
	((D3DVERTEX*)pSrc)[3].tv = 0;

	pRectangleMesh->UnlockVertexBuffer();

	WORD* pIndexBuffer = 0;
	pRectangleMesh->LockIndexBuffer(0, (void**)&pIndexBuffer);

	// fill in the front face index data
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

	GenerateTexture(RGetDevice(), &pBlackTex, 0xFF000000);

	if (FAILED(D3DXCreateTextureFromFile(RGetDevice(), "Interface/portal1.png", &pPortalEdgeTex[0])))
	{
		MLog("Failed to load portal 1 tex\n");
	}
	if (FAILED(D3DXCreateTextureFromFile(RGetDevice(), "Interface/portal2.png", &pPortalEdgeTex[1])))
	{
		MLog("Failed to load portal 2 tex\n");
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

void Portal::OnInvalidate()
{
}

void Portal::OnRestore()
{
}

void Portal::Transform(D3DXVECTOR3 &v, const PortalInfo &portalinfo, int iPortal)
{
	Transform(v, v, portalinfo, iPortal);
}

void Portal::Transform(D3DXVECTOR3 &vOut, const D3DXVECTOR3 &vIn, const PortalInfo &portalinfo, int iPortal)
{
	rvector Displacement = vIn - portalinfo.vPos[iPortal];

	Rotate(Displacement, portalinfo, iPortal);

	vOut = portalinfo.vPos[!iPortal] + Displacement;
}

void Portal::Rotate(D3DXVECTOR3 &v, const PortalInfo &portalinfo, int iPortal)
{
	v = v * portalinfo.matRot[iPortal];
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

void Portal::RenderWorldStencil()
{
	if (!bPortalSetExists)
		return;

	D3DXMATRIX mOrigWorld, mOrigView, mOrigProjection;
	rplane OrigViewFrustum[6];

	RGetDevice()->GetTransform(D3DTS_WORLD, &mOrigWorld);
	RGetDevice()->GetTransform(D3DTS_VIEW, &mOrigView);
	RGetDevice()->GetTransform(D3DTS_PROJECTION, &mOrigProjection);

	memcpy(OrigViewFrustum, RViewFrustum, sizeof(OrigViewFrustum));

	RGetDevice()->SetRenderState(D3DRS_STENCILENABLE, TRUE);

	for (const auto &pair : PortalList)
	{
		const PortalInfo &portalinfo = pair.second;

		for (int i = 0; i < 2; i++)
		{
			if (!portalinfo.bIsVisible[i])
				continue;

			RGetDevice()->Clear(0, NULL, D3DCLEAR_STENCIL, 0x00000000, 1.0f, 0);

			if (i == 1)
			{
				RGetDevice()->SetTransform(D3DTS_VIEW, &mOrigView);
				RGetDevice()->SetTransform(D3DTS_PROJECTION, &mOrigProjection);
			}

			RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			RGetDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
			RGetDevice()->SetRenderState(D3DRS_STENCILENABLE, FALSE);

			RGetDevice()->SetTransform(D3DTS_WORLD, &portalinfo.matWorld[i]);

			RGetDevice()->SetFVF(pEdgeMesh->GetFVF());
			RGetDevice()->SetVertexShader(NULL);
			RGetDevice()->SetTexture(0, pBlackTex);

			RGetDevice()->SetMaterial(&Mat);

			//RGetDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, 0);

			pEdgeMesh->DrawSubset(0);

			//RGetDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);

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


			D3DXVECTOR3 pos = vCameraPos * portalinfo.matTransform[i],
				dir = vCameraDir * portalinfo.matRot[i];

			rmatrix mView;
			D3DXVECTOR3 at = pos + dir, up = portalinfo.vUp[!i];

			D3DXMatrixLookAtLH(&mView, &pos, &at, &up);

			RGetDevice()->SetTransform(D3DTS_VIEW, &mView);

			rmatrix mProjection;
			MakeObliquelyClippingProjectionMatrix(mProjection, mView, portalinfo.vPos[!i], portalinfo.vNormal[!i]);
			RGetDevice()->SetTransform(D3DTS_PROJECTION, &mProjection);

			memcpy(RViewFrustum, portalinfo.Frustum[i], sizeof(RViewFrustum));

			bForceProjection = true;
			bDontDraw = true;
			ZGetGame()->Draw();
			bDontDraw = false;
			bForceProjection = false;
		}
	}

	RGetDevice()->SetTransform(D3DTS_VIEW, &mOrigView);
	RGetDevice()->SetTransform(D3DTS_PROJECTION, &mOrigProjection);

	RGetDevice()->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0.0f);

	for (const auto &pair : PortalList)
	{
		const PortalInfo &portalinfo = pair.second;

		for (int i = 0; i < 2; i++)
		{
			if (!portalinfo.bIsVisible[i])
				continue;

			// Write to Z buffer
			RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_STENCILENABLE, FALSE);

			RGetDevice()->SetTransform(D3DTS_WORLD, &portalinfo.matWorld[i]);

			RGetDevice()->SetFVF(pRectangleMesh->GetFVF());
			RGetDevice()->SetVertexShader(NULL);
			RGetDevice()->SetTexture(0, NULL);

			RGetDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
			/*RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);   //Ignored


			//Set the alpha to come completely from the diffuse

			RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);   //Ignored*/

			//RGetDevice()->SetTexture(0, pPortalEdgeTex[i]);

			pEdgeMesh->DrawSubset(0);

			//RGetDevice()->SetTexture(0, NULL);

			RGetDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		}
	}

	RGetDevice()->SetRenderState(D3DRS_STENCILENABLE, FALSE);

	RGetDevice()->SetTransform(D3DTS_WORLD, &mOrigWorld);
	RGetDevice()->SetTransform(D3DTS_VIEW, &mOrigView);

	if (!bMakeNearProjectionMatrix)
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
			RGetDevice()->SetTransform(D3DTS_PROJECTION, &mOrigProjection);
			bForceProjection = false;
		}
	}
	else
	{
		rmatrix mProjection;
		MakeProjectionMatrix(mProjection, 0.01);
		RGetDevice()->SetTransform(D3DTS_PROJECTION, &mProjection);
		bForceProjection = true;
	}

	memcpy(RViewFrustum, OrigViewFrustum, sizeof(RViewFrustum));
}

void Portal::RenderEdge()
{
	for (const auto &pair : PortalList)
	{
		const PortalInfo &portalinfo = pair.second;

		for (int i = 0; i < 2; i++)
		{
			if (!portalinfo.bIsVisible[i])
				continue;

			// Write to Z buffer
			RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_STENCILENABLE, FALSE);

			rmatrix matWorld;
			MakeWorldMatrix(&matWorld, portalinfo.vPos[i] + portalinfo.vNormal[i] * 0.5, portalinfo.vNormal[i], portalinfo.vUp[i]);

			RGetDevice()->SetTransform(D3DTS_WORLD, &matWorld);

			RGetDevice()->SetFVF(pRectangleMesh->GetFVF());
			RGetDevice()->SetVertexShader(NULL);
			RGetDevice()->SetTexture(0, NULL);

			RGetDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
			RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);   //Ignored


			//Set the alpha to come completely from the diffuse

			RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);   //Ignored

			RGetDevice()->SetTexture(0, pPortalEdgeTex[i]);

			pRectangleMesh->DrawSubset(0);

			RGetDevice()->SetTexture(0, NULL);

			/*rmatrix world;
			D3DXMatrixIdentity(&world);
			RGetDevice()->SetTransform(D3DTS_WORLD, &world);

			RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
			RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
			RGetDevice()->SetRenderState(D3DRS_ZENABLE, FALSE);
			RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
			RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
			RDrawLine(const_cast<D3DXVECTOR3 &>(portalinfo.topright[i]), const_cast<D3DXVECTOR3 &>(portalinfo.topleft[i]), 0xFFFF0000);
			RDrawLine(const_cast<D3DXVECTOR3 &>(portalinfo.topright[i]), const_cast<D3DXVECTOR3 &>(portalinfo.bottomright[i]), 0xFFFF0000);
			RDrawLine(const_cast<D3DXVECTOR3 &>(portalinfo.topleft[i]), const_cast<D3DXVECTOR3 &>(portalinfo.bottomleft[i]), 0xFFFF0000);
			RDrawLine(const_cast<D3DXVECTOR3 &>(portalinfo.bottomright[i]), const_cast<D3DXVECTOR3 &>(portalinfo.bottomleft[i]), 0xFFFF0000);*/
		}
	}
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

void GetIntersectedScreenLine(rvector &pos, rvector &dir)
{
	MPOINT Crosshair = ZGetCombatInterface()->GetCrosshairPoint();

	RGetScreenLine(Crosshair.x, Crosshair.y, &pos, &dir);

	rvector mypos = ZGetGame()->m_pMyCharacter->m_Position + rvector(0, 0, 100);
	rplane myplane;
	D3DXPlaneFromPointNormal(&myplane, &mypos, &dir);

	rvector checkpos, checkposto = pos + 100000.f*dir;
	D3DXPlaneIntersectLine(&checkpos, &myplane, &pos, &checkposto);
}

void Portal::OnShot()
{
	int n = -1;
#ifndef DEBUG
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

#ifndef DEBUG
	MMatchItemDesc *pDesc = ZGetGame()->m_pMyCharacter->GetSelectItemDesc();

	if (!pDesc)
		return;

	char *pszItemName = pDesc->m_szName;

	if (!pszItemName)
		return;

	if (strcmp(pszItemName, "Portal Gun") != 0)
		return;
#endif
	
	D3DXVECTOR3 Pos, Dir;
	GetIntersectedScreenLine(Pos, Dir);

	ZPICKINFO zpi;
	if (!ZGetGame()->Pick(ZGetGame()->m_pMyCharacter, Pos, Dir, &zpi, RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSBULLET) || !zpi.bBspPicked)
		return;

	rvector normal = D3DXVECTOR3(zpi.bpi.pInfo->plane.a, zpi.bpi.pInfo->plane.b, zpi.bpi.pInfo->plane.c);
	rvector pos = zpi.bpi.PickPos + normal;
	rvector up = fabs(normal.z) != 1 ? rvector(0, 0, 1) : rvector(1, 0, 0);

	CreatePortal(ZGetGame()->m_pMyCharacter, n, pos, normal, up);
	ZPostPortal(n, pos, normal, up);
}

D3DXVECTOR3 g_v[4];

bool Portal::RedirectPos(D3DXVECTOR3 &from, D3DXVECTOR3 &to)
{
	if (!bPortalSetExists)
		return false;

	for (auto pair : PortalList)
	{
		PortalInfo &portalinfo = pair.second;

		for (int i = 0; i < 2; i++)
		{
			D3DXVECTOR3 dim = D3DXVECTOR3(.1f, 100.f, 150.f);
			D3DXVECTOR3 v1 = portalinfo.vPos[i] - dim;
			D3DXVECTOR3 v2 = portalinfo.vPos[i] + dim;
			D3DXVECTOR3 hit;

			g_v[0] = from;
			g_v[1] = to;

			/*g_v[2] = v1;
			g_v[3] = v2;*/

			rvector dir = to - from;
			Normalize(dir);

			if (!LinePortalIntersection(from, to, portalinfo, i, hit))
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

			rvector Displacement = hit - portalinfo.vPos[i];

			Rotate(Displacement, portalinfo, i);

			Rotate(dir, portalinfo, i);

			from = portalinfo.vPos[!i] + Displacement;

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

	rvector &charpos = pZChar->GetPosition();

	PortalInfo *ppi;
	int i;

	if (!ZObjectPortalIntersection(pZChar, &ppi, &i))
		return;

	/*rmatrix mOrigView, mView;

	RGetDevice()->GetTransform(D3DTS_VIEW, &mOrigView);

	rvector Displacement = ZGetCamera()->GetPosition() - ppi->vPos[i];
	rvector dir = ZGetCamera()->GetCurrentDir();

	Displacement.z += (ppi->vPos[i].z - ppi->vPos[!i].z) * 2;

	rvector disp = Displacement;

	Rotate(Displacement, *ppi, i);
	Rotate(dir, *ppi, i);*/
	
	/*rmatrix matInvRot;

	D3DXMatrixInverse(&matInvRot, NULL, &matRot[i]);

	Displacement = Displacement * matInvRot;
	dir = dir * matInvRot;*/

	//cprint("%f, %f, %f; %f, %f, %f\n", Displacement.x, Displacement.y, Displacement.z, disp.x, disp.y, disp.z);

	/*D3DXVECTOR3 pos = ppi->vPos[!i] + Displacement;

	D3DXVECTOR3 at = pos + dir, up = ppi->vUp[!i];

	D3DXMatrixLookAtLH(&mView, &pos, &at, &up);

	RGetDevice()->SetTransform(D3DTS_VIEW, &mView);*/

	
	rvector &pos = pZChar->m_Position; // Change bounding box
	//rvector &dir = pZChar->GetVisualMesh()->m_vDir;
	rmatrix &mat = pZChar->GetVisualMesh()->m_WorldMat;
	//rmatrix &uppermat = pZChar->GetVisualMesh()->m_UpperRotMat;

	rvector oldpos = pos;
	//rvector olddir = dir;

	/*rvector Displacement = ppi->vPos[i] - pZChar->m_Position;
	rvector newdir = ZGetCamera()->GetCurrentDir();

	Rotate(Displacement, *ppi, i);
	Rotate(newdir, *ppi, i);

	D3DXVECTOR3 newpos = ppi->vPos[!i] + Displacement;

	newpos = pZChar->GetPosition() * ppi->matTransform[i];*/

	rvector newpos = pos * ppi->matTransform[i];

	rmatrix oldmat = mat;

	//MakeWorldMatrix(&mat, newpos, newdir, ppi->vUp[i]);

	mat = mat * ppi->matTransform[i];

	pos = newpos;
	//dir = newdir;


	bDontDrawChar = true;
	pZChar->Draw();
	bDontDrawChar = false;


	pos = oldpos;
	//dir = olddir;

	mat = oldmat;

	//MLog("Intersect %f, %f, %f; %f, %f, %f; %f, %f, %f\n", ppi->vPos[!i].x, ppi->vPos[!i].y, ppi->vPos[!i].z, newpos.x, newpos.y, newpos.z, pos.x, pos.y, pos.z);


	//RGetDevice()->SetTransform(D3DTS_VIEW, &mOrigView);
}

bool Portal::Move(ZObject *pObj, D3DXVECTOR3 &diff)
{
	if (!bPortalSetExists)
		return false;

	rvector &origin = pObj->m_Position;
	rvector target = origin + diff;
	rvector hit;

	int iPortal;
	PortalInfo *ppi;

	if (!CheckIntersection(target, pObj->GetCollRadius(), pObj->GetCollHeight(), &ppi, &iPortal))
		return false;

	if (DotProduct(target, ppi->vNormal[iPortal]) + ppi->d[iPortal] < 0)
	{
		origin = target * ppi->matTransform[iPortal];
		pObj->m_Direction *= ppi->matRot[iPortal];
		pObj->SetVelocity(pObj->GetVelocity() * ppi->matRot[iPortal]);

		ZGetCamera()->SetDirection(ZGetCamera()->GetCurrentDir() * ppi->matRot[iPortal]);

		RCameraPosition *= ppi->matTransform[iPortal];
		RCameraDirection *= ppi->matRot[iPortal];
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

	/*for (const auto &pair : PortalList)
	{
		const PortalInfo &portalinfo = pair.second;

		for (int i = 0; i < 2; i++)
		{
			rvector vec = portalinfo.vPos[i] - target;
			if (LinePortalIntersection(origin, target, portalinfo, i, hit) && DotProduct(target, portalinfo.vNormal[i]) + portalinfo.d > 0)
			{
				rvector Displacement = hit - portalinfo.vPos[i];
				origin = portalinfo.vPos[!i] + Displacement;

				bTeleported = true;

				rvector dir = ZGetCamera()->GetCurrentDir();

				Rotate(dir, portalinfo, i);

				ZGetCamera()->SetDirection(dir);

				return true;
			}
		}
	}*/
}

void Portal::RedirectCamera()
{
	if(!bPortalSetExists)
		return;

	bMakeNearProjectionMatrix = false;
	bLookingThroughPortal = false;

	vCameraPos = RCameraPosition;//ZGetCamera()->GetPosition();
	vCameraDir = RCameraDirection;//ZGetCamera()->GetCurrentDir();

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

	int iPortal;
	PortalInfo *ppi;
	if (ZObjectPortalIntersection(ZGetGame()->m_pMyCharacter, &ppi, &iPortal))
	{
		if (DotProduct(vCameraDir, ppi->vNormal[iPortal]) >= 0) // arccos(0) = 2/pi
		{
			target += vCameraDir * 30;
		}
	}

	ZPICKINFO zpi;

	rvector dir = -ZGetCamera()->GetCurrentDir();

	rvector idealpos = target + dir * ZGetCamera()->m_fDist;

	if (!( ZGetGame()->Pick(ZGetGame()->m_pMyCharacter, target, dir, &zpi, RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSBULLET, 0)
		&& zpi.bBspPicked
		&& Magnitude(target - zpi.bpi.PickPos) < ZGetCamera()->m_fDist ))
	//{
		//cprint("No hit\n");
		return;
	//}

	rvector &pick = zpi.bpi.PickPos;

	int portal = -1;
	rvector hit;
	for (auto pair : PortalList)
	{
		for (int i = 0; i < 2; i++)
		{
			if (LinePortalIntersection(target, pick, pair.second, i, hit))
			{
				ppi = &pair.second;
				portal = i;
				break;
			}
		}
	}

	if (portal == -1)
	//{
		//cprint("No portal");
		return;
	//}

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
	Transform(disp, *ppi, portal);

	rvector newdir = dir;
	Rotate(newdir, *ppi, portal);

	rvector newpos = disp + newdir * distoutside;

	/*pZCam->m_Position = newpos;
	pZCam->SetDirection(-newdir);*/

	D3DXMATRIX mView;

	MakeViewMatrix(mView, newpos, -newdir, D3DXVECTOR3(0, 0, 1));
	RGetDevice()->SetTransform(D3DTS_VIEW, &mView);

	vCameraPos = newpos;
	vCameraDir = -newdir;

	bMakeNearProjectionMatrix = distoutside < 15;

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

bool Portal::CheckIntersection(const D3DXVECTOR3 &target, float fRadius, float fHeight, PortalInfo **pppi, int *piPortal)
{
	rvector mins = target - rvector(fRadius, fRadius, 0),
		maxs = target + rvector(fRadius, fRadius, fHeight);

	for (auto &pair : PortalList)
	{
		PortalInfo &portalinfo = pair.second;

		for (int i = 0; i < 2; i++)
		{
			if (AABBPortalIntersection(mins, maxs, portalinfo, i))
			{
				if(pppi)
					*pppi = &portalinfo;
				if(piPortal)
					*piPortal = i;

				return true;
			}
		}
	}

	return false;
}

bool Portal::ZObjectPortalIntersection(const ZObject *pObj, PortalInfo **retppi, int *piPortal)
{
	return CheckIntersection(pObj->m_Position, pObj->GetCollRadius(), pObj->GetCollHeight(), retppi, piPortal);
}

bool Portal::LinePortalIntersection(const D3DXVECTOR3 &L1, const D3DXVECTOR3 &L2, const PortalInfo &portalinfo, int iPortal, D3DXVECTOR3 &Hit)
{
	return LineOBBIntersection(portalinfo.matWorld[iPortal], portalinfo.matInvWorld[iPortal], vDim, L1, L2, Hit);
}

bool Portal::LineOBBIntersection(const D3DXMATRIX &matWorld, const D3DXMATRIX &matInvWorld, const D3DXVECTOR3 &extents, const D3DXVECTOR3 &L1, const D3DXVECTOR3 &L2, D3DXVECTOR3 &Hit)
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

bool Portal::LineOBBIntersection(const D3DXVECTOR3 &center, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &up, const D3DXVECTOR3 &extents, const D3DXVECTOR3 &L1, const D3DXVECTOR3 &L2, D3DXVECTOR3 &Hit)
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

bool Portal::LineAABBIntersection(const D3DXVECTOR3 &B1, const D3DXVECTOR3 &B2, const D3DXVECTOR3 &L1, const D3DXVECTOR3 &L2, D3DXVECTOR3 &Hit)
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

bool Portal::AABBPortalIntersection(const D3DXVECTOR3 &B1, const D3DXVECTOR3 &B2, const PortalInfo &ppi, int iPortal)
{
	return AABBAABBIntersection(ppi.bb[iPortal].vmin, ppi.bb[iPortal].vmax, B1, B2);
}

bool Portal::AABBAABBIntersection(const D3DXVECTOR3 mins1, const D3DXVECTOR3 maxs1, const D3DXVECTOR3 mins2, const D3DXVECTOR3 maxs2)
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

void Portal::MakeOrientationMatrix(D3DXMATRIX &mat, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &up)
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

void Portal::MakeViewMatrix(D3DXMATRIX &mat, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &up)
{
	rvector at = pos + dir;

	D3DXMatrixLookAtLH(&mat, &pos, &at, &up);
}

void Portal::MakeProjectionMatrix(D3DXMATRIX &mat, float NearZ, float FarZ)
{
	float fAspect = float(RGetScreenWidth()) / RGetScreenHeight();
	float fovy = atanf(tanf(g_fFOV / 2.0f) / fAspect) * 2.0f;

	D3DXMatrixPerspectiveFovLH(&mat, fovy, fAspect, NearZ, FarZ);
}

inline float sgn(float a)
{
	if (a > 0.0F) return (1.0F);
	if (a < 0.0F) return (-1.0F);
	return (0.0F);
}

void Portal::MakeObliquelyClippingProjectionMatrix(D3DXMATRIX &matProjection, const D3DXMATRIX &matView, const D3DXVECTOR3 &p, const D3DXVECTOR3 &normal)
{
	//MakeProjectionMatrix(matProjection);

	rplane plane;

	D3DXPlaneFromPointNormal(&plane, &p, &normal);

	D3DXMATRIX WorldToProjection = matView;

	D3DXMatrixInverse(&WorldToProjection, NULL, &WorldToProjection);
	D3DXMatrixTranspose(&WorldToProjection, &WorldToProjection);

	D3DXVECTOR4 clipPlane(plane.a, plane.b, plane.c, plane.d);
	D3DXVECTOR4 projClipPlane;

	// Transform clip plane into projection space
	D3DXVec4Transform(&projClipPlane, &clipPlane, &WorldToProjection);

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
		// Flip!
		D3DXVECTOR4 clipPlane(-plane.a, -plane.b, -plane.c, -plane.d);

		D3DXVec4Transform(&projClipPlane, &clipPlane, &WorldToProjection);
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

void Portal::AddPlayer(ZCharacter *pZChar)
{
	PortalList.insert(std::make_pair(pZChar, PortalInfo()));
}

void Portal::DeletePlayer(ZCharacter *pZChar)
{
	PortalListIt it = PortalList.find(pZChar);
	if(it != PortalList.end())
		PortalList.erase(it);
}

void Portal::Update()
{
	for (auto &pair : PortalList)
	{
		PortalInfo &portalinfo = pair.second;

		if (!portalinfo.IsValid())
			continue;

		for (int iPortal = 0; iPortal < 2; iPortal++)
		{
			if (!isInViewFrustum(&portalinfo.bb[iPortal], RViewFrustum))
			{
				portalinfo.bIsVisible[iPortal] = false;
				continue;
			}

			portalinfo.bIsVisible[iPortal] = true;

			rvector TransformedCameraPos = vCameraPos * portalinfo.matTransform[iPortal];

			MakePlane(portalinfo.Frustum[iPortal][0], portalinfo.topleft[!iPortal], portalinfo.topright[!iPortal], TransformedCameraPos);
			MakePlane(portalinfo.Frustum[iPortal][1], portalinfo.bottomright[!iPortal], portalinfo.bottomleft[!iPortal], TransformedCameraPos);
			MakePlane(portalinfo.Frustum[iPortal][2], portalinfo.topright[!iPortal], portalinfo.bottomright[!iPortal], TransformedCameraPos);
			MakePlane(portalinfo.Frustum[iPortal][3], portalinfo.bottomleft[!iPortal], portalinfo.topleft[!iPortal], TransformedCameraPos);
		}
	}
}

void Portal::MakePlane(D3DXPLANE &plane, const D3DXVECTOR3 &v, const D3DXVECTOR3 &u, const D3DXVECTOR3 &origin)
{
	D3DXVECTOR3 a = v - origin, b = u - origin, normal;
	CrossProduct(&normal, a, b);
	Normalize(normal);
	plane.a = normal.x;
	plane.b = normal.y;
	plane.c = normal.z;
	plane.d = -DotProduct(origin, normal);
}

void Portal::CreatePortal(ZCharacter *pZChar, int iPortal, const D3DXVECTOR3 &vPos, const D3DXVECTOR3 &vNormal, const D3DXVECTOR3 &vUp)
{
	PortalInfo &portalinfo = PortalList[pZChar];

	int n = iPortal;

	portalinfo.vPos[n] = vPos;
	portalinfo.vNormal[n] = vNormal;
	portalinfo.vUp[n] = vUp;

	portalinfo.d[n] = -DotProduct(portalinfo.vNormal[n], portalinfo.vPos[n]);

	rvector Pos = portalinfo.vPos[n];
	rvector Dir = -portalinfo.vNormal[n];
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
	mat._31 = dir.x; mat._32 = dir.y; mat._33 = dir.z;

	D3DXMatrixTranslation(&matTranslation, Pos.x, Pos.y, Pos.z);

	portalinfo.matWorld[n] = mat * matTranslation;

	D3DXMatrixInverse(&portalinfo.matInvWorld[n], NULL, &portalinfo.matWorld[n]);

	for (int i = 0; i < 2; i++)
	{
		rmatrix mat1, mat2;

		MakeOrientationMatrix(mat1, -portalinfo.vNormal[i], portalinfo.vUp[i]);
		MakeOrientationMatrix(mat2, portalinfo.vNormal[!i], portalinfo.vUp[!i]);
		D3DXMatrixInverse(&mat1, NULL, &mat1);

		portalinfo.matRot[i] = mat1 * mat2;

		D3DXMatrixTranslation(&mat1, -portalinfo.vPos[i].x, -portalinfo.vPos[i].y, -portalinfo.vPos[i].z);
		D3DXMatrixTranslation(&mat2, portalinfo.vPos[!i].x, portalinfo.vPos[!i].y, portalinfo.vPos[!i].z);

		portalinfo.matTransform[i] = mat1 * portalinfo.matRot[i] * mat2;
	}

	portalinfo.bSet[n] = true;

	if (portalinfo.bSet[0] && portalinfo.bSet[1])
		bPortalSetExists = true;

	portalinfo.topright[n] = vDim * portalinfo.matWorld[n];
	portalinfo.topleft[n] = rvector(vDim.x, -vDim.y, vDim.z) * portalinfo.matWorld[n];
	portalinfo.bottomright[n] = rvector(-vDim.x, vDim.y, vDim.z) * portalinfo.matWorld[n];
	portalinfo.bottomleft[n] = rvector(-vDim.x, -vDim.y, vDim.z) * portalinfo.matWorld[n];

	portalinfo.bb[n].vmin = portalinfo.topright[n];
	portalinfo.bb[n].vmax = portalinfo.topright[n];

	for (int i = 0; i < 3; i++)
	{
		portalinfo.bb[n].vmin[i] = min(portalinfo.bb[n].vmin[i], portalinfo.topright[n][i]);
		portalinfo.bb[n].vmin[i] = min(portalinfo.bb[n].vmin[i], portalinfo.topleft[n][i]);
		portalinfo.bb[n].vmin[i] = min(portalinfo.bb[n].vmin[i], portalinfo.bottomright[n][i]);
		portalinfo.bb[n].vmin[i] = min(portalinfo.bb[n].vmin[i], portalinfo.bottomleft[n][i]);
		portalinfo.bb[n].vmax[i] = max(portalinfo.bb[n].vmax[i], portalinfo.topright[n][i]);
		portalinfo.bb[n].vmax[i] = max(portalinfo.bb[n].vmax[i], portalinfo.topleft[n][i]);
		portalinfo.bb[n].vmax[i] = max(portalinfo.bb[n].vmax[i], portalinfo.bottomright[n][i]);
		portalinfo.bb[n].vmax[i] = max(portalinfo.bb[n].vmax[i], portalinfo.bottomleft[n][i]);
	}

	// Near
	rvector normal = portalinfo.vNormal[!n];
	portalinfo.Frustum[n][4].a = normal.x;
	portalinfo.Frustum[n][4].b = normal.y;
	portalinfo.Frustum[n][4].c = normal.z;
	portalinfo.Frustum[n][4].d = -DotProduct(normal, portalinfo.vPos[!n]);

	// Far
	normal = -portalinfo.vNormal[!n];
	portalinfo.Frustum[n][5].a = normal.x;
	portalinfo.Frustum[n][5].b = normal.y;
	portalinfo.Frustum[n][5].c = normal.z;
	portalinfo.Frustum[n][5].d = -DotProduct(normal, portalinfo.vPos[!n] - normal * 100000);
}

void Portal::PreDraw()
{
	if (bDontDraw || !bPortalSetExists))
		return;

#ifndef PORTAL_USE_RT_TEXTURE
	if (!RIsStencilBuffer())
		return;
#endif

	RedirectCamera();

	Update();

	RenderWorldStencil();
}

void Portal::PostDraw()
{
	if (bDontDraw || !bPortalSetExists || !RIsStencilBuffer())
		return;

#ifndef PORTAL_USE_RT_TEXTURE
	if (!RIsStencilBuffer())
		return;
#endif

	RenderEdge();
}

void Portal::PostCameraUpdate()
{
	if (bDontDraw || !bPortalSetExists)
		return;

	RedirectCamera();
}

#endif