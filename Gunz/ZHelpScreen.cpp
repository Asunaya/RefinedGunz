#include "stdafx.h"
#include "ZConfiguration.h"
#include "RealSpace2.h"
#include "Mint.h"
#include "Mint4r2.h"
#include "ZActiondef.h"
#include "ZHelpScreen.h"
#include "ZInput.h"

_USING_NAMESPACE_REALSPACE2

static int Floorer2PowerSize(int v)
{
	if(v<=2)			return 2;
	else if(v<=4)		return 4;
	else if(v<=8)		return 8;
	else if(v<=16)		return 16;
	else if(v<=32)		return 32;
	else if(v<=64)		return 64;
	else if(v<=128)		return 128;
	else if(v<=256)		return 256;
	else if(v<=512)		return 512;
	else if(v<=1024)	return 1024;

	_ASSERT(FALSE);	// Too Big!

	return 2;
}

struct CUSTOMVERTEX{
	FLOAT	x, y, z, rhw;
	FLOAT	tu, tv;
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_TEX1)

#define NUM_BAR_BITMAP	1
#define IL_TEXT_BUFFER_SIZE 512
#define ADJUST_SIZE				0.5f
#define ADJUST_SIZE2			0.0f
#define FADEIN_NUM_DRAWING		120
#define FADEOUT_NUM_DRAWING		120
#define BLENDING_NUM_DRAWING	240

extern MDrawContextR2* g_pDC;

void TextOutKeymap(int skey,int x,int y)
{
	static char szTemp[256] = "";

	ZCONFIG_KEYBOARD* pKey = ZGetConfiguration()->GetKeyboard();
	int nScanCode = pKey->ActionKeys[ skey ].nVirtualKey;

	char pStr[128];
	ZGetInput()->GetInputKeyName(nScanCode,pStr,sizeof(pStr));
	int nLen = (int)strlen(pStr);
	if(nLen>5)
	{
		pStr[4]='.';
		pStr[5]='.';
		pStr[6]=0;
		nLen = 5;
	}
	if(nLen) { nLen/=2;	}

	sprintf_safe(szTemp,"%s",pStr);
	g_pDC->Text( x-nLen*7, y, szTemp);
}

ZHelpScreen::ZHelpScreen()
{
	m_bDrawHelpScreen = false;
	m_pHelpScreenBitmap = NULL;
}

ZHelpScreen::~ZHelpScreen()
{
	if(m_pHelpScreenBitmap) {
		delete m_pHelpScreenBitmap;
		m_pHelpScreenBitmap = NULL;
	}
}

void ZHelpScreen::ChangeMode() {

	m_bDrawHelpScreen = !m_bDrawHelpScreen;

	if(m_bDrawHelpScreen) {
		m_pHelpScreenBitmap = new MBitmapR2;
		m_pHelpScreenBitmap->Create("HelpScreen",RGetDevice(), "Interface/default/help/key.png" );
	}
	else {
		if(m_pHelpScreenBitmap) {
			delete m_pHelpScreenBitmap;
			m_pHelpScreenBitmap = NULL;
		}
	}
}

void ZHelpScreen::DrawHelpScreen()
{
	if( !m_bDrawHelpScreen )
		return;

	if(m_pHelpScreenBitmap==NULL) 
		return;

	if(m_pHelpScreenBitmap->m_pTexture==NULL)
		return;

	float mx = 0.f;
	float my = 0.f;
	float mw = (float)RGetScreenWidth();
	float mh = (float)RGetScreenHeight();

	float msx = 0.f;
	float msy = 0.f;
	float msw = 800.f;
	float msh = 600.f;

	float ftw = 800.f;//(float)m_pHelpScreenBitmap->m_pTexture->GetWidth();
	float fth = 600.f;//(float)m_pHelpScreenBitmap->m_pTexture->GetHeight();
	float msw2 = msw; 
	float msh2 = msh;

	D3DFORMAT d3dformat = m_pHelpScreenBitmap->m_pTexture->GetFormat();

	if( d3dformat==D3DFMT_DXT1 ||
		d3dformat==D3DFMT_DXT2 ||
		d3dformat==D3DFMT_DXT3 ||
		d3dformat==D3DFMT_DXT4 ||
		d3dformat==D3DFMT_DXT5 )
	{
		msw2 = (float)Floorer2PowerSize((int)msw);
		msh2 = (float)Floorer2PowerSize((int)msh);
	}

	RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE);

	CUSTOMVERTEX Sprite[4] = 
	{
		{mx      - ADJUST_SIZE , my      - ADJUST_SIZE , 0, 1.0f, (msx)/ftw       , (msy)/fth },
		{mx + mw - ADJUST_SIZE2, my      - ADJUST_SIZE , 0, 1.0f, (msx + msw2)/ftw, (msy)/fth },
		{mx + mw - ADJUST_SIZE2, my + mh - ADJUST_SIZE2, 0, 1.0f, (msx + msw2)/ftw, (msy + msh2)/fth },
		{mx      - ADJUST_SIZE , my + mh - ADJUST_SIZE2, 0, 1.0f, (msx)/ftw       , (msy + msh2)/fth},
	};

	RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	RGetDevice()->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );	
	RGetDevice()->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );

	RGetDevice()->SetFVF(D3DFVF_CUSTOMVERTEX);
	RGetDevice()->SetTexture( 0, m_pHelpScreenBitmap->m_pTexture->GetTexture() );
	RGetDevice()->SetSamplerState( 0, D3DSAMP_MAGFILTER , D3DTEXF_POINT );
	RGetDevice()->SetSamplerState( 0, D3DSAMP_MINFILTER , D3DTEXF_POINT );

	RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Sprite, sizeof(CUSTOMVERTEX));

	RGetDevice()->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	RGetDevice()->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );

	g_pDC->SetColor(MCOLOR(0xFF000000));

	g_pDC->SetFont(MFontManager::Get("FONTb11b"));

	float aspect_x = RGetScreenWidth()/800.f;
	float aspect_y = RGetScreenHeight()/600.f;

	MFontR2* pFont = (MFontR2*)g_pDC->GetFont();
	pFont->m_fScale = 1.0f * aspect_x;

	int nHeight = 193 * aspect_y;

	TextOutKeymap(ZACTION_MELEE_WEAPON    , 90*aspect_x,nHeight);
	TextOutKeymap(ZACTION_PRIMARY_WEAPON  ,148*aspect_x,nHeight);
	TextOutKeymap(ZACTION_SECONDARY_WEAPON,209*aspect_x,nHeight);
	TextOutKeymap(ZACTION_ITEM1           ,265*aspect_x,nHeight);
	TextOutKeymap(ZACTION_ITEM2           ,323*aspect_x,nHeight);

	nHeight = 258 * aspect_y;

	TextOutKeymap(ZACTION_PREV_WEAPON  ,105*aspect_x,nHeight);
	TextOutKeymap(ZACTION_FORWARD      ,162*aspect_x,nHeight);
	TextOutKeymap(ZACTION_NEXT_WEAPON  ,222*aspect_x,nHeight);
	TextOutKeymap(ZACTION_RELOAD       ,280*aspect_x,nHeight);

	nHeight = 323 * aspect_y;

	TextOutKeymap(ZACTION_LEFT     ,120*aspect_x,nHeight);
	TextOutKeymap(ZACTION_BACK     ,178*aspect_x,nHeight);
	TextOutKeymap(ZACTION_RIGHT    ,239*aspect_x,nHeight);

	nHeight = 390 * aspect_y;

	TextOutKeymap(ZACTION_JUMP     ,196*aspect_x,nHeight);

	pFont->m_fScale = 1.0f;
}
