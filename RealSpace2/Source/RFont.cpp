#include "stdafx.h"
//#include <afxdlgs.h>
#include "RFont.h"
#include <crtdbg.h>
//#include <dxutil.h>		// Program Files\DXSDK\Samples\C++\Common\Include
#include <mbstring.h>
#include <tchar.h>
#include "mDebug.h"
#include "mprofiler.h"
#include "dxerr9.h"
#pragma comment(lib, "dxerr9.lib")

#ifdef _USE_GDIPLUS
#include "unknwn.h"
#include "gdiplus.h"
	 using namespace Gdiplus;
#pragma comment(lib,"gdiplus.lib")
#endif

_USING_NAMESPACE_REALSPACE2

_NAMESPACE_REALSPACE2_BEGIN

//#undef _USE_GDIPLUS

// 모든 rfont 는 g_FontTexture 하나만을 사용한다. ( 16x16 셀로 이루어진 하나의 텍스쳐를 버퍼로 쓴다 )

#define TEXTURE_SIZE	512
#define CELL_SIZE		32

RFontTexture::RFontTexture() 
{ 
	m_pTexture = NULL;
	m_CellInfo = NULL;
}

RFontTexture::~RFontTexture()
{
	Destroy();
}

bool RFontTexture::Create() 
{
	m_nWidth = TEXTURE_SIZE;
	m_nHeight = TEXTURE_SIZE;
	HRESULT hr = RGetDevice()->CreateTexture(m_nWidth, m_nHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_pTexture,NULL);
	if(hr!=D3D_OK) return false;

	m_nX = m_nWidth/CELL_SIZE;
	m_nY = m_nHeight/CELL_SIZE;

	m_nCell = m_nX * m_nY;
	m_CellInfo = new RFONTTEXTURECELLINFO[m_nCell];
	for(int i=0;i<m_nCell;i++) {
		m_CellInfo[i].nID = 0;
		m_CellInfo[i].nIndex = i;
		m_PriorityQueue.push_back(&m_CellInfo[i]);
	}

	m_LastUsedID = 0;

//	memory dc 와 buffer 를 만든다

	m_hDC = CreateCompatibleDC(NULL);

	////////////////////////////////////////////////--------------------------------------------------------
	// Bitmap Creation
	BITMAPINFO bmi;
	ZeroMemory(&bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
	bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth       =  (int)CELL_SIZE;
	bmi.bmiHeader.biHeight      = -(int)CELL_SIZE;
	bmi.bmiHeader.biPlanes      = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount    = 32;

	m_hbmBitmap = CreateDIBSection(m_hDC, &bmi, DIB_RGB_COLORS, (VOID**)&m_pBitmapBits, NULL, 0);
	if( m_pBitmapBits == NULL )
	{
		DeleteDC(m_hDC);
		return false;
	}

	HBITMAP m_hPrevBitmap = (HBITMAP)SelectObject(m_hDC, m_hbmBitmap);

	SetMapMode(m_hDC, MM_TEXT);

	return true;
}

void RFontTexture::Destroy() {
	
	if(m_hDC) {
		SelectObject(m_hDC, m_hPrevBitmap);
		if(m_hbmBitmap)
		{
			DeleteObject(m_hbmBitmap);
			m_hbmBitmap = NULL;
		}
		DeleteDC(m_hDC);
		m_hDC = NULL;
	}
	SAFE_RELEASE(m_pTexture);
	SAFE_DELETE_ARRAY(m_CellInfo);
}

void BlitRect(BYTE* pDest, int x1,int y1,int x2,int y2,int w, int h, DWORD* pBitmapBits, INT Pitch)
{
	if(pDest==NULL) return;

	DWORD* pDestTemp = NULL;

	int by=0;
	int bx=0;

	for(int y=y1; y<y2; y++) {
		pDestTemp = (DWORD*)(pDest+(y*Pitch));
		bx = 0;
		for(int x=x1; x<x2; x++) {

			DWORD dwPixel = pBitmapBits[(w * by) + (bx)];
			if ( dwPixel & 0x00ffffff)	//색이 값이 있다면~
				dwPixel |= 0xff000000;

			*(pDestTemp+x) = dwPixel;

			bx++;
		}
		by++;
	}
}


bool RFontTexture::UploadTexture(RCHARINFO *pCharInfo,DWORD* pBitmapBits,int w,int h)
{
	D3DLOCKED_RECT d3dlr;
	HRESULT hr = m_pTexture->LockRect(0,&d3dlr,NULL,NULL);

	if(hr == D3D_OK)
	{
		m_LastUsedID++;

		RFONTTEXTURECELLINFO *pInfo = *(m_PriorityQueue.begin());
		pInfo->nID = m_LastUsedID;
		pInfo->itr = m_PriorityQueue.begin();

		int x = pInfo->nIndex % GetCellCountX();
		int y = pInfo->nIndex / GetCellCountX();

		int _x = x*CELL_SIZE;
		int _y = y*CELL_SIZE;

		BlitRect((BYTE*)d3dlr.pBits, _x, _y, _x+w, _y+h, CELL_SIZE, CELL_SIZE, pBitmapBits, d3dlr.Pitch);

		hr = m_pTexture->UnlockRect(0);

		m_PriorityQueue.splice(m_PriorityQueue.end(),m_PriorityQueue,m_PriorityQueue.begin());

		pCharInfo->nFontTextureID = pInfo->nID;
		pCharInfo->nFontTextureIndex = pInfo->nIndex;
		pCharInfo->nWidth = w;

		return true;
	}

	return false;
}

bool RFontTexture::IsNeedUpdate(int nIndex, int nID)		// 갱신되어야 하는지 검사
{
	if(nIndex==-1) return true;
	if(m_CellInfo[nIndex].nID == nID) {
		// 막 사용되어진것이므로 뒤로 돌린다.
		m_PriorityQueue.splice(m_PriorityQueue.end(),m_PriorityQueue,m_CellInfo[nIndex].itr);
		return false;	// 갱신할 필요없다
	}
	return true;
}

bool RFontTexture::MakeFontBitmap(HFONT hFont, RCHARINFO *pInfo, const TCHAR* szText, int nOutlineStyle, DWORD nColorArg1, DWORD nColorArg2)
{
	HFONT hPrevFont = (HFONT)SelectObject(m_hDC, hFont);

	SIZE size;
	GetTextExtentPoint32(m_hDC, szText, _tcslen(szText), &size);

	int nWidth = min(size.cx,CELL_SIZE);

#ifdef _USE_GDIPLUS
	// GDI+ Font
	WCHAR wstrText[256];
	int nTextLen = strlen(szText)+1;
	MultiByteToWideChar(CP_ACP, 0, szText, -1, wstrText, nTextLen-1);
	wstrText[nTextLen-1] = 0;

	Graphics graphics(m_hDC);
	Gdiplus::Font font(m_hDC, hFont);

	//	graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);
	graphics.Clear(Color(0,0,0,0));

	const StringFormat* pTypoFormat = StringFormat::GenericTypographic();

	if (nOutlineStyle == 0)
	{
		SolidBrush  solidBrush(Color(255, 255, 255, 255));
		graphics.DrawString(wstrText, -1, &font, PointF(0.0f, 0.0f), pTypoFormat, &solidBrush);
	}
	else if (nOutlineStyle == 1)	// 아웃라인이 그려진다
	{
		GraphicsPath path;
		FontFamily fontFamily;
		font.GetFamily(&fontFamily);

		TEXTMETRIC tm;
		GetTextMetrics(m_hDC, &tm);

		int nHeight;
		nHeight = min( (int)tm.tmHeight, (int)CELL_SIZE-2);

		path.AddString(	wstrText, -1, &fontFamily, FontStyleBold, nHeight, PointF(-1.0f, -1.0f), pTypoFormat);

		graphics.SetSmoothingMode(SmoothingModeAntiAlias);

		Pen pen(Color(nColorArg2), 2.0f);
		graphics.DrawPath(&pen, &path);

		SolidBrush brush(Color((Gdiplus::ARGB)nColorArg1));
		graphics.FillPath((Brush*)&brush, &path);
	}
	else if (nOutlineStyle == 2)	// 그림자만 포함된다. 좌측의 한픽셀 여백이 있으므로 왼쪽으로 한픽셀. 자간은 같다
	{
		SolidBrush  solidBrush2(Color((Gdiplus::ARGB)nColorArg2));
		graphics.DrawString(wstrText, -1, &font, PointF(1.0f, 1.0f), pTypoFormat, &solidBrush2);

		SolidBrush  solidBrush1(Color((Gdiplus::ARGB)nColorArg1));

		graphics.DrawString(wstrText, -1, &font, PointF(0.0f, 0.0f), pTypoFormat, &solidBrush1);

		// 숫자일 경우엔 자간이 좁기 때문에 1을 늘린다.
		char chChar = (char)wstrText[0];
		if ( (chChar >= '0') && (chChar <= '9'))
			nWidth++;
	}
#else	

	SetTextColor(m_hDC, RGB(255,255,255));
	SetBkColor(m_hDC, 0x00000000);
	SetTextAlign(m_hDC, TA_TOP);
	ExtTextOut(m_hDC, 0, 0, ETO_OPAQUE, NULL, szText, _tcslen(szText), NULL);

#endif

	bool bRet = UploadTexture(pInfo,m_pBitmapBits,nWidth,CELL_SIZE);
	
	SelectObject(m_hDC, hPrevFont);

	return bRet;

}

int RFontTexture::GetCharWidth(HFONT hFont, const TCHAR* szChar)
{
	SIZE size;
	HFONT hPrevFont = (HFONT)SelectObject(m_hDC, hFont);
	GetTextExtentPoint32(m_hDC, szChar, _tcslen(szChar), &size);
	SelectObject(m_hDC, hPrevFont);
	return size.cx;
}

RFontTexture g_FontTexture;

bool RFontCreate()
{
	return g_FontTexture.Create();
}

void RFontDestroy()
{
	g_FontTexture.Destroy();
	RCHARINFO::Release();
}

#define RFONT_VERTEXCOUNT	4
struct FONT2DVERTEX { 
	D3DXVECTOR4 p;   
	DWORD color;     
	FLOAT tu, tv; 
};

#define D3DFVF_FONT2DVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)

#define MAX_FONT_BUFFER 4000

static	int				g_nFontCount=0;
static	FONT2DVERTEX	g_FontVertexBuffer[4*MAX_FONT_BUFFER];//그림자까지 200자정도
static	WORD			g_FontIndexBuffer[6*MAX_FONT_BUFFER];


RFont::RFont()
{
}

RFont::~RFont()
{
	Destroy();
}

bool RFont::Create(const TCHAR* szFontName, int nHeight, bool bBold/* =false */, bool bItalic/* =false */, int nOutlineStyle/* =0 */, int nCacheSize, bool bAntiAlias, DWORD nColorArg1, DWORD nColorArg2)
{
	m_nOutlineStyle = nOutlineStyle;
//	m_nSamplingMultiplier = nSamplingMultiplier;	// 한방향 샘플링 횟수, 2이면 2*2크기를 샘플링한다.
	HDC hDC = GetDC(g_hWnd);
	SetMapMode(hDC, MM_TEXT);
	nHeight = MulDiv(nHeight, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	
	m_nHeight = nHeight;

	m_ColorArg1 = nColorArg1;
	m_ColorArg2 = nColorArg2;
	m_bAntiAlias = bAntiAlias;

	m_hFont = CreateFont(-nHeight, 0, 0, 0, bBold==true?FW_BOLD:FW_NORMAL, bItalic==true?TRUE:FALSE, 
		FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH , szFontName);

	if(m_hFont==NULL)
		return false;
	ReleaseDC(g_hWnd, hDC);

	m_pFontTexture = &g_FontTexture;
	if(m_pFontTexture->GetTexture()==NULL) 
		m_pFontTexture->Create();

	_ASSERT(nHeight <= CELL_SIZE);

	return true;
}

void RFont::Destroy()
{
	DeleteObject(m_hFont);
	m_hFont = NULL;

	while(m_CharInfoMap.size())
	{
		RCHARINFO* pCharInfo = m_CharInfoMap.begin()->second;
		delete pCharInfo;
		m_CharInfoMap.erase(m_CharInfoMap.begin());
	}
}

bool RFont::m_bInFont = false;

bool RFont::BeginFont()
{
	if(m_bInFont) return true;

	LPDIRECT3DDEVICE9 pDevice = RGetDevice();

	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA );
	pDevice->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA );
	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE );
	pDevice->SetRenderState( D3DRS_ALPHAREF,         0x08 );
	pDevice->SetRenderState( D3DRS_ALPHAFUNC,  D3DCMP_GREATEREQUAL );

	const bool bFiltering = true;

	if(bFiltering==true){
		pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	}
	else{
		pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
		pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	}

	pDevice->SetRenderState( D3DRS_LIGHTING,   FALSE );
	pDevice->SetRenderState( D3DRS_SPECULARENABLE,   FALSE );
	pDevice->SetRenderState( D3DRS_FILLMODE,   D3DFILL_SOLID );
	pDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_NONE );
	pDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
	pDevice->SetRenderState( D3DRS_CLIPPING,         TRUE );
