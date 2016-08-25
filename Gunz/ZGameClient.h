#ifndef _ZGAMECLIENT_H
#define _ZGAMECLIENT_H

#include "ZPrerequisites.h"
#include "MMatchClient.h"
#include "MSharedCommandTable.h"
#include "ZChannelRule.h"
#include "ZGame.h"
#include "ZNetAgreementBuilder.h"
#include "MEmblemMgr.h"

#include "SafeString.h"

typedef bool(ZONCOMMANDCALLBACK)(MCommand* pCommand);
class MListBox;
class ZCharacterViewList;
class UPnP;


class ZGameClient : public MMatchClient
{
protected:
	char				m_szChannel[256];
	char				m_szChannelRule[128];
	char				m_szStageName[256];
	char				m_szChatRoomInvited[64];
	unsigned int		m_nRoomNo;
	int					m_nStageCursor;
	bool				m_bLadderGame;
	MCHANNEL_TYPE		m_CurrentChannelType;
	char				m_szVoteText[256];

private:
	unsigned long int		m_nPrevClockRequestAttribute;
	
	int						m_nBridgePeerCount;
	unsigned long int		m_tmLastBridgePeer;

	int						m_nCountdown;
	unsigned long int		m_tmLastCountdown;

	int						m_nRequestID;			///< 클랜 생성등에서 쓰이는 RequestID
	MUID					m_uidRequestPlayer;		///< 클랜 생성등의 요청자 
	ZNetAgreementBuilder	m_AgreementBuilder;		///< 동의빌더
	MMatchProposalMode		m_nProposalMode;

	bool					m_bVoteInProgress;		///< 투표가 진행중인가
	bool					m_bCanVote;				///< 투표권이 있는가

	MEmblemMgr				m_EmblemMgr;

	bool					m_bPriorityBoost;		///< 성능최대화
	bool					m_bRejectNormalChat;	///< 일반챗 허용
	bool					m_bRejectTeamChat;		///< 팀챗 허용
	bool					m_bRejectClanChat;		///< 클랜챗 허용
	bool					m_bRejectWhisper;		///< 귓말허용
	bool					m_bRejectInvite;		///< 초대허용

	int PingToServer = 0;

protected:
	void SetChannelRuleName(const char* pszName)	{ strcpy_safe(m_szChannelRule, pszName); }
	int GetBridgePeerCount()			{ return m_nBridgePeerCount; }
	void SetBridgePeerCount(int nCount)	{ m_nBridgePeerCount = nCount; }
	void UpdateBridgePeerTime(unsigned long int nClock)	{ m_tmLastBridgePeer = nClock; }
	void StartBridgePeer();

	bool JustJoinedStage = false;
	MSTAGE_SETTING_NODE LastStageSetting;

	void UpdateStageSetting(MSTAGE_SETTING_NODE* pSetting, STAGE_STATE nStageState, const MUID& uidMaster);
	void SetCountdown(int nCountdown)	{ m_nCountdown = nCountdown; m_tmLastCountdown = 0; }
	int GetCountdown()					{ return m_nCountdown; }
	bool Countdown(int nClock) {
		if (nClock - m_tmLastCountdown > 1000) {
			m_nCountdown--;
			m_tmLastCountdown = nClock;
			return true;
		}
		return false;
	}

	static int FindListItem(MListBox* pListBox, const MUID& uid);

protected:
	ZONCOMMANDCALLBACK*		m_fnOnCommandCallback;
	
	bool					m_bIsBigGlobalClock;
	unsigned long int		m_nClockDistance;

	MMatchStageSetting		m_MatchStageSetting;
	bool					m_bForcedEntry;
protected:
	virtual bool OnCommand(MCommand* pCommand);
	virtual bool OnSockDisconnect(SOCKET sock);
	virtual bool OnSockConnect(SOCKET sock);
	virtual void OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode);
	virtual int OnConnected(SOCKET sock, MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp);
	virtual void OnRegisterCommand(MCommandManager* pCommandManager);
	virtual void OnPrepareCommand(MCommand* pCommand);
	virtual int OnResponseMatchLogin(const MUID& uidServer, int nResult, const char* szServerName,
		const MMatchServerMode nServerMode,
		const char* szAccountID, const MMatchUserGradeID nUGradeID,
		const MMatchPremiumGradeID nPGradeID, const MUID& uidPlayer,
		const char* szRandomValue, unsigned char* szEncryptMsg);
	virtual void OnBridgePeerACK(const MUID& uidChar, int nCode);
	virtual void OnObjectCache(unsigned int nType, void* pBlob, int nCount);
	virtual void OnAgentError(int nError);
