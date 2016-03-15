#include "stdafx.h"
#include "Hitboxes.h"
#include "RGMain.h"
#include "Rules.h"
#include "ZConfiguration.h"

HitboxManager *g_pHitboxManager;

static const int nSlashVertices = 17;
static const int nMassiveVertices = 100;
static const int nVBOffset[] = { 0, nSlashVertices, nSlashVertices + nMassiveVertices };

HitboxManager::HitboxManager()
{
	static const int size = (nSlashVertices + nMassiveVertices + 2) * sizeof(Vertex);
	if (FAILED(RGetDevice()->CreateVertexBuffer(size, 0, D3DFVF_XYZ, D3DPOOL_MANAGED, &pVB, NULL)))
	{
		MLog("HitboxManager::HitboxManager() - Failed to create vertex buffer\n");
		return;
	}

	Vertex *pV;
	if (FAILED(pVB->Lock(0, size, (void **)&pV, 0)))
	{
		MLog("HitboxManager::HitboxManager() - Failed to lock vertex buffer\n");
		return;
	}

	pV[0].x = 0;
	pV[0].y = 0;
	pV[0].z = 0;

	for (int i = 1; i < nSlashVertices - 1; i++)
	{
		float f = (float(i - 1) / (nSlashVertices - 2) - 0.5) * (120.f / 360.f);

		pV[i].x = -sin(f * TAU) * RG_SLASH_RADIUS;
		pV[i].y = 0;
		pV[i].z = -cos(f * TAU) * RG_SLASH_RADIUS;
	}

	// End of line strip
	pV[nSlashVertices - 1].x = 0;
	pV[nSlashVertices - 1].y = 0;
	pV[nSlashVertices - 1].z = -cos(0) * RG_SLASH_RADIUS;

	for (int i = nSlashVertices; i < nSlashVertices + nMassiveVertices - 1; i++)
	{
		float f = float(i - nSlashVertices) / nMassiveVertices;

		pV[i].x = sin(f * TAU) * RG_MASSIVE_RADIUS;
		pV[i].y = cos(f * TAU) * RG_MASSIVE_RADIUS;
		pV[i].z = 0;
	}

	// End of line strip
	pV[nSlashVertices + nMassiveVertices - 1].x = 0;
	pV[nSlashVertices + nMassiveVertices - 1].y = cos(0) * RG_MASSIVE_RADIUS;
	pV[nSlashVertices + nMassiveVertices - 1].z = 0;

	// Position line
	int i = nSlashVertices + nMassiveVertices;
	pV[i].x = 0;
	pV[i].y = 0;
	pV[i].z = -180;
	i++;

	pV[i].x = 0;
	pV[i].y = 0;
	pV[i].z = 180;

	if (FAILED(pVB->Unlock()))
	{
		MLog("HitboxManager::HitboxManager() - Failed to unlock vertex buffer\n");
		return;
	}
}

HitboxManager::~HitboxManager()
{
	SAFE_RELEASE(pVB);
}

void HitboxManager::OnSlash(const D3DXVECTOR3 &Pos, const D3DXVECTOR3 &Dir)
{
	if (!Enabled())
		return;

	Info i;
	i.Pos = Pos;
	i.Pos.z += 10;
	i.Dir = Dir;
	i.Dir.z = 0;
	Normalize(i.Dir);
	i.type = SLASH;
	i.Time = ZGetGame()->GetTime();
	MakeWorldMatrix(&i.World, i.Pos, i.Dir, D3DXVECTOR3(0, 0, 1));
	List.push_back(i);

	AddEnemyPositions();
}

void HitboxManager::OnMassive(const D3DXVECTOR3 &Pos)
{
	if (!Enabled())
		return;

	Info i;
	i.Pos = Pos;
	i.Pos.z += 10;
	i.Dir = D3DXVECTOR3(1, 0, 0);
	i.type = MASSIVE;
	i.Time = ZGetGame()->GetTime();
	D3DXMatrixTranslation(&i.World, i.Pos.x, i.Pos.y, i.Pos.z);
	List.push_back(i);

	AddEnemyPositions();
}

void HitboxManager::AddEnemyPositions()
{
	for (auto &it : *ZGetCharacterManager())
	{
		auto pChar = it.second;

		Info i;
		i.Pos = pChar->GetPosition();
		i.type = POS;
		i.Time = ZGetGame()->GetTime();
		D3DXMatrixTranslation(&i.World, i.Pos.x, i.Pos.y, i.Pos.z);
		List.push_back(i);
	}
}

void HitboxManager::Update()
{
}

void HitboxManager::Draw()
{
	if (!Enabled())
		return;

	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CONSTANT);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CONSTANT);
	RGetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetFVF(D3DFVF_XYZ);

	RGetDevice()->SetStreamSource(0, pVB, 0, sizeof(Vertex));

	for (auto it = List.begin(); it != List.end(); it++)
	{
		auto &val = *it;

		float t = ZGetGame()->GetTime();

		if (val.Time + 5 < t)
		{
			it = List.erase(it);
			if (it == List.end())
				break;
		}

		int alpha = int((1 - ((t - val.Time) / 5)) * 0x40);
		DWORD Color = (alpha << 24) | 0xFF0000;
		RGetDevice()->SetTextureStageState(0, D3DTSS_CONSTANT, Color);

		RGetDevice()->SetTransform(D3DTS_WORLD, &val.World);

		if (val.type == SLASH)
		{
			RGetDevice()->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, nSlashVertices - 3);
			RGetDevice()->SetTextureStageState(0, D3DTSS_CONSTANT, 0xFFFF0000);
			RGetDevice()->DrawPrimitive(D3DPT_LINESTRIP, 0, nSlashVertices - 1);
		}
		else if (val.type == MASSIVE)
		{
			RGetDevice()->DrawPrimitive(D3DPT_TRIANGLEFAN, nSlashVertices, nMassiveVertices - 3);
			RGetDevice()->SetTextureStageState(0, D3DTSS_CONSTANT, 0xFFFF0000);
			RGetDevice()->DrawPrimitive(D3DPT_LINESTRIP, nSlashVertices, nMassiveVertices - 1);
		}
		else if (val.type == POS)
		{
			RGetDevice()->SetTextureStageState(0, D3DTSS_CONSTANT, 0xFF000080);
			RGetDevice()->DrawPrimitive(D3DPT_LINESTRIP, nSlashVertices + nMassiveVertices, 1);
		}
	}
}

bool HitboxManager::Enabled() const
{
	return ZGetConfiguration()->GetShowHitboxes();
}