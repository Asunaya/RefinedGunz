#include "stdafx.h"

#include "ZApplication.h"
#include "ZEffectBillboard.h"
#include "MZFileSystem.h"
#include "RTypes.h"

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

//////////////////////////////////////////////////////////////////////////////////////

LPDIRECT3DVERTEXBUFFER9	g_pVB;

// ZEffectManager.cpp 생성자와 소멸자에 등록

bool CreateCommonRectVertexBuffer()
{
	static CUSTOMVERTEX Billboard[] = {
		{-1, -1, 0, 0xFFFFFFFF, 1, 0},
		{-1,  1, 0, 0xFFFFFFFF, 1, 1},
		{ 1,  1, 0, 0xFFFFFFFF, 0, 1},
		{ 1, -1, 0, 0xFFFFFFFF, 0, 0},
	};

	if(FAILED(RGetDevice()->CreateVertexBuffer(sizeof(Billboard), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_MANAGED, &g_pVB,NULL)))
		return false;

	BYTE* pVertices;

	if(FAILED(g_pVB->Lock(0, sizeof(Billboard), (VOID**)&pVertices, 0))) return false;
	memcpy(pVertices, Billboard, sizeof(Billboard));
	g_pVB->Unlock();

	return true;
}

void RealeaseCommonRectVertexBuffer()
{
	SAFE_RELEASE(g_pVB);
}

LPDIRECT3DVERTEXBUFFER9	GetCommonRectVertexBuffer()
{
	return g_pVB;
}

//////////////////////////////////////////////////////////////////////////////////////

ZEffectBillboardSource::ZEffectBillboardSource(const char* szTextureFileName)
{
	m_pTex = RCreateBaseTexture(szTextureFileName);

	/*
	m_pTex = new RBaseTexture;
	m_pTex->Create(szTextureFileName);
*/
/*
	static CUSTOMVERTEX Billboard[] = {
		{-1, -1, 0, 0xFFFFFFFF, 1, 0},
		{-1, 1, 0, 0xFFFFFFFF, 1, 1},
		{1, 1, 0, 0xFFFFFFFF, 0, 1},
		{1, -1, 0, 0xFFFFFFFF, 0, 0},
	};

	memcpy(m_pVertices, Billboard, sizeof(CUSTOMVERTEX)*4);
*/
/*
	if(FAILED(RGetDevice()->CreateVertexBuffer(sizeof(Billboard), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_MANAGED, &m_pVB,NULL))) return;
	BYTE* pVertices;
	if(FAILED(m_pVB->Lock(0, sizeof(Billboard), &pVertices, 0))) return;
	memcpy(pVertices, Billboard, sizeof(Billboard));
	m_pVB->Unlock();
*/
}

ZEffectBillboardSource::~ZEffectBillboardSource(void)
{
	if(m_pTex) {
		RDestroyBaseTexture(m_pTex);
		m_pTex = NULL;
	}
//	SAFE_RELEASE(m_pVB);
}

bool ZEffectBillboardSource::Draw(rvector &Pos, rvector &Dir, rvector &Up, rvector &Scale, float fOpacity)
{
	if (m_pTex == NULL) return false;

	RealSpace2::rboundingbox bbox;// 보통 scale 이 변한다~..

//	rvector _scale = Scale*2/3;
	rvector _scale = Scale/2;

	bbox.vmin = Pos-_scale;
	bbox.vmax = Pos+_scale;

	if(isInViewFrustum( &bbox, RGetViewFrustum())==false) {
//	if(isInViewFrustum( Pos, RGetViewFrustum())==false) {
  		return false;
	}

	/*
	// 이전 상태 저장
	DWORD dwStateBlock;
	RGetDevice()->CreateStateBlock(D3DSBT_PIXELSTATE, &dwStateBlock);
	RGetDevice()->CaptureStateBlock(dwStateBlock);
*/

	// Transform
	rmatrix matTranslation, matScaling, matWorld;

	rvector right;
	D3DXVec3Cross(&right, &Up, &Dir);
	D3DXVec3Normalize(&right, &right);

	rvector up;
	D3DXVec3Cross(&up, &right, &Dir);
	D3DXVec3Normalize(&up, &up);

	rvector dir = RCameraDirection;

	rmatrix mat;
	D3DXMatrixIdentity(&mat);
	mat._11=right.x;mat._12=right.y;mat._13=right.z;
	mat._21=up.x;mat._22=up.y;mat._23=up.z;
	mat._31=dir.x;mat._32=dir.y;mat._33=dir.z;

	D3DXMatrixTranslation(&matTranslation, Pos.x, Pos.y, Pos.z);
	D3DXMatrixScaling(&matScaling, Scale.x, Scale.y, Scale.z);

	D3DXMatrixMultiply(&matWorld, &matScaling, &mat);
	D3DXMatrixMultiply(&matWorld, &matWorld, &matTranslation);
	RGetDevice()->SetTransform(D3DTS_WORLD, &matWorld);

	// State
	RGetDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, (DWORD)((BYTE)(0xFF*fOpacity))<<24);
