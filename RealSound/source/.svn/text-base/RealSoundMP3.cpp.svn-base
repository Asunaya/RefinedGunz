// MPEG1 Audio Layer-3 Class
// Copyright(C) 2000 by TOx2RO / O2 Software All rights reserved.
// Modified by Joongpil Cho
//
// RealSoundMP3.cpp
#include "RealSoundMP3.h"

#ifdef _DEBUG
#include <crtdbg.h>
	#define _D		::OutputDebugString
#else 
	#define _D		(void)0
#endif

const UINT DefMp3MaxBuffers = 64;

RealSoundMP3::RealSoundMP3()
{
	m_pRealSound	= NULL;
	m_pdsb			= NULL;

	Bitrate			= 0;
	Channel			= 0;
	Buffer[0]		= NULL;
	Buffer[1]		= NULL;
	BufferingNumber = DefMp3MaxBuffers;
	sw				= 0;
	State			= REALSOUND_CLOSED;
	LoopCount		= 0;
	LoopMode		= 0;
	FileHandle		= NULL;
	FileName		= NULL;
	FileSize		= 0;
	MapFile			= NULL;
	pSrcFile		= NULL;
	pFrameData		= NULL;
	FrameSize		= 0;
	Frequency		= 0;
	NowPos			= 0;
	WaveOut			= NULL;
	PartitionNumber	= 0;
	Partition		= 0;
	startSec		= 0;
	endSec			= 0;
	loopSec			= 0;
	ThreadHandle	= NULL;
	ThreadID		= 0;
	NowLooping		= false;
	InfiniteLoop	= false;

	ZeroMemory(&m_dsbd,sizeof(DSBUFFERDESC));

	ZeroMemory(&PCMwf,sizeof(WAVEFORMATEX));
	ZeroMemory(&PCMwh[0],sizeof(WAVEHDR));
	ZeroMemory(&PCMwh[1],sizeof(WAVEHDR));
	ZeroMemory(&MP3DI,sizeof(MPEG_DECODE_INFO));
	ZeroMemory(&MP3DP[0],sizeof(MPEG_DECODE_PARAM));
	ZeroMemory(&MP3DP[1],sizeof(MPEG_DECODE_PARAM));
}

RealSoundMP3::~RealSoundMP3()
{
	Destroy();
}

bool RealSoundMP3::Create(LPSTR pszFilename, RealSound* pRealSound)
{
	Destroy();

#ifdef _DEBUG
    _ASSERT (pszFilename);
    _ASSERT (pRealSound);
#endif
    m_pRealSound = pRealSound;

	FileHandle	= CreateFile(pszFilename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,NULL);
	MapFile		= CreateFileMapping(FileHandle,NULL,PAGE_READONLY,0,0,NULL);
	pSrcFile	= (BYTE*)MapViewOfFile(MapFile,FILE_MAP_READ,0,0,0);

	if(FileHandle == INVALID_HANDLE_VALUE) { return false; }
	if(!MapFile) { return false; }
	if(!pSrcFile) { return false; }

	if(!memcmp(&pSrcFile[0],"RIFF",4))
	{
		if(!memcmp(&pSrcFile[8],"WAVEfmt",7))
		{
			if(!memcmp(&pSrcFile[50],"fact",4)) { pFrameData = pSrcFile + 70; }
			if(!memcmp(&pSrcFile[52],"fact",4)) { pFrameData = pSrcFile + 72; }
			if(!memcmp(&pSrcFile[50],"data",4)) { pFrameData = pSrcFile + 58; }
		}
		if(!memcmp(&pSrcFile[8],"RMP3",4))
		{
			if(!memcmp(&pSrcFile[12],"data",4)) { pFrameData = pSrcFile + 16; }
		}
	} else {
		pFrameData = pSrcFile;
	}
	
	FileSize = GetFileSize(FileHandle,NULL);
	FileSize -= (DWORD)(pFrameData - pSrcFile);
	if((pSrcFile[FileSize - 128] == 'T') && (pSrcFile[FileSize - 127] == 'A') && (pSrcFile[FileSize - 126] == 'G'))
	{
		FileSize -= 128;
	}

	int	aBitrate[2][16] = {
		{ 0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0 },
		{ 0, 8,16,24,32,64,80,56, 64,128,160,112,128,256,320,0 }
	};

	int aFrequency[2][4] = {
		{ 44100,48000,32000,0 },
		{ 22050,24000,16000,0 }
	};

	MpegSpec	= ((pFrameData[1] & 8) == 0) ? 1 : 0;
	Bitrate		= aBitrate[ MpegSpec ][ pFrameData[2] >> 4 ];
	Frequency	= aFrequency[ MpegSpec ][ (pFrameData[2] >> 2) & 3 ];
	Channel		= (pFrameData[3] >> 6 == 3) ? 1 : 2;
	FrameSize	= 144000 * Bitrate / Frequency;

	PCMwf.wFormatTag			= WAVE_FORMAT_PCM;
	PCMwf.nChannels				= Channel;
	PCMwf.nSamplesPerSec		= Frequency;
	PCMwf.nAvgBytesPerSec		= Frequency * Channel * 2;
	PCMwf.nBlockAlign			= Channel * 2;
	PCMwf.wBitsPerSample		= 16;

	mp3DecodeInit();
	mp3GetDecodeInfo(pFrameData,FrameSize,&MP3DI,1);
	mp3DecodeStart(pFrameData,FileSize);

	Buffer[0] = GlobalAlloc(GPTR,MP3DI.outputSize * DefMp3MaxBuffers);
	Buffer[1] = GlobalAlloc(GPTR,MP3DI.outputSize * DefMp3MaxBuffers);
	if(!Buffer[0]) { return false; }
	if(!Buffer[1]) { return false; }

	waveOutOpen( &WaveOut, WAVE_MAPPER, &PCMwf, (DWORD)waveOutProc, (DWORD)this, CALLBACK_FUNCTION );

	State = REALSOUND_OPENED;

	return true;
}

