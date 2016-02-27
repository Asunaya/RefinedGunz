#include "stdafx.h"
#include "Portal.h"
#include "NewChat.h"
#include "ZConfiguration.h"
#include "Hitboxes.h"
#include "Draw.h"

void OnCreateDevice()
{
#ifdef PORTAL
	g_pPortal = new Portal();
#endif
	g_pChat = new Chat(std::string("Arial"), 16);
	g_pChat->SetBackgroundColor(ZGetConfiguration()->GetChatBackgroundColor());

	g_pHitboxManager = new HitboxManager();
}

HRESULT GenerateTexture(IDirect3DDevice9 *pD3Ddev, IDirect3DTexture9 **ppD3Dtex, DWORD colour32){
	if (pD3Ddev->CreateTexture(8, 8, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, ppD3Dtex, NULL))
		return E_FAIL;

	WORD colour16 = ((WORD)((colour32 >> 28) & 0xF) << 12) | (WORD)(((colour32 >> 20) & 0xF) << 8) | (WORD)(((colour32 >> 12) & 0xF) << 4) | (WORD)(((colour32 >> 4) & 0xF) << 0);

	D3DLOCKED_RECT d3dlr;
	(*ppD3Dtex)->LockRect(0, &d3dlr, 0, 0);
	WORD *pDst16 = (WORD*)d3dlr.pBits;

	for (int xy = 0; xy < 8 * 8; xy++)
		*pDst16++ = colour16;

	(*ppD3Dtex)->UnlockRect(0);

	return S_OK;
}