//	RGetDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
	// Render

//	RGetDevice()->SetStreamSource(0, m_pVB, 0,sizeof(CUSTOMVERTEX));
	RGetDevice()->SetStreamSource(0, GetCommonRectVertexBuffer(), 0, sizeof(CUSTOMVERTEX));
	RGetDevice()->SetFVF(D3DFVF_CUSTOMVERTEX);
	RGetDevice()->SetTexture(0, m_pTex->m_pTex);
	RGetDevice()->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
//	RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_pVertices, sizeof(CUSTOMVERTEX));
	RGetDevice()->SetStreamSource( 0, NULL, 0, 0 );
	/*
	// 이전 상태 복구
	D3DXMatrixIdentity(&matWorld);
	RGetDevice()->SetTransform(D3DTS_WORLD, &matWorld);

	RGetDevice()->ApplyStateBlock(dwStateBlock);
	RGetDevice()->DeleteStateBlock(dwStateBlock);
	*/
	return true;
}

ZEffectBillboard::ZEffectBillboard(ZEffectBillboardSource* pEffectBillboardSource)
{
	m_Pos.x = m_Pos.y = m_Pos.z = 0;
	m_Scale.x = m_Scale.y = m_Scale.z = 1;
	m_fOpacity = 1;
	m_Normal.x = m_Normal.y = 0; m_Normal.z = 1;
	m_Up.x = m_Up.y = 1; m_Up.z = 0;

	m_pEffectBillboardSource = pEffectBillboardSource;
}

ZEffectBillboard::~ZEffectBillboard(void)
{
}

bool ZEffectBillboard::Draw(unsigned long int nTime)
{
	if(m_pEffectBillboardSource!=NULL) {
		if(m_bRender) {
			m_bisRendered = m_pEffectBillboardSource->Draw(m_Pos, m_Normal, m_Up, m_Scale, m_fOpacity);
		}
		else m_bisRendered = false;
	}
	return true;
}



ZEffectBillboardDrawer::ZEffectBillboardDrawer(void)
{
//	m_pVB = NULL;
	m_bCreate = false;
}

ZEffectBillboardDrawer::~ZEffectBillboardDrawer(void)
{
//	SAFE_RELEASE(m_pVB);
	m_bCreate = false;
}

void ZEffectBillboardDrawer::Create(void)
{
	if(m_bCreate) return;
/*
	static CUSTOMVERTEX Billboard[] = {
		{-1, -1, 0, 0xFFFFFFFF, 1, 0},
		{-1, 1, 0, 0xFFFFFFFF, 1, 1},
		{1, 1, 0, 0xFFFFFFFF, 0, 1},
		{1, -1, 0, 0xFFFFFFFF, 0, 0},
	};

	memcpy(m_pVertices, Billboard, sizeof(CUSTOMVERTEX)*4);
*/
/*
	if(FAILED(RGetDevice()->CreateVertexBuffer(sizeof(Billboard), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_MANAGED, &m_pVB,NULL))) return;
	BYTE* pVertices;
	if(FAILED(m_pVB->Lock(0, sizeof(Billboard), &pVertices, 0))) return;
	memcpy(pVertices, Billboard, sizeof(Billboard));
	m_pVB->Unlock();
*/
	m_bCreate = true;
}