void RealSoundMP3::Destroy()
{
	if(State == REALSOUND_CLOSED) return;
	if(State == REALSOUND_PLAYING || State == REALSOUND_PAUSED) { Stop(); }

	waveOutClose(WaveOut);

	if(Buffer[0]) { GlobalFree(Buffer[0]); }
	if(Buffer[1]) { GlobalFree(Buffer[1]); }

	UnmapViewOfFile(pSrcFile);
	CloseHandle(MapFile);
	CloseHandle(FileHandle);

	Bitrate			= 0;
	Channel			= 0;
	Buffer[0]		= NULL;
	Buffer[1]		= NULL;
	BufferingNumber = DefMp3MaxBuffers;
	sw				= 0;
	State			= REALSOUND_CLOSED;
	LoopCount		= 0;
	LoopMode		= 0;
	FileHandle		= NULL;
	FileName		= NULL;
	FileSize		= 0;
	MapFile			= NULL;
	pSrcFile		= NULL;
	pFrameData		= NULL;
	FrameSize		= 0;
	Frequency		= 0;
	NowPos			= 0;
	WaveOut			= NULL;
	PartitionNumber	= 0;
	Partition		= 0;
	startSec		= 0;
	endSec			= 0;
	loopSec			= 0;
	ThreadHandle	= NULL;
	ThreadID		= 0;
	NowLooping		= false;
	InfiniteLoop	= false;
	ZeroMemory(&PCMwf,sizeof(WAVEFORMATEX));
	ZeroMemory(&PCMwh[0],sizeof(WAVEHDR));
	ZeroMemory(&PCMwh[1],sizeof(WAVEHDR));
	ZeroMemory(&MP3DI,sizeof(MPEG_DECODE_INFO));
	ZeroMemory(&MP3DP[0],sizeof(MPEG_DECODE_PARAM));
	ZeroMemory(&MP3DP[1],sizeof(MPEG_DECODE_PARAM));

	State = REALSOUND_CLOSED;
}

