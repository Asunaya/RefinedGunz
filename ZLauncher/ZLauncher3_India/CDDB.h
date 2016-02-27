/*
 CDDB 클래스(CDDB.h);

  desc : DDB를 생성하고 처리하는 클래스
  date : 2004년 5월 30일
  comp : 임동환
*/


#ifndef _INCLUDED_CDDB_
#define _INCLUDED_CDDB_


// Define text style
#define TSTYLE_NORMAL				0x0000		// Normal
#define TSTYLE_BOLD					0x0001		// Bold
#define TSTYLE_ITALIC				0x0002		// Italic
#define TSTYLE_UDLINE				0x0004		// Underline

// Define axis method
#define AXISMETHOD_ENDPOINT			0x0001		// Start-End method
#define AXISMETHOD_WIDTHHEIGHT		0x0002		// Start-Width/Height method


/////////////////////////////////////////////////////////////////////////////
// CDDB Class
class CDDB
{
public:
	// Variables
	CDC m_cDDB;								// DDB
	int m_nWndX;							// Window position x
	int m_nWndY;							// Window position y

	// Functions
	CDDB();
	~CDDB();

	BOOL CreateDDB( int x, int y, CDC* pDC, int width, int height, COLORREF color);
	BOOL CreateDDB( CDC* pDC, UINT nIDResource);
	BOOL DeleteDDB();

	void Rendering();
	void Rendering( int x, int y, int x1Src, int y1Src, int x2Src_or_width, int y2Src_or_height);

	BOOL PutBitmap( int x, int y, CDC *pSrcDC, int x1Src, int y1Src, int x2Src_or_width, int y2Src_or_height, DWORD dwRop);
	BOOL PutBitmap( int x, int y, CDC *pSrcDC, int x1Src, int y1Src, int x2Src_or_width, int y2Src_or_height, int widthSrc, int heightSrc);
	BOOL PutAlphaBlendBitmap( int x, int y, CDC *pSrcDC, int x1Src, int y1Src, int x2Src_or_width, int y2Src_or_height, UINT alpha);
	BOOL PutBmpResource( int x, int y, UINT nIDResource, int x1Src, int y1Src, int x2Src_or_width, int y2Src_or_height);
	BOOL PutBmpResource( int x, int y, UINT nIDResource, int x1Src, int y1Src, int x2Src_or_width, int y2Src_or_height, int widthSrc, int heightSrc);
	
	void FillColor( COLORREF color);
	BOOL FillTiledBitmap( int x1, int y1, int x2_or_width, int y2_or_height, CDC *pSrcDC, int x1Src, int y1Src, int x2Src_or_width, int y2Src_or_height);
	BOOL FillTiledBmpResource( UINT nIDResource);
	
	void PutPixel( int x, int y, COLORREF color);
	void PutLine( int x1, int y1, int x2_or_width, int y2_or_height, int width, COLORREF color);
	void PutBox( int x1, int y1, int x2_or_width, int y2_or_height, int width, COLORREF color);
	void PutFillBox( int x1, int y1, int x2_or_width, int y2_or_height, COLORREF color);
	void PutRoundBox( int x1, int y1, int x2_or_width, int y2_or_height, int radius, int width, COLORREF color);
	void PutFillRoundBox( int x1, int y1, int x2_or_width, int y2_or_height, int radius, int width, COLORREF color);
	void PutCircle( int x, int y, int radius, int width, COLORREF color);
	void PutFillCircle( int x, int y, int radius, COLORREF color);
	void PutFillEllips( int x, int y, int radiusX, int radiusY, COLORREF color);
	void PutText( int x, int y, int nSize, LPCTSTR lpszFont, LPCTSTR lpszMsg, UINT nStyle, COLORREF color);
	void PutText( int x1, int y1, int x2_or_width, int y2_or_height, int nSize, LPCTSTR lpszFont, LPCTSTR lpszMsg, UINT nStyle, UINT nAlign, COLORREF color);
	
	void SetAxisMethod( UINT nAxisMethod);
	UINT GetAxisMethod();
	
	void SetTransColor( COLORREF color);
	COLORREF GetTransColor();
	
	CDC *GetDC();
	
	int GetWidth();
	int GetHeight();
	
	COLORREF GetColor( int x, int y);


protected:
	// Variables
	CDC *m_pdcParentWnd;					// DC pointer of parent window
	UINT m_nAxisMethod;						// Axis method
	int m_nWidth;							// Width of bitmap
	int m_nHeight;							// Height of bitmap
	COLORREF m_nTransColor;					// Transparent color

	void GetWidthHeight(int *x1, int *y1, int *x2, int *y2);
	void GetEndPoint(int *x1, int *y1, int *width, int *height);
};

#endif
