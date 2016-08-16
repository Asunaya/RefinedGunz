#ifndef _MQUEST_NPC_H
#define _MQUEST_NPC_H

#include "MQuestConst.h"
#include "MTypes.h"

/// NPC 종류
enum MQUEST_NPC
{
	// 상수값을 바꾸면 절대 안된다. xml도 함께 바꿔야함.

	NPC_NONE					= 0,	///< N/A
	NPC_GOBLIN					= 11,	///< 병정 고블린
	NPC_GOBLIN_GUNNER			= 12,	///< 고블린 거너
	NPC_GOBLIN_WIZARD			= 13,	///< 고블린 위저드
	NPC_GOBLIN_COMMANDER		= 14,	///< 대장 고블린
	NPC_GOBLIN_CHIEF			= 15,	///< 고블린 족장
	NPC_GOBLIN_KING				= 16,	///< 고블린 킹

	NPC_PALMPOA					= 21,	///< 팜포아
	NPC_PALMPOA_COMMANDER		= 22,	///< 대장 팜포아
	NPC_PALMPOW					= 23,	///< 팜포우
	NPC_CURSED_PALMPOW			= 24,	///< 저주받은 팜포우
	NPC_PALMPOW_BABY			= 25,	///< 팜포우 베이비

	NPC_SKELETON				= 31,	///< 스켈레톤
	NPC_SKELETON_MAGE			= 32,	///< 스켈레톤 메이지
	NPC_SKELETON_COMMANDER		= 33,	///< 대장 스켈레톤
	NPC_GIANT_SKELETON			= 34,	///< 거대 스켈레톤
	NPC_THE_UNHOLY				= 35,	///< 저주받은 시신
	NPC_LICH					= 36,	///< 리치

	NPC_KOBOLD					= 41,	///< 코볼트
	NPC_KOBOLD_SHAMAN			= 42,	///< 코볼트 샤만
	NPC_KOBOLD_COMMANDER		= 43,	///< 대장 코볼트
	NPC_KOBOLD_KING				= 44,	///< 코볼트 왕
	NPC_BROKEN_GOLEM			= 45,	///< 고장난 골렘
	NPC_SCRIDER					= 46,	///< 스크라이더
};

/// NPC 등급
enum MQUEST_NPC_GRADE
{
	NPC_GRADE_REGULAR = 0,			///< 일반
	NPC_GRADE_VETERAN,				///< 베테랑
	NPC_GRADE_ELITE,				///< 엘리트
	NPC_GRADE_BOSS,					///< 보스급
	NPC_GRADE_LEGENDARY,			///< 특수 보스급
	NPC_GRADE_END
};

/// NPC 공격 타입
enum MQUEST_NPC_ATTACK
{
	NPC_ATTACK_NONE			= 0,		///< N/A
	NPC_ATTACK_MELEE		= 0x1,		///< 근접 공격
	NPC_ATTACK_RANGE		= 0x2,		///< 원근 공격
	NPC_ATTACK_MAGIC		= 0x4,		///< 마법 공격
};

/// NPC 행동 타입
enum MQUEST_NPC_BEHAVIOR
{

};


/// NPC 사운드
enum MQUEST_NPC_SOUND
{
	NPC_SOUND_ATTACK = 0,		// 공격할때
	NPC_SOUND_WOUND,			// 맞았을때
	NPC_SOUND_DEATH,			// 죽었을때
	NPC_SOUND_END
};

/// 퀘스트 NPC 정보
struct MQuestNPCInfo
{
	// 기본 정보
	MQUEST_NPC			nID;				///< id
	MQUEST_NPC_GRADE	nGrade;				///< 등급
	char				szName[256];		///< 이름
	char				szDesc[256];		///< 설명
	unsigned long int	nNPCAttackTypes;	///< 공격타입	- MQUEST_NPC_ATTACK형의 SET
	float				fSpeed;				///< 속도
	float				fWeight;			///< 몸무게
	int					nMaxHP;				///< 최대 HP
	int					nMaxAP;				///< 최대 AP
	int					nDC;				///< 난이도 상수
	char				szMeshName[256];	///< 메쉬이름
	int					nWeaponDamage;		///< 갖고 있는 무기 공격력
	float				fRotateSpeed;		///< 방향 전환 속도 (초당 회전 각도임, 예를 들어 3.14159이면 초당 반바퀴 회전함. -값이면 바로 회전)
	float				fCollRadius;		///< 충돌 반지름
	float				fCollHeight;		///< 충돌 높이
	bool				bColPick;			///< 원거리 피격시 반지름말고 피킹여부로 피격체크할지 여부(주로 보스만 true)
	MVector				vScale;				///< 크기
	MVector				vColor;				///< 색
	unsigned char		nSpawnWeight;		///< 등장 가중치
	unsigned long int	nWeaponItemID;		///< 장비하고 있는 무기 ID(Zitem에 있는거)
	float				fDyingTime;			///< 죽는 애니메이션 시간(기본은 5초)
	float				fTremble;			///< 피격시 몸 떠는 정도(기본은 30)
	// int					nDBIndex;			///< DB와 NPC와 Matching되는 인덱스.

