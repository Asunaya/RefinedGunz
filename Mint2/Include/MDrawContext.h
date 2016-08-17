//
//	MINT Draw Context
//
#ifndef MDRAWCONTEXT_H
#define MDRAWCONTEXT_H

#include "CMList.h"
#include "MTypes.h"
#include "MBitmap.h"
#include "MFont.h"
#include "MCursor.h"
#include "MResourceManager.h"

struct MRECT;
struct MPOINT;

#define MINT_BASE_CLASS_TYPE	0x1030		///< RTTI와 같은 기능을 _DEBUG모드에서 구현하기 위한 ID

/// 정렬 모드 ( 비트 플래그 )
/*
enum MAlignmentMode{
	MAM_NOTALIGN	= 0,	///< 정렬하지 않음
	MAM_LEFT		= 1,	///< 왼쪽 정렬
	MAM_RIGHT		= 2,	///< 오른쪽 정렬
	MAM_HCENTER		= 4,	///< 가로 가운데 정렬
	MAM_EDIT		= 8,	///< Edit위젯에서 쓰이는 정렬
	MAM_TOP			= 16,	///< 윗 정렬
	MAM_BOTTOM		= 32,	///< 아래 정렬
	MAM_VCENTER		= 64,	///< 세로 가운데 정렬
};
*/
typedef int MAlignmentMode;
#define MAM_NOTALIGN	0	///< 정렬하지 않음
#define MAM_LEFT		1	///< 왼쪽 정렬
#define MAM_RIGHT		2	///< 오른쪽 정렬
#define MAM_HCENTER		4	///< 가로 가운데 정렬
#define MAM_EDIT		8	///< Edit위젯에서 쓰이는 정렬
#define MAM_TOP			16	///< 윗 정렬
#define MAM_BOTTOM		32	///< 아래 정렬
#define MAM_VCENTER		64	///< 세로 가운데 정렬


enum MDrawEffect
{
	MDE_NORMAL = 0,
	MDE_ADD,
	MDE_MULTIPLY,

	MDE_MAX
};

/// Draw Context, 위젯내에 그릴수 있는 모든 함수를 제공한다.
class MDrawContext{
protected:
#ifdef _DEBUG
	int		m_nTypeID;
#endif
	MCOLOR			m_Color;			///< 현재 선택되어 있는 컬러
	MCOLOR			m_HighlightColor;	///< 현재 선택되어 있는 하일라이트 컬러(&문자 다음에 오는 문자를 하일라이팅해준다.)
	MCOLOR			m_ColorKey;			///< 스프라이트 출력을 위한 컬러키
	MCOLOR			m_BitmapColor;	/// < 비트맵에 섞는 색 (기본값 : White(255,255,255,255) )
	MBitmap*		m_pBitmap;			///< 현재 선택되어 있는 비트맵
	MFont*			m_pFont;			///< 현재 선택되어 있는 폰트
	MRECT			m_Clip;				///< 스크린 좌표계에서의 클리핑 영역
	MPOINT			m_Origin;			///< 스크린에 그려질때 현재 논리적 원점
	MDrawEffect		m_Effect;			///< 효과
	unsigned char	m_nOpacity;
public:
	MDrawContext(void);
	virtual ~MDrawContext(void);

