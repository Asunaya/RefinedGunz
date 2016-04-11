#include "stdafx.h"
#include "VoiceChat.h"
#include "RGMain.h"
#include "Draw.h"

VoiceChat g_VoiceChat;

template <typename SampleType = VoiceChat::SampleFormat>
static inline unsigned long GetSampleFormat()
{
	static_assert(false, "Unkwown type");
}

template <> static inline unsigned long GetSampleFormat<char>()				{ return paInt8; }
template <> static inline unsigned long GetSampleFormat<unsigned char>()	{ return paUInt8; }
template <> static inline unsigned long GetSampleFormat<short>()			{ return paInt16; }
template <> static inline unsigned long GetSampleFormat<int>()				{ return paInt32; }
template <> static inline unsigned long GetSampleFormat<float>()			{ return paFloat32; }

VoiceChat::VoiceChat()
{
#ifdef WAVEIN
	thr = std::thread([this]()
	{
		while (true)
		{
			ThreadLoop();
		}
	});
#endif

	int error;

	CanPlay = true;

	error = Pa_Initialize();
	if (error != paNoError)
	{
		MLog("Pa_Initialize failed with error code %d: %s\n", error, Pa_GetErrorText(error));

		CanPlay = false;

#ifndef WAVEIN
		CanRecord = true;
#endif

		return;
	}

	[&] {
		CanRecord = true;

#ifdef WAVEIN
		WAVEFORMATEX Format;
		Format.wFormatTag = WAVE_FORMAT_PCM;
		Format.nChannels = 1;
		Format.nSamplesPerSec = SampleRate;
		Format.wBitsPerSample = 16;
		Format.nAvgBytesPerSec = Format.nSamplesPerSec * Format.nChannels * Format.wBitsPerSample / 8;
		Format.nBlockAlign = Format.nChannels * Format.wBitsPerSample / 8;
		Format.cbSize = 0;

		auto ret = waveInOpen(&WaveIn, WAVE_MAPPER, &Format, (DWORD_PTR)GetThreadId(thr.native_handle()), 0, WAVE_FORMAT_DIRECT | CALLBACK_THREAD);

		if (ret != MMSYSERR_NOERROR)
		{
			char Reason[256];

			waveInGetErrorText(ret, Reason, sizeof(Reason));

			MLog("waveInOpen failed: %s\n", Reason);

			CanRecord = false;

			return;
		}
#else
		error = Pa_OpenDefaultStream(&InputStream, NumChannels, 0, GetSampleFormat(), SampleRate, FrameSize, &RecordCallbackWrapper, nullptr);

		if (error != paNoError)
		{
			MLog("Pa_OpenStream failed with error code %d: %s\n", error, Pa_GetErrorText(error));

			CanRecord = false;

			return;
		}
#endif

		pOpusEncoder = opus_encoder_create(SampleRate, NumChannels, OPUS_APPLICATION_VOIP, &error);

		if (error != OPUS_OK)
		{
			MLog("opus_encoder_create failed with error code %d: %s\n", error, opus_strerror(error));

			CanRecord = false;

			return;
		}
	}();

	[&] {
		CanDecode = true;

		pOpusDecoder = opus_decoder_create(SampleRate, 1, &error);

		if (error != OPUS_OK)
		{
			MLog("opus_decoder_create failed with error code %d: %s\n", error, opus_strerror(error));

			CanDecode = false;
		}
	}();
}

VoiceChat::~VoiceChat()
{
#ifdef WAVEIN
	waveInClose(WaveIn);
#endif

	opus_encoder_destroy(pOpusEncoder);
	opus_decoder_destroy(pOpusDecoder);
}

