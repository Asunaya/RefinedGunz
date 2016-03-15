#ifndef _ZREPLAY_H
#define _ZREPLAY_H

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
};

class ZGame;

class ZReplayLoader
{
private:
	ZFile *pFile;
	ReplayVersion Version;
	REPLAY_STAGE_SETTING_NODE m_StageSetting;
	float m_fGameTime;
	BYTE *InflatedFile;
	bool bDojo;

	bool LoadHeader();
	bool LoadStageSetting();
	bool LoadStageSettingEtc();
	bool LoadCharInfo();
	bool LoadCommandStream();

	void ConvertStageSettingNode(REPLAY_STAGE_SETTING_NODE* pSource, MSTAGE_SETTING_NODE* pTarget);
	void ChangeGameState();

	bool CreateCommandFromStream(char* pStream, MCommand **pRetCommand);
	MCommand* CreateCommandFromStreamVersion2(char* pStream);
	bool ParseVersion2Command(char* pStream, MCommand* pCmd);
	MCommandParameter* MakeVersion2CommandParameter(MCommandParameterType nType, char* pStream, unsigned short int* pnDataCount);
public:
	ZReplayLoader();
	~ZReplayLoader();
	bool Load(const char* filename);	
	float GetGameTime() const { return m_fGameTime; }
};






#endif