#ifndef MMATCHSTAGE_H
#define MMATCHSTAGE_H

#include <list>
using namespace std;
#include "MMatchItem.h"
#include "MMatchTransDataType.h"
#include "MUID.h"
#include "MMatchRule.h"
#include "MMatchObject.h"
#include "MMatchWorldItem.h"
#include "MMatchStageSetting.h"
#include "MVoteMgr.h"
#include "MMatchGlobal.h"

#define MTICK_STAGE			100

class MMatchObject;
class MMatchStage;
class MMatchServer;
class MLadderGroup;

// 스테이지 타입
enum MMatchStageType
{
	MST_NORMAL	= 0,			// 일반
	MST_LADDER,					// 레더나 클랜게임
	
	MST_MAX
};

struct MMatchStageTeamBonus
{
	bool		bApplyTeamBonus;		// 해당 게임에서 경험치 보너스가 적용되는지 여부
};

// Ladder전일경우 래더팀 정보
struct MMatchLadderTeamInfo
{
	int		nTID;				// Ladder Team ID
	int		nFirstMemberCount;	// 초기멤버수

	// Clan전에서만 사용하는 변수
	int		nCLID;				// 클랜ID
	int		nCharLevel;			// 캐릭터 평균 레벨
	int		nContPoint;			// 캐릭터 기여도 평균
};


// Stage에서 사용하는 팀의 추가정보
struct MMatchStageTeam
{
	int						nTeamBonusExp;			// 누적된 팀 경험치
	int						nTeamTotalLevel;		// 팀원들의 레벨 총합 - 팀보너스 배분시 사용한다
	int						nScore;					// 스코어
	int						nSeriesOfVictories;		// 연승정보
	int						nTotalKills;			// 누적 팀 킬수 (라운드 시작, 즉 게임 시작때 초기화 - 무한팀데스매치를 위함)
	MMatchLadderTeamInfo	LadderInfo;
};


// 스테이지 메인 클래스 - 스테이지와 관련된 일을 총괄한다.
class MMatchStage {
private:
	int						m_nIndex;
	STAGE_STATE				m_nState;
	MMatchStageType			m_nStageType;
	MUID					m_uidStage;
	MUID					m_uidOwnerChannel;
	char					m_szStageName[STAGENAME_LENGTH];
	bool					m_bPrivate;		// 비밀방
	char					m_szStagePassword[STAGENAME_LENGTH];
	MMatchStageTeamBonus	m_TeamBonus;
	MMatchStageTeam			m_Teams[MMT_END];

	MUIDRefCache			m_ObjUIDCaches;
	list<int>				m_BanCIDList;

	unsigned long			m_nStateTimer;
	unsigned long			m_nLastTick;
	unsigned long			m_nChecksum;	// 목록및 정보 갱신용
	unsigned long			m_nLastChecksumTick;
	int						m_nAdminObjectCount;
	unsigned long			m_nStartTime;	// 게임 시작한 시간

	
	MMatchStageSetting		m_StageSetting;
	MMatchRule*				m_pRule;

	MUID					m_uidAgent;
	bool					m_bAgentReady;
	int						m_nRoundObjCount[MMT_END];

	MVoteMgr				m_VoteMgr;

	char					m_szFirstMasterName[MATCHOBJECT_NAME_LENGTH];

	void SetMasterUID(const MUID& uid)	{ m_StageSetting.SetMasterUID(uid);}
	MMatchRule* CreateRule(MMATCH_GAMETYPE nGameType);
protected:
	inline bool IsChecksumUpdateTime(unsigned long nTick);
	void UpdateChecksum(unsigned long nTick);
	void OnStartGame();
	void OnFinishGame();
	void OnApplyTeamBonus(MMatchTeam nTeam);
protected:
	friend MMatchServer;
	void SetStageType(MMatchStageType nStageType);
	void SetLadderTeam(MMatchLadderTeamInfo* pRedLadderTeamInfo, MMatchLadderTeamInfo* pBlueLadderTeamInfo);
public:
	MMatchWorldItemManager	m_WorldItemManager;

	void UpdateStateTimer();
	unsigned long GetStateTimer()	{ return m_nStateTimer; }
	unsigned long GetChecksum()		{ return m_nChecksum; }
	unsigned long GetStartTime()	{ return m_nStartTime; }		///< 게임 시작한 시간
public:
	MMatchStage();
	virtual ~MMatchStage();

	bool Create(const MUID& uid, const char* pszName, bool bPrivate, const char* pszPassword);
	void Destroy();
	void OnCommand(MCommand* pCommand);
	void OnGameKill(const MUID& uidAttacker, const MUID& uidVictim);
	bool CheckAutoTeamBalancing();	// true이면 팀원을 섞어야 한다.
	void ShuffleTeamMembers();


	const char* GetName()		{ return m_szStageName; }
	const char* GetPassword()	{ return m_szStagePassword; }
	void SetPassword(const char* pszPassword)	{ strcpy_safe(m_szStagePassword, pszPassword); }
	const bool IsPrivate()		{ return m_bPrivate; }
	void SetPrivate(bool bVal)	{ m_bPrivate = bVal; }
	MUID GetUID()				{ return m_uidStage; }

	const char* GetMapName()	{ return m_StageSetting.GetMapName(); }
	void SetMapName(char* pszMapName)	{ m_StageSetting.SetMapName(pszMapName); }

