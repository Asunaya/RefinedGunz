//#include <afxdlgs.h>
#include "RFont.h"
#include <crtdbg.h>
#include <dxutil.h>		// Program Files\DXSDK\Samples\C++\Common\Include
#include <mbstring.h>
#include <tchar.h>
#include "mDebug.h"
#include "mprofiler.h"
#include "d3dxerr.h"

#ifdef _USE_GDIPLUS
#include "unknwn.h"
#include "gdiplus.h"
	 using namespace Gdiplus;
#pragma comment(lib,"gdiplus.lib")
#endif

_USING_NAMESPACE_REALSPACE2

//#undef _USE_GDIPLUS

DWORD g_color = 0xffff00ff;

enum RBASE_FONT{
	RBASE_FONT_GULIM = 0,
	RBASE_FONT_BATANG = 1,

	RBASE_FONT_END
};

#include "io.h"
#include "stdio.h"

static int g_base_font[RBASE_FONT_END];
static char g_UserDefineFont[256];

void SetUserDefineFont(char* FontName) {
	if(!FontName) return;
	strcpy(g_UserDefineFont,FontName);
}

bool _GetFileFontName(char* pUserDefineFont)
{
	if(pUserDefineFont==NULL) return false;

	FILE* fp = fopen("_Font", "rt");
	if (fp) {
		fgets(pUserDefineFont,256, fp);
		fclose(fp);
		return true;
	}
	return false;
}

bool _SetFileFontName(const char* pUserDefineFont)
{
	if(pUserDefineFont==NULL) return false;

	FILE* fp = fopen("_Font", "wt");
	if (fp) {
		fputs(pUserDefineFont, fp);
		fclose(fp);
		return true;
	}
	return false;
}

bool CheckFont()
{
	char FontPath[MAX_PATH];
	char FontNames[MAX_PATH+100];

	::GetWindowsDirectory(FontPath, MAX_PATH);

	strcpy(FontNames,FontPath);
	strcat(FontNames, "\\Fonts\\gulim.ttc");

	if (_access(FontNames,0) != -1)	{ g_base_font[RBASE_FONT_GULIM] = 1; }
	else							{ g_base_font[RBASE_FONT_GULIM] = 0; }

	strcpy(FontNames,FontPath);
	strcat(FontNames, "\\Fonts\\batang.ttc");

	if (_access(FontNames,0) != -1)	{ g_base_font[RBASE_FONT_BATANG] = 1; }
	else							{ g_base_font[RBASE_FONT_BATANG] = 0; }

//	strcpy(FontNames,FontPath);
//	strcat(FontNames, "\\Fonts\\System.ttc");
//	if (_access(FontNames,0) != -1)	{ g_font[RBASE_FONT_BATANG] = 1; }
//	else							{ g_font[RBASE_FONT_BATANG] = 0; }

	if(g_base_font[RBASE_FONT_GULIM]==0 && g_base_font[RBASE_FONT_BATANG]==0) {//둘다없으면..

		if( _access("_Font",0) != -1) { // 이미 기록되어 있다면..
			_GetFileFontName( g_UserDefineFont );
		}
		else {

			int hr = IDOK;

//			hr = ::MessageBox(NULL,"귀하의 컴퓨터에는 건즈가 사용하는 (굴림,돋움) 폰트가 없는 것 같습니다.\n 다른 폰트를 선택 하시겠습니까?","알림",MB_OKCANCEL);
			hr = ::MessageBox(NULL,"귀하의 컴퓨터에는 건즈가 사용하는 (굴림,돋움) 폰트가 없는 것 같습니다.\n 계속 진행 하시겠습니까?","알림",MB_OKCANCEL);

			if(hr==IDOK) {
/*			
				CFontDialog dlg;
				if(dlg.DoModal()==IDOK) {
					CString facename = dlg.GetFaceName();
					lstrcpy((LPSTR)g_UserDefineFont,(LPSTR)facename.operator const char*());

					hr = ::MessageBox(NULL,"선택하신 폰트를 저장 하시겠습니까?","알림",MB_OKCANCEL);

					if(hr==IDOK)
						_SetFileFontName(g_UserDefineFont);
				}
*/
				return true;
			}
			else {
				return false;
			}
		}
	}
	return true;
}


static int CALLBACK FontEnumProc(const LOGFONT* pLogfont, const TEXTMETRIC* pTextmetric, DWORD type, LPARAM lParam)
{
	if (lParam != NULL)	{
		*(BOOL*)lParam = TRUE;
	}
	return 0;
}

BOOL isFontExist(HDC hdc,const char* szFontName) {

	BOOL bExist;
	LOGFONT lf;

	memset(&lf, 0, sizeof(lf));

	strcpy(lf.lfFaceName, szFontName);
//	strcpy(lf.lfFaceName, "\0");
	lf.lfCharSet = DEFAULT_CHARSET;
	bExist = FALSE;

	::EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC )FontEnumProc, (LPARAM)&bExist, 0);

	return bExist;
}


void BlitRect(BYTE* pDest, int _x,int _y,int w,int h,int nTextureWidth, int nTextureHeight, DWORD* pBitmapBits, INT Pitch, int nSamplingMultiplier)
{
	if(pDest==NULL)
		return;

	DWORD* pDestTemp = NULL;

	int ns = nSamplingMultiplier;
	int by=0;
	int bx=0;

	for(int y=_y; y<h; y++) {
		pDestTemp = (DWORD*)(pDest+(y*Pitch));
		bx = 0;
		for(int x=_x; x<w; x++) {
//			*(pDestTemp+x) = pBitmapBits[(nTextureWidth*by) + (bx)];
//			*(pDestTemp+x) = g_color;//pBitmapBits[nTextureWidth*by + bx];

			if(pBitmapBits[(nTextureWidth*by) + (bx)] & 0x00ffffff)//색이 값이 있다면~
				pBitmapBits[(nTextureWidth*by) + (bx)] |= 0xff000000;

			*(pDestTemp+x) = pBitmapBits[(nTextureWidth*by) + (bx)];
/*
			if(pBitmapBits[(nTextureWidth*by) + (bx)])
				*(pDestTemp+x) = 0xffff0000;
			else 
				*(pDestTemp+x) = 0xff000000;
*/
//			bx+=ns;
			bx++;
		}
//		by+=ns;
		by++;
	}
/*
	for(int y=_y; y<h; y++) {
		pDestTemp = (DWORD*)(pDest+(y*Pitch));
		bx = 0;
		for(int x=_x; x<w; x++) {
			*(pDestTemp+x) = pBitmapBits[(nTextureWidth*by) + (bx)];
			bx++;
		}
		by++;
	}
*/
}

