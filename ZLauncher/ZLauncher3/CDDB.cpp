/*
 CDDB 클래스(CDDB.cpp)

  desc : DDB를 생성하고 처리하는 클래스
  date : 2004년 5월 30일
  comp : 임동환

  반드시 Project Settings에서 Link->Object/library modules에
  msimg32.lib을 추가해줘야 함.
*/


#include <stdafx.h>
#include "CDDB.h"



/*************************************************************
 CDDB

  desc : 생성자
**************************************************************/
CDDB::CDDB()
{
	m_nWndX			= 0;							// Reset window position x
	m_nWndY			= 0;							// Reset window position y
	m_nWidth		= 0;							// Reset width
	m_nHeight		= 0;							// Reset height
	m_nTransColor	= RGB(255,0,255);				// Set transparent color
	m_pdcParentWnd	= NULL;							// Reset parent window DC pointer
	m_nAxisMethod	= AXISMETHOD_WIDTHHEIGHT;		// Reset axis method
}


/*************************************************************
 ~CDDB

  desc : 소멸자
**************************************************************/
CDDB::~CDDB()
{
	m_cDDB.DeleteDC();								// Delete memory DC
	m_pdcParentWnd	= NULL;							// Reset parent window DC pointer
	m_nWndX			= 0;							// Reset window position x
	m_nWndY			= 0;							// Reset window position y
	m_nWidth		= 0;							// Reset width
	m_nHeight		= 0;							// Reset height
}


/*************************************************************
 CreateDDB

  desc : 지정한 크기의 DDB를 생성한 후에 입력한 색으로 DDB 전체를 채운다.
  arg  : pDC = 윈도우의 DC 포인터 입력
         x1, y1 = 생성할 DDB의 시작 좌표
         x2_or_width, y2_or_height = 생성할 DDB의 끝 좌표 혹은 크기
		 color = DDB를 채울 생상값(RGB) 입력
  ret  : true(=성공) or false(=실패)
  ex   : CreateDDB( GetDC(), 200, 100, RGB(255,255,255));
**************************************************************/
BOOL CDDB::CreateDDB( int x, int y, CDC* pDC, int width, int height, COLORREF color)
{
	if ( m_cDDB.CreateCompatibleDC( pDC))			// Create memory DC
	{
		m_pdcParentWnd = pDC;						// Set parent window DC pointer

		// Create bitmap
		CBitmap bitmap;
		bitmap.CreateCompatibleBitmap( pDC, width, height);
		m_cDDB.SelectObject( &bitmap);

		// Initialize
		m_nWndX   = x;								// Set window position x
		m_nWndY   = y;								// Set window position y
		m_nWidth  = width;							// Set width
		m_nHeight = height;							// Set height
		m_cDDB.SetBkMode( TRANSPARENT);				// Set text background color

		// Fill color
		m_cDDB.FillSolidRect( 0, 0, width, height, color);

		return true;								// Success
	}

	return false;									// Fail
}


/*************************************************************
 CreateDDB

  desc : 지정한 비트맵 리소스를 가진 DDB를 생성한다.
  arg  : pDC = 윈도우의 DC 포인터 입력
		 nIDResource = 이미지 리소스 ID
  ret  : true(=성공) or false(=실패)
  ex   : CreateDDB( GetDC(), IDB_BITMAP);
**************************************************************/
BOOL CDDB::CreateDDB( CDC* pDC, UINT nIDResource)
{
	if ( m_cDDB.CreateCompatibleDC( pDC))			// Create memory DC
	{
		m_pdcParentWnd = pDC;						// Set window DC

		// Create bitmap
		CBitmap bitmap;
		if ( !bitmap.LoadBitmap( nIDResource))
			return false;							// Load bitmap fail
		m_cDDB.SelectObject( &bitmap);				// Create bitmap

		// Initialize
		BITMAP bmpInfo;
		bitmap.GetBitmap( &bmpInfo);				// Get bitmap infomation
		m_nWndX   = 0;								// Set window position x
		m_nWndY   = 0;								// Set window position y
		m_nWidth  = bmpInfo.bmWidth;				// Get bitmap width size
		m_nHeight = bmpInfo.bmHeight;				// Get bitmap height size
		m_cDDB.SetBkMode( TRANSPARENT);				// Set text background color

		return true;								// Success
	}

	return false;									// Fail
}


/*************************************************************
 DeleteDDB

  desc : DDB를 삭제한다.
  arg  : none
  ret  : true(=성공) or false(=실패)
  exp  : DeleteDDB();
**************************************************************/
BOOL CDDB::DeleteDDB()
{
	m_pdcParentWnd	= NULL;							// Reset parent window pointer
	m_nWndX			= 0;							// Reset window position x
	m_nWndY			= 0;							// Reset window position y
	m_nWidth		= 0;							// Reset width
	m_nHeight		= 0;							// Reset height

	return  m_cDDB.DeleteDC();						// Delete memory DC
}


