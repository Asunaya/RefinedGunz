#ifndef MMATCHOBJECT_H
#define MMATCHOBJECT_H

#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <deque>
using namespace std;
#include "MMatchItem.h"
#include "MUID.h"
#include "MObject.h"
#include "MMatchGlobal.h"
#include "MMatchFriendInfo.h"
#include "MMatchClan.h"
#include "MMatchChannel.h"
#include "MSmartRefreshImpl.h"
#include "MQuestItem.h"
#include "MMatchAntiHack.h"
#include "MMatchHShield.h"
#include "GlobalTypes.h"
#include "stuff.h"

// 등급 - 이것은 디비의 UserGrade테이블과 싱크가 맞아야 한다.
enum MMatchUserGradeID
{
	MMUG_FREE			= 0,	// 무료 계정
	MMUG_REGULAR		= 1,	// 정액 유저
	MMUG_STAR			= 2,	// 스타유저(게임짱)

	MMUG_CRIMINAL		= 100,	// 전과자
	MMUG_WARNING_1		= 101,	// 1차 경고
	MMUG_WARNING_2		= 102,	// 2차 경고
	MMUG_WARNING_3		= 103,	// 3차 경고
	MMUG_CHAT_LIMITED	= 104,  // 채팅 금지
	MMUG_PENALTY		= 105,	// 기간 정지

	MMUG_EVENTMASTER	= 252,	// 이벤트 진행자
	MMUG_BLOCKED		= 253,	// 사용 정지
	MMUG_DEVELOPER		= 254,	// 개발자
	MMUG_ADMIN			= 255	// 관리자
};



enum MMatchDisconnectStatus
{
	MMDS_CONNECTED = 1,
	MMDS_DISCONN_WAIT,
	MMDS_DISCONNECT,

	MMDS_END,
};



// 유료이용자 권한 - 지금은 DB의 PGradeID를 읽지 않고, 단지 넷마블PC방 보너스 용도로만 사용한다.
enum MMatchPremiumGradeID
{
	MMPG_FREE			= 0,	// 무료
	MMPG_PREMIUM_IP		= 1		// 넷마블 PC방 보너스
};

// 성별 - 이것은 디비의 성별정보와 싱크가 맞아야 한다.
enum MMatchSex
{
	MMS_MALE = 0,		// 남자
	MMS_FEMALE = 1		// 여자
};


enum MMatchBlockType
{
	MMBT_NO = 0,
	MMBT_BANNED,
	MMBT_MUTED,

	MMBT_END,
};


/// 계정 정보
struct MMatchAccountInfo
{
	int						m_nAID;
	char					m_szUserID[32];			// 계정ID
	MMatchUserGradeID		m_nUGrade;				// 등급
	MMatchPremiumGradeID	m_nPGrade;				// 유료 이용자 권한
	MMatchBlockType			m_BlockType;
	SYSTEMTIME				m_EndBlockDate;	// 해킹 유저블럭이 유지되는 시간.

	MMatchAccountInfo() : m_nAID(-1), m_nUGrade(MMUG_FREE), m_nPGrade(MMPG_FREE)
	{
		m_BlockType = MMBT_NO;
		
		memset(m_szUserID, 0, sizeof(m_szUserID));
	}
};

// 플레이어가 현재 위치하고 있는 곳
enum MMatchPlace
{
	MMP_OUTSIDE	= 0,
	MMP_LOBBY = 1,
	MMP_STAGE	= 2,
	MMP_BATTLE	= 3,
	MMP_END
};

enum MMatchObjectStageState
{
	MOSS_NONREADY	= 0,
	MOSS_READY		= 1,
	MOSS_SHOP		= 2,
	MOSS_EQUIPMENT	= 3,
	MOSS_END
};

#define DEFAULT_CHAR_HP				100
#define DEFAULT_CHAR_AP				0

#define DBCACHING_REQUEST_POINT		40
struct DBCharCachingData
{
	int	nAddedXP;
	int	nAddedBP;
	int	nAddedKillCount;
	int	nAddedDeathCount;
	void Reset()
	{
		nAddedXP = 0;
		nAddedBP = 0;
		nAddedKillCount = 0;
		nAddedDeathCount = 0;
	}
	bool IsRequestUpdate()
	{
		if ((nAddedKillCount > DBCACHING_REQUEST_POINT) || (nAddedDeathCount > DBCACHING_REQUEST_POINT))
			return true;

		return false;
	}
};


