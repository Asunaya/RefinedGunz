#ifndef _ZREPLAY_H
#define _ZREPLAY_H

struct REPLAY_STAGE_SETTING_NODE 
{
	MUID				uidStage;
	char				szMapName[MAPNAME_LENGTH];	// 맵이름
	char				nMapIndex;					// 맵인덱스
	MMATCH_GAMETYPE		nGameType;					// 게임타입
	int					nRoundMax;					// 라운드
	int					nLimitTime;					// 제한시간(1 - 1분)
	int					nLimitLevel;				// 제한레벨
	int					nMaxPlayers;				// 최대인원
	bool				bTeamKillEnabled;			// 팀킬여부
	bool				bTeamWinThePoint;			// 선승제 여부
	bool				bForcedEntryEnabled;		// 게임중 참가 여부
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

class ZGame;

class ZReplayLoader
{
private:
	unsigned int					m_nVersion;
	REPLAY_STAGE_SETTING_NODE		m_StageSetting;
	float							m_fGameTime;

	bool LoadHeader(ZFile* file);
	bool LoadStageSetting(ZFile* file);
	bool LoadStageSettingEtc(ZFile* file);	// 스테이지타입에 따른 추가 스테이지 데이터, ( duel모드의 순서등.. )
	bool LoadCharInfo(ZFile* file);
	bool LoadCommandStream(ZFile* file);

	void ConvertStageSettingNode(REPLAY_STAGE_SETTING_NODE* pSource, MSTAGE_SETTING_NODE* pTarget);
	void ChangeGameState();

	MCommand* CreateCommandFromStream(char* pStream);
	MCommand* CreateCommandFromStreamVersion2(char* pStream);
	bool ParseVersion2Command(char* pStream, MCommand* pCmd);
	MCommandParameter* MakeVersion2CommandParameter(MCommandParameterType nType, char* pStream, unsigned short int* pnDataCount);
public:
	ZReplayLoader();
	~ZReplayLoader() {}
	bool Load(const char* filename);	
	float GetGameTime() const { return m_fGameTime; }
};






#endif