void RealSoundMP3::Play(DWORD start,DWORD end)
{
	if(State != REALSOUND_OPENED) { return; }

	sw = 0;
	Partition = 0;
	startSec = start;
	endSec = end;
	LoopMode = 0;

	State = REALSOUND_PLAYING;

	if(startSec > endSec) { startSec = 0; endSec = (FileSize / Bitrate * 8); }
	if(startSec < 0) { startSec = 0; }
	if(startSec == 0 && endSec == 0 ) { startSec = 0; endSec = ((FileSize / FrameSize) * FrameSize / Bitrate * 8); }
	if((FileSize / Bitrate * 8) < startSec) { startSec = 0; }
	if((FileSize / Bitrate * 8) < endSec) { endSec = (FileSize / Bitrate * 8); }

	BufferingNumber = DefMp3MaxBuffers;
	if(BufferingNumber > (msec2frame(endSec) - msec2frame(startSec)) + 1) { BufferingNumber = msec2frame(endSec) - msec2frame(startSec) + 1; }

	PartitionNumber = (((msec2frame(endSec) - msec2frame(startSec)) + 1) / BufferingNumber);
	if(((msec2frame(endSec) - msec2frame(startSec)) + 1) % BufferingNumber == 0) { PartitionNumber--; }

	ZeroMemory(&MP3DP[sw],sizeof(MPEG_DECODE_PARAM));

	WORD i = 0;

	if(msec2frame(startSec) <= 9)
	{
		for(i = 0 ; i < msec2frame(startSec); i++)
		{
			mp3GetDecodeInfo(pFrameData + NowPos,FrameSize,&MP3DI,1);
			MP3DP[sw].header			=	MP3DI.header;
			MP3DP[sw].bitRate			=	MP3DI.bitRate;
			MP3DP[sw].inputBuf			=	pFrameData + NowPos;
			MP3DP[sw].inputSize			=	MP3DI.header.padding ? MP3DI.maxInputSize : MP3DI.minInputSize;
			MP3DP[sw].outputBuf			=	(BYTE*)Buffer[sw] + (MP3DI.outputSize * i);
			MP3DP[sw].outputSize		=	MP3DI.outputSize;	
			mp3DecodeFrame(&MP3DP[sw]);

			NowPos += MP3DI.header.padding ? MP3DI.maxInputSize : MP3DI.minInputSize;
		}
	}
	else if(msec2frame(startSec) > 9)
	{
		for(i = 0 ; i < msec2frame(startSec) - 9;i++)
		{
			mp3GetDecodeInfo(pFrameData + NowPos,FrameSize,&MP3DI,1);
			NowPos += MP3DI.header.padding ? MP3DI.maxInputSize : MP3DI.minInputSize;
		}

		for(i = 0 ; i < 9; i++)
		{
			
			mp3GetDecodeInfo(pFrameData + NowPos,FrameSize,&MP3DI,1);
			MP3DP[sw].header			=	MP3DI.header;
			MP3DP[sw].bitRate			=	MP3DI.bitRate;
			MP3DP[sw].inputBuf			=	pFrameData + NowPos;
			MP3DP[sw].inputSize			=	MP3DI.header.padding ? MP3DI.maxInputSize : MP3DI.minInputSize;
			MP3DP[sw].outputBuf			=	(BYTE*)Buffer[sw] + (MP3DI.outputSize * i);
			MP3DP[sw].outputSize		=	MP3DI.outputSize;	
			mp3DecodeFrame(&MP3DP[sw]);
			
			NowPos += MP3DI.header.padding ? MP3DI.maxInputSize : MP3DI.minInputSize;
		}
	}

	for(i = 0 ; i < BufferingNumber ; i++)
	{
	mp3GetDecodeInfo(pFrameData + NowPos,FrameSize,&MP3DI,1);
	MP3DP[sw].header			=	MP3DI.header;
	MP3DP[sw].bitRate			=	MP3DI.bitRate;
	MP3DP[sw].inputBuf			=	pFrameData + NowPos;
	MP3DP[sw].inputSize			=	MP3DI.header.padding ? MP3DI.maxInputSize : MP3DI.minInputSize;
	MP3DP[sw].outputBuf			=	(BYTE*)Buffer[sw] + (MP3DI.outputSize * i);
	MP3DP[sw].outputSize		=	MP3DI.outputSize;
	mp3DecodeFrame(&MP3DP[sw]);

	NowPos += MP3DI.header.padding ? MP3DI.maxInputSize : MP3DI.minInputSize;
	}

	ZeroMemory(&PCMwh[sw],sizeof(WAVEHDR));
	PCMwh[sw].lpData			= (char*)Buffer[sw] + (msec2bytes(startSec) - (msec2frame(startSec) * MP3DI.outputSize));
	PCMwh[sw].dwBufferLength	= MP3DI.outputSize * BufferingNumber - (msec2bytes(startSec) - (msec2frame(startSec) * MP3DI.outputSize));

	waveOutPrepareHeader( WaveOut, &PCMwh[sw], sizeof(WAVEHDR));
	waveOutWrite( WaveOut, &PCMwh[sw], sizeof(WAVEHDR));

	ThreadHandle = CreateThread(NULL,0,DecodeFunc,(LPVOID)this,0,&ThreadID);
}