// 캐릭터의 클랜정보
struct MMatchCharClanInfo
{
	int					m_nClanID;							// db상의 클랜 ID
	char				m_szClanName[CLAN_NAME_LENGTH];		// 클랜 이름
	MMatchClanGrade		m_nGrade;							// 클랜에서의 권한
	int					m_nContPoint;						// 클랜 기여도
	string				m_strDeleteDate;

	MMatchCharClanInfo() {  Clear(); }
	void Clear()
	{
		m_nClanID = 0; 
		m_szClanName[0] = 0; 
		m_nGrade=MCG_NONE;
		m_nContPoint = 0;
		m_strDeleteDate.clear();
	}
	bool IsJoined() { return (m_nClanID == 0) ? false : true; }
};


#define DEFAULT_CHARINFO_MAXWEIGHT		100
#define DEFAULT_CHARINFO_SAFEFALLS		0
#define DEFAULT_CHARINFO_BONUSRATE		0.0f
#define DEFAULT_CHARINFO_PRIZE			0


/// 캐릭터 정보
class MMatchCharInfo
{
public:
	unsigned long int	m_nCID;
	int					m_nCharNum;
	char				m_szName[MATCHOBJECT_NAME_LENGTH];
	int					m_nLevel;
	MMatchSex			m_nSex;			// 성별
	int					m_nHair;		// 머리
	int					m_nFace;		// 얼굴
	unsigned long int	m_nXP;
	int					m_nBP;
	float				m_fBonusRate;
	int					m_nPrize;
	int					m_nHP;
	int					m_nAP;
	int					m_nMaxWeight;
	int					m_nSafeFalls;
	int					m_nFR;
	int					m_nCR;
	int					m_nER;
	int					m_nWR;
	unsigned long int	m_nEquipedItemCIID[MMCIP_END];
	MMatchItemMap		m_ItemList;			// 아이템 정보
	MMatchEquipedItem	m_EquipedItem;		// 장비하고 있는 아이템 정보
	MMatchCharClanInfo	m_ClanInfo;			// 클랜 정보

	// 퀘스트 아이템.
	MQuestItemMap		m_QuestItemList;
	DBQuestCachingData	m_DBQuestCachingData;
	DBQuestCachingData& GetDBQuestCachingData() { return m_DBQuestCachingData; }


	// 몬스터 바이블
	MQuestMonsterBible	m_QMonsterBible;


	unsigned long int	m_nTotalPlayTimeSec;	// 플레이 시간
	unsigned long int	m_nConnTime;			// 접속한 시간(1초 = 1000)

	unsigned long int	m_nTotalKillCount;			// 전체 킬수
	unsigned long int	m_nTotalDeathCount;			// 전체 데쓰수
	unsigned long int	m_nConnKillCount;			// 접속이후로 누적된 킬수
	unsigned long int	m_nConnDeathCount;			// 접속이후로 누적된 데쓰수
	unsigned long int   m_nConnXP;					// 접속이후로 누적된 경험치

protected:
	DBCharCachingData	m_DBCachingData;
public:
	MMatchCharInfo() : m_nCID(0), m_nCharNum(0), m_nLevel(0), m_nSex(MMS_MALE), m_nFace(0),
		               m_nHair(0), m_nXP(0), m_nBP(0), m_fBonusRate(DEFAULT_CHARINFO_BONUSRATE), m_nPrize(DEFAULT_CHARINFO_PRIZE), m_nHP(0),
					   m_nAP(0), m_nMaxWeight(DEFAULT_CHARINFO_MAXWEIGHT), m_nSafeFalls(DEFAULT_CHARINFO_SAFEFALLS),
					   m_nFR(0), m_nCR(0), m_nER(0), m_nWR(0),
					   m_nConnTime(0), m_nTotalKillCount(0), m_nTotalDeathCount(0), m_nConnKillCount(0), m_nConnDeathCount(0), 
					   m_nConnXP(0)
	{
		memset(m_szName, 0, sizeof(m_szName));
		memset(m_nEquipedItemCIID, 0, sizeof(m_nEquipedItemCIID));
		memset(&m_DBCachingData, 0, sizeof(m_DBCachingData));
		memset(&m_QMonsterBible, 0, sizeof(MQuestMonsterBible) );

		m_QuestItemList.Clear();
		m_DBQuestCachingData.Reset();
	}
	void EquipFromItemList();
	void ClearItems();
	void Clear();
	void GetTotalWeight(int* poutWeight, int* poutMaxWeight);