bool RBaseFontTexture::Create(LPDIRECT3DDEVICE8 dev,int w,int h) {

	HRESULT hr;

	int n_w = w;
	int n_h = h;

	D3DCAPS8 d3dCaps;
	dev->GetDeviceCaps( &d3dCaps );

	if( n_w > (int)d3dCaps.MaxTextureWidth ) { n_w = d3dCaps.MaxTextureWidth; }
	if( n_h > (int)d3dCaps.MaxTextureHeight ) { n_h = d3dCaps.MaxTextureHeight; }

//	n_w = 256;
//	n_h = 256;

	hr = dev->CreateTexture(n_w, n_h, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_pTexture);

	if( FAILED(hr) ) {

		n_w /= 2;
		n_h /= 2;

		hr = dev->CreateTexture(n_w, n_h, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_pTexture);

		if(FAILED(hr)) {//256/256 도 생성실패~

			char buffer[256];
			D3DXGetErrorString(hr, buffer, 256 );

			mlog("RBaseFontTexture::Create 생성실패 %d ( %s )\n",hr,buffer);

			return false;
		}
	}
	
	m_nTextureWidth = n_w;
	m_nTextureHeight = n_h;

	// 16 size

	m_info[font_type_16].m_nMinIndexSize = 0;
	m_info[font_type_16].m_nMaxIndexSize = (int)((w/16)*(h/16) * 1.0f);//텍스쳐사용비율..
//	m_info[font_type_16].m_nMaxIndexSize = (int)((w/16)*(h/16) * 0.25f);//텍스쳐사용비율..
	m_info[font_type_16].m_fXLen = (16.f/w);// m_fXLen = 1.f/(w / 16.f);
	m_info[font_type_16].m_fYLen = (16.f/h);// m_fYLen = 1.f/(h / 16.f);
	m_info[font_type_16].m_nSize = 16;
	m_info[font_type_16].m_nLastIndex = m_info[font_type_16].m_nMinIndexSize;

	// 32 size

	m_info[font_type_32].m_nMinIndexSize = (int)((w/32)*(h/32) * 0.0f);//텍스쳐사용비율..
//	m_info[font_type_32].m_nMinIndexSize = (int)((w/32)*(h/32) * 0.25f);//텍스쳐사용비율..
	m_info[font_type_32].m_nMaxIndexSize = (int)((w/32)*(h/32));

	m_info[font_type_32].m_fXLen = (32.f/w);// m_fXLen = 1.f/(w / 32.f);
	m_info[font_type_32].m_fYLen = (32.f/h);// m_fYLen = 1.f/(h / 32.f);
	m_info[font_type_32].m_nSize = 32;
	m_info[font_type_32].m_nLastIndex = m_info[font_type_32].m_nMinIndexSize;

	mlog("RBaseFontTexture::Create %d %d \n",m_nTextureWidth,m_nTextureHeight);

	return true;
}

void RBaseFontTexture::Clear() {
	if(m_pTexture) {
		m_pTexture->Release();
		m_pTexture = NULL;
	}
}

bool RBaseFontTexture::GetInfo(int type) {

	if(type < 0 || type >= font_type_end) {
		m_pInfo = NULL;
		return false;
	}

	m_pInfo = &m_info[type];

	return true;
}

void RBaseFontTexture::GetIndexPos(int type,int index,int& x,int& y) 
{
	if(m_pTexture==NULL) {
		x = 0;
		y = 0;
		return;
	}

	if(!GetInfo(type)) return;

	if(index==0) {
		y = 0;
		x = 0;
		return;
	}

	DWORD dev = 1;

	if(m_pInfo->m_nSize) {
		dev = m_nTextureWidth / m_pInfo->m_nSize;// 512 / 16,32
	} 

	if( index ) {
		y = index / dev;
		x = index % dev;
	}
}

BYTE* RBaseFontTexture::GetPosPtr(int type,int x,int y) 
{
	if(m_pTexture==NULL)
		return NULL;

	if(!GetInfo(type)) return NULL;

	x *= m_pInfo->m_nSize;// x *= 16;
	y *= m_pInfo->m_nSize;// y *= 16;

	BYTE* pDest = (BYTE*)m_d3dlr.pBits;
	return &pDest[ y * m_d3dlr.Pitch + x ];
}

BYTE* RBaseFontTexture::GetIndexPtr(int type,int index) {

	if(m_pTexture==NULL)
		return NULL;

	if(!GetInfo(type)) return NULL;

	if(index >= m_pInfo->m_nMaxIndexSize)
		return NULL;

	int x=0,y=0;

	GetIndexPos(type,index,x,y);

	int px = x * m_pInfo->m_nSize;
	int py = y * m_pInfo->m_nSize;

	px -= m_pInfo->m_nSize * x;
	py -= m_pInfo->m_nSize * y;

	BYTE* pDest = (BYTE*)m_d3dlr.pBits;
	return &pDest[ py * m_d3dlr.Pitch + px ];
}

void RBaseFontTexture::GetIndexUV(int type,int index,int w,int h,float*uv) {

	if(m_pTexture==NULL) {

		uv[0] = 0.f;
		uv[1] = 0.f;
		uv[2] = 1.f;
		uv[3] = 0.f;
		uv[4] = 1.f;
		uv[5] = 1.f;
		uv[6] = 0.f;
		uv[7] = 1.f;

		return;
	}

	if(!GetInfo(type)) 
		return;

	if(index >= m_pInfo->m_nMaxIndexSize)
		return;

	int x=0,y=0;

	GetIndexPos(type,index,x,y);

	x*=m_pInfo->m_nSize;
	y*=m_pInfo->m_nSize;

	// 0,0
	// 1,0
	// 1,1
	// 0,1

	float min_u = x/(float)m_nTextureWidth;
	float min_v = y/(float)m_nTextureHeight;

	float max_u = (x+w)/(float)m_nTextureWidth;
	float max_v = (y+h)/(float)m_nTextureHeight;

	uv[0] = min_u;
	uv[1] = min_v;

	uv[2] = max_u;
	uv[3] = min_v;

	uv[4] = max_u;
	uv[5] = max_v;

	uv[6] = min_u;
	uv[7] = max_v;

}

