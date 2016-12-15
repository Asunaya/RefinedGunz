#include "stdafx.h"
#include "Draw.h"
#include "RGMain.h"
#include <array>

Draw g_Draw;

void Draw::OnCreateDevice()
{
	Font.Create("DrawFont", "Arial", int(float(FontSize) / 1080 * RGetScreenHeight() + 0.5), 1, true);
}

void Draw::OnReset()
{
	/*SafeDestroy(Font);

	Font.Create("DrawFont", "Arial", int(float(FontSize) / 1080 * RGetScreenHeight() + 0.5), 1, true);*/
}

void Draw::Line(const D3DXVECTOR2 &v1, const D3DXVECTOR2 &v2, D3DCOLOR Color, float z)
{
	ScreenSpaceColorVertex v;

	v.x = v1.x;
	v.y = v1.y;
	v.z = z;
	v.rhw = z;
	v.color = Color;
	Lines.push_back(v);

	v.x = v2.x;
	v.y = v2.y;
	Lines.push_back(v);

	if (Color < 0xFF000000)
	{
		Alpha = true;
	}

	if (!BegunDrawing)
		EndDraw();
}

void Draw::Quad(const D3DXVECTOR2 &v1, const D3DXVECTOR2 &v2, D3DCOLOR Color, float z)
{
	ScreenSpaceColorVertex v;

	v.x = v1.x;
	v.y = v1.y;
	v.z = z;
	v.rhw = z;
	v.color = Color;
	Triangles.push_back(v);

	v.x = v2.x;
	v.y = v1.y;
	Triangles.push_back(v);

	v.x = v1.x;
	v.y = v2.y;
	Triangles.push_back(v);

	v.x = v2.x;
	v.y = v2.y;
	Triangles.push_back(v);

	v.x = v1.x;
	v.y = v2.y;
	Triangles.push_back(v);

	v.x = v2.x;
	v.y = v1.y;
	Triangles.push_back(v);

	if (Color < 0xFF000000)
	{
		Alpha = true;
	}

	if (!BegunDrawing)
		EndDraw();
}

void Draw::TexturedQuad(const D3DXVECTOR2 & v1, const D3DXVECTOR2 & v2, IDirect3DTexture9 * Texture, D3DCOLOR Color, float z)
{
	std::array<ScreenSpaceTexVertex, 6> Triangles;

	int i = 0;

	auto Set = [&] (bool a, bool b)
	{
		Triangles[i].x = a ? v1.x : v2.x;
		Triangles[i].y = b ? v1.y : v2.y;
		Triangles[i].z = 0;
		Triangles[i].rhw = 0;
		Triangles[i].tu = a ? 0 : 1;
		Triangles[i].tv = b ? 0 : 1;

		i++;
	};

	Set(true, true);
	Set(false, true);
	Set(false, false);
	Set(true, false);

	RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	RGetDevice()->SetTexture(0, Texture);
	RGetDevice()->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	RGetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	
	RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Triangles.data(), sizeof(ScreenSpaceTexVertex));
}

void Draw::Border(const D3DXVECTOR2 &v1, const D3DXVECTOR2 &v2, D3DCOLOR Color, float z)
{
	Line(D3DXVECTOR2(v1.x, v1.y), D3DXVECTOR2(v1.x, v2.y), Color, z);
	Line(D3DXVECTOR2(v1.x, v2.y), D3DXVECTOR2(v2.x, v2.y), Color, z);
	Line(D3DXVECTOR2(v2.x, v2.y), D3DXVECTOR2(v2.x, v1.y), Color, z);
	Line(D3DXVECTOR2(v2.x, v1.y), D3DXVECTOR2(v1.x, v1.y), Color, z);
}

void Draw::Text(const char* Str, int x, int y)
{
	Font.m_Font.DrawTextA(x, y, Str);
}

void Draw::BeginDraw()
{
	BegunDrawing = true;
}

void Draw::EndDraw()
{
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	if (Alpha)
	{
		RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		RGetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}

	int nLines = Lines.size() / 2;

	if (nLines)
		RGetDevice()->DrawPrimitiveUP(D3DPT_LINELIST, nLines, Lines.data(), sizeof(ScreenSpaceColorVertex));

	int nTriangles = Triangles.size() / 3;

	if (nTriangles)
		RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, nTriangles, Triangles.data(), sizeof(ScreenSpaceColorVertex));

	if (Alpha)
	{
		RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}

	Lines.clear();
	Triangles.clear();

	Alpha = false;
	BegunDrawing = false;
}