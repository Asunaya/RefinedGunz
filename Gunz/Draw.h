#pragma once
#include "d3dx9.h"
#include <vector>
#include "VertexTypes.h"

class Draw
{
public:
	void OnCreateDevice();
	void OnReset();

	void Line(float x1, float y1, float x2, float y2, D3DCOLOR Color = 0xFF00A5C3, float z = 0) {
		Line(D3DXVECTOR2(x1, y1), D3DXVECTOR2(x2, y2), Color, z);
	}
	void Line(const D3DXVECTOR2 &v1, const D3DXVECTOR2 &v2, D3DCOLOR Color = 0xFF00A5C3, float z = 0);
	void Quad(float x1, float y1, float x2, float y2, D3DCOLOR Color = 0xFF00A5C3) {
		Quad(D3DXVECTOR2(x1, y1), D3DXVECTOR2(x2, y2), Color);
	}
	void Quad(const D3DRECT &r, D3DCOLOR Color = 0xFF00A5C3, float z = 0) {
		Quad(D3DXVECTOR2(r.x1, r.y1), D3DXVECTOR2(r.x2, r.y2), Color, z);
	}
	void Quad(const D3DXVECTOR2 &v1, const D3DXVECTOR2 &v2, D3DCOLOR Color = 0xFF00A5C3, float z = 0);

	void TexturedQuad(const D3DXVECTOR2 &v1, const D3DXVECTOR2 &v2, IDirect3DTexture9 *Texture, D3DCOLOR Color = 0xFF00A5C3, float z = 0);

	void Border(const D3DXVECTOR2 &v1, const D3DXVECTOR2 &v2, D3DCOLOR Color = 0xFF00A5C3, float z = 0);

	void Text(const char * Str, int x, int y);

	void BeginDraw();
	void EndDraw();

private:
	std::vector<ScreenSpaceColorVertex> Lines;
	std::vector<ScreenSpaceColorVertex> Triangles;
	bool Alpha;
	bool BegunDrawing;

	MFontR2 Font;
	static constexpr int FontSize = 16;
};

extern Draw g_Draw;