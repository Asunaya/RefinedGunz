#include "stdafx.h"
#include "MMatchDBMgr.h"
#include "MMatchObject.h"
#include "afxwin.h"
#include "MMatchTransDataType.h"
#include "MMatchFriendInfo.h"
#include <stdio.h>
#include "MErrorTable.h"
#include "MDebug.h"
#include "MMatchStatus.h"
#include "MMatchFriendInfo.h"
#include "MMatchClan.h"
#include "mmsystem.h"
#include "MInetUtil.h"
#include "mcrc32.h"
#include "MQuestConst.h"
#include "MQuestItem.h"

#define SODIUM_STATIC
#include "sodium.h"

// SQL ////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// 계정 생성 - UserID, Password, UGradeID, PGradeID, Name, Email, RegNum, Sex, ZipCode, Address
// 바꾼자. - 넷마블 서버는 아직 안바꿨음, Cert컬럼도 추가해야함
//TCHAR g_szDB_CREATE_ACCOUNT[] = _T("{CALL spInsertAccount ('%s', '%s', %d, %d, '%s', '%s', '%s', %d, '%s', '%s')}");
// UserID, Password, Cert, Name, Age, Sex 
TCHAR g_szDB_CREATE_ACCOUNT[] = _T("{CALL spInsertAccount ('%s', '%s', %d, '%s', %d, %d)}");

// 계정 로그인 정보 받아오기
TCHAR g_szDB_GET_LOGININFO[] = _T("{CALL spGetLoginInfo ('%s')}");

// 계정 정보 받아오기
TCHAR g_szDB_GET_ACCOUNTINFO[] = _T("{CALL spGetAccountInfo (%d)}");

// 캐릭터 생성
TCHAR g_szDB_CREATE_CHAR[] = _T("{CALL spInsertChar (%d, %d, '%s', %d, %d, %d, %d)}");

// 캐릭터 삭제
TCHAR g_szDB_DELETE_CHAR[] = _T("{CALL spDeleteChar (%d, %d, '%s')}");

// 캐릭터 리스트 받아오기
TCHAR g_szDB_GET_CHARLIST[] = _T("{CALL spGetCharList (%d)}");

// 계정 캐릭터 정보 받아오기
TCHAR g_szDB_GET_ACCOUNT_CHARINFO[] = _T("{CALL spGetAccountCharInfo (%d, %d)}");

// 캐릭터 정보 받아오기
TCHAR g_szDB_GET_CHARINFO_BY_AID[] = _T("{CALL spGetCharInfoByCharNum (%d, %d)}");

// 캐릭터 정보 업데이트
TCHAR g_szDB_SIMPLE_UPDATE_CHARINFO[] = _T("{CALL spSimpleUpdateChar (%d, '%s', %d, %d, %d)}");

// 캐릭터 아이템 추가
TCHAR g_szDB_INSERT_CHAR_ITEM[] = _T("{CALL spInsertCharItem (%d, %d, %d)}");

// 캐릭터 아이템 삭제
TCHAR g_szDB_DELETE_CHAR_ITEM[] = _T("{CALL spDeleteCharItem (%d, %d)}");

// 캐릭터 아이템 정보 가져오기
TCHAR g_szDB_SELECT_CHAR_ITEM[] = _T("{CALL spSelectCharItem (%d)}");

// 캐릭터의 창고 아이템 가져오기
TCHAR g_szDB_SELECT_ACCOUNT_ITEM[] = _T("{CALL spSelectAccountItem (%d)}");

// 캐릭터 장비 아이템 정보 가져오기
//TCHAR g_szDB_SELECT_CHAR_EQUIPED_ITEM[] = _T("{CALL spSelectCharEquipedItemCIID (%d)}");

// 상점에서 아이템 구매
TCHAR g_szDB_BUY_BOUNTY_ITEM[] = _T("{CALL spBuyBountyItem (%d, %d, %d)}");

// 상점에서 아이템 판매
TCHAR g_szDB_SELL_BOUNTY_ITEM[] = _T("{CALL spSellBountyItem (%d, %d, %d, %d, %d)}");


// 캐릭터 아이템 장비하기
TCHAR g_szDB_UPDATE_EQUIPITEM[] = _T("{CALL spUpdateEquipItem (%d, %d, %d, %d)}");

// 접속 로그
// TCHAR g_szDB_INSERT_CONN_LOG[] = _T("{CALL spInsertConnLog (%d, '%s', '%s')}");
TCHAR g_szDB_INSERT_CONN_LOG[] = _T("{CALL spInsertConnLog (%d, %d, %d, %d, %d, '%s')}");

// 게임 로그
TCHAR g_szDB_INSERT_GAME_LOG[] = _T("{CALL spInsertGameLog ('%s', '%s', '%s', %d, %d, %d, '%s')}");

// 킬 로그
TCHAR g_szDB_INSERT_KILL_LOG[] = _T("{CALL spInsertKillLog (%d, %d)}");

// 바운티로 아이템 구입 로그
TCHAR g_szDB_INSERT_ITEM_PURCHASE_BY_BOUNTY_LOG[] = _T("{CALL spInsertItemPurchaseLogByBounty (%d, %d, %d, %d, '%s')}");

// 캐릭터 생성 로그
TCHAR g_szDB_INSERT_CHAR_MAKING_LOG[] = _T("{CALL spInsertCharMakingLog (%d, '%s', '%s')}");

// 서버 로그
TCHAR g_szDB_INSERT_SERVER_LOG[] = _T("{CALL spInsertServerLog (%d, %d, %d, %u, %u)}");

// 서버 동접자 상태
//TCHAR g_szDB_UPDATE_SERVER_STATUS[] = _T("UPDATE ServerStatus Set CurrPlayer=%d, Time=GETDATE() WHERE ServerID=%d");
// 인자: CurrPlayer, ServerID
TCHAR g_szDB_UPDATE_SERVER_STATUS[] = _T("{CALL spUpdateServerStatus (%d, %d)}");

// 서버 최대 인원 업데이트
TCHAR g_szDB_UPDATE_SERVER_INFO[] = _T("UPDATE ServerStatus Set MaxPlayer=%d, ServerName='%s' WHERE ServerID=%d");

// 플레이어 로그
TCHAR g_szDB_INSERT_PLAYER_LOG[] = _T("{CALL spInsertPlayerLog (%d, %d, %d, %d, %d, %d)}");

// 레벨업 로그
TCHAR g_szDB_INSERT_LEVELUP_LOG[] = _T("{CALL spInsertLevelUpLog (%d, %d, %d, %d, %d, %d)}");


// 레벨 업데이트
//TCHAR g_szDB_UPDATE_CHAR_LEVEL[] = _T("UPDATE Character Set Level=%d WHERE CID=%d");
// 인자: Level, CID
TCHAR g_szDB_UPDATE_CHAR_LEVEL[] = _T("{CALL spUpdateCharLevel (%d, %d)}");

// BP 업데이트
//TCHAR g_szDB_UPDATE_CHAR_BP[] = _T("UPDATE Character SET BP=BP+(%d) WHERE CID=%d");
// 인자: BPInc, CID
TCHAR g_szDB_UPDATE_CHAR_BP[] = _T("{CALL spUpdateCharBP (%d, %d)}");

// 캐릭터 정보(XP, BP, KillCount, DeathCount) 업데이트
//TCHAR g_szDB_UPDATE_CHAR_INFO_DATA[] = _T("UPDATE Character SET XP=XP+(%d), BP=BP+(%d), KillCount=KillCount+(%d), DeathCount=DeathCount+(%d) WHERE CID=%d");
// 인자: XPInc, BPInc, KillInc, DeathInc, CID
TCHAR g_szDB_UPDATE_CHAR_INFO_DATA[] = _T("{CALL spUpdateCharInfoData (%d, %d, %d, %d, %d)}");

// 캐릭터 플레이 시간 업데이트
//TCHAR g_szDB_UPDATE_CHAR_PLAYTIME[] = _T("UPDATE Character SET PlayTime=PlayTime+(%d), LastTime=GETDATE() WHERE CID=%d");
// 인자: PlayTimeInc, CID
TCHAR g_szDB_UPDATE_CHAR_PLAYTIME[] = _T("{CALL	spUpdateCharPlayTime (%d, %d)}");

// 접속시간, IP저장
// 인자: IP, 넷마블UserID
TCHAR g_szDB_UPDATE_LAST_CONNDATE[] = _T("{CALL	spUpdateLastConnDate ('%s', '%s')}");

// 중앙은행 아이템 내 캐릭터로 가져오기
// 인자: AID, CID, AIID
TCHAR g_szDB_BRING_ACCOUNTITEM[] = _T("{CALL spBringAccountItem (%d, %d, %d)}");

// 중앙은행에 내 캐쉬아이템 넣기
// 인자: AID, CID, CIID
TCHAR g_szDB_BRING_BACK_ACCOUNTITEM[] = _T("{CALL spBringBackAccountItem (%d, %d, %d)}");

// 모든 장비 해제
// 인자: CID
TCHAR g_szDB_CLEARALL_EQUIPEDITEM[] = _T("{CALL spClearAllEquipedItem (%d)}");

// 친구 추가
// 인자: CID, FriendCID, Favorite
TCHAR g_szDB_ADD_FRIEND[] = _T("{CALL spAddFriend (%d, %d, %d)}");

// 인자: CID, FriendCID
TCHAR g_szDB_REMOVE_FRIEND[] = _T("{CALL spRemoveFriend (%d, %d)}");

// 인자: CID
TCHAR g_szDB_GET_FRIEND_LIST[] = _T("{CALL spGetFriendList (%d)}");

// 캐릭터가 속한 클랜 알아오기
// 인자: CID
TCHAR g_szDB_GET_CHAR_CLAN[] = _T("{CALL spGetCharClan (%d)}");

// 클랜이름으로 CLID 알아오기
// 인자: ClanName	varchar(24)
TCHAR g_szDB_GET_CLID_FROM_CLANNAME[] = _T("{CALL spGetCLIDFromClanName ('%s')}");

// 클랜 만들기
// 인자: ClanName	varchar(24), MasterCID, Member1CID, Member2CID, Member3CID, Member4CID
TCHAR g_szDB_CREATE_CLAN[] = _T("{CALL spCreateClan ('%s', %d, %d, %d, %d, %d)}");

// 클랜 폐쇄 예약
// 인자: CLID, ClanName, MasterCID
TCHAR g_szDB_RESERVE_CLOSE_CLAN[] = _T("{CALL spReserveCloseClan (%d, '%s', %d, '%s')}");

// 클랜원 추가
// 인자: CLID, JoinerCID, ClanGrade
TCHAR g_szDB_ADD_CLAN_MEMBER[] = _T("{CALL spAddClanMember (%d, %d, %d)}");

// 클랜원 탈퇴
// 인자: CLID, LeaverCID
TCHAR g_szDB_REMOVE_CLAN_MEMBER[] = _T("{CALL spRemoveClanMember (%d, %d)}");

// 클랜 멤버 권한 변경
// 인자: CLID, MemberCID, ClanGrade
TCHAR g_szDB_UPDATE_CLAN_GRADE[] = _T("{CALL spUpdateClanGrade (%d, %d, %d)}");

// 클랜 멤버 강제 탈퇴
// 인자: CLID, AdminClanGrade, MemberCharName
TCHAR g_szDB_EXPEL_CLAN_MEMBER[] = _T("{CALL spRemoveClanMemberFromCharName (%d, %d, '%s')}");

// 클랜 정보 얻기 - 클랜전서버만 사용
TCHAR g_szDB_GET_CLAN_INFO[] = _T("{CALL spGetClanInfo (%d)}");

