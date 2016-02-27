#pragma once
#include <list>
#include "d3dx9.h"

class HitboxManager
{
public:
	HitboxManager();
	~HitboxManager();

	void Draw();
	void Update();
	void OnSlash(const D3DXVECTOR3 &Pos, const D3DXVECTOR3 &Dir);
	void OnMassive(const D3DXVECTOR3 &Pos);
	void AddEnemyPositions();

private:
	enum Type
	{
		SLASH,
		MASSIVE,
		POS,
		TYPE_END,
	};
	struct Info
	{
		D3DXVECTOR3 Pos, Dir;
		Type type;
		float Time;
		D3DXMATRIX World;
	};
	std::list<Info> List;

	struct Vertex
	{
		float x, y, z;
	};
	IDirect3DVertexBuffer9 *pVB;

	bool Enabled() const;
};

extern HitboxManager *g_pHitboxManager;