/*************************************************************
 Rendering

  desc : DDB 이미지의 전체를 parent window의 DC에 출력한다.
  arg  : none
  ret  : none
  ex   : Rendering();
**************************************************************/
void CDDB::Rendering()
{
	// Copy memory bitmap
	m_pdcParentWnd->BitBlt( m_nWndX, m_nWndY,
		                    m_nWidth, m_nHeight,
							&m_cDDB,
							0, 0,
							SRCCOPY);
}


/*************************************************************
 Rendering

  desc : DDB 이미지의 일부 영역을 parent window의 DC에 출력한다.
  arg  : x, y = 이미지가 출력될 parent window DC의 시작 좌표
         x1Src, y1Src = 출력할 이미지의 시작 좌표
		 x2Src_or_width, y2Src_or_height = 출력할 이미지의 끝 좌표 혹은 크기
  ret  : none
  ex   : Rendering( 200, 100, 300, 400, 100, 100);
**************************************************************/
void CDDB::Rendering( int x, int y, int x1Src, int y1Src, int x2Src_or_width, int y2Src_or_height)
{
	// Axis method is not Start-Width/Height
	if ( m_nAxisMethod != AXISMETHOD_WIDTHHEIGHT)
		GetWidthHeight( &x1Src, &y1Src, &x2Src_or_width, &y2Src_or_height);

	m_pdcParentWnd->BitBlt( x+m_nWndX, y+m_nWndY,
		                    x2Src_or_width, y2Src_or_height,
							&m_cDDB,
	                        x1Src, y1Src,
							SRCCOPY);
}


/*************************************************************
 PutBitmap

  desc : DDB의 지정한 위치에 다른 DDB로부터 이미지 일부 영역을 출력한다.
  arg  : x, y = 출력을 시작할 위치 좌표 입력
		 *pSrcDC = 이미지를 가져올 source DDB의 DC 포인터 입력
         x1Src, y1Src = Source DDB에서 이미지를 가져올 시작 좌표 입력
		 x2Src_or_width, y2Src_or_height = Source DDB에서 이미지를 가져올 끝 좌표 혹은 크기
		 dwRop = 래스터 오퍼레이션(TRANSPARENT=투명색 처리, SRCCOPY, SRCAND, SRCOR...)
  ret  : true(=성공) or false(=실패)
  ex   : PutBitmap( 100, 100, GetDC(), 200, 200, 300, 300, SRCCOPY);
**************************************************************/
BOOL CDDB::PutBitmap( int x, int y, CDC *pSrcDC, int x1Src, int y1Src, int x2Src_or_width, int y2Src_or_height, DWORD dwRop)
{
	// Copy transparent memory DC
	if ( dwRop == TRANSPARENT)
	{
		// Copy memory bitmap
		// Works with only Windows XP, 2000, Windows 98, and later systems.
		TransparentBlt( m_cDDB,
			            x, y,
						x2Src_or_width, y2Src_or_height,
						pSrcDC->m_hDC,
						x1Src, y1Src,
						x2Src_or_width, y2Src_or_height,
						m_nTransColor);
	}

	// Copy memory DC
	else
	{
		// Axis method is not Start-Width/Height
		if ( m_nAxisMethod != AXISMETHOD_WIDTHHEIGHT)
			GetWidthHeight( &x1Src, &y1Src, &x2Src_or_width, &y2Src_or_height);

		// Copy memory bitmap
		m_cDDB.BitBlt( x, y,
			            x2Src_or_width, y2Src_or_height,
						pSrcDC,
						x1Src, y1Src, dwRop);
	}

	return true;									// Success
}


/*************************************************************
 PutBitmap

  desc : DDB의 지정한 위치에 비트맵 이미지를 지정한 크기로 늘이거나 줄여서 출력한다.
  arg  : x, y = 복사를 시작할 위치 좌표 입력
		 *pSrcDC = 이미지를 가져올 source DDB의 DC 포인터 입력
         x1Src, y1Src = Source DDB에서 이미지를 가져올 시작 좌표 입력
		 x2Src_or_width, y2Src_or_height = Source DDB에서 이미지를 가져올 끝 좌표 혹은 크기
		 widthSrc, heightSrc = 출력될 이미지의 크기
  ret  : true(=성공) or false(=실패)
  ex   : PutBitmap( 100, 100, GetDC(), 200, 200, 300, 300, 400, 400);
**************************************************************/
BOOL CDDB::PutBitmap( int x, int y, CDC *pSrcDC, int x1Src, int y1Src, int x2Src_or_width, int y2Src_or_height, int widthSrc, int heightSrc)
{
	// Axis method is not Start-Width/Height
	if ( m_nAxisMethod != AXISMETHOD_WIDTHHEIGHT)
		GetWidthHeight( &x1Src, &y1Src, &x2Src_or_width, &y2Src_or_height);

	// Copy memory bitmap
	return m_cDDB.StretchBlt( x, y,
		                       widthSrc, heightSrc,
							   pSrcDC,
							   x1Src, y1Src,
							   x2Src_or_width, y2Src_or_height,
							   SRCCOPY);
}


