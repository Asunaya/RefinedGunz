#pragma once

#include <thread>
#include <array>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#undef OPUS_BUILD
#define OPUS_EXPORT
#include "opus.h"
#include "../portaudio/include/portaudio.h"

class VoiceChat
{
public:
	VoiceChat();
	~VoiceChat();
	VoiceChat(const VoiceChat &) = delete;

	void OnCreateDevice();
	void OnReset();

	void StartRecording();
	void StopRecording();

	void OnReceiveVoiceChat(ZCharacter *Char, const BYTE *Buffer, int Length);

	void OnDestroyCharacter(ZCharacter *Char);

	bool MutePlayer(const MUID& UID);

	void Draw();

	static constexpr int SampleRate = 48000;
	static constexpr int FrameSize = SampleRate * 0.06; // 60 ms
	static constexpr int NumChannels = 1;
	typedef short SampleFormat;

private:
#ifdef WAVEIN
	void ThreadLoop();
	std::thread thr;
#endif

	static int PlayCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);
	static int RecordCallbackWrapper(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);
	int RecordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

	bool CanRecord = false;
	bool CanDecode = false;
	bool CanPlay = false;

	bool Recording = false;

#ifdef WAVEIN
	static constexpr int NumBuffers = 2;

	std::array<std::array<SampleFormat, FrameSize>, NumBuffers> FrameBuffers;
	std::array<std::array<unsigned char, FrameSize * sizeof(short)>, NumBuffers> EncodedFrames;
	int EncodedSizes[2];

	int CurrentBufferIndex = 0;
#endif

	OpusEncoder *pOpusEncoder;
	OpusDecoder *pOpusDecoder;

	//MicStream test;

	PaStream *InputStream = nullptr;

#ifdef WAVEIN
	HWAVEIN WaveIn;

	WAVEHDR wavehdrs[2];
#endif

	struct MicFrame
	{
		short pcm[FrameSize];
	};

	class MicStuff
	{
	public:
		std::queue<MicFrame> Data;
		std::mutex QueueMutex;
		int SampleRate = 48000;
		bool Streaming = false;
		PaStream *Stream;

		MicStuff() = default;
		MicStuff(PaStream *s) : Stream(s) { }
		// std::mutex has neither copy ctor nor move ctor
		MicStuff(const MicStuff &rhs)
		{
			Data = rhs.Data;
			SampleRate = rhs.SampleRate;
			Streaming = rhs.Streaming;
			Stream = rhs.Stream;
		}
	};

	std::unordered_map<ZCharacter*, MicStuff> MicStreams;
	std::unordered_set<MUID> MutedPlayers;

	IDirect3DTexture9* SpeakerTexture;
};

extern VoiceChat g_VoiceChat;