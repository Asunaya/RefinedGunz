#ifndef _MBASEGAMETYPE_H
#define _MBASEGAMETYPE_H

#include <crtdbg.h>
#include <map>
#include <set>
using namespace std;


/// 게임 타입
enum MMATCH_GAMETYPE {
	MMATCH_GAMETYPE_DEATHMATCH_SOLO		=0,			///< 개인 데쓰매치
	MMATCH_GAMETYPE_DEATHMATCH_TEAM		=1,			///< 팀 데쓰매치
	MMATCH_GAMETYPE_GLADIATOR_SOLO		=2,			///< 개인 글래디에이터
	MMATCH_GAMETYPE_GLADIATOR_TEAM		=3,			///< 팀 글래디에이터
	MMATCH_GAMETYPE_ASSASSINATE			=4,			///< 보스전
	MMATCH_GAMETYPE_TRAINING			=5,			///< 연습

#ifdef _QUEST
	MMATCH_GAMETYPE_SURVIVAL			=6,			///< 서바이벌
	MMATCH_GAMETYPE_QUEST				=7,			///< 퀘스트
#endif

	MMATCH_GAMETYPE_BERSERKER			=8,
	MMATCH_GAMETYPE_DEATHMATCH_TEAM2	=9,
	MMATCH_GAMETYPE_DUEL				=10,
/*
#ifndef _CLASSIC
	MMATCH_GAMETYPE_CLASSIC_SOLO,
	MMATCH_GAMETYPE_CLASSIC_TEAM,
#endif
*/
	MMATCH_GAMETYPE_MAX,

	MMATCH_GAMETYPE_ALL = 100,		// event에만 사용된다.
};


// 디폴트 게임 타입
const MMATCH_GAMETYPE MMATCH_GAMETYPE_DEFAULT = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
// const MMATCH_GAMETYPE MMATCH_GAMETYPE_DEFAULT = MMATCH_GAMETYPE_DEATHMATCH_TEAM2;


struct MMatchGameTypeInfo
{
	MMATCH_GAMETYPE		nGameTypeID;			// ID
	char				szGameTypeStr[24];		// 게임 타입 이름
	float				fGameExpRatio;			// 경험치 배분 비율 - 현재 Gladiator만 50%이다
	float				fTeamMyExpRatio;		// 팀전에서 개인에게 적용되는 경험치
	float				fTeamBonusExpRatio;		// 팀전에서 팀에게 적립되는 경험치
	set<int>			MapSet;					// 이 게임타입에서 플레이 가능한 맵
	void Set(const MMATCH_GAMETYPE a_nGameTypeID, const char* a_szGameTypeStr, const float a_fGameExpRatio,
		     const float a_fTeamMyExpRatio, const float a_fTeamBonusExpRatio);
	void AddMap(int nMapID);
	void AddAllMap();
};


class MBaseGameTypeCatalogue
{
private:
	MMatchGameTypeInfo			m_GameTypeInfo[MMATCH_GAMETYPE_MAX];
public:
	MBaseGameTypeCatalogue();
	virtual ~MBaseGameTypeCatalogue();

	inline MMatchGameTypeInfo* GetInfo(MMATCH_GAMETYPE nGameType);
	inline const char* GetGameTypeStr(MMATCH_GAMETYPE nGameType);
	inline void SetGameTypeStr(MMATCH_GAMETYPE nGameType, const char* szName);
	inline bool IsCorrectGameType(const int nGameTypeID);	///< 올바른 게임 타입 ID인지 여부
	inline bool IsTeamGame(MMATCH_GAMETYPE nGameType);		///< 해당 게임타입이 팀전인지 여부
	inline bool IsTeamLimitTime(MMATCH_GAMETYPE nGameType);
	inline bool IsWaitForRoundEnd(MMATCH_GAMETYPE nGameType);		///< 라운드 끝날때까지 대기모드 해야하는지 여부
	inline bool IsQuestOnly(MMATCH_GAMETYPE nGameType);	///< 해당 게임타입이 퀘스트인지 여부
	inline bool IsQuestDerived(MMATCH_GAMETYPE nGameType);	///< 해당 게임타입이 퀘스트 관련(NPC가 나오는지)인지 여부
	inline bool IsWorldItemSpawnEnable(MMATCH_GAMETYPE nGameType);	///< 해당 게임타입에서 월드아이템이 스폰되는지 여부
};


//////////////////////////////////////////////////////////////////////////////////
inline bool MBaseGameTypeCatalogue::IsTeamGame(MMATCH_GAMETYPE nGameType)
{
	// 좀더 복잡해지면 Description에 teamgame인지 여부값을 넣도록 하자.

	if ((nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM2) ||
		(nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_ASSASSINATE) )
	{
		return true;
	}
	return false;
}

inline bool MBaseGameTypeCatalogue::IsTeamLimitTime(MMATCH_GAMETYPE nGameType)
{
	// 좀더 복잡해지면 Description에 teamgame인지 여부값을 넣도록 하자.

	if ((nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_DUEL) ||
		(nGameType == MMATCH_GAMETYPE_ASSASSINATE) )
	{
		return true;
	}
	return false;
}

inline bool MBaseGameTypeCatalogue::IsWaitForRoundEnd(MMATCH_GAMETYPE nGameType)
{
	// 라운드 끝날때까지 대기모드 하는가?

	if ((nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_DUEL) ||
		(nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_ASSASSINATE) )
	{
		return true;
	}
	return false;
}


inline bool MBaseGameTypeCatalogue::IsQuestDerived(MMATCH_GAMETYPE nGameType)
{
#ifdef _QUEST
	if ( (nGameType == MMATCH_GAMETYPE_SURVIVAL) ||(nGameType == MMATCH_GAMETYPE_QUEST) )
	{
		return true;
	}
#endif
	return false;
}

inline bool MBaseGameTypeCatalogue::IsQuestOnly(MMATCH_GAMETYPE nGameType)
{
#ifdef _QUEST
	if ( nGameType == MMATCH_GAMETYPE_QUEST)
	{
		return true;
	}
#endif
	return false;
}

inline const char* MBaseGameTypeCatalogue::GetGameTypeStr(MMATCH_GAMETYPE nGameType)
{
	return m_GameTypeInfo[nGameType].szGameTypeStr;
}

inline void MBaseGameTypeCatalogue::SetGameTypeStr(MMATCH_GAMETYPE nGameType, const char* szName)
{
	strcpy( m_GameTypeInfo[nGameType].szGameTypeStr, szName) ;
}

bool MBaseGameTypeCatalogue::IsCorrectGameType(const int nGameTypeID)
{
	if ((nGameTypeID < 0) || (nGameTypeID >= MMATCH_GAMETYPE_MAX)) return false;
	return true;
}

inline MMatchGameTypeInfo* MBaseGameTypeCatalogue::GetInfo(MMATCH_GAMETYPE nGameType)
{
	_ASSERT((nGameType >= 0) && (nGameType < MMATCH_GAMETYPE_MAX));
	return &m_GameTypeInfo[nGameType];
}

inline bool MBaseGameTypeCatalogue::IsWorldItemSpawnEnable(MMATCH_GAMETYPE nGameType)
{
#ifdef _QUEST
	if ( (nGameType == MMATCH_GAMETYPE_SURVIVAL) ||(nGameType == MMATCH_GAMETYPE_QUEST) )
	{
		return false;
	}
#endif
	return true;

}

#endif