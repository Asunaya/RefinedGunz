#ifndef _ZREPLAY_H
#define _ZREPLAY_H

struct REPLAY_STAGE_SETTING_NODE_OLD 
{
	MUID				uidStage;
	char				szMapName[32];	// 맵이름
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

struct REPLAY_STAGE_SETTING_NODE_FG
{
	MUID				uidStage;
	char				szMapName[32];
	char				nMapIndex;
	MMATCH_GAMETYPE		nGameType;
	int					nRoundMax;
	int					nLimitTime;
	int					nLimitLevel;
	int					nMaxPlayers;
	bool				bTeamKillEnabled;
	bool				bTeamWinThePoint;
	bool				bForcedEntryEnabled;
	char				szStageName[64];
};

#pragma pack(push)
#pragma pack(1)

struct MTD_CharInfo_FG_V7_0
{
	char				szName[32];
	char				szClanName[16];
	DWORD		nClanGrade;
	unsigned short		nClanContPoint;
	char				nCharNum;
	unsigned short		nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	unsigned long int	nXP;
	int					nBP;
	float				fBonusRate;
	unsigned short		nPrize;
	unsigned short		nHP;
	unsigned short		nAP;
	unsigned short		nMaxWeight;
	unsigned short		nSafeFalls;
	unsigned short		nFR;
	unsigned short		nCR;
	unsigned short		nER;
	unsigned short		nWR;

	// 아이템 정보
	unsigned long int	nEquipedItemDesc[17];

	// account 의 정보
	DWORD	nUGradeID;

	// ClanCLID
	unsigned int		nClanCLID;

	// 지난주 듀얼토너먼트 등급
	int					nDTLastWeekGrade;

	char unk[204];
};

struct MTD_CharInfo_FG_V7_1
{
	char				szName[32];
	char				szClanName[16];
	DWORD		nClanGrade;
	unsigned short		nClanContPoint;
	char				nCharNum;
	unsigned short		nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	unsigned long int	nXP;
	int					nBP;
	float				fBonusRate;
	unsigned short		nPrize;
	unsigned short		nHP;
	unsigned short		nAP;
	unsigned short		nMaxWeight;
	unsigned short		nSafeFalls;
	unsigned short		nFR;
	unsigned short		nCR;
	unsigned short		nER;
	unsigned short		nWR;

	unsigned long int	nEquipedItemDesc[22];

	DWORD	nUGradeID;

	unsigned int		nClanCLID;

	int					nDTLastWeekGrade;

	__int64				uidEquipedItem[22];
	unsigned long int	nEquipedItemCount[22];
	unsigned long int	nEquipedItemRarity[22];
	unsigned long int	nEquipedItemLevel[22];
};

struct MTD_CharInfo_FG_V9
{
	char				szName[32];
	char				szClanName[16];
	DWORD		nClanGrade;
	unsigned short		nClanContPoint;
	char				nCharNum;
	unsigned short		nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	unsigned long int	nXP;
	int					nBP;
	float				fBonusRate;
	unsigned short		nPrize;
	unsigned short		nHP;
	unsigned short		nAP;
	unsigned short		nMaxWeight;
	unsigned short		nSafeFalls;
	unsigned short		nFR;
	unsigned short		nCR;
	unsigned short		nER;
	unsigned short		nWR;

	// 아이템 정보
	unsigned long int	nEquipedItemDesc[22];

	// account 의 정보
	DWORD	nUGradeID;

	// ClanCLID
	unsigned int		nClanCLID;

	// 지난주 듀얼토너먼트 등급
	int					nDTLastWeekGrade;

	DWORD unk[6];

	// 아이템 정보 추가
	__int64				uidEquipedItem[22];
	unsigned long int	nEquipedItemCount[22];
	unsigned long int	nEquipedItemRarity[22];
	unsigned long int	nEquipedItemLevel[22];

	char unk2[24];
};

#pragma pack(pop)

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
	BYTE *FileBuffer;

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