/*************************************************************
 PutAlphaBlendBitmap

  desc : DDB의 지정한 위치에 다른 DDB로부터 이미지 일부 영역을 지정한 투명도 값으로 출력한다.
  arg  : x, y = 출력을 시작할 위치 좌표 입력
		 *pSrcDC = 이미지를 가져올 source DDB의 DC 포인터 입력
         x1Src, y1Src = Source DDB에서 이미지를 가져올 시작 좌표 입력
		 x2Src_or_width, y2Src_or_height = Source DDB에서 이미지를 가져올 끝 좌표 혹은 크기
		 alpha = 투명도 값(0 ~ 255)
  ret  : true(=성공) or false(=실패)
  ex   : PutAlphaBlendBitmap( 100, 100, GetDC(), 200, 200, 300, 300, 128);
**************************************************************/
BOOL CDDB::PutAlphaBlendBitmap( int x, int y, CDC *pSrcDC, int x1Src, int y1Src, int x2Src_or_width, int y2Src_or_height, UINT alpha)
{
	BLENDFUNCTION blendFunc = { AC_SRC_OVER, 0, alpha, 0 };

	// Copy memory bitmap
	// Works with only Windows XP, 2000, Windows 98, and later systems.
	AlphaBlend( m_cDDB,
		        x, y,
				x2Src_or_width, y2Src_or_height,
				pSrcDC->m_hDC,
				x1Src, y1Src,
				x2Src_or_width, y2Src_or_height,
				blendFunc);

	return true;
}


/*************************************************************
 PutBmpResource

  desc : DDB에 지정한 비트맵 리소스의 일부 영역을 출력한다.
  arg  : x, y = DDB에 이미지를 출력할 시작 좌표
		 nIDResource = 비트맵 리소스 ID
         x1Src, y1Src = 비트맵 리소스에서 출력할 이미지 영역의 시작 좌표
		 x2Src_or_width, y2Src_or_height = 출력할 이미지 영역의 끝 좌표 혹은 크기
  ret  : true(=성공) or false(=실패)
  ex   : PutBmpResource( 200, 100, IDB_BITMAP, 300, 400, 100, 100);
**************************************************************/
BOOL CDDB::PutBmpResource( int x, int y, UINT nIDResource, int x1Src, int y1Src, int x2Src_or_width, int y2Src_or_height)
{
	CDC NewDC;
	NewDC.CreateCompatibleDC( m_pdcParentWnd);
	
	CBitmap NewBmp;
	if ( NewBmp.LoadBitmap( nIDResource))
	{
		CBitmap *pOldBmp = (CBitmap *)NewDC.SelectObject( &NewBmp);
		BITMAP bmpInfo;
		NewBmp.GetBitmap(&bmpInfo);

		// Axis method is not Start-Width/Height
		if ( m_nAxisMethod != AXISMETHOD_WIDTHHEIGHT)
			GetWidthHeight( &x1Src, &y1Src, &x2Src_or_width, &y2Src_or_height);

		// Copy memory bitmap
		m_cDDB.BitBlt( x, y,
			            x2Src_or_width, y2Src_or_height,
						&NewDC,
						x1Src, y1Src,
						SRCCOPY);
	
		NewDC.SelectObject( pOldBmp);

		return true;
	}

	return false;
}


