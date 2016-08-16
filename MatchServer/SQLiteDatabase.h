#pragma once

#include <utility>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include "IDatabase.h"
#include "..\..\sdk\sqlite\include\sqlite3.h"

struct StringView
{
	const char* Ptr;
	size_t Size;
};

inline void BindParameter(sqlite3_stmt* stmt, int ParamNum) { }

template <typename... Args, typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
void BindParameter(sqlite3_stmt* stmt, int ParamNum, T Value, Args&&... args)
{
	sqlite3_bind_int(stmt, ParamNum, static_cast<int>(Value));
	BindParameter(stmt, ParamNum + 1, std::forward<Args>(args)...);
}

template <typename... Args>
void BindParameter(sqlite3_stmt* stmt, int ParamNum, int Value, Args&&... args)
{
	sqlite3_bind_int(stmt, ParamNum, Value);
	BindParameter(stmt, ParamNum + 1, std::forward<Args>(args)...);
}

template <typename... Args>
void BindParameter(sqlite3_stmt* stmt, int ParamNum, int64_t Value, Args&&... args)
{
	sqlite3_bind_int64(stmt, ParamNum, Value);
	BindParameter(stmt, ParamNum + 1, std::forward<Args>(args)...);
}

template <typename... Args>
void BindParameter(sqlite3_stmt* stmt, int ParamNum, unsigned int Value, Args&&... args)
{
	sqlite3_bind_int64(stmt, ParamNum, Value);
	BindParameter(stmt, ParamNum + 1, std::forward<Args>(args)...);
}

template <typename... Args>
void BindParameter(sqlite3_stmt* stmt, int ParamNum, unsigned long Value, Args&&... args)
{
	sqlite3_bind_int64(stmt, ParamNum, Value);
	BindParameter(stmt, ParamNum + 1, std::forward<Args>(args)...);
}

template <typename... Args>
void BindParameter(sqlite3_stmt* stmt, int ParamNum, const char *Value, Args&&... args)
{
	sqlite3_bind_text(stmt, ParamNum, Value, -1, [](void*) {});
	BindParameter(stmt, ParamNum + 1, std::forward<Args>(args)...);
}

template <typename... Args>
void BindParameter(sqlite3_stmt* stmt, int ParamNum, const StringView& Value, Args&&... args)
{
	sqlite3_bind_text(stmt, ParamNum, Value.Ptr, Value.Size, [](void*) {});
	BindParameter(stmt, ParamNum + 1, std::forward<Args>(args)...);
}

struct Blob
{
	const void* Ptr;
	size_t Size;
};

template <typename... Args>
void BindParameter(sqlite3_stmt* stmt, int ParamNum, const Blob& Value, Args&&... args)
{
	sqlite3_bind_blob(stmt, ParamNum, Value.Ptr, Value.Size, [](void*) {});
	BindParameter(stmt, ParamNum + 1, std::forward<Args>(args)...);
}

class SQLiteError : public std::runtime_error
{
public:
	SQLiteError(int err_code)
		: err_code(err_code), std::runtime_error("")
	{ }

	SQLiteError(int err_code, const char* err_msg)
		: err_code(err_code), std::runtime_error(err_msg)
	{ }

	SQLiteError(int err_code, const std::string& err_msg)
		: err_code(err_code), std::runtime_error(err_msg)
	{ }

	int GetErrorCode() const
	{
		return err_code;
	}

private:
	int err_code;
};

class SQLite
{
public:
	SQLite(const char* Filename)
	{
		auto err_code = sqlite3_open(Filename, &sqlite);
		if (err_code != SQLITE_OK)
			throw std::runtime_error(std::string("sqlite3_open failed: error code: ") + std::to_string(err_code) + ", error message: " + sqlite3_errmsg(sqlite));
	}

	~SQLite()
	{
		if (sqlite)
			sqlite3_close(sqlite);
	}

	operator sqlite3*()
	{
		return sqlite;
	}

	sqlite3** operator&()
	{
		return &sqlite;
	}

private:
	sqlite3* sqlite = nullptr;
};

