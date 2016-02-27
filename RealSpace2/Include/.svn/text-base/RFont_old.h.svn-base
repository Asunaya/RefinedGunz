#ifndef RFONT_H
#define RFONT_H

#pragma warning(disable: 4786)

#include "RTypes.h"
#include <d3dx8.h>
#include <map>
#include <list>
#include <string>
#include "Realspace2.h"

using namespace std;


/*
	16 픽셀이상의 폰트와 특수폰트들은 이전방식으로 개별처리한다..( 아주 가끔 출력 )	
	한가해지면 정리하기 - 너무지저분 ~ -.-
*/

enum r_font_type {
	font_type_16 = 0,
	font_type_32,

	font_type_end,
};

class RBaseFontTextureInfo {
public:
	RBaseFontTextureInfo() {

		m_nSize = 0;
		m_nLastIndex = 0;
		m_nTableIndex = 0; // 한바퀴 돌때마다 갱신

		m_nMinIndexSize	= 0;
		m_nMaxIndexSize	= 0;

		m_fXLen = 0.f;
		m_fYLen = 0.f;
	}

	int m_nSize;
	int m_nLastIndex;

	float m_fXLen;
	float m_fYLen;

	int m_nTableIndex;

	int m_nMinIndexSize;//시작위치
	int m_nMaxIndexSize;//최대위치
};

#define FONT_TEXTURE_MAX 512
//#define FONT_TEXTURE_MAX 256


class RBaseFontTexture;

class RBaseFontTexture {
protected:
	RBaseFontTexture() {

		m_bLock = false;

		m_nTextureWidth	= 0;
		m_nTextureHeight= 0;

		m_pTexture = NULL;
		m_pInfo = NULL;

		Create(RealSpace2::RGetDevice(),FONT_TEXTURE_MAX,FONT_TEXTURE_MAX);
	}

public:

	~RBaseFontTexture() {
		Destroy();
	}

	static RBaseFontTexture* GetInstance() {
		static RBaseFontTexture pInst;
		return &pInst;
	}
	bool Create(LPDIRECT3DDEVICE8 dev,int w,int h);


	void Destroy() {
		Clear();
	}

	void Clear();

	long Lock() {

		HRESULT hr;

		if(m_pTexture==NULL) return -1;
		if(m_bLock) return 0;

		hr = m_pTexture->LockRect(0, &m_d3dlr, 0, 0);

		m_bLock = true;

		return hr;
	}

	long UnLock() {

		HRESULT hr;

		if(m_pTexture==NULL) return -1;
		if(m_bLock==false) return 0;

		hr = m_pTexture->UnlockRect(0);

		m_bLock = false;

		return hr;
	}

	bool GetInfo(int type);
	void GetIndexPos(int type,int index,int& x,int& y);
	BYTE* GetPosPtr(int type,int x,int y);
	BYTE* GetIndexPtr(int type,int index);
	void GetIndexUV(int type,int index,int w,int h,float*uv);

	int MakeTexture(int type);
	int MakeTexture(int type,DWORD* pBitmapBits,int w,int h,int nSamplingMultiplier,int debug_cnt);

	LPDIRECT3DTEXTURE8 GetTexture() {
		return m_pTexture;
	}

	// 다른 폰트에 의해 덮어 써졌는가 체크~

	bool isNeedUpdate(int type,int table,int index) {

		if( m_info[type].m_nTableIndex!=table ) {
			if(m_info[type].m_nTableIndex > table+1) {//1바퀴 이상 순환한 경우 무조건~
				return true;
			}
			else {
				if( m_info[type].m_nLastIndex >= index ) {//한바퀴라면 자신을 앞질러야~
					return true;
				}
			}
		}
		return false;
	}

public:

	bool m_bLock;

	D3DLOCKED_RECT		m_d3dlr;
	LPDIRECT3DTEXTURE8	m_pTexture;

	int	m_nTextureWidth;
	int m_nTextureHeight;		

	RBaseFontTextureInfo* m_pInfo;
	RBaseFontTextureInfo m_info[font_type_end];
};

#define GetBaseFontTexture() RBaseFontTexture::GetInstance()

_NAMESPACE_REALSPACE2_BEGIN

class RFont;

class RFontTexture{
public:
	LPDIRECT3DDEVICE8		m_pd3dDevice;		// Local Copy
	LPDIRECT3DTEXTURE8		m_pTexture;			// Text Texture

