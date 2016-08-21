#pragma once
#ifndef _MMATCHGLOBAL_H
#define _MMATCHGLOBAL_H

#include "MBaseLocale.h"

#define MATCHOBJECT_NAME_LENGTH		32		// 캐릭터 이름 길이
#define MAX_CHAR_COUNT				4		// 만들 수 있는 캐릭터 수


#define CYCLE_STAGE_UPDATECHECKSUM	500		// 방리스트 정보 변경사항 보여주는 딜레이 - 0.5초

// 게임 Rule 관련
#define NUM_APPLYED_TEAMBONUS_TEAM_PLAYERS		3		// 팀전에서 팀인원이 3명 이상일때만 경험치 보너스를 적용한다
#define RESPAWN_DELAYTIME_AFTER_DYING			7000	///< 죽고나서 리스폰되는 딜레이시간

#define MAX_XP_BONUS_RATIO						2.0f	///< 경험치 보너스는 최대 2배이다.
#define MAX_BP_BONUS_RATIO						2.0f	///< 바운티 보너스는 최대 2배이다.

// 스테이지 관련
#define STAGENAME_LENGTH			64			// 방이름 길이
#define STAGEPASSWD_LENGTH			8			// 패스워드 최대길이
#define STAGE_QUEST_MAX_PLAYER		4			// 퀘스트 게임모드의 최대인원


#define TRANS_STAGELIST_NODE_COUNT				8	// 한번에 클라이언트에게 보내주는 스테이지노드 개수
#define TRANS_STANDBY_CLANLIST_NODE_COUNT		4	// 클랜전에서 한번에 클라이언트에게 보내주는 대기중 클랜 개수


#define MAX_REPLIER	16			// proposal 답변자는 최대 16명

//
// Clan
//
#define CLAN_SPONSORS_COUNT			4		// 클랜생성시 필요한 발기인 수
#define CLAN_CREATING_NEED_BOUNTY		1000	// 클랜생성에 필요한 바운티
#define CLAN_CREATING_NEED_LEVEL		10		// 클랜생성에 필요한 레벨


#define CLAN_NAME_LENGTH			16		// 클랜 이름 최대 길이 - 이게 변경되면 프로토콜 버전도 변경되어야 함
#define MIN_CLANNAME	4				// 최소 4자이상 12자이하만 클랜 이름을 만들 수 있다.
#define MAX_CLANNAME	12

enum MMatchClanGrade : i32
{
	MCG_NONE = 0,
	MCG_MASTER = 1,
	MCG_ADMIN = 2,

	MCG_MEMBER = 9,
	MCG_END
};

inline bool IsUpperClanGrade(MMatchClanGrade nSrcGrade, MMatchClanGrade nDstGrade)
{
	if ((nSrcGrade != MCG_NONE) && ((int)nSrcGrade <= (int)nDstGrade)) return true;
	return false;
}

//
// Character
//
#define MIN_CHARNAME	4				// 최소 4자이상 12자 이하만 캐릭터 이름을 만들 수 있다.
#define MAX_CHARNAME	16

#define MAX_CHAR_LEVEL	99

#define MATCH_SIMPLE_DESC_LENGTH	64

enum MMatchUserGradeID : i32
{
	MMUG_FREE = 0,
	MMUG_REGULAR = 1,
	MMUG_STAR = 2,

	MMUG_CRIMINAL = 100,
	MMUG_WARNING_1 = 101,
	MMUG_WARNING_2 = 102,
	MMUG_WARNING_3 = 103,
	MMUG_CHAT_LIMITED = 104,
	MMUG_PENALTY = 105,

	MMUG_VIP = 251,
	MMUG_EVENTMASTER = 252,
	MMUG_BLOCKED = 253,
	MMUG_DEVELOPER = 254,
	MMUG_ADMIN = 255
};

enum MMatchPlace
{
	MMP_OUTSIDE = 0,
	MMP_LOBBY = 1,
	MMP_STAGE = 2,
	MMP_BATTLE = 3,
	MMP_END
};

enum MMatchObjectStageState
{
	MOSS_NONREADY = 0,
	MOSS_READY = 1,
	MOSS_SHOP = 2,
	MOSS_EQUIPMENT = 3,
	MOSS_END
};