/*************************************************************
 PutBmpResource

  desc : 지정한 비트맵 리소스 이미지의 크기를 늘이거나 줄여서 출력한다.
  arg  : nIDResource = 비트맵 리소스 ID
         x1Src, y1Src = DDB에 이미지를 출력할 시작 좌표
         x2Src_or_width, y2Src_or_height = DDB에 이미지를 출력할 끝 좌표 혹은 크기
  ret  : true(=성공) or false(=실패)
  ex   : PutBmpResource( 100, 100, IDB_BITMAP, 200, 100, 100, 100);
**************************************************************/
BOOL CDDB::PutBmpResource( int x, int y, UINT nIDResource, int x1Src, int y1Src, int x2Src_or_width, int y2Src_or_height, int widthSrc, int heightSrc)
{
	CDC NewDC;
	NewDC.CreateCompatibleDC( m_pdcParentWnd);
	
	CBitmap NewBmp;
	if ( NewBmp.LoadBitmap( nIDResource))
	{
		CBitmap *pOldBmp = (CBitmap *)NewDC.SelectObject( &NewBmp);
		BITMAP bmpInfo;
		NewBmp.GetBitmap( &bmpInfo);

		// Axis method is not Start-Width/Height
		if ( m_nAxisMethod != AXISMETHOD_WIDTHHEIGHT)
			GetWidthHeight( &x1Src, &y1Src, &x2Src_or_width, &y2Src_or_height);

		// Copy memory bitmap
		m_cDDB.StretchBlt( x, y,
			                widthSrc, heightSrc,
							&NewDC,
			                x1Src, y1Src,
							bmpInfo.bmWidth, bmpInfo.bmHeight,
							SRCCOPY);

		NewDC.SelectObject( pOldBmp);
		return true;
	}

	return false;
}


/*************************************************************
 FillColor

  desc : DDB 전체를 지정한 색으로 채운다.
  arg  : color = 채울 색상(RGB)
  ret  : none
  ex   : FillColor( RGB(255,255,255));
**************************************************************/
void CDDB::FillColor( COLORREF color)
{
	m_cDDB.FillSolidRect( 0, 0, m_nWidth, m_nHeight, color);
}


/*************************************************************
 FillTiledBitmap

  desc : DDB 전체를 지정한 비트맵 리소스를 타일 형태로 채운다.
  arg  : x1, y1 = 비트맵으로 채워질 영역 시작 좌표
         x2_or_width, y2_or_height = 비트맵으로 채워질 영역 끝 좌표 혹은 크기
		 pSrcDC = 이미지를 가져올 source DDB의 DC 포인터 입력
		 x1Src, y1Src = Source DDB에서 이미지를 가져올 시작 좌표
		 x2Src_or_width, y2Src_or_height = Source DDB에서 이미지를 가져올 끝 좌표 혹은 크기
  ret  : true(=성공) or false(=실패)
  ex   : FillTiledBitmap( 100, 100, 200, 200, GetDC(), 50, 50, 100, 100);
**************************************************************/
BOOL CDDB::FillTiledBitmap( int x1, int y1, int x2_or_width, int y2_or_height, CDC *pSrcDC, int x1Src, int y1Src, int x2Src_or_width, int y2Src_or_height)
{
	// Axis method is not Start-End point
	if ( m_nAxisMethod != AXISMETHOD_ENDPOINT)
		GetEndPoint( &x1, &y1, &x2_or_width, &y2_or_height);

	// Axis method is not Start-Width/Height
	if ( m_nAxisMethod != AXISMETHOD_WIDTHHEIGHT)
		GetWidthHeight( &x1Src, &y1Src, &x2Src_or_width, &y2Src_or_height);

	int px, py;
	int width, height;
	for ( py = y1;  py < y2_or_height;  py+=y2Src_or_height)
	{
		height = y2_or_height - py;
		if ( height > y2Src_or_height)
			height = y2Src_or_height;

		for ( px = x1;  px < x2_or_width;  px+=x2Src_or_width)
		{
			width = x2_or_width - px;
			if ( width > x2Src_or_width)
				width = x2Src_or_width;

			m_cDDB.BitBlt( px, py,
				            width, height,
							pSrcDC,
							x1Src, y1Src,
							SRCCOPY);
		}
	}

	return true;
}


/*************************************************************
 FillTiledBmpResource

  desc : DDB 전체를 지정한 비트맵 리소스를 타일 형태로 채운다.
  arg  : nIDResource = 채울 비트맵 리소스 ID
  ret  : true(=성공) or false(=실패)
  ex   : FillTiledBitmap( IDB_BITMAP);
**************************************************************/
BOOL CDDB::FillTiledBmpResource( UINT nIDResource)
{
	CDC NewDC;
	NewDC.CreateCompatibleDC( m_pdcParentWnd);
	
	CBitmap NewBmp;
	if ( NewBmp.LoadBitmap( nIDResource))
	{
		CBitmap *pOldBmp = (CBitmap *)NewDC.SelectObject( &NewBmp);
		BITMAP bmpInfo;
		NewBmp.GetBitmap( &bmpInfo);

		int x, y;
		for ( y = 0;  y < m_nHeight;  y+=bmpInfo.bmHeight)
		{
			for ( x = 0;  x < m_nWidth;  x+=bmpInfo.bmWidth)
			{
				m_cDDB.BitBlt( x, y,
				                bmpInfo.bmWidth, bmpInfo.bmHeight,
								&NewDC,
								0, 0,
								SRCCOPY);
			}
		}

		NewDC.SelectObject( pOldBmp);
		return true;
	}

	return false;
}


