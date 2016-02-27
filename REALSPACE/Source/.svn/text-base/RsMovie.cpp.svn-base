/*
	RsMovie.cpp
	-----------

	RealSpace AVI Engine : AVI only

	Programming by Joongpil Cho
	All copyright (c) by MAIET entertainment
*/
#include <windows.h>
#include <crtdbg.h>
#include "RealSpace.h"
#include "RsMovie.h"
#include "RSImage.h"
#include "RSMMX.h"
#include "RSD3D.H"
#include <d3d8.h>
#include <d3dx8tex.h>
#include <dsound.h>
#include "RSDebug.h"

#define SOUNDSLOTCOUNT	2

static bool g_bPlaying=false;
static LPDIRECT3DSURFACE8	g_pBuffer;	// Image Buffer
static D3DFORMAT g_d3d_pf;
static int g_BufferWidth,g_BufferHeight;

static BOOL RSA_InitSound( PMOVIEOBJECT pMovieObject, LPDIRECTSOUND pDS )
{
	LONG lFmtLengh;
	DSBUFFERDESC dsbd;

	AVIStreamFormatSize( pMovieObject->pasSound, 0, &lFmtLengh );
	pMovieObject->pWavFormat = (WAVEFORMATEX *) malloc( lFmtLengh );
	AVIStreamReadFormat( pMovieObject->pasSound, 0, pMovieObject->pWavFormat, &lFmtLengh );

	AVIStreamInfo( pMovieObject->pasSound, &pMovieObject->asiSound, sizeof(AVISTREAMINFO) );

	pMovieObject->dwLoadSize = pMovieObject->pWavFormat->nAvgBytesPerSec;

#ifdef _LOGDEBUG
	LOG( "RSA_InitSound : dwLoadSize = %d, dwFPS = %d\n", pMovieObject->dwLoadSize, pMovieObject->dwFps );
#endif

	memset (&dsbd, 0, sizeof (DSBUFFERDESC));
	dsbd.dwSize = sizeof(dsbd);
    dsbd.dwFlags = DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME;
    dsbd.dwBufferBytes = pMovieObject->dwLoadSize * SOUNDSLOTCOUNT ;

#ifdef _LOGDEBUG
	LOG( "RSA_InitSound : dwBufferBytes = %d\n", dsbd.dwBufferBytes );
#endif
    dsbd.dwReserved = 0;
    dsbd.lpwfxFormat = pMovieObject->pWavFormat;
	pMovieObject->pSndBuffer = NULL;

	if( pDS->CreateSoundBuffer( &dsbd, &pMovieObject->pSndBuffer, NULL ) != DS_OK ){
		RSA_Close( pMovieObject );
		return FALSE;
	}

	return TRUE;
}