/* 래더 관련 */
// 4명팀 ID 알아오기
// 인자: Member1CID, Member2CID, Member3CID, Member4CID
TCHAR g_szDB_GET_LADDER_TEAM4_ID[] = _T("{CALL spGetTeamID4 (%d, %d, %d, %d)}");

// 4명팀 승리 전적 업데이트
// 인자: WinnerTeamID, LoserTeamID, DrawFlag
TCHAR g_szDB_TEAM4_WIN_THE_GAME[] = _T("{CALL spTeam4WinTheGame (%d, %d, %d, %d, %d, %d)}");

// CID로 나머지 팀멤버 리스트 얻어오기
// 인자: CID
TCHAR g_szDB_GET_LADDER_TEAM_MEMBERLIST[] = _T("{CALL spGetLadderTeamMemberByCID (%d)}");

/* 클랜전 */
// 클랜전 승리 전적 업데이트
// 인자: WinnerCLID, LoserCLID, DrawFlag, WinnerPoint, LoserPoint, WinnerClanName, LoserClanName, 
//       RoundWins, RoundLosses, MapID, GameType
TCHAR g_szDB_WIN_THE_CLAN_GAME[] = _T("{CALL spWinTheClanGame (%d, %d, %d, %d, %d, '%s', '%s', %d, %d, %d, %d, '%s', '%s')}");

// 클랜 기여도 업데이트
TCHAR g_szDB_UPDATE_CHAR_CLAN_CONTPOINT[] = _T("{CALL spUpdateCharClanContPoint (%d, %d, %d)}");

/* 이벤트 */
// 이벤트 짱마크 부여
// 인자 GradeID, AID
TCHAR g_szDB_EVENT_JJANG_UPDATE[] = _T("UPDATE Account SET UGradeID=%d WHERE AID=%d");

// 중앙은행의 기간 만료 아이템 삭제
TCHAR g_szDB_DELETE_EXPIRED_ACCOUNT_ITEM[] = _T("{CALL spDeleteExpiredAccountItem (%d)}");

// 케릭터의 퀘스트 아이템 정보 요청.
TCHAR g_szDB_SELECT_QUEST_ITEM_INFO_BY_CID[] = _T( "{CALL spSelectCharQuestItemInfoByCID (%d)}" );
// Quest게임 로그.
// @GameName varchar(64)
// @Master int 
// @Player1 int 
// @Player2 int
// @Player3 int
// @TotalQItemCount smallint 
// @ScenarioID smallint 
// @GamePlayTime tinyint 
TCHAR g_szDB_INSERT_QUEST_GAME_LOG[] = _T( "{CALL spInsertQuestGameLog ('%s', %d, %d, %d, %d, %d, %d, %d)}" );

// 유니크 퀘스트 아이템 로그 저장.
// @QGLID int 
// @CID int
// @QIID int
TCHAR g_szDB_INSERT_QUINQUEITEMLOG[] = _T( "{CALL spInsertQUniqueItemLog (%d, %d, %d)}" );

// 프리미엄 IP 검색
TCHAR g_szDB_CHECK_PREMIUM_IP[] = _T( "{CALL spCheckPremiumIP ('%s')}" );

// 이벤트 당첨자의 CID 저장.
// @AID int
// @CID int
// @EventName varchar(24)
TCHAR g_szInsertEvent [] = _T( "{CALL spInsertEvent (%u, %u, '%s')}" );

// @AID int
// @CID int
// @BlockType tinyint
// @Comment varchar(256)
// @EndBlockDate datetime
TCHAR g_szSetBlockAccount[] = _T( "{CALL spSetBlockAccount (%u, %u, %u, '%s', '%s', '%s')}" );

// @AID int
// @BlockType tinyint
TCHAR g_szResetAccountBlock[] = _T( "{CALL spResetAccountBlock (%u, %u)}" );

// @AID int
// @CID int
// @BlockType tinyint
// @Comment varchar(256)
// @IP varchar(15)
TCHAR g_szInsertBlockLog[] = _T( "{CALL spInsertBlockLog (%u, %u, %u,'%s', '%s')}" );

// @CID int
// @CLID int
// @DeleteDate smalldatetime
TCHAR g_szSetClanDeleteDate[] = _T( "{CALL spSetClanDeleteDate (%u, %u, '%s')}" );

// @CLID int
// @MasterCID int
// @DeleteName varchar(24)
// @WaitHour int = 168 -- 1주일
TCHAR g_szDeleteExpiredClan[] = _T( "{CALL spDeleteExpiredClan (%u, %u, '%s', %u)}" );

TCHAR g_szAdminResetAllHackingBlock[] = _T( "{CALL spResetHackingBlock}" );

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


MMatchDBMgr::MMatchDBMgr()
{
	m_strDSNConnect = "";
	m_DB.SetLogCallback( LogCallback );
}

MMatchDBMgr::~MMatchDBMgr()
{
}

bool MMatchDBMgr::CheckOpen()
{
	return m_DB.CheckOpen();

	/*
	bool ret = true;
	if (!m_DB.IsOpen())
	{
		ret = Connect(m_strDSNConnect);
		Log("MMatchDBMgr::CheckOpen - Database 재접\n");
	}

	return ret;
	*/
}

CString MMatchDBMgr::BuildDSNString(const CString strDSN, const CString strUserName, const CString strPassword)
{
	return m_DB.BuildDSNString( strDSN, strUserName, strPassword );
	/*
	CString strDSNConnect =  _T("ODBC;DSN=") + strDSN
					+ _T(";UID=") + strUserName
					+ _T(";PWD=") + strPassword;
	return strDSNConnect;
	*/
}

bool MMatchDBMgr::Connect(CString strDSNConnect)
{
	if( m_DB.Connect(strDSNConnect) )
	{
		m_CountryFilterDBMgr.SetDB( &m_DB );
		return true;
	}

	return false;
	/*
	if (m_DB.m_hdbc && m_DB.IsOpen()) m_DB.Close();

	m_strDSNConnect = strDSNConnect;

	BOOL bRet = FALSE;
	if (strDSNConnect.IsEmpty()) {
		try {
			bRet = m_DB.Open(NULL);
		} catch(CDBException* e) {
			Log("MMatchDBMgr::Connect - %s\n", e->m_strError);
		}
	} else {
		try {
			bRet = m_DB.Open(NULL,			//	DSN
							 FALSE,			//	Exclusive
							 FALSE,			//	ReadOnly
							 strDSNConnect,	//	ODBC Connect string
							 TRUE);			//	Use cursor lib
		} catch(CDBException* e) {
			Log("MMatchDBMgr::Connect - %s\n", e->m_strError);
			try {
				bRet = m_DB.Open(NULL);
			} catch(CDBException* e) {
				Log("MMatchDBMgr::Connect - %s\n", e->m_strError);

				AfxMessageBox(e->m_strError);
			}
		}
	}
	if (bRet == TRUE) {
		m_DB.SetQueryTimeout(60);
		return true;
	} else {
		OutputDebugString("DATABASE Error \n");
		return false;
	}
	*/
}

void MMatchDBMgr::Disconnect()
{
	m_DB.Disconnect();
	/*
	if (m_DB.IsOpen())
		m_DB.Close();
		*/
}


void MMatchDBMgr::Log(const char *pFormat,...)
{
	va_list args;
	char msg[1024];

	va_start(args, pFormat);
	vsprintf_safe(msg, pFormat, args);

	mlog(msg);

	va_end(args);
}


void MMatchDBMgr::LogCallback( const string& strLog )
{
	mlog( strLog.c_str() );
}


#define _STATUS_DB_START	unsigned long int nStatusStartTime = GetGlobalTimeMS();
#define _STATUS_DB_END(nID) MGetServerStatusSingleton()->AddDBQuery(nID, GetGlobalTimeMS()-nStatusStartTime);

bool MMatchDBMgr::GetLoginInfo(const TCHAR* szUserID, unsigned int* poutnAID, TCHAR* poutPassword, int maxlen)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;
	
	auto temp = m_DBFilter.Filtering(szUserID);

	szUserID = temp.c_str();

	CString strSQL;
	strSQL.Format(g_szDB_GET_LOGININFO, szUserID);

	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;

		Log("MMatchDBMgr::GetLoginInfo - %s\n", e->m_strError);

		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE))
	{
		return false;
	}

	_STATUS_DB_END(0);

	*poutnAID = rs.Field("AID").AsInt();
	strcpy_safe(poutPassword, maxlen, rs.Field("PasswordData").AsString());

	return true;
}
bool MMatchDBMgr::CreateAccount(const TCHAR* szUserID,
	const TCHAR* szPassword,
	const int nCert,
	const TCHAR* szName,
	const int nAge,
	const int nSex)

{
	_STATUS_DB_START
	if (!CheckOpen()) return false;


	try
	{
		auto temp1 = m_DBFilter.Filtering(szUserID);
		auto temp2 = m_DBFilter.Filtering(szPassword);
		CString strSQL;
		strSQL.Format(g_szDB_CREATE_ACCOUNT, temp1.c_str(), temp2.c_str(), nCert, szName, nAge, nSex);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MMatchDBMgr::CreateAccount - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(1);

	return true;
}

AccountCreationResult MMatchDBMgr::CreateAccountNew(const char *szUsername, const char *szPasswordData, const char *szEmail)
{
	_STATUS_DB_START
	if (!CheckOpen()) return ACR_DB_ERROR;

	CODBCRecordset rs(&m_DB);

	std::string Username = m_DBFilter.Filtering(szUsername), Email = m_DBFilter.Filtering(szEmail);

	szUsername = Username.c_str();
	szEmail = Email.c_str();

	try
	{
		TCHAR sql[1024];

		sprintf_safe(sql, "SELECT AID FROM Account WHERE UserID = '%s'",
			szUsername);

		rs.Open(sql, CRecordset::forwardOnly, CRecordset::readOnly);

		if (rs.IsOpen() == FALSE)
			return ACR_DB_ERROR;

		if (rs.GetRecordCount() > 0)
			return ACR_USERNAME_ALREADY_EXISTS;

		sprintf_safe(sql, "INSERT INTO Account (UserID, UGradeID, PGradeID, RegDate, Email, Age, Name) VALUES ('%s', '0', '0', GETDATE(), '%s', %d, '%s')",
			szUsername, szEmail, 0, "");

		m_DB.ExecuteSQL(sql);

		rs.Close();

		sprintf_safe(sql, "SELECT AID FROM Account WHERE UserID = '%s'",
			szUsername);

		rs.Open(sql, CRecordset::forwardOnly, CRecordset::readOnly);

		if (rs.IsOpen() == FALSE || rs.GetRecordCount() <= 0 || rs.IsBOF() == TRUE)
			return ACR_DB_ERROR;

		int AID = rs.Field("AID").AsInt();

		rs.Close();

		sprintf_safe(sql, "INSERT INTO Login(UserID, AID, PasswordData) VALUES('%s', %d, '%s')", szUsername, AID, szPasswordData);

		m_DB.ExecuteSQL(sql);
	}
	catch (CDBException* e)
	{
		Log("MMatchDBMgr::CreateAccountNew - CDBException: %s\n", e->m_strError);
		return ACR_DB_ERROR;
	}
	catch (CException *e)
	{
		char szError[128];
		e->GetErrorMessage(szError, sizeof(szError));
		Log("MMatchDBMgr::CreateAccountNew - CException: %s\n", szError);
		return ACR_DB_ERROR;
	}
	catch (...)
	{
		Log("MMatchDBMgr::CreateAccountNew - Unknown exception\n");
		return ACR_DB_ERROR;
	}

	_STATUS_DB_END(1);

	return ACR_OK;
}

bool MMatchDBMgr::BanPlayer(int nAID, const char *szReason, const time_t &UnbanTime)
{
	auto temp = m_DBFilter.Filtering(szReason);

	char szTime[128];

	tm Tm;

	localtime_s(&Tm, &UnbanTime);

	strftime(szTime, sizeof(szTime), "%c", &Tm);

	try
	{
		TCHAR sql[1024];

		sprintf_safe(sql, "UPDATE Account SET UGradeID = %d WHERE AID = %d", MMUG_BLOCKED, nAID);

		m_DB.ExecuteSQL(sql);

		sprintf_safe(sql, "INSERT INTO Blocks (AID, Type, Reason, EndDate) VALUES (%d, %d, '%s', '%s')", nAID, MMBT_BANNED, temp.c_str(), szTime);

		m_DB.ExecuteSQL(sql);
	}
	catch (CDBException* e)
	{
		Log("MMatchDBMgr::BanPlayer - %s\n", e->m_strError);
		return false;
	}

	return true;
}

bool MMatchDBMgr::CreateCharacter(int* pnOutResult,
								  const int nAID, const TCHAR* szNewName, const int nCharIndex, const int nSex,
								  const int nHair, const int nFace, const int nCostume)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;
	

	CODBCRecordset rs(&m_DB);


	try
	{
		auto temp = m_DBFilter.Filtering(szNewName);

		CString strSQL;
		strSQL.Format("SELECT COUNT(*) AS NUM FROM Character WHERE Name='%s'", temp.c_str());
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		Log("MMatchDBMgr::CreateCharacter - %s\n", e->m_strError);

		*pnOutResult = MERR_UNKNOWN;
		return false;
	}

	// 같은 이름의 캐릭이 이미 존재하는지 체크
	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		*pnOutResult = MERR_UNKNOWN;
		return false;
	}
	if (rs.Field("NUM").AsInt() > 0)
	{
		*pnOutResult = MERR_CLIENT_EXIST_CHARNAME;
		return false;
	}


	try 
	{
		CString strSQL;

		auto temp = m_DBFilter.Filtering(szNewName);

		strSQL.Format(g_szDB_CREATE_CHAR, nAID, nCharIndex, temp.c_str(), nSex, nHair, nFace, nCostume);
		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::CreateCharacter - %s\n", e->m_strError);
		*pnOutResult = MERR_UNKNOWN;
		return false;
	}
	
	_STATUS_DB_END(2);

	*pnOutResult = MOK;
	return true;
}

bool MMatchDBMgr::GetAccountCharList(const int nAID, MTD_AccountCharInfo* poutCharList, int* noutCharCount)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;
	

	CString strSQL;
	strSQL.Format(g_szDB_GET_CHARLIST, nAID);
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		Log("MMatchDBMgr::GetAccountCharList - %s\n", e->m_strError);
		return false;
	}

	if (rs.IsOpen() == FALSE)
	{
		return false;
	}

	int t = 0;
	for( ; ! rs.IsEOF(); rs.MoveNext() )
	{
		if (t >= 4) break;

		memset(&poutCharList[t], 0, sizeof(MTD_AccountCharInfo));

		strcpy_safe(poutCharList[t].szName, (LPCTSTR)rs.Field("Name").AsString());
		//strcpy_safe(poutCharList[t].szClanName, (LPCTSTR)rs.Field("ClanName").AsString());

		poutCharList[t].nCharNum = rs.Field("CharNum").AsInt();
		poutCharList[t].nLevel = rs.Field("Level").AsInt();
		t++;
	}

	*noutCharCount = t;

	_STATUS_DB_END(3);

	return true;
}

/*
bool MMatchDBMgr::GetAccountCharList(const int nAID, MTD_CharInfo* poutCharList, int* noutCharCount)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;
	

	CString strSQL;
	strSQL.Format(g_szDB_GET_CHARLIST, nAID);
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		Log("MMatchDBMgr::GetAccountCharList - %s\n", e->m_strError);
		return false;
	}

	if (rs.IsOpen() == FALSE)
	{
		return false;
	}

	int t = 0;
	for( ; ! rs.IsEOF(); rs.MoveNext() )
	{
		if (t >= 4) break;

		memset(&poutCharList[t], 0, sizeof(MTD_CharInfo));


		strcpy_safe(poutCharList[t].szName, (LPCTSTR)rs.Field("Name").AsString());
		strcpy_safe(poutCharList[t].szClanName, (LPCTSTR)rs.Field("ClanName").AsString());

		poutCharList[t].nCharNum = rs.Field("CharNum").AsInt();
		poutCharList[t].nLevel = rs.Field("Level").AsInt();
		poutCharList[t].nSex = rs.Field("Sex").AsInt();
		poutCharList[t].nHair = rs.Field("Hair").AsInt();
		poutCharList[t].nFace = rs.Field("Face").AsInt();
		poutCharList[t].nXP = (unsigned long)rs.Field("XP").AsLong();
		poutCharList[t].nBP = (unsigned long)rs.Field("BP").AsLong();

		
		memset(poutCharList[t].nEquipedItemDesc, 0, sizeof(unsigned long int) * MMCIP_END);
		poutCharList[t].nEquipedItemDesc[MMCIP_HEAD] = rs.Field("head_ItemID").AsInt();
		poutCharList[t].nEquipedItemDesc[MMCIP_CHEST] = rs.Field("chest_ItemID").AsInt();
		poutCharList[t].nEquipedItemDesc[MMCIP_HANDS] = rs.Field("hands_ItemID").AsInt();
		poutCharList[t].nEquipedItemDesc[MMCIP_LEGS] = rs.Field("legs_ItemID").AsInt();
		poutCharList[t].nEquipedItemDesc[MMCIP_FEET] = rs.Field("feet_ItemID").AsInt();
		poutCharList[t].nEquipedItemDesc[MMCIP_FINGERL] = rs.Field("fingerl_ItemID").AsInt();
		poutCharList[t].nEquipedItemDesc[MMCIP_FINGERR] = rs.Field("fingerr_ItemID").AsInt();
		poutCharList[t].nEquipedItemDesc[MMCIP_MELEE] = rs.Field("melee_ItemID").AsInt();
		poutCharList[t].nEquipedItemDesc[MMCIP_PRIMARY] = rs.Field("primary_ItemID").AsInt();
		poutCharList[t].nEquipedItemDesc[MMCIP_SECONDARY] = rs.Field("secondary_ItemID").AsInt();
		poutCharList[t].nEquipedItemDesc[MMCIP_CUSTOM1] = rs.Field("custom1_ItemID").AsInt();
		poutCharList[t].nEquipedItemDesc[MMCIP_CUSTOM2] = rs.Field("custom2_ItemID").AsInt();

		t++;
	}

	*noutCharCount = t;

	_STATUS_DB_END(3);

	return true;
}
*/

bool MMatchDBMgr::GetAccountCharInfo(const int nAID, const int nCharIndex, MTD_CharInfo* poutCharInfo)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	CString strSQL;

	strSQL.Format(g_szDB_GET_ACCOUNT_CHARINFO, nAID, nCharIndex);		
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		Log("MMatchDBMgr::GetAccountCharInfo - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		return false;
	}


	int t = 0;

	memset(poutCharInfo, 0, sizeof(MTD_CharInfo));


	strcpy_safe(poutCharInfo->szName, (LPCTSTR)rs.Field("Name").AsString());
	strcpy_safe(poutCharInfo->szClanName, (LPCTSTR)rs.Field("ClanName").AsString());

	poutCharInfo->nCharNum = rs.Field("CharNum").AsInt();
	poutCharInfo->nLevel = rs.Field("Level").AsInt();
	poutCharInfo->nSex = rs.Field("Sex").AsInt();
	poutCharInfo->nHair = rs.Field("Hair").AsInt();
	poutCharInfo->nFace = rs.Field("Face").AsInt();
	poutCharInfo->nXP = (unsigned long)rs.Field("XP").AsLong();
	poutCharInfo->nBP = (unsigned long)rs.Field("BP").AsLong();

	
	poutCharInfo->nEquipedItemDesc[MMCIP_HEAD] = rs.Field("head_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_CHEST] = rs.Field("chest_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_HANDS] = rs.Field("hands_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_LEGS] = rs.Field("legs_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_FEET] = rs.Field("feet_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_FINGERL] = rs.Field("fingerl_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_FINGERR] = rs.Field("fingerr_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_MELEE] = rs.Field("melee_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_PRIMARY] = rs.Field("primary_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_SECONDARY] = rs.Field("secondary_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_CUSTOM1] = rs.Field("custom1_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_CUSTOM2] = rs.Field("custom2_itemid").AsInt();


	_STATUS_DB_END(50);

	return true;
}