	// db caching까지 함께 더해준다.
	void IncKill()
	{ 
		m_nTotalKillCount += 1;
		m_nConnKillCount += 1;
		m_DBCachingData.nAddedKillCount += 1;
	}
	void IncDeath()
	{ 
		m_nTotalDeathCount += 1;
		m_nConnDeathCount += 1;
		m_DBCachingData.nAddedDeathCount += 1;
	}
	void IncBP(int nAddedBP)		
	{ 
		m_nBP += nAddedBP;
		m_DBCachingData.nAddedBP += nAddedBP;
	}
	void DecBP(int nDecBP)
	{ 
		m_nBP -= nDecBP;
		m_DBCachingData.nAddedBP -= nDecBP;
	}
	void IncXP(int nAddedXP)
	{ 
		m_nConnXP += nAddedXP;
		m_nXP += nAddedXP;
		m_DBCachingData.nAddedXP += nAddedXP;
	}
	void DecXP(int nDecXP)
	{ 
		m_nConnXP -= nDecXP; 
		m_nXP -= nDecXP; 
		m_DBCachingData.nAddedXP -= nDecXP; 
	}

	DBCharCachingData* GetDBCachingData() { return &m_DBCachingData; }
};

class MMatchTimeSyncInfo {
protected:
	int				m_nFoulCount;
	unsigned long	m_nLastSyncClock;
public:
	MMatchTimeSyncInfo()				{ m_nFoulCount=0; m_nLastSyncClock=0; }
	virtual ~MMatchTimeSyncInfo()		{}

	int GetFoulCount()					{ return m_nFoulCount; }
	void AddFoulCount()					{ m_nFoulCount++; }
	void ResetFoulCount()				{ m_nFoulCount = 0; }
	unsigned long GetLastSyncClock()	{ return m_nLastSyncClock; }
	void Update(unsigned long nClock)	{ m_nLastSyncClock = nClock; }
};

// MatchObject가 게임안에서 사용하는 변수들
struct MMatchObjectGameInfo
{
	bool		bJoinedGame;		// 게임에 참가중인지 여부 - 팀전에서 난입했을때
};


class MMatchDisconnStatusInfo
{
public :
	MMatchDisconnStatusInfo() 
	{
		m_DisconnStatus						= MMDS_CONNECTED;
		m_dwLastDisconnStatusUpdatedTime	= timeGetTime();
		m_bIsSendDisconnMsg					= false;
		m_dwDisconnSetTime					= 0;
		m_bIsUpdateDB						= false;
	}

	~MMatchDisconnStatusInfo() {}

	const MMatchDisconnectStatus	GetStatus()				{ return m_DisconnStatus; }
	const DWORD						GetLastUpdatedTime()	{ return m_dwLastDisconnStatusUpdatedTime; }
	const DWORD						GetMsgID()				{ return m_dwMsgID; }
	const MMatchBlockType			GetBlockType()			{ return m_BlockType; } 
	const MMatchBlockLevel			GetBlockLevel()			{ return m_BlockLevel; }
	const string&					GetEndDate()			{ return m_strEndDate; }
	const string&					GetComment()			{ return m_strComment; }
		
	const bool	IsSendDisconnMsg()	{ return m_bIsSendDisconnMsg; }
	void		SendCompleted()		{ m_bIsSendDisconnMsg = false; }	// MMatchServer에서 커맨드 처리를 위해서 사용...
																		// IsSendDisconnMsg로 접속 종료 메시지를 보내야 하는지 검사후,
																		// 커맨드를 보내후에는 SendCompleted()를 호출하여 다음에 중복으로 보내는 것을 막는다.
																		// 더 좋은 방법을 생각해 봐야 함. -by SungE. 2006-03-07.

	const bool IsUpdateDB()			{ return m_bIsUpdateDB; }
	void UpdateDataBaseCompleted()	{ m_bIsUpdateDB = false; } // Update되면은 다음 BlockType이 설정전까진 false로 설정.

	const bool IsDisconnectable( const DWORD dwTime = timeGetTime() )
	{
		if( (MMDS_DISCONNECT == GetStatus()) && (MINTERVAL_DISCONNECT_STATUS_MIN < (dwTime - m_dwDisconnSetTime)) )
			return true;
		return false;
	}
	
