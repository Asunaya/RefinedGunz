#ifndef _ZMYINFO_H
#define _ZMYINFO_H

#include "ZPrerequisites.h"
#include "ZMyItemList.h"
#include "HShield/HShield.h"

/// 시스템에서 필요한 내정보
struct ZMySystemInfo
{
	bool			bInGameNoChat;				// 게임중 대화 가능 여부
	char			szSerialKey[128];			// XTrap에서 쓰는 RandomValue
	unsigned char	pbyAckMsg[SIZEOF_ACKMSG];			// HShield CRC 응답 메세지
	unsigned char	pbyGuidAckMsg[SIZEOF_GUIDACKMSG];	// HShield GUID 응답 메세지

	ZMySystemInfo()
	{
		bInGameNoChat = false;
		memset(szSerialKey, 0, sizeof(szSerialKey));
		memset(pbyAckMsg, 0, sizeof(pbyAckMsg));
		memset(pbyGuidAckMsg, 0, sizeof(pbyGuidAckMsg));
	}
};

/// 게임중에 필요한 내정보
struct ZMyGameInfo
{
	bool		bForcedChangeTeam;		// 서버에서 강제로 팀이 변경되었는지 여부

	ZMyGameInfo() { Init(); }
	void Init()
	{
		bForcedChangeTeam = false;
	}
	void InitRound()		// 라운드 시작할때 필요한 초기화
	{
		bForcedChangeTeam = false;
	}
};

class ZMyInfo
{
private:
	bool			m_bCreated;
	ZMySystemInfo	m_MySystemInfo;
	ZMyGameInfo		m_MyGameInfo;
protected:
	// 계정 정보
	char					m_szAccountID[256];
	MMatchUserGradeID		m_nUGradeID;
	MMatchPremiumGradeID	m_nPGradeID;


	// 캐릭터 정보
	char			m_szCharName[MATCHOBJECT_NAME_LENGTH];
	char			m_szClanName[CLAN_NAME_LENGTH];
	MMatchSex		m_nSex;
	int				m_nHair;		// 머리
	int				m_nFace;		// 얼굴
	int				m_nRace;
	unsigned long int		m_nXP;
	int						m_nBP;
	int						m_nLevel;
	int						m_nLevelPercent;
	MMatchClanGrade			m_nClanGrade;
	bool					m_bNewbie;				

	ZMyItemList		m_ItemList;


#ifdef _QUEST_ITEM
	ZMyQuestItemMap	m_QuestItemMap;
	ZMyQuestItemMap m_ObtainQuestItemMap;

public :
	ZMyQuestItemMap& GetQuestItemMap()			{ return m_QuestItemMap; }
	ZMyQuestItemMap& GetObtainQuestItemMap()	{ return m_ObtainQuestItemMap; }
#endif

	void Clear();
public:
	ZMyInfo();
	virtual ~ZMyInfo();
	bool InitCharInfo(const char* szCharName, const char* szClanName, const MMatchClanGrade nClanGrade, const MMatchSex nSex, const int nHair, const int nFace);
	bool InitAccountInfo(const char* szAccountID, MMatchUserGradeID nUGradeID, MMatchPremiumGradeID nPGradeID);
	void Destroy();
	void Serialize();		// UI 업데이트 - 여기서 내 정보에 대한 UI 업데이트를 하도록 하자.

	// get 씨리즈
	static ZMyInfo*		GetInstance();
	ZMyItemList*		GetItemList() { return &m_ItemList; }
	ZMySystemInfo*		GetSystemInfo()	{ return &m_MySystemInfo; }
	ZMyGameInfo*		GetGameInfo()	{ return &m_MyGameInfo; }
	MMatchSex			GetSex() { return m_nSex; }
	int					GetHair() { return m_nHair; }
	int					GetFace() { return m_nFace; }
	int					GetRace() { return m_nRace; }
	int					GetLevel() const { return m_nLevel;}
	int					GetLevelPercent()	{ return m_nLevelPercent; }
	const char*			GetCharName() { return m_szCharName; }
	const char*			GetClanName() { return m_szClanName; }
	const char*			GetAccountID()	{ return m_szAccountID; }
	unsigned long int	GetXP() { return m_nXP; }
	int					GetBP() { return m_nBP; }
	int					GetHP();
	int					GetAP();
	bool				IsNewbie()	{ return m_bNewbie; }	// 초보자인지 여부(가지고 있는 캐릭터 최고레벨이 5레벨 미만)
	MMatchUserGradeID	GetUGradeID()		{ return m_nUGradeID; }
	MMatchPremiumGradeID	GetPGradeID()		{ return m_nPGradeID; }
	MMatchClanGrade		GetClanGrade()	{ return m_nClanGrade; }
	bool				IsAdminGrade() {
		if ( (GetUGradeID() == MMUG_EVENTMASTER) || (GetUGradeID() == MMUG_DEVELOPER) || (GetUGradeID() == MMUG_ADMIN) )
			return true;
		else
			return false;
	}
	bool				IsPremiumIPUser() { return (m_nPGradeID == MMPG_PREMIUM_IP); }
	bool				IsClanJoined() { return ((m_szClanName[0] == 0) ? false : true); }

	// set 씨리즈
	void SetXP(unsigned long int nXP)			{ m_nXP = nXP; }
	void SetBP(int nBP)							{ m_nBP = nBP; }
	void SetLevel( int nLevel );
	void SetLevelPercent(int nPercent)			{ m_nLevelPercent = nPercent; }
	void SetClanInfo(const char* szClanName, const MMatchClanGrade nClanGrade);
	void SetNewbie(bool bNewbie)				{ m_bNewbie = bNewbie; }
};

inline ZMyInfo* ZGetMyInfo() { return ZMyInfo::GetInstance(); }


#endif