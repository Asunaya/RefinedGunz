#ifndef MSHAREDCOMMANDTABLE_H
#define MSHAREDCOMMANDTABLE_H

class MCommandManager;


#define MCOMMAND_VERSION	55



#define MSCT_MASTER			1
#define MSCT_CLIENT			2
#define MSCT_MATCHSERVER	4
#define MSCT_AGENT			8
#define MSCT_ALL			(MSCT_MASTER+MSCT_CLIENT+MSCT_AGENT)




/// 모든 커뮤니케이터가 공유하는 커맨드
void MAddSharedCommandTable(MCommandManager* pCommandManager, int nSharedType);


/// 서버 클라이언트 공유 상수
#define MATCH_CYCLE_CHECK_SPEEDHACK	20000


// 기본 커맨드 리스트
#define MC_HELP						0
#define MC_VERSION					1

#define MC_DEBUG_TEST				100

#define MC_LOCAL_INFO				201		///< Local 정보를 얻는다.
#define MC_LOCAL_ECHO				202		///< Local Echo 테스트
#define MC_LOCAL_LOGIN				203		///< Login 처리 Local Command



#define MC_NET_ENUM					301		///< 연결할 수 있는 커뮤니케이터 Enum
#define MC_NET_CONNECT				302		///< 커뮤니케이터와 연결
#define MC_NET_DISCONNECT			303		///< 커뮤니케이터와 연결 해제
#define MC_NET_CLEAR				304		///< 커뮤니케이터와 연결 해제 후 관련자원 처리
#define MC_NET_ONCONNECT			311		///< 커뮤니케이터와 연결되었을 때
#define MC_NET_ONDISCONNECT			312		///< 커뮤니케이터와 연결이 끊겼을 때
#define MC_NET_ONERROR				313		///< 커뮤니케이터와 연결 에러
#define MC_NET_CHECKPING			321		///< 커뮤니케이터 반응속도검사
#define MC_NET_PING					322		///< 커뮤니케이터 반응속도검사 요청
#define MC_NET_PONG					323		///< 커뮤니케이터 반응속도검사 응답

#define MC_HSHIELD_PING				324		///< 핵실드 메세지 요청(매치서버 -> 클라이언트)
#define MC_HSHIELD_PONG				325		///< 핵실드 메세지 응답(클라이언트 -> 매치서버)

#define MC_NET_CONNECTTOZONESERVER	331		///< 127.0.0.1:6000 연결

#define MC_NET_REQUEST_INFO			341		///< Net 정보를 요청한다.
#define MC_NET_RESPONSE_INFO		342		///< Net 정보를 얻는다.
#define MC_NET_REQUEST_UID			343		///< MUID를 요청
#define MC_NET_RESPONSE_UID			344		///< MUID를 확보해서 돌려준다.
#define MC_NET_ECHO					351		///< Net Echo 테스트

#define MC_CLOCK_SYNCHRONIZE		361		///< 클럭 싱크

// 매치서버 관련 명령어
#define MC_MATCH_NOTIFY						401		///< 알림 메시지
#define MC_MATCH_ANNOUNCE					402		///< 공지
#define MC_MATCH_RESPONSE_RESULT			403		///< request에 대한 결과를 알림(S -> C)

// 관리자 전용 명령어
#define MC_ADMIN_ANNOUNCE					501		///< 전체 접속자에게 방송
#define MC_ADMIN_REQUEST_SERVER_INFO		505		///< 서버 정보 요청
#define MC_ADMIN_RESPONSE_SERVER_INFO		506		///< 서버 정보 응답
#define MC_ADMIN_SERVER_HALT				511		///< 서버를 종료 한다
#define MC_ADMIN_TERMINAL					512		///< 터미날
#define MC_ADMIN_REQUEST_UPDATE_ACCOUNT_UGRADE	513	///< 계정 권한 변경 요청
#define MC_ADMIN_RESPONSE_UPDATE_ACCOUNT_UGRADE	514	///< 계정 권한 변경 응답
#define MC_ADMIN_REQUEST_BAN_PLAYER			515		///< 타겟 접속 끊기 요청
#define MC_ADMIN_RESPONSE_BAN_PLAYER		516		///< 타겟 접속 끊기 응답
#define MC_ADMIN_PING_TO_ALL				521		///< Garbage Connection 청소 확인을 위한 수동명령
#define MC_ADMIN_REQUEST_SWITCH_LADDER_GAME	522		///< 클랜전 신청 가능여부 설정
#define MC_ADMIN_HIDE						531		///< 운영자 안보이기
#define MC_ADMIN_RELOAD_CLIENT_HASH			532		///< reload XTrap Hashmap
#define MC_ADMIN_RESET_ALL_HACKING_BLOCK	533		///< 모든 해킹 블럭을 취소한다.

// 이벤트 마스터 명령어
#define MC_EVENT_CHANGE_MASTER				601		///< 방장권한을 뺏어온다
#define MC_EVENT_CHANGE_PASSWORD			602		///< 방의 비밀번호를 바꾼다
#define MC_EVENT_REQUEST_JJANG				611		///< 짱마크 부여
#define MC_EVENT_REMOVE_JJANG				612		///< 짱마크 회수
#define MC_EVENT_UPDATE_JJANG				613		///< 짱마크 알림


// 매치서버-클라이언트
#define MC_MATCH_LOGIN							1001	///< 로그인
#define MC_MATCH_RESPONSE_LOGIN					1002	///< 로그인 응답
#define MC_MATCH_LOGIN_NETMARBLE				1003	///< 넷마블에서 로그인
#define MC_MATCH_LOGIN_NETMARBLE_JP				1004	///< 일본넷마블에서 로그인
#define MC_MATCH_LOGIN_FROM_DBAGENT				1005	///< DBAgent로부터의 로그인 응답(일본넷마블 전용)
#define MC_MATCH_LOGIN_FROM_DBAGENT_FAILED		1008	///< DBAgent로부터의 로그인 실패(일본넷마블 전용)
#define MC_MATCH_FIND_HACKING					1009	///< 해킹 검출
#define MC_MATCH_DISCONNMSG						1010	///< Discconst메시지.

#define MC_MATCH_BRIDGEPEER						1006	///< MatchServer에 Peer정보 알림
#define MC_MATCH_BRIDGEPEER_ACK					1007	///< Peer정보 수신확인

#define MC_MATCH_OBJECT_CACHE					1101	///< 오브젝트 캐시