enum MMatchDisconnectStatus
{
	MMDS_CONNECTED = 1,
	MMDS_DISCONN_WAIT,
	MMDS_DISCONNECT,

	MMDS_END,
};

enum MMatchPremiumGradeID
{
	MMPG_FREE = 0,
	MMPG_PREMIUM_IP = 1
};

enum MMatchSex
{
	MMS_MALE = 0,
	MMS_FEMALE = 1
};

enum MMatchBlockType
{
	MMBT_NO = 0,
	MMBT_BANNED,
	MMBT_MUTED,

	MMBT_END,
};

enum MCmdEnterBattleParam
{
	MCEP_NORMAL = 0,
	MCEP_FORCED = 1,
	MCEP_END
};

#define DEFAULT_CHAR_HP				100
#define DEFAULT_CHAR_AP				0

inline bool IsAdminGrade(MMatchUserGradeID nGrade)
{
	if ((nGrade == MMUG_EVENTMASTER) ||
		(nGrade == MMUG_ADMIN) ||
		(nGrade == MMUG_DEVELOPER))
		return true;

	return false;
}

// 캐릭터 생성할때 주는 기본 아이템
struct MINITIALCOSTUME
{
	// 무기
	unsigned int nMeleeItemID;
	unsigned int nPrimaryItemID;
	unsigned int nSecondaryItemID;
	unsigned int nCustom1ItemID;
	unsigned int nCustom2ItemID;

	// 장비 아이템
	unsigned int nChestItemID;
	unsigned int nHandsItemID;
	unsigned int nLegsItemID;
	unsigned int nFeetItemID;
};

#define MAX_COSTUME_TEMPLATE		6
const MINITIALCOSTUME g_InitialCostume[MAX_COSTUME_TEMPLATE][2] =
{
	{ { 1, 5001, 4001, 30301, 0,     21001, 0, 23001, 0 },{ 1, 5001, 4001, 30301, 0,     21501, 0, 23501, 0 } },	// 건나이트
	{ { 2, 5002, 0,    30301, 0,     21001, 0, 23001, 0 },{ 2, 5002, 0,    30301, 0,     21501, 0, 23501, 0 } },	// 건파이터
	{ { 1, 4005, 5001, 30401, 0,     21001, 0, 23001, 0 },{ 1, 4005, 5001, 30401, 0,     21501, 0, 23501, 0 } },	// 애서신
	{ { 2, 4001, 0,    30401, 0,     21001, 0, 23001, 0 },{ 2, 4001, 0,    30401, 0,     21501, 0, 23501, 0 } },	// 스카우트
	{ { 2, 4002, 0,    30401, 30001, 21001, 0, 23001, 0 },{ 2, 4002, 0,    30401, 30001, 21501, 0, 23501, 0 } },	// 건프리스트
	{ { 1, 4006, 0,	 30101, 30001, 21001, 0, 23001, 0 },{ 1, 4006, 4006, 30101, 30001, 21501, 0, 23501, 0 } }	// 닥터
};

#define MAX_COSTUME_HAIR		5
const std::string g_szHairMeshName[MAX_COSTUME_HAIR][2] =
{
	{ "eq_head_01", "eq_head_pony" },
	{ "eq_head_02", "eq_head_hair001" },
	{ "eq_head_08", "eq_head_hair04" },
	{ "eq_head_05", "eq_head_hair006" },
	{ "eq_head_08", "eq_head_hair002" }		// 이건 현재 사용안함 - 나중에 다른 모델로 대체해도 됨
};