bool MMatchDBMgr::GetCharInfoByAID( const int nAID, const int nCharIndex, MMatchCharInfo* poutCharInfo, int& nWaitHourDiff )
{
_STATUS_DB_START
	if (!CheckOpen()) return false;
	

	CString strSQL;

	strSQL.Format(g_szDB_GET_CHARINFO_BY_AID, nAID, nCharIndex);		
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		Log("MMatchDBMgr::GetCharInfoByAID - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		return false;
	}
	poutCharInfo->m_nCID = (unsigned long)rs.Field("CID").AsLong();
	CString db_strName = rs.Field("Name").AsString();
	strcpy_safe(poutCharInfo->m_szName, (LPCTSTR)db_strName);
	poutCharInfo->m_nLevel = (int)rs.Field("Level").AsInt();
	poutCharInfo->m_nSex = (MMatchSex)rs.Field("Sex").AsInt();
	poutCharInfo->m_nHair = rs.Field("Hair").AsInt();
	poutCharInfo->m_nFace = rs.Field("Face").AsInt();
	poutCharInfo->m_nXP = (unsigned long)rs.Field("XP").AsLong();
	poutCharInfo->m_nBP = (unsigned long)rs.Field("BP").AsLong();
//	poutCharInfo->m_fBonusRate = (float)rs.Field("BonusRate").AsFloat();
//	poutCharInfo->m_nPrize = (int)rs.Field("Prize").AsInt();
	poutCharInfo->m_nHP = (int)rs.Field("HP").AsInt();
	poutCharInfo->m_nAP = (int)rs.Field("AP").AsInt();
//	poutCharInfo->m_nMaxWeight = (int)rs.Field("MaxWeight").AsInt();
//	poutCharInfo->m_nSafeFalls = (INT)rs.Field("SafeFalls").AsInt();
	poutCharInfo->m_nFR = (int)rs.Field("FR").AsInt();
	poutCharInfo->m_nCR = (int)rs.Field("CR").AsInt();
	poutCharInfo->m_nER = (int)rs.Field("ER").AsInt();
	poutCharInfo->m_nWR = (int)rs.Field("WR").AsInt();
	poutCharInfo->m_nCharNum = (int)rs.Field("CharNum").AsInt();

	poutCharInfo->m_nTotalKillCount = rs.Field("KillCount").AsInt();
	poutCharInfo->m_nTotalDeathCount = rs.Field("DeathCount").AsInt();
	poutCharInfo->m_nTotalPlayTimeSec = rs.Field("PlayTime").AsInt();


	// clan
	poutCharInfo->m_ClanInfo.m_nClanID = (int)rs.Field("CLID").AsInt();

	CString db_strClanName = rs.Field("ClanName").AsString();
	strcpy_safe(poutCharInfo->m_ClanInfo.m_szClanName, (LPCTSTR)db_strClanName);

	poutCharInfo->m_ClanInfo.m_nGrade = (MMatchClanGrade)rs.Field("ClanGrade").AsInt();
	poutCharInfo->m_ClanInfo.m_nContPoint = (MMatchClanGrade)rs.Field("ClanContPoint").AsInt();

	if( !rs.Field("DeleteDate").IsNull() )
		poutCharInfo->m_ClanInfo.m_strDeleteDate = rs.Field( "DeleteDate" ).AsString().GetBuffer();

	if( !rs.Field("WaitHourDiff").IsNull() )
		nWaitHourDiff = (int)rs.Field( "WaitHourDiff" ).AsInt();

	// 아이템
	memset(&poutCharInfo->m_nEquipedItemCIID, 0, sizeof(poutCharInfo->m_nEquipedItemCIID));

	poutCharInfo->m_nEquipedItemCIID[MMCIP_HEAD] = (unsigned long int)rs.Field("head_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_CHEST] = (unsigned long int)rs.Field("chest_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_HANDS] = (unsigned long int)rs.Field("hands_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_LEGS] = (unsigned long int)rs.Field("legs_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_FEET] = (unsigned long int)rs.Field("feet_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_FINGERL] = (unsigned long int)rs.Field("fingerl_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_FINGERR] = (unsigned long int)rs.Field("fingerr_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_MELEE] = (unsigned long int)rs.Field("melee_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_PRIMARY] = (unsigned long int)rs.Field("primary_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_SECONDARY] = (unsigned long int)rs.Field("secondary_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_CUSTOM1] = (unsigned long int)rs.Field("custom1_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_CUSTOM2] = (unsigned long int)rs.Field("custom2_slot").AsLong();

	_STATUS_DB_END(4);

	return true;
}

bool MMatchDBMgr::GetAccountInfo(const unsigned long int nAID, MMatchAccountInfo* poutAccountInfo)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;
	

	CString strSQL;
	strSQL.Format(g_szDB_GET_ACCOUNTINFO, nAID);

	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		Log("MMatchDBMgr::GetAccountInfo - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		return false;
	}

	poutAccountInfo->m_nAID			= (unsigned long)rs.Field("AID").AsLong();
	poutAccountInfo->m_nUGrade		= (MMatchUserGradeID)rs.Field("UGradeID").AsInt();
	poutAccountInfo->m_nPGrade		= MMPG_FREE;
	strcpy_safe(poutAccountInfo->m_szUserID, (LPCTSTR)rs.Field("UserID").AsString());

#ifdef _BLOCK_HACKER
	if( rs.Field("BlockType").IsNull() )
		poutAccountInfo->m_BlockType = MMBT_NO;
	else
	{
		poutAccountInfo->m_BlockType = static_cast< MMatchBlockType >( rs.Field("BlockType").AsInt() );
		if( MMBT_NO != poutAccountInfo->m_BlockType )
		{
			poutAccountInfo->m_EndBlockDate.wYear	= static_cast<WORD>(rs.Field("year").AsShort());
			poutAccountInfo->m_EndBlockDate.wMonth	= static_cast<WORD>(rs.Field("month").AsShort());
			poutAccountInfo->m_EndBlockDate.wDay	= static_cast<WORD>(rs.Field("day").AsShort());
			poutAccountInfo->m_EndBlockDate.wHour	= static_cast<WORD>(rs.Field("hour").AsShort());
			poutAccountInfo->m_EndBlockDate.wMinute	= static_cast<WORD>(rs.Field("min").AsShort());
		}
	}
#endif

	_STATUS_DB_END(5);

	return true;
}

bool MMatchDBMgr::DeleteCharacter(const int nAID, const int nCharIndex, const TCHAR* szCharName)
{
_STATUS_DB_START

	if (!CheckOpen()) return false;

	CString strSQL;
	strSQL.Format(g_szDB_DELETE_CHAR, nAID, nCharIndex, m_DBFilter.Filtering(szCharName));

	CODBCRecordset rs(&m_DB);

	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::DeleteCharacter - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		return false;
	}
	
	int nRet = 0;
	nRet = rs.Field("Ret").AsInt();
	if (nRet != 1) return false;

_STATUS_DB_END(6);
	return true;

}


bool MMatchDBMgr::SimpleUpdateCharInfo(MMatchCharInfo* pCharInfo)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;
	

	try 
	{
		CString strSQL;
		strSQL.Format(g_szDB_SIMPLE_UPDATE_CHARINFO, pCharInfo->m_nCID, m_DBFilter.Filtering(pCharInfo->m_szName),
						pCharInfo->m_nLevel, pCharInfo->m_nXP, pCharInfo->m_nBP);

		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::SimpleUpdateCharInfo - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(7);
	return true;

}


bool MMatchDBMgr::InsertCharItem(const unsigned int nCID, int nItemDescID, bool bRentItem, int nRentPeriodHour, unsigned long int* poutCIID)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;


	// 무기한 아이템이면 nRentPeriodHour=0 이다.
	if (bRentItem == false) nRentPeriodHour = 0;

	CString strSQL;
	strSQL.Format(g_szDB_INSERT_CHAR_ITEM, nCID, nItemDescID, nRentPeriodHour);
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		Log("MMatchDBMgr::InsertCharItem - %s\n", e->m_strError);
		Log( "MMatchDBMgr::InsertCharItem - Err Query : %s\n", strSQL.GetBuffer() );
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		return false;
	}

	*poutCIID = rs.Field("ORDERCIID").AsLong();

	_STATUS_DB_END(8);
	return true;
}


bool MMatchDBMgr::BuyBountyItem(const unsigned int nCID, int nItemID, int nPrice, unsigned long int* poutCIID)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;


	CString strSQL;
	strSQL.Format(g_szDB_BUY_BOUNTY_ITEM, nCID, nItemID, nPrice);
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		Log("MMatchDBMgr::BuyBountyItem - %s , Query: %s\n", e->m_strError, strSQL.GetBuffer());
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		return false;
	}

	*poutCIID = rs.Field("ORDERCIID").AsLong();

	_STATUS_DB_END(55);
	return true;
}

bool MMatchDBMgr::SellBountyItem(const unsigned int nCID, unsigned int nItemID, unsigned int nCIID, int nPrice, int nCharBP)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;


	CString strSQL;
	strSQL.Format(g_szDB_SELL_BOUNTY_ITEM, nCID, nItemID, nCIID, nPrice, nCharBP);
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		Log("MMatchDBMgr::SellBountyItem - %s , Query: %s\n", e->m_strError, strSQL.GetBuffer());
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		return false;
	}

	int nRet  = rs.Field("Ret").AsLong();

	_STATUS_DB_END(56);

	if (nRet == 1) return true;

	return false;
}


bool MMatchDBMgr::DeleteCharItem(const unsigned int nCID, int nCIID)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;


	try 
	{
		CString strSQL;
		strSQL.Format(g_szDB_DELETE_CHAR_ITEM, nCID, nCIID);
		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::DeleteCharItem - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(9);
	return true;
}

bool MMatchDBMgr::GetCharItemInfo(MMatchCharInfo* pCharInfo)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;


	CString strSQL;
	strSQL.Format(g_szDB_SELECT_CHAR_ITEM, pCharInfo->m_nCID);
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		Log("MMatchDBMgr::GetCharItemInfo - %s\n", e->m_strError);
		return false;
	}

	if (rs.IsOpen() == FALSE)
	{
		return false;
	}

	int t = 0;
	int nItemDescID = 0;
	unsigned long int nCIID = 0;

	for( ; ! rs.IsEOF(); rs.MoveNext() )
	{
		nItemDescID = 0; nCIID = 0;
		nItemDescID = (int)rs.Field("ItemID").AsInt();
		nCIID = (unsigned long int)rs.Field("CIID").AsLong();
		

		bool bIsRentItem = !rs.Field("RentPeriodRemainder").IsNull();		// 기간제 아이템 여부
		int nRentMinutePeriodRemainder = RENT_MINUTE_PERIOD_UNLIMITED;		// 기간제아이템 남은시간(분)
		bool bSpendingItem = false;											// 소비아이템 여부 (지금은 무조건 false - 아직 완전히 구현되지 않음)
		int nCnt = -1;														// 소비아이템 개수
		

		if (bIsRentItem)
		{
			nRentMinutePeriodRemainder = rs.Field("RentPeriodRemainder").AsInt();
		}
/*
		if (!rs.Field("Cnt").IsNull())
		{
			bSpendingItem = true;
			nCnt = (int)rs.Field("Cnt").AsShort();
		}
*/		

		MUID uidNew = MMatchItemMap::UseUID();
		pCharInfo->m_ItemList.CreateItem(uidNew, nCIID, nItemDescID, bIsRentItem, nRentMinutePeriodRemainder);
		
		t++;
	}

	pCharInfo->m_ItemList.SetDbAccess();	// 디비에서 가져왔다고 알림

_STATUS_DB_END(10);
	return true;
}

bool MMatchDBMgr::GetAccountItemInfo(const int nAID, MAccountItemNode* pOut, int* poutNodeCount, int nMaxNodeCount,
									 MAccountItemNode* pOutExpiredItemList, int* poutExpiredItemCount, int nMaxExpiredItemCount)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;


	CString strSQL;
	strSQL.Format(g_szDB_SELECT_ACCOUNT_ITEM, nAID);
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		Log("MMatchDBMgr::GetAccountItemInfo - %s\n", e->m_strError);
		return false;
	}

	if (rs.IsOpen() == FALSE)
	{
		return false;
	}

	int aiid = 0, itemid = 0;
	int nodecount = 0;

	int nExpiredItemCount=0;		// 기간만료된 아이템 개수

	for( ; ! rs.IsEOF(); rs.MoveNext() )
	{
		itemid = (int)rs.Field("ItemID").AsInt();
		aiid = (int)rs.Field("AIID").AsInt();

		bool bIsRentItem = !rs.Field("RentPeriodRemainder").IsNull();		// 기간제 아이템 여부
		int nRentMinutePeriodRemainder = RENT_MINUTE_PERIOD_UNLIMITED;		// 기간제아이템 남은 시간(분단위)
		bool bSpendingItem = false;											// 소비아이템 여부(아직 완전히 구현되어있지 않음)
		int nCnt = -1;														// 소비아이템 개수


		if (bIsRentItem)
		{
			nRentMinutePeriodRemainder = rs.Field("RentPeriodRemainder").AsInt();
		}
/*
		if (!rs.Field("Cnt").IsNull())
		{
			bSpendingItem = true;
			nCnt = (int)rs.Field("Cnt").AsShort();
		}
*/

		if ((bIsRentItem) && (nRentMinutePeriodRemainder <= 0))
		{
			if (nExpiredItemCount < nMaxExpiredItemCount)
			{
				// 만약 기간만료 아이템이 있으면 accountitem에 추가하지 않는다.
				pOutExpiredItemList[nExpiredItemCount].nAIID = aiid;
				pOutExpiredItemList[nExpiredItemCount].nItemID = (unsigned long int)itemid; 
				nExpiredItemCount++;
			}
		}
		else
		{
			pOut[nodecount].nAIID = aiid;
			pOut[nodecount].nItemID = (unsigned long int)itemid;
			pOut[nodecount].nRentMinutePeriodRemainder = nRentMinutePeriodRemainder;

			nodecount++;
			if (nodecount >= nMaxNodeCount) break;
		}
	}

	*poutNodeCount = nodecount;
	*poutExpiredItemCount = nExpiredItemCount;

	_STATUS_DB_END(25);
	return true;
}

