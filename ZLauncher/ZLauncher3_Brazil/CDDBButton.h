/*
 CDDBButton 클래스(CDDBButton.h)

  desc : DDB 버튼을 생성하고 처리하는 클래스
  date : 2004년 5월 30일
  comp : 임동환
*/


#ifndef _INCLUDED_CDDBBUTTON_
#define _INCLUDED_CDDBBUTTON_


// Include
#include "CDDB.h"
#include "CSound.h"



/////////////////////////////////////////////////////////////////////////////
// CDDBButton window
class CDDBButton : public CButton
{
	// Button window text structure
	struct CDDBBUTTONWINDOWTEXT
	{
		CDDBBUTTONWINDOWTEXT()
		{
			nTextUp				= RGB(255,255,255);
			nTextShadowUp		= RGB(20,20,100);
			nTextFocus			= RGB(255,255,255);
			nTextShadowFocus	= RGB(20,20,100);
			nTextDown			= RGB(255,255,100);
			nTextShadowDown		= RGB(50,50,150);
			nTextDisable		= RGB(255,255,255);
			nTextShadowDisable	= RGB(210,210,210);
			lpszFont			= "Arial";
			nTextSize			= 14;
		}

		COLORREF nTextUp;
		COLORREF nTextShadowUp;
		COLORREF nTextFocus;
		COLORREF nTextShadowFocus;
		COLORREF nTextDown;
		COLORREF nTextShadowDown;
		COLORREF nTextDisable;
		COLORREF nTextShadowDisable;
		LPCTSTR lpszFont;
		UINT nTextSize;
	};


public:
	const enum BUTTONSTATE { BBS_UP, BBS_FOCUS, BBS_DOWN, BBS_DISABLE};
	const enum MOUSESTATE  { BMS_LEAVE, BMS_OVER};

	CDDBButton();
	virtual ~CDDBButton();
	BOOL InitDDBButton( UINT nIDSkinResource, UINT nIDWavResource);
	BOOL InitDDBButton( CDC* pDC, int xSrc, int ySrc, int widthSrc, int heightSrc, UINT nIDWavResource);
	void SetWindowText( LPCTSTR lpszString);
	void MoveWindow( UINT x, UINT y);
	UINT GetWidth();
	UINT GetHeight();

	//{{AFX_VIRTUAL(CDDBButton)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL


protected:
	CDDB ddbRendering;						// Rendering DDB
	CDDB ddbSkin;							// Skin DDB
	UINT m_nWidth;							// Width
	UINT m_nHeight;							// Height
	UINT m_nButtonState;					// Button state
	UINT m_nMouseState;						// Mouse state
	BOOL m_fTrackMouseEvent;				// Track mouse event
	UINT m_nIDWavResource;					// Wave ID resource
	CDDBBUTTONWINDOWTEXT m_WindowText;		// Informations of window text
	CSound m_sndSound;						// Sound

	void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct);

	//{{AFX_MSG(CDDBButton)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseHover(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif
