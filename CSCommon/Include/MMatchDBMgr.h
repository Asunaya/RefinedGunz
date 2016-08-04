#ifndef MMATCHDBMGR_H
#define MMATCHDBMGR_H

#include "ODBCRecordset.h"
#include "MMatchItem.h"
#include "MMatchGlobal.h"
#include "MQuestItem.h"
#include "MQuestConst.h"
#include "MMatchDBFilter.h"
#include "MCountryFilterDBMgr.h"
#include "IDatabase.h"

class MMatchCharInfo;
class MMatchFriendInfo;
struct MMatchAccountInfo;
struct MTD_CharInfo;
struct MTD_AccountCharInfo;

class MMatchDBMgr {
protected:
	MDatabase	m_DB;
	CString		m_strDSNConnect;

	MCountryFilterDBMgr m_CountryFilterDBMgr;

	MMatchDBFilter m_DBFilter;

	bool CheckOpen();
	void Log(const char *pFormat,...);
public:
	MMatchDBMgr();
	virtual ~MMatchDBMgr();

	MDatabase* GetDatabase()	{ return &m_DB; }

	CString BuildDSNString(const CString strDSN, const CString strUserName, const CString strPassword);
	bool Connect(CString strDSNConnect);
	void Disconnect();

	static void LogCallback( const string& strLog );
private:
	bool UpdateCharLevel(const int nCID, const int nLevel);
	bool InsertLevelUpLog(const int nCID, const int nLevel, const int nBP, 
				const int nKillCount, const int nDeathCount, const int nPlayTime);
public:
	template <size_t size>
	bool GetLoginInfo(const TCHAR* szUserID, unsigned int* poutnAID, TCHAR(&poutPassword)[size]) {
		return GetLoginInfo(szUserID, poutnAID, poutPassword, size);
	}
	bool GetLoginInfo(const TCHAR* szUserID, unsigned int* poutnAID, TCHAR* poutPassword, int maxlen);

	bool UpdateLastConnDate(const TCHAR* szUserID, const TCHAR* szIP);

	bool CreateAccount(const TCHAR* szUserID,					// 사용자ID
					   const TCHAR* szPassword,					// 패스워드
					   const int nCert,							// 실명인증
					   const TCHAR* szName,						// 사용자이름
					   const int nAge,							// 나이
					   const int nSex);							// 성별 0 - 여자, 1 - 남자

	AccountCreationResult CreateAccountNew(const char *szUsername, const char *szPasswordData, const char *szEmail);

	bool BanPlayer(int nAID, const char *szReason, const time_t &UnbanTime);

	bool CreateCharacter(int* pnOutResult,						// 결과값
						 const int nAID,						// AID
						 const TCHAR* szNewName,				// 캐릭터 이름
						 const int nCharIndex,					// 계정의 캐릭 슬롯 인덱스
		                 const int nSex,						// 성별
						 const int nHair,						// 헤어
						 const int nFace,						// 얼굴
						 const int nCostume);					// 초기 코스튬
	bool DeleteCharacter(const int nAID,
		                 const int nCharIndex,
						 const TCHAR* szCharName);
//	bool GetAccountCharList(const int nAID, 
//		                    MTD_CharInfo* poutCharList, 
//							int* noutCharCount);
	bool GetAccountCharList(const int nAID, 
		                    MTD_AccountCharInfo* poutCharList, 
							int* noutCharCount);
	bool GetAccountCharInfo(const int nAID, const int nCharIndex, MTD_CharInfo* poutCharInfo);
	bool GetAccountInfo(const unsigned long int nAID,
		                MMatchAccountInfo* poutAccountInfo);

	bool GetCharInfoByAID(const int nAID, 
		                  const int nCharIndex, 
						  MMatchCharInfo* poutCharInfo,
						  int& nWaitHourDiff );

	bool GetCharCID(const TCHAR* pszName, int* poutCID);

	bool SimpleUpdateCharInfo(MMatchCharInfo* pCharInfo);

	// 게임 플레이 관련
	bool UpdateCharBP(const int nCID, const int nBPInc);

	bool UpdateCharInfoData(const int nCID, const int nAddedXP, const int nAddedBP, 
							const int nAddedKillCount, const int nAddedDeathCount);

	// 아이템 관련
	bool InsertCharItem(const unsigned int nCID, int nItemDescID, bool bRentItem, int nRentPeriodHour, unsigned long int* poutCIID);
	bool DeleteCharItem(const unsigned int nCID, int nCIID);
	bool GetCharItemInfo(MMatchCharInfo* pCharInfo);	// 캐릭터의 모든 아이템을 가져온다
	bool GetAccountItemInfo(const int nAID, MAccountItemNode* pOut, int* poutNodeCount, int nMaxNodeCount,
							MAccountItemNode* pOutExpiredItemList, int* poutExpiredItemCount, int nMaxExpiredItemCount);			// 창고 아이템을 가져온다
	bool UpdateEquipedItem(const unsigned long nCID, 
							MMatchCharItemParts parts, 
							unsigned long int nCIID, 
							const unsigned long int nItemID, 
							bool* poutRet);
	bool ClearAllEquipedItem(const unsigned long nCID);
	bool DeleteExpiredAccountItem(const int nAIID);
	bool BuyBountyItem(const unsigned int nCID, int nItemID, int nPrice, unsigned long int* poutCIID);
	bool SellBountyItem(const unsigned int nCID, unsigned int nItemID, unsigned int nCIID, int nPrice, int nCharBP);


	// Quest Item관련.
	// bool UpdateQuestItem( MMatchCharInfo* pCharInfo );
	bool UpdateQuestItem( int nCID, MQuestItemMap& rfQuestIteMap, MQuestMonsterBible& rfQuestMonster );
	bool GetCharQuestItemInfo( MMatchCharInfo* pCharInfo );
	/*
	bool InsertQuestGameLog( const char* pszStageName, const int nMasterCID, const char* pszMap, 
							 const int nQL, const int nPlayerCount, const char* pszPlayers, 
							 const char* pszSacriSlot, const char* pszQuestItems );
							 */
	bool InsertQuestGameLog( const char* pszStageName, 
							 const int nScenarioID,
							 const int nMasterCID, const int nPlayer1, const int nPlayer2, const int nPlayer3,
							 const int nTotalRewardQItemCount,
							 const int nElapsedPlayTime,
							 int& outQGLID );

	bool InsertQUniqueGameLog( const int nQGLID, const int nCID, const int nQIID );
							 

	// 로그 관련
	// bool InsertConnLog(const int nAID, const char* szUserID, const char* szIP);
	bool InsertConnLog(const int nAID, const char* szIP, const string& strCountryCode3 );
	bool InsertGameLog(const char* szGameName, const char* szMap, const char* GameType,
					   const int nRound, const unsigned int nMasterCID,
					   const int nPlayerCount, const char* szPlayers);
	bool InsertKillLog(const unsigned int nAttackerCID, const unsigned int nVictimCID);
	bool InsertChatLog(const unsigned long int nCID, const char* szMsg, unsigned long int nTime);
	bool InsertServerLog(const int nServerID, const int nPlayerCount, const int nGameCount, const DWORD dwBlockCount, const DWORD dwNonBlockCount);
	bool InsertPlayerLog(const unsigned long int nCID,
				const int nPlayTime, const int nKillCount, const int nDeathCount, const int nXP, const int nTotalXP);

	bool UpdateServerStatus(const int nServerID, const int nPlayerCount);
	bool UpdateMaxPlayer(const int nServerID, const int nMaxPlayer);
	bool UpdateServerInfo(const int nServerID, const int nMaxPlayer, const char* szServerName);
	bool UpdateCharPlayTime(const unsigned long int nCID, const unsigned long int nPlayTime);

	enum _ITEMPURCHASE_TYPE
	{
		IPT_BUY = 0,
		IPT_SELL = 1,
	};

	bool InsertItemPurchaseLogByBounty(const unsigned long int nItemID, const unsigned long int nCID,
		const int nBounty, const int nCharBP, const _ITEMPURCHASE_TYPE nType);

	enum _CHARMAKING_TYPE
	{
		CMT_CREATE = 0,
		CMT_DELETE = 1
	};
	bool InsertCharMakingLog(const unsigned int nAID, const char* szCharName,
							const _CHARMAKING_TYPE nType);

	// 중앙은행에서 내 캐쉬 아이템 가져오기
	bool BringAccountItem(const int nAID, const int nCID, const int nAIID, 
							unsigned long int* poutCIID, unsigned long int* poutItemID,
							bool* poutIsRentItem, int* poutRentMinutePeriodRemainder);
	// 중앙은행으로 내 캐쉬 아이템 넣기
	bool BringBackAccountItem(const int nAID, const int nCID, const int nCIID);

	//// Friends ////
	bool FriendAdd(const int nCID, const int nFriendCID, const int nFavorite);
	bool FriendRemove(const int nCID, const int nFriendCID);
	bool FriendGetList(const int nCID, MMatchFriendInfo* pFriendInfo);

	//// Clan //////
	template<size_t size>
	bool GetCharClan(const int nCID, int* poutClanID, TCHAR(&poutClanName)[size]) {
		return GetCharClan(nCID, poutClanID, poutClanName, size);
	}
	bool GetCharClan(const int nCID, int* poutClanID, TCHAR* poutClanName, int maxlen);
	bool GetClanIDFromName(const TCHAR* szClanName, int* poutCLID);
	bool CreateClan(const TCHAR* szClanName, const int nMasterCID, const int nMember1CID, const int nMember2CID,
		            const int nMember3CID, const int nMember4CID, bool* boutRet, int* noutNewCLID);
	bool DeleteExpiredClan( const DWORD dwCID, const DWORD dwCLID, const string& strDeleteName, const DWORD dwWaitHour = 24 );
	bool SetDeleteTime( const DWORD dwMasterCID, const DWORD dwCLID, const string& strDeleteDate );
	bool ReserveCloseClan(const int nCLID, const TCHAR* szClanName, const int nMasterCID, const string& strDeleteDate);
	bool AddClanMember(const int nCLID, const int nJoinerCID, const int nClanGrade, bool* boutRet);
	bool RemoveClanMember(const int nCLID, const int nLeaverCID);
	bool UpdateClanGrade(const int nCLID, const int nMemberCID, const int nClanGrade);
	bool ExpelClanMember(const int nCLID, const int nAdminGrade, TCHAR* szMember, int* noutRet);
	
	struct MDB_ClanInfo
	{
		int		nCLID;
		char	szClanName[CLAN_NAME_LENGTH];
		int		nLevel;
		int		nRanking;
		int		nPoint;
		int		nTotalPoint;
		int		nWins;
		int		nLosses;
		char	szMasterName[CLAN_NAME_LENGTH];
		int		nTotalMemberCount;
		char	szEmblemUrl[256];
		int		nEmblemChecksum;
	};
	bool GetClanInfo(const int nCLID, MDB_ClanInfo* poutClanInfo);
	bool UpdateCharClanContPoint(const int nCID, const int nCLID, const int nAddedContPoint);


	enum _EXPEL_RETURN
	{
		ER_OK =				1,		// 성공
		ER_NO_MEMBER =		0,		// 해당 클랜원이 없다
		ER_WRONG_GRADE =	2		// 권한이 맞지 않다.
	};


	/// Ladder ///////
	bool GetLadderTeamID(const int nTeamTableIndex, const int* pnMemberCIDArray, const int nMemberCount, int* pnoutTID);
	bool LadderTeamWinTheGame(const int nTeamTableIndex, const int nWinnerTID, const int nLoserTID, const bool bIsDrawGame,
		const int nWinnerPoint, const int nLoserPoint, const int nDrawPoint);
	template<size_t size>
	bool GetLadderTeamMemberByCID(const int nCID, int* poutTeamID, char *(&ppoutCharArray)[size], int nCount) {
		return GetLadderTeamMemberByCID(nCID, poutTeamID, ppoutCharArray, size, nCount);
	}
	bool GetLadderTeamMemberByCID(const int nCID, int* poutTeamID, char** ppoutCharArray, int maxlen, int nCount);


	/// 클랜전 //////
	bool WinTheClanGame(const int nWinnerCLID, const int nLoserCLID, const bool bIsDrawGame,
						const int nWinnerPoint, const int nLoserPoint, const char* szWinnerClanName,
						const char* szLoserClanName, const int nRoundWins, const int nRoundLosses,
						const int nMapID, const int nGameType,
						const char* szWinnerMembers, const char* szLoserMembers);


	// Util Func
	bool UpdateCharLevel(const int nCID, const int nNewLevel, const int nBP, const int nKillCount, 
		                 const int nDeathCount, const int nPlayTime, bool bIsLevelUp);

	// 이벤트 관련
	bool EventJjangUpdate(const int nAID, const bool bJjang);
	bool CheckPremiumIP(const char* szIP, bool& outbResult);

	// DB editor에서 사용하는 함수.
	bool GetCID( const char* pszCharName, int& outCID );
	bool GetCharName( const int nCID, string& outCharName );

	// Country filter.
	bool GetIPContryCode( const string& strIP, 
						  DWORD& dwOutIPFrom, 
						  DWORD& dwOutIPTo, 
						  string& strOutCountryCode );
	bool GetBlockCountryCodeList( BlockCountryCodeList& rfBlockCountryCodeList );
	bool GetIPtoCountryList( IPtoCountryList& rfIPtoCountryList );
	bool GetCustomIP( const string& strIP, DWORD& dwIPFrom, DWORD& dwIPTo, bool& bIsBlock, string& strCountryCode3, string& strComment );
	bool GetCustomIPList( CustomIPList& rfCustomIPList );

	// Event
	bool InsertEvent( const DWORD dwAID,  const DWORD dwCID, const string& strEventName );

	bool SetBlockAccount( const DWORD dwAID, 
						  const DWORD dwCID, 
						  const BYTE btBlockType, 
						  const string& strComment, 
						  const string& strIP,
						  const string& strEndHackBlockerDate );

	bool ResetAccountBlock( const DWORD dwAID, const BYTE btBlockType );

	bool InsertBlockLog( const DWORD dwAID, const DWORD dwCID, const BYTE btBlockType, const string& strComment,
		const string& strIP );

    // admin
	bool AdminResetAllHackingBlock();
};


#endif