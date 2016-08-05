 #ifndef MMATCHSERVER_H
#define MMATCHSERVER_H

#include "winsock2.h"
#include "MXml.h"
#include "MServer.h"
#include "MMatchObject.h"
#include "MAgentObject.h"
#include "MMatchChannel.h"
#include "MMatchStage.h"
#include "MMatchClan.h"
#include "MSafeUDP.h"
#include "MMatchTransDataType.h"
#include "MMatchAdmin.h"
#include "MAsyncProxy.h"
#include "MMatchGlobal.h"
#include "MMatchShutdown.h"
#include "MMatchChatRoom.h"
#include "MLadderMgr.h"
#include "MMatchQuest.h"
#include "MTypes.h"
#include "MMatchDebug.h"
#include "MStringRes.h"
#include "MMatchStringResManager.h"
#include "MMatchEventManager.h"
#include "GlobalTypes.h"
#include <queue>
#include <unordered_map>
#include "LagCompensation.h"
#include "SQLiteDatabase.h"
#include "MSSQLDatabase.h"

class MMatchAuthBuilder;
class MMatchScheduleMgr;
class MNJ_DBAgentClient;

#define MATCHSERVER_UID		MUID(0, 2)	///< MatchServer의 고유 UID(불변)

// 디버그 코드들 삽입
#define CHECKMEMORYNUMBER	888888

enum CUSTOM_IP_STATUS
{
	CIS_INVALID = 0,
	CIS_NONBLOCK,
	CIS_BLOCK,
	CIS_NON,
};

enum COUNT_CODE_STATUS
{
	CCS_INVALID = 0,
	CCS_NONBLOCK,
	CCS_BLOCK,
	CCS_NON,
};

struct DBVariant
{
	union U
	{
		U() {}
		~U() {}
		SQLiteDatabase sqlite;
		MSSQLDatabase mssql;
	};

	DatabaseType Type;
	U u;

	// TODO: Fixfix
	/*DBVariant(DatabaseType Type)
		: Type(Type)*/
	DBVariant() {}
	void Create(DatabaseType Type)
	{
		this->Type = Type;

		if (Type == DatabaseType::SQLite)
			new (&u) SQLiteDatabase;
		else
			new (&u) MSSQLDatabase;
	}
	DBVariant(const DBVariant&) = delete;
	DBVariant(DBVariant&&) = delete;
	~DBVariant()
	{
		if (Type == DatabaseType::SQLite)
			u.sqlite.~SQLiteDatabase();
		else
			u.mssql.~MSSQLDatabase();
	}
};

class MMatchServer : public MServer{
private:
	static MMatchServer*	m_pInstance;		///< 전역 인스턴스
	unsigned long int		m_nTickTime;		///< 틱 타이머
	inline void SetTickTime(unsigned long int nTickTime);		///< 틱 타이머 설정

	unsigned long		m_HSCheckCounter;

protected:
	unsigned long		m_nItemFileChecksum;	// ZItem.xml 의 변조방지 검사

	MUID				m_NextUseUID;
	MCriticalSection	m_csUIDGenerateLock;
	MCriticalSection	m_csTickTimeLock;

	DWORD				m_checkMemory1;
	MMatchObjectList	m_Objects;
	DWORD				m_checkMemory2;

	MMatchChannelMap	m_ChannelMap;
	DWORD				m_checkMemory3;

	char				m_szDefaultChannelName[CHANNELNAME_LEN];
	char				m_szDefaultChannelRuleName[CHANNELRULE_LEN];

	DWORD				m_checkMemory4;
	MMatchStageMap		m_StageMap;
	DWORD				m_checkMemory5;
	MMatchClanMap		m_ClanMap;
	DWORD				m_checkMemory6;
	MAgentObjectMap		m_AgentMap;
	DWORD				m_checkMemory7;


	DWORD				m_checkMemory8;
	MSafeUDP			m_SafeUDP;
	DWORD				m_checkMemory9;
	DBVariant			Database;
	
	DWORD				m_checkMemory10;
	MAsyncProxy			m_AsyncProxy;
	DWORD				m_checkMemory11;
	MMatchAdmin			m_Admin;
	DWORD				m_checkMemory12;
	MMatchShutdown		m_MatchShutdown;
	DWORD				m_checkMemory13;
	MMatchChatRoomMgr	m_ChatRoomMgr;
	DWORD				m_checkMemory14;
	MLadderMgr			m_LadderMgr;
	DWORD				m_checkMemory15;

	bool				m_bCreated;

	DWORD					m_checkMemory16;

	DWORD					m_checkMemory17;

	DWORD					m_checkMemory18;
	MMatchScheduleMgr*		m_pScheduler;
	DWORD					m_checkMemory19;
	MMatchAuthBuilder*		m_pAuthBuilder;
	DWORD					m_checkMemory20;
	MMatchQuest				m_Quest;	// 임시로 위치이동
	DWORD					m_checkMemory21;

	MCountryFilter			m_CountryFilter;
	IPtoCountryList			m_TmpIPtoCountryList;
	BlockCountryCodeList	m_TmpBlockCountryCodeList;
	CustomIPList			m_TmpCustomIPList;
	DWORD					m_dwBlockCount;
	DWORD					m_dwNonBlockCount;

	MMatchEventManager		m_CustomEventManager;

	u32 LastPingTime = 0;

public:
	LagCompManager LagComp;

	MMatchServer(void);
	virtual ~MMatchServer(void);

	/// 디버그용 
	void CheckMemoryTest(int nState=0, int nValue=0);

	/// 전역 인스턴스 얻기
	static MMatchServer* GetInstance(void);

	/// 초기화
	bool Create(int nPort);
	/// 해제
	void Destroy(void);
	virtual void Shutdown();
	/// 새로운 UID 얻어내기
	virtual MUID UseUID(void);

	MMatchAuthBuilder* GetAuthBuilder()					{ return m_pAuthBuilder; }
	void SetAuthBuilder(MMatchAuthBuilder* pBuilder)	{ m_pAuthBuilder = pBuilder; }