bool MMatchDBMgr::UpdateEquipedItem(const unsigned long nCID, MMatchCharItemParts parts, 
									unsigned long int nCIID, const unsigned long int nItemID, bool* poutRet)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	CString strSQL;
	strSQL.Format(g_szDB_UPDATE_EQUIPITEM, (int)nCID, (int)parts, (int)nCIID, (int)nItemID);
	CODBCRecordset rs(&m_DB);

	int nRet = 0;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::UpdateEquipedItem - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		return false;
	}

	nRet = rs.Field("Ret").AsInt();

	*poutRet = (nRet == 0) ? false : true;


_STATUS_DB_END(11);

	return true;
}


bool MMatchDBMgr::InsertConnLog(const int nAID, const char* szIP, const string& strCountryCode3)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	if( 0 == szIP ) return false;
	
	// char szShortIP[64];
	// MConvertCompactIP(szShortIP, szIP);
	vector< uint8_t > vIP;
	if( !SplitStrIP(szIP, vIP) || (4 != vIP.size()) ) 
	{
		mlog( "MMatchDBMgr::InsertConnLog - 문자열 IP(%s)분해 실패. AID(%u)\n", 
			nAID, szIP );
		return false;
	}
	
	try 
	{
		auto temp = m_DBFilter.Filtering(strCountryCode3);

		CString strSQL;
		// strSQL.Format(g_szDB_INSERT_CONN_LOG, nAID, szUserID, szIP);
		strSQL.Format(g_szDB_INSERT_CONN_LOG, nAID, vIP[0], vIP[1], vIP[2], vIP[3], temp.c_str());

		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::InsertConnLog - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(12);
	return true;
}

bool MMatchDBMgr::InsertGameLog(const char* szGameName, const char* szMap, const char* szGameType,
								const int nRound, const unsigned int nMasterCID, 
								const int nPlayerCount, const char* szPlayers)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	CString strSQL;
	try 
	{
		string strStageName = m_DBFilter.Filtering( string(szGameName) );

		strSQL.Format(g_szDB_INSERT_GAME_LOG, &strStageName[0], szMap, szGameType, nRound, nMasterCID,
			nPlayerCount, szPlayers);

		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::InsertGameLog - %s Query:%s\n", e->m_strError, strSQL.GetBuffer() );
		
		return false;
	}

_STATUS_DB_END(13);
	return true;
}

bool MMatchDBMgr::InsertKillLog(const unsigned int nAttackerCID, const unsigned int nVictimCID)
{
	if (!CheckOpen()) return false;

	try 
	{
		CString strSQL;
		strSQL.Format(g_szDB_INSERT_KILL_LOG, nAttackerCID, nVictimCID);

		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::InsertKillLog - %s\n", e->m_strError);
		return false;
	}

	return true;
}

bool MMatchDBMgr::InsertChatLog(const unsigned long int nCID, const char* szMsg, unsigned long int nTime)
{
	return true;

	if (!CheckOpen()) return false;

	try 
	{
		auto temp = m_DBFilter.Filtering(szMsg);
		CString strSQL;
		strSQL.Format("INSERT Into ChatLog (CID, Msg, Time) Values (%u, '%s', GETDATE())", nCID, temp.c_str());
		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::InsertChatLog - %s\n", e->m_strError);
		return false;
	}

	return true;
}


// 캐릭터 레벨 업데이트
bool MMatchDBMgr::UpdateCharLevel(const int nCID, const int nLevel)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;
	try 
	{
		CString strSQL;
		strSQL.Format(g_szDB_UPDATE_CHAR_LEVEL, nLevel, nCID);

		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::UpdateCharLevel - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(14);
	return true;
}

// bp 업데이트
bool MMatchDBMgr::UpdateCharBP(const int nCID, const int nBPInc)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;


#ifdef _DEBUG
	mlog( "MMatchDBMgr::UpdateCharBP - Added bounty price:%d\n", nBPInc );
#endif


	try 
	{
		CString strSQL;
		strSQL.Format(g_szDB_UPDATE_CHAR_BP, nBPInc, nCID);

		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::UpdateCharBP - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(15);
	return true;
}

bool MMatchDBMgr::InsertItemPurchaseLogByBounty(const unsigned long int nItemID, const unsigned long int nCID,
	const int nBounty, const int nCharBP, const _ITEMPURCHASE_TYPE nType)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;



	char szType[8] = "";

	switch (nType)
	{
	case IPT_BUY: strcpy_safe(szType, "구입"); break;
	case IPT_SELL: strcpy_safe(szType, "판매"); break;
	}

	try 
	{
		CString strSQL;
		strSQL.Format(g_szDB_INSERT_ITEM_PURCHASE_BY_BOUNTY_LOG, nItemID, nCID, nBounty, nCharBP, szType);

		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::InsertItemPurchaseLogByBounty - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(16);
	return true;

}

bool MMatchDBMgr::InsertCharMakingLog(const unsigned int nAID, const char* szCharName,
						const _CHARMAKING_TYPE nType)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;



	char szType[8] = "";

	switch (nType)
	{
	case CMT_CREATE: strcpy_safe(szType, "생성"); break;
	case CMT_DELETE: strcpy_safe(szType, "삭제"); break;
	}

	try 
	{
		auto temp = m_DBFilter.Filtering(szCharName);

		CString strSQL;
		strSQL.Format(g_szDB_INSERT_CHAR_MAKING_LOG, nAID, temp.c_str(), szType);

		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::InsertCharMakingLog - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(17);
	return true;
}


bool MMatchDBMgr::InsertServerLog(const int nServerID, const int nPlayerCount, const int nGameCount, const DWORD dwBlockCount, const DWORD dwNonBlockCount)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;


	try 
	{
		CString strSQL;
		strSQL.Format(g_szDB_INSERT_SERVER_LOG, nServerID, nPlayerCount, nGameCount, dwBlockCount, dwNonBlockCount);

		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::InsertServerLog - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(18);
	return true;
}

bool MMatchDBMgr::UpdateServerStatus(const int nServerID, const int nCurrPlayer)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	try 
	{
		CString strSQL;
		strSQL.Format(g_szDB_UPDATE_SERVER_STATUS, nCurrPlayer, nServerID);

		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::UpdateServerStatus - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(19);
	return true;
}

bool MMatchDBMgr::UpdateServerInfo(const int nServerID, const int nMaxPlayer, const char* szServerName)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;


	try 
	{
		auto temp = m_DBFilter.Filtering(szServerName);

		CString strSQL;
		strSQL.Format(g_szDB_UPDATE_SERVER_INFO, nMaxPlayer, temp.c_str(), nServerID);

		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::UpdateServerInfo - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(20);
	return true;
}

bool MMatchDBMgr::InsertPlayerLog(const unsigned long int nCID,
			const int nPlayTime, const int nKillCount, const int nDeathCount, const int nXP, const int nTotalXP)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	try 
	{
		CString strSQL;
		strSQL.Format(g_szDB_INSERT_PLAYER_LOG, nCID, nPlayTime, nKillCount, nDeathCount, nXP, nTotalXP);

		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::InsertPlayerLog - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(21);
	return true;
}

bool MMatchDBMgr::InsertLevelUpLog(const int nCID, const int nLevel, const int nBP, 
			const int nKillCount, const int nDeathCount, const int nPlayTime)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	try 
	{
		CString strSQL;
		strSQL.Format(g_szDB_INSERT_LEVELUP_LOG, nCID, nLevel, nBP, nKillCount, nDeathCount, nPlayTime);

		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::InsertLevelUpLog - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(41);
	return true;

}

bool MMatchDBMgr::UpdateCharPlayTime(const unsigned long int nCID, const unsigned long int nPlayTime)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;


	try 
	{
		CString strSQL;
		strSQL.Format(g_szDB_UPDATE_CHAR_PLAYTIME, nPlayTime, nCID);

		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::UpdateCharPlayTime - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(22);
	return true;
}

bool MMatchDBMgr::UpdateCharInfoData(const int nCID, const int nAddedXP, const int nAddedBP, 
						const int nAddedKillCount, const int nAddedDeathCount)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;


	try 
	{
		CString strSQL;
		strSQL.Format(g_szDB_UPDATE_CHAR_INFO_DATA, nAddedXP, nAddedBP, 
							nAddedKillCount, nAddedDeathCount, nCID);

		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::UpdateCharInfoData - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(23);
	return true;
}

bool MMatchDBMgr::UpdateLastConnDate(const TCHAR* szUserID, const TCHAR* szIP)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	try 
	{
		auto temp = m_DBFilter.Filtering(szIP);

		CString strSQL;
		strSQL.Format(g_szDB_UPDATE_LAST_CONNDATE, temp.c_str(), szUserID);

		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::UpdateLastConnDate - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(24);
	return true;
}

bool MMatchDBMgr::BringAccountItem(const int nAID, const int nCID, const int nAIID, 
								   unsigned long int* poutCIID, unsigned long int* poutItemID,
								   bool* poutIsRentItem, int* poutRentMinutePeriodRemainder)
{
	_STATUS_DB_START
	if (!CheckOpen()) return false;

	CString strSQL;
	strSQL.Format(g_szDB_BRING_ACCOUNTITEM, nAID, nCID, nAIID);
	CODBCRecordset rs(&m_DB);

	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::BringAccountItem - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		return false;
	}

	*poutCIID = rs.Field("ORDERCIID").AsLong();
	*poutItemID = rs.Field("ItemID").AsLong();


	bool bIsRentItem = !rs.Field("RentPeriodRemainder").IsNull();		// 기간제 아이템 여부
	int nRentMinutePeriodRemainder = RENT_MINUTE_PERIOD_UNLIMITED;		// 기간제아이템 남은시간(분)

	if (bIsRentItem)
	{
		nRentMinutePeriodRemainder = rs.Field("RentPeriodRemainder").AsInt();
	}

	*poutIsRentItem = bIsRentItem;
	*poutRentMinutePeriodRemainder = nRentMinutePeriodRemainder;

	_STATUS_DB_END(26);
	return true;
}

bool MMatchDBMgr::BringBackAccountItem(const int nAID, const int nCID, const int nCIID)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	try 
	{
		CString strSQL;
		strSQL.Format(g_szDB_BRING_BACK_ACCOUNTITEM, nAID, nCID, nCIID);

		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::BringBackAccountItem - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(32);
	return true;


}

bool MMatchDBMgr::ClearAllEquipedItem(const unsigned long nCID)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	try 
	{
		CString strSQL;
		strSQL.Format(g_szDB_CLEARALL_EQUIPEDITEM, nCID);

		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::ClearAllEquipedItem - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(27);
	return true;


}

bool MMatchDBMgr::GetCharCID(const TCHAR* pszName, int* poutCID)
{
_STATUS_DB_START

	if (!CheckOpen()) return false;
	CODBCRecordset rs(&m_DB);

	try
	{
		auto temp = m_DBFilter.Filtering(pszName);

		CString strSQL;
		strSQL.Format("SELECT CID, Name FROM Character(NOLOCK) WHERE Name='%s'", temp.c_str());
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		Log("MMatchDBMgr::GetCharCID - %s\n", e->m_strError);
		return false;
	}

	// 존재하는 캐릭이름인지 체크
	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		return false;
	}
	*poutCID = rs.Field("CID").AsInt();

_STATUS_DB_END(28);
	return true;
}