protected:
	void OnMatchNotify(unsigned int nMsgID);
	void OnAnnounce(unsigned int nType, char* szMsg);
	void OnResponseResult(const int nResult);

	void OnChannelResponseJoin(const MUID& uidChannel, MCHANNEL_TYPE nChannelType, char* szChannelName);
	void OnChannelChat(const MUID& uidChannel, char* szName, char* szChat,int nGrade);
	void OnChannelList(void* pBlob, int nCount);
	void OnChannelResponseRule(const MUID& uidchannel, const char* pszRuleName);

	void OnLadderPrepare(const MUID& uidStage, const int nTeam);
	void OnLadderLaunch(const MUID& uidStage, const char* pszMapName);
	void OnLadderResponseChallenge(const int nResult);

	void OnStageJoin(const MUID& uidChar, const MUID& uidStage, unsigned int nRoomNo, char* szStageName);
	void OnStageLeave(const MUID& uidChar, const MUID& uidStage);
	void OnStageStart(const MUID& uidChar, const MUID& uidStage, int nCountdown);
	void OnStageLaunch(const MUID& uidStage, const char* pszMapName);
	void OnStageFinishGame(const MUID& uidStage);
	void OnStageMap(const MUID& uidStage, char* szMapName);
	void OnStageTeam(const MUID& uidChar, const MUID& uidStage, unsigned int nTeam);
	void OnStagePlayerState(const MUID& uidChar, const MUID& uidStage, MMatchObjectStageState nStageState);
	void OnStageMaster(const MUID& uidStage, const MUID& uidChar);
	void OnStageChat(const MUID& uidChar, const MUID& uidStage, char* szChat);
	void OnStageList(int nPrevStageCount, int nNextStageCount, void* pBlob, int nCount);
	void OnResponseFriendList(void* pBlob, int nCount);
	void OnChannelPlayerList(int nTotalPlayerCount, int nPage, void* pBlob, int nCount);
	void OnChannelAllPlayerList(const MUID& uidChannel, void* pBlob, int nBlobCount);
	void OnResponseStageSetting(const MUID& uidStage, void* pStageBlob, int nStageCount, void* pCharBlob, 
		                        int nCharCount, STAGE_STATE nStageState, const MUID& uidMaster);
	void OnResponseRecommandedChannel(const MUID& uidChannel);
	void OnResponsePeerRelay(const MUID& uidPeer);
	void OnResponseGameInfo(const MUID& uidStage, void* pGameInfoBlob, void* pRuleInfoBlob, void* pPlayerInfoBlob);
	void OnResponseCharInfoDetail(void* pBlob);

	void OnLoadingComplete(const MUID& uidChar, int nPercent);
	void OnForcedEntryToGame();

	void OnUserWhisper(char* pszSenderName, char* pszTargetName, char* pszMessage);
	void OnChatRoomJoin(char* pszPlayerName, char* pszChatRoomName);
	void OnChatRoomLeave(char* pszPlayerName, char* pszChatRoomName);
	void OnChatRoomSelectWrite(char* pszChatRoomName);
	void OnChatRoomInvite(char* pszSenderName, char* pszRoomName);
	void OnChatRoomChat(char* pszChatRoomName, char* pszPlayerName, char* pszChat);

	void OnFollowResponse( const int nMsgID );
	void OnExpiredRentItem(void* pBlob);

	void OnBirdTest();
protected:
	void OnResponseCreateClan(const int nResult, const int nRequestID);
	void OnResponseAgreedCreateClan(const int nResult);
	void OnClanAskSponsorAgreement(const int nRequestID, const char* szClanName, MUID& uidMasterObject, const char* szMasterName);
	void OnClanAnswerSponsorAgreement(const int nRequestID, const MUID& uidClanMaster, char* szSponsorCharName, const bool bAnswer);
	void OnClanResponseCloseClan(const int nResult);
	void OnClanResponseJoinClan(const int nResult);
	void OnClanAskJoinAgreement(const char* szClanName, MUID& uidClanAdmin, const char* szClanAdmin);
	void OnClanAnswerJoinAgreement(const MUID& uidClanAdmin, const char* szJoiner, const bool bAnswer);
	void OnClanResponseAgreedJoinClan(const int nResult);
	void OnClanUpdateCharClanInfo(void* pBlob);
	void OnClanResponseLeaveClan(const int nResult);
	void OnClanResponseChangeGrade(const int nResult);
	void OnClanResponseExpelMember(const int nResult);
	void OnClanMsg(const char* szSenderName, const char* szMsg);
	void OnClanMemberList(void* pBlob);
	void OnClanResponseClanInfo(void* pBlob);
	void OnClanStandbyClanList(int nPrevStageCount, int nNextStageCount, void* pBlob);
	void OnClanMemberConnected(const char* szMember);

	void OnResponseProposal(const int nResult, const MMatchProposalMode nProposalMode, const int nRequestID);
	void OnAskAgreement(const MUID& uidProposer, 
		                void* pMemberNamesBlob, 
						const MMatchProposalMode nProposalMode, 
						const int nRequestID);
	void OnReplyAgreement(const MUID& uidProposer, 
		                  const MUID& uidChar, 
						  const char* szReplierName, 
						  const MMatchProposalMode nProposalMode,
					      const int nRequestID, 
						  const bool bAgreement);
	void ReplyAgreement(const MUID& uidProposer, const MMatchProposalMode nProposalMode, bool bAgreement);