	MMatchChatRoomMgr* GetChatRoomMgr()					{ return &m_ChatRoomMgr; }

protected:
	/// Create()호출시에 불리는 함수
	virtual bool OnCreate(void);
	/// Destroy()호출시에 불리는 함수
	virtual void OnDestroy(void);
	/// 사용자 커맨드 등록
	virtual void OnRegisterCommand(MCommandManager* pCommandManager);
	/// 사용자 커맨드 처리
	virtual bool OnCommand(MCommand* pCommand);
	/// 사용자 루프
	virtual void OnRun(void);
	/// 루프 전
	virtual void OnPrepareRun();

	virtual void OnNetClear(const MUID& CommUID);
	virtual void OnNetPong(const MUID& CommUID, unsigned int nTimeStamp);
	virtual void OnHShieldPong(const MUID& CommUID, unsigned int nTimeStamp);
	bool CheckOnLoginPre(const MUID& CommUID, int nCmdVersion, bool& outbFreeIP, string& strCountryCode3);
	void OnMatchLogin(MUID CommUID, const char* szUserID, const unsigned char *HashedPassword, int HashLength, int nCommandVersion, unsigned long nChecksumPack, int nVersion);
	void OnMatchLoginFromNetmarble(const MUID& CommUID, const char* szCPCookie, const char* szSpareData, int nCmdVersion, unsigned long nChecksumPack);
	void OnMatchLoginFromNetmarbleJP(const MUID& CommUID, const char* szLoginID, const char* szLoginPW, int nCmdVersion, unsigned long nChecksumPack);
	void OnMatchLoginFromDBAgent(const MUID& CommUID, const char* szLoginID, const char* szName, int nSex, bool bFreeLoginIP, unsigned long nChecksumPack);
	void OnMatchLoginFailedFromDBAgent(const MUID& CommUID, int nResult);
	void OnBridgePeer(const MUID& uidChar, DWORD dwIP, DWORD nPort);
	bool AddObjectOnMatchLogin(const MUID& uidComm, 
							   MMatchAccountInfo* pSrcAccountInfo,
							   bool bFreeLoginIP,
							   string strCountryCode3,
							   unsigned long nChecksumPack);

	void NotifyFailedLogin(const MUID &uidComm, const char *szReason);
	void CreateAccount(const MUID &uidComm, const char *szUsername, const unsigned char *HashedPassword, int HashLength, const char *szEmail);
	void CreateAccountResponse(const MUID &uidComm, const char *szMessage);

	void LockUIDGenerate()		{ m_csUIDGenerateLock.Lock(); }
	void UnlockUIDGenerate()	{ m_csUIDGenerateLock.Unlock(); }

	/// 오브젝트 생성
	int ObjectAdd(const MUID& uidComm);
	/// 오브젝트 제거
	int ObjectRemove(const MUID& uid, MMatchObjectList::iterator* pNextItor);

	/// 대화 메시지
	int MessageSay(MUID& uid, char* pszSay);

	/// UDP
	MSafeUDP* GetSafeUDP() { return &m_SafeUDP; }
	void SendCommandByUDP(MCommand* pCommand, char* szIP, int nPort);
	void ParsePacket(char* pData, MPacketHeader* pPacketHeader, DWORD dwIP, WORD wRawPort);
	static bool UDPSocketRecvEvent(DWORD dwIP, WORD wRawPort, char* pPacket, DWORD dwSize);
	void ParseUDPPacket(char* pData, MPacketHeader* pPacketHeader, DWORD dwIP, WORD wRawPort);

	/// Async DB
	void ProcessAsyncJob();
	void OnAsyncGetAccountCharList(MAsyncJob* pJobResult);
	void OnAsyncGetAccountCharInfo(MAsyncJob* pJobResult);
	void OnAsyncGetCharInfo(MAsyncJob* pJobResult);
	void OnAsyncCreateChar(MAsyncJob* pJobResult);
	void OnAsyncDeleteChar(MAsyncJob* pJobResult);
	void OnAsyncGetFriendList(MAsyncJob* pJobInput);
	void OnAsyncGetLoginInfo(MAsyncJob* pJobInput);
	void OnAsyncWinTheClanGame(MAsyncJob* pJobInput);
	void OnAsyncUpdateCharInfoData(MAsyncJob* pJobInput);
	void OnAsyncCharFinalize(MAsyncJob* pJobInput);
	void OnAsyncBringAccountItem(MAsyncJob* pJobResult);
	void OnAsyncInsertConnLog(MAsyncJob* pJobResult);
	void OnAsyncInsertGameLog(MAsyncJob* pJobResult);
	void OnAsyncCreateClan(MAsyncJob* pJobResult);
	void OnAsyncExpelClanMember(MAsyncJob* pJobResult);
	void OnAsyncInsertEvent( MAsyncJob* pJobResult );
	void OnAsyncUpdateIPtoCoutryList( MAsyncJob* pJobResult );
	void OnAsyncUpdateBlockCountryCodeList( MAsyncJob* pJobResult );
	void OnAsyncUpdateCustomIPList( MAsyncJob* pJobResult );

	/// 스케쥴러 초기화
	bool InitScheduler();
	/// 로케일 초기화
	bool InitLocale();
	/// Event초기화.
	bool InitEvent();
public:
	/// Async DB
	void PostAsyncJob(MAsyncJob* pJob);

private :
	// 추가적인 스케쥬을 추가하려면 이 함수를 재정의 하면 됨.
	virtual bool InitSubTaskSchedule() { return true; }

protected:
	/// Object의 접속을 끊는다.
	void DisconnectObject(const MUID& uidObject);
	void DebugTest();
protected:
	// 채널 관련
	const char* GetDefaultChannelName()					{ return m_szDefaultChannelName; }
	void SetDefaultChannelName(const char* pszName)		{ strcpy_safe(m_szDefaultChannelName, pszName); }
	const char* GetDefaultChannelRuleName()				{ return m_szDefaultChannelRuleName; }
	void SetDefaultChannelRuleName(const char* pszName)	{ strcpy_safe(m_szDefaultChannelRuleName, pszName); }