#define MC_MATCH_REQUEST_RECOMMANDED_CHANNEL	1201	///< 최초 참가할 채널에대해 추천을 요청한다.
#define MC_MATCH_RESPONSE_RECOMMANDED_CHANNEL	1202	///< 최초 참가할 채널에대해 추천한다.
#define MC_MATCH_CHANNEL_REQUEST_JOIN			1205	///< 채널에 참가(C -> S)
#define MC_MATCH_CHANNEL_REQUEST_JOIN_FROM_NAME	1206	///< 채널이름으로 채널 조인 요청
#define MC_MATCH_CHANNEL_RESPONSE_JOIN			1207	///< 채널 조인 응답(S -> C)
#define MC_MATCH_CHANNEL_LEAVE					1208	///< 채널 탈퇴
#define MC_MATCH_CHANNEL_LIST_START				1211	///< 채널 목록 전송시작 요청
#define MC_MATCH_CHANNEL_LIST_STOP				1212	///< 채널 목록 전송중지 요청
#define MC_MATCH_CHANNEL_LIST					1213	///< 채널 목록
#define MC_MATCH_CHANNEL_REQUEST_PLAYER_LIST	1221	///< 플레이어 리스트 요청
#define MC_MATCH_CHANNEL_RESPONSE_PLAYER_LIST	1222	///< 플레이어 리스트 응답
#define MC_MATCH_CHANNEL_REQUEST_CHAT			1225	///< 채널에 대화 요청
#define MC_MATCH_CHANNEL_CHAT					1226	///< 채널에서 대화 응답
#define MC_MATCH_CHANNEL_REQUEST_RULE			1230	///< 채널규칙 요청
#define MC_MATCH_CHANNEL_RESPONSE_RULE			1231	///< 채널규칙 응답
#define MC_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST		1232	///< 채널의 로비에 있는 모든 플레이어 리스트 요청
#define MC_MATCH_CHANNEL_RESPONSE_ALL_PLAYER_LIST		1233	///< 채널의 로비에 있는 플레이어 리스트 요청


#define MC_MATCH_STAGE_CREATE					1301	///< 스테이지 생성
#define MC_MATCH_RESPONSE_STAGE_CREATE			1302	///< 스테이지 생성 응답
#define MC_MATCH_STAGE_JOIN						1303	///< 스테이지 참가
#define MC_MATCH_REQUEST_STAGE_JOIN				1304	///< 스테이지 참가 요청
#define MC_MATCH_REQUEST_PRIVATE_STAGE_JOIN		1305	///< 비밀 스테이지 참가 요청
#define MC_MATCH_RESPONSE_STAGE_JOIN			1306	///< 스테이지에 조인여부를 알림
#define MC_MATCH_STAGE_LEAVE					1307	///< 스테이지 탈퇴
#define MC_MATCH_STAGE_REQUEST_PLAYERLIST		1308	///< 스테이지의 유저목록 요청
#define MC_MATCH_STAGE_FOLLOW					1309	///< 지정 유저를 따라서 스테이지 참가
#define MC_MATCH_RESPONSE_STAGE_FOLLOW			1310	///< 지정 유저를 따라서 스테이지 참가 응답
#define MC_MATCH_REQUEST_STAGE_LIST				1311	///< 클라이언트가 스테이지 리스트 요청
#define MC_MATCH_STAGE_LIST_START				1312	///< 스테이지 목록 전송시작 요청
#define MC_MATCH_STAGE_LIST_STOP				1313	///< 스테이지 목록 전송중지 요청
#define MC_MATCH_STAGE_LIST						1314	///< 스테이지 목록
#define MC_MATCH_STAGE_CHAT						1321	///< 스테이지 대화
#define MC_MATCH_STAGE_REQUEST_QUICKJOIN		1322	///< 퀵조인 요청
#define MC_MATCH_STAGE_RESPONSE_QUICKJOIN		1323	///< 퀵조인 응답
#define MC_MATCH_STAGE_GO						1331	///< 스테이지 번호로 참가
#define MC_MATCH_STAGE_REQUIRE_PASSWORD			1332	///< 비밀방이라 패스워드가 필요하다.(S -> C)


#define MC_MATCH_STAGE_REQUEST_ENTERBATTLE		1401	///< 스테이지 전투 참가한다고 요청
#define MC_MATCH_STAGE_ENTERBATTLE				1402	///< 스테이지 전투참가
#define MC_MATCH_STAGE_LEAVEBATTLE				1403	///< 스테이지 전투탈퇴

#define MC_MATCH_REQUEST_STAGESETTING			1411	///< 스테이지의 설정을 요청
#define MC_MATCH_RESPONSE_STAGESETTING			1412	///< 스테이지의 설정을 알림
#define MC_MATCH_STAGESETTING					1413	///< 스테이지 설정
#define MC_MATCH_STAGE_MAP						1414	///< 스테이지 맵 설정
#define MC_MATCH_STAGE_REQUEST_FORCED_ENTRY		1415	///< 스테이지에서 게임에 난입 요청
#define MC_MATCH_STAGE_RESPONSE_FORCED_ENTRY	1416	///< 스테이지 게임 난입 응답
#define MC_MATCH_STAGE_MASTER					1421	///< 스테이지 방장 알림
#define MC_MATCH_STAGE_PLAYER_STATE				1422	///< 스테이지 레디 등의 플레이어 상태 정보
#define MC_MATCH_STAGE_TEAM						1423	///< 스테이지 팀
#define MC_MATCH_STAGE_START					1431	///< 스테이지 시작(카운트다운)
#define MC_MATCH_STAGE_LAUNCH					1432	///< 스테이지 실행
#define MC_MATCH_LOADING_COMPLETE				1441	///< 로딩이 끝났음
#define MC_MATCH_STAGE_FINISH_GAME				1442	///< 스테이지 게임 종료

#define MC_MATCH_REQUEST_GAME_INFO				1451	///< 게임안 정보를 달라 요청
#define MC_MATCH_RESPONSE_GAME_INFO				1452	///< 게임안 정보를 알려준다

#define MC_MATCH_REQUEST_PEERLIST				1461	///< 참여중인 유저들의 Peer정보를 요청
#define MC_MATCH_RESPONSE_PEERLIST				1462	///< 참여중인 유저들의 Peer정보를 알림

#define MC_MATCH_REQUEST_PEER_RELAY				1471	///< 메치서버에 해당Peer에 대한 중계요청
#define MC_MATCH_RESPONSE_PEER_RELAY			1472	///< 해당Peer에 대한 중계허가 통보

