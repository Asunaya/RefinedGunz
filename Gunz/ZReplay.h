#pragma once

#include "ZCharacter.h"
#include "ZReplayStructs.h"
#include "ArrayView.h"
#include "RGTypes.h"

bool CreateReplayGame(const char *filename);


#define GUNZ_REC_FILE_ID		0x95b1308a

#define GUNZ_REC_FILE_VERSION	4
#define GUNZ_REC_FILE_EXT		"gzr"

#define RG_REPLAY_MAGIC_NUMBER 0x00DEFBAD
#define RG_REPLAY_BINARY_VERSION 2

enum class SERVER
{
	NONE,
	OFFICIAL, // igunz, ijji gunz, aeria gunz
	REFINED_GUNZ,
	FREESTYLE_GUNZ,
	DarkGunz,
	MAX,
};

struct ReplayVersion
{
	SERVER Server;
	int nVersion;
	int nSubVersion;

	const char* GetServerString()
	{
		switch (Server)
		{
		case SERVER::OFFICIAL:
			return "Official";
		case SERVER::FREESTYLE_GUNZ:
			return "Freestyle Gunz";
		case SERVER::REFINED_GUNZ:
			return "Refined Gunz";
		case SERVER::DarkGunz:
			return "Dark Gunz";
		case SERVER::NONE:
		default:
			return "Unknown";
		}
	}

	std::string GetVersionString()
	{
		std::string ret = "Version: ";
		ret += GetServerString();
		ret += " V";
		ret += std::to_string(nVersion);

		if (Server == SERVER::FREESTYLE_GUNZ && nVersion == 7)
		{
			ret += ".";
			ret += std::to_string(nSubVersion);
		}

		return ret;
	}
};

class ZGame;

class ZReplayLoader
{
public:
	ZReplayLoader();

	bool LoadFile(const char* FileName);

	float GetGameTime() const { return m_fGameTime; }

	ReplayVersion GetVersion();
	time_t GetTimestamp() const { return Timestamp; }
	void GetStageSetting(REPLAY_STAGE_SETTING_NODE &ret);
	void GetDuelQueueInfo(MTD_DuelQueueInfo* QueueInfo = nullptr);
	std::vector<ReplayPlayerInfo> GetCharInfo();
	template <typename T>
	bool GetCommands(T ForEachCommand, bool PersistentMCommands, ArrayView<u32>* WantedCommandIDs = nullptr);

	int GetPosition() const { return Position; }

private:
	std::vector<unsigned char> InflatedFile;

	ReplayVersion Version;
	float m_fGameTime = 0.f;
	int Position = 0;
	bool IsDojo = false;
	MMATCH_GAMETYPE GameType;
	time_t Timestamp = 0;

	template <typename T>
	void Read(T& Obj);
	template <typename T>
	void ReadAt(T& Obj, int Position);
	template <typename T>
	void Peek(T& Obj);
	void ReadN(void* Obj, size_t Size);
	template <typename T>
	bool TryRead(T& Obj);

	template <typename T>
	bool GetCommandsImpl(T fn, ArrayView<u32>* WantedCommandIDs);
	template <typename T = std::allocator<uint8_t>>
	bool CreateCommandFromStream(const char* pStream, MCommand& Command, T& Alloc = T());
	bool FixCommand(MCommand& Command);
	static bool CreateCommandFromStreamVersion2(const char* pStream, MCommand& Command);
	static bool ParseVersion2Command(const char* pStream, MCommand* pCmd);
	static MCommandParameter* MakeVersion2CommandParameter(MCommandParameterType nType, const char* pStream, unsigned short int* pnDataCount);
};