	char* GetFirstMasterName()	{ return m_szFirstMasterName; }
	void SetFirstMasterName(char* pszName)	{ strcpy_safe(m_szFirstMasterName, pszName); }

	MMatchObject* GetObj(const MUID& uid)	{ if (m_ObjUIDCaches.count(uid) == 0) return NULL; else return (MMatchObject*)(m_ObjUIDCaches[uid]); }			///< 추가by 동섭, 듀얼을 위해 -_-
	size_t GetObjCount()					{ return m_ObjUIDCaches.size(); }
	int GetPlayers();
	MUIDRefCache::iterator GetObjBegin()	{ return m_ObjUIDCaches.begin(); }
	MUIDRefCache::iterator GetObjEnd()		{ return m_ObjUIDCaches.end(); }
	int GetObjInBattleCount();															///< 전투하고 있는 플레이어수
	int GetCountableObjCount()				{ return ((int)GetObjCount() - m_nAdminObjectCount); }	///< 운영자를 제외한 플레이어수


	void AddBanList(int nCID);
	bool CheckBanList(int nCID);

	void AddObject(const MUID& uid, const MMatchObject* pObj);
	MUIDRefCache::iterator RemoveObject(const MUID& uid);
	bool KickBanPlayer(const char* pszName, bool bBanPlayer=true);

	const MUID RecommandMaster(bool bInBattleOnly);
	void EnterBattle(MMatchObject* pObj);
	void LeaveBattle(MMatchObject* pObj);

	STAGE_STATE GetState()		{ return m_nState; }
	void ChangeState(STAGE_STATE nState)	{ m_nState = nState; UpdateStateTimer(); }

	bool CheckTick(unsigned long nClock);
	void Tick(unsigned long nClock);

	MMatchStageSetting* GetStageSetting() { return &m_StageSetting; }

	MMatchRule* GetRule()			{ return m_pRule; }
	void ChangeRule(MMATCH_GAMETYPE nRule);
	void GetTeamMemberCount(int* poutnRedTeamMember, int* poutnBlueTeamMember, int* poutSpecMember, bool bInBattle);
	MMatchTeam GetRecommandedTeam();

	MVoteMgr* GetVoteMgr()			{ return &m_VoteMgr; }

	MUID GetAgentUID()				{ return m_uidAgent; }
	void SetAgentUID(MUID uid)		{ m_uidAgent = uid; }
	bool GetAgentReady()			{ return m_bAgentReady; }
	void SetAgentReady(bool bReady)	{ m_bAgentReady = bReady; }

	MUID GetMasterUID()				{ return m_StageSetting.GetMasterUID(); }
	int GetIndex()					{ return m_nIndex; }

	void SetOwnerChannel(MUID& uidOwnerChannel, int nIndex);
	MUID GetOwnerChannel() { return m_uidOwnerChannel; }

	void PlayerTeam(const MUID& uidPlayer, MMatchTeam nTeam);
	void PlayerState(const MUID& uidPlayer, MMatchObjectStageState nStageState);
	bool StartGame();
	bool FinishGame();
	bool CheckBattleEntry();

	void RoundStateFromClient(const MUID& uidStage, int nState, int nRound);
	void ObtainWorldItem(MMatchObject* pObj, const int nItemID);
	void RequestSpawnWorldItem(MMatchObject* pObj, const int nItemID, 
							   const float x, const float y, const float z);
	void SpawnServerSideWorldItem(MMatchObject* pObj, const int nItemID, 
							   const float x, const float y, const float z, 
							   int nLifeTime, int* pnExtraValues );

	bool IsApplyTeamBonus();	// 팀전 보너스 적용여부 확인
	void AddTeamBonus(int nExp, MMatchTeam nTeam);
	int GetTeamScore(MMatchTeam nTeam)		{ return m_Teams[nTeam].nScore; }

	int GetTeamKills(MMatchTeam nTeam)		{ return m_Teams[nTeam].nTotalKills; }
	void AddTeamKills(MMatchTeam nTeam, int amount=1)		{ m_Teams[nTeam].nTotalKills+=amount; }
	void InitTeamKills()		{ m_Teams[MMT_BLUE].nTotalKills = m_Teams[MMT_RED].nTotalKills = 0; }

	const MMatchStageType GetStageType()	{ return m_nStageType; }
	int GetMinPlayerLevel();	// 방에 있는 플레이어중 최소 레벨을 구한다.

	bool CheckUserWasVoted( const MUID& uidPlayer );
public:
	// Rule에서 호출하는 함수들
	void OnRoundEnd_FromTeamGame(MMatchTeam nWinnerTeam);
	void OnInitRound();			// 라운드 시작시 Rule클래스에서 호출
};


class MMatchStageMap : public map<MUID, MMatchStage*> {
	MUID	m_uidGenerate;
public:
	MMatchStageMap()			{	m_uidGenerate = MUID(0,0);	}
	virtual ~MMatchStageMap()	{	}
	MUID UseUID()				{	m_uidGenerate.Increase();	return m_uidGenerate;	}
	void Insert(const MUID& uid, MMatchStage* pStage)	{	insert(value_type(uid, pStage));	}
};

MMatchItemBonusType GetStageBonusType(MMatchStageSetting* pStageSetting);


#define TRANS_STAGELIST_NODE_COUNT	8	// 한번에 클라이언트에게 보내주는 스테이지노드 개수



#endif