int RBaseFontTexture::MakeTexture(int type) {

	if(!GetInfo(type)) return -1;

	// 모두 사용한 경우 가장 오래전에 사용된 텍스쳐의 인덱스를 준다... -> 개선하기..
	// 사용된 시간을 기록했다가 가장점수낮은것 지우기..지금은 그냥 앞에 것부터 지운다~

	m_pInfo->m_nLastIndex++;

	if(m_pInfo->m_nLastIndex > m_pInfo->m_nMaxIndexSize) {
		m_pInfo->m_nLastIndex = m_pInfo->m_nMinIndexSize+1;
		m_pInfo->m_nTableIndex++;
	}

	return m_pInfo->m_nLastIndex-1;
}

int RBaseFontTexture::MakeTexture(int type,DWORD* pBitmapBits,int w,int h,int nSamplingMultiplier,int debug_cnt)
{
	if(m_pTexture==NULL) 
		return -1;

	int index = MakeTexture(type);

	if(index==-1)
		return -1;

	if(!GetInfo(type)) return -1;

	int __y = 0;
	int __x = 0;

	GetIndexPos(type,index,__x,__y);

	__x *= m_pInfo->m_nSize;//16
	__y *= m_pInfo->m_nSize;//16

	// 부두,intel 계열~ 기타등등~ lock 이 실패하는경우...

	HRESULT hr;

	hr = Lock();

	if(hr!=S_OK) {
		mlog("RBaseFontTexture::MakeTexture : texture lock 실패 \n");
		return -2;
	}

	BlitRect(GetIndexPtr(type,index),__x,__y,__x+w,__y+h,w,h,pBitmapBits, m_d3dlr.Pitch,nSamplingMultiplier);

	hr = UnLock();

	if(hr!=S_OK) {
		mlog("RBaseFontTexture::MakeTexture : texture unlock 실패 \n");
	}

//	RGetDevice()->EndScene();

	////////////////////////////////////////////////////////////////////////////
/*
	static char _temp_tex_name[256];

	sprintf(_temp_tex_name,"font.bmp");
	hr = D3DXSaveTextureToFile(_temp_tex_name,D3DXIFF_BMP,GetTexture(),NULL);
*/
	////////////////////////////////////////////////////////////////////////////

	return index;
}

_NAMESPACE_REALSPACE2_BEGIN

struct FONT2DVERTEX { 
	D3DXVECTOR4 p;   
	DWORD color;     
	FLOAT tu, tv; 
};

#define D3DFVF_FONT2DVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)

#define MAX_FONT_BUFFER 4000

static	int				g_font_cnt=0;
static	FONT2DVERTEX	g_font_vertex_buffer[4*MAX_FONT_BUFFER];//그림자까지 200자정도
static	WORD			g_font_index_buffer[6*MAX_FONT_BUFFER];


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

DWORD RFontTexture::m_dwStateBlock = NULL;
DWORD RFont::m_dwStateBlock = NULL;

RFontTexture::RFontTexture(void)
{
	m_dwStateBlock = NULL;
	m_pTexture = NULL;
	m_pFont = NULL;
	m_nIndex = 0;
	m_nFontType = font_type_16;
	m_nTableIndex = 0;

	m_nRealTextWidth = 0;
	m_nRealTextHeight = 0;
}

RFontTexture::~RFontTexture(void)
{
	Destroy();
}

// 정수배 샘플링만 가능
void Sampling(BYTE* pDstRow, int nTextureWidth, int nTextureHeight, DWORD* pBitmapBits, INT Pitch, int nSamplingMultiplier)
{
	DWORD* pDst32;
	BYTE bAlpha; // 4-bit measure of pixel intensity

	int nSamplingDivide = nSamplingMultiplier*nSamplingMultiplier;
	for(int y=0; y<nTextureHeight; y++) {
		pDst32 = (DWORD*)pDstRow;
		for(int x=0; x<nTextureWidth; x++) {
			float fValue = 0;
			for(int sy=0; sy<nSamplingMultiplier; sy++) {
				for(int sx=0; sx<nSamplingMultiplier; sx++) {
					bAlpha = (BYTE)((pBitmapBits[(nTextureWidth*nSamplingMultiplier)*(y*nSamplingMultiplier+sy) + x*nSamplingMultiplier+sx] & 0xff) >> 4);
					if(bAlpha>0){
						fValue++;
						//*pDst16++ = 0xffff;
						//*pDst16++ = (bAlpha << 12) | 0x0fff;
					}
				}
			}
			fValue /= (float)nSamplingDivide;
			*pDst32++ = (BYTE(0xFF*fValue)<<24) | 0x00FFFFFF;
		}
		pDstRow += Pitch;
	}
}