//	pDevice->SetRenderState( D3DRS_EDGEANTIALIAS,    FALSE );
	pDevice->SetRenderState( D3DRS_CLIPPLANEENABLE,  FALSE );
	pDevice->SetRenderState( D3DRS_VERTEXBLEND,      FALSE );
	pDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_FOGENABLE,        FALSE );

	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
//	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

	pDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE );

	pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
	pDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	pDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
/*
	if(pDevice->EndStateBlock(&m_dwStateBlock)!=D3D_OK) {
		m_dwStateBlock = NULL;
	}
	pDevice->ApplyStateBlock(m_dwStateBlock);

}
else {
//		m_pd3dDevice->ApplyStateBlock(m_dwStateBlock);
}
*/
	RGetDevice()->SetFVF(D3DFVF_FONT2DVERTEX);
	RGetDevice()->SetTexture(0,g_FontTexture.GetTexture());

	g_nFontCount = 0;

	m_bInFont = true;

	return true;
}

bool RFont::EndFont()
{
	if(!m_bInFont) return true;

	if(g_nFontCount)
	{
		HRESULT hr = RGetDevice()->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,g_nFontCount*4,g_nFontCount*2,
			g_FontIndexBuffer,D3DFMT_INDEX16,g_FontVertexBuffer,sizeof(FONT2DVERTEX));

		_ASSERT(hr==D3D_OK);

		g_nFontCount = 0;
	}

	
	RGetDevice()->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	RGetDevice()->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

	m_bInFont = false;
	return true;
}