// 게임 관련
#define MC_MATCH_GAME_ROUNDSTATE				1501	///< 라운드의 상태를 동기화
#define MC_MATCH_ROUND_FINISHINFO				1502	///< 라운드 종료시 정보 업데이트 (C <- S)
#define MC_MATCH_GAME_KILL						1511	///< 캐릭터 사망 (C -> S)
#define MC_MATCH_GAME_DEAD						1512	///< 캐릭터 죽었다 (S -> C)
#define MC_MATCH_GAME_LEVEL_UP					1513	///< 게임중 레벨업
#define MC_MATCH_GAME_LEVEL_DOWN				1514	///< 게임중 레벨다운
#define MC_MATCH_GAME_REQUEST_SPAWN				1515	///< 캐릭터 부활요청 (C -> S)
#define MC_MATCH_GAME_RESPONSE_SPAWN			1516	///< 캐릭터 부활요청 (C -> S)
#define MC_MATCH_GAME_TEAMBONUS					1517	///< 팀 보너스(S -> C)

#define MC_MATCH_GAME_REQUEST_TIMESYNC			1521	///< Match서버에 시간동기화 요청
#define MC_MATCH_GAME_RESPONSE_TIMESYNC			1522	///< 시간동기화
#define MC_MATCH_GAME_REPORT_TIMESYNC			1523	///< 시간동기 검사

#define MC_MATCH_REQUEST_SUICIDE				1531	///< 자살 요청
#define MC_MATCH_RESPONSE_SUICIDE				1532	///< 자살 응답
#define MC_MATCH_REQUEST_OBTAIN_WORLDITEM		1541	///< 아이템 먹도록 요청
#define MC_MATCH_OBTAIN_WORLDITEM				1542	///< 아이템 먹음 브로드캐스트(S -> C)
#define MC_MATCH_SPAWN_WORLDITEM				1543	///< 아이템이 스폰됨
#define MC_MATCH_REQUEST_SPAWN_WORLDITEM		1544	///< 아이템을 떨구도록 요청(C -> S)
#define MC_MATCH_REMOVE_WORLDITEM				1545	///< 아이템 없어짐(S -> C)
#define MC_MATCH_ASSIGN_COMMANDER				1551	///< 암살전 대장으로 임명
#define MC_MATCH_RESET_TEAM_MEMBERS				1552	///< 팀멤버 다시 세팅
#define MC_MATCH_SET_OBSERVER					1553	///< 강제로 옵저버모드로 전환


// 동의관련
#define MC_MATCH_REQUEST_PROPOSAL				1561	///< 다른 사람의 동의 요청
#define MC_MATCH_RESPONSE_PROPOSAL				1562	///< 동의 요청 응답
#define MC_MATCH_ASK_AGREEMENT					1563	///< 다른 사람에게 동의 질의(S -> C)
#define MC_MATCH_REPLY_AGREEMENT				1564	///< 동의 응답(C -> S)

// 래더 관련
#define MC_MATCH_LADDER_REQUEST_CHALLENGE		1571	///< 래더 도전 요청(C -> S)
#define MC_MATCH_LADDER_RESPONSE_CHALLENGE		1572	///< 래더 도전 응답
#define MC_MATCH_LADDER_SEARCH_RIVAL			1574	///< 래더 검색중		
#define MC_MATCH_LADDER_REQUEST_CANCEL_CHALLENGE	1575	///< 래더 도전 취소요청
#define MC_MATCH_LADDER_CANCEL_CHALLENGE		1576	///< 래더 도전 취소
#define MC_MATCH_LADDER_PREPARE					1578	///< 래더 시작준비
#define MC_MATCH_LADDER_LAUNCH					1579	///< 래더 런치 (S -> C)

// 의사소통 관련
#define MC_MATCH_USER_WHISPER					1601	///< 귓속말
#define MC_MATCH_USER_WHERE						1602	///< 위치확인
#define MC_MATCH_USER_OPTION					1605	///< 각종옵션(귓말거부,초대거부,친구거부 등등)
#define MC_MATCH_CHATROOM_CREATE				1651	///< 채팅룸 개설
#define MC_MATCH_CHATROOM_JOIN					1652	///< 채팅룸 참가
#define MC_MATCH_CHATROOM_LEAVE					1653	///< 채팅룸 탈퇴
#define MC_MATCH_CHATROOM_INVITE				1661	///< 채팅룸 초대
#define MC_MATCH_CHATROOM_CHAT					1662	///< 채팅
#define MC_MATCH_CHATROOM_SELECT_WRITE			1665	///< 채팅룸 선택

// 캐릭터 관련
#define MC_MATCH_REQUEST_ACCOUNT_CHARLIST		1701	///< 계정의 전체 캐릭터 리스트를 요청
#define MC_MATCH_RESPONSE_ACCOUNT_CHARLIST		1702	///< 계정의 전체 캐릭터 리스트를 응답
#define MC_MATCH_REQUEST_SELECT_CHAR			1703	///< 캐릭터 선택 요청
#define MC_MATCH_RESPONSE_SELECT_CHAR			1704	///< 캐릭터 선택 응답
#define MC_MATCH_REQUEST_MYCHARINFO				1705	///< 내 캐릭터 정보 요청
#define MC_MATCH_RESPONSE_MYCHARINFO			1706	///< 내 캐릭터 정보 응답
#define MC_MATCH_REQUEST_CREATE_CHAR			1711	///< 내 캐릭터 생성 요청
#define MC_MATCH_RESPONSE_CREATE_CHAR			1712	///< 내 캐릭터 생성 응답
#define MC_MATCH_REQUEST_DELETE_CHAR			1713	///< 내 캐릭터 삭제 요청
#define MC_MATCH_RESPONSE_DELETE_CHAR			1714	///< 내 캐릭터 삭제 응답
#define MC_MATCH_REQUEST_COPY_TO_TESTSERVER		1715	///< 캐릭터 테스트서버로 전송 요청 - 미구현
#define MC_MATCH_RESPONSE_COPY_TO_TESTSERVER	1716	///< 캐릭터 테스트서버로 전송 응답 - 미구현
#define MC_MATCH_REQUEST_CHARINFO_DETAIL		1717	///< 다른 플레이어 정보보기 등의 자세한 플레이어 정보 요청
#define MC_MATCH_RESPONSE_CHARINFO_DETAIL		1718	///< 다른 플레이어 정보보기 등의 자세한 플레이어 정보 응답
#define MC_MATCH_REQUEST_ACCOUNT_CHARINFO		1719	///< 계정의 한 캐릭터 정보 요청
#define MC_MATCH_RESPONSE_ACCOUNT_CHARINFO		1720	///< 계정의 한 캐릭터 정보 응답

