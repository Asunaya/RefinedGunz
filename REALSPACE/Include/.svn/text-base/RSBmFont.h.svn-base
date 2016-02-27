/**
	RSBmFont.h
	----------

	Bitmap Font for RealSpace

	Programming by Joongpil Cho, Na Ja Young
	All copyright (c) MAIET entertainment
*/
#include "RSFont.h"
#include <memory.h>

#ifndef __RSBMFONT_HEADER__
#define __RSBMFONT_HEADER__


// RsBmFont class
// version 2
// ALPHA/KOREAN support

class RSBmFont {
private:
	RECT		m_rcClip;		// 클립핑 영역
	RealSpace*	m_pRS;
	RSFont*		m_pKorFnt;		// 국문 폰트
	RSFont*		m_pEngFnt;		// 영문 폰트

	int			GetKsIndex(BYTE fstByte, BYTE sndByte){
		return (((int)fstByte  - 0xb0) * 94 + (int)sndByte - 0xa1);
	}
public:
	RSBmFont(RealSpace *pRS);
	virtual	~RSBmFont();

	// 만일 szKorFontName에 NULL을 넣으면 영문 출력 전용으로...
	BOOL		Open(const char *szEngFontName, const char *szKorFontName = NULL);
	void		Close();
	
	// 만일 0이면 영문폰트의 높이를... 1이면 한글폰트의 높이를 리턴한다.
	int			GetHeight(int nIndex = 0){
		if(nIndex==0) return m_pEngFnt->GetHeight(); else return m_pKorFnt->GetHeight();
	}
	
	int			GetMaxHeight() { return max(m_pEngFnt?m_pEngFnt->GetHeight():0,m_pKorFnt?m_pKorFnt->GetHeight():0); }
	
	// return value : width of the drawn character.
	void		Puts( LPBYTE pBuffer, long lPitch, int nX, int nY, DWORD nColor, char *szMessage );
	// return value : 출력된 캐릭터 수(byte로...)
	int			PutsWithClip( LPBYTE pBuffer, long lPitch, int nX, int nY, DWORD nColor, char *szMessage );

/*
	//'\n'이 포함되어 있는 텍스트를 출력한다.
	// 반환값은 지금까지 출력된 텍스트의 인덱스
	int			TextWithClip(LPBYTE pBuffer, long lPitch, int nX, int nY, DWORD nColor, char *szMessage);
*/

	// If it fails, returns -1
	int			GetStringWidth( char *szMessage );
	int			GetPossibleCharacterCount( int nWidth, const char *string );

	void		SetClipRect(int nLeft, int nTop, int nRight, int nBottom){
		RECT rc = {nLeft, nTop, nRight, nBottom};
		SetClipRect(rc);
	}

	//현재 그려져 있는 텍스트에는 아무런 효력이 없다.
	void		SetClipRect(RECT rc){ memcpy(&m_rcClip,&rc,sizeof(RECT)); }
	void		GetClipRect(RECT* rc){ memcpy(rc, &m_rcClip, sizeof(RECT)); }
};


#endif // __RSFONT_HEADER__

/////////////////////////////////////////////////////////////////////////