PMOVIEOBJECT RSA_Open( char *szFileName, RealSpace *pRS, LPDIRECTSOUND pDS, BOOL bLoop, BOOL bSndSupport )
{
	PMOVIEOBJECT pMovieObject;
	LONG lFmtLength;
	int PixelFormat;

	AVIFileInit();		// init AVI windows subsystem.

	pMovieObject = (PMOVIEOBJECT) malloc(sizeof(MOVIEOBJECT));
	if( !pMovieObject ) return NULL;

	ZeroMemory(pMovieObject, sizeof(MOVIEOBJECT));
	ZeroMemory(&(pMovieObject->asiMovie), sizeof(AVISTREAMINFO));

	// 0x6C번지의 "vids"를 "aids"로 바꾼다.
#define AVI_IDENTIFIER	mmioFOURCC('v', 'i', 'd', 's')
#define AVI_IDENTIFIER2	mmioFOURCC('a', 'i', 'd', 's')


	if( AVIStreamOpenFromFile( &(pMovieObject->pasMovie), szFileName, /*streamtypeVIDEO,*/ AVI_IDENTIFIER, 0, OF_READ, NULL) ){	// Open a stream of AVI file.
		if( AVIStreamOpenFromFile( &(pMovieObject->pasMovie), szFileName, /*streamtypeVIDEO,*/ AVI_IDENTIFIER2, 0, OF_READ, NULL) ){	// Open a stream of AVI file.
			//RSA_Close( pMovieObject );
			free( pMovieObject );
			return FALSE;
		}
	}

/*
	if( AVIFileOpen( &(pMovieObject->AviFile), szFileName, OF_READ, 0 ) ) {	
		AVIFileExit();
		free( pMovieObject );
		return FALSE;
	}

	if( AVIFileGetStream( pMovieObject->AviFile, &(pMovieObject->pasMovie), streamtypeVIDEO, 0 ) ) {	
		AVIFileExit();
		free( pMovieObject );
		return FALSE;
	}
*/
	AVIStreamFormatSize( pMovieObject->pasMovie, 0, &lFmtLength );
	pMovieObject->lpScrFmt = (LPBITMAPINFOHEADER) malloc( lFmtLength );
	pMovieObject->lpb4hTargetFmt = (LPBITMAPV4HEADER) malloc( max(lFmtLength, sizeof(BITMAPV4HEADER)));
	ZeroMemory( pMovieObject->lpb4hTargetFmt, sizeof(BITMAPV4HEADER) );
	
	AVIStreamReadFormat( pMovieObject->pasMovie, 0, pMovieObject->lpScrFmt, &lFmtLength );

	pMovieObject->lFrames = AVIStreamLength( pMovieObject->pasMovie );
	AVIStreamInfo( pMovieObject->pasMovie, &(pMovieObject->asiMovie), sizeof(AVISTREAMINFO) );

	PixelFormat = RSGetPixelFormat();

	if( PixelFormat == RS_PIXELFORMAT_565 ){		// RSPIXELFORMAT565
		g_d3d_pf = D3DFMT_R5G6B5;
	} else {										// RSPIXELFORMAT555
		g_d3d_pf = D3DFMT_X1R5G5B5;
	}

	g_BufferWidth=pMovieObject->lpScrFmt->biWidth;
	g_BufferHeight=pMovieObject->lpScrFmt->biHeight;

	g_bPlaying=true;
	RSA_Restore();

	if(!g_pBuffer){
#ifdef _LOGDEBUG
		LOG( "[RsMovie.cpp] Cannot create a temporary buffer for movie object.\n" );
#endif
		RSA_Close( pMovieObject );
		return NULL;
	}
	
	pMovieObject->nWidth = pMovieObject->lpScrFmt->biWidth;
	pMovieObject->nHeight = pMovieObject->lpScrFmt->biHeight;
	
	RSMemCopy( pMovieObject->lpb4hTargetFmt, pMovieObject->lpScrFmt, lFmtLength );

	pMovieObject->lpb4hTargetFmt->bV4Size = sizeof(BITMAPV4HEADER);
	pMovieObject->lpb4hTargetFmt->bV4Width = pMovieObject->lpScrFmt->biWidth;
	pMovieObject->lpb4hTargetFmt->bV4Height = pMovieObject->lpScrFmt->biHeight;
	pMovieObject->lpb4hTargetFmt->bV4Planes = 1;
	pMovieObject->lpb4hTargetFmt->bV4BitCount = 16;					// 16 bit only(RealSpace)		
	pMovieObject->lpb4hTargetFmt->bV4V4Compression = BI_BITFIELDS;	// It's not compressed format
	pMovieObject->lpb4hTargetFmt->bV4SizeImage = 
			((pMovieObject->lpb4hTargetFmt->bV4Width +3)&0xFFFFFFFC) *
		     pMovieObject->lpb4hTargetFmt->bV4Height * (pMovieObject->lpb4hTargetFmt->bV4BitCount>>3);
	pMovieObject->lpb4hTargetFmt->bV4ClrUsed = 0;

/*
  DDPF_RGB  16 : 565

	R: 0x0000F800
    G: 0x000007E0
    B: 0x0000001F

  DDPF_RGB  16 : 555

	R: 0x00007C00
    G: 0x000003E0
    B: 0x0000001F
*/
	if( PixelFormat == RS_PIXELFORMAT_565 ){		// RSPIXELFORMAT565
		pMovieObject->lpb4hTargetFmt->bV4RedMask	= 0x0000F800;
		pMovieObject->lpb4hTargetFmt->bV4GreenMask	= 0x000007E0;
		pMovieObject->lpb4hTargetFmt->bV4BlueMask	= 0x0000001F;
	} else {										// RSPIXELFORMAT555
		pMovieObject->lpb4hTargetFmt->bV4RedMask	= 0x00007C00;
		pMovieObject->lpb4hTargetFmt->bV4GreenMask	= 0x000003E0;
		pMovieObject->lpb4hTargetFmt->bV4BlueMask	= 0x0000001F;
	}
	
	pMovieObject->lLength = pMovieObject->lpScrFmt->biWidth * pMovieObject->lpScrFmt->biHeight * (pMovieObject->lpScrFmt->biBitCount >> 3);

	if( pMovieObject->asiMovie.dwSuggestedBufferSize ){
		pMovieObject->lLength = (LONG) pMovieObject->asiMovie.dwSuggestedBufferSize;
	}

	pMovieObject->hicDecompressor = ICDecompressOpen( ICTYPE_VIDEO, 
		pMovieObject->asiMovie.fccHandler, 
		pMovieObject->lpScrFmt, 
		(LPBITMAPINFOHEADER) pMovieObject->lpb4hTargetFmt );
	pMovieObject->lpInput = (BYTE *) calloc( pMovieObject->lLength, 1 );
	ZeroMemory( pMovieObject->lpInput, pMovieObject->lLength );
	pMovieObject->lpOutput = (BYTE *) calloc( pMovieObject->lpb4hTargetFmt->bV4SizeImage, 1 );
	ZeroMemory( pMovieObject->lpOutput, pMovieObject->lpb4hTargetFmt->bV4SizeImage );

	if( !pMovieObject->hicDecompressor ){
		RSA_Close( pMovieObject );
		return NULL;
	}
	pMovieObject->lLinePitch = pMovieObject->lpb4hTargetFmt->bV4Width 
		* (pMovieObject->lpb4hTargetFmt->bV4BitCount>>3);
	ICDecompressBegin( pMovieObject->hicDecompressor, pMovieObject->lpScrFmt,
		(LPBITMAPINFOHEADER) pMovieObject->lpb4hTargetFmt );
	pMovieObject->dwFps = pMovieObject->asiMovie.dwRate/pMovieObject->asiMovie.dwScale;
//	pMovieObject->nTimeTick = (1000 * pMovieObject->asiMovie.dwScale + (pMovieObject->asiMovie.dwRate >> 1)) / pMovieObject->asiMovie.dwRate;
	
	pMovieObject->Flags.bLoop = bLoop;
	pMovieObject->Flags.bSndSupport = bSndSupport;

/*
			{   DSBUFFERDESC dsbd;

				AVIStreamFormatSize(m_AviSound, 0, &FmtLenght);
				m_SoundFormat = (WAVEFORMATEX *)malloc(FmtLenght);

				AVIStreamReadFormat(m_AviSound, 0, m_SoundFormat, &FmtLenght);
				AVIStreamInfo(m_AviSound, &m_SoundInfo, sizeof(AVISTREAMINFO));

				m_LoadSize = (m_SoundFormat->nAvgBytesPerSec + m_Fps - 1 ) / m_Fps;

				dsbd.dwSize = sizeof(dsbd);
				dsbd.dwFlags = 0;
				dsbd.dwBufferBytes = m_LoadSize * m_Fps;
				dsbd.dwReserved = 0;
				dsbd.lpwfxFormat = m_SoundFormat;

				if( pSound->m_lpDS->CreateSoundBuffer(&dsbd, &m_SoundBuffer, NULL) != DS_OK )
				{       AVIFileExit();
						return FALSE;
				}

*/
	if( bSndSupport ){	// Sound Support
		if( !pDS ){
			pMovieObject->Flags.bSndSupport = FALSE;
		} else {
			pMovieObject->Flags.bSndSupport = TRUE;

			ZeroMemory( &pMovieObject->pasSound, sizeof(pMovieObject->pasSound) );
			if( AVIStreamOpenFromFile( &pMovieObject->pasSound, szFileName, streamtypeAUDIO, 0, OF_READ, NULL ) ){
				pMovieObject->Flags.bSndSupport = FALSE;
				return pMovieObject;
			}
			if( !RSA_InitSound( pMovieObject, pDS ) ){
				RSA_Close( pMovieObject );
				return NULL;
			}
		}
	}

	return pMovieObject;
}