void Blit(BYTE* pDstRow, int nTextureWidth, int nTextureHeight, DWORD* pBitmapBits, INT Pitch, int nSamplingMultiplier)
{
	DWORD* pDst32;

	for(int y=0; y<nTextureHeight; y++) {
		pDst32 = (DWORD*)pDstRow;
		for(int x=0; x<nTextureWidth; x++) {
			*pDst32++ = pBitmapBits[(nTextureWidth*nSamplingMultiplier)*(y*nSamplingMultiplier) + x*nSamplingMultiplier] 
			;//| 0xffffff;	// 택스트를 Alpha만 쓰겠다.
		}
		pDstRow += Pitch;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

bool RFontTexture::Create(RFont* pFont, HFONT hFont, const TCHAR* szText, int nOutlineStyle, DWORD nColorArg1, DWORD nColorArg2, int nSamplingMultiplier)
{
	if(!pFont) 
		return false;

	m_pFont = pFont;

	LPDIRECT3DDEVICE8 pd3dDevice = pFont->m_pd3dDevice;

	m_pd3dDevice = pd3dDevice;

//	m_pd3dDevice->EndScene();//<-----------

	HDC hDC = CreateCompatibleDC(NULL);

	HFONT hPrevFont = (HFONT)SelectObject(hDC, hFont);

	SIZE size;
	GetTextExtentPoint32(hDC, szText, _tcslen(szText), &size);

	if(m_pFont->m_isBigFont) {
		m_nTextWidth = size.cx/nSamplingMultiplier;
		m_nTextHeight = size.cy/nSamplingMultiplier;

		m_nTextureWidth = Floorer2PowerSize(m_nTextWidth);
		m_nTextureHeight = Floorer2PowerSize(m_nTextHeight);

	}
	else {
		m_nTextWidth = size.cx;
		m_nTextHeight = size.cy;

		m_nTextureWidth = Floorer2PowerSize(m_nTextWidth);
		m_nTextureHeight = Floorer2PowerSize(m_nTextHeight);

		int tex_max = max(m_nTextureWidth,m_nTextureHeight);

//		m_nTextureWidth = tex_max;
//		m_nTextureHeight = tex_max;

		m_nRealTextWidth = m_nTextWidth;
		m_nRealTextHeight = m_nTextHeight;

		//	좌우 폭이 같아야 하는가?

		if( tex_max > 32 ) {

			m_nTextWidth = 32;
			m_nTextHeight = 32;
			m_nTextureWidth = 32;
			m_nTextureHeight = 32;

			m_nFontType = font_type_32;

			mlog("RFontTexture::Create 막힌곳으로 들어왔다~ -.- 0001 \n");
		}
		else if( tex_max > 16 ) {

			m_nFontType = font_type_32;

			mlog("RFontTexture::Create 막힌곳으로 들어왔다~ -.- 0002 \n");
		} else {

			m_nFontType = font_type_16;
		}

	}

	if(m_pFont->m_isBigFont) {

		HRESULT hr;
		//	hr = m_pd3dDevice->CreateTexture(m_nTextureWidth, m_nTextureHeight, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &m_pTexture);
		hr = m_pd3dDevice->CreateTexture(m_nTextureWidth, m_nTextureHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_pTexture);
		if(FAILED(hr)) {
			char buffer[256];
			D3DXGetErrorString(hr, buffer, 256 );
			mlog("Fail to create texture for Font... %s \n",buffer);

			SelectObject(hDC, hPrevFont);
			DeleteDC(hDC);
//			m_pd3dDevice->BeginScene();
			return false;
		}
	}

	////////////////////////////////////////////////--------------------------------------------------------

	// Bitmap Creation
	DWORD* pBitmapBits;
	BITMAPINFO bmi;
	ZeroMemory(&bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
	bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth       =  (int)m_nTextureWidth*nSamplingMultiplier;
	bmi.bmiHeader.biHeight      = -(int)m_nTextureHeight*nSamplingMultiplier;
	bmi.bmiHeader.biPlanes      = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount    = 32;

	HBITMAP hbmBitmap = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (VOID**)&pBitmapBits, NULL, 0);

//	if((rand()%2)==1) {	pBitmapBits = NULL;	} //test

	if( pBitmapBits == NULL )
	{
//		mlog("Fail to create Bitmap for Font... code = %d str = %s ,width = %d height = %d \n",
//			GetLastError(),szText,bmi.bmiHeader.biWidth,bmi.bmiHeader.biHeight);
		SAFE_RELEASE( m_pTexture );

//		m_pd3dDevice->BeginScene();
		SelectObject(hDC, hPrevFont);
		DeleteDC(hDC);

		return false;
	}

	SetMapMode(hDC, MM_TEXT);

	HBITMAP hPrevBitmap = (HBITMAP)SelectObject(hDC, hbmBitmap);

#ifdef _USE_GDIPLUS
	// GDI+ Font
	WCHAR wstrText[256];
	int nTextLen = strlen(szText)+1;
	MultiByteToWideChar(CP_ACP, 0, szText, -1, wstrText, nTextLen-1);
	wstrText[nTextLen-1] = 0;

	Graphics graphics(hDC);
	Font font(hDC, hFont);

	//	graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);
	graphics.Clear(Color(0,0,0,0));

	const StringFormat* pTypoFormat = StringFormat::GenericTypographic();

	if (nOutlineStyle == 0) {
		SolidBrush  solidBrush(Color(255, 255, 255, 255));
		graphics.DrawString(wstrText, -1, &font, PointF(0.0f, 0.0f), pTypoFormat, &solidBrush);
	} else if (nOutlineStyle == 1) {	// 아웃라인이 그려진다
		GraphicsPath path;
		FontFamily fontFamily;
		font.GetFamily(&fontFamily);

		TEXTMETRIC tm;
		GetTextMetrics(hDC, &tm);

		path.AddString(	wstrText, -1, &fontFamily, FontStyleBold, (float)tm.tmHeight, PointF(0.0f, 0.0f), pTypoFormat);

		graphics.SetSmoothingMode(SmoothingModeAntiAlias);

		Pen pen(Color(nColorArg2), 4.5f);
		graphics.DrawPath(&pen, &path);

		SolidBrush brush(Color((ARGB)nColorArg1));
		graphics.FillPath((Brush*)&brush, &path); 
	} else if (nOutlineStyle == 2) {	// 그림자만 포함된다. 좌측의 한픽셀 여백이 있으므로 왼쪽으로 한픽셀. 자간은 같다
		SolidBrush  solidBrush2(Color((ARGB)nColorArg2));
		graphics.DrawString(wstrText, -1, &font, PointF(0.0f, 1.0f), pTypoFormat, &solidBrush2);
		
		SolidBrush  solidBrush1(Color((ARGB)nColorArg1));
		graphics.DrawString(wstrText, -1, &font, PointF(-1.0f, 0.0f), pTypoFormat, &solidBrush1);
	}

#else	

	SetTextColor(hDC, RGB(255,255,255));
	SetBkColor(hDC, 0x00000000);
	SetTextAlign(hDC, TA_TOP);
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, NULL, szText, _tcslen(szText), NULL);

#endif


	bool error = false;

	if(m_pFont->m_isBigFont) {

		D3DLOCKED_RECT d3dlr;

		HRESULT hr;

		hr = m_pTexture->LockRect(0, &d3dlr, 0, 0);

		if(hr != S_OK) {
			mlog("RFontTexture::Create m_pTexture->LockRect hr = %d  \n",hr);
			error = true;
			goto FONT_ERROR;
		}

	//	Sampling((BYTE*)d3dlr.pBits, m_nTextureWidth, m_nTextureHeight, pBitmapBits, d3dlr.Pitch, nSamplingMultiplier);
		Blit((BYTE*)d3dlr.pBits, m_nTextureWidth, m_nTextureHeight, pBitmapBits, d3dlr.Pitch, nSamplingMultiplier);

		hr = m_pTexture->UnlockRect(0);

		if(hr != S_OK) {
			mlog("RFontTexture::Create m_pTexture->UnLockRect hr = %d  \n",hr);
			error = true;
			goto FONT_ERROR;
		}

	} else {

		m_nIndex = pFont->m_pTex->MakeTexture(m_nFontType,pBitmapBits,m_nTextureWidth,m_nTextureHeight,nSamplingMultiplier,pFont->m_nDebugFontCount);

		if(m_nIndex < 0) {
			if(m_nIndex == -2) {
				mlog("RFontTexture::Create pFont->m_pTex->MakeTexture 실패~\n");
			}
			error = true;
			goto FONT_ERROR;
		}

		m_nTableIndex = pFont->m_pTex->m_info[m_nFontType].m_nTableIndex;//꼭 보관해야함~
	}

FONT_ERROR :

	SelectObject(hDC, hPrevBitmap);
	SelectObject(hDC, hPrevFont);
	DeleteObject(hbmBitmap);
	DeleteDC(hDC);

/*
	static char str_temp[1024];
	sprintf( str_temp,"font 생성 %s %d %d \n", szText,m_nTextureWidth,m_nTextureHeight);
	mlog( str_temp );
*/
//	m_pd3dDevice->BeginScene();//<-----------

	if(error)
		return false;

//	mlog("RFontTexture::Create pFont->m_pTex->MakeTexture %s 성공~\n",szText);

	return true;
}

void RFontTexture::Destroy(void)
{
	if(m_dwStateBlock!=NULL){
		m_pd3dDevice->DeleteStateBlock(m_dwStateBlock);
		m_dwStateBlock = NULL;
	}

	SAFE_RELEASE(m_pTexture);
	m_pd3dDevice = NULL;
}


void RFontTexture::BeginState(FTCOLORMODE nMode)
{
/*
	if( m_dwStateBlock == 0xFFFFFFFF ) // Invalid Handle
	{
		m_dwStateBlock	= NULL;
	}

	if(m_dwStateBlock==NULL) { 

		m_pd3dDevice->BeginStateBlock();
		DWORD dwNumPass;
		m_pd3dDevice->ValidateDevice(&dwNumPass);
*/
		m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
		m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA );
		m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF,         0x08 );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC,  D3DCMP_GREATEREQUAL );

		const bool bFiltering = false;

		if(bFiltering==true){
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
		}
		else{
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT );
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT );
		}

		m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,   FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE,   FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_FILLMODE,   D3DFILL_SOLID );
		m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CCW );
		m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_CLIPPING,         TRUE );
		m_pd3dDevice->SetRenderState( D3DRS_EDGEANTIALIAS,    FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_CLIPPLANEENABLE,  FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_VERTEXBLEND,      FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE,        FALSE );

		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