// 아이템 관련
#define MC_MATCH_REQUEST_SIMPLE_CHARINFO		1801	///< 캐릭터의 간단한 정보 요청	
#define MC_MATCH_RESPONSE_SIMPLE_CHARINFO		1802	///< 캐릭터의 간단한 정보 응답
#define MC_MATCH_REQUEST_MY_SIMPLE_CHARINFO		1803	///< 내 캐릭터 XP, BP등 정보 요청
#define MC_MATCH_RESPONSE_MY_SIMPLE_CHARINFO	1804	///< 내 캐릭터 XP, BP등 정보 응답

#define MC_MATCH_REQUEST_BUY_ITEM				1811	///< 아이템 구매 요청
#define MC_MATCH_RESPONSE_BUY_ITEM				1812	///< 아이템 구매 응답
#define MC_MATCH_REQUEST_SELL_ITEM				1813	///< 아이템 판매 요청
#define MC_MATCH_RESPONSE_SELL_ITEM				1814	///< 아이템 판매 응답
#define MC_MATCH_REQUEST_SHOP_ITEMLIST			1815	///< 샵에서 판매하는 아이템 리스트 요청
#define MC_MATCH_RESPONSE_SHOP_ITEMLIST			1816	///< 샵에서 판매하는 아이템 리스트 응답

#define MC_MATCH_REQUEST_CHARACTER_ITEMLIST		1821	///< 내 아이템 리스트 주시오
#define MC_MATCH_RESPONSE_CHARACTER_ITEMLIST	1822	///< 아이템 리스트 여깄오
#define MC_MATCH_REQUEST_EQUIP_ITEM				1823	///< 아이템 장비 요청
#define MC_MATCH_RESPONSE_EQUIP_ITEM			1824	///< 아이템 장비 응답
#define MC_MATCH_REQUEST_TAKEOFF_ITEM			1825	///< 아이템 해제 요청
#define MC_MATCH_RESPONSE_TAKEOFF_ITEM			1826	///< 아이템 해제 응답

#define MC_MATCH_REQUEST_ACCOUNT_ITEMLIST		1831	///< 내 창고 아이템 리스트 주시오
#define MC_MATCH_RESPONSE_ACCOUNT_ITEMLIST		1832	///< 창고 아이템 리스트 여깄오
#define MC_MATCH_REQUEST_BRING_ACCOUNTITEM		1833	///< 창고 아이템을 내 캐릭터로 가져오기
#define MC_MATCH_RESPONSE_BRING_ACCOUNTITEM		1834	///< 창고 아이템 가져오기 응답
#define MC_MATCH_REQUEST_BRING_BACK_ACCOUNTITEM	1835	///< 내 캐쉬아이템 창고로 옮기기 요청
#define MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM	1836	///< 내 캐쉬아이템 창고로 옮기기 응답
#define MC_MATCH_EXPIRED_RENT_ITEM				1837	///< 아이템이 기간 만료되었습니다.(S -> C)

// 친구 관련
#define MC_MATCH_FRIEND_ADD						1901	///< 친구 추가
#define MC_MATCH_FRIEND_REMOVE					1902	///< 친구 삭제
#define MC_MATCH_FRIEND_LIST					1903	///< 친구 목록 요청
#define MC_MATCH_RESPONSE_FRIENDLIST			1904	///< 친구 목록 전송
#define MC_MATCH_FRIEND_MSG						1905	///< 친구 채팅


// 클랜관련
#define MC_MATCH_CLAN_REQUEST_CREATE_CLAN			2000	///< 클랜 생성 요청(C -> S)
#define MC_MATCH_CLAN_RESPONSE_CREATE_CLAN			2001	///< 클랜 생성 응답(S -> C)
#define MC_MATCH_CLAN_ASK_SPONSOR_AGREEMENT			2002	///< 발기인에게 클랜 생성 동의 질의(S -> C)
#define MC_MATCH_CLAN_ANSWER_SPONSOR_AGREEMENT		2003	///< 발기인이 동의를 응답(C -> S)
#define MC_MATCH_CLAN_REQUEST_AGREED_CREATE_CLAN	2004	///< 발기인이 동의한 클랜 생성 요청(C -> S)
#define MC_MATCH_CLAN_RESPONSE_AGREED_CREATE_CLAN	2005	///< 발기인이 동의한 클랜 생성 응답(S -> C)
#define MC_MATCH_CLAN_REQUEST_CLOSE_CLAN			2006	///< 마스터가 클랜 폐쇄 요청(C -> S)
#define MC_MATCH_CLAN_RESPONSE_CLOSE_CLAN			2007	///< 마스터의 클랜 폐쇄 응답(S -> C)
#define MC_MATCH_CLAN_REQUEST_JOIN_CLAN				2008	///< 운영자가 가입자의 가입을 요청(C -> S)
#define MC_MATCH_CLAN_RESPONSE_JOIN_CLAN			2009	///< 운영자의 가입자 가입의 응답(S -> C)
#define MC_MATCH_CLAN_ASK_JOIN_AGREEMENT			2010	///< 가입자에게 가입 동의 질의(S -> C)
#define MC_MATCH_CLAN_ANSWER_JOIN_AGREEMENT			2011	///< 가입자의 가입 동의 응답(C -> S)
#define MC_MATCH_CLAN_REQUEST_AGREED_JOIN_CLAN		2012	///< 가입자가 동의한 클랜 가입 요청(C -> S)
#define MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN		2013	///< 가입자가 동의한 클랜 가입 응답(S -> C)
#define MC_MATCH_CLAN_REQUEST_LEAVE_CLAN			2014	///< 클랜원이 탈퇴를 요청(C -> S)
#define MC_MATCH_CLAN_RESPONSE_LEAVE_CLAN			2015	///< 클랜원의 탈퇴의 응답(S -> C)
#define MC_MATCH_CLAN_UPDATE_CHAR_CLANINFO			2016	///< 캐릭터의 클랜정보가 업데이트됨(S -> C)
#define MC_MATCH_CLAN_MASTER_REQUEST_CHANGE_GRADE	2017	///< 멤버의 권한 변경 요청(C -> S)
#define MC_MATCH_CLAN_MASTER_RESPONSE_CHANGE_GRADE	2018	///< 멤버의 권한 변경 응답(S -> C)
#define MC_MATCH_CLAN_ADMIN_REQUEST_EXPEL_MEMBER	2019	///< 멤버를 탈퇴 요청(C -> S)
#define MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER	2020	///< 멤버를 탈퇴 응답(S -> C)
#define MC_MATCH_CLAN_REQUEST_MSG					2021	///< 클랜 채팅 요청(C -> S)
#define MC_MATCH_CLAN_MSG							2022	///< 클랜 채팅(S -> C)
#define MC_MATCH_CLAN_REQUEST_MEMBER_LIST			2023	///< 클랜 멤버 리스트 요청(C -> S)
#define MC_MATCH_CLAN_RESPONSE_MEMBER_LIST			2024	///< 클랜 멤버 리스트 응답(S -> C)
#define MC_MATCH_CLAN_REQUEST_CLAN_INFO				2025	///< 클랜 정보 요청(C -> S)
#define MC_MATCH_CLAN_RESPONSE_CLAN_INFO			2026	///< 클랜 정보 응답(S -> C)
#define MC_MATCH_CLAN_STANDBY_CLAN_LIST				2027	///< 클랜전 대기중인 클랜 리스트 응답(S -> C)
#define MC_MATCH_CLAN_MEMBER_CONNECTED				2028	///< 클랜원이 접속함(S -> C)
#define MC_MATCH_CLAN_REQUEST_EMBLEMURL				2051	///< 클랜마크 URL 요청(C -> S)
#define MC_MATCH_CLAN_RESPONSE_EMBLEMURL			2052	///< 클랜마크 URL 응답(S -> C)
#define MC_MATCH_CLAN_LOCAL_EMBLEMREADY				2055	///< 클랜마크 다운로드 완료(C -> C)
#define MC_MATCH_CLAN_ACCOUNCE_DELETE				2056	///< 클랜 폐쇄 요청 접수 정보 공지.(S -> C)