void VoiceChat::StartRecording()
{
	if (Recording || !CanRecord || ZGetGame()->IsReplay())
		return;

	//MLog("StartRecording\n");

	//test = ZGetSoundFMod()->OpenMicStream(SampleRate);

#ifdef WAVEIN
	for (int i = 0; i < 2; i++)
	{
		WAVEHDR &wavehdr = wavehdrs[i];
		wavehdr.lpData = (LPSTR)FrameBuffers[i].data();
		wavehdr.dwBufferLength = sizeof(FrameBuffers[i]);
		wavehdr.dwBytesRecorded = 0;
		wavehdr.dwUser = 0;
		wavehdr.dwFlags = 0;
		wavehdr.dwLoops = 0;
		//MLog("size: %d\n", wavehdr.dwBufferLength);

		auto result = waveInPrepareHeader(WaveIn, &wavehdr, sizeof(WAVEHDR));
		if (result)
		{
			MLog("waveInPrepareHeader failed: %d\n", result);
			return;
		}

		result = waveInAddBuffer(WaveIn, &wavehdr, sizeof(WAVEHDR));
		if (result)
		{
			MLog("waveInAddBuffer failed: %d\n", result);
			return;
		}
	};

	MLog("Added buffers\n");

	waveInStart(WaveIn);

	MLog("Called waveInStart\n");
#else
	if (!Pa_IsStreamActive(InputStream))
	{
		Pa_StopStream(InputStream);
		Pa_StartStream(InputStream);
	}
#endif

	Recording = true;
}

void VoiceChat::StopRecording()
{
	if (!Recording || !CanRecord || ZGetGame()->IsReplay())
		return;

	//MLog("StopRecording\n");

#ifdef WAVEIN
	waveInReset(WaveIn);
	waveInStop(WaveIn);
#else
	Pa_StopStream(InputStream);
#endif

	Recording = false;
}

void VoiceChat::OnReceiveVoiceChat(ZCharacter *Char, const BYTE *Buffer, int Length)
{
	if (!CanDecode)
		return;

	if (MutedPlayers.find(Char->GetUID()) != MutedPlayers.end())
		return;

	//MLog("OnReceiveVoiceChat size %d\n", Length);

	//short DecodedFrame[FrameSize];

	MicFrame mf;

	int ret = opus_decode(pOpusDecoder, Buffer, Length, mf.pcm, FrameSize, 0);

	if (ret < 0)
	{
		MLog("opus_decode failed with error code %d: %s\n", ret, opus_strerror(ret));
		return;
	}

	auto it = MicStreams.find(Char);

	if (it == MicStreams.end())
	{
		PaStream *Stream = nullptr;

		auto error = Pa_OpenDefaultStream(
			&Stream,
			0,
			1,
			paInt16,
			SampleRate,
			FrameSize,
			//paClipOff,
			//paPrimeOutputBuffersUsingStreamCallback,
			PlayCallback,
			Char);

		if (error != paNoError)
		{
			MLog("Pa_OpenStream failed with error code %d: %s\n", error, Pa_GetErrorText(error));

			//return;
		}

		it = MicStreams.insert({ Char, MicStuff(Stream) }).first;

		MLog("Inserted stream, size %d\n", MicStreams.size());

		return;
	}

	if (!it->second.Stream) // Dead stream object that failed to create
		return;

	{
		std::lock_guard<std::mutex> lock(it->second.QueueMutex);

		it->second.Data.push(mf);
	}

	if (!Pa_IsStreamActive(it->second.Stream) && it->second.Data.size() > 2)
	{
		Pa_StopStream(it->second.Stream);
		Pa_StartStream(it->second.Stream);
		it->second.Streaming = true;
	}

	//ZGetSoundFMod()->AddMicData(test, DecodedFrame, FrameSize);

	//MLog("OnReceiveVoiceChat finished\n");
}

void VoiceChat::OnDestroyCharacter(ZCharacter * Char)
{
	auto it = MicStreams.find(Char);

	if (it == MicStreams.end())
		return;

	MicStreams.erase(it);
}

bool VoiceChat::MutePlayer(const MUID & UID)
{
	auto it = MutedPlayers.find(UID);

	if (it == MutedPlayers.end())
	{
		MutedPlayers.insert(UID);
		return true;
	}

	MutedPlayers.erase(it);

	return false;
}

#ifdef WAVEIN
void VoiceChat::ThreadLoop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, MM_WIM_DATA, MM_WIM_DATA) == 1)
	{
		WAVEHDR &wavehdr = *LPWAVEHDR(msg.lParam);

		EncodedSizes[CurrentBufferIndex] = opus_encode(pOpusEncoder, (const opus_int16 *)FrameBuffers[CurrentBufferIndex].data(), FrameSize, &EncodedFrames[CurrentBufferIndex][0], sizeof(EncodedFrames[0]));

		if (EncodedSizes[CurrentBufferIndex] < 0)
		{
			MLog("opus_encode failed with error code %d: %s\n", EncodedSizes[CurrentBufferIndex], opus_strerror(EncodedSizes[CurrentBufferIndex]));
			return;
		}

		//MLog("Encoded data, size %d\n", EncodedSizes[CurrentBufferIndex]);

		auto lambda = [EncodedFrame = EncodedFrames[CurrentBufferIndex], Size = EncodedSizes[CurrentBufferIndex]]
		{
			ZPostVoiceChat(EncodedFrame.data(), Size);
		};

		g_RGMain.Invoke(lambda, 2);

		CurrentBufferIndex = (CurrentBufferIndex + 1) % NumBuffers;

		wavehdr.lpData = LPSTR(FrameBuffers[CurrentBufferIndex].data());

		waveInAddBuffer(WaveIn, &wavehdr, sizeof(WAVEHDR));
	}
}
#else