	void SetStatus( const MMatchDisconnectStatus Status )	
	{
		m_DisconnStatus = Status; 
		if( MMDS_DISCONN_WAIT == Status )
			SendDisconnMsgPrepared();

		if( MMDS_DISCONNECT == Status )
			m_dwDisconnSetTime = (timeGetTime() - 2000);
		
	}
	void SetUpdateTime( const DWORD dwTime )				{ m_dwLastDisconnStatusUpdatedTime = dwTime; }
	void SetMsgID( const DWORD dwMsgID )					{ m_dwMsgID = dwMsgID; }
	void SetBlockType( const MMatchBlockType BlockType )	{ m_BlockType = BlockType; m_bIsUpdateDB = true; } // 새로운 BlockType이 설정되면 DB업데이트 준비도 같이함.
	void SetBlockLevel( const MMatchBlockLevel BlockLevel ) { m_BlockLevel = BlockLevel; }
	void SetEndDate( const string& strEndDate )				{ m_strEndDate = strEndDate; }
	void SetComment( const string& strComment )				{ m_strComment = strComment; }
	
	void Update( const DWORD dwTime )
	{
		if( (dwTime - GetLastUpdatedTime()) > MINTERVAL_DISCONNECT_STATUS_MIN ) 
		{
			if( MMDS_DISCONN_WAIT == GetStatus() )
				SetStatus( MMDS_DISCONNECT );
			
			SetUpdateTime( dwTime );
		}
	}

private :
	void SendDisconnMsgPrepared()	{ m_bIsSendDisconnMsg = true; }

private :
	MMatchDisconnectStatus	m_DisconnStatus;
	DWORD					m_dwLastDisconnStatusUpdatedTime;
	DWORD					m_dwDisconnSetTime;
	DWORD					m_dwMsgID;
	MMatchBlockType			m_BlockType;
	MMatchBlockLevel		m_BlockLevel;	// Level에따라 계정 블럭, 로그작업만 하는등으로 나눠짐.
	string					m_strEndDate;
	string					m_strComment;
	bool					m_bIsSendDisconnMsg;
	bool					m_bIsUpdateDB;

	const static DWORD MINTERVAL_DISCONNECT_STATUS_MIN;
};


struct MMatchObjectChannelInfo
{
	MUID			uidChannel;
	MUID			uidRecentChannel;
	bool			bChannelListTransfer;
	MCHANNEL_TYPE	nChannelListType;
	unsigned long	nChannelListChecksum;
	int				nTimeLastChannelListTrans;
	void Clear()
	{
		uidChannel = MUID(0,0);
		uidRecentChannel = MUID(0,0);
		bChannelListTransfer = false;
		nChannelListType = MCHANNEL_TYPE_PRESET;
		nChannelListChecksum = 0;
		nTimeLastChannelListTrans = 0;
	}
};


class MMatchObject : public MObject {
protected:
	MMatchAccountInfo			m_AccountInfo;		// 계정 정보
	MMatchCharInfo*				m_pCharInfo;		// 캐릭터 정보
	MMatchFriendInfo*			m_pFriendInfo;		// 친구정보
	MMatchPlace					m_nPlace;			// 위치 정보
	MMatchTimeSyncInfo			m_nTimeSyncInfo;	// 스피드핵 감시	
	MMatchObjectChannelInfo		m_ChannelInfo;		// 채널 정보
	MMatchObjectGameInfo		m_GameInfo;			// 게임안에서의 정보
	MMatchObjectAntiHackInfo	m_AntiHackInfo;		// XTrap에서 사용하는 것들 정보
	MMatchDisconnStatusInfo		m_DisconnStatusInfo;// 오브젝으 접속종료 상태 정보.
	MMatchObjectHShieldInfo		m_HSieldInfo;

	bool			m_bHacker;						// 핵이 검출된 유저
	bool			m_bBridgePeer;
	bool			m_bRelayPeer;
	MUID			m_uidAgent;
	MMatchTeam		m_nTeam;

	DWORD			m_dwIP;
	char 			m_szIP[64];
	unsigned int	m_nPort;
	bool			m_bFreeLoginIP;					// 인원제한 상관없이 로그인 허용된 클라이언트

	unsigned char	m_nPlayerFlags;					// 휘발성 속성등 (MTD_PlayerFlags)
	unsigned long	m_nUserOptionFlags;				// 귓말,초대 거부등의 옵션

	MUID			m_uidStage;
	MUID			m_uidChatRoom;

