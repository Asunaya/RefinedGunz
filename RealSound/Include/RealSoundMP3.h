// MPEG1 Audio Layer-3 Class
// Copyright(C) 2000 by TOx2RO / O2 Software All rights reserved.
// Modified by Joongpil Cho
//
// RealSoundMP3.h

#ifndef __REALSOUNDMP3_HEADER__
#define __REALSOUNDMP3_HEADER__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <process.h>
#include "mp3dec.h"

#include "RealSound.h"

enum RealSoundMP3_State
{
	REALSOUND_CLOSED,
	REALSOUND_OPENED,
	REALSOUND_PLAYING,
	REALSOUND_PAUSED
};

// 10-Bands Equalizer
struct REALSOUND_EQ_PARAM
{
	signed char _60;							// 60Hz
	signed char _170;							// 170Hz
	signed char _310;							// 310Hz
	signed char _600;							// 600Hz
	signed char _1k;							// 1kHz
	signed char _3k;							// 3kHz
	signed char _6k;							// 6kHz
	signed char _12k;							// 12kHz
	signed char _14k;							// 14kHz
	signed char _16k;							// 16kHz
};

class RealSoundMP3 {
public:
	RealSoundMP3();
	~RealSoundMP3();
	bool Create(LPSTR mp3file, RealSound* pRealSound);
	void Destroy();
	void Play(DWORD start,DWORD end);
	void LoopPlay(DWORD start,
					 DWORD end,
					 DWORD loop,
					 DWORD number);
	void Pause();
	void Resume();
	void Stop();
	void SetEqualizer(REALSOUND_EQ_PARAM eq);

private:
	static void CALLBACK waveOutProc(HWAVE hwo,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2);
	static DWORD WINAPI DecodeFunc(LPVOID dwInstance); 
	inline DWORD msec2frame(DWORD msec);
	inline DWORD frame2msec(DWORD frame);
	inline DWORD msec2bytes(DWORD bytes);

private:
	RealSoundMP3_State State;
	DWORD LoopCount;
	DWORD Partition;
	BYTE sw;
	DWORD BufferingNumber;

	HANDLE ThreadHandle;
	DWORD ThreadID;

	HANDLE FileHandle;
	HANDLE MapFile;
	BYTE* pSrcFile;
	BYTE* pFrameData;
	DWORD NowPos;
	HWAVEOUT WaveOut;
	WAVEFORMATEX PCMwf;

	MPEG_DECODE_PARAM MP3DP[2];
	MPEG_DECODE_INFO MP3DI;

	WAVEHDR PCMwh[2];
	LPVOID Buffer[2];

	LPSTR FileName;
	DWORD FileSize;
	DWORD PartitionNumber;
	DWORD startSec;
	DWORD endSec;
	DWORD loopSec;
	BYTE LoopMode;
	bool NowLooping;
	bool InfiniteLoop;

	WORD MpegSpec;
	WORD Bitrate;
	WORD Frequency;
	WORD Channel;
	WORD FrameSize;

	///////////////////////////////////////////////////////////////////////////
	// DirectSound Relative
	///////////////////////////////////////////////////////////////////////////

	RealSound*				m_pRealSound;
	LPDIRECTSOUNDBUFFER		m_pdsb;
	DSBUFFERDESC			m_dsbd;
};

#endif	// __REALSOUNDMP3_HEADER__

///////////////////////////////////////////////////////////////////////////////