int VoiceChat::RecordCallbackWrapper(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	return g_VoiceChat.RecordCallback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags, userData);
}

int VoiceChat::RecordCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	if (!Recording)
		return paComplete;

	std::array<unsigned char, FrameSize> EncodedFrame;
	auto Size = opus_encode(pOpusEncoder, (const opus_int16 *)inputBuffer, FrameSize, EncodedFrame.data(), sizeof(EncodedFrame));

	if (Size < 0)
	{
		MLog("opus_encode failed with error code %d: %s\n", Size, opus_strerror(Size));
		return paContinue;
	}

	//MLog("Encoded data, size %d\n", Size);

	auto lambda = [EncodedFrame, Size]
	{
		ZPostVoiceChat(EncodedFrame.data(), Size);
	};

	g_RGMain.Invoke(lambda);

	return paContinue;
}

#endif

int VoiceChat::PlayCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	auto it = g_VoiceChat.MicStreams.find((ZCharacter *)userData);

	if (it == g_VoiceChat.MicStreams.end())
		return paComplete;

	//MLog("Play! Size: %d\n", g_VoiceChat.MicStreams.size());

	{
		std::lock_guard<std::mutex> lock(it->second.QueueMutex);

		auto &Queue = it->second.Data;

		if (Queue.empty())
		{
			MLog("Empty!\n");
			it->second.Streaming = false;
			return paComplete;
		}

		auto &p = Queue.front();

		memcpy(outputBuffer, p.pcm, sizeof(p.pcm));

		Queue.pop();
	}

	//MLog("Added more stuff! statusFlags: %d, time: %f, framesPerBuffer: %d\n", statusFlags, timeInfo->currentTime, framesPerBuffer);

	return paContinue;
}

void VoiceChat::OnCreateDevice()
{
	auto ret = ReadMZFile("Interface/default/SpeakerIcon.png");

	if (!ret.first)
		return MLog("Failed to load speaker icon texture file\n");

	auto hr = D3DXCreateTextureFromFileInMemory(RGetDevice(), ret.second.data(), ret.second.size(), &SpeakerTexture);

	if (FAILED(hr))
		return MLog("Failed to create speaker icon texture\n");
}

void VoiceChat::Draw()
{
	//MLog("VoiceChat::Draw() %d\n", MicStreams.size());

	int i = 0;

	auto DrawStuff = [&](ZCharacter* Player)
	{
		D3DXVECTOR2 TopLeft(RELWIDTH(1920 - 400), RELHEIGHT(1080 / 2 + i * 100));
		D3DXVECTOR2 Extents(RELWIDTH(300), RELHEIGHT(50));

		auto color = Player->GetTeamID() == MMT_BLUE ? 0xC000A5C3 : 0xC0FF0000;

		g_Draw.Quad(TopLeft, TopLeft + Extents, color);

		D3DXVECTOR2 TextOffset(RELWIDTH(50), RELHEIGHT(10));

		auto v = TopLeft + TextOffset;

		g_Draw.Text(Player->GetUserNameA(), v.x, v.y);

		D3DXVECTOR2 SpeakerIconOrigin = TopLeft + D3DXVECTOR2(RELWIDTH(10), RELHEIGHT(10));
		D3DXVECTOR2 SpeakerIconExtents(RELWIDTH(30), RELHEIGHT(30));

		g_Draw.TexturedQuad(SpeakerIconOrigin, SpeakerIconOrigin + SpeakerIconExtents, SpeakerTexture);

		i++;
	};

	if (Recording)
		DrawStuff(ZGetGame()->m_pMyCharacter);

	for (auto item : MicStreams)
	{
		if (!item.second.Streaming)
			continue;

		DrawStuff(item.first);
	}
}