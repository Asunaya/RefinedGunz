// RSConsoleRenderer.cpp: coded by dubble. since 99-8-13

#include "RSFont.h"
#include "RSD3D.h"
#include "RSConsoleRenderer.h"

RSConsoleRenderer::RSConsoleRenderer()
{
	m_efont=NULL;
	m_hfont=NULL;
}

RSConsoleRenderer::~RSConsoleRenderer()
{
	if(m_efont) delete m_efont;
	if(m_hfont) delete m_hfont;
}

RSConsoleRenderer::Create(const char *englishfontname,const char *hangulfontname)
{
	m_efont=new RSFont;
	if(!m_efont->Open(englishfontname)) { delete m_efont;m_efont=NULL;return false; }
	if(hangulfontname)
	{
		m_hfont=new RSFont;
		if(!m_hfont->Open(hangulfontname)) { delete m_efont;delete m_hfont; m_efont=NULL;m_hfont=NULL;return false; }
	}
	return true;
}
/*
void RSConsoleRenderer::SetPixelFormat(RSPIXELFORMAT pixelformat)
{
	m_efont->SetPixelFormat(pixelformat);
	if(m_hfont) m_hfont->SetPixelFormat(pixelformat);
}
*/
void RSConsoleRenderer::TextOut(LPBYTE pTarget,int lPitch,int nX,int nY,DWORD color,char *string,...)
{
	if(!m_efont) return;

#define GetKsIndex( fstByte, sndByte )	(((int)fstByte  - 0xb0) * 94 + (int)sndByte - 0xa1)

	char buffer[_MAX_PATH];
	va_list argptr;

	va_start( argptr, string );
	vsprintf( buffer, string, argptr );
	va_end( argptr );

	m_efont->SetColor(color);
	m_efont->SetlPitch(lPitch);
	if(m_hfont)
	{
		m_hfont->SetColor(color);
		m_hfont->SetlPitch(lPitch);
	}

	int Bpp=RSPixelFormat==RS_PIXELFORMAT_X888 ? 4 : 2;
	int i,nLength=strlen(buffer);
	for(i=0;i<nLength;i++)
	{
		if((unsigned char)buffer[i]>127)
		{
			if(m_hfont)
			{
				BYTE hi=(BYTE)buffer[i];
				BYTE lo=(BYTE)buffer[i+1];
				WORD hcode=GetKsIndex(hi,lo);
				m_hfont->Draw(hcode,((BYTE*)pTarget)+nY*lPitch+nX*Bpp);
			}
			nX+=m_hfont->GetWidth(0);
			i++;
		}
		else
		{
			m_efont->Draw(buffer[i],((BYTE*)pTarget)+nY*lPitch+nX*Bpp);
			nX+=m_efont->GetWidth(buffer[i]);
		}
	}
}

int RSConsoleRenderer::GetHeight()
{
	if(!m_efont) return 0;
	if(m_hfont)
		return max(m_efont->GetHeight(),m_hfont->GetHeight());
	else
		return m_efont->GetHeight();
}