	bool ChannelAdd(const char* pszChannelName, const char* pszRuleName, MUID* pAllocUID, MCHANNEL_TYPE nType=MCHANNEL_TYPE_PRESET, int nMaxPlayers=DEFAULT_CHANNEL_MAXPLAYERS, int nLevelMin=-1, int nLevelMax=-1);
	bool ChannelJoin(const MUID& uidPlayer, const MUID& uidChannel);
	bool ChannelJoin(const MUID& uidPlayer, const MCHANNEL_TYPE nChannelType, const char* pszChannelName);
	bool ChannelLeave(const MUID& uidPlayer, const MUID& uidChannel);
	bool ChannelChat(const MUID& uidPlayer, const MUID& uidChannel, char* pszChat);

	void ResponseChannelRule(const MUID& uidPlayer, const MUID& uidChannel);

	void OnRequestRecommendedChannel(const MUID& uidComm);
	void OnRequestChannelJoin(const MUID& uidPlayer, const MUID& uidChannel);
	void OnRequestChannelJoin(const MUID& uidPlayer, const MCHANNEL_TYPE nChannelType, const char* pszChannelName);
	void OnChannelChat(const MUID& uidPlayer, const MUID& uidChannel, char* pszChat);
	void OnStartChannelList(const MUID& uidPlayer, const int nChannelType);
	void OnStopChannelList(const MUID& uidPlayer);

	void OnChannelRequestPlayerList(const MUID& uidPlayer, const MUID& uidChannel, int nPage);
	void OnChannelRequestAllPlayerList(const MUID& uidPlayer, const MUID& uidChannel, unsigned long int nPlaceFilter, unsigned long int nOptions);

public:
	void ChannelResponsePlayerList(const MUID& uidPlayer, const MUID& uidChannel, int nPage);
	void ChannelResponseAllPlayerList(const MUID& uidPlayer, const MUID& uidChannel, unsigned long int nPlaceFilter, unsigned long int nOptions);

public:
	MMatchStage* FindStage(const MUID& uidStage);
protected:
	friend MMatchStage;
	friend MNJ_DBAgentClient;
	bool StageAdd(MMatchChannel* pChannel, const char* pszStageName, bool bPrivate, const char* pszStagePassword, MUID* pAllocUID);
	bool StageRemove(const MUID& uidStage, MMatchStageMap::iterator* pNextItor);
	bool StageJoin(const MUID& uidPlayer, const MUID& uidStage);
	bool StageLeave(const MUID& uidPlayer, const MUID& uidStage);
	bool StageEnterBattle(const MUID& uidPlayer, const MUID& uidStage);
	bool StageLeaveBattle(const MUID& uidPlayer, const MUID& uidStage);
	bool StageChat(const MUID& uidPlayer, const MUID& uidStage, char* pszChat);
	bool StageTeam(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam);
	bool StagePlayerState(const MUID& uidPlayer, const MUID& uidStage, MMatchObjectStageState nStageState);
	bool StageMaster(const MUID& uidStage);

protected:
	MCommand* CreateCmdResponseStageSetting(const MUID& uidStage);
	MCommand* CreateCmdMatchResponseLoginOK(const MUID& uidComm, 
											MUID& uidPlayer, 
											const char* szUserID, 
											MMatchUserGradeID nUGradeID, 
											MMatchPremiumGradeID nPGradeID,
											const char* szRandomValue,
											const unsigned char* pbyGuidReqMsg);
	MCommand* CreateCmdMatchResponseLoginFailed(const MUID& uidComm, const int nResult);

	
	float GetDuelVictoryMultiflier(int nVictorty);		// 듀얼 연승일때의 가중치~
	float GetDuelPlayersMultiflier(int nPlayerCount);	// 듀얼 사람수에 따른 가중치~
	void CalcExpOnGameKill(MMatchStage* pStage, MMatchObject* pAttacker, MMatchObject* pVictim, 
					   int* poutAttackerExp, int* poutVictimExp);
	const int CalcBPonGameKill( MMatchStage* pStage, MMatchObject* pAttacker, const int nAttackerLevel, const int nVictimLevel );
	void ProcessOnGameKill(MMatchStage* pStage, MMatchObject* pAttacker, MMatchObject* pVictim);
	void PostGameDeadOnGameKill(MUID& uidStage, MMatchObject* pAttacker, MMatchObject* pVictim,
									int nAddedAttackerExp, int nSubedVictimExp);	// ProcessOnGameKill함수에서 사용