class SQLiteStatement
{
public:
	SQLiteStatement()
		: stmt(nullptr), bHasRow(false)
	{ }
	SQLiteStatement(sqlite3_stmt* stmt)
		: stmt(stmt), bHasRow(false)
	{ }

	~SQLiteStatement()
	{
		Reset();
	}

	SQLiteStatement(const SQLiteStatement&) = delete;
	SQLiteStatement& operator=(const SQLiteStatement&) = delete;

	SQLiteStatement(SQLiteStatement&& src)
		: stmt(nullptr), col(0)
	{
		Move(std::move(src));
	}
	SQLiteStatement& operator=(SQLiteStatement&& src)
	{
		Move(std::move(src));
		return *this;
	}

	operator sqlite3_stmt*() { return stmt; }

	template <typename T>
	T Get(int Column);

	template <typename T>
	T Get()
	{
		auto ret = Get<T>(col);
		col++;
		return ret;
	}

	template <typename T>
	T Get(const char* ColumnName)
	{
		for (int i = 1; i <= Columns(); i++)
		{
			if (strcmp(ColumnName, sqlite3_column_name(stmt, i)))
				continue;

			return Get<T>(i);
		}
	}

	bool IsNull(int Column)
	{
		return sqlite3_column_type(stmt, Column) == SQLITE_NULL;
	}

	bool IsNull() { return IsNull(col); }

	sqlite3_stmt*& GetStatement() { return stmt; }

	int Columns() { return sqlite3_data_count(stmt); }
	int ExpectedColumns() { return sqlite3_column_count(stmt); }

	auto Step()
	{
		auto ret = sqlite3_step(stmt);
		bHasRow = ret == SQLITE_ROW;
		if (ret != SQLITE_ROW && ret != SQLITE_DONE)
			throw SQLiteError(ret);
		col = 0;
		return ret;
	}
	bool HasRow() const { return bHasRow; }

	int NextColumn() { col++; return col; }

private:
	void Move(SQLiteStatement&& src)
	{
		Reset();
		stmt = src.stmt;
		col = src.col;
		bHasRow = src.bHasRow;
		src.stmt = nullptr;
	}

	void Reset()
	{
		if (stmt)
			sqlite3_reset(stmt);
	}

	sqlite3_stmt* stmt = nullptr;
	int col = 0;
	bool bHasRow = false;
};

class SQLiteStatementStorage
{
public:
	SQLiteStatementStorage()
		: stmt(nullptr)
	{ }

	SQLiteStatementStorage(sqlite3_stmt* stmt)
		: stmt(stmt)
	{ }

	~SQLiteStatementStorage()
	{
		if (stmt)
			sqlite3_finalize(stmt);
	}

	SQLiteStatementStorage(const SQLiteStatementStorage&) = delete;
	SQLiteStatementStorage& operator=(const SQLiteStatementStorage&) = delete;

	SQLiteStatementStorage(SQLiteStatementStorage&& src)
	{
		Move(std::move(src));
	}
	SQLiteStatementStorage& operator=(SQLiteStatementStorage&& src)
	{
		Move(std::move(src));
		return *this;
	}

	sqlite3_stmt*& GetStatement()
	{
		return stmt;
	}

	explicit operator SQLiteStatement()
	{
		return SQLiteStatement(stmt);
	}

private:
	void Move(SQLiteStatementStorage&& src)
	{
		stmt = src.stmt;
		src.stmt = nullptr;
	}

	sqlite3_stmt* stmt;
};

class SQLiteDatabase final : public IDatabase
{
public:
	SQLiteDatabase();

	virtual bool IsOpen() override { return true; }

	virtual bool GetLoginInfo(const char* szUserID, unsigned int* poutnAID, char* poutPassword, size_t maxlen) override;
	virtual bool UpdateCharLevel(int nCID, int nLevel) override;
	virtual bool InsertLevelUpLog(int nCID, int nLevel, int nBP,
		int nKillCount, int nDeathCount, int nPlayTime) override;

	virtual bool UpdateLastConnDate(const char* szUserID, const char* szIP) override;

	virtual bool CreateAccount(const char* szUserID,
		const char* szPassword,
		int nCert,
		const char* szName,
		int nAge,
		int nSex) override
	{ return false; }