//// Friends ////
bool MMatchDBMgr::FriendAdd(const int nCID, const int nFriendCID, const int nFavorite)
{
_STATUS_DB_START

	if (!CheckOpen()) return false;
	CODBCRecordset rs(&m_DB);

	// 친구 추가
	CString strSQL;
	strSQL.Format(g_szDB_ADD_FRIEND, nCID, nFriendCID, nFavorite);

	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		Log("MMatchDBMgr::FriendAdd - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(29);
	return true;
}

bool MMatchDBMgr::FriendRemove(const int nCID, const int nFriendCID)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	try 
	{
		CString strSQL;
		strSQL.Format(g_szDB_REMOVE_FRIEND, nCID, nFriendCID);
		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::FriendRemove - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(30);
	return true;
}

bool MMatchDBMgr::FriendGetList(const int nCID, MMatchFriendInfo* pFriendInfo)
{
	#define MAX_FRIEND_LISTCOUNT 30

	_STATUS_DB_START
		if (!CheckOpen()) return false;

	CString strSQL;
	strSQL.Format(g_szDB_GET_FRIEND_LIST, nCID);
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		Log("MMatchDBMgr::FriendGetList - %s\n", e->m_strError);
		return false;
	}

	if (rs.IsOpen() == FALSE)
	{
		return false;
	}

	int nFriendCID=0, nFavorite=0;
	int nodecount = 0;
	CString strName;

	for( ; ! rs.IsEOF(); rs.MoveNext() )
	{
		nFriendCID = (int)rs.Field("FriendCID").AsInt();
		nFavorite = (int)rs.Field("Favorite").AsInt();
		strName = rs.Field("Name").AsString();

		pFriendInfo->Add(nFriendCID, nFavorite, (LPCTSTR)strName);
	}

	_STATUS_DB_END(31);
	return true;
}


bool MMatchDBMgr::GetCharClan(const int nCID, int* poutClanID, TCHAR* poutClanName, int maxlen)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;
	
	CString strSQL;
	strSQL.Format(g_szDB_GET_CHAR_CLAN, nCID);

	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		Log("MMatchDBMgr::GetCharClan - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		return false;
	}
	
	*poutClanID = rs.Field("CLID").AsInt();
	strcpy_safe(poutClanName, maxlen, (LPCTSTR)rs.Field("ClanName").AsString());

_STATUS_DB_END(33);

	return true;
}

bool MMatchDBMgr::GetClanIDFromName(const TCHAR* szClanName, int* poutCLID)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	auto temp = m_DBFilter.Filtering(szClanName);
	
	CString strSQL;
	strSQL.Format(g_szDB_GET_CLID_FROM_CLANNAME, temp.c_str());

	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		Log("MMatchDBMgr::GetClanIDFromName - %s\n", e->m_strError);

		*poutCLID = 0;
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		*poutCLID = 0;
		return false;
	}
	
	*poutCLID = rs.Field("CLID").AsInt();

_STATUS_DB_END(34);

	return true;
}

bool MMatchDBMgr::CreateClan(const TCHAR* szClanName, const int nMasterCID, const int nMember1CID, const int nMember2CID,
		        const int nMember3CID, const int nMember4CID, bool* boutRet, int* noutNewCLID)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	auto temp = m_DBFilter.Filtering(szClanName);

	CString strSQL;
	strSQL.Format(g_szDB_CREATE_CLAN, temp.c_str(), nMasterCID, nMember1CID, nMember2CID, nMember3CID, nMember4CID);

	CODBCRecordset rs(&m_DB);


	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		Log("MMatchDBMgr::CreateClan - %s\n", e->m_strError);

		*boutRet = false;
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		*boutRet = false;
		return false;
	}
	
	*boutRet = rs.Field("Ret").AsBool();
	*noutNewCLID = rs.Field("NewCLID").AsInt();


_STATUS_DB_END(35);
	return true;


}

bool MMatchDBMgr::ReserveCloseClan(const int nCLID, const TCHAR* szClanName, const int nMasterCID, const string& strDeleteDate )
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	try 
	{
		auto temp1 = m_DBFilter.Filtering(szClanName);
		auto temp2 = m_DBFilter.Filtering(strDeleteDate);

		CString strSQL;
		strSQL.Format(g_szDB_RESERVE_CLOSE_CLAN, nCLID, temp1.c_str(), nMasterCID, temp2.c_str());
		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::ReserveCloseClan - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(36);
	return true;
}

bool MMatchDBMgr::AddClanMember(const int nCLID, const int nJoinerCID, const int nClanGrade, bool* boutRet)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;


	CString strSQL;
	strSQL.Format(g_szDB_ADD_CLAN_MEMBER, nCLID, nJoinerCID, nClanGrade);

	CODBCRecordset rs(&m_DB);


	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		Log("MMatchDBMgr::AddClanMember - %s\n", e->m_strError);

		*boutRet = false;
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		*boutRet = false;
		return false;
	}
	
	*boutRet = rs.Field("Ret").AsBool();


_STATUS_DB_END(37);
	return true;

}

bool MMatchDBMgr::RemoveClanMember(const int nCLID, const int nLeaverCID)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	try 
	{
		CString strSQL;
		strSQL.Format(g_szDB_REMOVE_CLAN_MEMBER, nCLID, nLeaverCID);
		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::RemoveClanMember - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(38);
	return true;


}


bool MMatchDBMgr::UpdateClanGrade(const int nCLID, const int nMemberCID, const int nClanGrade)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	try 
	{
		CString strSQL;
		strSQL.Format(g_szDB_UPDATE_CLAN_GRADE, nCLID, nMemberCID, nClanGrade);
		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::UpdateClanGrade - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(39);
	return true;
	
}

bool MMatchDBMgr::ExpelClanMember(const int nCLID, const int nAdminGrade, TCHAR* szMember, int* noutRet)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	auto temp = m_DBFilter.Filtering(szMember);

	CString strSQL;
	strSQL.Format(g_szDB_EXPEL_CLAN_MEMBER, nCLID, nAdminGrade, temp.c_str());

	CODBCRecordset rs(&m_DB);


	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		Log("MMatchDBMgr::ExpelClanMember - %s\n", e->m_strError);

		*noutRet = ER_NO_MEMBER;
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		*noutRet = ER_NO_MEMBER;
		return false;
	}
	
	*noutRet = rs.Field("Ret").AsInt();

/* Ret값 설명 : 1 - 성공, 0 - 해당클랜원이 없다. , 2 - 권한이 맞지 않다. */

_STATUS_DB_END(40);
	return true;
}



bool MMatchDBMgr::GetLadderTeamID(const int nTeamTableIndex, const int* pnMemberCIDArray, const int nMemberCount, int* pnoutTID)
{
_STATUS_DB_START

	if (!CheckOpen()) return false;


	CString strSQL;

	// 현재는 4명 팀밖에 지원하지 않는다.
	if ((nTeamTableIndex == 4) && (nMemberCount == 4))
	{
		strSQL.Format(g_szDB_GET_LADDER_TEAM4_ID, pnMemberCIDArray[0], pnMemberCIDArray[1], pnMemberCIDArray[2], pnMemberCIDArray[3]);
	}

#ifdef _DEBUG
	else if ((nTeamTableIndex == 1) && (nMemberCount == 1))
	{
		// for test
		strSQL.Format(g_szDB_GET_LADDER_TEAM4_ID, pnMemberCIDArray[0], 1, 1, 1);
	}
	else if ((nTeamTableIndex == 2) && (nMemberCount == 2))
	{
		// for test
		strSQL.Format(g_szDB_GET_LADDER_TEAM4_ID, pnMemberCIDArray[0], pnMemberCIDArray[1], 1, 1);
	}
#endif
	else
	{
		return false;
	}

	CODBCRecordset rs(&m_DB);


	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::GetLadderTeamID - %s\n", e->m_strError);
		*pnoutTID = 0;
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		*pnoutTID = 0;
		return false;
	}
	
	char szFieldName[64] = "";

//#ifdef _DEBUG
	strcpy_safe(szFieldName, "TID4");	// 지금은 모두다 TID4로 통일중
//#else
//	sprintf_safe(szFieldName, "TID%d", nTeamTableIndex);
//#endif

	*pnoutTID = rs.Field(szFieldName).AsInt();


_STATUS_DB_END(42);
	return true;


}


bool MMatchDBMgr::LadderTeamWinTheGame(const int nTeamTableIndex, const int nWinnerTID, const int nLoserTID, const bool bIsDrawGame,
									   const int nWinnerPoint, const int nLoserPoint, const int nDrawPoint)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	int nDrawGame = 0;
	if (bIsDrawGame) nDrawGame = 1;

	try 
	{
		CString strSQL;

		strSQL.Format(g_szDB_TEAM4_WIN_THE_GAME, nWinnerTID, nLoserTID, nDrawGame, nWinnerPoint, nLoserPoint, nDrawPoint);
		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::Team4WinTheGame - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(43);
	return true;
}

bool MMatchDBMgr::GetLadderTeamMemberByCID(const int nCID, int* poutTeamID, char** ppoutCharArray, int maxlen, int nCount)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	CODBCRecordset rs(&m_DB);

	try 
	{
		CString strSQL;
		strSQL.Format(g_szDB_GET_LADDER_TEAM_MEMBERLIST, nCID);
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::GetLadderTeamMemberByCID - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		return false;
	}

	int nTeamID = rs.Field("TID4").AsInt();
	if (poutTeamID)
		*poutTeamID = nTeamID;

	int nIndex = 0;
	for( ; ! rs.IsEOF(); rs.MoveNext() )
	{
		if (nIndex >= nCount) break;

		CString strName = rs.Field("Name").AsString();
		if (ppoutCharArray)
			strcpy_safe(ppoutCharArray[nIndex], maxlen, (LPCTSTR)strName);
		nIndex++;
	}

_STATUS_DB_END(44);
	return true;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////
bool MMatchDBMgr::UpdateCharLevel(const int nCID, const int nNewLevel, const int nBP, const int nKillCount, 
		                const int nDeathCount, const int nPlayTime, bool bIsLevelUp)
{
	bool ret = UpdateCharLevel(nCID, nNewLevel);

	if ((ret == true) && (bIsLevelUp) && (nNewLevel >= 10))		// 10레벨이상 레벨 올릴때에만 로그를 남긴다.
	{
		
		InsertLevelUpLog(nCID, nNewLevel, nBP, nKillCount, nDeathCount, nPlayTime);
	}

	return ret;
}