#define MAX_COSTUME_FACE		20
const std::string g_szFaceMeshName[MAX_COSTUME_FACE][2] =
{
	{ "eq_face_01", "eq_face_001" },
	{ "eq_face_02", "eq_face_002" },
	{ "eq_face_04", "eq_face_003" },
	{ "eq_face_05", "eq_face_004" },
	{ "eq_face_a01", "eq_face_001" },
	{ "eq_face_newface01", "eq_face_newface01" },
	{ "eq_face_newface02", "eq_face_newface02" },
	{ "eq_face_newface03", "eq_face_newface03" },
	{ "eq_face_newface04", "eq_face_newface04" },
	{ "eq_face_newface05", "eq_face_newface05" },
	{ "eq_face_newface06", "eq_face_newface06" },
	{ "eq_face_newface07", "eq_face_newface07" },
	{ "eq_face_newface08", "eq_face_newface08" },
	{ "eq_face_newface09", "eq_face_newface09" },
	{ "eq_face_newface10", "eq_face_newface10" },
	{ "eq_face_newface11", "eq_face_newface11" },
	{ "eq_face_newface12", "eq_face_newface12" },
	{ "eq_face_newface13", "eq_face_newface13" },
	{ "eq_face_newface13", "eq_face_newface14" },
	{ "eq_face_newface13", "eq_face_newface15" },
};

//
// Clan war
//
#define ACTIONLEAGUE_TEAM_MEMBER_COUNT		4		// 액션리그는 4명이 모두 함께 게임해야된다.
#define MAX_LADDER_TEAM_MEMBER				4		// 래더팀은 1~4명까지 만들 수 있다.
#define MAX_CLANBATTLE_TEAM_MEMBER			8		// 클랜전은 최대 8명까지 만들 수 있다.

#define CLAN_BATTLE					// 클랜전 개발용 디파인 - 개발이 끝나면 사라질 예정

//
// Channel
//
#define CHANNELNAME_LEN		64
#define CHANNELRULE_LEN		64
#define DEFAULT_CHANNEL_MAXPLAYERS			200
#define DEFAULT_CHANNEL_MAXSTAGES			100
#define MAX_CHANNEL_MAXSTAGES				500
#define NUM_PLAYERLIST_NODE					6
#define CHANNEL_NO_LEVEL					(-1)

enum MCHANNEL_TYPE {
	MCHANNEL_TYPE_PRESET = 0,
	MCHANNEL_TYPE_USER = 1,
	MCHANNEL_TYPE_PRIVATE = 2,
	MCHANNEL_TYPE_CLAN = 3,
	MCHANNEL_TYPE_MAX
};

struct MCHANNELLISTNODE {
	MUID			uidChannel;						// 채널 UID
	short			nNo;							// 채널번호
	unsigned char	nPlayers;						// 현재인원
	short			nMaxPlayers;					// 최대인원
	short			nLevelMin;						// 최소레벨
	short			nLevelMax;						// 최대레벨
	char			nChannelType;					// 채널타입
	char			szChannelName[CHANNELNAME_LEN];	// 채널이름
};

//
// Round
//
enum MMATCH_ROUNDSTATE {
	MMATCH_ROUNDSTATE_PREPARE = 0,
	MMATCH_ROUNDSTATE_COUNTDOWN = 1,
	MMATCH_ROUNDSTATE_PLAY = 2,
	MMATCH_ROUNDSTATE_FINISH = 3,
	MMATCH_ROUNDSTATE_EXIT = 4,
	MMATCH_ROUNDSTATE_FREE = 5,
	MMATCH_ROUNDSTATE_FAILED = 6,
	MMATCH_ROUNDSTATE_END
};

enum MMATCH_ROUNDRESULT {
	MMATCH_ROUNDRESULT_DRAW = 0,
	MMATCH_ROUNDRESULT_REDWON,
	MMATCH_ROUNDRESULT_BLUEWON,
	MMATCH_ROUNDRESULT_END
};



enum MMatchTeam
{
	MMT_ALL			= 0,
	MMT_SPECTATOR	= 1,
	MMT_RED			= 2,
	MMT_BLUE		= 3,
	MMT_END
};


// 서버모드
enum MMatchServerMode
{
	MSM_NORMAL_		= 0,		// 일반
	MSM_CLAN		= 1,		// 클랜전 전용 서버
	MSM_LADDER		= 2,		// 래더 전용 서버
	MSM_EVENT		= 3,		// 이벤트 서버
	MSM_TEST		= 4,		// 테스트 서버
	MSM_MAX,

	MSM_ALL			= 100,		// event에만 사용된다.
};