	virtual AccountCreationResult CreateAccountNew(const char *szUsername,
		const char *szPasswordData, size_t PasswordSize, const char *szEmail) override;

	virtual bool BanPlayer(int nAID, const char *szReason, const time_t &UnbanTime) override;

	virtual int CreateCharacter(int nAID,
		const char* szNewName,
		int nCharIndex,
		int nSex,
		int nHair,
		int nFace,
		int nCostume) override;
	virtual bool DeleteCharacter(const int nAID,
		const int nCharIndex,
		const char* szCharName) override;

	virtual bool GetAccountCharList(int nAID,
	struct MTD_AccountCharInfo* poutCharList,
		int* noutCharCount) override;
	virtual bool GetAccountCharInfo(int nAID, int nCharIndex, struct MTD_CharInfo* poutCharInfo) override;
	virtual bool GetAccountInfo(int AID,
	struct MMatchAccountInfo* outAccountInfo) override;

	virtual bool GetCharInfoByAID(int nAID,
		int nCharIndex,
	class MMatchCharInfo* poutCharInfo,
		int& nWaitHourDiff) override;

	virtual bool GetCharCID(const char* pszName, int* poutCID) override;

	virtual bool SimpleUpdateCharInfo(const MMatchCharInfo& CharInfo) override;

	virtual bool UpdateCharBP(const int nCID, const int nBPInc) override;

	virtual bool UpdateCharInfoData(const int nCID, const int nAddedXP, const int nAddedBP,
		const int nAddedKillCount, const int nAddedDeathCount) override;

	virtual bool InsertCharItem(unsigned int nCID, int nItemDescID, bool bRentItem,
		int nRentPeriodHour, unsigned long int* poutCIID) override;
	virtual bool DeleteCharItem(unsigned int nCID, int nCIID) override;
	virtual bool GetCharItemInfo(MMatchCharInfo& CharInfo) override;
	virtual bool GetAccountItemInfo(int nAID, struct MAccountItemNode* pOut, int* poutNodeCount,
		int nMaxNodeCount, MAccountItemNode* pOutExpiredItemList, int* poutExpiredItemCount,
		int nMaxExpiredItemCount) override;
	virtual bool UpdateEquipedItem(const unsigned long nCID,
	enum MMatchCharItemParts parts,
		unsigned long int nCIID,
		unsigned long int nItemID) override;
	virtual bool ClearAllEquipedItem(unsigned long nCID) override;
	virtual bool DeleteExpiredAccountItem(int nAIID) override;
	virtual bool BuyBountyItem(unsigned int CID, int ItemID, int Price, unsigned long* outCIID) override;
	virtual bool SellBountyItem(unsigned int nCID, unsigned int nItemID, unsigned int nCIID,
		int nPrice, int nCharBP) override;


	virtual bool UpdateQuestItem(int nCID, class MQuestItemMap& rfQuestIteMap,
	class MQuestMonsterBible& rfQuestMonster) override;
	virtual bool GetCharQuestItemInfo(MMatchCharInfo* pCharInfo) override;

	virtual bool InsertQuestGameLog(const char* pszStageName,
		int nScenarioID,
		int nMasterCID, int nPlayer1, int nPlayer2, int nPlayer3,
		int nTotalRewardQItemCount,
		int nElapsedPlayTime,
		int& outQGLID) override;

	virtual bool InsertQUniqueGameLog(int nQGLID, int nCID, int nQIID) override;


	virtual bool InsertConnLog(int nAID, const char* szIP, const std::string& strCountryCode3) override;
	virtual bool InsertGameLog(const char* szGameName, const char* szMap, const char* GameType,
		int nRound, unsigned int nMasterCID,
		int nPlayerCount, const char* szPlayers) override;
	virtual bool InsertKillLog(unsigned int nAttackerCID, unsigned int nVictimCID) override;
	virtual bool InsertChatLog(unsigned long int nCID, const char* szMsg, u64 nTime) override;
	virtual bool InsertServerLog(int nServerID, int nPlayerCount, int nGameCount,
		uint32_t dwBlockCount, uint32_t dwNonBlockCount) override;
	virtual bool InsertPlayerLog(unsigned long int nCID,
		int nPlayTime, int nKillCount, int nDeathCount, int nXP, int nTotalXP) override;