//		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_NONE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
		m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
		m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
/*
		if(m_pd3dDevice->EndStateBlock(&m_dwStateBlock)!=D3D_OK) {
			m_dwStateBlock = NULL;
		}
		m_pd3dDevice->ApplyStateBlock(m_dwStateBlock);

	}
	else {
//		m_pd3dDevice->ApplyStateBlock(m_dwStateBlock);
	}
*/
}
void RFontTexture::EndState(void)
{

}

// 남아있는 버퍼의 폰트를 그린다..
// 마지막 flip 하기 전에도 호출해준다..
void RFont_Render()
{
	if(g_font_cnt==0)
		return;
	// 폰트 스테이지 상태보장~

	if(RGetDevice()) {
//		RGetDevice()->SetTexture(0,GetBaseFontTexture()->GetTexture());
		RGetDevice()->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,g_font_cnt*6,g_font_cnt*2,g_font_index_buffer,D3DFMT_INDEX16,g_font_vertex_buffer,sizeof(FONT2DVERTEX));
		g_font_cnt = 0;
	}
}

void RFontTexture::DrawText(FTCOLORMODE nMode, float x, float y, DWORD dwColor,float fScale)
{
	if(m_pd3dDevice==NULL) return;

	if(fScale > 1.f) {
		mlog("scale 값이 %d : 1을 초과...\n",fScale);//이상유저의 로그를 보기위해..
		fScale = 1.f;
	}

	// uv 가 커지는 것도 조사..
	// size 자체가 20 픽셀을 넘어가는 경우도 조사..

	if (nMode == FTCOLORMODE_FIXED)	// DIFFUSE값이 1이므로 택스춰 고유의 색이 나온다.
		dwColor = 0xFFFFFFFF;

#define RFONT_VERTEXCOUNT	4
	static FONT2DVERTEX pVertices[RFONT_VERTEXCOUNT];

	////////////////////////////////////////////////--------------------------------------------------------

	int nsampling = m_pFont->GetSamplingMultiplier();
	
	static float uv[8];

	float w = (float)m_nRealTextWidth;
	float h = (float)m_nRealTextHeight;

	if(m_pFont->m_isBigFont) {
		m_pd3dDevice->SetVertexShader(D3DFVF_FONT2DVERTEX);
		m_pd3dDevice->SetPixelShader(NULL);
	//	m_pd3dDevice->SetStreamSource(0, NULL, 0);
		m_pd3dDevice->SetTexture(0, m_pTexture);

		w = (float)m_nTextureWidth;
		h = (float)m_nTextureHeight;
	}
	else {

//		m_pFont->m_pTex->GetIndexUV( m_nFontType , m_nIndex , m_nTextureWidth , m_nTextureHeight ,uv );
		m_pFont->m_pTex->GetIndexUV( m_nFontType , m_nIndex , m_nTextWidth , m_nTextHeight ,uv );
	}

//	dwColor = 0xffff0000;

	float add = 0.5f;

	pVertices[0].p.x = float(x)-add;
	pVertices[0].p.y = float(y)-add;
	pVertices[0].p.z = 0;
	pVertices[0].p.w = 1;

	if(m_pFont->m_isBigFont) {
		pVertices[0].tu = 0;
		pVertices[0].tv = 0;
	} else {
		pVertices[0].tu = uv[0];
		pVertices[0].tv = uv[1];
	}

	pVertices[0].color = dwColor;

	pVertices[1].p.x = float(x+(w)*fScale)-add;
//	pVertices[1].p.x = float(x+(m_nTextureWidth)*fScale)-0.5f;
	pVertices[1].p.y = float(y)-add;
	pVertices[1].p.z = 0;
	pVertices[1].p.w = 1;

	if(m_pFont->m_isBigFont) {
		pVertices[1].tu = 1;
		pVertices[1].tv = 0;
	}else {
		pVertices[1].tu = uv[2];
		pVertices[1].tv = uv[3];
	}

	pVertices[1].color = dwColor;

	pVertices[2].p.x = float(x+(w)*fScale)-add;
	pVertices[2].p.y = float(y+(h)*fScale)-add;
//	pVertices[2].p.x = float(x+(m_nTextureWidth)*fScale)-0.5f;
//	pVertices[2].p.y = float(y+(m_nTextureHeight)*fScale)-0.5f;
	pVertices[2].p.z = 0;
	pVertices[2].p.w = 1;

	if(m_pFont->m_isBigFont) {
		pVertices[2].tu = 1;
		pVertices[2].tv = 1;
	} else {
		pVertices[2].tu = uv[4];
		pVertices[2].tv = uv[5];
	}

	pVertices[2].color = dwColor;

	pVertices[3].p.x = float(x)-add;
	pVertices[3].p.y = float(y+(h)*fScale)-add;
//	pVertices[3].p.y = float(y+(m_nTextureHeight)*fScale)-0.5f;
	pVertices[3].p.z = 0;
	pVertices[3].p.w = 1;

	if(m_pFont->m_isBigFont) {
		pVertices[3].tu = 0;
		pVertices[3].tv = 1;
	} else {
		pVertices[3].tu = uv[6];
		pVertices[3].tv = uv[7];
	}

	pVertices[3].color = dwColor;

	////////////////////////////////////////////////--------------------------------------------------------

	if(m_pFont->m_isBigFont) {//아주 큰 폰트라면 예전방식~

		BeginState(nMode);
		m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, pVertices, sizeof(FONT2DVERTEX));
		EndState();

	} else {

		int add = 0; 

		add = g_font_cnt*4;

		WORD index[6] = {
			3,0,2,0,1,2
		};

		for(int i=0;i<6;i++)
			index[i] += add;

		memcpy(&g_font_vertex_buffer[g_font_cnt*4],pVertices,sizeof(FONT2DVERTEX)*4);
		memcpy(&g_font_index_buffer[g_font_cnt*6],index,sizeof(WORD)*6);

		g_font_cnt++;

		if(g_font_cnt > MAX_FONT_BUFFER-2) {// 버퍼를 넘길경우 -> 한번에4000글자 이상을 찍는경우..
			// 보통 font begin ~ end 사이에 장치의 자유도를 주기위해~ State 를 그리는 순간에 결정한다...
			m_pFont->BeginFontState();
			RFont_Render();
			m_pFont->EndState();
		}
	}
}