	void OnStageCreate(const MUID& uidChar, char* pszStageName, bool bPrivate, char* pszStagePassword);
	void OnStageJoin(const MUID& uidPlayer, const MUID& uidStage);
	void OnPrivateStageJoin(const MUID& uidPlayer, const MUID& uidStage, char* pszPassword);
	void OnStageFollow(const MUID& uidPlayer, const char* pszTargetName);
	void OnStageLeave(const MUID& uidPlayer, const MUID& uidStage);
	void OnStageRequestPlayerList(const MUID& uidPlayer, const MUID& uidStage);
	void OnStageEnterBattle(const MUID& uidPlayer, const MUID& uidStage);
	void OnStageLeaveBattle(const MUID& uidPlayer, const MUID& uidStage);
	void OnStageChat(const MUID& uidPlayer, const MUID& uidStage, char* pszChat);
	void OnRequestQuickJoin(const MUID& uidPlayer, void* pQuickJoinBlob);
	void ResponseQuickJoin(const MUID& uidPlayer, MTD_QuickJoinParam* pQuickJoinParam);
	void OnStageGo(const MUID& uidPlayer, unsigned int nRoomNo);
	void OnStageTeam(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam);
	void OnStagePlayerState(const MUID& uidPlayer, const MUID& uidStage, MMatchObjectStageState nStageState);
	void OnStageStart(const MUID& uidPlayer, const MUID& uidStage, int nCountdown);
	void OnStartStageList(const MUID& uidComm);
	void OnStopStageList(const MUID& uidComm);
	void OnStageRequestStageList(const MUID& uidPlayer, const MUID& uidChannel, const int nStageCursor);
	void OnStageMap(const MUID& uidStage, const char* pszMapName);
	void OnStageSetting(const MUID& uidPlayer, const MUID& uidStage, void* pStageBlob, int nStageCount);
	void OnRequestStageSetting(const MUID& uidComm, const MUID& uidStage);
	void OnRequestPeerList(const MUID& uidChar, const MUID& uidStage);
	void OnRequestGameInfo(const MUID& uidChar, const MUID& uidStage);
	void OnMatchLoadingComplete(const MUID& uidPlayer, int nPercent);
	void OnRequestRelayPeer(const MUID& uidChar, const MUID& uidPeer);
	void OnPeerReady(const MUID& uidChar, const MUID& uidPeer);
	void OnGameRoundState(const MUID& uidStage, int nState, int nRound);
public:
	void OnGameKill(const MUID& uidAttacker, const MUID& uidVictim);
protected:
	void OnRequestSpawn(const MUID& uidChar, const MVector& pos, const MVector& dir);
	void OnGameRequestTimeSync(const MUID& uidComm, unsigned long nLocalTimeStamp);
	void OnGameReportTimeSync(const MUID& uidComm, unsigned long nLocalTimeStamp, unsigned int nDataChecksum);
	void OnUpdateFinishedRound(const MUID& uidStage, const MUID& uidChar, 
							   void* pPeerInfo, void* pKillInfo);
	void OnRequestForcedEntry(const MUID& uidStage, const MUID& uidChar);
	void OnRequestSuicide(const MUID& uidPlayer);
	void OnRequestObtainWorldItem(const MUID& uidPlayer, const int nItemUID);
	void OnRequestSpawnWorldItem(const MUID& uidPlayer, const int nItemID, const float x, const float y, const float z);
	void OnUserWhisper(const MUID& uidComm, char* pszSenderName, char* pszTargetName, char* pszMessage);
	void OnUserWhere(const MUID& uidComm, char* pszTargetName);
	void OnUserOption(const MUID& uidComm, unsigned long nOptionFlags);
	void OnChatRoomCreate(const MUID& uidPlayer, const char* pszChatRoomName);
	void OnChatRoomJoin(const MUID& uidComm, char* pszPlayerName, char* pszChatRoomName);
	void OnChatRoomLeave(const MUID& uidComm, char* pszPlayerName, char* pszChatRoomName);
	void OnChatRoomSelectWrite(const MUID& uidComm, const char* szChatRoomName);
	void OnChatRoomInvite(const MUID& uidComm, const char* pszTargetName);
	void OnChatRoomChat(const MUID& uidComm, const char* pszMessage);

protected:	// 래더
	friend MLadderMgr;
	bool LadderJoin(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam);
	void LadderGameLaunch(MLadderGroup* pGroupA, MLadderGroup* pGroupB);

protected:	// 래더
	void OnLadderRequestInvite(const MUID& uidPlayer, void* pGroupBlob);
	void OnLadderInviteAgree(const MUID& uidPlayer);
	void OnLadderInviteCancel(const MUID& uidPlayer);
	void OnLadderRequestChallenge(const MUID& uidPlayer, void* pGroupBlob, unsigned long int nOptions);
	void OnLadderRequestCancelChallenge(const MUID& uidPlayer);

	void OnRequestProposal(const MUID& uidProposer, const int nProposalMode, const int nRequestID, 
		                   const int nReplierCount, void* pReplierNamesBlob);
	void OnReplyAgreement(MUID& uidProposer, MUID& uidReplier, const char* szReplierName, 
		                  const int nProposalMode, const int nRequestID, const bool bAgreement);
protected:
	void OnRequestCopyToTestServer(const MUID& uidPlayer);						// 현재 사용하지 않는다.
	void ResponseCopyToTestServer(const MUID& uidPlayer, const int nResult);	// 현재 사용하지 않는다.

	void OnRequestMySimpleCharInfo(const MUID& uidPlayer);
	void ResponseMySimpleCharInfo(const MUID& uidPlayer);
	void OnRequestCharInfoDetail(const MUID& uidChar, const char* szCharName);
	void ResponseCharInfoDetail(const MUID& uidChar, const char* szCharName);
	void OnRequestAccountCharList(const MUID& uidPlayer, char* szXTrapSerialKey, unsigned char* pbyGuidAckMsg);
	void OnRequestAccountCharInfo(const MUID& uidPlayer, int nCharNum);
	void OnRequestSelectChar(const MUID& uidPlayer, const int nCharIndex);
	void OnRequestDeleteChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName);
	bool ResponseDeleteChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName);
	void OnRequestCreateChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName,
				const unsigned int nSex, const unsigned int nHair, const unsigned int nFace, 
				const unsigned int nCostume);
	bool ResponseCreateChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName,
		MMatchSex nSex, const unsigned int nHair, const unsigned int nFace, 
		const unsigned int nCostume);
	void OnCharClear(const MUID& uidPlayer);
	bool CharInitialize(const MUID& uidPlayer);
	bool CharFinalize(const MUID& uidPlayer);
	bool CorrectEquipmentByLevel(MMatchObject* pPlayer, MMatchCharItemParts nPart, int nLegalItemLevelDiff=0);	// 수정되면 true
	bool RemoveCharItem(MMatchObject* pObject, MUID& uidItem);
protected: // 친구
	void OnFriendAdd(const MUID& uidPlayer, const char* pszName);
	void OnFriendRemove(const MUID& uidPlayer, const char* pszName);
	void OnFriendList(const MUID& uidPlayer);
	void OnFriendMsg(const MUID& uidPlayer, const char* szMsg);
	void FriendList(const MUID& uidPlayer);