	virtual bool UpdateServerStatus(int nServerID, int nPlayerCount) override { return true; }
	virtual bool UpdateMaxPlayer(int nServerID, int nMaxPlayer) override { return true; }
	virtual bool UpdateServerInfo(int nServerID, int nMaxPlayer, const char* szServerName) override { return true; }

	virtual bool UpdateCharPlayTime(unsigned long int nCID, unsigned long int nPlayTime) override;

	virtual bool InsertItemPurchaseLogByBounty(unsigned long int nItemID, unsigned long int nCID,
		int nBounty, int nCharBP, ItemPurchaseType nType) override
	{ return false; }

	virtual bool InsertCharMakingLog(unsigned int nAID, const char* szCharName,
		CharMakingType nType) override;

	virtual bool BringAccountItem(int nAID, int nCID, int nAIID,
		unsigned int* poutCIID, unsigned long int* poutItemID,
		bool* poutIsRentItem, int* poutRentMinutePeriodRemainder) override;

	virtual bool BringBackAccountItem(int nAID, int nCID, int nCIID) override;


	//// Friends ////
	virtual bool FriendAdd(int nCID, int nFriendCID, int nFavorite) override;
	virtual bool FriendRemove(int nCID, int nFriendCID) override;
	virtual bool FriendGetList(int nCID, class MMatchFriendInfo* pFriendInfo) override;


	//// Clan ////
	template<size_t size>
	bool GetCharClan(int nCID, int* poutClanID, char(&poutClanName)[size]) {
		return GetCharClan(nCID, poutClanID, poutClanName, size); }
	virtual bool GetCharClan(int nCID, int* poutClanID, char* poutClanName, int maxlen) override;
	virtual bool GetClanIDFromName(const char* szClanName, int* poutCLID) override;
	virtual bool CreateClan(const char* szClanName, int nMasterCID, int nMember1CID, int nMember2CID,
		int nMember3CID, int nMember4CID, bool* boutRet, int* noutNewCLID) override;
	virtual bool DeleteExpiredClan(uint32_t dwCID, uint32_t dwCLID, const std::string& strDeleteName,
		uint32_t dwWaitHour = 24) override;
	virtual bool SetDeleteTime(uint32_t dwMasterCID, uint32_t dwCLID, const std::string& strDeleteDate) override;
	virtual bool ReserveCloseClan(const int nCLID, const char* szClanName, int nMasterCID,
		const std::string& strDeleteDate) override;
	virtual bool AddClanMember(int nCLID, int nJoinerCID, int nClanGrade, bool* boutRet) override;
	virtual bool RemoveClanMember(int nCLID, int nLeaverCID) override;
	virtual bool UpdateClanGrade(int nCLID, int nMemberCID, int nClanGrade) override;
	virtual ExpelResult ExpelClanMember(int CLID, int AdminGrade, const char* Member) override;
	virtual bool GetClanInfo(int nCLID, MDB_ClanInfo* poutClanInfo) override;
	virtual bool UpdateCharClanContPoint(int nCID, int nCLID, int nAddedContPoint) override;


	//// Ladder ////
	virtual bool GetLadderTeamID(const int nTeamTableIndex, const int* pnMemberCIDArray, int nMemberCount,
		int* pnoutTID) override;
	virtual bool LadderTeamWinTheGame(int nTeamTableIndex, int nWinnerTID, int nLoserTID,
		bool bIsDrawGame, int nWinnerPoint, int nLoserPoint, int nDrawPoint) override;
	virtual bool GetLadderTeamMemberByCID(const int nCID, int* poutTeamID, char** ppoutCharArray, int maxlen, int nCount) override;


	virtual bool WinTheClanGame(int nWinnerCLID, int nLoserCLID, bool bIsDrawGame,
		int nWinnerPoint, int nLoserPoint, const char* szWinnerClanName,
		const char* szLoserClanName, int nRoundWins, int nRoundLosses,
		int nMapID, int nGameType,
		const char* szWinnerMembers, const char* szLoserMembers) override;


