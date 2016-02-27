// RSFont.h: interface for the RSFont class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __RSFONT_HEADER__
#define __RSFONT_HEADER__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <windows.h>
#include <rstypes.h>

#define RSFONTDATAHEADER_HEADERID	6666
#define RSFONTDATAHEADER_BUILD		2

struct RSFONTDATA
{
	int Width;
	BYTE* data;
};


struct RSFONTDATAHEADER
{
	int HeaderID;
	int Build;
	int nFontCount;
	int nBitsPerPixel;
};

// RSFONT class
class RSFont  
{

private:

	RSFONTDATA *mFontArray;
	int mFontCount,mHeight,mBPP;
	int m_lPitch;
	DWORD mColor;

public:

	RSFont();
	virtual		~RSFont();

	BOOL		New(int nFontCount,int BPP=1);
	BOOL		Open(const char*);
	BOOL		Save(const char*);
	BOOL		Destroy();
	
	void		SetColor(DWORD color)		{ mColor=color; }
	void		SetlPitch(int lPitch)		{ m_lPitch=lPitch; }
	void		SetHeight( int nHeight )	{ mHeight = nHeight;	}
	int			GetHeight()					{ return mHeight;		}
	int			GetWidth(int i)				{ return (i>=0)?((i<mFontCount)?
												(mFontArray[i].Width):NULL):NULL;	}
	int			GetFontCount()				{ return mFontCount;	}
	int			GetBPP()					{ return mBPP;			}
	int			GetByteSize(int Width);
	
	void		SetRSFONTDATA(int,RSFONTDATA*);
	RSFONTDATA	*GetPRSFONTDATA(int i)		{ return (i>=0)?((i<mFontCount)?
												(mFontArray+i):NULL):NULL;		}
	
	void		Draw(int i,void *pscreen);

	//Clipping을 수행하는 함수
	//nSkipX	: 앞에서 얼마만큼 안 찍을 것인가?
	//nLenX		: 찍을 가로의 길이
	//nSkipY	: 위에서 얼마만큼 안 찍을 것인가?
	//nLenY		: 찍을 세로의 길이

	void		Draw(int i,void *pscreen,int nSkipX,int nLenX,int nSkipY,int nLenY);
};

#endif // __RSFONT_HEADER__

/////////////////////////////////////////////////////////////////////////