RFont::RFont(void)
{
	m_hFont = NULL;
	m_pd3dDevice = NULL;
	m_nSamplingMultiplier = 1;
	m_pTex = NULL;

	m_nDebugFontCount = 0;

	m_isBigFont = false;

	m_nCacheSize = 1;
	m_nHeight = 1;
	m_ColorArg1 =  0;
	m_ColorArg2 = 0;

	m_bFontBegin = false;
}

RFont::~RFont(void)
{
	Destroy();
}

void RFont::BeginFontState()
{
//	m_pd3dDevice->EndScene();

	BeginState();

	m_pd3dDevice->SetVertexShader(D3DFVF_FONT2DVERTEX);
	m_pd3dDevice->SetPixelShader(NULL);
//	m_pd3dDevice->SetStreamSource(0, NULL, 0);

	////////////////////////////////////////////////

//	LPDIRECT3DTEXTURE8 _pTex = NULL;
//	IDirect3DBaseTexture8* _ppTex = NULL;

//	m_pd3dDevice->GetTexture( 0,&_ppTex );				// 폰트들은 모아 그릴 가능성이 크다~ 비용계산~
//	if( _ppTex != m_pTex->GetTexture() )
	m_pd3dDevice->SetTexture(0,m_pTex->GetTexture());	// 모두 같은 texture..
}

void RFont::EndFontState(void)
{

}

void RFont::BeginState()
{
/*
	if( m_dwStateBlock == 0xFFFFFFFF ) // Invalid Handle
	{
		m_dwStateBlock	= NULL;
	}

	if(m_dwStateBlock==NULL) {

		m_pd3dDevice->BeginStateBlock();
		DWORD dwNumPass;
		m_pd3dDevice->ValidateDevice(&dwNumPass);
*/
		m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
		m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA );
		m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF,         0x08 );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC,  D3DCMP_GREATEREQUAL );

		const bool bFiltering = false;
		if(bFiltering==true){
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
		}
		else{
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT );
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT );
		}

		m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,   FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE,   FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_FILLMODE,   D3DFILL_SOLID );
		m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CCW );
		m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_CLIPPING,         TRUE );
		m_pd3dDevice->SetRenderState( D3DRS_EDGEANTIALIAS,    FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_CLIPPLANEENABLE,  FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_VERTEXBLEND,      FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE,        FALSE );

		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_NONE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
		m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
		m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
/*
		if(m_pd3dDevice->EndStateBlock(&m_dwStateBlock)!=D3D_OK) {
			m_dwStateBlock = NULL;
		}

		m_pd3dDevice->ApplyStateBlock(m_dwStateBlock);

	}
	else{
		m_pd3dDevice->ApplyStateBlock(m_dwStateBlock);
	}
*/
}

void RFont::EndState()
{
}

static int g_nFontCnt = 0;

extern HWND g_hWnd;

