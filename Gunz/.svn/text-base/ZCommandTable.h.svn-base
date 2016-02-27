#ifndef _ZCOMMANDTABLE_H
#define _ZCOMMANDTABLE_H

/// 클라이언트 커뮤니케이터가 사용하는 커맨드
void ZAddCommandTable(MCommandManager* pCommandManager);


// 콘솔 or 콘솔의 alias 관련 명령어 ///////////////////////////////////////////
#define ZC_CON_CLEAR		41000	///< 클라이언트 콘솔 화면을 지운다.
#define ZC_CON_SIZE			41001	///< 클라이언트 콘솔 창 크기를 변경한다.
#define ZC_CON_HIDE			41002
#define ZC_CON_CONNECT		41003
#define ZC_CON_DISCONNECT	41004
#define ZC_CON_SAY			41005

#define ZC_TEST_INFO		42000	// 정보값알아내기 위한 테스트 명령어
#define ZC_BEGIN_PROFILE	42001	// 프로파일링을 시작한다.
#define ZC_END_PROFILE		42002	// 프로파일링을 시작한다.

// 관리자용 ///////////////////////////////////////////////////////////////////


// test code //////////////////////////////////////////////////////////////////
#ifdef _DEBUG
	#define ZC_TEST_SETCLIENT1		61000
	#define ZC_TEST_SETCLIENT2		61001
	#define ZC_TEST_SETCLIENT3		61002
	#define ZC_TEST_SETCLIENTALL	61003
#endif

#define ZC_TEST_BIRD1			61004	// 버드의 테스트





// 게임에서 사용하는 명령어 ///////////////////////////////////////////////////
#define ZC_CHANGESKIN						51000	///< 인터페이스 스킨을 변경한다.
#define ZC_REPORT_119						51001	///< 119
#define ZC_MESSAGE							51002	///< 메시지

#define ZC_EVENT_OPTAIN_SPECIAL_WORLDITEM	52001	///< 특별한 월드아이템을 먹었을 경우 처리

// 퀘스트 관련

#define MC_QUEST_NPC_LOCAL_SPAWN			53000	///< 특별한 월드아이템을 먹었을 경우 처리


#endif