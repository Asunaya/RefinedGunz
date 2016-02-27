#ifndef _MQUESTCONST_H
#define _MQUESTCONST_H


/// MAX QL
#define MAX_QL			5			

/// 최대 맵셋 갯수	
#define MAX_MAPSET		6


/// scenario 관련 /////////////////////////////////////////////////////////

/// 시나리오에서 필요로 하는 희생 아이템 최고 개수
#define MAX_SCENARIO_SACRI_ITEM			2

/// 시나리오에서 주는 특별 아이템 보상 최고 개수
#define MAX_SCENARIO_REWARD_ITEM		3		

/// 시나리오를 결정하는 주사위 면체수
#define SCENARIO_STANDARD_DICE_SIDES	6		


/// Map 관련 //////////////////////////////////////////////////////////////

/// 섹터의 링크는 최대 10개까지 만들 수 있음
#define MAX_SECTOR_LINK		10		

/// 스폰개수는 스폰타입별 20개까지 만들 수 있다.
#define MAX_SPAWN_COUNT		20


/// NPC의 스폰 타입
enum MQuestNPCSpawnType
{
	MNST_MELEE = 0,
	MNST_RANGE,
	MNST_BOSS,
	MNST_END
};

/// 똑같은 스폰지점에서 NPC를 한번 스폰시킨 후 최소한 2초가 지나야 스폰시킬 수 있다.
#define SAME_SPAWN_DELAY_TIME			2000

/// 퀘스트에서 떨궈지는 월드아이템은 10초가 지나면 자동적으로 소멸한다.
#define QUEST_DYNAMIC_WORLDITEM_LIFETIME	10000

/// 아이템박스 월드아이템 ID는 51이다. - 수정하려면 worlditem.xml의 ID도 함께 수정해야 한다.
#define QUEST_WORLDITEM_ITEMBOX_ID			51

/// NPC 관련 //////////////////////////////////////////////////////////////

/// NPC가 가질 수 있는 최대 스킬수
#define MAX_SKILL	4



/// Rule 관련 /////////////////////////////////////////////////////////////

/// 포탈이 열리고서 다음 포탈로 이동할 수 있는 시간
#define PORTAL_MOVING_TIME				30000		// (30 초)

/// 새로운 섹터 시작하고나서 NPC가 처음 나올때까지의 지연 시간
#define QUEST_COMBAT_PLAY_START_DELAY	7000		// (7 초)

/// 퀘스트 Complete하고 나서 아이템 먹을 수 있는 지연시간
#define QUEST_COMPLETE_DELAY	7000				// (7 초)

/// 퀘스트의 게임 상태
enum MQuestCombatState
{
	MQUEST_COMBAT_NONE		= 0,
	MQUEST_COMBAT_PREPARE	= 1,	// 대기중
	MQUEST_COMBAT_PLAY		= 2,	// 전투 시작
	MQUEST_COMBAT_COMPLETED	= 3		// 전투종료 - 다음맵으로 이동
};



enum MQuestItemType
{
	MMQIT_PAGE = 1,
	MMQIT_SKULL,
	MMQIT_FRESH,
	MMQIT_RING,
	MMQIT_NECKLACE,
	MMQIT_DOLL,
	MMQIT_BOOK,
	MMQIT_OBJECT,
	MMQIT_SWORD,
	MMQIT_GUN,
	MMQIT_LETTER,
	MMQIT_ITEMBOX,
	MMQIT_MONBIBLE,
	

	MMQIT_END,
};


#define MIN_QUEST_ITEM_ID		200001	// 퀘스트 아이템 시작 값.

//#define MAX_UNIQUE_ITEM_COUNT	3
//#define START_QUEST_ITEM_COUNT	2
#define MIN_QUEST_DB_ITEM_COUNT	1
#define MAX_QUEST_DB_ITEM_COUNT	100 // 하나의 종류에대해 유저가 가질수 있는 최대 아이템 수. (최대 가질수 있는 개수 + MIN_QUEST_ITEM_COUNT)
									// 카운트: 0은 한번도 획득한 적이 없으면서 현재 수량이 0.
									//		 : 1은 적어도 한번은 획득한 적이 있으나 현재 수량이 0임.
									//		 : 2은 아이템 카운트가 시작되는 시점. 2가 실질적으로는 1개를 나타냄.