void RealSoundMP3::LoopPlay(DWORD start,DWORD end,DWORD loop,DWORD number)
{
	if(State != REALSOUND_OPENED) return;

	sw = 0;
	Partition = 0;
	startSec = start;
	endSec = end;
	loopSec = loop;
	NowLooping = false;
	LoopCount = number;
	LoopMode = 1;
	if (number == 0) { InfiniteLoop = true; }
	
	State = REALSOUND_PLAYING;

	if(startSec > endSec) { startSec = 0; endSec = (FileSize / Bitrate * 8); }
	if(startSec < 0) { startSec = 0; }
	if(loopSec < 0) { loopSec = 0; }
	if(startSec == 0 && endSec == 0 && loopSec == 0) { startSec = 0; endSec = (FileSize / Bitrate * 8); loopSec = 0; }
	if((FileSize / Bitrate * 8) < startSec) { startSec = 0; }
	if((FileSize / Bitrate * 8) < endSec) { endSec = (FileSize / Bitrate * 8); }
	if(loopSec > endSec) { loopSec = 0; }

	BufferingNumber = DefMp3MaxBuffers;
	if(BufferingNumber > (msec2frame(endSec) - msec2frame(startSec)) + 1) { BufferingNumber = msec2frame(endSec) - msec2frame(startSec) + 1; }

	PartitionNumber = (((msec2frame(endSec) - msec2frame(startSec)) + 1) / BufferingNumber);
	if(((msec2frame(endSec) - msec2frame(startSec)) + 1) % BufferingNumber == 0) { PartitionNumber--; }

	ZeroMemory(&MP3DP[sw],sizeof(MPEG_DECODE_PARAM));

	WORD i = 0;

	if(msec2frame(startSec) <= 9)
	{
		for(i = 0 ; i < msec2frame(startSec); i++)
		{
			mp3GetDecodeInfo(pFrameData + NowPos,FrameSize,&MP3DI,1);
			MP3DP[sw].header			=	MP3DI.header;
			MP3DP[sw].bitRate			=	MP3DI.bitRate;
			MP3DP[sw].inputBuf			=	pFrameData + NowPos;
			MP3DP[sw].inputSize			=	MP3DI.header.padding ? MP3DI.maxInputSize : MP3DI.minInputSize;
			MP3DP[sw].outputBuf			=	(BYTE*)Buffer[sw] + (MP3DI.outputSize * i);
			MP3DP[sw].outputSize		=	MP3DI.outputSize;	
			mp3DecodeFrame(&MP3DP[sw]);

			NowPos += MP3DI.header.padding ? MP3DI.maxInputSize : MP3DI.minInputSize;
		}
	}
	else if(msec2frame(startSec) > 9)
	{
		for(i = 0 ; i < msec2frame(startSec) - 9;i++)
		{
			mp3GetDecodeInfo(pFrameData + NowPos,FrameSize,&MP3DI,1);
			NowPos += MP3DI.header.padding ? MP3DI.maxInputSize : MP3DI.minInputSize;
		}

		for(i = 0 ; i < 9; i++)
		{
			mp3GetDecodeInfo(pFrameData + NowPos,FrameSize,&MP3DI,1);
			MP3DP[sw].header			=	MP3DI.header;
			MP3DP[sw].bitRate			=	MP3DI.bitRate;
			MP3DP[sw].inputBuf			=	pFrameData + NowPos;
			MP3DP[sw].inputSize			=	MP3DI.header.padding ? MP3DI.maxInputSize : MP3DI.minInputSize;
			MP3DP[sw].outputBuf			=	(BYTE*)Buffer[sw] + (MP3DI.outputSize * i);
			MP3DP[sw].outputSize		=	MP3DI.outputSize;	
			mp3DecodeFrame(&MP3DP[sw]);
			
			NowPos += MP3DI.header.padding ? MP3DI.maxInputSize : MP3DI.minInputSize;
		}
	}

	for(i = 0 ; i < BufferingNumber ; i++)
	{
	mp3GetDecodeInfo(pFrameData + NowPos,FrameSize,&MP3DI,1);
	MP3DP[sw].header			=	MP3DI.header;
	MP3DP[sw].bitRate			=	MP3DI.bitRate;
	MP3DP[sw].inputBuf			=	pFrameData + NowPos;
	MP3DP[sw].inputSize			=	MP3DI.header.padding ? MP3DI.maxInputSize : MP3DI.minInputSize;
	MP3DP[sw].outputBuf			=	(BYTE*)Buffer[sw] + (MP3DI.outputSize * i);
	MP3DP[sw].outputSize		=	MP3DI.outputSize;
	mp3DecodeFrame(&MP3DP[sw]);

	NowPos += MP3DI.header.padding ? MP3DI.maxInputSize : MP3DI.minInputSize;
	}

	ZeroMemory(&PCMwh[sw],sizeof(WAVEHDR));
	PCMwh[sw].lpData			= (char*)Buffer[sw] + (msec2bytes(startSec) - (msec2frame(startSec) * MP3DI.outputSize));
	PCMwh[sw].dwBufferLength	= MP3DI.outputSize * BufferingNumber - (msec2bytes(startSec) - (msec2frame(startSec) * MP3DI.outputSize));

	waveOutPrepareHeader( WaveOut, &PCMwh[sw], sizeof(WAVEHDR));
	waveOutWrite( WaveOut, &PCMwh[sw], sizeof(WAVEHDR));

	ThreadHandle = CreateThread(NULL,0,DecodeFunc,(LPVOID)this,0,&ThreadID);
}

void RealSoundMP3::Pause()
{
	if(State != REALSOUND_PLAYING) { return; }
	waveOutPause(WaveOut);
	State = REALSOUND_PAUSED;
}

void RealSoundMP3::Resume()
{
	if(State != REALSOUND_PAUSED) { return; }
	waveOutRestart(WaveOut);
	State = REALSOUND_PLAYING;
}

void RealSoundMP3::Stop()
{
	if(State == REALSOUND_OPENED) { return; }

	State = REALSOUND_OPENED;

	waveOutReset(WaveOut);
	waveOutUnprepareHeader(WaveOut, &PCMwh[0], sizeof(WAVEHDR));
	waveOutUnprepareHeader(WaveOut, &PCMwh[1], sizeof(WAVEHDR));
}

void CALLBACK RealSoundMP3::waveOutProc(HWAVE hwo,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
{
	RealSoundMP3 *pMP3 = (RealSoundMP3*)dwInstance;

	if(uMsg == WOM_DONE && pMP3->State == REALSOUND_PLAYING){
		pMP3->ThreadHandle = CreateThread(NULL,0,pMP3->DecodeFunc,(LPVOID)dwInstance,0,&pMP3->ThreadID);
	}
}

DWORD WINAPI RealSoundMP3::DecodeFunc(LPVOID dwInstance)
{
	WORD i = 0;

	RealSoundMP3 *pMP3 = (RealSoundMP3*)dwInstance;

	if(pMP3->Partition == pMP3->PartitionNumber - 1)
	{
		if(pMP3->Partition != 0 || pMP3->NowLooping == true)
		{
			pMP3->sw ^= 1;
			waveOutUnprepareHeader( pMP3->WaveOut, &pMP3->PCMwh[pMP3->sw], sizeof(WAVEHDR) );
			pMP3->sw ^= 1;
		}

		pMP3->sw ^= 1;
		pMP3->Partition++;
		ZeroMemory(pMP3->Buffer[pMP3->sw],pMP3->MP3DI.outputSize * pMP3->BufferingNumber);

		for(i = 0 ; i < pMP3->msec2frame(pMP3->endSec) - pMP3->msec2frame(pMP3->startSec) - (pMP3->BufferingNumber * pMP3->Partition) + 1 ; i++)
		{
		mp3GetDecodeInfo(pMP3->pFrameData + pMP3->NowPos,pMP3->FrameSize,&pMP3->MP3DI,1);
		pMP3->MP3DP[pMP3->sw].header			=	pMP3->MP3DI.header;
		pMP3->MP3DP[pMP3->sw].bitRate			=	pMP3->MP3DI.bitRate;
		pMP3->MP3DP[pMP3->sw].inputBuf			=	pMP3->pFrameData + pMP3->NowPos;
		pMP3->MP3DP[pMP3->sw].inputSize			=	pMP3->MP3DI.header.padding ? pMP3->MP3DI.maxInputSize : pMP3->MP3DI.minInputSize;
		pMP3->MP3DP[pMP3->sw].outputBuf			=	(BYTE*)pMP3->Buffer[pMP3->sw] + (pMP3->MP3DI.outputSize * i);
		pMP3->MP3DP[pMP3->sw].outputSize		=	pMP3->MP3DI.outputSize;	
		mp3DecodeFrame(&pMP3->MP3DP[pMP3->sw]);

		pMP3->NowPos += pMP3->MP3DI.header.padding ? pMP3->MP3DI.maxInputSize : pMP3->MP3DI.minInputSize;
		}
		
		ZeroMemory(&pMP3->PCMwh[pMP3->sw],sizeof(WAVEHDR));
		pMP3->PCMwh[pMP3->sw].lpData			= (char*)pMP3->Buffer[pMP3->sw];
		pMP3->PCMwh[pMP3->sw].dwBufferLength	= pMP3->msec2bytes(pMP3->endSec) - pMP3->msec2bytes(pMP3->startSec) - pMP3->MP3DI.outputSize * (pMP3->BufferingNumber * pMP3->Partition);

		waveOutPrepareHeader( pMP3->WaveOut, &pMP3->PCMwh[pMP3->sw], sizeof(WAVEHDR));
		waveOutWrite( pMP3->WaveOut, &pMP3->PCMwh[pMP3->sw], sizeof(WAVEHDR));

		return 0;
	}

	if(pMP3->Partition < pMP3->PartitionNumber)
	{
		if(pMP3->Partition != 0 || pMP3->NowLooping == true)
		{
			pMP3->sw ^= 1;
			waveOutUnprepareHeader( pMP3->WaveOut, &pMP3->PCMwh[pMP3->sw], sizeof(WAVEHDR) );
			pMP3->sw ^= 1;
		}
			
		pMP3->sw ^= 1;
		pMP3->Partition++;

		for(i = 0 ; i < pMP3->BufferingNumber ; i++)
		{
		mp3GetDecodeInfo(pMP3->pFrameData + pMP3->NowPos,pMP3->FrameSize,&pMP3->MP3DI,1);
		pMP3->MP3DP[pMP3->sw].header			=	pMP3->MP3DI.header;
		pMP3->MP3DP[pMP3->sw].bitRate			=	pMP3->MP3DI.bitRate;
		pMP3->MP3DP[pMP3->sw].inputBuf			=	pMP3->pFrameData + pMP3->NowPos;
		pMP3->MP3DP[pMP3->sw].inputSize			=	pMP3->MP3DI.header.padding ? pMP3->MP3DI.maxInputSize : pMP3->MP3DI.minInputSize;
		pMP3->MP3DP[pMP3->sw].outputBuf			=	(BYTE*)pMP3->Buffer[pMP3->sw] + (pMP3->MP3DI.outputSize * i);
		pMP3->MP3DP[pMP3->sw].outputSize		=	pMP3->MP3DI.outputSize;	
		mp3DecodeFrame(&pMP3->MP3DP[pMP3->sw]);

		pMP3->NowPos += pMP3->MP3DI.header.padding ? pMP3->MP3DI.maxInputSize : pMP3->MP3DI.minInputSize;
		}

		ZeroMemory(&pMP3->PCMwh[pMP3->sw],sizeof(WAVEHDR));
		pMP3->PCMwh[pMP3->sw].lpData			= (char*)pMP3->Buffer[pMP3->sw];
		pMP3->PCMwh[pMP3->sw].dwBufferLength	= pMP3->MP3DI.outputSize * pMP3->BufferingNumber;

		waveOutPrepareHeader( pMP3->WaveOut, &pMP3->PCMwh[pMP3->sw], sizeof(WAVEHDR));
		waveOutWrite( pMP3->WaveOut, &pMP3->PCMwh[pMP3->sw], sizeof(WAVEHDR));

		return 0;
	}
	
	if(pMP3->Partition == pMP3->PartitionNumber)
	{
		if(pMP3->Partition != 0 || pMP3->NowLooping == true)
		{
			pMP3->sw ^= 1;
			waveOutUnprepareHeader( pMP3->WaveOut, &pMP3->PCMwh[pMP3->sw], sizeof(WAVEHDR) );
			pMP3->sw ^= 1;
		}

		pMP3->Partition++;

		if(pMP3->LoopMode == 1)
		{
			if(pMP3->LoopCount == 0 && pMP3->InfiniteLoop == false) { _endthread(); return 0; }

			pMP3->LoopCount--;
			pMP3->sw ^= 1;
			pMP3->BufferingNumber = DefMp3MaxBuffers;

			if(pMP3->BufferingNumber > (pMP3->msec2frame(pMP3->endSec) - pMP3->msec2frame(pMP3->loopSec)) + 1) { pMP3->BufferingNumber = pMP3->msec2frame(pMP3->endSec) - pMP3->msec2frame(pMP3->loopSec) + 1; }

			pMP3->PartitionNumber = (((pMP3->msec2frame(pMP3->endSec) - pMP3->msec2frame(pMP3->loopSec)) + 1) / pMP3->BufferingNumber);
			if(((pMP3->msec2frame(pMP3->endSec) - pMP3->msec2frame(pMP3->loopSec)) + 1) % pMP3->BufferingNumber == 0) { pMP3->PartitionNumber--; }

			pMP3->Partition = 0;
			pMP3->NowPos = 0;

			ZeroMemory(&pMP3->MP3DP[pMP3->sw],sizeof(MPEG_DECODE_PARAM));

			if(pMP3->msec2frame(pMP3->loopSec) <= 9)
			{
				for(i = 0 ; i < pMP3->msec2frame(pMP3->loopSec); i++)
				{
					mp3GetDecodeInfo(pMP3->pFrameData + pMP3->NowPos,pMP3->FrameSize,&pMP3->MP3DI,1);
					pMP3->MP3DP[pMP3->sw].header			=	pMP3->MP3DI.header;
					pMP3->MP3DP[pMP3->sw].bitRate			=	pMP3->MP3DI.bitRate;
					pMP3->MP3DP[pMP3->sw].inputBuf			=	pMP3->pFrameData + pMP3->NowPos;
					pMP3->MP3DP[pMP3->sw].inputSize			=	pMP3->MP3DI.header.padding ? pMP3->MP3DI.maxInputSize : pMP3->MP3DI.minInputSize;
					pMP3->MP3DP[pMP3->sw].outputBuf			=	(BYTE*)pMP3->Buffer[pMP3->sw] + (pMP3->MP3DI.outputSize * i);
					pMP3->MP3DP[pMP3->sw].outputSize		=	pMP3->MP3DI.outputSize;	
					mp3DecodeFrame(&pMP3->MP3DP[pMP3->sw]);

					pMP3->NowPos += pMP3->MP3DI.header.padding ? pMP3->MP3DI.maxInputSize : pMP3->MP3DI.minInputSize;
				}
			}
			else if(pMP3->msec2frame(pMP3->loopSec) > 9)
			{
				for(i = 0 ; i < pMP3->msec2frame(pMP3->loopSec) - 9;i++)
				{
					mp3GetDecodeInfo(pMP3->pFrameData + pMP3->NowPos,pMP3->FrameSize,&pMP3->MP3DI,1);
					pMP3->NowPos += pMP3->MP3DI.header.padding ? pMP3->MP3DI.maxInputSize : pMP3->MP3DI.minInputSize;
				}

				for(i = 0 ; i < 9; i++)
				{
					mp3GetDecodeInfo(pMP3->pFrameData + pMP3->NowPos,pMP3->FrameSize,&pMP3->MP3DI,1);
					pMP3->MP3DP[pMP3->sw].header			=	pMP3->MP3DI.header;
					pMP3->MP3DP[pMP3->sw].bitRate			=	pMP3->MP3DI.bitRate;
					pMP3->MP3DP[pMP3->sw].inputBuf			=	pMP3->pFrameData + pMP3->NowPos;
					pMP3->MP3DP[pMP3->sw].inputSize			=	pMP3->MP3DI.header.padding ? pMP3->MP3DI.maxInputSize : pMP3->MP3DI.minInputSize;
					pMP3->MP3DP[pMP3->sw].outputBuf			=	(BYTE*)pMP3->Buffer[pMP3->sw] + (pMP3->MP3DI.outputSize * i);
					pMP3->MP3DP[pMP3->sw].outputSize		=	pMP3->MP3DI.outputSize;	
					mp3DecodeFrame(&pMP3->MP3DP[pMP3->sw]);

					pMP3->NowPos += pMP3->MP3DI.header.padding ? pMP3->MP3DI.maxInputSize : pMP3->MP3DI.minInputSize;
				}
			}

			for(i = 0 ; i < pMP3->BufferingNumber ; i++)
			{
			mp3GetDecodeInfo(pMP3->pFrameData + pMP3->NowPos,pMP3->FrameSize,&pMP3->MP3DI,1);
			pMP3->MP3DP[pMP3->sw].header			=	pMP3->MP3DI.header;
			pMP3->MP3DP[pMP3->sw].bitRate			=	pMP3->MP3DI.bitRate;
			pMP3->MP3DP[pMP3->sw].inputBuf			=	pMP3->pFrameData + pMP3->NowPos;
			pMP3->MP3DP[pMP3->sw].inputSize			=	pMP3->MP3DI.header.padding ? pMP3->MP3DI.maxInputSize : pMP3->MP3DI.minInputSize;
			pMP3->MP3DP[pMP3->sw].outputBuf			=	(BYTE*)pMP3->Buffer[pMP3->sw] + (pMP3->MP3DI.outputSize * i);
			pMP3->MP3DP[pMP3->sw].outputSize		=	pMP3->MP3DI.outputSize;
			mp3DecodeFrame(&pMP3->MP3DP[pMP3->sw]);

			pMP3->NowPos += pMP3->MP3DI.header.padding ? pMP3->MP3DI.maxInputSize : pMP3->MP3DI.minInputSize;
			}

			ZeroMemory(&pMP3->PCMwh[pMP3->sw],sizeof(WAVEHDR));
			pMP3->PCMwh[pMP3->sw].lpData			= (char*)pMP3->Buffer[pMP3->sw] + (pMP3->msec2bytes(pMP3->loopSec) - (pMP3->msec2frame(pMP3->loopSec) * pMP3->MP3DI.outputSize));
			pMP3->PCMwh[pMP3->sw].dwBufferLength	= pMP3->MP3DI.outputSize * pMP3->BufferingNumber - (pMP3->msec2bytes(pMP3->loopSec) - (pMP3->msec2frame(pMP3->loopSec) * pMP3->MP3DI.outputSize));

			waveOutPrepareHeader( pMP3->WaveOut, &pMP3->PCMwh[pMP3->sw], sizeof(WAVEHDR));
			waveOutWrite( pMP3->WaveOut, &pMP3->PCMwh[pMP3->sw], sizeof(WAVEHDR));
			
			pMP3->startSec = pMP3->loopSec;
			pMP3->NowLooping = true;

				return 0;
		}

		return 0;
	}
		
	if(pMP3->Partition > pMP3->PartitionNumber)
	{
		waveOutUnprepareHeader( pMP3->WaveOut, &pMP3->PCMwh[pMP3->sw], sizeof(WAVEHDR) );
			
		pMP3->State = REALSOUND_OPENED;
		pMP3->LoopMode = 0;

		return 0;
	}

	return 0;
}

void RealSoundMP3::SetEqualizer(REALSOUND_EQ_PARAM eq)
{
	int tEQ[10];

	if(State == REALSOUND_CLOSED) { return; }

	tEQ[0] = eq._60;
	tEQ[1] = eq._170;
	tEQ[2] = eq._310;
	tEQ[3] = eq._600;
	tEQ[4] = eq._1k;
	tEQ[5] = eq._3k;
	tEQ[6] = eq._6k;
	tEQ[7] = eq._12k;
	tEQ[8] = eq._14k;
	tEQ[9] = eq._16k;
	
	mp3SetEqualizer(&tEQ[0]);
}

inline DWORD RealSoundMP3::msec2frame(DWORD msec)
{
	float spf[] = {26.12245f, 24.0f, 36.0f};

	return (DWORD)(msec / spf[ (pFrameData[2] >> 2) & 3 ]);
}

inline DWORD RealSoundMP3::frame2msec(DWORD frame)
{
	return frame * ((FrameSize / Bitrate) * 8);
}

inline DWORD RealSoundMP3::msec2bytes(DWORD msec)
{
	return (DWORD)((msec * (Frequency * Channel * 2 / 1000.0f)) + ((DWORD)(msec * (Frequency * Channel * 2 / 1000.0f)) % (DWORD)(Frequency * Channel * 2 / 1000.0f)));
}