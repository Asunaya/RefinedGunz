/*
 CDDBButton 클래스(CDDBButton.cpp)

  desc : DDB 버튼을 생성하고 처리하는 클래스
  date : 2004년 5월 30일
  comp : 임동환
*/


#include <stdafx.h>
#include "CDDBButton.h"


/************************************************************************************
  CDDBButton window
*************************************************************************************/
// CDDBButton
CDDBButton::CDDBButton()
{
	m_nButtonState		= BBS_UP;
	m_nMouseState		= BMS_LEAVE;
	m_fTrackMouseEvent	= false;
}


//~CDDBButton
CDDBButton::~CDDBButton()
{
}


// Message map
BEGIN_MESSAGE_MAP(CDDBButton, CButton)
	//{{AFX_MSG_MAP(CDDBButton)
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/************************************************************************************
  CDDBButton message handlers
*************************************************************************************/
// InitDDBButton
BOOL CDDBButton::InitDDBButton( UINT nIDSkinResource, UINT nIDWavResource)
{
	// Crear DDB
	ddbSkin.DeleteDDB();
	ddbRendering.DeleteDDB();

	// Create skin DDB
	if ( ! ddbSkin.CreateDDB( GetDC(), nIDSkinResource))
		return false;

	// Get button Srceen
	m_nWidth  = (ddbSkin.GetWidth() - 3) / 4;
	m_nHeight = ddbSkin.GetHeight();

	// Create rendering DDB
	if ( ! ddbRendering.CreateDDB( 0, 0, GetDC(), ddbSkin.GetWidth(), m_nHeight, 0))
		return false;

	// Draw button DDB
	ddbRendering.PutBitmap( 0, 0, ddbSkin.GetDC(), 0, 0, ddbSkin.GetWidth(), m_nHeight, SRCCOPY);
	CString str;
	GetWindowText( str);
	SetWindowText( str);

	// Set wave ID resource
	m_nIDWavResource = nIDWavResource;

	// Draw button Srceen
	SetWindowPos( NULL, 0, 0, m_nWidth, m_nHeight, SWP_NOMOVE | SWP_NOOWNERZORDER);

	return true;
}


// InitDDBButton
BOOL CDDBButton::InitDDBButton( CDC* pDC, int xSrc, int ySrc, int widthSrc, int heightSrc, UINT nIDWavResource)
{
	// Crear DDB
	ddbSkin.DeleteDDB();
	ddbRendering.DeleteDDB();

	// Create skin DDB
	if ( ! ddbSkin.CreateDDB( 0, 0, GetDC(), widthSrc*4+3, heightSrc, 0))
		return false;
	ddbSkin.PutBitmap( 0, 0, pDC, xSrc, ySrc, widthSrc*4+3, heightSrc, SRCCOPY);

	// Get button size
	m_nWidth  = widthSrc;
	m_nHeight = heightSrc;

	// Create rendering DDB
	if ( ! ddbRendering.CreateDDB( 0, 0, GetDC(), ddbSkin.GetWidth(), heightSrc, RGB(255,255,255)))
		return false;

	// Draw button DDB
	ddbRendering.PutBitmap( 0, 0, ddbSkin.GetDC(), 0, 0, ddbSkin.GetWidth(), m_nHeight, SRCCOPY);
	CString str;
	GetWindowText( str);
	SetWindowText( str);

	// Set wave ID resource
	m_nIDWavResource = nIDWavResource;

	// Draw button Srceen
	SetWindowPos( NULL, 0, 0, m_nWidth, m_nHeight, SWP_NOMOVE | SWP_NOOWNERZORDER);

	return true;
}


// DrawItem
void CDDBButton::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
		m_nButtonState = BBS_DOWN;

	else if (lpDrawItemStruct->itemState & ODS_DISABLED)
		m_nButtonState = BBS_DISABLE;

	else
	{	
		if (m_nMouseState == BMS_OVER)
			m_nButtonState = BBS_FOCUS;

		else
			m_nButtonState = BBS_UP;
	}

	ddbRendering.Rendering( 0, 0, m_nWidth*m_nButtonState+m_nButtonState, 0, m_nWidth, m_nHeight);
}