	bool			m_bStageListTransfer;
	unsigned long	m_nStageListChecksum;
	unsigned long	m_nStageListLastChecksum;
	int				m_nTimeLastStageListTrans;
	int				m_nStageCursor;

	MRefreshClientChannelImpl		m_RefreshClientChannelImpl;
	MRefreshClientClanMemberImpl	m_RefreshClientClanMemberImpl;

	MMatchObjectStageState	m_nStageState;	// 대기방에서의 상태정보
	int				m_nLadderGroupID;
	bool			m_bLadderChallenging;	// 클랜전 상대팀 대기중인지 여부

//	bool			m_bStageMaster;	
	bool			m_bEnterBattle;
	bool			m_bAlive;
	unsigned int	m_nDeadTime;

	bool			m_bNewbie;				// 자신의 캐릭터들의 최고레벨이 10레벨이상이면 입문채널에 들어갈 수 없다.
	bool			m_bForcedEntried;		// 난입한 플레이어인지 여부.
	bool			m_bLaunchedGame;

	unsigned int			m_nKillCount;	// 한라운드 죽인수
	unsigned int			m_nDeathCount;	// 한라운드 죽은수
	unsigned long int		m_nAllRoundKillCount;	// 전체 라운드의 킬수
	unsigned long int		m_nAllRoundDeathCount;	// 전체 라운드의 데쓰수

	bool			m_bWasCallVote;		// 한 게임 안에서 투표건의를 했는가?

	bool			m_bDBFriendListRequested;		// 친구리스트 DB에 정보요청을 했는지 여부
	unsigned long int	m_nTickLastPacketRecved;	// 최근 패킷 받은시간
	unsigned long int	m_nLastHShieldMsgRecved;
	bool			m_bHShieldMsgRecved;
	string				m_strCountryCode3; // filter에서 설정됨.

	DWORD	m_dwLastHackCheckedTime;
	DWORD	m_dwLastRecvNewHashValueTime;
	bool	m_bIsRequestNewHashValue;

	DWORD	m_dwLastSpawnTime;				// 마지막으로 스폰 되었던 시간

	unsigned long int m_nLastPingTime;		// 퀘스트모드, 핑 보낸 시간
	unsigned long int m_nQuestLatency;		// 퀘스트모드
	
	// This is awkward.
	std::deque<BasicInfo> BasicInfoHistory;
	std::deque<int> Pings;
	int AveragePing;

protected:
	void UpdateChannelListChecksum(unsigned long nChecksum)	{ m_ChannelInfo.nChannelListChecksum = nChecksum; }
	unsigned long GetChannelListChecksum()					{ return m_ChannelInfo.nChannelListChecksum; }

	void UpdateStageListChecksum(unsigned long nChecksum)	{ m_nStageListChecksum = nChecksum; }
	unsigned long GetStageListChecksum()					{ return m_nStageListChecksum; }
	MMatchObject() : MObject() 
	{
	}
	void DeathCount()				{ m_nDeathCount++; m_nAllRoundDeathCount++; }
	void KillCount()				{ m_nKillCount++; m_nAllRoundKillCount++; }

	void CheckClientHashValue( const DWORD dwTime );
public:
	MMatchObject(const MUID& uid);
	virtual ~MMatchObject();

	char* GetName() { 
		if (m_pCharInfo)
			return m_pCharInfo->m_szName; 
		else
			return "Unknown";
	}
	char* GetAccountName()			{ return m_AccountInfo.m_szUserID; }

	bool GetBridgePeer()			{ return m_bBridgePeer; }
	void SetBridgePeer(bool bValue)	{ m_bBridgePeer = bValue; }
	bool GetRelayPeer()				{ return m_bRelayPeer; }
	void SetRelayPeer(bool bRelay)	{ m_bRelayPeer = bRelay; }
	const MUID& GetAgentUID()		{ return m_uidAgent; }
	void SetAgentUID(const MUID& uidAgent)	{ m_uidAgent = uidAgent; }

	void SetPeerAddr(DWORD dwIP, char* szIP, unsigned short nPort)	{ m_dwIP=dwIP; strcpy_safe(m_szIP, szIP); m_nPort = nPort; }
	DWORD GetIP()					{ return m_dwIP; }
	char* GetIPString()				{ return m_szIP; }
	unsigned short GetPort()		{ return m_nPort; }
	bool GetFreeLoginIP()			{ return m_bFreeLoginIP; }
	void SetFreeLoginIP(bool bFree)	{ m_bFreeLoginIP = bFree; }