// 동의 관련
enum MMatchProposalMode
{
	MPROPOSAL_NONE = 0,				// 사용하지 않음
	MPROPOSAL_LADDER_INVITE,		// 래더게임 요청
	MPROPOSAL_CLAN_INVITE,			// 클랜전 요청
	MPROPOSAL_END
};


// 래더 타입
enum MLADDERTYPE {
	MLADDERTYPE_NORMAL_2VS2		= 0,
	MLADDERTYPE_NORMAL_3VS3,
	MLADDERTYPE_NORMAL_4VS4,
//	MLADDERTYPE_NORMAL_8VS8,
	MLADDERTYPE_MAX
};

// 각 래더타입별 필요한 인원수
const int g_nNeedLadderMemberCount[MLADDERTYPE_MAX] = {	2, 3, 4/*, 8*/};


/// Clan관련.
#define DEFAULT_CLAN_POINT			1000			// 기본 클랜 포인트
#define DAY_OF_DELETE_CLAN			(7)				// 클랜 폐쇄요청후 DAY_OF_DELETE_CLAN일만큼 지난후 패쇄작업이 진행됨.
#define MAX_WAIT_CLAN_DELETE_HOUR	(24)			// DAY_OF_DELETE_CLAN + MAX_WAIT_CLAN_DELETE_HOUR후 디비에서 클랜삭제.
#define UNDEFINE_DELETE_HOUR		(2000000000)	// 정상적인 클랜의 DeleteTime의 null값처리용.

enum MMatchClanDeleteState
{
	MMCDS_NORMAL = 1,
	MMCDS_WAIT,
	MMCDS_DELETE,

	MMCDS_END,
};


// 옵션 관련
enum MBITFLAG_USEROPTION {
	MBITFLAG_USEROPTION_REJECT_WHISPER	= 1,
	MBITFLAG_USEROPTION_REJECT_INVITE	= 1<<1
};

// 퀘스트 관련 ///////////////////////////////////////////////////////////////////////////////

#define MAX_QUEST_MAP_SECTOR_COUNT					16			// 퀘스트에서 최대 만들어질 수 있는 맵 개수
#define MAX_QUEST_NPC_INFO_COUNT					8			// 퀘스트에서 최대 나올 NPC 종류 개수


#define ALL_PLAYER_NOT_READY					1	// 모든 유저가 레디를 하지 못해서 게임을 시작하지 못함.
#define QUEST_START_FAILED_BY_SACRIFICE_SLOT	2	// 희생 아이템 슬롯 검사시 문제가 있어서 시작을 실패함.

#define MIN_QUESTITEM_ID							200001	// item id가 200001부터는 퀘스트 아이템이다
#define MAX_QUESTITEM_ID							299999

// Keeper Manager와의 Schedule관련. ////////////////////////////////////////////////////////////

enum KMS_SCHEDULE_TYPE
{
	KMST_NO = 0,
	KMST_REPEAT,
	KMST_COUNT,
	KMST_ONCE,

	KMS_SCHEDULE_TYPE_END,
};

enum KMS_COMMAND_TYPE
{
	KMSC_NO = 0,
	KMSC_ANNOUNCE,
	KMSC_STOP_SERVER,
	KMSC_RESTART_SERVER,
	
	KMS_COMMAND_TYPE_END,
};

enum SERVER_STATE_KIND
{
	SSK_OPENDB = 0,

	SSK_END,
};

enum SERVER_ERR_STATE
{
	SES_NO = 0,
	SES_ERR_DB,
    
	SES_END,
};

enum SERVER_TYPE
{
	ST_NULL = 0,
	ST_DEBUG,
	ST_NORMAL,
	ST_CLAN,
	ST_QUEST,
	ST_EVENT,
};


enum MMatchBlockLevel
{
	MMBL_NO = 0,
	MMBL_ACCOUNT,
	MMBL_LOGONLY,

	MMBL_END,
};

/////////////////////////////////////////////////////////////////////////////////////////////
// Util 함수

/// 상대편 반환
inline MMatchTeam NegativeTeam(MMatchTeam nTeam)
{
	if (nTeam == MMT_RED) return MMT_BLUE;
	else if (nTeam == MMT_BLUE) return MMT_RED;
	return nTeam;
}

#endif