// PreSubclassWindow
void CDDBButton::PreSubclassWindow() 
{
	ModifyStyle( 0, BS_OWNERDRAW);

	CButton::PreSubclassWindow();
}


// OnMouseMove
void CDDBButton::OnMouseMove( UINT nFlags, CPoint point) 
{
	if ( m_fTrackMouseEvent == false)
	{
		SetClassLong( m_hWnd, GCL_HCURSOR, (LONG)(AfxGetApp()->LoadStandardCursor(IDC_ARROW)));

		TRACKMOUSEEVENT trackMouseEvent;
		trackMouseEvent.cbSize = sizeof( trackMouseEvent);
		trackMouseEvent.hwndTrack = m_hWnd;
		trackMouseEvent.dwFlags = TME_LEAVE | TME_HOVER;
		trackMouseEvent.dwHoverTime = 1;
		m_fTrackMouseEvent = _TrackMouseEvent( &trackMouseEvent);
	}
	
	CButton::OnMouseMove(nFlags, point);
}


// OnMouseHover
LRESULT CDDBButton::OnMouseHover(WPARAM wParam, LPARAM lParam) 
{	
	m_nMouseState = BMS_OVER;

	Invalidate();
		
	return 0;
}


// OnMouseLeave
LRESULT CDDBButton::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	m_fTrackMouseEvent = false;
	m_nMouseState = BMS_LEAVE;

	Invalidate();
	
	return 0;
}


// OnLButtonDown
void CDDBButton::OnLButtonDown( UINT nFlags, CPoint point) 
{
	// PlaySound
	if ( m_nIDWavResource != NULL)
		m_sndSound.PlayWavSound( m_nIDWavResource);

	CButton::OnLButtonDown( nFlags, point);
}



// SetWindowText
void CDDBButton::SetWindowText(LPCTSTR lpszString)
{
	// Clear bitmap
	ddbRendering.PutBitmap( 0, 0, ddbSkin.GetDC(), 0, 0, ddbSkin.GetWidth(), m_nHeight, SRCCOPY);

	// Draw Pop Button Text
	COLORREF color[] = { m_WindowText.nTextUp,      m_WindowText.nTextShadowUp,
		                 m_WindowText.nTextFocus,   m_WindowText.nTextShadowFocus,
						 m_WindowText.nTextDown,    m_WindowText.nTextShadowDown,
						 m_WindowText.nTextDisable, m_WindowText.nTextShadowDisable};
	for ( int i = 0;  i < 4;  i++)
	{
		// Text shadow
		ddbRendering.PutText( m_nWidth*i+i+(i==2 ? 1:0), 1+(i==2 ? 1:0), m_nWidth, m_nHeight-2,
			                  m_WindowText.nTextSize, m_WindowText.lpszFont, lpszString,
							  TSTYLE_BOLD, DT_SINGLELINE | DT_CENTER | DT_VCENTER,
							  color[i*2+1]);
		// Text shadow
		ddbRendering.PutText( m_nWidth*i+i+1+(i==2 ? 1:0), 1+(i==2 ? 1:0), m_nWidth, m_nHeight-2,
			                  m_WindowText.nTextSize, m_WindowText.lpszFont, lpszString,
							  TSTYLE_BOLD, DT_SINGLELINE | DT_CENTER | DT_VCENTER,
							  color[i*2+1]);
		// Text
		ddbRendering.PutText( m_nWidth*i+i+(i==2 ? 1:0), (i==2 ? 1:0), m_nWidth, m_nHeight-2,
			                  m_WindowText.nTextSize, m_WindowText.lpszFont, lpszString,
							  TSTYLE_BOLD, DT_SINGLELINE | DT_CENTER | DT_VCENTER,
							  color[i*2]);
	}

	// Set dialog button text
	::SetWindowText( m_hWnd, lpszString);
}


// MoveWindow
void CDDBButton::MoveWindow( UINT x, UINT y)
{
	::MoveWindow( m_hWnd, x, y, m_nWidth, m_nHeight, true);
}


// GetWidth
UINT CDDBButton::GetWidth()
{
	return m_nWidth;
}


// GetHeight
UINT CDDBButton::GetHeight()
{
	return m_nHeight;
}