void RSA_Close( PMOVIEOBJECT pMovieObject )
{
	g_bPlaying=false;

	if( !pMovieObject ) return;
	if( pMovieObject->Flags.bPlaying ) RSA_Stop( pMovieObject );

	if( pMovieObject->hicDecompressor ){
		ICDecompressEnd( pMovieObject->hicDecompressor );
		ICClose( pMovieObject->hicDecompressor );
	}

	RSA_Invalidate();

	free( pMovieObject->lpScrFmt );
	free( pMovieObject->lpb4hTargetFmt );
	free( pMovieObject->lpInput );
	free( pMovieObject->lpOutput );

	AVIStreamRelease( pMovieObject->pasMovie );

	if( pMovieObject->pWavFormat ){
		free( pMovieObject->pWavFormat );
		pMovieObject->pWavFormat = NULL;
	}
	if( pMovieObject->pSndBuffer ){
		pMovieObject->pSndBuffer->Release();
	}
	if( pMovieObject->pasSound ) AVIStreamRelease( pMovieObject->pasSound );
	
	free( pMovieObject );
	
	AVIFileExit();	// release AVI windows subsystem
}

static DWORD dwStartedTime;

static HRESULT RSA_GetFrameSound( PMOVIEOBJECT pMovieObject )
{
	HRESULT hr;
	DWORD dwSize1, dwSize2;
	LPVOID Data1,Data2;

	if( !pMovieObject->Flags.bSndSupport ){
		return DS_OK;
	}

	DWORD currenttime=timeGetTime();
	while((currenttime-dwStartedTime)/1000+1 >= pMovieObject->dwLoadPos)
	{
//		rslog("%d loadpos\n",pMovieObject->dwLoadPos);
		hr = pMovieObject->pSndBuffer->Lock(
					(pMovieObject->dwLoadPos % SOUNDSLOTCOUNT) * pMovieObject->dwLoadSize,
					pMovieObject->dwLoadSize,
					&Data1, &dwSize1,
					&Data2, &dwSize2,
					0
				);
		_ASSERT(hr==DS_OK);

		AVIStreamRead( 
			pMovieObject->pasSound, 
			pMovieObject->dwLoadPos * (pMovieObject->dwLoadSize / pMovieObject->asiSound.dwSampleSize ),
			(pMovieObject->dwLoadSize / pMovieObject->asiSound.dwSampleSize ), 
			Data1, 
			dwSize1, 
			NULL, 
			NULL 
		);

		hr = pMovieObject->pSndBuffer->Unlock( Data1, dwSize1, Data2, dwSize2 );

		pMovieObject->dwLoadPos++;

	}

	return hr;
}