// 투표관련
#define MC_MATCH_CALLVOTE							2100	///< 투표를 제안한다. (C -> S)
#define MC_MATCH_NOTIFY_CALLVOTE					2101	///< 투표제안을 알린다. (S -> C)
#define MC_MATCH_NOTIFY_VOTERESULT					2102	///< 투표결과를 알린다. (S -> C)
#define MC_MATCH_VOTE_YES							2105	///< 투표 (C -> S)
#define MC_MATCH_VOTE_NO							2106	///< 투표 (C -> S)
#define MC_MATCH_VOTE_RESPONSE						2107	///< 투표 예외.(S -> C)
#define MC_MATCH_VOTE_STOP							2108	///< 투표 취소. (S -> C)

// 방송관련
#define MC_MATCH_BROADCAST_CLAN_RENEW_VICTORIES		2200	///< 클랜이 계속 연승중입니다.(S -> C)
#define MC_MATCH_BROADCAST_CLAN_INTERRUPT_VICTORIES	2201	///< 클랜의 연승을 저지하였습니다.(S -> C)
#define MC_MATCH_BROADCAST_DUEL_RENEW_VICTORIES		2202	///< xx님이 xx채널의 xx방에서 x연승중입니다.(S -> C)
#define MC_MATCH_BROADCAST_DUEL_INTERRUPT_VICTORIES		2203	///< xx님이 xx님의 xx연승을 저지했습니다. (S -> C)



// 게임 룰 관련
// 버서커 모드
#define MC_MATCH_ASSIGN_BERSERKER					3001	///< 버서커 임명(S -> C)


#define MC_MATCH_DUEL_QUEUEINFO						3100	///< 대기열 순번 알림(S -> C)


// 매치 에이젼트 관련 명령어
#define MC_MATCH_REGISTERAGENT				5001	///< 에이젼트 등록
#define MC_MATCH_UNREGISTERAGENT			5002	///< 에이젼트 해지
#define MC_MATCH_AGENT_REQUEST_LIVECHECK	5011	///< 에이젼트 확인요청
#define MC_MATCH_AGENT_RESPONSE_LIVECHECK	5012	///< 에이젼트 확인응답
#define MC_AGENT_ERROR						5013	///< 에이전트 관련 에러

#define MC_AGENT_CONNECT					5021	///< 매치서버에 에이젼트 서버 등록
#define MC_AGENT_DISCONNECT					5022	///< 매치서버에 에이젼트 서버 등록해지
#define MC_AGENT_LOCAL_LOGIN				5023	///< 클라이언트 접속처리
#define MC_AGENT_RESPONSE_LOGIN				5024	///< 클라이언트에 로그인 통지
//#define MC_AGENT_MATCH_VALIDATE				5031	///< 매치서버와의 접속확인
#define MC_AGENT_STAGE_RESERVE				5051	///< 에이전트에 스테이지 배정
#define MC_AGENT_STAGE_RELEASE				5052	///< 에이전트에 스테이지 해지
#define MC_AGENT_STAGE_READY				5053	///< 에이전트에 스테이지 준비완료
#define MC_AGENT_LOCATETO_CLIENT			5061	///< 클라이언트에게 Agent 알림
#define MC_AGENT_RELAY_PEER					5062	///< 에이전트에 중계지시
#define MC_AGENT_PEER_READY					5063	///< 메치서버에 피어준비 알림
#define MC_AGENT_PEER_BINDTCP				5071	///< 클라이언트 Bind TCP
#define MC_AGENT_PEER_BINDUDP				5072	///< 클라이언트 Bind UDP
#define MC_AGENT_PEER_UNBIND				5073	///< 클라이언트 Unbind

#define MC_AGENT_TUNNELING_TCP				5081	///< TCP 터널링
#define MC_AGENT_TUNNELING_UDP				5082	///< UDP 터널링
#define MC_AGENT_ALLOW_TUNNELING_TCP		5083	///< TCP 터널링 허용
#define MC_AGENT_ALLOW_TUNNELING_UDP		5084	///< UDP 터널링 허용
#define MC_AGENT_DEBUGPING					5101	///< 디버깅용
#define MC_AGENT_DEBUGTEST					5102	///< 디버깅용

#define MC_TEST_BIRDTEST1					60001	///< 버드 테스트1
#define MC_TEST_PEERTEST_PING				60002	///< Target Peer Test
#define MC_TEST_PEERTEST_PONG				60003	///< Target Peer Test



/// 커맨드 세부 옵션

// MC_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST의 세부옵션
#define MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_NORMAL		0		
#define MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_NONCLAN		1
#define MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_MYCLAN		2