	void ResetPlayerFlags()						{ m_nPlayerFlags = 0; }
	unsigned char GetPlayerFlags()				{ return m_nPlayerFlags; }
	bool CheckPlayerFlags(unsigned char nFlag)	{ return (m_nPlayerFlags&nFlag?true:false); }
	void SetPlayerFlag(unsigned char nFlagIdx, bool bSet)	
	{ 
		if (bSet) m_nPlayerFlags |= nFlagIdx; 
		else m_nPlayerFlags &= (0xff ^ nFlagIdx);
	}

	void SetUserOption(unsigned long nFlags)	{ m_nUserOptionFlags = nFlags; }
	bool CheckUserOption(unsigned long nFlag)	{ return (m_nUserOptionFlags&nFlag?true:false); }

	MUID GetChannelUID()						{ return m_ChannelInfo.uidChannel; }
	void SetChannelUID(const MUID& uid)			{ SetRecentChannelUID(m_ChannelInfo.uidChannel); m_ChannelInfo.uidChannel = uid; }
	MUID GetRecentChannelUID()					{ return m_ChannelInfo.uidRecentChannel; }
	void SetRecentChannelUID(const MUID& uid)	{ m_ChannelInfo.uidRecentChannel = uid; }

	MUID GetStageUID()					{ return m_uidStage; }
	void SetStageUID(const MUID& uid)	{ m_uidStage = uid; }
	MUID GetChatRoomUID()				{ return m_uidChatRoom; }
	void SetChatRoomUID(const MUID& uid){ m_uidChatRoom = uid; }

	bool CheckChannelListTransfer()	{ return m_ChannelInfo.bChannelListTransfer; }
	void SetChannelListTransfer(const bool bVal, const MCHANNEL_TYPE nChannelType=MCHANNEL_TYPE_PRESET);

	bool CheckStageListTransfer()	{ return m_bStageListTransfer; }
	void SetStageListTransfer(bool bVal)	{ m_bStageListTransfer = bVal; UpdateStageListChecksum(0); }

	MRefreshClientChannelImpl* GetRefreshClientChannelImplement()		{ return &m_RefreshClientChannelImpl; }
	MRefreshClientClanMemberImpl* GetRefreshClientClanMemberImplement()	{ return &m_RefreshClientClanMemberImpl; }

	MMatchTeam GetTeam() const { return m_nTeam; }
	void SetTeam(MMatchTeam nTeam);
	MMatchObjectStageState GetStageState()	{ return m_nStageState; }
	void SetStageState(MMatchObjectStageState nStageState)	{ m_nStageState = nStageState; }
	bool GetEnterBattle()			{ return m_bEnterBattle; }
	void SetEnterBattle(bool bEnter){ m_bEnterBattle = bEnter; }
	bool CheckAlive() const			{ return m_bAlive; }
	bool IsAlive() const			{ return m_bAlive; }
	void SetAlive(bool bVal)		{ m_bAlive = bVal; }
	void SetKillCount(unsigned int nKillCount) { m_nKillCount = nKillCount; }
	unsigned int GetKillCount()		{ return m_nKillCount; }
	void SetDeathCount(unsigned int nDeathCount) { m_nDeathCount = nDeathCount; }
	unsigned int GetDeathCount()	{ return m_nDeathCount; }
	unsigned int GetAllRoundKillCount()	{ return m_nAllRoundKillCount; }
	unsigned int GetAllRoundDeathCount()	{ return m_nAllRoundDeathCount; }
	void SetAllRoundKillCount(unsigned int nAllRoundKillCount) { m_nAllRoundKillCount = nAllRoundKillCount; }
	void SetAllRoundDeathCount(unsigned int nAllRoundDeathCount) { m_nAllRoundDeathCount = nAllRoundDeathCount; }
	void FreeCharInfo();
	void FreeFriendInfo();
	void OnDead();
	void OnKill();
	bool IsEnabledRespawnDeathTime(unsigned int nNowTime);

	void SetForcedEntry(bool bVal) { m_bForcedEntried = bVal; }
	bool IsForcedEntried() { return m_bForcedEntried; }
	void SetLaunchedGame(bool bVal) { m_bLaunchedGame = bVal; }
	bool IsLaunchedGame() { return m_bLaunchedGame; }
	void CheckNewbie(int nCharMaxLevel);
	bool IsNewbie()					{ return m_bNewbie; }
	bool IsHacker()					{ return m_bHacker; }
	void SetHacker(bool bHacker)	{ m_bHacker = bHacker; }