int RSA_GetNextFrame( PMOVIEOBJECT pMovieObject )
{
	if( !pMovieObject->Flags.bPlaying ) return -1;

	if( pMovieObject->Flags.bSndSupport ){
		RSA_GetFrameSound( pMovieObject );
	}

	pMovieObject->lIndex++;	
	
	if( pMovieObject->lIndex == pMovieObject->lFrames ){		// 끝났을때
//		timeKillEvent( pMovieObject->nTimerID );
		if( pMovieObject->Flags.bSndSupport ) pMovieObject->pSndBuffer->Stop();
		pMovieObject->Flags.bPlaying = FALSE;
	}

	return 0;
}

/*
void CALLBACK RSA_TimerProc( UINT uId, UINT uMsg, DWORD pMovieObject, DWORD dw1, DWORD dw2 )
{
	PMOVIEOBJECT pmo=(PMOVIEOBJECT) pMovieObject;

	DWORD currenttime=timeGetTime();
	
	if((currenttime-dwStartedTime) *  pmo->asiMovie.dwRate / (1000 * pmo->asiMovie.dwScale)  
		> pmo->lIndex)
	
	RSA_GetNextFrame( pmo );	
}
*/

void RSA_CheckFrame( PMOVIEOBJECT pMovieObject)
{
	DWORD currenttime=timeGetTime();
	
	while(LONG((currenttime-dwStartedTime) *  pMovieObject->asiMovie.dwRate / (1000 * pMovieObject->asiMovie.dwScale)) > pMovieObject->lIndex){
		if(RSA_GetNextFrame( pMovieObject )==-1)
			break;
	}
}