	// Color Setting
	MCOLOR SetBitmapColor(MCOLOR& color);
	MCOLOR SetBitmapColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a=255);
	MCOLOR GetBitmapColor(void);
	MCOLOR SetColor(MCOLOR& color);
	MCOLOR SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a=255);
	MCOLOR GetColor(void);
	MCOLOR SetHighlightColor(MCOLOR& color);
	MCOLOR SetHighlightColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a=255);
	MCOLOR GetHighlightColor(void);
	MCOLOR SetColorKey(MCOLOR& color);
	MCOLOR SetColorKey(unsigned char r, unsigned char g, unsigned char b, unsigned char a=255);
	MCOLOR GetColorKey(void);

	// Font Setting
	MFont* SetFont(MFont* pFont);
	MFont* GetFont(){ return m_pFont; }

	// Bitmap Setting
	MBitmap* SetBitmap(MBitmap* pBitmap);
	MBitmap* GetBitmap(){ return m_pBitmap; }

	// Clipping ( Screen Coordinate )
	virtual void SetClipRect(MRECT& r);
	void SetClipRect(int x, int y, int w, int h);
	MRECT GetClipRect(void);

	// Logical Origin
	void SetOrigin(int x, int y);
	void SetOrigin(MPOINT& p);
	MPOINT GetOrigin(void);

	// Draw Effect
	void SetEffect(MDrawEffect effect);
	MDrawEffect GetEffect() { return m_Effect; }

	// Basic Drawing Functions
	virtual void SetPixel(int x, int y, MCOLOR& color) = 0;
	virtual void HLine(int x, int y, int len) = 0;
	virtual void VLine(int x, int y, int len) = 0;
	virtual void Line(int sx, int sy, int ex, int ey) = 0;
	virtual void Rectangle(int x, int y, int cx, int cy);
	void Rectangle(MRECT& r);
	virtual void FillRectangle(int x, int y, int cx, int cy) = 0;
	void FillRectangle(const MRECT& r);

	// Bitmap Drawing
	void Draw(int x, int y);
	void Draw(int x, int y, int w, int h);
	void Draw(int x, int y, int sx, int sy, int sw, int sh);
	void Draw(MPOINT &p);
	void Draw(MRECT& r);
	void Draw(int x, int y, MRECT& s);
	virtual void Draw(MRECT& d, MRECT& s);
	virtual void DrawInverse( int x, int y, int w, int h );
	virtual void DrawInverse( int x, int y, int w, int h, int sx, int sy, int sw, int sh) {};
	virtual void DrawEx(int tx1, int ty1, int tx2, int ty2, 
		                int tx3, int ty3, int tx4, int ty4);
	virtual void Draw(int x, int y, int w, int h, int sx, int sy, int sw, int sh);

	// Text
	virtual bool BeginFont() = 0;
	virtual bool EndFont() = 0;
	virtual int Text(int x, int y, const char* szText) = 0;
	int Text(MPOINT& p, const char* szText){ return Text(p.x, p.y, szText); }
	virtual int TextMultiLine(MRECT& r, const char* szText,int nLineGap=0,bool bAutoNextLine=true,int nIndentation=0,int nSkipLine=0, MPOINT* pPositions=NULL);
	virtual int TextMultiLine2(MRECT& r, const char* szText,int nLineGap=0,bool bAutoNextLine=true,MAlignmentMode am=(MAM_HCENTER|MAM_VCENTER) );

	/*
	/// nPos번째 글자가 찍히는 상대위치를 구한다
	static bool GetTextPosition(MPOINT *pOut,MFont *pFont,int nRectWidth, const char* szText,int nPos,bool bColorSupport=false);
	/// 상대위치로 몇번째 글자인지 얻어낸다
	static int GetCharPosition(MFont *pFont,int nX,int nLine,int nRectWidth,const char* szText,bool bColorSupport=false);
	*/

	/// &를 고려하여 &부분을 하일라이팅
	int TextWithHighlight(int x, int y, const char* szText);
	/// 특정 텍스트의 정렬되었을때 시작 위치 얻기
	void GetPositionOfAlignment(MPOINT* p, MRECT& r, const char* szText, MAlignmentMode am, bool bAndInclude=true);
	int Text(MRECT& r, const char* szText, MAlignmentMode am=(MAM_HCENTER|MAM_VCENTER));
	int TextWithHighlight(MRECT& r, const char* szText, MAlignmentMode am=(MAM_HCENTER|MAM_VCENTER));

	// Multiple Color Support
	void TextMC(int x, int y, const char* szText);
	void TextMC(MRECT& r, const char* szText, MAlignmentMode am);
	
	/// Multiple Color 기호를 제거한 텍스트를 구한다.
	static char* GetPureText(const char *szText);

	/// Opacity를 설정한다.
	unsigned char SetOpacity(unsigned char nOpacity);
	unsigned char GetOpacity();

private:
	virtual void Draw(MBitmap *pBitmap,int x, int y, int w, int h, int sx, int sy, int sw, int sh) = 0;		// User Defined Draw Funtion
};


/// Draw Context for 3D
///		- 3D Context Object의 다양성에 따라 특정 펑션셋을 미리 정의하지 않는다.
///		- 단순히 객체 전달을 위한 클래스이다.
/// @deprecated	AceSaga를 위한 코드로, 3D Rendering Pass가 따로 존재하는 경우에 사용되었던 임의의 코드이다. 앞으로는 더이상 사용되지 않는다.
class MDrawContext3D{
};


// multiline/color support 펑션들. TextMultiline 을 사용해서 출력할때 모양새를 알수있다.

// 텍스트의 폭을 얻는다. (한줄에서만 정상동작을 보장한다)
int MMGetWidth(MFont *pFont,const char* szText, int nSize, bool bColorSupport=false);

// 다음 줄의 시작 위치를 리턴한다
int MMGetNextLinePos(MFont *pFont,const char* szText, int nWidth, bool bAutoNextLine=true, bool bColorSupport=false);

/// nLine 행이 시작하는 위치를 리턴. nIndentation은 2행부터 들여쓰기 픽셀수
int MMGetLinePos(MFont *pFont,const char* szText, int nWidth, bool bAutoNextLine=true, bool bColorSupport=false,int nLine=1,int nIndentation=0);

/// 찍히는 행의 수를 구한다.  TextMultiLine을 위한 함수.
int MMGetLineCount(MFont *pFont,const char* szText, int nWidth, bool bAutoNextLine=true, bool bColorSupport=false,int nIndentation=0);


#endif
