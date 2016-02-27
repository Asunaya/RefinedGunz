/*
	RsPrint-e.cpp
	-------------

	Programming by Joongpil Cho
	All copyright (c) 1998, MAIET entertainment software
*/
#include "RealSpace.h"
#include "RSImage.h"
#include "RSDebug.h"
//#include "TBTextureList.h"
//#include "TBTexture.h"
#include <stdio.h>

#define __RS_FONT_HEIGHT	16

typedef char	RSFONT[256][__RS_FONT_HEIGHT];

static RSFONT _RsPrimFont;
static RECT _RsClipRect = { 0, 0, 640, 480 };
static unsigned char _RsSpaceWidth = 8;

void RsPrintSetClipRect( RECT rc )
{
	_RsClipRect.left = rc.left;
	_RsClipRect.top = rc.top;
	_RsClipRect.right = rc.right;
	_RsClipRect.bottom = rc.bottom;
}

static void _PutEngChar( LPWORD v, int nWidth, int nHeight, int color, char *pData )
{
	int i;
	nWidth >>= 1;

	for( i = 0; i < nHeight; i ++ ){
		if( *pData ){
			if( *pData & 0x80 ) * (v)	= color;
			if( *pData & 0x40 ) * (v+1)	= color;
			if( *pData & 0x20 ) * (v+2)	= color;
			if( *pData & 0x10 ) * (v+3)	= color;
			if( *pData & 0x08 ) * (v+4)	= color;
			if( *pData & 0x04 ) * (v+5)	= color;
			if( *pData & 0x02 ) * (v+6) = color;
			if( *pData & 0x01 ) * (v+7) = color;
		}
		v += nWidth;
		pData ++;
	}
}

static void _PutEngCharWithClip( LPWORD v, int nWidth, int nHeight, int color, char *pData )
{
	int i;
	nWidth >>= 1;

	for( i = 0; i < nHeight; i ++ ){
		if( *pData ){
			if( *pData & 0x80 ) * (v)	= color;
			if( *pData & 0x40 ) * (v+1)	= color;
			if( *pData & 0x20 ) * (v+2)	= color;
			if( *pData & 0x10 ) * (v+3)	= color;
			if( *pData & 0x08 ) * (v+4)	= color;
			if( *pData & 0x04 ) * (v+5)	= color;
			if( *pData & 0x02 ) * (v+6) = color;
			if( *pData & 0x01 ) * (v+7) = color;
		}
		v += nWidth;
		pData ++;
	}
}

BOOL RsSetSmFont(char *szPath)
{
	FILE *fp;

	fp = fopen( szPath, "rb" );
	if( !fp ) return FALSE;
	fread( _RsPrimFont, (256*__RS_FONT_HEIGHT), 1, fp );
	fclose( fp);

	return TRUE;
}

void RsPrintSm0( LPBYTE pTarget, int nX, int nY, int lPitch, int nColor, char *szString )
{
	BYTE fstByte;
	LPWORD pPtr;
	
	pPtr = (LPWORD)pTarget + ((nY * lPitch) >> 1) + nX;
	
	while( *szString ){
		fstByte = *szString ++;	
		if( fstByte != ' ' ){	// 영어
			_PutEngChar( pPtr, lPitch, __RS_FONT_HEIGHT, nColor, *(_RsPrimFont+fstByte));
			pPtr += _RsSpaceWidth;
		} else {
			pPtr += _RsSpaceWidth;
		}
	}
}

void RsPrintSm0WithCursor( LPBYTE pTarget, int nX, int nY, int lPitch, int nColor, char *szString )
{
	BYTE fstByte;
	LPWORD pPtr;
	
	pPtr = (LPWORD)pTarget + ((nY * lPitch) >> 1) + nX;
	
	while( *szString ){
		fstByte = *szString ++;	
		if( fstByte != ' ' ){	// 영어
			_PutEngChar( pPtr, lPitch, __RS_FONT_HEIGHT, nColor, *(_RsPrimFont+fstByte));
			pPtr += _RsSpaceWidth;
		} else {
			pPtr += _RsSpaceWidth;
		}
	}
	_PutEngChar( pPtr, lPitch, __RS_FONT_HEIGHT, nColor, *(_RsPrimFont+'_') );
}

void RsPrintSm0WithClip( LPBYTE pTarget, int nX, int nY, int lPitch, int nColor, char *szString )
{
	BYTE fstByte;
	LPWORD pPtr;
	
	pPtr = (LPWORD)pTarget + ((nY * lPitch) >> 1) + nX;
	
	while( *szString ){
		fstByte = *szString ++;	
		if( fstByte != ' ' ){	// 영어
			_PutEngChar( pPtr, lPitch, __RS_FONT_HEIGHT, nColor, *(_RsPrimFont+fstByte));
			pPtr += _RsSpaceWidth;
		} else {
			pPtr += _RsSpaceWidth;
		}
	}
}

void RsPrintSm( LPBYTE pTarget, int nX, int nY, int lPitch, int nColor, char *szString, ... )
{
	char buffer[_MAX_PATH];
	va_list argptr;

	va_start( argptr, szString );
	vsprintf( buffer, szString, argptr );
	va_end( argptr );

	RsPrintSm0( pTarget, nX, nY, lPitch, nColor, buffer );
}

void RsPrintSmWithClip( LPBYTE pTarget, int nX, int nY, int lPitch, int nColor, char *szString, ... )
{
	char buffer[_MAX_PATH];
	va_list argptr;

	if( nX >= _RsClipRect.right || nY >= _RsClipRect.bottom ) return;

	va_start( argptr, szString );
	vsprintf( buffer, szString, argptr );
	va_end( argptr );

	RsPrintSm0WithClip( pTarget, nX, nY, lPitch, nColor, buffer );
}

void RsSetSpaceWidth( unsigned char nWidth )
{
	_RsSpaceWidth = nWidth;
}