void RFont::DrawText(float x, float y, const TCHAR* szText, DWORD dwColor/* =0xFFFFFFFF */, float fScale/* =1.0f */)
{
	if(szText==NULL)	return;
	if(szText[0]==NULL) return;

//	bool bPrevInFont = m_bInFont;	// 하단에서 부적절하게 사용중이다.
	bool bPrevInFont = false;	

	BeginFont();

	const TCHAR* p = (const TCHAR*)szText;
	TCHAR szChar[4];

//	if(m_nAutoScale)
//		fScale = RGetScreenWidth() / 800.f;

	while(1) {

		TCHAR* pp = _tcsinc(p);

		if(pp-p==sizeof(TCHAR)){
			szChar[0] = *p;
			szChar[1] = 0;
		}
		else{
			szChar[0] = *p;
			szChar[1] = *(p+1);
			szChar[2] = 0;
		}
		_ASSERT(pp-p==sizeof(TCHAR)*2 || pp-p==sizeof(TCHAR));

		WORD key = *(WORD*)szChar;

		RCHARINFOMAP::iterator i = m_CharInfoMap.find(key);
		
		RCHARINFO *pInfo = NULL;
		if(i==m_CharInfoMap.end()) {
			pInfo = new RCHARINFO;
			bool bRet = m_pFontTexture->MakeFontBitmap(m_hFont,pInfo,szChar,m_nOutlineStyle,m_ColorArg1,m_ColorArg2);
			if(bRet)
				m_CharInfoMap.insert(RCHARINFOMAP::value_type(key,pInfo));
			else {
				SAFE_DELETE(pInfo);
			}
		}else
		{
			// 갱신할 필요가 있는지 검사
			pInfo = i->second;
			if(m_pFontTexture->IsNeedUpdate(pInfo->nFontTextureIndex,pInfo->nFontTextureID)) {
				m_pFontTexture->MakeFontBitmap(m_hFont,pInfo,szChar,m_nOutlineStyle,m_ColorArg1,m_ColorArg2);
			}
		}

		if(pInfo != NULL) {

			// 버텍스를 만들어서 버퍼에 밀어넣는다
			static FONT2DVERTEX vertices[RFONT_VERTEXCOUNT];
			WORD indices[6] = { 3,0,2,0,1,2 };
			/*		0 3

					1 2			*/

			int nWidth = min(CELL_SIZE,pInfo->nWidth);
/*
			vertices[0].p = D3DXVECTOR4(x,y,0,1);
			vertices[1].p = D3DXVECTOR4(x,y+CELL_SIZE,0,1);
			vertices[2].p = D3DXVECTOR4(x+nWidth,y+CELL_SIZE,0,1);
			vertices[3].p = D3DXVECTOR4(x+nWidth,y,0,1);
*/
			int w = nWidth*fScale;
			int h = CELL_SIZE*fScale;

			if(x+w > RGetViewport()->X && x < RGetViewport()->X+RGetViewport()->Width &&
				y+h > RGetViewport()->Y && y < RGetViewport()->Y+RGetViewport()->Height) 
			{
				vertices[0].p = D3DXVECTOR4(x,y,0,1);
				vertices[1].p = D3DXVECTOR4(x,y+h,0,1);
				vertices[2].p = D3DXVECTOR4(x+w,y+h,0,1);
				vertices[3].p = D3DXVECTOR4(x+w,y,0,1);

				int nCellX = pInfo->nFontTextureIndex % m_pFontTexture->GetCellCountX();
				int nCellY = pInfo->nFontTextureIndex / m_pFontTexture->GetCellCountX();
				
				float fMinX = (float)(.5f+nCellX*CELL_SIZE) / (float)m_pFontTexture->GetWidth();
				float fMaxX = (float)(.5f+nCellX*CELL_SIZE+nWidth) / (float)m_pFontTexture->GetWidth();
				float fMinY = (float)(.5f+nCellY*CELL_SIZE) / (float)m_pFontTexture->GetHeight();
				float fMaxY = (float)(.5f+(nCellY+1)*CELL_SIZE) / (float)m_pFontTexture->GetHeight();
				
				vertices[0].tu = fMinX;vertices[0].tv = fMinY;
				vertices[1].tu = fMinX;vertices[1].tv = fMaxY;
				vertices[2].tu = fMaxX;vertices[2].tv = fMaxY;
				vertices[3].tu = fMaxX;vertices[3].tv = fMinY;

				vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = dwColor;

				for(int i=0;i<6;i++) {
					indices[i]+=g_nFontCount*4;
				}

				memcpy(g_FontVertexBuffer+g_nFontCount*4,vertices,sizeof(vertices));
				memcpy(g_FontIndexBuffer+g_nFontCount*6,indices,sizeof(indices));
				g_nFontCount++;
			}

			if (m_nOutlineStyle==1)
				x += min((pInfo->nWidth*fScale+1),CELL_SIZE);
			else
				x += (pInfo->nWidth*fScale);

		}

		p = pp;

		if(pp==NULL) break;
		if(*pp==0)	 break;
	}

	// 이전에 BeginFont - EndFont 상태가 아니었다면 복구한다.
	if(!bPrevInFont)
		EndFont();
}