protected:
	void OnGameLevelUp(const MUID& uidChar);
	void OnGameLevelDown(const MUID& uidChar);

public:
	void OnSpawnWorldItem(void* pBlob);
	void OnObtainWorldItem(const MUID& uidChar, const int nItemUID);
	void OnRemoveWorldItem(const int nItemUID);
protected:
	void OnLocalReport119();
protected:
	void OnAdminAnnounce(const char* szMsg, const ZAdminAnnounceType nType);
public:
	bool CreatedStage = false;
	MTD_ClientSettings ClientSettings;

	ZGameClient();
	virtual ~ZGameClient();

	void PriorityBoost(bool bBoost);
	bool GetPriorityBoost()				{ return m_bPriorityBoost; }
	bool GetRejectNormalChat()			{ return m_bRejectNormalChat; }
	void SetRejectNormalChat(bool bVal)	{ m_bRejectNormalChat = bVal; }
	bool GetRejectTeamChat()			{ return m_bRejectTeamChat; }
	void SetRejectTeamChat(bool bVal)	{ m_bRejectTeamChat = bVal; }
	bool GetRejectClanChat()			{ return m_bRejectClanChat; }
	void SetRejectClanChat(bool bVal)	{ m_bRejectClanChat = bVal; }
	bool GetRejectWhisper()				{ return m_bRejectWhisper; }
	void SetRejectWhisper(bool bVal)	{ m_bRejectWhisper = bVal; }
	bool GetRejectInvite()				{ return m_bRejectInvite; }
	void SetRejectInvite(bool bVal)		{ m_bRejectInvite = bVal; }

	unsigned long int GetClockCount(void) { return GetGlobalTimeMS(); }
	unsigned long int GetGlobalClockCount(void);

	virtual void OutputMessage(const char* szMessage, MZMOMType nType=MZMDM_GENERAL);
	
	void SetOnCommandCallback(ZONCOMMANDCALLBACK pCallback) { m_fnOnCommandCallback = pCallback;}

	void Tick(void);
	void Disconnect()							{ MMatchClient::Disconnect(m_Server); }
	
	MMatchStageSetting* GetMatchStageSetting() { return &m_MatchStageSetting; }
	bool IsForcedEntry() { return m_bForcedEntry; }
	bool IsLadderGame() { return m_bLadderGame; }
	void ReleaseForcedEntry();
	void ClearStageSetting();
public:
	void RequestPrevStageList();
	void RequestNextStageList();
	void RequestStageList(int nPage);
	void StartStageList();
	void StopStageList();
	void StartChannelList(MCHANNEL_TYPE nChannelType);
	void StopChannelList();

	const char*		GetChannelName()	{ return m_szChannel; }
	MCHANNEL_TYPE	GetChannelType()	{ return m_CurrentChannelType; }
	const char*		GetChannelRuleName(){ return m_szChannelRule; }
	const char*		GetStageName()		{ return m_szStageName; }
	int				GetStageNumber()	{ return m_nRoomNo; }
	
	const char* GetChatRoomInvited(){ return m_szChatRoomInvited; }
	void SetChatRoomInvited(const char* pszRoomName)	{ strcpy_safe(m_szChatRoomInvited, pszRoomName); }

	bool AmIStageMaster() { return (m_MatchStageSetting.GetMasterUID() == GetPlayerUID()); }

	const char* GetVoteMessage() { return m_szVoteText; }
public:
	void AnswerSponsorAgreement(bool bAnswer);
	void AnswerJoinerAgreement(bool bAnswer);
	void RequestCreateClan(char* szClanName, char** ppMemberCharNames);

	void RequestProposal(const MMatchProposalMode nProposalMode, char** ppReplierCharNames, const int nReplierCount);
	void ReplyAgreement(bool bAgreement);