	virtual bool UpdateCharLevel(int nCID, int nNewLevel, int nBP, int nKillCount,
		int nDeathCount, int nPlayTime, bool bIsLevelUp) override;

	virtual bool EventJjangUpdate(int nAID, bool bJjang) override;
	virtual bool CheckPremiumIP(const char* szIP, bool& outbResult) override;

	virtual bool GetCID(const char* pszCharName, int& outCID) override;
	virtual bool GetCharName(const int nCID, std::string& outCharName) override;

	virtual bool GetIPCountryCode(const std::string& strIP,
		uint32_t& dwOutIPFrom,
		uint32_t& dwOutIPTo,
		std::string& strOutCountryCode) override;
	virtual bool GetBlockCountryCodeList(BlockCountryCodeList& rfBlockCountryCodeList) override;
	virtual bool GetIPtoCountryList(IPtoCountryList& rfIPtoCountryList) override;
	virtual bool GetCustomIP(const std::string& strIP, uint32_t& dwIPFrom, uint32_t& dwIPTo, bool& bIsBlock,
		std::string& strCountryCode3, std::string& strComment) override;
	virtual bool GetCustomIPList(CustomIPList& rfCustomIPList) override;

	virtual bool InsertEvent(uint32_t dwAID, uint32_t dwCID, const std::string& strEventName) override;

	virtual bool SetBlockAccount(uint32_t dwAID,
		uint32_t dwCID,
		uint8_t btBlockType,
		const std::string& strComment,
		const std::string& strIP,
		const std::string& strEndHackBlockerDate) override;

	virtual bool ResetAccountBlock(uint32_t dwAID, uint8_t btBlockType) override;

	virtual bool InsertBlockLog(uint32_t dwAID, uint32_t dwCID, uint8_t btBlockType, const std::string& strComment,
		const std::string& strIP) override;

	virtual bool AdminResetAllHackingBlock() override;

private:
	friend class Transaction;

	template <size_t size>
	auto PrepareStatement(const char(&sql)[size])
	{
		SQLiteStatementStorage stmt;
		const char* tail = nullptr;

		auto err_code = sqlite3_prepare_v2(sqlite, sql, size, &stmt.GetStatement(), &tail);
		if (err_code != SQLITE_OK)
			throw SQLiteError(err_code, std::string("Prepare threw ") + std::to_string(err_code) + ": " + sqlite3_errmsg(sqlite) + " on " + sql);

		return stmt;
	}

	template <size_t size, typename... Args>
	auto ExecuteSQL(const char(&sql)[size], Args&&... args)
	{
		auto it = PreparedStatements.find(sql);
		if (it == PreparedStatements.end())
		{
			it = PreparedStatements.emplace(sql, PrepareStatement(sql)).first;
		}

		auto stmt = SQLiteStatement{ it->second };

		BindParameter(stmt, 1, std::forward<Args>(args)...);
		auto err_code = stmt.Step();
		if (err_code != SQLITE_DONE && err_code != SQLITE_ROW)
			throw SQLiteError(err_code, sqlite3_errmsg(sqlite));

		return stmt;
	}

	void HandleException(const SQLiteError& e);

	class Transaction
	{
	public:
		Transaction(SQLiteDatabase& DB) : DB(DB), Active(true) { }
		~Transaction()
		{
			if (DB.InTransaction && Active)
				DB.RollbackTransaction();
		}

		Transaction(const Transaction&) = delete;
		Transaction operator=(const Transaction&) = delete;

		Transaction(Transaction&& Src) : DB(Src.DB)
		{
			Move(std::move(Src));
		}
		Transaction operator=(Transaction&& Src)
		{
			Move(std::move(Src));
		}

	private:
		void Move(Transaction&& Src)
		{
			Active = Src.Active;
			Src.Active = false;
		}

		SQLiteDatabase& DB;
		bool Active = false;
	};

	Transaction BeginTransaction();
	void RollbackTransaction();
	void CommitTransaction();

	void Log(const char* Format, ...);

	SQLite sqlite = nullptr;

	bool InTransaction = false;

	std::unordered_map<const char*, SQLiteStatementStorage> PreparedStatements;
};