int RFont::GetTextWidth(const TCHAR* szText, int nSize /* =-1 */)
{
	if(nSize==0) return 0;
	if(szText==NULL) return 0;
	if(szText[0]==NULL) return 0;

	//	return _tcslen(szText)*8;

	int nWidth=0,nCount=0;

	const TCHAR* p = (const TCHAR*)szText;
	TCHAR szChar[4];

	while(1) {

		TCHAR* pp = _tcsinc(p);

		if(pp-p==sizeof(TCHAR)){
			szChar[0] = *p;
			szChar[1] = 0;
			nCount++;
		}
		else{
			szChar[0] = *p;
			szChar[1] = *(p+1);
			szChar[2] = 0;
			nCount+=2;
		}
		_ASSERT(pp-p==sizeof(TCHAR)*2 || pp-p==sizeof(TCHAR));

		//		STRING2FONTTEXTURE::iterator i = m_FontMap.find(szChar);

		WORD key = *(WORD*)szChar;

		RCHARINFOMAP::iterator i = m_CharInfoMap.find(key);

		RCHARINFO *pInfo = NULL;

		int nCurWidth;
		if(i==m_CharInfoMap.end()) {
			nCurWidth = m_pFontTexture->GetCharWidth(m_hFont, szChar);
			pInfo = new RCHARINFO;
			pInfo->nWidth = nCurWidth;
			pInfo->nFontTextureIndex = -1;
			m_CharInfoMap.insert(RCHARINFOMAP::value_type(key,pInfo));
		}else
		{
			nCurWidth=(*i->second).nWidth;
		}

		nWidth+=nCurWidth;

		p = pp;

		if(pp==NULL) break;
		if(*pp==0)	 break;
		if(nSize!=-1 && nCount>=nSize) break;
	}

	return nWidth;
}

bool DumpFontTexture()
{
	if ( D3DXSaveTextureToFile("fonttex.bmp", D3DXIFF_BMP , g_FontTexture.GetTexture(),NULL) != D3D_OK)
		return false;
	return true;
}

_NAMESPACE_REALSPACE2_END