	// Sound
	char				szSoundName[NPC_SOUND_END][128];	///< 사운드 파일 이름

	// AI 관련 파라메타
	char				nIntelligence;			///< 지능(1 ~ 5단계) 1이 가장 좋다. - 길찾기 업데이트 속도와 관련있다.
	char				nAgility;				///< 민첩성(1 ~ 5단계) 1이 가장 좋다. - 공격 업데이트 속도와 관련있다.

	float				fAttackRange;			///< 기본 공격범위
	float				fAttackRangeAngle;		///< 기본 공격 시야 각도
	float				fAttackHitRate;			///< 기본 공격 명중률(특히 원거리)
	float				fAttackCoolTime;		///< 기본 공격 쿨타임(특히 원거리)
	float				fCollisionCheckSpeed;	///< 충돌체크 속도
	float				fViewAngle;				///< 시야 각도(라디안)
	float				fViewDistance;			///< 시야 거리
	float				fViewDistance360;		///< 전방향 시야거리
	
	// 플래그
	bool				bNeverBlasted;				///< 칼로 띄워올리기 등 적용 여부
	bool				bMeleeWeaponNeverDamaged;	///< 근접 공격에 맞는지 여부
	bool				bRangeWeaponNeverDamaged;	///< 원근 공격에 맞는지 여부
	bool				bShadow;					///< 그림자가 있는지 여부
	bool				bNeverPushed;				///< 공격 받았을때 밀릴것인가?
	bool				bNeverAttackCancel;			///< 공격 받았을때 하던 공격이 취소되는가?

	int					nSkills;				///< 가진 스킬 개수
	int					nSkillIDs[MAX_SKILL];	///< 가지고있는 스킬

	// 서버만 사용하는 파라메타
	int					nDropTableID;			///< drop table index - 서버만 사용한다.
	char				szDropTableName[8];		///< drop table name

	/////////////////////////////////////////////

	/// 초기화
	void SetDefault()
	{
		nID					= NPC_GOBLIN;
		nGrade				= NPC_GRADE_REGULAR;
		strcpy_safe(szName, "Noname");
		szDesc[0]			= 0;
		nNPCAttackTypes		= NPC_ATTACK_MELEE;
		fSpeed				= 300.0f;
		fWeight				= 1.0f;
		nMaxHP				= 100;
		nMaxAP				= 0;
		nDC					= 5;
		szMeshName[0]		= 0;
		nWeaponDamage		= 5;
		fRotateSpeed		= 6.28318f;
		fCollRadius			= 35.0f;
		fCollHeight			= 180.0f;
		bColPick			= false;
		vScale				= MVector(1.0f,1.0f,1.0f);
		vColor				= MVector(0.6f,0.6f,0.6f);
		nSpawnWeight		= 100;
		nWeaponItemID		= 300000;		// 고블린 단검
		fTremble			= 30.0f;		// 기본은 캐릭터값이랑 동일
		
		nIntelligence		= 3;
		nAgility			= 3;
		fDyingTime			= 5.0f;

		fAttackRange		= 130.0f;
		fAttackRangeAngle	= 1.570796f;		// 90도
		fAttackHitRate		= 1.0f;			// 기본은 100% 명중 (원근 공격 NPC만 이 수치를 사용)
		fAttackCoolTime		= 0.0f;
		fCollisionCheckSpeed = 0.0f;


		fViewAngle			= 3.14159f;		// 180도
		fViewDistance		= 800.0f;
		fViewDistance360	= 800.0f;

		bNeverBlasted				= false;
		bMeleeWeaponNeverDamaged	= false;
		bRangeWeaponNeverDamaged	= false;
		bShadow						= true;
		bNeverPushed				= false;
		bNeverAttackCancel			= false;

		nSkills				= 0;

		nDropTableID		= 0;
		szDropTableName[0]	= 0;

		for (int i = 0; i < NPC_SOUND_END; i++) szSoundName[i][0] = 0;
	}

