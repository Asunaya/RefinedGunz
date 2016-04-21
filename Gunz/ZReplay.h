#ifndef _ZREPLAY_H
#define _ZREPLAY_H

#include "ZCharacter.h"
#include "ZReplayStructs.h"

struct REPLAY_STAGE_SETTING_NODE
{
	MUID				uidStage;
	char				szStageName[64];
	char				szMapName[MAPNAME_LENGTH];
	char				nMapIndex;
	MMATCH_GAMETYPE		nGameType;
	int					nRoundMax;
	int					nLimitTime;
	int					nLimitLevel;
	int					nMaxPlayers;
	bool				bTeamKillEnabled;
	bool				bTeamWinThePoint;
	bool				bForcedEntryEnabled;
	bool				bAutoTeamBalancing;
};

extern bool g_bTestFromReplay;

bool CreateReplayGame(char *filename);


// 건즈 리플레이 파일
#define GUNZ_REC_FILE_ID		0x95b1308a

// version 4 : duel 모드의 현재 상태 저장이 추가되었습니다.
#define GUNZ_REC_FILE_VERSION	4
#define GUNZ_REC_FILE_EXT		"gzr"

#define RG_REPLAY_MAGIC_NUMBER 0x00DEFBAD
#define RG_REPLAY_BINARY_VERSION 1

class ZReplay
{
private:
public:
	ZReplay() {}
	~ZReplay() {}

};

enum SERVER
{
	SERVER_NONE,
	SERVER_OFFICIAL, // igunz, ijji gunz, aeria gunz
	SERVER_REFINED_GUNZ,
	SERVER_FREESTYLE_GUNZ,
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
		case SERVER_OFFICIAL:
			return "Official";
		case SERVER_FREESTYLE_GUNZ:
			return "Freestyle Gunz";
		case SERVER_REFINED_GUNZ:
			return "Refined Gunz";
		case SERVER_NONE:
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

		if (Server == SERVER_FREESTYLE_GUNZ && nVersion == 7)
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
	~ZReplayLoader();
	bool Load(const char* filename);
	float GetGameTime() const { return m_fGameTime; }

	ReplayVersion GetVersion();
	void GetStageSetting(REPLAY_STAGE_SETTING_NODE &ret);
	void GetDuelQueueInfo(MTD_DuelQueueInfo* QueueInfo = nullptr);
	std::vector<ReplayPlayerInfo> GetCharInfo();
	template <typename T>
	bool GetCommands(T ForEachCommand);

private:
	ZFile *pFile = nullptr;
	ReplayVersion Version;
	REPLAY_STAGE_SETTING_NODE m_StageSetting;
	float m_fGameTime = 0.f;
	std::vector<unsigned char> InflatedFile;
	int Position = 0;
	bool IsDojo = false;

	template <typename T>
	void Read(T& Obj);
	template <typename T>
	void ReadAt(T& Obj, int Position);
	template <typename T>
	void Peek(T& Obj);
	void ReadN(void* Obj, size_t Size);
	template <typename T>
	bool TryRead(T& Obj);

	void CreatePlayers(const std::vector<ReplayPlayerInfo>& Players);

	void ConvertStageSettingNode(REPLAY_STAGE_SETTING_NODE* pSource, MSTAGE_SETTING_NODE* pTarget);
	void ChangeGameState();

	bool CreateCommandFromStream(char* pStream, MCommand **ppRetCommand);
	static MCommand* CreateCommandFromStreamVersion2(char* pStream);
	static bool ParseVersion2Command(char* pStream, MCommand* pCmd);
	static MCommandParameter* MakeVersion2CommandParameter(MCommandParameterType nType, char* pStream, unsigned short int* pnDataCount);
};





#endif