protected:	// 클랜
	int ValidateCreateClan(const char* szClanName, MMatchObject* pMasterObject, MMatchObject** ppSponsorObject);
	void UpdateCharClanInfo(MMatchObject* pObject, const int nCLID, const char* szClanName, const MMatchClanGrade nGrade);

	void OnClanRequestCreateClan(const MUID& uidPlayer, const int nRequestID, const char* szClanName, char** szSponsorNames);
	void OnClanAnswerSponsorAgreement(const int nRequestID, const MUID& uidClanMaster, char* szSponsorCharName, const bool bAnswer);
	void OnClanRequestAgreedCreateClan(const MUID& uidPlayer, const char* szClanName, char** szSponsorNames);
	void OnClanRequestCloseClan(const MUID& uidClanMaster, const char* szClanName);
	void ResponseCloseClan(const MUID& uidClanMaster, const char* szClanName);
	void OnClanRequestJoinClan(const MUID& uidClanAdmin, const char* szClanName, const char* szJoiner);
	void ResponseJoinClan(const MUID& uidClanAdmin, const char* szClanName, const char* szJoiner);
	void OnClanAnswerJoinAgreement(const MUID& uidClanAdmin, const char* szJoiner, const bool bAnswer);
	void OnClanRequestAgreedJoinClan(const MUID& uidClanAdmin, const char* szClanName, const char* szJoiner);
	void ResponseAgreedJoinClan(const MUID& uidClanAdmin, const char* szClanName, const char* szJoiner);

	void OnClanRequestLeaveClan(const MUID& uidPlayer);
	void ResponseLeaveClan(const MUID& uidPlayer);
	void OnClanRequestChangeClanGrade(const MUID& uidClanMaster, const char* szMember, int nClanGrade);
	void ResponseChangeClanGrade(const MUID& uidClanMaster, const char* szMember, int nClanGrade);
	void OnClanRequestExpelMember(const MUID& uidClanAdmin, const char* szMember);
	void ResponseExpelMember(const MUID& uidClanAdmin, const char* szMember);
	void OnClanRequestMsg(const MUID& uidSender, const char* szMsg);
	void OnClanRequestMemberList(const MUID& uidChar);
	void OnClanRequestClanInfo(const MUID& uidChar, const char* szClanName);

	void OnClanRequestEmblemURL(const MUID& uidChar, int nCLID);
public:
	MMatchClan* FindClan(const int nCLID);
	void ResponseClanMemberList(const MUID& uidChar);
public:
	int GetLadderTeamIDFromDB(const int nTeamTableIndex, const int* pnMemberCIDArray, const int nMemberCount);
	void SaveLadderTeamPointToDB(const int nTeamTableIndex, const int nWinnerTeamID, const int nLoserTeamID, const bool bIsDrawGame);
	void SaveClanPoint(MMatchClan* pWinnerClan, MMatchClan* pLoserClan, const bool bIsDrawGame,
						const int nRoundWins, const int nRoundLosses, const int nMapID, const int nGameType,
						const int nOneTeamMemberCount, list<MUID>& WinnerObjUIDs,
						const char* szWinnerMemberNames, const char* szLoserMemberNames, float fPointRatio);
	void BroadCastClanRenewVictories(const char* szWinnerClanName, const char* szLoserClanName, const int nVictories);
	void BroadCastClanInterruptVictories(const char* szWinnerClanName, const char* szLoserClanName, const int nVictories);
	void BroadCastDuelRenewVictories(const MUID& chanID, const char* szChampionName, const char* szChannelName, int nRoomNumber, const int nVictories);
	void BroadCastDuelInterruptVictories(const MUID& chanID, const char* szChampionName, const char* szInterrupterName, const int nVictories);
public:
	friend MVoteDiscuss;
	// 투쵸관련.
	void OnVoteCallVote(const MUID& uidPlayer, const char* pszDiscuss, const char* pszArg);
	void OnVoteYes(const MUID& uidPlayer);
	void OnVoteNo(const MUID& uidPlayer);
	void VoteAbort( const MUID& uidPlayer );
	
	void OnAdminServerHalt(void);

protected:
	// 관리자 기능
	void OnAdminTerminal(const MUID& uidAdmin, const char* szText);
	void OnAdminAnnounce(const MUID& uidAdmin, const char* szChat, unsigned long int nType);
	void OnAdminRequestServerInfo(const MUID& uidAdmin);
	void OnAdminServerHalt(const MUID& uidAdmin);
	
	void OnAdminRequestBanPlayer(const MUID& uidAdmin, const char* szPlayer);
	void OnAdminRequestUpdateAccountUGrade(const MUID& uidAdmin, const char* szPlayer);
	void OnAdminPingToAll(const MUID& uidAdmin);
	void OnAdminRequestSwitchLadderGame(const MUID& uidAdmin, const bool bEnabled);
	void OnAdminHide(const MUID& uidAdmin);
	void OnAdminResetAllHackingBlock( const MUID& uidAdmin );

	// 이벤트 마스터 기능
	void OnEventChangeMaster(const MUID& uidAdmin);
	void OnEventChangePassword(const MUID& uidAdmin, const char* szPassword);
	void OnEventRequestJjang(const MUID& uidAdmin, const char* pszTargetName);
	void OnEventRemoveJjang(const MUID& uidAdmin, const char* pszTargetName);

public:
	void AdminTerminalOutput(const MUID& uidAdmin, const char* szText);
	template<size_t size> bool OnAdminExecute(MAdminArgvInfo *pAI, char(&szOut)[size]) {
		return OnAdminExecute(pAI, szOut, size);
	}
	bool OnAdminExecute(MAdminArgvInfo* pAI, char* szOut, int maxlen);
	void ApplyObjectTeamBonus(MMatchObject* pObject, int nAddedExp);
	void ProcessPlayerXPBP(MMatchStage* pStage, MMatchObject* pPlayer, int nAddedXP, int nAddedBP);
	bool InsertCharItem(const MUID& uidPlayer, const unsigned long int nItemID, bool bRentItem, int nRentPeriodHour);
