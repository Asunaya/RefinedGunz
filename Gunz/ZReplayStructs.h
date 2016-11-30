#pragma once

#include "ZCharacterStructs.h"

#pragma pack(push)
#pragma pack(1)

struct REPLAY_HEADER
{
	u32 Header;
	u32 ReplayBinaryVersion;
};

struct REPLAY_HEADER_RG
{
	u32 Header;
	u32 ReplayBinaryVersion;
	u32 ClientVersion;
	i64 Timestamp;
};

struct REPLAY_STAGE_SETTING_NODE
{
	MUID				uidStage;
	char				szStageName[64];
	char				szMapName[MAPNAME_LENGTH];
	i32					nMapIndex;
	MMATCH_GAMETYPE		nGameType;
	i32					nRoundMax;
	i32					nLimitTime;
	i32					nLimitLevel;
	i32					nMaxPlayers;
	bool				bTeamKillEnabled;
	bool				bTeamWinThePoint;
	bool				bForcedEntryEnabled;
	bool				bAutoTeamBalancing;
	NetcodeType			Netcode;
	bool				ForceHPAP;
	int					HP;
	int					AP;
	bool				NoFlip;
	bool				SwordsOnly;
};

struct REPLAY_STAGE_SETTING_NODE_RG_V1
{
	MUID				uidStage;
	char				szStageName[64];
	char				szMapName[MAPNAME_LENGTH];
	i32					nMapIndex;
	MMATCH_GAMETYPE		nGameType;
	i32					nRoundMax;
	i32					nLimitTime;
	i32					nLimitLevel;
	i32					nMaxPlayers;
	bool				bTeamKillEnabled;
	bool				bTeamWinThePoint;
	bool				bForcedEntryEnabled;
	bool				bAutoTeamBalancing;
};

struct REPLAY_STAGE_SETTING_NODE_OLD
{
	MUID				uidStage;
	char				szMapName[32];
	i32					nMapIndex;
	MMATCH_GAMETYPE		nGameType;
	int					nRoundMax;
	int					nLimitTime;
	int					nLimitLevel;
	int					nMaxPlayers;
	bool				bTeamKillEnabled;
	bool				bTeamWinThePoint;
	bool				bForcedEntryEnabled;
	char				pad;
};
struct REPLAY_STAGE_SETTING_NODE_V11
{
	MUID				uidStage;
	char				szMapName[32];
	char				unk[32];
	i32					nMapIndex;
	MMATCH_GAMETYPE		nGameType;
	int					nRoundMax;
	int					nLimitTime;
	int					nLimitLevel;
	int					nMaxPlayers;
	bool				bTeamKillEnabled;
	bool				bTeamWinThePoint;
	bool				bForcedEntryEnabled;
	char				pad;
};

struct REPLAY_STAGE_SETTING_NODE_FG
{
	MUID				uidStage;
	char				szMapName[32];
	i32					nMapIndex;
	MMATCH_GAMETYPE		nGameType;
	int					nRoundMax;
	int					nLimitTime;
	int					nLimitLevel;
	int					nMaxPlayers;
	bool				bTeamKillEnabled;
	bool				bTeamWinThePoint;
	bool				bForcedEntryEnabled;
	char				szStageName[64];
	char				unk;
};

struct REPLAY_STAGE_SETTING_NODE_DG
{
	MUID				uidStage;
	char				szMapName[32];
	i32					nMapIndex;
	MMATCH_GAMETYPE		nGameType;
	int					nRoundMax;
	int					nLimitTime;
	int					nLimitLevel;
	int					nMaxPlayers;
	bool				bTeamKillEnabled;
	bool				bTeamWinThePoint;
	bool				bForcedEntryEnabled;
	char				unk[5];
};

struct MTD_CharInfo_V5
{
	// 캐릭터 정보
	char				szName[32];
	char				szClanName[16];
	MMatchClanGrade		nClanGrade;
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
	u32					nEquipedItemDesc[12];

	// account 의 정보
	MMatchUserGradeID	nUGradeID;

	// ClanCLID
	unsigned int		nClanCLID;
};

struct MTD_CharInfo_V6
{
	char				szName[32];
	char				szClanName[16];
	MMatchClanGrade		nClanGrade;
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
	u32	nEquipedItemDesc[17];

	// account 의 정보
	MMatchUserGradeID	nUGradeID;

	// ClanCLID
	unsigned int		nClanCLID;

	// 지난주 듀얼토너먼트 등급
	int					nDTLastWeekGrade;

	MUID				uidEquipedItem[17];
	u32					nEquipedItemCount[17];
};

struct MTD_CharInfo_V11
{
	char				szName[32];
	char				szClanName[16];
	MMatchClanGrade		nClanGrade;
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
	u32					nEquipedItemDesc[17];

	// account 의 정보
	MMatchUserGradeID	nUGradeID;

	// ClanCLID
	unsigned int		nClanCLID;

	// 지난주 듀얼토너먼트 등급
	int					nDTLastWeekGrade;

	MUID				uidEquipedItem[17];
	u32					nEquipedItemCount[17];

	char unk[8];
};

using MTD_CharInfo_FG_V7_0 = MTD_CharInfo_V6;