	int	m_nTextWidth, m_nTextHeight;			// Text Size
	int	m_nTextureWidth, m_nTextureHeight;		// Texture Size
	int	m_nRealTextWidth,m_nRealTextHeight;

	RFont*	m_pFont;
	int		m_nIndex;
	int		m_nFontType;
	int		m_nTableIndex;

public:
	enum FTCOLORMODE {
		FTCOLORMODE_DIFFUSE,
		FTCOLORMODE_FIXED
	};


public:
	RFontTexture(void);
	virtual ~RFontTexture(void);

	bool Create(RFont* pFont , HFONT hFont, const TCHAR* szText, int nOutlineStyle, DWORD nColorArg1, DWORD nColorArg2, int nSamplingMultiplier);
	void Destroy(void);

	void BeginState(FTCOLORMODE nMode=FTCOLORMODE_DIFFUSE);
	void EndState(void);

	void DrawText(FTCOLORMODE nMode, float x, float y, DWORD dwColor,float fScale=1.0f);

	int GetTextWidth(void){ return m_nTextWidth; }
	int GetTextHeight(void){ return m_nTextHeight; }

	static DWORD			m_dwStateBlock;		// 하나만 있으면 ㅇㅋ
};

struct RFONTTEXTUREINFO;

/*
#if defined(UNICODE) || defined(_UNICODE)
	typedef map<wstring, RFONTTEXTUREINFO*>		STRING2FONTTEXTURE;
#else
	typedef map<string, RFONTTEXTUREINFO*>		STRING2FONTTEXTURE;
#endif
*/

typedef map<WORD,RFONTTEXTUREINFO*>		FONTTEXTUREMAP;
typedef map<WORD,int>					FONTWIDTHMAP;

//typedef list<STRING2FONTTEXTURE::iterator>	FONTTEXTUREPRIORITYQUE;
typedef list<FONTTEXTUREMAP::iterator>	FONTTEXTUREPRIORITYQUE;

struct RFONTTEXTUREINFO{
	RFontTexture						FontTexture;
	FONTTEXTUREPRIORITYQUE::iterator	pqi;
};


class RFont {
public:
	LPDIRECT3DDEVICE8	m_pd3dDevice;		// Local Copy
	RBaseFontTexture*	m_pTex;

	static DWORD m_dwStateBlock;

	void BeginFontState();
	void EndFontState(void);

	void BeginState();
	void EndState(void);

	int m_nDebugFontCount;

	bool m_isBigFont;

protected:
	HFONT					m_hFont;	// Font Handle
//	STRING2FONTTEXTURE		m_FontMap;
	FONTTEXTUREMAP			m_FontMap;
	FONTWIDTHMAP			m_FontWidthMap;
	int						m_nCacheSize;
	FONTTEXTUREPRIORITYQUE	m_FontUsage;
	int						m_nHeight;
	int						m_nOutlineStyle;
	int						m_nSamplingMultiplier;

	DWORD					m_ColorArg1;
	DWORD					m_ColorArg2;

	bool					m_bFontBegin;

public:
	RFont(void);
	virtual ~RFont(void);

	bool Create(LPDIRECT3DDEVICE8 pd3dDevice, const TCHAR* szFontName, int nHeight, bool bBold=false, bool bItalic=false, int nOutlineStyle=0, int nCacheSize=-1, int nSamplingMultiplier=1, DWORD nColorArg1=0, DWORD nColorArg2=0);
	void Destroy(void);

//
//	주의 사항
//	BeginFont 와 EndFont 사이에 다른 텍스쳐를 가지는 큰 폰트를 그리면 안된다~
//

	bool BeginFont();
	bool EndFont();

	void DrawText(float x, float y, const TCHAR* szText, DWORD dwColor=0xFFFFFFFF, float fScale=1.0f);

	int GetSamplingMultiplier(void){ return m_nSamplingMultiplier; }
	int GetHeight(void){ return m_nHeight; }
	int GetTextWidth(const TCHAR* szText, int nSize=-1);

protected:
	int GetCharWidth(const TCHAR* szChar);
};

void RFont_Render();

void Frame_Begin();
void Frame_End();

void ResetFont();

_NAMESPACE_REALSPACE2_END

bool CheckFont();
void SetUserDefineFont(char* FontName);

#endif