protected:
	bool BuyItem(MMatchObject* pObject, unsigned int nItemID, bool bRentItem=false, int nRentPeriodHour=0);
	void OnRequestBuyItem(const MUID& uidPlayer, const unsigned long int nItemID);
	bool ResponseBuyItem(const MUID& uidPlayer, const unsigned long int nItemID);
	void OnRequestSellItem(const MUID& uidPlayer, const MUID& uidItem);
	bool ResponseSellItem(const MUID& uidPlayer, const MUID& uidItem);
	void OnRequestShopItemList(const MUID& uidPlayer, const int nFirstItemIndex, const int nItemCount);
	void ResponseShopItemList(const MUID& uidPlayer, const int nFirstItemIndex, const int nItemCount);
	void OnRequestCharacterItemList(const MUID& uidPlayer);
	void ResponseCharacterItemList(const MUID& uidPlayer);
	void OnRequestAccountItemList(const MUID& uidPlayer);
	void ResponseAccountItemList(const MUID& uidPlayer);
	void OnRequestEquipItem(const MUID& uidPlayer, const MUID& uidItem, const long int nEquipmentSlot);
	void ResponseEquipItem(const MUID& uidPlayer, const MUID& uidItem, const MMatchCharItemParts parts);
	void OnRequestTakeoffItem(const MUID& uidPlayer, const unsigned long int nEquipmentSlot);
	void ResponseTakeoffItem(const MUID& uidPlayer, const MMatchCharItemParts parts);
	void OnRequestBringAccountItem(const MUID& uidPlayer, const int nAIID);
	void ResponseBringAccountItem(const MUID& uidPlayer, const int nAIID);
	void OnRequestBringBackAccountItem(const MUID& uidPlayer, const MUID& uidItem);
	void ResponseBringBackAccountItem(const MUID& uidPlayer, const MUID& uidItem);

public:
	void OnDuelSetObserver(const MUID& uidChar);
	void OnDuelQueueInfo(const MUID& uidStage, const MTD_DuelQueueInfo& QueueInfo);	// 쳇-.-
	void OnQuestSendPing(const MUID& uidStage, unsigned long int t);

protected :
	// Keeper관련.
	void OnResponseServerStatus( const MUID& uidSender );
	void OnRequestServerHearbeat( const MUID& uidSender );
	void OnResponseServerHeartbeat( const MUID& uidSender );
	void OnRequestConnectMatchServer( const MUID& uidSender );
	void OnResponseConnectMatchServer( const MUID& uidSender );
	void OnRequestKeeperAnnounce( const MUID& uidSender, const char* pszAnnounce );
	void OnRequestStopServerWithAnnounce( const MUID& uidSender );
	void OnResponseStopServerWithAnnounce( const MUID& uidSender );
	void OnRequestSchedule( const MUID& uidSender, 
							const int nType, 
							const int nYear, 
							const int nMonth, 
							const int nDay, 
							const int nHour, 
							const int nMin,
							const int nCount,
							const int nCommand,
							const char* pszAnnounce );
	void OnResponseSchedule( const MUID& uidSender, 
							 const int nType, 
							 const int nYear, 
							 const int nMonth, 
							 const int nDay, 
							 const int nHour, 
							 const int nMin,
							 const int nCount,
							 const int nCommand,
							 const char* pszAnnounce );

protected:
	// 퀘스트 관련
	void OnRequestNPCDead(const MUID& uidSender, const MUID& uidKiller, MUID& uidNPC, MVector& pos);
	void OnQuestRequestDead(const MUID& uidVictim);
	void OnQuestTestRequestNPCSpawn(const MUID& uidPlayer, int nNPCType, int nNPCCount);
	void OnQuestTestRequestClearNPC(const MUID& uidPlayer);
	void OnQuestTestRequestSectorClear(const MUID& uidPlayer);
	void OnQuestTestRequestQuestFinish(const MUID& uidPlayer);
	void OnQuestRequestMovetoPortal(const MUID& uidPlayer);
	void OnQuestReadyToNewSector(const MUID& uidPlayer);
	void OnQuestStageMapset(const MUID& uidStage, int nMapsetID);
	// 퀘스트 아이템 관련.
	void OnResponseCharQuestItemList( const MUID& uidSender );
	void OnRequestBuyQuestItem( const MUID& uidSender, const unsigned long int nItemID );
	void OnResponseBuyQeustItem( const MUID& uidSender, const unsigned long int nItemID );
	void OnRequestSellQuestItem( const MUID& uidSender, const unsigned long int nItemID, const int nCount );
	void OnResponseSellQuestItem( const MUID& uidSender, const unsigned long int nItemID, const int nCount );
	void OnRequestDropSacrificeItemOnSlot( const MUID& uidSender, const int nSlotIndex, const unsigned long int nItemID );
	void OnRequestCallbackSacrificeItem( const MUID& uidSender, const int nSlotIndex, const unsigned long int nItemID );
	void OnRequestQL( const MUID& uidSender );
	void OnRequestSacrificeSlotInfo( const MUID& uidSender );
	void OnRequestMonsterBibleInfo( const MUID& uidSender );
	void OnResponseMonsterBibleInfo( const MUID& uidSender );

	void OnQuestPong( const MUID& uidSender );
public :
	void OnRequestCharQuestItemList( const MUID& uidSender );
	///


protected:
	/// Agent 생성
	int AgentAdd(const MUID& uidComm);
	/// Agent 제거
	int AgentRemove(const MUID& uidAgent, MAgentObjectMap::iterator* pNextItor);

	/// Agent
	MAgentObject* GetAgent(const MUID& uidAgent);
	MAgentObject* GetAgentByCommUID(const MUID& uidComm);
	bool CheckBridgeFault();
	MAgentObject* FindFreeAgent();
	void ReserveAgent(MMatchStage* pStage);
	void LocateAgentToClient(const MUID& uidPlayer, const MUID& uidAgent);

	void OnRegisterAgent(const MUID& uidComm, char* szIP, int nTCPPort, int nUDPPort);
	void OnUnRegisterAgent(const MUID& uidComm);
	void OnAgentStageReady(const MUID& uidCommAgent, const MUID& uidStage);
	void OnRequestLiveCheck(const MUID& uidComm, unsigned long nTimeStamp, 
							unsigned long nStageCount, unsigned long nUserCount);
