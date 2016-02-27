#pragma once
#include <d3dx9.h>
#include <vector>

class Draw
{
	struct ScreenSpaceVertex {
		float x, y, z, rhw;
		DWORD color;
	};

	std::vector<ScreenSpaceVertex> Lines;
	std::vector<ScreenSpaceVertex> Triangles;
	bool bAlpha;
	bool bBegunDrawing;
public:
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
	void Border(const D3DXVECTOR2 &v1, const D3DXVECTOR2 &v2, D3DCOLOR Color = 0xFF00A5C3, float z = 0);

	void BeginDraw();
	void EndDraw();
};

extern Draw g_Draw;