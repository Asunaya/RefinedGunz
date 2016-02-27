/*
	RsMovie.h
	---------

	RealSpace AVI Engine : AVI only

	현재 풀스크린 모드만 지원한다.

	Programming by Joongpil Cho
	All copyright (c) by MAIET entertainment
*/
#ifndef __RSMOVIE_HEADER__
#define __RSMOVIE_HEADER__

#include <windows.h>
#include <vfw.h>
#include <dsound.h>
#include <d3d8.h>

#pragma comment( lib, "vfw32.lib" )
#pragma comment( lib, "winmm.lib" )

class RealSpace;
class RSImage;

#pragma pack(1)

typedef struct movieObject {
	///////////////////////////
	// Buffer Object
	LPDIRECTSOUNDBUFFER	pSndBuffer;	// Sound Stream Buffer (DirectX Sound Buffer)

	int		nWidth;
	int		nHeight;

	///////////////////////////
	// AVI Information

	LONG	lIndex;					// actual Frame - Index	
	LONG	lFrames;				// Video-Frames in AVI-File
	DWORD	dwFps;					// Freames per Second
	int		nTimerID;				// TimerID of timeEvents
//	int		nTimeTick;				// Time between two frames 

	struct {
		unsigned int bLoop:1;		// Looping Flag
		unsigned int bPlaying:1;	// Playing Flag
		unsigned int bSndSupport:1;	// Sound Playback Flag
	} Flags;

	///////////////////////////
	// Video For Window Object
	PAVIFILE		AviFile;		//<--
	
	PAVISTREAM		pasMovie;		// Video Stream Handle
	AVISTREAMINFO	asiMovie;		// Video Stream Format Information
	LPBYTE			lpInput;		// Decompressor Input -Buffer
	LPBYTE			lpOutput;		// Decompressor Output-Buffer
	HIC				hicDecompressor;
									// Handle to the AVI-Decompressor

	///////////////////////////
	// Sound Object
	PAVISTREAM		pasSound;		// Sound Stream Handle
	AVISTREAMINFO	asiSound;		// Sound Stream Format Information
	WAVEFORMATEX*	pWavFormat;		// Sample Data Format
	DWORD			dwLoadPos;		// Loadpos in streaming Buffer
	DWORD			dwLoadSize;		// Samples to load per frame
	int				nSoundFramesAhead;
									// Frames of sound to load befor Videoplayback starts

	///////////////////////////
	// Bitmap Informations
	LONG				lLength;		// Input-Bufferlength
	LONG				lLinePitch;		// Bitmap-Linepitch
	LPBITMAPINFOHEADER	lpScrFmt;		// Format of VideoInput
	LPBITMAPV4HEADER	lpb4hTargetFmt;	// Format of Surface
} MOVIEOBJECT, *PMOVIEOBJECT, **PPMOVIEOBJECT;

#pragma pack()

// initialize RSA system
PMOVIEOBJECT RSA_Open( char *szFileName, RealSpace *pRS, LPDIRECTSOUND pDS, BOOL bLoop, BOOL bSndSupport );
void RSA_Close( PMOVIEOBJECT pMovieObject );
BOOL RSA_Play( PMOVIEOBJECT pMovieObject );
BOOL RSA_Stop( PMOVIEOBJECT pMovieObject );
//BOOL RSA_DrawFast( PMOVIEOBJECT pMovieObject, int nX, int nY );
BOOL RSA_Draw( PMOVIEOBJECT pMovieObject, int nWidth, int nHeight );

void RSA_Invalidate();
void RSA_Restore();

#endif // __RSMOVIE_HEADER__