public:
	/// UID로 오브젝트 얻어내기
	MMatchObject* GetObject(const MUID& uid);
	/// UID로 캐릭터 오브젝트 얻어내기
	MMatchObject* GetPlayerByCommUID(const MUID& uid);
	/// Name으로 오브젝트 얻어내기
	MMatchObject* GetPlayerByName(const char* pszName);
	/// AID로 오브젝트 얻어내기
	MMatchObject* GetPlayerByAID(unsigned long int nAID);

	/// UID로 채널 얻어내기
	MMatchChannel* FindChannel(const MUID& uidChannel);
	/// Name으로 채널 얻어내기
	MMatchChannel* FindChannel(const MCHANNEL_TYPE nChannelType, const char* pszChannelName);

	/// 서버의 공지 메시지 전송
	void Announce(const MUID& CommUID, char* pszMsg);
	void Announce(MObject* pObj, char* pszMsg);
	void AnnounceF(const MUID& CommUID, char* pszMsg, ...)
	{
		char buf[512];

		va_list args;

		va_start(args, pszMsg);
		vsprintf_safe(buf, pszMsg, args);
		va_end(args);

		Announce(CommUID, buf);
	}
	/// 서버의 에러 메시지 전송
	void AnnounceErrorMsg(const MUID& CommUID, const int nErrorCode);
	void AnnounceErrorMsg(MObject* pObj, const int nErrorCode);
	/// Command를 Object의 Listener에게 전송
	void RouteToListener(MObject* pObject, MCommand* pCommand);
	/// Command를 전체 커낵션으로 전송
	void RouteToAllConnection(MCommand* pCommand);
	/// Command를 전체 클라이언트로 전송
	void RouteToAllClient(MCommand* pCommand);
	template <typename T>
	void RouteToAllClientIf(MCommand* pCommand, T& pred)
	{
		for (MMatchObjectList::iterator i = m_Objects.begin(); i != m_Objects.end(); i++) {
			MMatchObject* pObj = (MMatchObject*)((*i).second);
			if (pObj->GetUID() < MUID(0, 3)) continue;	// MUID로 Client인지 판별할수 있는 코드 필요함
			if (!pred(*pObj))
				continue;

			MCommand* pSendCmd = pCommand->Clone();
			pSendCmd->m_Receiver = pObj->GetUID();
			Post(pSendCmd);
		}
		delete pCommand;
	}
	/// Command를 지정 Channel 참가자에게 전송
	void RouteToChannel(const MUID& uidChannel, MCommand* pCommand);
	/// Command를 지정 Channel 로비에 있는 참가자에게 전송
	void RouteToChannelLobby(const MUID& uidChannel, MCommand* pCommand);
	/// Command를 지정 Stage 참가자에게 전송
	void RouteToStage(const MUID& uidStage, MCommand* pCommand);
	/// Command를 지정 Stage 대기방 참가자에게 전송
	void RouteToStageWaitRoom(const MUID& uidStage, MCommand* pCommand);
	/// Command를 지정 Stage 배틀 참가자에게 전송
	void RouteToBattle(const MUID& uidStage, MCommand* pCommand);
	void RouteToBattleExcept(const MUID& uidStage, MCommand* pCommand, const MUID& uidExceptedPlayer);
	/// Command를 지정 Clan 에게 전송
	void RouteToClan(const int nCLID, MCommand* pCommand);
	// int 결과값있는 Command를 지정 Object Listener에게 전송
	void RouteResponseToListener(MObject* pObject, const int nCmdID, int nResult);

	void OnVoiceChat(const MUID& Player, unsigned char* EncodedFrame, int Length);

	void OnTunnelledP2PCommand(const MUID& Sender, const MUID& Receiver, const char* Blob, size_t BlobSize);
	//void PickHistory(MMatchObject& Exception, u32 Time, rvector &origin, rvector &to, MMatchStage& Stage, struct MPICKINFO& pickinfo);

//	void ResponseObjectUpdate(MUID& TargetUID, MObject* pObject);
	void ResponseBridgePeer(const MUID& uidChar, int nCode);
	void ResponseRoundState(const MUID& uidStage);
	void ResponseRoundState(MMatchObject* pObj, const MUID& uidStage);
	void ResponsePeerList(const MUID& uidChar, const MUID& uidStage);
	void ResponseGameInfo(const MUID& uidChar, const MUID& uidStage);

	// x-trap : check client new hash value.
	void RequestNewHashValue( const MUID& uidChar, const char* szNewRandom );
	void OnResponeNewHashValue( const MUID& uidChar, char* szSerialKey );

	void NotifyMessage(const MUID& uidChar, int nMsgID);

	unsigned long GetChannelListChecksum()	{ return m_ChannelMap.GetChannelListChecksum(); }
	void ChannelList(const MUID& uidPlayer, MCHANNEL_TYPE nChannelType);

	unsigned long int GetStageListChecksum(MUID& uidChannel, int nStageCursor, int nStageCount);
	void StageList(const MUID& uidPlayer, int nStageStartIndex, bool bCacheUpdate);
	void StageLaunch(const MUID& uidStage);
	void StageFinishGame(const MUID& uidStage);

	void StandbyClanList(const MUID& uidPlayer, int nClanListStartIndex, bool bCacheUpdate);


	/// 현재 클럭 얻어내기
	unsigned long int GetGlobalClockCount(void);
	/// 클라이언트에게 클럭을 맞추게 한다.
	void SetClientClockSynchronize(const MUID& CommUID);
	/// Local Clock을 Global Clock으로 변환
	static unsigned long int ConvertLocalClockToGlobalClock(unsigned long int nLocalClock, unsigned long int nLocalClockDistance);
	/// Global Clock을 Local Clock으로 변환
	static unsigned long int ConvertGlobalClockToLocalClock(unsigned long int nGlobalClock, unsigned long int nLocalClockDistance);

	bool IsCreated() { return m_bCreated; }
	inline unsigned long int GetTickTime();			///< 쓰레드에 안전한 틱 타이머 반환
protected:
	void InsertChatDBLog(const MUID& uidPlayer, const char* szMsg);
	int ValidateMakingName(const char* szCharName, int nMinLength, int nMaxLength);		// 등록가능한 이름인지 확인한다.

	int ValidateStageJoin(const MUID& uidPlayer, const MUID& uidStage);
	int ValidateChannelJoin(const MUID& uidPlayer, const MUID& uidChannel);
	int ValidateEquipItem(MMatchObject* pObj, MMatchItem* pItem, const MMatchCharItemParts parts);
	int ValidateChallengeLadderGame(MMatchObject** ppMemberObject, int nMemberCount);
	void CheckExpiredItems(MMatchObject* pObj);
	void ResponseExpiredItemIDList(MMatchObject* pObj, vector<unsigned long int>& vecExpiredItemIDList);

	bool LoadInitFile();
	bool LoadChannelPreset();
	bool InitDB();
	void UpdateServerLog();
	void UpdateServerStatusDB();

	void UpdateCharDBCachingData(MMatchObject* pObject);
