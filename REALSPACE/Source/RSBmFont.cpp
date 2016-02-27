/**
	RSBmFont.cpp
	------------

	Bitmap Font for RealSpace

	Programming by Joongpil Cho, Jayoung Na
	All copyright (c) MAIET entertainment
*/
#include <windows.h>
#include <stdio.h>

#include "RealSpace.h"
#include "RSBmFont.h"
#include "RSDebug.h"
#include "RSD3D.h"

/////////////////////////////////////////////////////////////////////////
// Constructors & Destructors
/////////////////////////////////////////////////////////////////////////

RSBmFont::RSBmFont(RealSpace *pRS)
{
	SetClipRect(0,0,640,480);

	m_pKorFnt	= NULL;
	m_pEngFnt	= NULL;
	m_pRS		= pRS;		// 절대 pRS가 NULL이면 안된다.
}

RSBmFont::~RSBmFont()
{
	Close();
}

/////////////////////////////////////////////////////////////////////////
// Methods
/////////////////////////////////////////////////////////////////////////

/*
void RSBmFont::SetPixelFormat(WORD rmask,WORD gmask,WORD bmask)
{
	m_pEngFnt->SetPixelFormat(rmask,gmask,bmask);
	if(m_pKorFnt) m_pKorFnt->SetPixelFormat(rmask,gmask,bmask);
}
*/// modified by dubble for support 32bit display modes.
/*
void RSBmFont::SetPixelFormat(RSPIXELFORMAT pixelformat)
{
	m_pEngFnt->SetPixelFormat(pixelformat);
	if(m_pKorFnt) m_pKorFnt->SetPixelFormat(pixelformat);
}
*/
BOOL RSBmFont::Open(const char *szEngFontName, const char *szKorFontName)
{
	Close();
		
	m_pEngFnt = new RSFont();
	if(m_pEngFnt->Open(szEngFontName) == FALSE){
		return FALSE;
	}

	if(szKorFontName != NULL){
		m_pKorFnt = new RSFont();
		if(m_pKorFnt->Open(szKorFontName) == FALSE){
			return FALSE;
		}
	}

//	SetPixelFormat(RSPixelFormat);
	/*
	switch(RSPixelFormat){
	case RS_PIXELFORMAT_565: SetPixelFormat(0xf800,0x07e0,0x001f);break;
	case RS_PIXELFORMAT_555: SetPixelFormat(0x7c00,0x03e0,0x001f);break;
	}
	*/
		
	return TRUE;
}

void RSBmFont::Close()
{
	if(m_pKorFnt){
		m_pKorFnt->Destroy();
		delete m_pKorFnt;
		m_pKorFnt = NULL;
	}

	if(m_pEngFnt){
		m_pEngFnt->Destroy();
		delete m_pEngFnt;
		m_pEngFnt = NULL;
	}
}

void RSBmFont::Puts( LPBYTE pTarget, long lPitch, int nX, int nY, DWORD color, char *string )
{
	int Bpp=RSPixelFormat==RS_PIXELFORMAT_X888 ? 4 : 2;
	if(m_pEngFnt == NULL) return;

	m_pEngFnt->SetColor(color);
	m_pEngFnt->SetlPitch(lPitch);

	if(m_pKorFnt){
		m_pKorFnt->SetColor(color);
		m_pKorFnt->SetlPitch(lPitch);
	}

	int i,nLength=strlen(string);

	for(i=0;i<nLength;i++)
	{
		if((unsigned char)string[i]>127)	//한글
		{
			if(m_pKorFnt)
			{
				if(nY+m_pKorFnt->GetHeight()<0) return;
				WORD hcode=GetKsIndex(((BYTE)string[i]),((BYTE)string[i+1]));
				m_pKorFnt->Draw(hcode,pTarget+nY*lPitch+nX*Bpp);
				nX += m_pKorFnt->GetWidth(0);
				i++;
			}
		}
		else								//영문
		{
			if(nY+m_pEngFnt->GetHeight()<0) return;
			m_pEngFnt->Draw(string[i],pTarget+nY*lPitch+nX*Bpp);
			nX += m_pEngFnt->GetWidth(string[i]);
		}
	}
}

