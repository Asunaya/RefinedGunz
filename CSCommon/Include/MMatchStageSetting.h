#ifndef _MMATCHSTAGESETTING_H
#define _MMATCHSTAGESETTING_H

#include <list>
using namespace std;
#include "MUID.h"
#include "MMatchGlobal.h"
#include "MMatchGameType.h"
#include "MMatchObject.h"
#include "MMatchMap.h"

class MMatchObject;
class MMatchStage;

#define MMATCH_TEAM_MAX_COUNT		2

#define MMATCH_SPECTATOR_STR		"SPECTATOR"
#define MMATCH_TEAM1_NAME_STR		"RED TEAM"
#define MMATCH_TEAM2_NAME_STR		"BLUE TEAM"


inline const char* GetTeamNameStr(MMatchTeam nTeam)
{
	switch (nTeam)
	{
	case MMT_SPECTATOR:
		return MMATCH_SPECTATOR_STR;
	case MMT_RED:
		return MMATCH_TEAM1_NAME_STR;
	case MMT_BLUE:
		return MMATCH_TEAM2_NAME_STR;
	default:
		return "";
	}
	return "";
}


enum STAGE_STATE {
	STAGE_STATE_STANDBY		= 0,
	STAGE_STATE_COUNTDOWN,
	STAGE_STATE_RUN,
	STAGE_STATE_CLOSE
};

#define MSTAGENODE_FLAG_FORCEDENTRY_ENABLED		1		// 난입
#define MSTAGENODE_FLAG_PRIVATE					2		// 비밀방
#define MSTAGENODE_FLAG_LIMITLEVEL				4		// 레벨제한

enum class NetcodeType : u8
{
	ServerBased,
	P2PAntilead,
	P2PLead,
};


#pragma pack(push, 1)
struct MSTAGE_SETTING_NODE {
	MUID				uidStage;
	char				szStageName[64];
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

	// 추가됨
	bool				bAutoTeamBalancing;			// 오토팀밸런스 - 팀플게임에서만 사용
#ifdef _VOTESETTING
	bool				bVoteEnabled;				// 투표가능 여부
	bool				bObserverEnabled;			// 관전모드 여부
#endif

	NetcodeType Netcode = NetcodeType::ServerBased;

	bool ForceHPAP = true;

	int HP = 100;
	int AP = 50;
};
#pragma pack(pop)

// 방 처음 만들었을때 스테이지 세팅 초기값
#define MMATCH_DEFAULT_STAGESETTING_MAPNAME			"Mansion"

#define MMATCH_DEFAULT_STAGESETTING_GAMETYPE			MMATCH_GAMETYPE_DEATHMATCH_SOLO
#define MMATCH_DEFAULT_STAGESETTING_ROUNDMAX			50		// 50라운드
#define MMATCH_DEFAULT_STAGESETTING_LIMITTIME			30		// 30분
#define MMATCH_DEFAULT_STAGESETTING_LIMITLEVEL			0		// 무제한
#define MMATCH_DEFAULT_STAGESETTING_MAXPLAYERS			8		// 8명
#define MMATCH_DEFAULT_STAGESETTING_TEAMKILL			false	// 팀킬불가
#define MMATCH_DEFAULT_STAGESETTING_TEAM_WINTHEPOINT	false	// 선승제 여부
#define MMATCH_DEFAULT_STAGESETTING_FORCEDENTRY			true	// 난입가능
#define MMATCH_DEFAULT_STAGESETTING_AUTOTEAMBALANCING	true	// 오토팀밸런스


#define STAGESETTING_LIMITTIME_UNLIMITED				0		// 제한시간이 무제한은 0


struct MSTAGE_CHAR_SETTING_NODE {
	MUID	uidChar;
	int		nTeam;
	MMatchObjectStageState	nState;
	MSTAGE_CHAR_SETTING_NODE() : uidChar(MUID(0,0)), nTeam(0), nState(MOSS_NONREADY) {	}
};
class MStageCharSettingList : public list<MSTAGE_CHAR_SETTING_NODE*> {
public:
	void DeleteAll() {
		for (iterator i=begin(); i!=end(); i++) {
			delete (*i);
		}
		clear();
	}
};




class MMatchStageSetting {
protected:
	MSTAGE_SETTING_NODE		m_StageSetting;
	MUID					m_uidMaster;	// 방장
	STAGE_STATE				m_nStageState;	// 현재 State (게임중,대기중,..)
public:
	MStageCharSettingList	m_CharSettingList;
public:
	MMatchStageSetting();
	virtual ~MMatchStageSetting();
	void Clear();
	void SetDefault();
	unsigned long GetChecksum();
	MSTAGE_CHAR_SETTING_NODE* FindCharSetting(const MUID& uid);

	// Get
	char* GetMapName()								{ return m_StageSetting.szMapName; }
	int GetMapIndex()								{ return m_StageSetting.nMapIndex; }
	int GetRoundMax()								{ return m_StageSetting.nRoundMax; }
	int GetLimitTime()								{ return m_StageSetting.nLimitTime; }
	int GetLimitLevel()								{ return m_StageSetting.nLimitLevel; }
	MUID GetMasterUID()								{ return m_uidMaster; }
	STAGE_STATE GetStageState()						{ return m_nStageState; }
	MMATCH_GAMETYPE GetGameType()					{ return m_StageSetting.nGameType; }
	int GetMaxPlayers()								{ return m_StageSetting.nMaxPlayers; }
	bool GetForcedEntry()							{ return m_StageSetting.bForcedEntryEnabled; }
	bool GetAutoTeamBalancing()						{ return m_StageSetting.bAutoTeamBalancing; }
	auto GetNetcode() const { return m_StageSetting.Netcode; }
	bool IsForcedHPAP() const { return m_StageSetting.ForceHPAP; }
	int GetForcedHP() const { return m_StageSetting.HP; }
	int GetForcedAP() const { return m_StageSetting.AP; }
	MSTAGE_SETTING_NODE* GetStageSetting()			{ return &m_StageSetting; }
	const MMatchGameTypeInfo* GetCurrGameTypeInfo();

	// Set
	void SetMasterUID(const MUID& uid)		{ m_uidMaster = uid; }
	void SetMapName(char* pszName);
	void SetMapIndex(int nMapIndex);
	void SetRoundMax(int nRound)			{ m_StageSetting.nRoundMax = nRound; }
	void SetLimitTime(int nTime)			{ m_StageSetting.nLimitTime = nTime; }
	void SetGameType(MMATCH_GAMETYPE type)	{ m_StageSetting.nGameType=type; }
	void SetStageState(STAGE_STATE nState)	{ m_nStageState = nState; }
	void SetTeamWinThePoint(bool bValue)	{ m_StageSetting.bTeamWinThePoint = bValue; }
	void SetAutoTeamBalancing(bool bValue)	{ m_StageSetting.bAutoTeamBalancing = bValue; }
	
	void UpdateStageSetting(MSTAGE_SETTING_NODE* pSetting);
	void UpdateCharSetting(const MUID& uid, unsigned int nTeam, MMatchObjectStageState nStageState);

	void ResetCharSetting()			{ m_CharSettingList.DeleteAll(); }
	bool IsTeamPlay();
	bool IsWaitforRoundEnd();
	bool IsQuestDrived();
	bool IsTeamWinThePoint()		{ return m_StageSetting.bTeamWinThePoint; }		///< 선승제 여부
};












#endif