bool MMatchDBMgr::GetClanInfo(const int nCLID, MDB_ClanInfo* poutClanInfo)
{
_STATUS_DB_START

	if (!CheckOpen()) return false;
	CODBCRecordset rs(&m_DB);

	try
	{
		CString strSQL;
		strSQL.Format(g_szDB_GET_CLAN_INFO, nCLID);
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		Log("MMatchDBMgr::GetClanInfo - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		return false;
	}


	poutClanInfo->nCLID = rs.Field("CLID").AsInt();
	poutClanInfo->nLevel = rs.Field("Level").AsInt();
	strcpy_safe(poutClanInfo->szClanName , rs.Field("Name").AsString());
	strcpy_safe(poutClanInfo->szMasterName , rs.Field("ClanMaster").AsString());
	strcpy_safe(poutClanInfo->szEmblemUrl , rs.Field("EmblemUrl").AsString());
	poutClanInfo->nTotalPoint = rs.Field("TotalPoint").AsInt();
	poutClanInfo->nPoint = rs.Field("Point").AsInt();
	poutClanInfo->nWins = rs.Field("Wins").AsInt();
	poutClanInfo->nLosses = rs.Field("Losses").AsInt();
	poutClanInfo->nTotalMemberCount = rs.Field("MemberCount").AsInt();
	poutClanInfo->nRanking = rs.Field("Ranking").AsInt();
	poutClanInfo->nEmblemChecksum = rs.Field("EmblemChecksum").AsInt();
	

_STATUS_DB_END(45);
	return true;


}

bool MMatchDBMgr::WinTheClanGame(const int nWinnerCLID, const int nLoserCLID, const bool bIsDrawGame,
					const int nWinnerPoint, const int nLoserPoint, const char* szWinnerClanName,
					const char* szLoserClanName, const int nRoundWins, const int nRoundLosses,
					const int nMapID, const int nGameType,
					const char* szWinnerMembers, const char* szLoserMembers)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	int nDrawGame = 0;
	if (bIsDrawGame) nDrawGame = 1;

	try 
	{
		std::string temp[] = { m_DBFilter.Filtering(szWinnerClanName),
			m_DBFilter.Filtering(szLoserClanName),
			m_DBFilter.Filtering(szWinnerMembers),
			m_DBFilter.Filtering(szLoserMembers),
		};

		CString strSQL;

		strSQL.Format(g_szDB_WIN_THE_CLAN_GAME, nWinnerCLID, nLoserCLID, nDrawGame, nWinnerPoint, nLoserPoint,
			temp[0].c_str(), temp[1].c_str(), nRoundWins, nRoundLosses, nMapID, nGameType,
			temp[2].c_str(), temp[3].c_str());
						
		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::WinTheClanGame - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(46);
	return true;
}

bool MMatchDBMgr::UpdateCharClanContPoint(const int nCID, const int nCLID, const int nAddedContPoint)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	try 
	{
		CString strSQL;

		strSQL.Format(g_szDB_UPDATE_CHAR_CLAN_CONTPOINT, nCID, nCLID, nAddedContPoint);
		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::UpdateCharClanContPoint - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(47);
	return true;
}

bool MMatchDBMgr::EventJjangUpdate(const int nAID, bool bJjang)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	try 
	{
		CString strSQL;

		MMatchUserGradeID nGrade = bJjang ? MMUG_STAR : MMUG_FREE;
		strSQL.Format(g_szDB_EVENT_JJANG_UPDATE, nGrade, nAID);
		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::EventJjangUpdate - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(48);
	return true;
}


bool MMatchDBMgr::DeleteExpiredAccountItem(const int nAIID)
{
_STATUS_DB_START
	if (!CheckOpen()) return false;

	try 
	{
		CString strSQL;

		strSQL.Format(g_szDB_DELETE_EXPIRED_ACCOUNT_ITEM, nAIID);
		m_DB.ExecuteSQL( strSQL );
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::DeleteExpiredAccountItem - %s\n", e->m_strError);
		return false;
	}

_STATUS_DB_END(49);
	return true;

}



#define QUEST_ITEM_NUM_SIZE			sizeof( int )
#define SIZE_OF_SIMPLE_QUEST_ITEM	sizeof(SimpleQuestItem)


bool MMatchDBMgr::UpdateQuestItem( int nCID, MQuestItemMap& rfQuestIteMap, MQuestMonsterBible& rfQuestMonster )
{
_STATUS_DB_START
	try
	{
		// 7000byte가 넘는지 검사함.
		if( QUEST_DATA < MCRC32::SIZE + MAX_DB_QUEST_ITEM_SIZE + MAX_DB_MONSTERBIBLE_SIZE )
		{
			ASSERT( 0 );
			Log( "MMatchDBMgr::UpdateQuestItem - 아이템의 크기가 %d보다 큼.", QUEST_DATA );
			return false;
		}

		unsigned char			szData[ QUEST_DATA ];
		unsigned char			szMonData[ MAX_DB_MONSTERBIBLE_SIZE ];
		MCRC32::crc_t			Crc32;
		int						i;
		MQuestItemMap::iterator	it, end;

		memset( szData, 0, QUEST_DATA );
		memset( szMonData, 0, MAX_DB_MONSTERBIBLE_SIZE );

		// 퀘스트 아이템 정보 저장.
		end = rfQuestIteMap.end();
		for( i = 0, it = rfQuestIteMap.begin(); it != end; ++it )
		{
			MQuestItem* pQuestItem = it->second;

			// DB에 저장이 되는 QuestItem타입인지 검사. 몬스터 도감관련 아이템은 저장될수 없음.
			if( GetQuestItemDescMgr().IsQItemID(it->first) )
			{
				// 배열은 인덱스가 0부터 시작을 하고, 퀘스트 아이템의 ItemID는 1부터 시작한다.
				int nIndex = MCRC32::SIZE + pQuestItem->GetItemID() - MIN_QUEST_ITEM_ID;
				unsigned char nValue = 0;
				if (pQuestItem->GetCount() > 0) 
				{
					nValue = pQuestItem->GetCount() + MIN_QUEST_DB_ITEM_COUNT;
				}
				else if (pQuestItem->IsKnown()) 
				{
					nValue = MIN_QUEST_DB_ITEM_COUNT;
				}
				else nValue = 0;

				szData[ nIndex ] = 
					static_cast< unsigned char >( nValue );
			}
		}

		// 몬스터 정보 저장.
		memcpy( szData + MCRC32::SIZE + MAX_DB_QUEST_ITEM_SIZE, &rfQuestMonster, MAX_DB_MONSTERBIBLE_SIZE );
		
		// make crc checksum.
		Crc32 = MCRC32::BuildCRC32( reinterpret_cast<BYTE*>(szData + MCRC32::SIZE), MAX_DB_QUEST_ITEM_SIZE + MAX_DB_MONSTERBIBLE_SIZE );
		if( 0 == Crc32 )
		{
			mlog( "MMatchDBMgr::UpdateQuestItem - CRC32 is 0 CID:%d\n", nCID );
			// 만약 여기서 문제가 방생한 것이라면, 이부분에 왔을경우는 모든 데이터를 검사해 줘야함. 
		}
		
		// copy crc checksum.
		memcpy( szData, &Crc32, MCRC32::CRC::SIZE );

		CString strQuery;
		strQuery.Format( "UPDATE Character SET QuestItemInfo = (?) WHERE CID = %d", nCID );

		CODBCRecordset rs( &m_DB );
		if( !rs.InsertBinary(strQuery, szData, MCRC32::CRC::SIZE + MAX_DB_QUEST_ITEM_SIZE + MAX_DB_MONSTERBIBLE_SIZE) )
		{
			Log( "MMatchDBMgr::UpdateQuestItem - Querry fail." );
			return false;
		}
	}
	catch( CDBException* e )
	{
		Log( "MMatchDBMgr::InsertQuestItem - %s\n", e->m_strError );
		return false;
	}

_STATUS_DB_END(51);

	return true;
}

bool MMatchDBMgr::GetCharQuestItemInfo( MMatchCharInfo* pCharInfo )
{
_STATUS_DB_START

	try
	{
		if (!CheckOpen()) return false;

		if( 0 == pCharInfo )
			return false;

		CODBCRecordset rs( &m_DB );

		CString strQuery;
		strQuery.Format( g_szDB_SELECT_QUEST_ITEM_INFO_BY_CID, pCharInfo->m_nCID );

		CDBBinary bn = rs.SelectBinary( strQuery );
		// 쿼리가 실패하면 Index를 -1로 설정해서 반환해줌.
		if( -1 == bn.GetCurIndex() )
		{
			mlog( "MMatchDBMgr::GetCharQuestItemInfo - 퀘스트 아이템 SELECT쿼리 실패. UserName:%s CID:%d\n", 
				pCharInfo->m_szName, 
				pCharInfo->m_nCID );
			return false;
		}

		unsigned char	szData[ QUEST_DATA ];
		MCRC32::crc_t	Crc32;

		// 반드시 Begin()을 호출해야 함.
		bn.Begin();
		int nDataSize = bn.GetNextData(szData, QUEST_DATA);
		if( -1 == nDataSize )
		{
			Log( "MMatchDMBgr::GetQuestItem - 데이터를 복사하는데 오류." );
			return false;
		}

		// copy crc check sum.
		memcpy( &Crc32, szData, MCRC32::CRC::SIZE );

		// 케릭터를 처음 만들면 퀘스트 아이템 필드와 몬스터 도감 필드가 모두 0이기에 CRC32검사를 할수가 없음.
		//  그러므로 처음 4Byte가 모두 0이면 필드에 데이터가 없는것으로 판단함.
		if( 0 == nDataSize )
		{
			pCharInfo->m_QuestItemList.Clear();
			memset( &pCharInfo->m_QMonsterBible, 0, sizeof(MQuestMonsterBible) );
			pCharInfo->m_QuestItemList.SetDBAccess( true );

			_STATUS_DB_END(52);
			return true;
		}
		
		if( Crc32 != MCRC32::BuildCRC32(reinterpret_cast<BYTE*>(szData + MCRC32::CRC::SIZE), nDataSize - MCRC32::CRC::SIZE) )
		{
			Log( "MMatchDBMgr::GetQuestItem - crc check error." );
			return false;
		}

		// 정보를 가져오기전에 이전 데이터 클리어.
		pCharInfo->m_QuestItemList.Clear();

		// 순서를 반드시 지켜야 함. 퀘스트 아이템 다음에 몬스터 정보.
		// 퀘스트아이템 로드.
		for( int i = 0; i < MAX_DB_QUEST_ITEM_SIZE; ++i )
		{
			if( MIN_QUEST_DB_ITEM_COUNT > static_cast<int>(szData[MCRC32::CRC::SIZE + i]) )
				continue; // 한번도 획득한 적이 없는 아이템.

			// 한번이라도 획든한적이 있던 아이템. 수량은 상관 없음. 
			unsigned long int nItemID = i + MIN_QUEST_ITEM_ID;
			int nQuestItemCount = static_cast<int>(szData[MCRC32::CRC::SIZE + i]);
			bool bKnownItem;

			if (nQuestItemCount > 0)
			{
				bKnownItem = true;
				nQuestItemCount--;		// DB에 저장된 값 1은 사실 0이다.
			}
			else
			{
				bKnownItem = false;
			}

			if( !pCharInfo->m_QuestItemList.CreateQuestItem(nItemID, nQuestItemCount, bKnownItem) )
				mlog( "MMatchDBMgr::GetCharQuestItemInfo - 새로운 퀘스트 아이템 생성 실패.\n" ); // error...
		}

		// 몬스터 정보 저장.
		memcpy( &pCharInfo->m_QMonsterBible, szData + MCRC32::SIZE + MAX_DB_QUEST_ITEM_SIZE, MAX_DB_MONSTERBIBLE_SIZE );
#ifdef _DEBUG
		{
			mlog( "\nStart %s's debug MonsterBible info.\n", pCharInfo->m_szName );
			for( int i = 0; i < (220000 - 210001); ++i )
			{
				if( pCharInfo->m_QMonsterBible.IsKnownMonster(i) )
				{
					MQuestItemDesc* pMonDesc = GetQuestItemDescMgr().FindMonserBibleDesc( i );
					if( 0 == pMonDesc )
					{
						mlog( "MMatchDBMgr::GetCharQuestItemInfo - %d Fail to find monster bible description.\n", i );
						continue;
					}
					else
					{
						mlog( "Get DB MonBibleID:%d MonName:%s\n", i, pMonDesc->m_szQuestItemName );
					}
				}
			}
			mlog( "End %s's debug MonsterBible info.\n\n", pCharInfo->m_szName );
		}
#endif

		pCharInfo->m_QuestItemList.SetDBAccess( true );
	}
	catch( CDBException* e )
	{
		Log( "MMatchDBMgr::SelectQuestItem - %s\n", e->m_strError );
		return false;
	}

_STATUS_DB_END(52);

	return true;
}


bool MMatchDBMgr::InsertQuestGameLog( const char* pszStageName, 
									  const int nScenarioID,
									  const int nMasterCID, const int nPlayer1, const int nPlayer2, const int nPlayer3,
									  const int nTotalRewardQItemCount,
									  const int nElapsedPlayTime,
									  int& outQGLID )
{
_STATUS_DB_START
	if( !CheckOpen() )
		return false;

	CString strQuery;
	CODBCRecordset rs(&m_DB);

	try
	{
		string strStageName = m_DBFilter.Filtering( string(pszStageName) );

		strQuery.Format( g_szDB_INSERT_QUEST_GAME_LOG, 
			&strStageName[0], nMasterCID, nPlayer1, nPlayer2, nPlayer3, nTotalRewardQItemCount, nScenarioID, nElapsedPlayTime );
		rs.Open( strQuery, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch( CDBException* e )
	{
		mlog( "MMatchDBMgr::InsertQuestGameLog - %s\n", e->m_strError );
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		return false;
	}

	outQGLID = rs.Field("ORDERQGLID").AsLong();

_STATUS_DB_END(53);
	return true;
}


bool MMatchDBMgr::InsertQUniqueGameLog( const int nQGLID, const int nCID, const int nQIID )
{
_STATUS_DB_START
	if( !CheckOpen() )
		return false;

	if( (0 == nCID) || (0 == nQIID) )
		return false;

	CString strQuery;
	try
	{
		strQuery.Format( g_szDB_INSERT_QUINQUEITEMLOG, nQGLID, nCID, nQIID );
		m_DB.ExecuteSQL( strQuery );
	}
	catch( CDBException* e )
	{
		mlog( "MMatchDBMgr::InsertQUniqueGameLog - %s\n", e->m_strError );
		return false;
	}
_STATUS_DB_END(54);
	return true;
}

// 최고 54

bool MMatchDBMgr::GetCID( const char* pszCharName, int& outCID )
{
	if( 0 == pszCharName ) 
		return false;

	CString strQuery;
	strQuery.Format( "SELECT CID FROM Character (UPDLOCK) WHERE Name LIKE '%s'", 
		pszCharName );

	CODBCRecordset rs(&m_DB);
	try
	{
		rs.Open( strQuery, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch( CDBException* e )
	{
		mlog( "MMatchDBMgr::GetCID - %s\n", e->m_strError );
		return false;
	}

	outCID = rs.Field("CID").AsLong();

	return true;
}


bool MMatchDBMgr::GetCharName( const int nCID, string& outCharName )
{
	if( !CheckOpen() )
		return false;

	CString strQuery;
	strQuery.Format( "SELECT Name FROM Character (NOLOCK) WHERE CID = %u", nCID );

	CODBCRecordset rs(&m_DB);
	try
	{
		rs.Open( strQuery, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch( CDBException* e )
	{
		mlog( "MMatchDBMgr::GetCharName - %s\n", e->m_strError );
		return false;
	}

	outCharName = rs.Field("Name").AsString().GetBuffer();

	return true;
}

bool MMatchDBMgr::CheckPremiumIP(const char* szIP, bool& outbResult)
{
_STATUS_DB_START
	if( !CheckOpen() )
		return false;

	auto temp = m_DBFilter.Filtering(szIP);

	CString strSQL;
	strSQL.Format(g_szDB_CHECK_PREMIUM_IP, temp.c_str());
	
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		Log("MMatchDBMgr::CheckPremiumIP - %s\n", e->m_strError);
		return false;
	}

	if (rs.IsOpen() == FALSE)
	{
		return false;
	}

	const bool IsGameRoom	= rs.Field( "IsGameRoom" ).AsBool();
	const int  nEtc			= rs.Field( "etc" ).AsInt();

	if( IsGameRoom )
	{
		outbResult = IsGameRoom;

		// 추가적인 작업은 여기에.
	}
	else 
	{
		outbResult = false;
	}

	/* 2006-08-31이후 사용하지 않음. 혹시 모르니 보시면 삭제 바람.
	else if ((rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 	// 결과값이 0개이면 false
	{
		outbResult = false;
	}
	else
	{
		outbResult = true;
	}
	*/

	
_STATUS_DB_END(57);
	return true;
}


bool MMatchDBMgr::GetIPContryCode( const string& strIP, 
					 DWORD& dwOutIPFrom, 
					 DWORD& dwOutIPTo, 
					 string& strOutCountryCode )
{
	_STATUS_DB_START

	auto temp = m_DBFilter.Filtering(strIP);

	try 
	{
	m_CountryFilterDBMgr.GetIPContryCode(temp.c_str(), dwOutIPFrom, dwOutIPTo, strOutCountryCode);
	}
	catch( CDBException* e )
	{
		Log( "MMatchDBMgr::GetIPContryCode - %s\n", e->m_strError );
		return false;
	}
_STATUS_DB_END(58);
	return true;
}


bool MMatchDBMgr::GetBlockCountryCodeList( BlockCountryCodeList& rfBlockCountryCodeList )
{
_STATUS_DB_START
	try
	{
		m_CountryFilterDBMgr.GetBlockCountryCodeList( rfBlockCountryCodeList );
	}
	catch( CDBException* e )
	{
		Log( "MMatchDBMgr::GetBlockCountryCodeList - %s\n", e->m_strError );
		return false;
	}
_STATUS_DB_END(60);
	return true;
}


bool MMatchDBMgr::GetCustomIP( const string& strIP, DWORD& dwIPFrom, DWORD& dwIPTo, bool& bIsBlock, string& strCountryCode3, string& strComment )
{
_STATUS_DB_START

	auto temp = m_DBFilter.Filtering(strIP);

	try
	{
	m_CountryFilterDBMgr.GetCustomIP(temp.c_str(), dwIPFrom, dwIPTo, bIsBlock, strCountryCode3, strComment);
	}
	catch( CDBException* e )
	{
		Log( "MMatchDBMgr::GetCustomIP - %s\n", e->m_strError );
		return false;
	}
_STATUS_DB_END(59);
	return true;

}


bool MMatchDBMgr::GetIPtoCountryList( IPtoCountryList& rfIPtoCountryList )
{
_STATUS_DB_START
	try
	{
		m_CountryFilterDBMgr.GetIPtoCountryList( rfIPtoCountryList );
	}
	catch( CDBException* e )
	{
		Log( "MMatchDBMgr::GetIPtoCountryList - %s\n", e->m_strError );
		return false;
	}
_STATUS_DB_END(61);
	return true;
}


bool MMatchDBMgr::GetCustomIPList( CustomIPList& rfCustomIPList )
{
_STATUS_DB_START
	try
	{
		m_CountryFilterDBMgr.GetCustomIPList( rfCustomIPList );
	}
	catch( CDBException* e )
	{
		Log( "MMatchDBMgr::GetCustomIPList - %s\n", e->m_strError );
		return false;
	}
_STATUS_DB_END(62);
	return true;
}

bool MMatchDBMgr::InsertEvent( const DWORD dwAID,  const DWORD dwCID, const string& strEventName )
{
_STATUS_DB_START
	if( !CheckOpen() )
		return false;

	auto temp = m_DBFilter.Filtering(strEventName);

	CString strSQL;
	strSQL.Format(g_szInsertEvent, dwAID, dwCID, temp.c_str());
	try
	{
		m_DB.ExecuteSQL( strSQL );
	}
	catch(CDBException* e)
	{
		Log( "MMatchDBMgr::InsertEvent - %s\n", e->m_strError );
		return false;
	}
_STATUS_DB_END(63);
	
	return true;
}


bool MMatchDBMgr::SetBlockAccount( const DWORD dwAID, const DWORD dwCID, const BYTE btBlockType, const string& strComment, const string& strIP, const string& strEndHackBlockerDate )
{
#ifdef _BLOCK_HACKER
_STATUS_DB_START
	if( !CheckOpen() )
		return false;

	CString strSQL;
	strSQL.Format( g_szSetBlockAccount, dwAID, dwCID, btBlockType, strComment.c_str(), strIP.c_str(), strEndHackBlockerDate.c_str() );
	try
	{
		m_DB.ExecuteSQL( strSQL );
	}
	catch( CDBException* e )
	{
		Log( "MMatchDBMgr::InsertBlockLog - %s.\n", e->m_strError );
		return false;
	}

_STATUS_DB_END(64);
#endif
	return true;
}


bool MMatchDBMgr::ResetAccountBlock( const DWORD dwAID, const BYTE btBlockType )
{
#ifdef _BLOCK_HACKER
_STATUS_DB_START
	if( !CheckOpen() )
		return false;

	CString strSQL;
	strSQL.Format( g_szResetAccountBlock, dwAID, btBlockType );
	try
	{
		m_DB.ExecuteSQL( strSQL );
	}
	catch( CDBException* e )
	{
		Log( "MMatchDBMgr::ResetAccountBlock - %s.\n", e->m_strError );
		return false;
	}
_STATUS_DB_END(65);
#endif
	return true;
}



bool MMatchDBMgr::InsertBlockLog( const DWORD dwAID, const DWORD dwCID, const BYTE btBlockType, const string& strComment,
 								  const string& strIP )
{
#ifdef _BLOCK_HACKER
_STATUS_DB_START
	if( !CheckOpen() )
		return false;

	CString strSQL;
	strSQL.Format( g_szInsertBlockLog, dwAID, dwCID, btBlockType, strComment.c_str(), strIP.c_str() );
	try
	{
		m_DB.ExecuteSQL( strSQL );
	}
	catch( CDBException* e )
	{
		Log( "MMatchDBMgr::InsertBlockLog - %s.\n", e->m_strError );
		return false;
	}
_STATUS_DB_END(66);
#endif
	return true;
}


bool MMatchDBMgr::DeleteExpiredClan( const DWORD dwCID, const DWORD dwCLID, const string& strDeleteName, const DWORD dwWaitHour )
{
#ifdef _DELETE_CLAN
_STATUS_DB_START
	if( !CheckOpen() )
		return false;

	CString strSQL;
	strSQL.Format( g_szDeleteExpiredClan, dwCLID, dwCID, strDeleteName.c_str(), dwWaitHour );
	try
	{
		m_DB.ExecuteSQL( strSQL );
	}
	catch( CDBException* e )
	{
		Log( "MMatchDBMgr::DeleteExpiredClan - %s.\n", e->m_strError );
		return false;
	}
_STATUS_DB_END(67);
#endif
	return true;
}


bool MMatchDBMgr::SetDeleteTime( const DWORD dwMasterCID, const DWORD dwCLID, const string& strDeleteDate )
{
#ifdef _DELETE_CLAN
_STATUS_DB_START
	if( !CheckOpen() )
		return false;

	if( strDeleteDate.empty() )
		return false;

	CString strSQL;
	strSQL.Format( g_szSetClanDeleteDate, dwMasterCID, dwCLID, strDeleteDate.c_str() );
	try
	{
		m_DB.ExecuteSQL( strSQL );
	}
	catch( CDBException* e )
	{
		Log( "MMatchDBMgr::SetDeleteTime - %s.\n", e->m_strError );
		return false;
	}
_STATUS_DB_END(68);
#endif

	return true;
}


bool MMatchDBMgr::AdminResetAllHackingBlock()
{
_STATUS_DB_START
	if( !CheckOpen() )
		return false;

	try
	{
		m_DB.ExecuteSQL( g_szAdminResetAllHackingBlock );
	}
	catch( CDBException* e )
	{
		Log( "MMatchDBMgr::AdminResetAllHackingBlock - %s\n", e->m_strError );
		return false;
	}

_STATUS_DB_END(69);

	return true;
}

// 최고 69
