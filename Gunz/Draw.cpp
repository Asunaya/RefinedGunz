#include "stdafx.h"
#include "Draw.h"

Draw g_Draw;

void Draw::Line(const D3DXVECTOR2 &v1, const D3DXVECTOR2 &v2, D3DCOLOR Color, float z)
{
	ScreenSpaceVertex v;

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
		bAlpha = true;
	}

	if (!bBegunDrawing)
		EndDraw();
}

void Draw::Quad(const D3DXVECTOR2 &v1, const D3DXVECTOR2 &v2, D3DCOLOR Color, float z)
{
	ScreenSpaceVertex v;

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
		bAlpha = true;
	}

	if (!bBegunDrawing)
		EndDraw();
}

void Draw::Border(const D3DXVECTOR2 &v1, const D3DXVECTOR2 &v2, D3DCOLOR Color, float z)
{
	Line(D3DXVECTOR2(v1.x, v1.y), D3DXVECTOR2(v1.x, v2.y), Color, z);
	Line(D3DXVECTOR2(v1.x, v2.y), D3DXVECTOR2(v2.x, v2.y), Color, z);
	Line(D3DXVECTOR2(v2.x, v2.y), D3DXVECTOR2(v2.x, v1.y), Color, z);
	Line(D3DXVECTOR2(v2.x, v1.y), D3DXVECTOR2(v1.x, v1.y), Color, z);
}

void Draw::BeginDraw()
{
	bBegunDrawing = true;
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

	if (bAlpha)
	{
		RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		RGetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}

	int nLines = Lines.size() / 2;

	if (nLines)
		RGetDevice()->DrawPrimitiveUP(D3DPT_LINELIST, nLines, Lines.data(), sizeof(ScreenSpaceVertex));

	int nTriangles = Triangles.size() / 3;

	if (nTriangles)
		RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, nTriangles, Triangles.data(), sizeof(ScreenSpaceVertex));

	if (bAlpha)
	{
		RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}

	Lines.clear();
	Triangles.clear();

	bAlpha = false;
	bBegunDrawing = false;
}