// 하나의 종류에 대해 캐릭터는 0~99개까지 퀘스트 아이템을 가질 수 있다.
#define MIN_QUEST_ITEM_COUNT	0	
#define MAX_QUEST_ITEM_COUNT	99

//#define REAL_MAX_QUEST_ITEM_COUNT (MAX_QUEST_ITEM_COUNT - MIN_QUEST_ITEM_COUNT)

// zquestitem.xml에 기술되는 아이템 ID의 범위.
#define MINID_QITEM_LIMIT		200001	// Qeust item.
#define MAXID_QITEM_LIMIT		200256
#define MINID_MONBIBLE_LIMIT	210001	// Monster bible.
#define MAXID_MONBIBLE_LIMIT	220000

#define	MAX_PLAY_COUNT				10					// 최대 DB업데이트 없이 할수있는 퀘스트 완료수.
#define	MAX_SHOP_TRADE_COUNT		20					// 최대 DB업데이트 없이 아이템 거래를 할수 있는수.
#define	MAX_ELAPSE_TIME				(1000 * 60 * 30)	// 최대 DB업데이트를 지연시킬수 있는 시간(/ms).
#define MAX_REWARD_COUNT			10					// 최대 DB업데이트 없이 보상받을수 있는 수. (아이템의 종류를 기준. ex. 날고기:2, 불고기:3 일경우 +2가 된다.)
#define MAX_QUSET_PLAYER_COUNT		4

#define MAX_SACRIFICE_SLOT_COUNT 2	// 희생 아이템을 올려놓을수 있는 슬롯의 최대 수.

// Quest와 관련된 DB 상수 값.
#define MAX_DB_QUEST_ITEM_SIZE		256
#define MAX_DB_MONSTERBIBLE_SIZE	32
#define MAX_DB_QUEST_BINARY_SIZE	292	// 총 바이너리 필드 사이즈는 292( CRC32 4Byte + MAX_DB_QUEST_ITEM_SIZE + MAX_DB_MONSTERBIBLE_SIZE ).
#define QUEST_DATA					293	// MAX_DB_QUEST_BINARY_SIZE + 1(\0)

// 아이템 희생관련 커맨드 응답 결과 상수 값.
#define ITEM_TYPE_NOT_SACRIFICE			-2		// 아이템의 타입이 Sacrifice가 아니라 슬롯에 올려 놓을수 없음.
#define NEED_MORE_QUEST_ITEM			-4		// 아이템의 수량이 부족함.
#define SACRIFICE_ITEM_DUPLICATED		-5		// 아이템 중복. 이전에 올려져있던 아이템과 같은 아이템을 슬롯에 올리려 하였을때.
#define ERR_SACRIFICE_ITEM_INFO			-255	// 요청한 희생아이템 정보가 정상적이지 않음.


// 슬롯에 올려놓은 아이템 검사 결과 상수 값.
#define MSQITRES_NOR  1	// 일반시나리오에 대한 희생 아이템만 있는 상황.
#define MSQITRES_SPC  2	// 특별시나리오에 해당하는 희생아이템이 있음.
#define MSQITRES_INV  3	// 해당 QL에대한 희생아이템 정보 테이블이 없음. 이경우는 맞지 않는 희생 아이템이 올려져 있을경우.
#define MSQITRES_DUP  4 // 양쪽 슬롯에 같은 아이템이 올려져 있음.
#define MSQITRES_EMP  5 // 양쪽 슬롯이 모두 비어 있음. 이 상태는 QL값을1로 해줘야 함.
#define MSQITRES_ERR -1	// 에러... 테이블에서 해당 QL을 찾을수 없음. QL = 0 or QL값이 현재 구성된 MAX QL보다 클경우.


// 몬스터 도감 관련. 
#define MSMB_UNKNOW		0	// 해당 몬스터에 대한 정보는 아는게 없음.
#define MSMB_KNOW		1	// 해당 몬스터에 대한 정보는 이미 알고 있음.
#define MSMB_NEWINFO	2	// 새롭게 획득. 이 정보는 도감에 등록을 시키고 MSMB_KNOW로 바꿔줘야 함.



#endif