	MQuestNPCSpawnType GetSpawnType();		///< 스폰 타입 반환
};


#define NPC_INTELLIGENCE_STEPS		5
#define NPC_AGILITY_STEPS			5

struct MQuestNPCGlobalAIValue
{
	// shaking ratio
	float		m_fPathFinding_ShakingRatio;
	float		m_fAttack_ShakingRatio;
	float		m_fSpeed_ShakingRatio;

	// update time
	float		m_fPathFindingUpdateTime[NPC_INTELLIGENCE_STEPS];
	float		m_fAttackUpdateTime[NPC_AGILITY_STEPS];
};

/// NPC 정보 관리자 클래스
class MQuestNPCCatalogue : public std::map<MQUEST_NPC, MQuestNPCInfo*>
{
private:
	MQuestNPCGlobalAIValue			m_GlobalAIValue;

	// 함수
	void ParseNPC(MXmlElement& element);
	void Insert(MQuestNPCInfo* pNPCInfo);
	void ParseGlobalAIValue(MXmlElement& element);

	// 몬스터 도감.
	std::map< int, MQUEST_NPC > m_MonsterBibleCatalogue;

public :
	MQuestNPCInfo* GetIndexInfo( int nIndex );

public:
	MQuestNPCCatalogue();													///< 생성자
	~MQuestNPCCatalogue();													///< 소멸자

	bool ReadXml(const char* szFileName);									///< xml로부터 npc정보를 읽는다.
	bool ReadXml(class MZFileSystem* pFileSystem,const char* szFileName);			///< xml로부터 npc정보를 읽는다.
	void Clear();															///< 초기화

	MQuestNPCInfo* GetInfo(MQUEST_NPC nNpc);								///< NPC 정보 반환
	MQuestNPCInfo* GetPageInfo( int nPage);									///< 페이지로부터 NPC 정보 반환

	MQuestNPCGlobalAIValue* GetGlobalAIValue() { return &m_GlobalAIValue; }
};

/////////////////////////////////////////////////////////////////////////////////////////////

/// NPC Set의 NPC정보
struct MNPCSetNPC
{
	MQUEST_NPC		nNPC;					///< NPC 정보
	int				nMinRate;				///< 등장할 최소 비율(퍼센트)
	int				nMaxRate;				///< 등장할 최대 비율
	int				nMaxSpawnCount;			///< 등장할 수 있는 최대 개수

	/// 생성자
	MNPCSetNPC()
	{
		nNPC = NPC_NONE;
		nMinRate = 0;
		nMaxRate = 0;
		nMaxSpawnCount = 0;
	}
};

/// NPC Set
struct MQuestNPCSetInfo
{
	int					nID;				///< ID
	char				szName[16];			///< 이름
	MQUEST_NPC			nBaseNPC;			///< 기본 베이스 NPC
	vector<MNPCSetNPC>	vecNPCs;			///< 등장할 NPC
};

/// NPC Set 정보 관리자 클래스
class MQuestNPCSetCatalogue : public std::map<int, MQuestNPCSetInfo*>
{
private:
	std::map<string, MQuestNPCSetInfo*>		m_NameMap;
	// 함수
	void Clear();
	void ParseNPCSet(MXmlElement& element);
	void Insert(MQuestNPCSetInfo* pNPCSetInfo);
public:
	MQuestNPCSetCatalogue();											///< 생성자
	~MQuestNPCSetCatalogue();											///< 소멸자

	bool ReadXml(const char* szFileName);								///< xml로부터 npc정보를 읽는다.
	bool ReadXml(MZFileSystem* pFileSystem,const char* szFileName);		///< xml로부터 npc정보를 읽는다.

	MQuestNPCSetInfo* GetInfo(int nID);									///< NPC Set 정보 반환
	MQuestNPCSetInfo* GetInfo(const char* szName);						///< NPC Set 정보 반환
};


#endif