struct MTD_CharInfo_FG_V7_1
{
	char				szName[32];
	char				szClanName[16];
	MMatchClanGrade		nClanGrade;
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

	u32					nEquipedItemDesc[22];

	MMatchUserGradeID	nUGradeID;

	unsigned int		nClanCLID;

	int					nDTLastWeekGrade;

	MUID				uidEquipedItem[22];
	u32					nEquipedItemCount[22];
	u32					nEquipedItemRarity[22];
	u32					nEquipedItemLevel[22];
};

struct MTD_CharInfo_FG_V8
{
	char				szName[32];
	char				szClanName[16];
	MMatchClanGrade		nClanGrade;
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
	MMatchUserGradeID	nUGradeID;

	// ClanCLID
	unsigned int		nClanCLID;

	// 지난주 듀얼토너먼트 등급
	int					nDTLastWeekGrade;

	uint32_t unk[6];

	// 아이템 정보 추가
	MUID				uidEquipedItem[22];
	u32					nEquipedItemCount[22];
	u32					nEquipedItemRarity[22];
	u32					nEquipedItemLevel[22];
};

struct MTD_CharInfo_FG_V9
{
	char				szName[32];
	char				szClanName[16];
	MMatchClanGrade		nClanGrade;
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
	MMatchUserGradeID	nUGradeID;

	// ClanCLID
	unsigned int		nClanCLID;

	// 지난주 듀얼토너먼트 등급
	int					nDTLastWeekGrade;

	u32					 unk[6];

	// 아이템 정보 추가
	MUID				uidEquipedItem[22];
	u32					nEquipedItemCount[22];
	u32					nEquipedItemRarity[22];
	u32					nEquipedItemLevel[22];

	char				unk2[24];
};

struct BulletInfo
{
	int Clip;
	int Magazine;
};

template<size_t NumItems>
struct ZCharacterReplayStateImpl
{
	MUID UID;
	ZCharacterProperty Property;
	float HP;
	float AP;
	ZCharacterStatus Status;

	BulletInfo BulletInfos[NumItems];

	rvector Position;
	rvector Direction;

	MMatchTeam Team;

	bool Dead;

	bool HidingAdmin;
};

using ZCharacterReplayState = ZCharacterReplayStateImpl<MMCIP_END>;
using ZCharacterReplayState_FG_V7_0 = ZCharacterReplayStateImpl<17>;
using ZCharacterReplayState_FG_V7_1 = ZCharacterReplayStateImpl<22>;
using ZCharacterReplayState_FG_V8 = ZCharacterReplayStateImpl<23>;
using ZCharacterReplayState_FG_V9 = ZCharacterReplayStateImpl<24>;
using ZCharacterReplayState_Official_V6 = ZCharacterReplayStateImpl<17>;
using ZCharacterReplayState_Official_V11 = ZCharacterReplayStateImpl<34>;

template <typename CharInfo, typename ReplayState>
struct ReplayPlayerInfoImpl
{
	bool IsHero;
	CharInfo Info;
	ReplayState State;
};

struct MTD_CharInfo_DG
{
	char				szName[32];
	char				szClanName[16];
	MMatchClanGrade		nClanGrade;
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
	MMatchUserGradeID	nUGradeID;

	// ClanCLID
	unsigned int		nClanCLID;

	// 지난주 듀얼토너먼트 등급
	int					nDTLastWeekGrade;

	MUID				uidEquipedItem[17];
	u32					nEquipedItemCount[17];

	char				unk[6];
};

struct ZCharacterReplayState_DG
{
	MUID UID;
	ZCharacterProperty Property;
	float HP;
	float AP;
	ZCharacterStatus Status;

	BulletInfo BulletInfos[17];

	char unk[8];

	rvector Position;
	rvector Direction;

	MMatchTeam Team;

	bool Dead;

	bool HidingAdmin;
};

struct ReplayPlayerInfo_DG
{
	bool IsHero;
	char unk[32];
	MTD_CharInfo_DG Info;
	ZCharacterReplayState_DG State;
};

using ReplayPlayerInfo = ReplayPlayerInfoImpl<MTD_CharInfo, ZCharacterReplayState>;

using ReplayPlayerInfo_FG_V7_0 = ReplayPlayerInfoImpl<MTD_CharInfo_FG_V7_0, ZCharacterReplayState_FG_V7_0>;
using ReplayPlayerInfo_FG_V7_1 = ReplayPlayerInfoImpl<MTD_CharInfo_FG_V7_1, ZCharacterReplayState_FG_V7_1>;
using ReplayPlayerInfo_FG_V8 = ReplayPlayerInfoImpl<MTD_CharInfo_FG_V8, ZCharacterReplayState_FG_V8>;
using ReplayPlayerInfo_FG_V9 = ReplayPlayerInfoImpl<MTD_CharInfo_FG_V9, ZCharacterReplayState_FG_V9>;
using ReplayPlayerInfo_Official_V6 = ReplayPlayerInfoImpl<MTD_CharInfo_V6, ZCharacterReplayState_Official_V6>;
using ReplayPlayerInfo_Official_V11 = ReplayPlayerInfoImpl<MTD_CharInfo_V11, ZCharacterReplayState_Official_V11>;

#pragma pack(pop)