// RSConsoleRenderer.h: interface for the RSConsoleRenderer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RSCONSOLERENDERER_H__2009E2F9_844E_4677_BE04_56A4033AA1C0__INCLUDED_)
#define AFX_RSCONSOLERENDERER_H__2009E2F9_844E_4677_BE04_56A4033AA1C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "windows.h"
#include "rsutils.h"

class RSFont;

class RSConsoleRenderer  
{
public:
	RSConsoleRenderer();
	virtual ~RSConsoleRenderer();

	BOOL Create(const char *englishfontname,const char *hangulfontname = NULL);

	void TextOut(LPBYTE pTarget, int lPitch, int nX,int nY,DWORD color,char* string,...);
	void TextOutWithClip(LPBYTE pTarget, int lPitch, int nX,int nY,DWORD color,char* string,...);

	void SetPixelFormat(WORD rmask,WORD gmask,WORD bmask);
	void SetPixelFormat(RSPIXELFORMAT p);
	void SetClipRect(RECT &rt);
	
	int GetHeight();

private:
	RECT m_rect;
	RSFont *m_efont,*m_hfont;

};

#endif // !defined(AFX_RSCONSOLERENDERER_H__2009E2F9_844E_4677_BE04_56A4033AA1C0__INCLUDED_)