bool RFont::Create(LPDIRECT3DDEVICE8 pd3dDevice, const TCHAR* szFontName, int nHeight, bool bBold, bool bItalic, int nOutlineStyle, int nCacheSize, int nSamplingMultiplier, DWORD nColorArg1, DWORD nColorArg2)
{
	m_nOutlineStyle = nOutlineStyle;
	m_nSamplingMultiplier = nSamplingMultiplier;	// 한방향 샘플링 횟수, 2이면 2*2크기를 샘플링한다.
	HDC hDC = GetDC(g_hWnd);
	SetMapMode(hDC, MM_TEXT);
	nHeight = MulDiv(nHeight, GetDeviceCaps(hDC, LOGPIXELSY), 72);

	static char _FontName[256];

	// 느리다면 폰트목록을 만든다..
/*
	BOOL bExist = isFontExist(hDC,"알지");

	bExist = isFontExist(hDC,"바탕");
	bExist = isFontExist(hDC,"굴림");
	bExist = isFontExist(hDC,"System");
	bExist = isFontExist(hDC,"궁서");

	BOOL bExist = isFontExist( hDC , (const char*)szFontName );
*/
	// 초기에만 불린다..

	int nExist = 0;

	// 0 굴림+돋움
	// 1 바탕+궁서

	// 다 있다..

//	g_base_font[RBASE_FONT_GULIM] = 1;
//	g_base_font[RBASE_FONT_BATANG] = 1;

	if( (g_base_font[RBASE_FONT_GULIM]==0) && (g_base_font[RBASE_FONT_BATANG]==0) ) { // 다 없으면..
		nExist = 0;
		// 유저가 폰트 등록으로 선택한 폰트..
//		strcpy(_FontName,g_UserDefineFont);
//		nHeight = 12;
		strcpy(_FontName,szFontName);// font 선택 dlg 필요..우선은 예전 코드로..
		mlog("Create Font Failure 굴림,바탕 폰트 모두 없음\n");
	}	
	else if((g_base_font[RBASE_FONT_GULIM]) && (g_base_font[RBASE_FONT_BATANG]==0)) {
		nExist = 1;
		strcpy(_FontName,"굴림");
		mlog("Create Font Failure 바탕 폰트 없음\n" );
	} 
	else if((g_base_font[RBASE_FONT_GULIM]==0) && (g_base_font[RBASE_FONT_BATANG])) {
		nExist = 2;
		strcpy(_FontName,"바탕");
		mlog("Create Font Failure 굴림 폰트 없음\n" );
	}
	else { // 일반적인 유저 모두 있게 된다..
		nExist = 3;
		strcpy(_FontName,szFontName);
	}

	m_hFont = CreateFont(-nHeight*m_nSamplingMultiplier, 0, 0, 0, bBold==true?FW_BOLD:FW_NORMAL, bItalic==true?TRUE:FALSE, 
		FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH , _FontName);

	if(m_hFont==NULL)
		return false;
/*
	if( nExist ) { // 굴림 돋움 바탕 궁서 중 폰트가 있다면~
	
		m_hFont = CreateFont(-nHeight*m_nSamplingMultiplier, 0, 0, 0, bBold==true?FW_BOLD:FW_NORMAL, bItalic==true?TRUE:FALSE, 
			FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH , _FontName);

	}
	else { // 다없다면~

		// 시스템 기본 글씨체로 만들어보고 그래도 안된다면 할수 없다..
		// 시스템 등록 폰트를 조사하는 것과 폰트를 열거하는 건 폰트 파일

//		NONCLIENTMETRICS info;
//		memset(&info,0,sizeof(info));
//		info.cbSize = sizeof(info);

//		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);


		if(isFontExist( hDC , info.lfMessageFont.lfFaceName )) {

			m_hFont = CreateFont(-nHeight*m_nSamplingMultiplier, 0, 0, 0, bBold==true?FW_BOLD:FW_NORMAL, bItalic==true?TRUE:FALSE, 
				FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,	DEFAULT_QUALITY , 
				DEFAULT_PITCH , info.lfMessageFont.lfFaceName );
		}
		else { // 그럴리없다~ 윈도우에 등록된폰트가 없다면 컴도 아니다..
//		if(m_hFont==NULL) { 
		// 유저가 폰트 등록으로 선택한 폰트..

			m_hFont = CreateFont(-nHeight*m_nSamplingMultiplier, 0, 0, 0, bBold==true?FW_BOLD:FW_NORMAL, bItalic==true?TRUE:FALSE, 
				FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,	DEFAULT_QUALITY , 
				DEFAULT_PITCH , g_UserDefineFontName );

			if(m_hFont==NULL) {
				mlog("Create Font Failure User Define Font \n");
				return false;
			}
//		}
	}
*/
	ReleaseDC(g_hWnd, hDC);

	if(nCacheSize<=0) m_nCacheSize = 10000/nHeight;
	else m_nCacheSize = nCacheSize;

	m_pd3dDevice = pd3dDevice;

	m_nHeight = nHeight;

	m_ColorArg1 = nColorArg1;
	m_ColorArg2 = nColorArg2;

	m_pTex = GetBaseFontTexture();

	m_nDebugFontCount = g_nFontCnt++;

//	HDC hDC = CreateCompatibleDC(NULL);
//	HFONT hPrevFont = (HFONT)SelectObject(hDC, hFont);
//	SIZE size;
//	GetTextExtentPoint32(hDC, szText, _tcslen(szText), &size);
//	int tex_max = max(m_nTextureWidth,m_nTextureHeight);

	////////////////////////////////
	if( (nHeight > 14) || nOutlineStyle==1 || (nSamplingMultiplier>1))
		m_isBigFont = true;

	return true;
}

void RFont::Destroy(void)
{
	_ASSERT(m_FontMap.size()<=(unsigned int)m_nCacheSize);
	_ASSERT(m_FontUsage.size()<=(unsigned int)m_nCacheSize);
	_ASSERT(m_FontUsage.size()==(unsigned int)m_FontMap.size());

/*
	if(m_pTex) {
	delete m_pTex;
	m_pTex = NULL;
	}
*/

	m_FontUsage.clear();

	while(m_FontMap.empty()==false) {
		delete (*m_FontMap.begin()).second;
		m_FontMap.erase(m_FontMap.begin());
	}

	if(m_hFont!=NULL) {
		DeleteObject(m_hFont);
		m_hFont = NULL;
	}

	if(m_dwStateBlock!=NULL) {
		m_pd3dDevice->DeleteStateBlock(m_dwStateBlock);
		m_dwStateBlock = NULL;
	}

	m_pd3dDevice = NULL;
}

static int g_font_draw_call = 0;
static int g_font_size_call = 0;

void Frame_Begin()
{
	g_font_draw_call = 0;
	g_font_size_call = 0;
}

void Frame_End()
{
	static char _temp[256];
	sprintf( _temp,"font cnt = %d map_size = %d \n",g_font_draw_call,g_font_size_call);
	OutputDebugString(_temp);
}

bool RFont::BeginFont()
{
	if(m_bFontBegin) {
		mlog("이미 BeginFont 중이다~\n");
//		ASSERT(0);
		return false;
	}

	m_bFontBegin = true;

	return true;
}

bool RFont::EndFont()
{
	if(!m_bFontBegin) {
		mlog("이미 EndFont 중이다~\n");
//		ASSERT(0);
		return false;
	}

	BeginFontState();

	RFont_Render();

	EndState();

	m_bFontBegin = false;

	return true;
}