/*************************************************************
 PutPixel

  desc : DDB에 점을 출력한다.
  arg  : x, y = DDB에 점을 출력할 좌표
         color = 점 색상
  ret  : none
  ex   : PutPixel( 200, 100, RGB(2255,255,255));
**************************************************************/
void CDDB::PutPixel( int x, int y, COLORREF color)
{
	m_cDDB.SetPixel( x, y, color);
}


/*************************************************************
 PutLine

  desc : DDB에 선을 출력한다. (x1,y1)-(x2,y2)
  arg  : x1, y1 = DDB에 선을 출력할 시작 좌표
         x2_or_width, y2_or_height = DDB에 선을 출력할 끝 좌표 혹은 크기
		 width = 선 두께
         color = 선 색상(RGB)
  ret  : none
  ex   : PutLine( 100, 100, 200, 200, 1, RGB(255,255,255));
**************************************************************/
void CDDB::PutLine( int x1, int y1, int x2_or_width, int y2_or_height, int width, COLORREF color)
{
	CPen pen;
	pen.CreatePen( PS_SOLID, width, color);
	CPen *pOldPen = (CPen *)m_cDDB.SelectObject( &pen);

	// Axis method is not Start-End point
	if ( m_nAxisMethod != AXISMETHOD_ENDPOINT)
		GetEndPoint( &x1, &y1, &x2_or_width, &y2_or_height);

	m_cDDB.MoveTo( x1, y1);
	m_cDDB.LineTo( x2_or_width, y2_or_height);
	m_cDDB.SelectObject( pOldPen);
}


/*************************************************************
 PutBox

  desc : DDB에 사각형을 출력한다.
  arg  : x1, y1 = DDB에 사각형을 출력할 시작 좌표
         x2_or_width, y2_or_height = DDB에 사각형을 출력할 끝 좌표 혹은 크기
		 width = 선 두께
         color = 사각형 색상(RGB)
  ret  : none
  ex   : PutBox( 100, 100, 200, 200, 1, RGB(255,255,255));
**************************************************************/
void CDDB::PutBox( int x1, int y1, int x2_or_width, int y2_or_height, int width, COLORREF color)
{
	CPen pen;
	pen.CreatePen( PS_SOLID, width, color);
	CPen *pOldPen = (CPen *)m_cDDB.SelectObject( &pen);

	// Axis method is not Start-End point
	if ( m_nAxisMethod != AXISMETHOD_ENDPOINT)
		GetEndPoint( &x1, &y1, &x2_or_width, &y2_or_height);

	m_cDDB.MoveTo( x1, y1);
	m_cDDB.LineTo( x2_or_width, y1);
	m_cDDB.LineTo( x2_or_width, y2_or_height);
	m_cDDB.LineTo( x1, y2_or_height);
	m_cDDB.LineTo( x1, y1);
	m_cDDB.SelectObject( pOldPen);
}


/*************************************************************
 PutFillBox

  desc : DDB에 내부를 채운 사각형을 출력한다.
  arg  : x1, y1 = DDB에 사각형을 출력할 시작 좌표
         x2_or_width, y2_or_height = DDB에 사각형을 출력할 끝 좌표
         color = 사각형 색상(RGB)
  ret  : none
  ex   : PutFillBox( 200, 100, 100, 100, RGB(255,255,255));
**************************************************************/
void CDDB::PutFillBox( int x1, int y1, int x2_or_width, int y2_or_height, COLORREF color)
{
	// Axis method is Start-End point
	if ( m_nAxisMethod != AXISMETHOD_WIDTHHEIGHT)
		GetWidthHeight( &x1, &y1, &x2_or_width, &y2_or_height);

	m_cDDB.FillSolidRect( x1, y1, x2_or_width, y2_or_height, color);
}