	inline bool WasCallVote()						{ return m_bWasCallVote; }
	inline void SetVoteState( const bool bState )	{ m_bWasCallVote = bState; }

	inline unsigned long int	GetTickLastPacketRecved()		{ return m_nTickLastPacketRecved; }
	inline unsigned long int	GetLastHShieldMsgRecved()		{ return m_nLastHShieldMsgRecved; }
	inline bool				GetHShieldMsgRecved()			{ return m_bHShieldMsgRecved; }
	inline void				SetHShieldMsgRecved(bool bSet)	{ m_bHShieldMsgRecved = bSet; }


	void UpdateTickLastPacketRecved();
	void UpdateLastHShieldMsgRecved();

	void SetLastRecvNewHashValueTime( const DWORD dwTime )	
	{ 
		m_dwLastRecvNewHashValueTime = dwTime; 
		m_bIsRequestNewHashValue = false; 
	}

	DWORD GetLastSpawnTime( void)					{ return m_dwLastSpawnTime;		}
	void SetLastSpawnTime( DWORD dwTime)			{ m_dwLastSpawnTime = dwTime;	}

	inline unsigned long int	GetQuestLatency();
	inline void					SetQuestLatency(unsigned long int l);
	inline void					SetPingTime(unsigned long int t);
public:
	// Ladder 관련
	int GetLadderGroupID()			{ return m_nLadderGroupID; }
	void SetLadderGroupID(int nID)	{ m_nLadderGroupID = nID; }
	void SetLadderChallenging(bool bVal) { m_bLadderChallenging = bVal; }
	bool IsLadderChallenging()			{ return m_bLadderChallenging; }		// 클랜전 상대팀 대기중인지 여부
public:
	MMatchAccountInfo* GetAccountInfo() { return &m_AccountInfo; }
	MMatchCharInfo* GetCharInfo()	{ return m_pCharInfo; }
	void SetCharInfo(MMatchCharInfo* pCharInfo);
	
	MMatchFriendInfo* GetFriendInfo()	{ return m_pFriendInfo; }
	void SetFriendInfo(MMatchFriendInfo* pFriendInfo);
	bool DBFriendListRequested()	{ return m_bDBFriendListRequested; }
	MMatchPlace GetPlace()			{ return m_nPlace; }
	void SetPlace(MMatchPlace nPlace);
	MMatchTimeSyncInfo* GetSyncInfo()	{ return &m_nTimeSyncInfo; }
	MMatchObjectAntiHackInfo* GetAntiHackInfo()		{ return &m_AntiHackInfo; }
	MMatchDisconnStatusInfo& GetDisconnStatusInfo() { return  m_DisconnStatusInfo; }
	MMatchObjectHShieldInfo* GetHShieldInfo()		{ return &m_HSieldInfo; }

	/// 틱 처리
	virtual void Tick(unsigned long int nTime);

	void OnStageJoin();
	void OnEnterBattle();
	void OnLeaveBattle();
	void OnInitRound();

	void SetStageCursor(int nStageCursor);
	const MMatchObjectGameInfo* GetGameInfo() { return &m_GameInfo; }

	//filter
	void			SetCountryCode3( const string strCountryCode3 ) { m_strCountryCode3 = strCountryCode3; }
	const string&	GetCountryCode3() const							{ return m_strCountryCode3; }

	void SetXTrapHackerDisconnectWaitInfo( const MMatchDisconnectStatus DisStatus = MMDS_DISCONN_WAIT );
	void SetHShieldHackerDisconnectWaitInfo( const MMatchDisconnectStatus DisStatus = MMDS_DISCONN_WAIT );
	void SetBadFileCRCDisconnectWaitInfo( const MMatchDisconnectStatus DisStatus = MMDS_DISCONN_WAIT );

	void OnBasicInfo(const BasicInfo& bi);

	bool GetPositions(v3& Head, v3& Root, u32 Time);

	void AddPing(int Ping)
	{
		Pings.push_front(Ping);

		while (Pings.size() > 10)
			Pings.pop_back();

		int sum = 0;
		for (auto val : Pings)
			sum += val;

		AveragePing = sum / Pings.size();
	}

	auto GetPing() const
	{
		return AveragePing;
	}