void RFont::DrawText(float x, float y, const TCHAR* szText, DWORD dwColor, float fScale)
{
	if(szText==NULL)	return;
	if(szText[0]==NULL) return;

	if(m_bFontBegin==false)
		g_font_cnt = 0;

	const TCHAR* p = (const TCHAR*)szText;
	TCHAR szChar[4];

	g_font_draw_call++;
//	g_font_size_call+=m_FontMap.size();

/*
	static char _temp[256];
	sprintf(_temp,"draw_text : %s \n",szText );
	OutputDebugString(_temp);
*/

	if(m_isBigFont==false)	{
		if(m_bFontBegin==false) // begin - end 모아서 그리는거라면 해줄필요없다~
			BeginFontState();
	}

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

//		STRING2FONTTEXTURE::iterator i = m_FontMap.find(szChar);
		
		WORD key = *(WORD*)szChar;
		FONTTEXTUREMAP::iterator i = m_FontMap.find(key);

		// New
		bool bMake = false;

		if(i==m_FontMap.end()) {// 없다면

			if(m_FontMap.size()>=(unsigned int)m_nCacheSize) {
				// Find Discardable FontTexture
				FONTTEXTUREPRIORITYQUE::iterator pqi = m_FontUsage.begin();
				FONTTEXTUREMAP::iterator fmi = *pqi;		// 가장 먼저 만든  폰트를 지우고 있다 ~ (-.-) ~ 수정해야 한다.
				RFONTTEXTUREINFO* pDiscard = (*fmi).second;		// 가장 오래전에 사용한 폰트를 먼저 지운다.
				delete pDiscard;
				m_FontUsage.erase(m_FontUsage.begin());
				m_FontMap.erase(fmi);
			}

			RFONTTEXTUREINFO* pNew = new RFONTTEXTUREINFO;

			// 생성못한경우..
			if(pNew->FontTexture.Create(this, m_hFont, szChar, m_nOutlineStyle, m_ColorArg1, m_ColorArg2, m_nSamplingMultiplier)==false) {
				delete pNew;
				pNew = NULL;
				p = pp;
				if(pp==NULL) break;
				if(*pp==0)	 break;
				continue;//이 글자만 skip 뻗는 것보다는 안보이는것이...
			}

			m_FontMap.insert(FONTTEXTUREMAP::value_type(key, pNew));

			i = m_FontMap.find(key);

			_ASSERT(i!=m_FontMap.end());

			(*i).second->pqi = m_FontUsage.insert(m_FontUsage.end(), i);

		} 
		else {//있어도 갱신이 필요하다면~

			// 한바퀴돌아서 폰트가 지워졌는가 검사~
			// 여러폰트맵들이 같은 텍스쳐를 사용하기 때문에 지워졌는가 알수가 없다~

			if(m_isBigFont==false) {

				RFontTexture* pFontTexture = &(((*i).second)->FontTexture);

				int type = pFontTexture->m_nFontType;
				int table = pFontTexture->m_nTableIndex;
				int index = pFontTexture->m_nIndex;

				if( m_pTex->isNeedUpdate(type,table,index) ) {
					if(pFontTexture->Create(this, m_hFont, szChar, m_nOutlineStyle, m_ColorArg1, m_ColorArg2, m_nSamplingMultiplier)==false) { 
						p = pp;
						if(pp==NULL) break;
						if(*pp==0)	 break;
						continue;// 이 글자만 skip 뻗는 것보다는 안보이는것이...
					}
				}
			}
		}

		RFontTexture* pFontTexture = &(((*i).second)->FontTexture);

		_ASSERT(pFontTexture!=NULL);

		if(pFontTexture != NULL) {

			RFontTexture::FTCOLORMODE nMode = RFontTexture::FTCOLORMODE_DIFFUSE;

			if(m_nOutlineStyle==1)
				nMode = RFontTexture::FTCOLORMODE_FIXED;

			pFontTexture->DrawText(nMode, x, y, dwColor, fScale);

			if(m_isBigFont==false) {

				if (m_nOutlineStyle==1)
					x += ((pFontTexture->m_nRealTextWidth) * fScale*1.2f);
				else
					x += ((pFontTexture->m_nRealTextWidth) * fScale);

			}
			else {

				if (m_nOutlineStyle==1)
					x += (pFontTexture->GetTextWidth()*fScale*1.2f);
				else
					x += (pFontTexture->GetTextWidth()*fScale);
			}

		}
//		_ASSERT(m_FontUsage.size()==m_FontMap.size());

		p = pp;

		if(pp==NULL) break;
		if(*pp==0)	 break;
	}

	if(m_isBigFont==false) {
		if(m_bFontBegin==false) {
			RFont_Render();
			EndState();
		}
	}

}

int RFont::GetCharWidth(const TCHAR* szChar)
{
	SIZE size;

	HDC hDC = CreateCompatibleDC(NULL);
	HFONT hPrevFont = (HFONT)SelectObject(hDC, m_hFont);

	GetTextExtentPoint32(hDC, szChar, _tcslen(szChar), &size);

	SelectObject(hDC, hPrevFont);

	DeleteDC(hDC);

	if(m_isBigFont==false) {
		return size.cx;
	}

	return (size.cx/m_nSamplingMultiplier);
}

int RFont::GetTextWidth(const TCHAR* szText, int nSize)
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
		FONTWIDTHMAP::iterator i = m_FontWidthMap.find(key);
		
		int nCurWidth;
		if(i==m_FontWidthMap.end()) {// 없다면
			nCurWidth=GetCharWidth(szChar);
			m_FontWidthMap.insert(FONTWIDTHMAP::value_type(key,nCurWidth));
		}else
		{
			nCurWidth=i->second;
		}

		nWidth+=nCurWidth;

		p = pp;

		if(pp==NULL) break;
		if(*pp==0)	 break;
		if(nSize!=-1 && nCount>=nSize) break;
	}

	return nWidth;
}

void ResetFont()
{
//	DWORD RFontTexture::m_dwStateBlock = NULL;
//	DWORD RFont::m_dwStateBlock = NULL;

	if(RFontTexture::m_dwStateBlock != NULL) {
		RGetDevice()->DeleteStateBlock(RFontTexture::m_dwStateBlock);
		RFontTexture::m_dwStateBlock = NULL;
	}

	if(RFont::m_dwStateBlock != NULL) {
		RGetDevice()->DeleteStateBlock(RFont::m_dwStateBlock);
		RFont::m_dwStateBlock = NULL;
	}
}

_NAMESPACE_REALSPACE2_END