BOOL RSA_Play( PMOVIEOBJECT pMovieObject )
{	
	if( !pMovieObject ) return FALSE;
	
	/*
	if( pMovieObject->Flags.bSndSupport ) 
		RSA_GetFrameSound( pMovieObject );

	pMovieObject->nTimerID = timeSetEvent(
		pMovieObject->nTimeTick/2,
		5,
		RSA_TimerProc,
		(ULONG) pMovieObject,
		TIME_PERIODIC );
*/

	dwStartedTime=timeGetTime();

	pMovieObject->dwLoadPos=0;
	pMovieObject->Flags.bPlaying = TRUE;
	RSA_GetNextFrame( pMovieObject );

	if( pMovieObject->Flags.bSndSupport ) {
		pMovieObject->nSoundFramesAhead = pMovieObject->asiSound.dwInitialFrames / pMovieObject->asiSound.dwScale;
		pMovieObject->pSndBuffer->SetCurrentPosition(0);
		return ( pMovieObject->pSndBuffer->Play( 0, 0, DSBPLAY_LOOPING ) == DS_OK );
	}

	return pMovieObject->Flags.bPlaying;
}

BOOL RSA_Stop( PMOVIEOBJECT pMovieObject )
{
	if( !pMovieObject ) return FALSE;

	Sleep( 30 );
	if( pMovieObject->Flags.bSndSupport ){
		pMovieObject->pSndBuffer->Stop();
	}

	pMovieObject->Flags.bPlaying = FALSE;
//	timeKillEvent(pMovieObject->nTimerID);

	return TRUE;
}

BOOL RSA_DrawFrame( PMOVIEOBJECT pMovieObject, LONG lFrame )
{	
	LPBYTE pbSrc;
	LPBYTE pDest;
	int i;
	D3DLOCKED_RECT lr;

	if( !pMovieObject->Flags.bPlaying ) return FALSE;

	if( pMovieObject->Flags.bSndSupport ){
		if( lFrame - pMovieObject->nSoundFramesAhead < 0 ) return TRUE;
		lFrame = lFrame - pMovieObject->nSoundFramesAhead;
	}	

	if( lFrame < pMovieObject->lFrames ){
		AVIStreamRead( pMovieObject->pasMovie, lFrame, 1, pMovieObject->lpInput, pMovieObject->lLength, NULL, NULL );
		ICDecompress( pMovieObject->hicDecompressor, 0, pMovieObject->lpScrFmt, pMovieObject->lpInput, (LPBITMAPINFOHEADER) pMovieObject->lpb4hTargetFmt, pMovieObject->lpOutput );
	} else return TRUE;
	
	pbSrc = pMovieObject->lpOutput + pMovieObject->lLinePitch * (pMovieObject->lpb4hTargetFmt->bV4Height - 1 );

	if( g_pBuffer->LockRect(&lr, NULL, 0) != D3D_OK ){ return FALSE; }

	pDest = (LPBYTE)lr.pBits;

	if( pDest ){
		for( i = 0; i < (int)pMovieObject->lpb4hTargetFmt->bV4Height; i ++ ){
			RSMemCopy( pDest, pbSrc, pMovieObject->lLinePitch );
			pDest += lr.Pitch;
			pbSrc -= pMovieObject->lLinePitch;
		}
		g_pBuffer->UnlockRect();
		return TRUE;
	}
	return FALSE;
}