/*************************************************************
 PutRoundBox

  desc : DDB에 둥근 사각형을 출력한다.
  arg  : x1, y1 = DDB에 둥근 사각형을 출력할 시작 좌표
         x2_or_width, y2_or_height = DDB에 사각형을 출력할 끝 좌표 혹은 크기
		 radius = 모서리 원의 지름
		 width = 선 두께
         color = 사각형 색상(RGB)
  ret  : none
  ex   : PutRoundBox( 100, 100, 200, 200, 5, 1, RGB(255,255,255));
**************************************************************/
void CDDB::PutRoundBox( int x1, int y1, int x2_or_width, int y2_or_height, int radius, int width, COLORREF color)
{
	CPen pen;
	pen.CreatePen( PS_SOLID, width, color);
	CPen *pOldPen = (CPen *)m_cDDB.SelectObject( &pen);

	// Axis method is not Start-End point
	if ( m_nAxisMethod != AXISMETHOD_ENDPOINT)
		GetEndPoint( &x1, &y1, &x2_or_width, &y2_or_height);

	m_cDDB.MoveTo( x1 + radius, y1); 
	m_cDDB.AngleArc( x1 + radius, y1 + radius, radius, 90, 90); 
	m_cDDB.MoveTo( x2_or_width, y1 + radius); 
	m_cDDB.AngleArc( x2_or_width - radius, y1 + radius, radius, 0, 90); 
	m_cDDB.MoveTo( x1, y2_or_height - radius); 
	m_cDDB.AngleArc( x1 + radius, y2_or_height - radius, radius, 180, 90); 
	m_cDDB.MoveTo( x2_or_width - radius, y2_or_height); 
	m_cDDB.AngleArc( x2_or_width - radius, y2_or_height - radius, radius, 270, 90); 

	m_cDDB.MoveTo( x1 + radius, y1); 
	m_cDDB.LineTo( x2_or_width - radius + 1, y1); 
	m_cDDB.MoveTo( x1 + radius, y2_or_height); 
	m_cDDB.LineTo( x2_or_width - radius + 1, y2_or_height); 
	m_cDDB.MoveTo( x1, y1 + radius); 
	m_cDDB.LineTo( x1, y2_or_height - radius + 1); 
	m_cDDB.MoveTo( x2_or_width, y1 + radius); 
	m_cDDB.LineTo( x2_or_width, y2_or_height - radius + 1); 

	m_cDDB.SelectObject( pOldPen);
}


/*************************************************************
 PutFillRoundBox

  desc : DDB에 사각형을 출력한다.
  arg  : x1, y1 = DDB에 사각형을 출력할 시작 좌표
         x2_or_width, y2_or_height = DDB에 사각형을 출력할 끝 좌표 혹은 크기
		 width = 선 두께
         color = 사각형 색상(RGB)
  ret  : none
  ex   : PutFillRoundBox( 100, 100, 200, 200, 5, 1, RGB(255,255,255));
**************************************************************/
void CDDB::PutFillRoundBox( int x1, int y1, int x2_or_width, int y2_or_height, int radius, int width, COLORREF color)
{
	CPen pen;
	pen.CreatePen( PS_SOLID, width, color);
	CPen *pOldPen = (CPen *)m_cDDB.SelectObject( &pen);

	CBrush brush;
	brush.CreateSolidBrush( color);
	CBrush *pOldBrush = (CBrush *)m_cDDB.SelectObject( &brush);

	// Axis method is not Start-End point
	if ( m_nAxisMethod != AXISMETHOD_ENDPOINT)
		GetEndPoint( &x1, &y1, &x2_or_width, &y2_or_height);

	m_cDDB.RoundRect( x1, y1, x2_or_width, y2_or_height, radius, radius);

	m_cDDB.SelectObject( pOldPen);
	m_cDDB.SelectObject( &pOldBrush);
}


/*************************************************************
 PutCircle

  desc : DDB에 원을 출력한다.
  arg  : x, y = 원을 출력할 중심점
         radius = 원의 반지름
		 width = 선 두께
         color = 원의 색상(RGB)
  ret  : none
  ex   : PutCircle( 100, 100, 50, 1, RGB(0,0,0));
**************************************************************/
void CDDB::PutCircle( int x, int y, int radius, int width, COLORREF color)
{
	CPen pen;
	pen.CreatePen( PS_SOLID, width, color);
	CPen *pOldPen = (CPen *)m_cDDB.SelectObject( &pen);

	m_cDDB.MoveTo( x + radius, y); 
	m_cDDB.AngleArc( x, y, radius, 0, 360); 

	m_cDDB.SelectObject( pOldPen);
}


/*************************************************************
 PutFillCircle

  desc : DDB에 내부를 채운 원을 출력한다.
  arg  : x, y = 원을 출력할 중심점
         radius = 원의 반지름
         color = 원의 색상(RGB)
  ret  : none
  ex   : PutFillCircle( 100, 100, 50, RGB(0,0,0));
**************************************************************/
void CDDB::PutFillCircle( int x, int y, int radius, COLORREF color)
{
	CPen pen;
	pen.CreatePen( PS_SOLID, 1, color);
	CPen *pOldPen = (CPen *)m_cDDB.SelectObject( &pen);

	CBrush brush;
	brush.CreateSolidBrush( color);
	CBrush *pOldBrush = (CBrush *)m_cDDB.SelectObject( &brush);

	m_cDDB.Ellipse( x-radius, y-radius, x+radius, y+radius);

	m_cDDB.SelectObject( pOldPen);
	m_cDDB.SelectObject( pOldBrush);
}