bool ZEffectBillboardDrawer::Draw(LPDIRECT3DTEXTURE9 pEffectBillboardTexture, rvector &Pos, rvector &Dir, rvector &Up, rvector &Scale, float fOpacity)
{
	if(isInViewFrustum( Pos, RGetViewFrustum())==false) {
		return false;
	}

	/*
	// 이전 상태 저장
	DWORD dwStateBlock;
	RGetDevice()->CreateStateBlock(D3DSBT_PIXELSTATE, &dwStateBlock);
	RGetDevice()->CaptureStateBlock(dwStateBlock);
	*/


	// Transform
	rmatrix matTranslation, matScaling, matWorld;

	rvector right;
	D3DXVec3Cross(&right, &Up, &Dir);
	D3DXVec3Normalize(&right, &right);

	rvector up;
	D3DXVec3Cross(&up, &right, &Dir);
	D3DXVec3Normalize(&up, &up);

	rvector dir = RCameraDirection;

	rmatrix mat;
	D3DXMatrixIdentity(&mat);
	mat._11=right.x;mat._12=right.y;mat._13=right.z;
	mat._21=up.x;mat._22=up.y;mat._23=up.z;
	mat._31=dir.x;mat._32=dir.y;mat._33=dir.z;

	D3DXMatrixTranslation(&matTranslation, Pos.x, Pos.y, Pos.z);
	D3DXMatrixScaling(&matScaling, Scale.x, Scale.y, Scale.z);

	D3DXMatrixMultiply(&matWorld, &matScaling, &mat);
	D3DXMatrixMultiply(&matWorld, &matWorld, &matTranslation);
	RGetDevice()->SetTransform(D3DTS_WORLD, &matWorld);

	// State
//	RGetDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, (DWORD)((BYTE)(0xFF*fOpacity))<<24);
	RGetDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);

	// Render
//	RGetDevice()->SetStreamSource(0, m_pVB, 0,sizeof(CUSTOMVERTEX));
	RGetDevice()->SetStreamSource(0, GetCommonRectVertexBuffer(), 0,sizeof(CUSTOMVERTEX));
	RGetDevice()->SetFVF(D3DFVF_CUSTOMVERTEX);
	RGetDevice()->SetTexture(0, pEffectBillboardTexture);
	RGetDevice()->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	RGetDevice()->SetStreamSource( 0, NULL, 0, 0 );
//	RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_pVertices, sizeof(CUSTOMVERTEX));

	/*
	// 이전 상태 복구
	D3DXMatrixIdentity(&matWorld);
	RGetDevice()->SetTransform(D3DTS_WORLD, &matWorld);

	RGetDevice()->ApplyStateBlock(dwStateBlock);
	RGetDevice()->DeleteStateBlock(dwStateBlock);
	*/
	return true;
}

ZEffectBillboardDrawer	ZEffectBillboard2::m_EffectBillboardDrawer;

ZEffectBillboard2::ZEffectBillboard2(LPDIRECT3DTEXTURE9 pEffectBillboardTexture)
{
	m_Pos.x = m_Pos.y = m_Pos.z = 0;
	m_Scale.x = m_Scale.y = m_Scale.z = 1;
	m_fOpacity = 1;
	m_Normal.x = m_Normal.y = 0; m_Normal.z = 1;
	m_Up.x = m_Up.y = 1; m_Up.z = 0;

	m_pEffectBillboardTexture = pEffectBillboardTexture;

	if(m_EffectBillboardDrawer.IsCreated()==false) m_EffectBillboardDrawer.Create();
}

ZEffectBillboard2::~ZEffectBillboard2(void)
{
}

bool ZEffectBillboard2::Draw(unsigned long int nTime)
{
	if(m_bRender)
		m_bisRendered = m_EffectBillboardDrawer.Draw(m_pEffectBillboardTexture, m_Pos, m_Normal, m_Up, m_Scale, m_fOpacity);
	else m_bisRendered = false;

	return true;
}

