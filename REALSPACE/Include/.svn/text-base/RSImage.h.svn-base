/*
	RSImage.h
	---------

	RealSpace Image Support Header

	Programming by Joongpil Cho
	All copyright (c) 1998, MAIET entertainment, inc
*/
#ifndef __RSIMAGE_HEADER__
#define __RSIMAGE_HEADER__

#include "RsTypes.h"

//#define __LOG_ENABLE__
#ifdef __LOG_ENABLE__
#include "RsDebug.h"
#endif//

class RSImage {
private:		
	int m_nSize;
	int m_nWidth;
	int m_nHeight;
	unsigned short *m_pBuffer;
	RECT	m_Rect;

	/* for tile output */
	void FastBlt( LPBYTE pDst, WORD nDstX, WORD nDstY, WORD nPitchByte, WORD nSrcX, WORD nSrcY, WORD nSrcWidth, WORD nSrcHeight );
	/* for sprite output */
	void FastBlt0( LPBYTE pDst, WORD nDstX, WORD nDstY, WORD nPitchByte, WORD nSrcX, WORD nSrcY, unsigned short uCol, WORD nSrcWidth, WORD nSrcHeight );
	// alpha functions
	void SlowBltAlpha555( LPBYTE pDst, WORD nDstX, WORD nDstY, WORD nPitchByte, WORD nSrcX, WORD nSrcY, 
							WORD nSrcWidth, WORD nSrcHeight, BYTE Oppacity );
	void SlowBlt0Alpha555( LPBYTE pDst, WORD nDstX, WORD nDstY, WORD nPitchByte, WORD nSrcX, WORD nSrcY, 
							unsigned short uCol, WORD nSrcWidth, WORD nSrcHeight, BYTE Oppacity );
	void SlowBltAlpha565( LPBYTE pDst, WORD nDstX, WORD nDstY, WORD nPitchByte, WORD nSrcX, WORD nSrcY, 
							WORD nSrcWidth, WORD nSrcHeight, BYTE Oppacity );
	void SlowBlt0Alpha565( LPBYTE pDst, WORD nDstX, WORD nDstY, WORD nPitchByte, WORD nSrcX, WORD nSrcY, 
							unsigned short uCol, WORD nSrcWidth, WORD nSrcHeight, BYTE Oppacity );
	void FastBltAlpha( LPBYTE pDst, WORD nDstX, WORD nDstY, WORD nPitchByte, WORD nSrcX, WORD nSrcY, 
							WORD nSrcWidth, WORD nSrcHeight);
	void FastBlt0Alpha( LPBYTE pDst, WORD nDstX, WORD nDstY, WORD nPitchByte, WORD nSrcX, WORD nSrcY, 
							unsigned short uCol, WORD nSrcWidth, WORD nSrcHeight);
public:
	RSImage();
	~RSImage();

	BOOL Open( RSMaterialList *ml, const char *szName );
	BOOL Open( const char *szPath );						// ¡÷¿« : 24BIT ONLY
	void Close();

	int GetWidth(void){ return m_nWidth; }
	int GetHeight(void){ return m_nHeight; }

	void SetClipRect(RECT &rect) { m_Rect=rect; }

	unsigned short *GetBuffer(){ return m_pBuffer; }
	
	BOOL Create( int nWidth, int nHeight ){
		m_nWidth = nWidth; m_nHeight = nHeight;
		m_nSize = m_nWidth * m_nHeight << 1; m_pBuffer = (unsigned short *) malloc( m_nSize );
		if( m_pBuffer ) return TRUE; else return FALSE;
	}

	void FillColor( unsigned short data ){ 
		int cnt = m_nWidth * m_nHeight;
		for( int i = 0; i < cnt; i ++ ) m_pBuffer[i] = data;
	}

	void Draw( LPBYTE pBuffer, int nX, int nY, int lPitch);
	void DrawSprite( LPBYTE pBuffer, int nX, int nY, int lPitch, DWORD sCol);

	/* Clipping Version - non 640 x 480 version */	
	void DrawWithClipRect( LPBYTE pBuffer, int nX, int nY, int lPitch, RECT& r, int nTargetWidth, int nTargetHeight );
	/* Sprite Display Output Clipping Version - non 640 x 480 version */
	void DrawSpriteWithClipRect( LPBYTE pBuffer, int nX, int nY, int lPitch, DWORD sCol, RECT &r, int nTargetWidth, int nTargetHeight );

	void DrawAlpha64( LPBYTE pBuffer, int nX, int nY, int lPitch, BYTE Oppacity );
	void DrawSpriteAlpha64( LPBYTE pBuffer, int nX, int nY, int lPitch, DWORD sCol, BYTE Oppacity );
	void DrawFastAlpha64( LPBYTE pBuffer, int nX, int nY, int lPitch );
	void DrawFastSpriteAlpha64( LPBYTE pBuffer, int nX, int nY, int lPitch, DWORD sCol);
};

typedef RSImage *PRSIMAGE;

class RSAnimation {
private:
	PRSIMAGE*	m_pImages;
	int			m_nIndex;	// Current Index Frame
	int			m_nCount;	// Frames
	int			m_nSpeed;	// Animation Speed
	RECT		m_Rect;

public:
	RSAnimation();
	~RSAnimation();

	BOOL Open(RSMaterialList *ml, const char *szName);
	void Close();

	void GetNextFrame(BOOL bLoop = FALSE){
		if(m_nIndex < (m_nCount-1)){
			m_nIndex++;
		} else {
			if( bLoop ){
				m_nIndex = 0;
			}
		}
	}
	int GetSpeed(){ return m_nSpeed; }
	void SetClipRect(RECT &rect) { m_Rect=rect;for(int i=0;i<m_nCount;i++) m_pImages[m_nIndex]->SetClipRect(rect); }

	void Draw( LPBYTE pBuffer, int nX, int nY, int lPitch ){
		m_pImages[m_nIndex]->Draw(pBuffer, nX, nY, lPitch );
	}

	void DrawSprite( LPBYTE pBuffer, int nX, int nY, int lPitch, DWORD sCol ){
		m_pImages[m_nIndex]->DrawSprite(pBuffer, nX, nY, lPitch, sCol);
	}

};



#endif // __RSIMAGE_HEADER__

//////////////////////////////////////////////////////////////////