// 퀘스트 관련 명령어(서바이벌 모드, 퀘스트 모드 공통)
#define MC_QUEST_NPC_SPAWN					6000	///< NPC 스폰(S -> C)
#define MC_QUEST_ENTRUST_NPC_CONTROL		6001	///< NPC의 조종을 피어에게 위임(S -> C)
#define MC_QUEST_CHECKSUM_NPCINFO			6002	///< NPC정보들의 체크섬을 서버에게 보낸다(C -> S)
#define MC_QUEST_REQUEST_NPC_DEAD			6003	///< NPC 죽음 요청(C -> S)
#define MC_QUEST_NPC_DEAD					6004	///< NPC 죽음(S -> C)
#define MC_QUEST_REFRESH_PLAYER_STATUS		6005	///< 죽었으면 다시 태어나고, 모든 상태를 최상으로 복귀(S -> C)
#define MC_QUEST_NPC_ALL_CLEAR				6006	///< NPC 모두 삭제(S -> C)
#define MC_MATCH_QUEST_REQUEST_DEAD			6007	///< 플레이어 죽었다고 요청(C -> S) 퀘스트모드에서는 MC_MATCH_GAME_KILL대신 보낸다.
#define MC_MATCH_QUEST_PLAYER_DEAD			6008	///< 플레이어 죽음(S -> C)
#define MC_QUEST_OBTAIN_QUESTITEM			6009	///< 퀘스트 아이템 획득(S -> C)
#define MC_QUEST_STAGE_MAPSET				6010	///< 스테이지 맵셋 변경(S <-> C)
#define MC_QUEST_OBTAIN_ZITEM				6011	///< 일반 아이템 획득(S -> C)
#define MC_QUEST_PING						6012	///< 서버->클라 핑 (S -> C)
#define MC_QUEST_PONG						6013	///< 클라->서버 핑 (C -> S)


#define MC_QUEST_PEER_NPC_BASICINFO			6040	///< 정기적으로 업데이트되는 NPC 기본정보
#define MC_QUEST_PEER_NPC_HPINFO			6041	///< 정기적으로 업데이트되는 NPC 체력정보
#define MC_QUEST_PEER_NPC_ATTACK_MELEE		6042	///< 근접 공격
#define MC_QUEST_PEER_NPC_ATTACK_RANGE		6043	///< 원거리 공격
#define MC_QUEST_PEER_NPC_SKILL_START		6044	///< 스킬 캐스팅 시작
#define MC_QUEST_PEER_NPC_SKILL_EXECUTE		6045	///< 스킬 캐스팅성공 효과 시작
#define MC_QUEST_PEER_NPC_DEAD				6046	///< 죽은 그 시점에 피어들에게 보낸다. 

// 퀘스트 게임 진행 관련(퀘스트 모드)
#define MC_QUEST_GAME_INFO					6051	///< 게임 정보. 로딩할 리소스 등을 알림(S -> C)
#define MC_QUEST_COMBAT_STATE				6052	///< 퀘스트 게임내 상태 정보(S -> C)
#define MC_QUEST_SECTOR_START				6053	///< 맵섹터 게임 시작(S -> C)
#define MC_QUEST_COMPLETED					6054	///< 해당 퀘스트 클리어(S -> C)
#define MC_QUEST_FAILED						6055	///< 퀘스트 실패(S -> C)

#define MC_QUEST_REQUEST_MOVETO_PORTAL		6057	///< 포탈로 이동함(C -> S)
#define MC_QUEST_MOVETO_PORTAL				6058	///< 포탈로 이동(S -> C)
#define MC_QUEST_READYTO_NEWSECTOR			6059	///< 새로운 섹터 준비 완료(S <-> C)

#define MC_GAME_START_FAIL					6060	///< 퀘스트 시작을 실패.(S -> C)
#define MC_QUEST_STAGE_GAME_INFO			6061	///< 대기중 스테이지 정보 알림(QL, 시나리오ID). (S -> C)
#define MC_QUEST_SECTOR_BONUS				6062	///< 섹터 보너스(S -> C)

// 퀘스트 게임 진행 관련(서바이벌 모드)
#define MC_QUEST_ROUND_START				6100	///< 새로운 라운드 시작(S -> C)
#define MC_QUEST_REQUEST_QL					6101	///< 현재 퀘스트 스테이지의 QL정보를 요청.(C -> S)
#define MC_QUEST_RESPONSE_QL				6102	///< 현재 퀘스트 스테이지의 QL정보를 알림.(S -> C)


// 퀘스트 테스트용
#define MC_QUEST_TEST_REQUEST_NPC_SPAWN		6901	///< NPC 스폰 부탁(C -> S)
#define MC_QUEST_TEST_REQUEST_CLEAR_NPC		6902	///< NPC Clear 부탁(C -> S)
#define MC_QUEST_TEST_REQUEST_SECTOR_CLEAR	6903	///< 섹터 Clear(C -> S)
#define MC_QUEST_TEST_REQUEST_FINISH		6904	///< 퀘스트 Complete(C -> S)


// MatchServer Schedule관련.
#define MC_MATCH_SCHEDULE_ANNOUNCE_MAKE				7001	///< 스케쥴 공지 생성.
#define MC_MATCH_SCHEDULE_ANNOUNCE_SEND				7002	///< 스케쥴 공지 전송.
#define MC_MATCH_SCHEDULE_CLAN_SERVER_SWITCH_DOWN	7003	///< 클랜서버 클랜전 비활성화.
#define MC_MATCH_SCHEDULE_CLAN_SERVER_SWITCH_ON		7004	///< 클랜서버 클랜전 활성화.
#define MC_MATCH_SCHEDULE_STOP_SERVER				7005	///< 서버 종료.


//// 피어투피어 관련 명령어 /////////////////////////////////////////////////////////////
#define MC_PEER_PING				10001	///< peer들 사이의 network latency(ping)을 측정하기위한 command
#define MC_PEER_PONG				10002	///< ping에 대한 응답
#define MC_PEER_UDPTEST				10005	///< Peer to Peer UDP 전송이 가능한지 검사한다.
#define MC_PEER_UDPTEST_REPLY		10006	///< Peer to Peer UDP 전송이 가능한지 검사한다.

#define MC_PEER_OPENED				10011	///< 개별 Peer 통신 개통알림
#define MC_PEER_BASICINFO			10012	///< 정기적으로 업데이트되는 캐릭터 기본정보 투표판정의 근거가 된다
#define MC_PEER_HPINFO				10013	///< 정기적으로 업데이트되는 캐릭터 체력정보 투표 판정의 근거가 된다
#define MC_PEER_HPAPINFO			10014	///< 정기적으로 업데이트되는 캐릭터 체력/아머 정보

