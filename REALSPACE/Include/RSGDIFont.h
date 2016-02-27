#ifndef __RSGDIFONT_H
#define __RSGDIFONT_H

#include <windows.h>
#include "RSTypes.h"

class RSGDIFont
{
	HFONT	m_hFont;
	HDC		m_hMemDC;
	HBITMAP m_hBitmap;
	int		m_iBmpWidth,m_iBmpHeight;
	DWORD	m_dwColor;
	RSPIXELFORMAT	m_PixelFormat;
	LPSTR           m_pBmpBits;     // pointer to DIB bits
	RECT			m_ClipRect;

public:
	RSGDIFont();
	virtual ~RSGDIFont();

	bool Create(const char *fontname,int nSize,bool bBold,bool bItalic,RSPIXELFORMAT pf,RSVIEWPORT *ClipRect);
	void DrawText(void *pBuffer,int lPitch,int x,int y,LPSTR text,...);
	void SetColor(DWORD color) { m_dwColor=color; }
	void SetClipRect(RSVIEWPORT *pClipRect);
	void Destroy();
	void GetTextExtentPoint(const char *string, SIZE *lpsize);

};

#endif