/*************************************************************
 PutFillEllips

  desc : DDB에 내부를 채운 타원을 출력한다.
  arg  : x, y = 타원을 출력할 중심점
         radiusX, radiusY = 원의 x,y축 반지름
         color = 원의 색상(RGB)
  ret  : none
  ex   : PutFillEllips( 100, 100, 50, 30, RGB(0,0,0));
**************************************************************/
void CDDB::PutFillEllips( int x, int y, int radiusX, int radiusY, COLORREF color)
{
	CPen pen;
	pen.CreatePen( PS_SOLID, 1, color);
	CPen *pOldPen = (CPen *)m_cDDB.SelectObject( &pen);

	CBrush brush;
	brush.CreateSolidBrush( color);
	CBrush *pOldBrush = (CBrush *)m_cDDB.SelectObject( &brush);

	m_cDDB.Ellipse( x-radiusX, y-radiusY, x+radiusX, y+radiusY);

	m_cDDB.SelectObject( pOldPen);
	m_cDDB.SelectObject( pOldBrush);
}



/*************************************************************
 PutText

  desc : DDB에 글씨를 출력한다.
  arg  : x, y = DDB에 글씨를 출력할 시작 좌표
         nSize = 글씨 크기
		 lpszFont = 글씨 폰트
		 lpszMsg = 출력 내용
		 nStyle = 글씨 스타일 ( TSTYLE_NORMAL | TSTYLE_BOLD | TSTYLE_ITALIC | TSTYLE_UDLINE)
         color = 색상
  ret  : none
  ex   : PutText( 200, 100, 14, "굴림", "PutText 예제", TSTYLE_BOLD, RGB(255,255,255));
**************************************************************/
void CDDB::PutText( int x, int y, int nSize, LPCTSTR lpszFont, LPCTSTR lpszMsg, UINT nStyle, COLORREF color)
{
	UINT nBold		= (nStyle & TSTYLE_BOLD)   ? FW_BOLD : FW_NORMAL;
	BYTE bItalic	= (nStyle & TSTYLE_ITALIC) ? true : false;
	BYTE bUnderLine	= (nStyle & TSTYLE_UDLINE) ? true : false;

	CFont font;
	font.CreateFont( nSize, 0, 0, 0, nBold, bItalic, bUnderLine, 0, ANSI_CHARSET,
	                 OUT_CHARACTER_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	                 DEFAULT_PITCH | FF_SWISS, lpszFont);
	CFont *pOldFont = (CFont *)m_cDDB.SelectObject( &font);
	m_cDDB.SetTextColor( color);
	m_cDDB.TextOut( x, y, lpszMsg);
	m_cDDB.SelectObject( pOldFont);
}


/*************************************************************
 PutText

  desc : DDB의 지정한 영역에 글씨를 출력한다.
  arg  : x1, y1 = 글씨가 출력될 영역의 시작 좌표
         x2_or_width, y2_or_height = 글씨가 출력될 영역의 끝 좌표 혹은 크기
         nSize = 글씨 크기
		 lpszFont = 글씨 폰트
		 lpszMsg = 출력 내용
		 nStyle = 글씨 스타일 ( TSTYLE_NORMAL | TSTYLE_BOLD | TSTYLE_ITALIC | TSTYLE_UDLINE)
		 nAlign = 글씨 정렬( DT_LEFT | DT_CENTER | DT_RIGHT | DT_TOP | DT_VCENTER | DT_BOTTOM)
         color = 색상
  ret  : none
  ex   : PutText( 200, 100, 300, 200, 14, "굴림", "PutText 예제", TSTYLE_BOLD, DT_CENTER|DT_VCENTER, RGB(255,255,255)
**************************************************************/
void CDDB::PutText( int x1, int y1, int x2_or_width, int y2_or_height, int nSize, LPCTSTR lpszFont, LPCTSTR lpszMsg, UINT nStyle, UINT nAlign, COLORREF color)
{
	UINT nBold		= (nStyle & TSTYLE_BOLD)   ? FW_BOLD : FW_NORMAL;
	BYTE bItalic	= (nStyle & TSTYLE_ITALIC) ? true : false;
	BYTE bUnderLine	= (nStyle & TSTYLE_UDLINE) ? true : false;

	CFont font;
	font.CreateFont( nSize, 0, 0, 0, nBold, bItalic, bUnderLine, 0, ANSI_CHARSET,
	                 OUT_CHARACTER_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	                 DEFAULT_PITCH | FF_SWISS, lpszFont);
	CFont *pOldFont = (CFont *)m_cDDB.SelectObject( &font);
	
	CRect rect;
	rect.top = y1;
	rect.left = x1;
	
	// Axis method is Start-End point
	if ( m_nAxisMethod != AXISMETHOD_ENDPOINT)
		GetEndPoint( &x1, &y1, &x2_or_width, &y2_or_height);

	rect.right = x2_or_width;
	rect.bottom = y2_or_height;

	m_cDDB.SetTextColor( color);
	m_cDDB.DrawText( lpszMsg, (LPRECT)rect, nAlign);
	m_cDDB.SelectObject( pOldFont);
}


/*************************************************************
 SetAxisMethod

  desc : Axis method를 설정한다.
  arg  : AXISMETHOD_ENDPOINT or AXISMETHOD_WIDTHHEIGHT
  ret  : none
  ex   : SetAxisMethod( AXISMETHOD_ENDPOINT);
**************************************************************/
void CDDB::SetAxisMethod( UINT nAxisMethod)
{
	if ( (nAxisMethod == AXISMETHOD_ENDPOINT) || (nAxisMethod ==  AXISMETHOD_WIDTHHEIGHT))
		m_nAxisMethod = nAxisMethod;
}


/*************************************************************
 GetAxisMethod

  desc : 현재 설정된 Axis method값을 구한다.
  arg  : none
  ret  : AXISMETHOD_ENDPOINT or AXISMETHOD_WIDTHHEIGHT
  ex   : UINT AxisMethod = GetAxisMethod();
**************************************************************/
UINT CDDB::GetAxisMethod()
{
	return m_nAxisMethod;
}


/*************************************************************
 SetTransColor

  desc : DDB에 적용할 투명색을 지정한다.
  arg  : color = 투명색 색상(RGB)
  ret  : none
  ex   : SetTransColor(255,0,255);
**************************************************************/
void CDDB::SetTransColor( COLORREF color)
{
	m_nTransColor = color;
}


/*************************************************************
 GetTransColor

  desc : 현재 DDB에 적용된 투명색 색상값(RGB)을 구한다.
  arg  : none
  ret  : 투명색 색상값
  ex   : COLORREF TransColor = GetTransColor();
**************************************************************/
COLORREF CDDB::GetTransColor()
{
	return m_nTransColor;
}


/*************************************************************
 GetDC

  desc : DDB의 DC 포인터를 구한다.
  arg  : none
  ret  : DC 포인터
  ex   : CDC dc = GetDC();
**************************************************************/
CDC *CDDB::GetDC()
{
	return  &m_cDDB;
}


/*************************************************************
 GetWidth

  desc : DDB의 전체 넓이를 구한다.
  arg  : none
  ret  : DDB의 넓이(pixel)
  ex   : int width = GetWidth();
**************************************************************/
int CDDB::GetWidth()
{
	return m_nWidth;
}


/*************************************************************
 GetHeight

  desc : DDB의 전체 높이를 구한다.
  arg  : none
  ret  : DDB의 높이(pixel)
  ex   : int height = GetHeight();
**************************************************************/
int CDDB::GetHeight()
{
	return m_nHeight;
}


/*************************************************************
 GetColor

  desc : DDB에서 지정한 좌표의 색상값(RGB)을 구한다.
  arg  : x, y = 색상값을 구할 좌표(pixel)
  ret  : pixel의 색상값
  ex   : COLORREF Color = GetColor( 200, 100);
**************************************************************/
COLORREF CDDB::GetColor( int x, int y)
{
	return m_cDDB.GetPixel( x, y);
}


/*************************************************************
 (protected)GetWidthHeight

  desc : Start-End point 좌표를 넓이/높이 값으로 변환한다
  arg  : *x1, *y1 = 시작 좌표
         *x2, *y2 = 끝 좌표
  ret  : none
**************************************************************/
void CDDB::GetWidthHeight(int *x1, int *y1, int *x2, int *y2)
{
	*x2 = *x2 - *x1;						// Get width
	*y2 = *y2 - *y1;						// Get height
}


/*************************************************************
 (protected)GetEndPoint

  desc : Start-Width/Height 좌표를 end point 값으로 변환한다
  arg  : *x1, *y1 = 시작 좌표
         *width, *height = 넓이/높이
  ret  : none
**************************************************************/
void CDDB::GetEndPoint(int *x1, int *y1, int *width, int *height)
{
	*width  = *x1 + *width;					// Get end x
	*height = *y1 + *height;				// Get end y
}