int	RSBmFont::GetPossibleCharacterCount( int nWidth, const char *string )
{
	int ret = 0, nLength, i;

	nLength = strlen(string);
	for(i=0;i<nLength;i++){
		if((unsigned char)string[i]>127){		// 한글
			if(m_pKorFnt){
				BYTE hi=(BYTE)string[i];
				BYTE lo=(BYTE)string[i+1];
				WORD hcode=GetKsIndex(hi,lo);
				ret += m_pKorFnt->GetWidth(hcode);
			}
			i++;
		} else {								// 영문
			ret += m_pEngFnt->GetWidth((int)(unsigned char)string[i]);
		}
		if(nWidth<ret)
			return i;
	}
	return nLength;
}

int	RSBmFont::GetStringWidth( char *string )
{
	int ret = 0, nLength, i;

	nLength = strlen(string);
	for(i=0;i<nLength;i++){
		if((unsigned char)string[i]>127){		// 한글
			if(m_pKorFnt){
				BYTE hi=(BYTE)string[i];
				BYTE lo=(BYTE)string[i+1];
				WORD hcode=GetKsIndex(hi,lo);
				ret += m_pKorFnt->GetWidth(hcode);
			}
			i++;
		} else {								// 영문
			ret += m_pEngFnt->GetWidth((int)(unsigned char)string[i]);
		}
	}
	return ret;
}

int RSBmFont::PutsWithClip( LPBYTE pTarget, long lPitch, int nX, int nY, DWORD color, char *string )
{
	int Bpp=RSPixelFormat==RS_PIXELFORMAT_X888 ? 4 : 2;

	int nTSkipX, nTLenX, nTSkipY = 0, nTLenY1 = -1, nTLenY2 = -1, nTemp, nTotalLength = 0;
	int nSkipX = 0, nLenX = 0;
	bool binit_k = false, binit_e = false;

	if(m_pEngFnt == NULL) return -1;

	if(nX > m_rcClip.right || nY > m_rcClip.bottom) return (0);

	m_pEngFnt->SetColor(color);
	m_pEngFnt->SetlPitch(lPitch);

	if(m_pKorFnt){
		m_pKorFnt->SetColor(color);
		m_pKorFnt->SetlPitch(lPitch);
	}

	nTSkipX = max(m_rcClip.left - nX, 0);
	nTemp = nX + GetStringWidth(string);
	nTLenX = GetStringWidth(string) - max(nTemp - m_rcClip.right, 0);

	nSkipX = nTSkipX;
	nLenX = nTLenX;
	int i,nLength=strlen(string);

	nTemp = 0;

	for(i=0;i<nLength;i++)
	{
		if((unsigned char)string[i]>127)	//한글
		{
			if(m_pKorFnt)
			{
				int yoffset = nY+(GetMaxHeight()-m_pKorFnt->GetHeight()+1)/2;
				nTSkipY = max(m_rcClip.top - yoffset, 0);

				if(!binit_k)
				{
					if(yoffset+m_pKorFnt->GetHeight() < m_rcClip.top) return nTotalLength;
					nTLenY1 = m_pKorFnt->GetHeight() - max(yoffset + m_pKorFnt->GetHeight() - m_rcClip.bottom, 0);
					binit_k = true;
				}

				nSkipX = 0; nLenX = -1;
				
				if(nTSkipX > 0)
				{
					if(nTSkipX >= m_pKorFnt->GetWidth(0))
					{
						nTSkipX -= m_pKorFnt->GetWidth(0);
						nX += m_pKorFnt->GetWidth(0);
						nTLenX -= m_pKorFnt->GetWidth(0);
						i++;
						continue;
					}
					else 
					{
						nSkipX = nTSkipX;
						nTSkipX = 0;
					}
				}

				if(nTLenX > m_pKorFnt->GetWidth(0)){
					nLenX = m_pKorFnt->GetWidth(0);
					nTLenX -= m_pKorFnt->GetWidth(0);
				} else {
					nLenX = nTLenX;
					nTLenX = 0;
					if(nLenX <= 0) return nTotalLength;
				}

				WORD hcode=GetKsIndex(((BYTE)string[i]),((BYTE)string[i+1]));
				m_pKorFnt->Draw(hcode,pTarget+yoffset*lPitch+(nX+nSkipX)*Bpp,nSkipX,nLenX,nTSkipY,nTLenY1);
				nX += m_pKorFnt->GetWidth(0);
				nTotalLength++;
				i++;
			}
		}
		else								//영문
		{
			nSkipX = 0;

			int yoffset = nY+(GetMaxHeight()-m_pEngFnt->GetHeight()+1)/2;
			nTSkipY = max(m_rcClip.top - yoffset, 0);

			if(!binit_e)
			{
				if(yoffset+m_pEngFnt->GetHeight()<m_rcClip.top){
					return nTotalLength;
				}
				nTLenY2 = m_pEngFnt->GetHeight() - max(yoffset + m_pEngFnt->GetHeight() - m_rcClip.bottom, 0);
				binit_e = true;
			}
			
			if(nTSkipX > 0)
			{
				if(m_pEngFnt->GetWidth(string[i]) <= nTSkipX)
				{
					nTSkipX -= m_pEngFnt->GetWidth(string[i]);
					nX += m_pEngFnt->GetWidth(string[i]);
					nTLenX -= m_pEngFnt->GetWidth(string[i]);
					continue;
				}
				else 
				{
					nSkipX = nTSkipX;
					nTSkipX = 0;
				}
			}

			if(nTLenX > m_pEngFnt->GetWidth(string[i])){
				nLenX = m_pEngFnt->GetWidth(string[i]);
				nTLenX -= m_pEngFnt->GetWidth(string[i]);
			} else {
				nLenX = nTLenX;
				nTLenX = 0;
				if(nLenX <= 0) return nTotalLength;
			}

			m_pEngFnt->Draw(string[i],pTarget+yoffset*lPitch+(nX+nSkipX)*Bpp,nSkipX,nLenX,nTSkipY,nTLenY2);
			nTotalLength++;
			nX += m_pEngFnt->GetWidth(string[i]);
		}
	}
	return nTotalLength;
}