#define MC_PEER_CHANGECHARACTER		10015
#define MC_PEER_MOVE				10021   ///< 에 나중에 MC_OBJECT_MOVE로 바뀌어야할듯..
#define MC_PEER_CHANGE_WEAPON		10022   
#define MC_PEER_CHANGE_PARTS		10023
#define MC_PEER_ATTACK				10031
#define MC_PEER_DAMAGE				10032
#define MC_PEER_RELOAD				10033
#define MC_PEER_SHOT				10034
#define MC_PEER_SHOT_SP				10035
#define MC_PEER_SKILL				10036
#define MC_PEER_SHOT_MELEE			10037	///< 근접공격
#define MC_PEER_DIE					10041
#define MC_PEER_SPAWN				10042
#define MC_PEER_DASH				10045
#define MC_PEER_SPMOTION			10046
#define MC_PEER_CHAT				10052
#define MC_PEER_CHAT_ICON			10053	///< 채팅 시작/끝. 머리에 아이콘을 보여주거나 감춘다.
#define MC_PEER_REACTION			10054	///< 이펙트를 위해 알려준다 
#define MC_PEER_ENCHANT_DAMAGE		10055	///< 인챈트 데미지를 맞았다


// 퀘스트 아이템 관련 부분.
#define MC_MATCH_REQUEST_CHAR_QUEST_ITEM_LIST		21000	///< 나의 퀘스트 아이템 목록을 요청.(C -> S)
#define MC_MATCH_RESPONSE_CHAR_QUEST_ITEM_LIST		21001	///< 나의 퀘스트 아이템 목록을 전송.(S -> C)
#define MC_MATCH_REQUEST_BUY_QUEST_ITEM				21002	///< 퀘스트 아이템 구입 요청.(C -> S)
#define MC_MATCH_RESPONSE_BUY_QUEST_ITEM			21003	///< 퀘스트 아이템 구입 완료.(S -> C)
#define MC_MATCH_REQUEST_SELL_QUEST_ITEM			21004	///< 나의 퀘스트 아이템 판매 요청.(C -> S)
#define MC_MATCH_RESPONSE_SELL_QUEST_ITEM			21005	///< 나의 퀘스트 아이템 판매 완료.(S -> C)
#define MC_MATCH_USER_REWARD_QUEST					21006	///< 퀘스트가 완료된후 보상된 퀘스트 아이템 정보.
#define MC_MATCH_REQUEST_DROP_SACRIFICE_ITEM		21007	///< 희생 아이템을 슬롯에 놓기 요청.(C -> S)
#define MC_MATCH_RESPONSE_DROP_SACRIFICE_ITEM		21008	///< 희생 아이템을 슬롯에 놓기 응담.(S -> C)
#define MC_MATCH_REQUEST_CALLBACK_SACRIFICE_ITEM	21009	///< 슬롯에 올려놓은 희생 아이템을 회수 요청.(C -> S)
#define MC_MATCH_RESPONSE_CALLBACK_SACRIFICE_ITEM	21010	///< 스롯에 올려놓은 희생 아이템을 휘수 응답.(S -> C)
#define MC_MATCH_REQUEST_SLOT_INFO					21011	///< 희생 아이템 슬롯 정보를 요청.(C -> S)
#define MC_MATCH_RESPONSE_SLOT_INFO					21012	///< 희생 아이템 슬롯 정보를 알림.(S -> C)
#define MC_MATCH_NEW_MONSTER_INFO					21013	///< 몬스터 도감에 추가될 새로운 몬스터 정보.
#define MC_MATCH_REQUEST_MONSTER_BIBLE_INFO			21014	///< 몬스터 도감의 내용 요청.(C -> S)
#define MC_MATCH_RESPONSE_MONSTER_BIBLE_INFO		21015	///< 몬스터 도감의 내용 알림.(S -> C)


/// Keeper와 Keeper Manager와 통신하는데 필요한 명령. 2005.06.03 커맨드 정리 필요.
#define MC_KEEPER_MANAGER_CONNECT				31001	///< keeper Manager에서 keeper로 접속 요청.
#define MC_CHECK_KEEPER_MANAGER_PING			31002	///< Keeper와 Keeper Manager가 정상적으로 연결되어 있는지 검사.
#define MC_REQUEST_MATCHSERVER_STATUS			31004	///< MatchServer의 정보를 요청.(Keeper -> MatchServer)
#define MC_RESPONSE_MATCHSERVER_STATUS			31005	///< Keeper로 MatchServer의 정보를 전송해 줌.(MatchServer -> Keeper)
// 2005.06.01 ~
#define MC_RESPONSE_KEEPER_MANAGER_CONNECT		31006	///< Keeper manager의 접속이 정상적으로 이루어 짐을 알림.( Keeper -> Keeper Manager )
#define MC_REQUEST_CONNECTION_STATE				31007	///< 현제 서버의 상태.(Keeper Manager -> Keeper)
#define MC_RESPONSE_CONNECTION_STATE			31008	///< 서버상태 응답.(Keeper -> Keeper Manager)
#define MC_REQUEST_SERVER_HEARBEAT				31009	///< 서버가 동작하고 있는지 응답 요청.(Keeper -> MatchServer)
#define MC_RESPONSE_SERVER_HEARHEAT				31010	///< 서버가 동작하고 있을을 응답.(MatchServer -> Keeper)
#define MC_REQUEST_KEEPER_CONNECT_MATCHSERVER	31011	///< Keeper가 MatchServer로 접속하도록 요청함(Keeper Manager -> Keeper -> MatchServer )
#define MC_RESPONSE_KEEPER_CONNECT_MATCHSERVER	31012	///< Keeper가 MatchServer에 접속했다는 응답.(MatchServer -> Keeper -> Keeper Manager )
#define MC_REQUEST_REFRESH_SERVER				31013	///< Keeper Manager의 서버 리스트 수동 갱신을 위해서 Keeper로 MatchServer의 Heartbaet채크 요청.(Keeper Manager -> Keeepr)
#define MC_REQUEST_LAST_JOB_STATE				31014	///< 마지막 실행된 일의 결과를 요청.(Keeper -> Keeper Manager )
#define MC_RESPONSE_LAST_JOB_STATE				31015	///< 작업 결과 요청 응답.(Keeper -> Keeper Manager)
#define MC_REQUEST_CONFIG_STATE					31016	///< Keeper에 저장된 설정 정보 요청.(Keeper Manager -> Keeper)
#define MC_RESPONSE_CONFIG_STATE				31017	///< Keeper에 저장된 설정 정보 요청 응답.(Keeper -> Keeper Manager)
#define MC_REQUEST_SET_ONE_CONFIG				31018	///< Config하나 설정 요청.(Keeper Manager -> Keeper)
#define MC_RESPONSE_SET_ONE_CONFIG				31019	///< Config하나 설정 요청 응답.(Keeper -> Keeper Manager)
#define MC_REQUEST_KEEPERMGR_ANNOUNCE			31020	///< Keeper Manager에서 각 서버로 공지를 보내도록 Keeper에 요청함.(Keeper Manager -> Keeper)
#define MC_REQUEST_KEEPER_ANNOUNCE				31021	///< Keeper에서 MatchServer가 공지사항을 알리도록 요청함.(Keeper -> MatchServer)
#define MC_REQUEST_RESET_PATCH					31022	///< 현제+까지 진행한 패치 작업상태를 초기화함.
#define MC_REQUEST_DISCONNECT_SERVER			31023	///< Keeper와 MatchServer와의 연결 종료.
#define MC_REQUEST_REBOOT_WINDOWS				31024	///< 윈도우 재부팅.(Keeper Manager -> Keeper)
#define MC_REQUEST_ANNOUNCE_STOP_SERVER			31025	///< 공지사항과 함게 서버를 종료 시킴 요청.
#define MC_RESPONSE_ANNOUNCE_STOP_SERVER		31026	///< 공지사항과 함게 서버를 종료 시킴 요청 응답.
#define MC_REQUEST_SERVER_AGENT_STATE			31027	///< 현제 서버와 에이전트의 실행 상태 요청.
#define MC_RESPONSE_SERVER_AGENT_STATE			31028	///< 현제 서버와 에이전트의 실행 상태 요청 응답.
#define MC_REQUEST_WRITE_CLIENT_CRC				31029	///< 클라이언트의 CRC CheckSum을 서버 ini파일에 저장 요청.(KeeperManager -> Keeper)
#define MC_RESPONSE_WRITE_CLIENT_CRC			31030	///< 클라이언트의 CRC CheckSum을 서버 ini파일에 저장 요청 응답.(Keeper -> Keeper Manager)
#define MC_REQUEST_KEEPER_RELOAD_SERVER_CONFIG	31031	///< Keeper와 연결되어있는 서버가 설정 파일을 다시 로드하도록 요청함.(KeeperManager -> Keeper)
#define MC_REQUEST_RELOAD_CONFIG				31032	///< 서버의 설정 파일을 다시 로다하도록 요청.(Keeper -> server)
#define MC_REQUEST_KEEPER_ADD_HASHMAP			31033	
#define MC_RESPONSE_KEEPER_ADD_HASHMAP			31034
#define MC_REQUEST_ADD_HASHMAP					31035
#define MC_RESPONSE_ADD_HASHMAP					31036