protected:
	unsigned long GetItemFileChecksum()					{ return m_nItemFileChecksum; }
	void SetItemFileChecksum(unsigned long nChecksum)	{ m_nItemFileChecksum = nChecksum; }

	bool CheckItemXML();

protected :
	friend bool StageKick(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat);
	// fitler
	MCountryFilter& GetCountryFilter()					{ return m_CountryFilter; }
	bool InitCountryFilterDB();
	const CUSTOM_IP_STATUS	CheckIsValidCustomIP( const MUID& CommUID, const string& strIP, string& strCountryCode3, const bool bUseFilter );
	const COUNT_CODE_STATUS CheckIsNonBlockCountry( const MUID& CommUID, const string& strIP, string& strCountryCode3, const bool bUseFilter );

public :
	bool CheckUpdateItemXML();
	IPtoCountryList& GetTmpIPtoCountryList()			{ return m_TmpIPtoCountryList; }
	BlockCountryCodeList& GetTmpBlockCountryCodeList()	{ return m_TmpBlockCountryCodeList; }
	CustomIPList& GetTmpCustomIPList()					{ return m_TmpCustomIPList; }
	void SetUseCountryFilter();
	void SetAccetpInvalidIP();
	void UpdateIPtoCountryList();
	void UpdateBlockCountryCodeLsit();
	void UpdateCustomIPList();
	void ResetBlockCount()								{ m_dwBlockCount = 0; }
	void ResetNonBlockCount()							{ m_dwNonBlockCount = 0; }
	void IncreaseBlockCount()							{ ++m_dwBlockCount = 0; }
	void IncreaseNonBlockCount()						{ ++m_dwNonBlockCount = 0; }
	DWORD GetBlockCount()								{ return m_dwBlockCount; }
	DWORD GetNonBlockCount()							{ return m_dwNonBlockCount; }
	bool CheckIsValidIP( const MUID& CommUID, const string& strIP, string& strCountryCode3, const bool bUseFilter );
	// filter

public :
	void CustomCheckEventObj( const DWORD dwEventID, MMatchObject* pObj, void* pContext );

friend bool StageKick(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat);

public:
	MLadderMgr*	GetLadderMgr()				{ return &m_LadderMgr; }
	MMatchObjectList*	GetObjects()		{ return &m_Objects; }
	MMatchStageMap*		GetStageMap()		{ return &m_StageMap; }
	MMatchChannelMap*	GetChannelMap()		{ return &m_ChannelMap; }
	MMatchClanMap*		GetClanMap()		{ return &m_ClanMap; }
	IDatabase*			GetDBMgr()			{ return reinterpret_cast<IDatabase*>(&Database.u); }
	MMatchQuest*		GetQuest()			{ return &m_Quest; }
	int GetClientCount()	{ return (int)m_Objects.size(); }
	int GetAgentCount()		{ return (int)m_AgentMap.size(); }

	virtual void	XTrap_RandomKeyGenW(char* strKeyValue) {}
	virtual int		XTrap_XCrackCheckW(char* strSerialKey, char* strRandomValue, char* strHashValue) { return 0; }
	virtual void	XTrap_OnAdminReloadFileHash(const MUID& uidAdmin);

	virtual ULONG	HShield_MakeGuidReqMsg(unsigned char *pbyGuidReqMsg, unsigned char *pbyGuidReqInfo) { return 0L; }
	virtual ULONG	HShield_AnalyzeGuidAckMsg(unsigned char *pbyGuidAckMsg, unsigned char *pbyGuidReqInfo, unsigned long **ppCrcInfo) { return 0L; }
	virtual ULONG   HShield_MakeReqMsg(unsigned long *pCrcInfo, unsigned char *pbyReqMsg, unsigned char *pbyReqInfo, unsigned long ulOption) { return 0L; }
	virtual ULONG   HShield_AnalyzeAckMsg(unsigned long *pCrcInfo, unsigned char *pbyAckMsg, unsigned char *pbyReqInfo) { return 0L; }

	void SendHShieldReqMsg();

	void PostDeath(const MMatchObject& Victim, const MMatchObject& Attacker);
	void PostDamage(const MUID& Target, const MUID& Sender, ZDAMAGETYPE DamageType, MMatchWeaponType WeaponType,
		int Damage, float PiercingRatio);
	void PostHPAPInfo(const MMatchObject& Object, int HP, int AP);
};

void CopyCharInfoForTrans(MTD_CharInfo* pDest, MMatchCharInfo* pSrc, MMatchObject* pSrcObject);
void CopyCharInfoDetailForTrans(MTD_CharInfo_Detail* pDest, MMatchCharInfo* pSrcCharInfo, MMatchObject* pSrcObject);


// line functions ///////////////////////////////////////////////////////////////////
inline MMatchServer* MGetMatchServer()
{
	return MMatchServer::GetInstance();
}

inline unsigned long int MMatchServer::GetTickTime()
{ 
	m_csTickTimeLock.Lock();
	unsigned long int ret = m_nTickTime;
	m_csTickTimeLock.Unlock();
	return ret;
}

inline void MMatchServer::SetTickTime(unsigned long int nTickTime)
{
	m_csTickTimeLock.Lock();		
	m_nTickTime = nTickTime;
	m_csTickTimeLock.Unlock();
}

inline const char* MErrStr(const int nID)
{
	return MGetStringResManager()->GetErrorStr(nID);
}

bool IsExpiredBlockEndTime( const SYSTEMTIME& st );

void _CheckValidPointer(void* pPointer1, void* pPointer2, void* pPointer3, int nState, int nValue);
#define CheckValidPointer(A, B)		_CheckValidPointer(m_pMessengerManager, m_pScheduler, m_pAuthBuilder, A, B);CheckMemoryTest(A, B);
#define CheckValidPointer2(A, B)	MMatchServer::GetInstance()->CheckMemoryTest(A, B);

#endif