/*	
	다 찍는 것이 아니면 다음줄로 스킵. 만일 y축으로 짤리면 그리지 않는다.
	일단 바뻐서 대충 만들어서 매우 느릴것으로 생각됨.
*/
// 쓰는곳이 없고 클리핑이 제대로 처리안됨. 일단 봉인했음.
/*
int RSBmFont::TextWithClip(LPBYTE pTarget, long lPitch, int nX, int nY, DWORD color, char *string)
{
	int Bpp=RSPixelFormat==RS_PIXELFORMAT_X888 ? 4 : 2;
	int sX = nX;

	if(m_pEngFnt == NULL) return 0;

	m_pEngFnt->SetColor(color);
	m_pEngFnt->SetlPitch(lPitch);

	if(m_pKorFnt){
		m_pKorFnt->SetColor(color);
		m_pKorFnt->SetlPitch(lPitch);
	}

	int i,nLength=strlen(string);

	for(i=0;i<nLength;i++)
	{
//		if((unsigned char)string[i]>127)	//한글
		if(IsDBCSLeadByteEx(949,(unsigned char)string[i]))	//한글
		{
			if(m_pKorFnt)
			{
				if(nY+m_pKorFnt->GetHeight()<m_rcClip.top) return i;

				if(nX+m_pKorFnt->GetWidth(string[i])>=m_rcClip.right){
					nX = sX;
					nY += m_pKorFnt->GetHeight();
					if(i>0)i--;
					if(nY + m_pEngFnt->GetHeight()>m_rcClip.bottom) return i;
					continue;
				}else{
					WORD hcode=GetKsIndex(((BYTE)string[i]),((BYTE)string[i+1]));
					m_pKorFnt->Draw(hcode,(pTarget+nY*lPitch+nX*Bpp));
					nX += m_pKorFnt->GetWidth(0);
					i++;
				}
			}
		}
		else											//영문
		{
			if(string[i] == '\n'){
				nX = sX;
				nY += m_pEngFnt->GetHeight();
				if(nY + m_pEngFnt->GetHeight() > m_rcClip.bottom) return i;
			}
			if(string[i] == '\t') nX += (m_pEngFnt->GetWidth(string[i]) * 4);

			if(nY+m_pEngFnt->GetHeight()<m_rcClip.top) return i;

			if(nX+m_pEngFnt->GetWidth(string[i])>=m_rcClip.right){
				nX = sX;
				nY += m_pEngFnt->GetHeight();
				if(i>0)i--;
				if(nY + m_pEngFnt->GetHeight()>m_rcClip.bottom) return i;
				continue;
			}else{
				m_pEngFnt->Draw(string[i],(pTarget+nY*lPitch+nX*Bpp));
				nX += m_pEngFnt->GetWidth(string[i]);
			}
		}
	}

	return i;
}
*/