	auto GetSelectedSlot() const
	{
		if (BasicInfoHistory.empty())
			return MMCIP_PRIMARY;

		return BasicInfoHistory.begin()->SelectedSlot;
	}

public:
	enum MMO_ACTION
	{
		MMOA_STAGE_FOLLOW,
		MMOA_MAX
	};
	bool CheckEnableAction(MMO_ACTION nAction);		// 해당 액션이 가능한지 여부 - 필요할때마다 추가한다.

	bool m_bQuestRecvPong;
	DWORD m_dwHShieldCheckCount;
};

class MMatchObjectList : public map<MUID, MMatchObject*>{};

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
	{{1, 5001, 4001, 30301, 0,     21001, 0, 23001, 0},	{1, 5001, 4001, 30301, 0,     21501, 0, 23501, 0}},	// 건나이트
	{{2, 5002, 0,    30301, 0,     21001, 0, 23001, 0},	{2, 5002, 0,    30301, 0,     21501, 0, 23501, 0}},	// 건파이터
	{{1, 4005, 5001, 30401, 0,     21001, 0, 23001, 0},	{1, 4005, 5001, 30401, 0,     21501, 0, 23501, 0}},	// 애서신
	{{2, 4001, 0,    30401, 0,     21001, 0, 23001, 0},	{2, 4001, 0,    30401, 0,     21501, 0, 23501, 0}},	// 스카우트
	{{2, 4002, 0,    30401, 30001, 21001, 0, 23001, 0},	{2, 4002, 0,    30401, 30001, 21501, 0, 23501, 0}},	// 건프리스트
	{{1, 4006, 0,	 30101, 30001, 21001, 0, 23001, 0},	{1, 4006, 4006, 30101, 30001, 21501, 0, 23501, 0}}	// 닥터
};

/*
const unsigned int g_InitialHair[4][2] = 
{
	{10000, 10022},
	{10001, 10023},
	{10002, 10024},
	{10003, 10025}
};
*/

#define MAX_COSTUME_HAIR		5
const string g_szHairMeshName[MAX_COSTUME_HAIR][2] = 
{
	{"eq_head_01", "eq_head_pony"},
	{"eq_head_02", "eq_head_hair001"},
	{"eq_head_08", "eq_head_hair04"},
	{"eq_head_05", "eq_head_hair006"},
	{"eq_head_08", "eq_head_hair002"}		// 이건 현재 사용안함 - 나중에 다른 모델로 대체해도 됨
};

#define MAX_COSTUME_FACE		20
const string g_szFaceMeshName[MAX_COSTUME_FACE][2] = 
{
	{"eq_face_01", "eq_face_001"},
	{"eq_face_02", "eq_face_002"},
	{"eq_face_04", "eq_face_003"},
	{"eq_face_05", "eq_face_004"},
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


// MC_MATCH_STAGE_ENTERBATTLE 커맨드에서 쓰이는 파라메타
enum MCmdEnterBattleParam
{
	MCEP_NORMAL = 0,
	MCEP_FORCED = 1,		// 난입
	MCEP_END
};

// 입을 수 있는 아이템인지 체크
bool IsEquipableItem(unsigned long int nItemID, int nPlayerLevel, MMatchSex nPlayerSex);

inline bool IsEnabledObject(MMatchObject* pObject) 
{
	if ((pObject == NULL) || (pObject->GetCharInfo() == NULL)) return false;
	return true;
}

inline bool IsAdminGrade(MMatchUserGradeID nGrade) 
{
	if ((nGrade == MMUG_EVENTMASTER) || 
		(nGrade == MMUG_ADMIN) || 
		(nGrade == MMUG_DEVELOPER))
		return true;

	return false;
}

inline bool IsAdminGrade(MMatchObject* pObject) 
{
	if (pObject == NULL) return false;

	return IsAdminGrade(pObject->GetAccountInfo()->m_nUGrade);
}


//////////////////////////////////////////////////////////////////////////


unsigned long int MMatchObject::GetQuestLatency()
{
	unsigned long int nowTime = timeGetTime();

	unsigned long int ret = nowTime - m_nLastPingTime;

	if (ret > m_nQuestLatency)
		m_nQuestLatency = ret;

	return m_nQuestLatency;
}

void MMatchObject::SetQuestLatency(unsigned long int l)
{
	m_nQuestLatency = l - m_nLastPingTime;
}

void MMatchObject::SetPingTime(unsigned long int t)
{
	m_nLastPingTime = t;
}

#endif