public:
	bool IsVoteInProgress()				{ return m_bVoteInProgress;	}
	void SetVoteInProgress(bool bVal)	{ m_bVoteInProgress = bVal; }
	bool CanVote()						{ return m_bCanVote; }
	void SetCanVote(bool bVal)			{ m_bCanVote = bVal; }

public:
	void RequestGameSuicide();
	void OnStageEnterBattle(const MUID& uidChar, MCmdEnterBattleParam nParam, MTD_PeerListNode* pPeerNode);
public:
	int ValidateRequestDeleteChar();
public:
	void RequestChannelJoin(const MUID& uidChannel);
	void RequestChannelJoin(const MCHANNEL_TYPE nChannelType, char* szChannelName);
	void RequestOnLobbyCreated();
	void RequestOnGameDestroyed();
protected:
	void OnNotifyCallVote(const char* pszDiscuss, const char* pszArg);
	void OnNotifyVoteResult(const char* pszDiscuss, int nResult);
	void OnVoteAbort( const int nMsgCode );
protected:
	void OnBroadcastClanRenewVictories(const char* pszWinnerClanName, const char* pszLoserClanName, int nVictories);
	void OnBroadcastClanInterruptVictories(const char* pszWinnerClanName, const char* pszLoserClanName, int nVictories);
	void OnBroadcastDuelRenewVictories(const char* pszChampionName, const char* pszChannelName, int nRoomno, int nVictories);
	void OnBroadcastDuelInterruptVictories(const char* pszChampionName, const char* pszInterrupterName, int nVictories);
protected:
	// Emblem 관련
	void ProcessEmblem(unsigned int nCLID, unsigned int nChecksum);
	void RequestEmblemURL(unsigned int nCLID);
	void OnClanResponseEmblemURL(unsigned int nCLID, unsigned int nEmblemChecksum, const char* szEmblemURL);
	void OnClanEmblemReady(unsigned int nCLID, const char* szURL);

public:
	MEmblemMgr *GetEmblemManager() { return &m_EmblemMgr; }	

	// UPnP
protected:
	UPnP *m_pUPnP;

public:
	bool CreateUPnP(unsigned short nUDPPort);
	bool DestroyUPnP();

	int GetPingToServer() const { return PingToServer; }

protected:
	virtual void OnStopUDPTest(const MUID& uid) override;
	virtual void OnUDPTestReply(const MUID& uid) override;
};



bool ZPostCommand(MCommand* pCmd);
MUID ZGetMyUID();	

MCommand* ZNewCmd(int nID);

unsigned long int ZGetClockDistance(unsigned long int nGlobalClock, unsigned long int nLocalClock);


// Post Command Macro For Convenience
#define ZPOSTCMD0(_ID)									{ MCommand* pC=ZNewCmd(_ID); ZPostCommand(pC); }
#define ZPOSTCMD1(_ID, _P0)								{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); ZPostCommand(pC); }
#define ZPOSTCMD2(_ID, _P0, _P1)						{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); pC->AddParameter(new _P1); ZPostCommand(pC); }
#define ZPOSTCMD3(_ID, _P0, _P1, _P2)					{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); pC->AddParameter(new _P1); pC->AddParameter(new _P2); ZPostCommand(pC); }
#define ZPOSTCMD4(_ID, _P0, _P1, _P2, _P3)				{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); pC->AddParameter(new _P1); pC->AddParameter(new _P2); pC->AddParameter(new _P3); ZPostCommand(pC); }
#define ZPOSTCMD5(_ID, _P0, _P1, _P2, _P3, _P4)			{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); pC->AddParameter(new _P1); pC->AddParameter(new _P2); pC->AddParameter(new _P3); pC->AddParameter(new _P4); ZPostCommand(pC); }
#define ZPOSTCMD6(_ID, _P0, _P1, _P2, _P3, _P4, _P5)	{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); pC->AddParameter(new _P1); pC->AddParameter(new _P2); pC->AddParameter(new _P3); pC->AddParameter(new _P4); pC->AddParameter(new _P5); ZPostCommand(pC); }
#define ZPOSTCMD7(_ID, _P0, _P1, _P2, _P3, _P4, _P5, _P6)	{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); pC->AddParameter(new _P1); pC->AddParameter(new _P2); pC->AddParameter(new _P3); pC->AddParameter(new _P4); pC->AddParameter(new _P5); pC->AddParameter(new _P6); ZPostCommand(pC); }

#define HANDLE_COMMAND(message, fn)    \
	case (message): return fn(pCommand);

bool GetUserInfoUID(MUID uid,MCOLOR& _color,char* sp_name,MMatchUserGradeID& gid);

#endif