// MatchServer 패치 관련.
#define MC_REQUEST_DOWNLOAD_SERVER_PATCH_FILE	32024	///< Keeper가 FTP서버에서 Patch파일을 다운로드 시킴.(Keeper Manager -> Keeper )
#define MC_REQUEST_START_SERVER					32026	///< MatchServer실행 요청.(Keeper Manager -> Keeper)
#define MC_REQUEST_STOP_SERVER					32028	///< 서버 정지 요청.(Keeper -> Keeper Manager)
#define MC_REQUEST_PREPARE_SERVER_PATCH			32030	///< 패치 준비 작업.(압축 풀기, 임시 파일 제거.)
#define MC_REQUEST_SERVER_PATCH					32032	///< 패치 파일을 복사함.(Keeper Manager -> Keeper )
// MatchAgent 패치 관련.
#define MC_REQUEST_STOP_AGENT_SERVER			33034	///< Agent server정지 요청.(Keeper Manager -> Keeper )
#define MC_REQUEST_START_AGENT_SERVER			33036	///< Agent server시작 요청.(Keeper Manager -> Keeper)
#define MC_REQUEST_DOWNLOAD_AGENT_PATCH_FILE	33038	///< request agent patch file download.(Keeper Manager -> Keeper)
#define MC_REQUEST_PREPARE_AGENT_PATCH			33040	///< reqeust prepare agent patch.(Keeper Manager -> Keeper)
#define MC_REQUEST_AGENT_PATCH					33042	///< request agent patch.(Keeper Manager -> Keeper)
#define MC_REQUEST_SERVER_STATUS				33043	///< server와 agent의 정보를 Keeper로 요청함.(Keeper Manager -> keeper)
#define MC_RESPONSE_SERVER_STATUS				33044	///< server와 agetn의 정보 요청 응답.(Keeper -> Keeper Manager)
// Keeper manager schedule.
#define MC_REQUEST_KEEPER_MANAGER_SCHEDULE		34001	///< request keeper manager schedule.(Keeper Manager -> Keeper -> MatchServer)
#define MC_RESPONSE_KEEPER_MANAGER_SCHEDULE		34002	///< Keeper Manager의 스케쥴등록 요청 응답.
#define MC_REQUEST_START_SERVER_SCHEDULE		34003	///< 다시시작 스케줄에서 서버를 시작하는 시케줄. 시작할때까지 스케줄이 제거되지 않음.
///


/// Locator관련.
#define MC_REQUEST_SERVER_LIST_INFO				40001	/// 접속가능한 서버의 리스트 정보 요청.(Client -> Locator)
#define MC_RESPONSE_SERVER_LIST_INFO			40002	/// 접속가능한 서버 리스트 정보 응답.(Locator -> Client)
#define MC_RESPONSE_BLOCK_COUNTRY_CODE_IP		40003	/// 접속불가 국가 코드의 IP응답.(Locator -> Client)


// filter.
#define MC_RESPONSE_BLOCK_COUNTRYCODE			50001	/// 블럭된 국가코드의 IP가 접속시 통보용.
#define MC_LOCAL_UPDATE_USE_COUNTRY_FILTER		50002
#define MC_LOCAL_GET_DB_IP_TO_COUNTRY			50003
#define MC_LOCAL_GET_DB_BLOCK_COUNTRY_CODE		50004
#define MC_LOCAL_GET_DB_CUSTOM_IP				50005
#define MC_LOCAL_UPDATE_IP_TO_COUNTRY			50006
#define MC_LOCAL_UPDATE_BLOCK_COUTRYCODE		50007
#define MC_LOCAL_UPDATE_CUSTOM_IP				50008
#define MC_LOCAL_UPDATE_ACCEPT_INVALID_IP		50009

#define MC_REQUEST_XTRAP_HASHVALUE				8001
#define MC_RESPONSE_XTRAP_HASHVALUE				8002


#endif