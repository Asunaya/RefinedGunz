/*	RSImage.cpp	Programming by Joongpil Cho and continuing by dubble.
	All copyright (c) 1998, MAIET entertainment, inc	*/

#include "Rutils.h"
#include "RealSpace.h"
#include "RSImage.h"
#include "RSDebug.h"
#include "RSMaterialList.h"

/* fast BitBlt for tile output */

void RSImage::FastBlt( LPBYTE pDst, WORD nDstX, WORD nDstY, WORD nPitchByte, WORD nSrcX, WORD nSrcY, WORD nSrcWidth, WORD nSrcHeight )
{
	if( nSrcWidth <= 0 || nSrcHeight <= 0 || pDst == NULL ) return;
	
	DWORD pDestAddress = (DWORD) pDst + (nDstX << 1) + nDstY * nPitchByte;
	DWORD pSrcAddress = (DWORD) m_pBuffer + (nSrcX << 1) + nSrcY * (m_nWidth << 1);	
	DWORD nDestGap = nPitchByte - (nSrcWidth << 1);	
	DWORD nSrcGap = (m_nWidth - nSrcWidth) << 1;	
	DWORD nWidthCount = nSrcWidth >> 1;	
	DWORD nLeft = (nSrcWidth << 1)%4;
	DWORD dwSrcHeight = (DWORD)nSrcHeight;

	__asm{
		mov edi, pDestAddress
		mov esi, pSrcAddress		
		mov ecx, dwSrcHeight
LOOPfb:
		push ecx
		mov ecx, nLeft
		rep movsb

		mov ecx, nWidthCount
		rep movsd
		add edi, nDestGap
		add esi, nSrcGap
		pop ecx
		dec ecx
		jnz LOOPfb
	}
}