BOOL RSA_DrawFast( PMOVIEOBJECT pMovieObject, int nX, int nY )
{
	if( !pMovieObject ) return FALSE;
	if( RSA_DrawFrame( pMovieObject, pMovieObject->lIndex ) != TRUE ) return FALSE;
	
    RECT  rcSrc;
    SetRect( &rcSrc, 0, 0, pMovieObject->nWidth, pMovieObject->nHeight );

    POINT ptDst;
    ptDst.x = nX;
    ptDst.y = nY;

//	g_pBuffer->Draw( pTarget, nX, nY, lPitch );
	LPDIRECT3DSURFACE8 pBackBuffer = RSGetBackBuffer();
	if(pBackBuffer == NULL) return FALSE;

	RSGetD3DDevice()->CopyRects(g_pBuffer, &rcSrc, 1, pBackBuffer, &ptDst);
	pBackBuffer->Release();

	return TRUE;
}

BOOL RSA_Draw( PMOVIEOBJECT pMovieObject, int nWidth, int nHeight )
{
	if( !pMovieObject ) return FALSE;
	RSA_CheckFrame(pMovieObject);
	if( RSA_DrawFrame( pMovieObject, pMovieObject->lIndex ) != TRUE ) return FALSE;
	
    RECT  rcSrc, rcDst;
    SetRect( &rcSrc, 0, 0, pMovieObject->nWidth, pMovieObject->nHeight );

    POINT ptDst;
    ptDst.x = 0;
    ptDst.y = 0;

//	g_pBuffer->Draw( pTarget, nX, nY, lPitch );
	LPDIRECT3DSURFACE8 pBackBuffer = RSGetBackBuffer();
	if(pBackBuffer == NULL) return FALSE;

	if( pMovieObject->nWidth > pMovieObject->nHeight ){
		rcDst.left = 0;
		rcDst.right = nWidth;
		int nTemp = (pMovieObject->nHeight * nWidth)/pMovieObject->nWidth;
		rcDst.top = (nHeight - nTemp)/2;
		rcDst.bottom = rcDst.top + nTemp;
	} else {
		int nTemp = (pMovieObject->nWidth * nHeight)/pMovieObject->nHeight;
		rcDst.left = (nWidth - nTemp)/2;
		rcDst.right = rcDst.left + nTemp;
		rcDst.top = 0;
		rcDst.bottom = nHeight;
	}

	D3DXLoadSurfaceFromSurface(pBackBuffer, NULL, &rcDst, g_pBuffer, NULL, NULL, D3DX_FILTER_POINT, 0);
	pBackBuffer->Release();
	return TRUE;
}

void RSA_Invalidate()
{
	if(g_pBuffer)
	{
		g_pBuffer->Release();
		g_pBuffer=NULL;
	}
}

void RSA_Restore()
{
	RSA_Invalidate();
	if(g_bPlaying)
		RSGetD3DDevice()->CreateImageSurface(g_BufferWidth, g_BufferHeight, g_d3d_pf, &(g_pBuffer));
}
