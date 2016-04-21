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

#define CLAN_SPONSORS_COUNT			4		// 클랜생성시 필요한 발기인 수
#define CLAN_CREATING_NEED_BOUNTY		1000	// 클랜생성에 필요한 바운티
#define CLAN_CREATING_NEED_LEVEL		10		// 클랜생성에 필요한 레벨


#define CLAN_NAME_LENGTH			16		// 클랜 이름 최대 길이 - 이게 변경되면 프로토콜 버전도 변경되어야 함
#define MIN_CLANNAME	4				// 최소 4자이상 12자이하만 클랜 이름을 만들 수 있다.
#define MAX_CLANNAME	12


// 캐릭터 관련
#define MIN_CHARNAME	4				// 최소 4자이상 12자 이하만 캐릭터 이름을 만들 수 있다.
#define MAX_CHARNAME	16

#define MAX_CHAR_LEVEL	99

#define MATCH_SIMPLE_DESC_LENGTH	64


// 액션리그 관련
#define ACTIONLEAGUE_TEAM_MEMBER_COUNT		4		// 액션리그는 4명이 모두 함께 게임해야된다.
#define MAX_LADDER_TEAM_MEMBER				4		// 래더팀은 1~4명까지 만들 수 있다.
#define MAX_CLANBATTLE_TEAM_MEMBER			8		// 클랜전은 최대 8명까지 만들 수 있다.

//#define LIMIT_ACTIONLEAGUE		// 넷마블 액션리그 전용 디파인
//#define LEAK_TEST
#define CLAN_BATTLE					// 클랜전 개발용 디파인 - 개발이 끝나면 사라질 예정





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