/* fast BitBlt for sprite output */
void RSImage::FastBlt0( LPBYTE pDst, WORD nDstX, WORD nDstY, WORD nPitchByte, WORD nSrcX, WORD nSrcY, unsigned short uCol, WORD nSrcWidth, WORD nSrcHeight )
{
	if( nSrcWidth <= 0 || nSrcHeight <= 0 || pDst == NULL ) return;

	// movsw를 사용하여 Sprite 전송
	DWORD pDestAddress = (DWORD) pDst + (nDstX << 1) + nDstY * nPitchByte;
	DWORD pSrcAddress = (DWORD) m_pBuffer + (nSrcX << 1) + nSrcY * (m_nWidth << 1);
	DWORD nDestGap = nPitchByte - (nSrcWidth<<1);
	DWORD nSrcGap = (m_nWidth - nSrcWidth) << 1;
	DWORD nWidthCount = (DWORD)(nSrcWidth<<1);
	DWORD dwSrcHeight = (DWORD)nSrcHeight;	
		
	__asm{
		cld
		mov edi, pDestAddress
		mov esi, pSrcAddress
		mov ecx, dwSrcHeight
loopHeight:
		push ecx
		mov ecx, nWidthCount
loopWidth:
		mov ax, word ptr [esi]
		cmp ax, uCol
		je labelzejmp
		movsw
		jmp labeljmp
labelzejmp:
		add esi, 2
		add edi, 2
labeljmp:
		dec ecx
		loop loopWidth

		add edi, nDestGap
		add esi, nSrcGap

		pop ecx
		dec ecx
		jnz loopHeight
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RSImage::RSImage()
{
	m_pBuffer = NULL;
	m_nWidth = m_nHeight = m_nSize = -1;
	m_Rect.top=RSViewport.y1;m_Rect.bottom=RSViewport.y2;
	m_Rect.left=RSViewport.x1;m_Rect.right=RSViewport.x2;
}

RSImage::~RSImage()
{
	Close();
}

//////////////////////////////////////////////////////////////////////
// Methods
//////////////////////////////////////////////////////////////////////

BOOL RSImage::Open( RSMaterialList *ml, const char *szName )
{
	RSTexture *pTex;
	LPBYTE pTemp;
	int nIndex, i, nSize;
	unsigned short *p;

	Close();
	if( ml == NULL || szName == NULL ) return FALSE;
	
	nIndex = ml->GetTextureIndex(szName);
	if( nIndex == -1 ) return FALSE;
	if( !ml->Lock( nIndex ) ) return FALSE;
	
	pTex = ml->GetTexture( nIndex );
	if( pTex == NULL ){
		ml->Unlock( nIndex );
		return FALSE;
	}

	pTemp = (LPBYTE)pTex->Data;
	if( pTemp == NULL ){
		ml->Unlock( nIndex );
		return FALSE;
	}

	m_nWidth = pTex->x;
	m_nHeight = pTex->y;
	m_nSize = (m_nWidth * m_nHeight) << 1;
	
	m_pBuffer = (unsigned short *) malloc( m_nSize );
	if( !m_pBuffer ){
		ml->Unlock( nIndex );
		return FALSE;
	}

	p = m_pBuffer;
	nSize = pTex->x*pTex->y;
	for( i = 0; i < nSize; i ++ ){		
		*p = RS_RGB( pTemp[i*3+2], pTemp[i*3+1], pTemp[i*3] );
		p++;
	}
	ml->Unlock( nIndex );
	
	return TRUE;
}

BOOL RSImage::Open(const char *szPath)
{
	RSTexture *pTex = NULL;
	LPBYTE pTemp;
	int i, nSize;
	unsigned short *p;

	Close();
	if(szPath == NULL) return FALSE;

	pTex = new RSTexture;
	if(pTex == NULL) return FALSE;
	if(pTex->CreateFromBMP(szPath) == FALSE) return FALSE;

	pTemp = (LPBYTE)pTex->Data;
	if( pTemp == NULL ){
		delete pTex;
		return FALSE;
	}

	m_nWidth = pTex->x;
	m_nHeight = pTex->y;
	m_nSize = (m_nWidth * m_nHeight) << 1;

	m_pBuffer = (unsigned short *) malloc( m_nSize );
	if( !m_pBuffer ){
		delete pTex;
		return FALSE;
	}

	p = m_pBuffer;
	nSize = pTex->x*pTex->y;
	for( i = 0; i < nSize; i ++ ){
		*p = RS_RGB( pTemp[i*3+2], pTemp[i*3+1], pTemp[i*3] );
		p++;
	}

	return TRUE;
}

void RSImage::Close()
{
	if( m_pBuffer ){
		free( m_pBuffer );
		m_pBuffer = NULL;
	}
}

// added by dubble, 99/2/1

void RSImage::SlowBltAlpha555( LPBYTE pDst, WORD nDstX, WORD nDstY, WORD nPitchByte, 
						WORD nSrcX, WORD nSrcY, WORD nSrcWidth, WORD nSrcHeight, BYTE Opacity )
{
	if( nSrcWidth <= 0 || nSrcHeight <= 0 || pDst == NULL ) return;
	
	DWORD pDestAddress = (DWORD) pDst + (nDstX << 1) + nDstY * nPitchByte;
	DWORD pSrcAddress = (DWORD) m_pBuffer + (nSrcX << 1) + nSrcY * (m_nWidth << 1);	
	DWORD nDestGap = nPitchByte - (nSrcWidth << 1);	
	DWORD nSrcGap = (m_nWidth - nSrcWidth) << 1;	
	DWORD dwSrcHeight = (DWORD)nSrcHeight;
	DWORD dwSrcWidth = (DWORD)nSrcWidth;
	WORD wOpacity=Opacity;
	WORD iwOpacity=255-Opacity;
	WORD i255=255;

	__asm{
		mov edi, pDestAddress
		mov esi, pSrcAddress		
		mov ecx, dwSrcHeight
LOOPfb:
		push ecx
		mov ecx, dwSrcWidth

LOOP2:
		xor ebx,ebx
		xor edx,edx

		mov ax,word ptr [esi]
		and eax,0x7c00
		mul wOpacity
		div i255
		and eax,0x7c00
		add ebx,eax

		mov ax,word ptr [esi]
		and eax,0x03e0
		mul wOpacity
		div i255
		and eax,0x03e0
		add ebx,eax

		mov ax,word ptr [esi]
		and eax,0x1f
		mul wOpacity
		div i255
		add ebx,eax

		mov ax,word ptr [edi]
		and eax,0x7c00
		mul iwOpacity
		div i255
		and eax,0x7c00
		add ebx,eax

		mov ax,word ptr [edi]
		and eax,0x03e0
		mul iwOpacity
		div i255
		and eax,0x03e0
		add ebx,eax

		mov ax,word ptr [edi]
		and eax,0x1f
		mul iwOpacity
		div i255
		add ebx,eax

		mov [edi],bx

		add esi,2
		add edi,2
		dec ecx
		jnz LOOP2

		add edi, nDestGap
		add esi, nSrcGap
		pop ecx
		dec ecx
		jnz LOOPfb
	}
}


void RSImage::SlowBlt0Alpha555( LPBYTE pDst, WORD nDstX, WORD nDstY, WORD nPitchByte, WORD nSrcX, WORD nSrcY, 
							   unsigned short uCol, WORD nSrcWidth, WORD nSrcHeight, BYTE Opacity )
{
	if( nSrcWidth <= 0 || nSrcHeight <= 0 || pDst == NULL ) return;
	
	DWORD pDestAddress = (DWORD) pDst + (nDstX << 1) + nDstY * nPitchByte;
	DWORD pSrcAddress = (DWORD) m_pBuffer + (nSrcX << 1) + nSrcY * (m_nWidth << 1);	
	DWORD nDestGap = nPitchByte - (nSrcWidth << 1);	
	DWORD nSrcGap = (m_nWidth - nSrcWidth) << 1;	
	DWORD dwSrcHeight = (DWORD)nSrcHeight;
	DWORD dwSrcWidth = (DWORD)nSrcWidth;
	WORD wOpacity=Opacity;
	WORD iwOpacity=255-Opacity;
	WORD i255=255;

	__asm{
		mov edi, pDestAddress
		mov esi, pSrcAddress		
		mov ecx, dwSrcHeight
LOOPfb:
		push ecx
		mov ecx, dwSrcWidth

LOOP2:
		xor ebx,ebx
		xor edx,edx

		mov ax,word ptr [esi]
		cmp ax, uCol
		je labelzejmp

		and eax,0x7c00
		mul wOpacity
		div i255
		and eax,0x7c00
		add ebx,eax

		mov ax,word ptr [esi]
		and eax,0x03e0
		mul wOpacity
		div i255
		and eax,0x03e0
		add ebx,eax

		mov ax,word ptr [esi]
		and eax,0x1f
		mul wOpacity
		div i255
		add ebx,eax

		mov ax,word ptr [edi]
		and eax,0x7c00
		mul iwOpacity
		div i255
		and eax,0x7c00
		add ebx,eax

		mov ax,word ptr [edi]
		and eax,0x03e0
		mul iwOpacity
		div i255
		and eax,0x03e0
		add ebx,eax

		mov ax,word ptr [edi]
		and eax,0x1f
		mul iwOpacity
		div i255
		add ebx,eax

		mov [edi],bx

labelzejmp:
		add esi,2
		add edi,2
		dec ecx
		jnz LOOP2

		add edi, nDestGap
		add esi, nSrcGap
		pop ecx
		dec ecx
		jnz LOOPfb
	}
}

//565
void RSImage::SlowBltAlpha565( LPBYTE pDst, WORD nDstX, WORD nDstY, WORD nPitchByte, 
						WORD nSrcX, WORD nSrcY, WORD nSrcWidth, WORD nSrcHeight, BYTE Opacity )
{
	if( nSrcWidth <= 0 || nSrcHeight <= 0 || pDst == NULL ) return;
	
	DWORD pDestAddress = (DWORD) pDst + (nDstX << 1) + nDstY * nPitchByte;
	DWORD pSrcAddress = (DWORD) m_pBuffer + (nSrcX << 1) + nSrcY * (m_nWidth << 1);	
	DWORD nDestGap = nPitchByte - (nSrcWidth << 1);	
	DWORD nSrcGap = (m_nWidth - nSrcWidth) << 1;	
	DWORD dwSrcHeight = (DWORD)nSrcHeight;
	DWORD dwSrcWidth = (DWORD)nSrcWidth;
	WORD wOpacity=Opacity;
	WORD iwOpacity=255-Opacity;
	WORD i255=255;

	__asm{
		mov edi, pDestAddress
		mov esi, pSrcAddress		
		mov ecx, dwSrcHeight
LOOPfb:
		push ecx
		mov ecx, dwSrcWidth

LOOP2:
		xor ebx,ebx
		xor edx,edx

		mov ax,word ptr [esi]
		and eax,0xf800
		mul wOpacity
		div i255
		and eax,0xf800
		add ebx,eax

		mov ax,word ptr [esi]
		and eax,0x07e0
		mul wOpacity
		div i255
		and eax,0x07e0
		add ebx,eax

		mov ax,word ptr [esi]
		and eax,0x1f
		mul wOpacity
		div i255
		add ebx,eax

		mov ax,word ptr [edi]
		and eax,0xf800
		mul iwOpacity
		div i255
		and eax,0xf800
		add ebx,eax

		mov ax,word ptr [edi]
		and eax,0x07e0
		mul iwOpacity
		div i255
		and eax,0x07e0
		add ebx,eax

		mov ax,word ptr [edi]
		and eax,0x1f
		mul iwOpacity
		div i255
		add ebx,eax

		mov [edi],bx

		add esi,2
		add edi,2
		dec ecx
		jnz LOOP2

		add edi, nDestGap
		add esi, nSrcGap
		pop ecx
		dec ecx
		jnz LOOPfb
	}
}


void RSImage::SlowBlt0Alpha565( LPBYTE pDst, WORD nDstX, WORD nDstY, WORD nPitchByte, WORD nSrcX, WORD nSrcY, 
							   unsigned short uCol, WORD nSrcWidth, WORD nSrcHeight, BYTE Opacity )
{
	if( nSrcWidth <= 0 || nSrcHeight <= 0 || pDst == NULL ) return;
	
	DWORD pDestAddress = (DWORD) pDst + (nDstX << 1) + nDstY * nPitchByte;
	DWORD pSrcAddress = (DWORD) m_pBuffer + (nSrcX << 1) + nSrcY * (m_nWidth << 1);	
	DWORD nDestGap = nPitchByte - (nSrcWidth << 1);	
	DWORD nSrcGap = (m_nWidth - nSrcWidth) << 1;	
	DWORD dwSrcHeight = (DWORD)nSrcHeight;
	DWORD dwSrcWidth = (DWORD)nSrcWidth;
	WORD wOpacity=Opacity;
	WORD iwOpacity=255-Opacity;
	WORD i255=255;

	__asm{
		mov edi, pDestAddress
		mov esi, pSrcAddress		
		mov ecx, dwSrcHeight
LOOPfb:
		push ecx
		mov ecx, dwSrcWidth

LOOP2:
		xor ebx,ebx
		xor edx,edx

		mov ax,word ptr [esi]
		cmp ax, uCol
		je labelzejmp

		and eax,0xf800
		mul wOpacity
		div i255
		and eax,0xf800
		add ebx,eax

		mov ax,word ptr [esi]
		and eax,0x07e0
		mul wOpacity
		div i255
		and eax,0x07e0
		add ebx,eax

		mov ax,word ptr [esi]
		and eax,0x1f
		mul wOpacity
		div i255
		add ebx,eax

		mov ax,word ptr [edi]
		and eax,0xf800
		mul iwOpacity
		div i255
		and eax,0xf800
		add ebx,eax

		mov ax,word ptr [edi]
		and eax,0x07e0
		mul iwOpacity
		div i255
		and eax,0x07e0
		add ebx,eax

		mov ax,word ptr [edi]
		and eax,0x1f
		mul iwOpacity
		div i255
		add ebx,eax

		mov [edi],bx

labelzejmp:
		add esi,2
		add edi,2
		dec ecx
		jnz LOOP2

		add edi, nDestGap
		add esi, nSrcGap
		pop ecx
		dec ecx
		jnz LOOPfb
	}
}

void RSImage::DrawAlpha64( LPBYTE pBuffer, int nX, int nY, int lPitch, BYTE Opacity )
{
	int w, h, temp, sx, sy, ncase;

	if( nX > 639 || nY > 479 ) return;
	
	temp = nX + m_nWidth; ncase = (nX < 0 && temp > 0);
	if( temp >= 640 ){ w = 640 - nX; }else if( ncase ){ w = m_nWidth + nX; }else{ w = m_nWidth; }
	if( nX < 0 && temp > 0 ){ sx = -nX; } else sx = 0;

	temp = nY + m_nHeight; ncase = (nY < 0 && temp > 0);
	if( temp >= 480 ){ h = 480 - nY; }else if( ncase ){ h = m_nHeight + nY; }else{ h = m_nHeight; }
	if( nY < 0 && temp > 0 ){ sy = -nY; } else sy = 0;

	switch(RSPixelFormat)
	{
	case RS_PIXELFORMAT_555:
		SlowBltAlpha555( pBuffer, min(640, max(0, nX)), min(480, max(0, nY)), lPitch, sx, sy, w, h, Opacity);
		break;
	case RS_PIXELFORMAT_565:
		SlowBltAlpha565( pBuffer, min(640, max(0, nX)), min(480, max(0, nY)), lPitch, sx, sy, w, h, Opacity);
		break;
	};
}

void RSImage::DrawSpriteAlpha64( LPBYTE pBuffer, int nX, int nY, int lPitch, DWORD sCol, BYTE Opacity )
{
	int w, h, temp, sx, sy, ncase;

	if( nX > 639 || nY > 479 ) return;
	
	temp = nX + m_nWidth; ncase = (nX < 0 && temp > 0);
	if( temp >= 640 ){ w = 640 - nX; }else if( ncase ){ w = m_nWidth + nX; }else{ w = m_nWidth; }
	if( nX < 0 && temp > 0 ){ sx = -nX; } else sx = 0;

	temp = nY + m_nHeight; ncase = (nY < 0 && temp > 0);
	if( temp >= 480 ){ h = 480 - nY; }else if( ncase ){ h = m_nHeight + nY; }else{ h = m_nHeight; }
	if( nY < 0 && temp > 0 ){ sy = -nY; } else sy = 0;
	
	switch(RSPixelFormat)
	{
	case RS_PIXELFORMAT_555:
		SlowBlt0Alpha555( pBuffer, min(640, max(0, nX)), min(480, max(0, nY)), lPitch, sx, sy, RGB322RGB16(sCol), w, h, Opacity);
		break;
	case RS_PIXELFORMAT_565:
		SlowBlt0Alpha565( pBuffer, min(640, max(0, nX)), min(480, max(0, nY)), lPitch, sx, sy, RGB322RGB16(sCol), w, h, Opacity);
		break;
	}
}


// fast alpha

void RSImage::FastBltAlpha( LPBYTE pDst, WORD nDstX, WORD nDstY, WORD nPitchByte, 
						WORD nSrcX, WORD nSrcY, WORD nSrcWidth, WORD nSrcHeight)
{
	if( nSrcWidth <= 0 || nSrcHeight <= 0 || pDst == NULL ) return;
	
	DWORD pDestAddress = (DWORD) pDst + (nDstX << 1) + nDstY * nPitchByte;
	DWORD pSrcAddress = (DWORD) m_pBuffer + (nSrcX << 1) + nSrcY * (m_nWidth << 1);	
	DWORD nDestGap = nPitchByte - (nSrcWidth << 1);	
	DWORD nSrcGap = (m_nWidth - nSrcWidth) << 1;	
	DWORD dwSrcHeight = (DWORD)nSrcHeight;
	DWORD dwSrcWidth = (DWORD)nSrcWidth;
	WORD Mask=(RSPixelFormat==RS_PIXELFORMAT_555)?0x7bde:0xf7de;

	__asm{
		mov edi, pDestAddress
		mov esi, pSrcAddress		
		mov ecx, dwSrcHeight
LOOPfb:
		push ecx
		mov ecx, dwSrcWidth

LOOP2:
		mov ax,word ptr [esi]
		and ax,Mask
		shr ax,1
		mov bx,word ptr [edi]
		and bx,Mask
		shr bx,1
		add ax,bx

		mov [edi],ax

		add esi,2
		add edi,2
		dec ecx
		jnz LOOP2

		add edi, nDestGap
		add esi, nSrcGap
		pop ecx
		dec ecx
		jnz LOOPfb
	}
}


void RSImage::FastBlt0Alpha( LPBYTE pDst, WORD nDstX, WORD nDstY, WORD nPitchByte, WORD nSrcX, WORD nSrcY, 
							   unsigned short uCol, WORD nSrcWidth, WORD nSrcHeight)
{
	if( nSrcWidth <= 0 || nSrcHeight <= 0 || pDst == NULL ) return;
	
	DWORD pDestAddress = (DWORD) pDst + (nDstX << 1) + nDstY * nPitchByte;
	DWORD pSrcAddress = (DWORD) m_pBuffer + (nSrcX << 1) + nSrcY * (m_nWidth << 1);	
	DWORD nDestGap = nPitchByte - (nSrcWidth << 1);	
	DWORD nSrcGap = (m_nWidth - nSrcWidth) << 1;	
	DWORD dwSrcHeight = (DWORD)nSrcHeight;
	DWORD dwSrcWidth = (DWORD)nSrcWidth;
	WORD Mask=(RSPixelFormat==RS_PIXELFORMAT_555)?0x7bde:0xf7de;

	__asm{
		mov edi, pDestAddress
		mov esi, pSrcAddress		
		mov ecx, dwSrcHeight
LOOPfb:
		push ecx
		mov ecx, dwSrcWidth

LOOP2:
		mov ax,word ptr [esi]
		cmp ax, uCol
		je labelzejmp
		and ax,Mask
		shr ax,1
		mov bx,word ptr [edi]
		and bx,Mask
		shr bx,1
		add ax,bx

		mov [edi],ax

labelzejmp:
		add esi,2
		add edi,2
		dec ecx
		jnz LOOP2

		add edi, nDestGap
		add esi, nSrcGap
		pop ecx
		dec ecx
		jnz LOOPfb
	}
}

void RSImage::DrawFastAlpha64( LPBYTE pBuffer, int nX, int nY, int lPitch)
{
	int w, h, temp, sx, sy, ncase;

	if( nX > 639 || nY > 479 ) return;
	
	temp = nX + m_nWidth; ncase = (nX < 0 && temp > 0);
	if( temp >= 640 ){ w = 640 - nX; }else if( ncase ){ w = m_nWidth + nX; }else{ w = m_nWidth; }
	if( nX < 0 && temp > 0 ){ sx = -nX; } else sx = 0;

	temp = nY + m_nHeight; ncase = (nY < 0 && temp > 0);
	if( temp >= 480 ){ h = 480 - nY; }else if( ncase ){ h = m_nHeight + nY; }else{ h = m_nHeight; }
	if( nY < 0 && temp > 0 ){ sy = -nY; } else sy = 0;
	FastBltAlpha( pBuffer, min(640, max(0, nX)), min(480, max(0, nY)), lPitch, sx, sy, w, h);
}

void RSImage::DrawFastSpriteAlpha64( LPBYTE pBuffer, int nX, int nY, int lPitch, DWORD sCol)
{
	int w, h, temp, sx, sy, ncase;

	if( nX > 639 || nY > 479 ) return;
	
	temp = nX + m_nWidth; ncase = (nX < 0 && temp > 0);
	if( temp >= 640 ){ w = 640 - nX; }else if( ncase ){ w = m_nWidth + nX; }else{ w = m_nWidth; }
	if( nX < 0 && temp > 0 ){ sx = -nX; } else sx = 0;

	temp = nY + m_nHeight; ncase = (nY < 0 && temp > 0);
	if( temp >= 480 ){ h = 480 - nY; }else if( ncase ){ h = m_nHeight + nY; }else{ h = m_nHeight; }
	if( nY < 0 && temp > 0 ){ sy = -nY; } else sy = 0;
	FastBlt0Alpha( pBuffer, min(640, max(0, nX)), min(480, max(0, nY)), lPitch, sx, sy, RGB322RGB16(sCol), w, h);
}


/**
	RSAnimation
*/

RSAnimation::RSAnimation()
{
	m_pImages = NULL;
}

RSAnimation::~RSAnimation()
{
	Close();
}

BOOL RSAnimation::Open(RSMaterialList *ml, const char *szName)
{
	RSMaterial *pMat;
	RSTexture *pTex;
	int nIndex, i, j;

	Close();
	
	if( ml == NULL || szName == NULL ) return FALSE;

	nIndex = ml->GetIndex(szName);
	if( nIndex == -1 ) return FALSE;
	
	if( !ml->LockMaterial(nIndex) ) return FALSE;
	pMat = ml->Get(nIndex);
	if( pMat == NULL ){
		ml->UnlockMaterial(nIndex);
		return FALSE;
	}
	m_nSpeed = pMat->AnimationSpeed;
	m_nCount = pMat->GetTextureCount();
	m_pImages = new PRSIMAGE[m_nCount];
		
	for( i = 0; i < m_nCount; i++ ){
		j = pMat->GetTextureIndex(i);
		pTex = ml->GetTexture(j);
		m_pImages[i] = new RSImage;
		m_pImages[i]->Open(ml, pTex->Name);
	}
	m_nIndex = 0;
	ml->UnlockMaterial(nIndex);

	return TRUE;
}

void RSAnimation::Close()
{
	int i;

	if(m_pImages != NULL && m_nCount > 0){
		for(i = 0; i < m_nCount; i++){
			m_pImages[i]->Close();
			delete m_pImages[i];
			m_pImages[i] = NULL;
		}
		delete []m_pImages;
		m_pImages = NULL;
	}
	m_nIndex = 0;
	m_nCount = 0;
}

void RSImage::DrawWithClipRect( LPBYTE pBuffer, int nX, int nY, int lPitch, RECT& r, int nTargetWidth, int nTargetHeight )
{
	int w, h, temp, sx, sy, ncase, tx, ty;

	if( nX > nTargetWidth-1 || nY > nTargetHeight-1 ) return;
	if( nX > r.right || nY > r.bottom ) return;
	if( nX + m_nWidth < r.left || nY + m_nHeight < r.top ) return;

	temp = nX + m_nWidth; ncase = (nX < 0 && temp > 0);
	if( temp >= nTargetWidth ){ w = nTargetWidth - nX; }else if( ncase ){ w = m_nWidth + nX; }else{ w = m_nWidth; }
	if( nX < 0 && temp > 0 ){ sx = -nX; } else sx = 0;

	temp = nY + m_nHeight; ncase = (nY < 0 && temp > 0);
	if( temp >= nTargetHeight ){ h = nTargetHeight - nY; }else if( ncase ){ h = m_nHeight + nY; }else{ h = m_nHeight; }
	if( nY < 0 && temp > 0 ){ sy = -nY; } else sy = 0;

	//tx, ty에 따라서 sx, sy를 얼만큼 씨프트를 시킬 것인가?

	tx = max(r.left,min(nTargetWidth, max(0, nX)));
	temp = r.left - min(nTargetWidth, max(0, nX));
	if(temp > 0) sx += temp;

	ty = max(r.top,min(nTargetHeight, max(0, nY)));
	temp = r.top - min(nTargetHeight, max(0, nY));
	if(temp > 0) sy += temp;

	w = min(w, r.right - r.left);
	h = min(h, r.bottom - r.top);
	
	FastBlt( pBuffer, tx, ty, lPitch, sx, sy, w, h);
}

void RSImage::DrawSpriteWithClipRect( LPBYTE pBuffer, int nX, int nY, int lPitch, DWORD sCol, RECT &r, int nTargetWidth, int nTargetHeight)
{
	int w, h, temp, sx, sy, ncase, tx, ty;

	if( nX > nTargetWidth-1 || nY > nTargetHeight-1 ) return;
	if( nX > r.right || nY > r.bottom ) return;
	if( nX + m_nWidth < r.left || nY + m_nHeight < r.top ) return;
	
	temp = nX + m_nWidth; ncase = (nX < 0 && temp > 0);
	if( temp >= nTargetWidth ){ w = nTargetWidth - nX; }else if( ncase ){ w = m_nWidth + nX; }else{ w = m_nWidth; }
	if( nX < 0 && temp > 0 ){ sx = -nX; } else sx = 0;

	temp = nY + m_nHeight; ncase = (nY < 0 && temp > 0);
	if( temp >= nTargetHeight ){ h = nTargetHeight - nY; }else if( ncase ){ h = m_nHeight + nY; }else{ h = m_nHeight; }
	if( nY < 0 && temp > 0 ){ sy = -nY; } else sy = 0;

	tx = max(r.left,min(nTargetWidth, max(0, nX)));
	temp = r.left - min(nTargetWidth, max(0, nX));
	if(temp > 0){ sx += temp; w -= temp; }

	ty = max(r.top,min(nTargetHeight, max(0, nY)));
	temp = r.top - min(nTargetHeight, max(0, nY));
	if(temp > 0){ sy += temp; h -= temp; }

	w = min(w, r.right - r.left);
	h = min(h, r.bottom - r.top);

	FastBlt0( pBuffer, tx, ty, lPitch, sx, sy, RGB322RGB16(sCol), w, h);
}

void RSImage::Draw( LPBYTE pBuffer, int nX, int nY, int lPitch){
	int w, h, temp, sx, sy, ncase;
	int nTargetWidth = m_Rect.right-m_Rect.left, nTargetHeight = m_Rect.bottom-m_Rect.top;

	if( nX > (nTargetWidth - 1) || nY > (nTargetHeight - 1) ) return;

	temp = nX + m_nWidth; ncase = (nX < 0 && temp > 0);
	if( temp >= nTargetWidth ){ w = nTargetWidth - nX; }else if( ncase ){ w = m_nWidth + nX; }else{ w = m_nWidth; }
	if( nX < 0 && temp > 0 ){ sx = -nX; } else sx = 0;

	temp = nY + m_nHeight; ncase = (nY < 0 && temp > 0);
	if( temp >= nTargetHeight ){ h = nTargetHeight - nY; }else if( ncase ){ h = m_nHeight + nY; }else{ h = m_nHeight; }
	if( nY < 0 && temp > 0 ){ sy = -nY; } else sy = 0;
	
	FastBlt( pBuffer, min(nTargetWidth, max(0, nX)), min(nTargetHeight, max(0, nY)), lPitch, sx, sy, w, h);
}

void RSImage::DrawSprite( LPBYTE pBuffer, int nX, int nY, int lPitch, DWORD sCol){
	int w, h, temp, sx, sy, ncase;
	int nTargetWidth = m_Rect.right-m_Rect.left, nTargetHeight = m_Rect.bottom-m_Rect.top;

	if( nX > (nTargetWidth - 1) || nY > (nTargetHeight - 1) ) return;

	temp = nX + m_nWidth; ncase = (nX < 0 && temp > 0);
	if( temp >= nTargetWidth ){ w = nTargetWidth - nX; }else if( ncase ){ w = m_nWidth + nX; }else{ w = m_nWidth; }
	if( nX < 0 && temp > 0 ){ sx = -nX; } else sx = 0;

	temp = nY + m_nHeight; ncase = (nY < 0 && temp > 0);
	if( temp >= nTargetHeight ){ h = nTargetHeight - nY; }else if( ncase ){ h = m_nHeight + nY; }else{ h = m_nHeight; }
	if( nY < 0 && temp > 0 ){ sy = -nY; } else sy = 0;
	
	FastBlt0( pBuffer, min(nTargetWidth, max(0, nX)), min(nTargetHeight, max(0, nY)), lPitch, sx, sy, RGB322RGB16(sCol), w, h);
}

