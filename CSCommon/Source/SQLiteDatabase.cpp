#include "stdafx.h"
#include "SQLiteDatabase.h"
#include <algorithm>
#include "MDebug.h"
#include "MMatchObject.h"
#include "MErrorTable.h"
#include "MMatchTransDataType.h"
#include <cstdarg>

template <>
int SQLiteStatement::Get<int>(int Column)
{
	return sqlite3_column_int(stmt, Column);
}

template <>
StringView SQLiteStatement::Get<StringView>(int Column)
{
	StringView ret;
	ret.Ptr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, Column));
	ret.Size = sqlite3_column_bytes(stmt, Column);
	return ret;
}

template <>
Blob SQLiteStatement::Get<Blob>(int Column)
{
	Blob ret;
	ret.Ptr = sqlite3_column_blob(stmt, Column);
	ret.Size = sqlite3_column_bytes(stmt, Column);
	return ret;
}

struct ItemBlob
{
	int32_t CIIDs[MMCIP_END];
	int32_t ItemIDs[MMCIP_END];
};

SQLiteDatabase::SQLiteDatabase()
	: sqlite("GunzDB.sq3")
{
	sqlite3_busy_timeout(sqlite, 5000);
	auto exec = [&](const char* sql)
	{
		char *err_msg = nullptr;
		auto err_code = sqlite3_exec(sqlite, sql, nullptr, nullptr, &err_msg);
		if (err_code != SQLITE_OK && err_msg)
			Log("Error during database construction: error code %d, error message: %s\n", err_code, err_msg);
	};

	exec("CREATE TABLE IF NOT EXISTS Login(AID integer NOT NULL, \
		UserID text UNIQUE, \
		PasswordData text, \
		LastConnDate text, \
		LastIP text)");

	exec("CREATE TABLE IF NOT EXISTS Account(AID integer PRIMARY KEY NOT NULL, \
		UserID text UNIQUE, \
		UGradeID integer, \
		PGradeID integer, \
		Email text, \
		RegDate datetime)");

	exec("CREATE TABLE IF NOT EXISTS Character( \
		CID integer PRIMARY KEY NOT NULL, \
		AID integer NOT NULL, \
		Name text NOT NULL, \
		Level integer NOT NULL, \
		Sex integer NOT NULL, \
		CharNum integer NOT NULL, \
		Hair integer NULL, \
		Face integer NULL, \
		XP integer NOT NULL, \
		BP integer NOT NULL, \
		Items blob NULL, \
		RegDate text NULL, \
		LastTime text NULL, \
		PlayTime integer NULL, \
		GameCount integer NULL, \
		KillCount integer NULL, \
		DeathCount integer NULL, \
		DeleteFlag integer NULL, \
		DeleteName text NULL, \
		QuestItemInfo blob NULL)");

	exec("CREATE TABLE IF NOT EXISTS CharacterMakingLog( \
		id integer PRIMARY KEY NOT NULL, \
		AID integer NULL, \
		CharName text NULL, \
		Type text NULL, \
		Date text NULL)");

	exec("CREATE TABLE IF NOT EXISTS CharacterItem( \
		CIID integer PRIMARY KEY NOT NULL, \
		CID integer NULL, \
		ItemID integer NOT NULL, \
		RegDate integer NULL, \
		RentDate integer NULL, \
		RentHourPeriod integer NULL, \
		Cnt integer NULL)");

	exec("CREATE TABLE IF NOT EXISTS Clan( \
		CLID integer PRIMARY KEY NOT NULL, \
		Name text NULL, \
		Exp integer NOT NULL, \
		Level integer NOT NULL, \
		Point integer NOT NULL, \
		MasterCID integer NULL, \
		Wins integer NOT NULL, \
		MarkWebImg text NULL, \
		Introduction text NULL, \
		RegDate text NOT NULL, \
		DeleteFlag text NULL, \
		DeleteName text NULL, \
		Homepage text NULL, \
		Losses integer NOT NULL, \
		Draws integer NOT NULL, \
		Ranking integer NOT NULL, \
		TotalPoint integer NOT NULL, \
		Cafe_Url text NULL, \
		Email text NULL, \
		EmblemUrl text NULL, \
		RankIncrease integer NOT NULL, \
		EmblemChecksum integer NOT NULL, \
		LastDayRanking integer NOT NULL, \
		LastMonthRanking integer NOT NULL)");

	exec("CREATE TABLE IF NOT EXISTS ClanMember( \
		CMID integer PRIMARY KEY NOT NULL, \
		CLID integer NULL, \
		CID integer NULL, \
		Grade integer NOT NULL, \
		RegDate text NOT NULL, \
		ContPoint integer NOT NULL)");

	exec("CREATE TABLE IF NOT EXISTS Friend( \
		id integer PRIMARY KEY NOT NULL, \
		CID integer NOT NULL, \
		FriendCID integer NOT NULL, \
		Type integer NOT NULL, \
		Favorite integer NULL, \
		DeleteFlag integer NULL)");
}

void SQLiteDatabase::HandleException(const SQLiteError & e)
{
	if (InTransaction)
		RollbackTransaction();

	Log("Caught SQLiteError: error code: %d, error message: %s\n", e.GetErrorCode(), e.what());
}

void SQLiteDatabase::BeginTransaction()
{
	InTransaction = true;
}

void SQLiteDatabase::RollbackTransaction()
{
	InTransaction = false;
}

void SQLiteDatabase::CommitTransaction()
{
	InTransaction = false;
}

void SQLiteDatabase::Log(const char * Format, ...)
{
	va_list va;
	va_start(va, Format);
	char buffer[512] = { 0 };
	vsprintf_safe(buffer, Format, va);
	va_end(va);

	MGetMatchServer()->Log(MMatchServer::LOG_ALL, buffer);
}

bool SQLiteDatabase::GetLoginInfo(const char * UserID, unsigned int * outAID, char * outPassword, size_t maxlen)
try
{
	auto stmt = ExecuteSQL("SELECT AID, PasswordData FROM Login WHERE UserID = ?", UserID);

	if (!stmt.HasRow())
		return false;

	auto AID = stmt.Get<int>();
	auto PasswordData = stmt.Get<StringView>();
	auto len = min(maxlen, PasswordData.Size);
	memcpy(outPassword, PasswordData.Ptr, len);
	outPassword[len] = 0;
	*outAID = AID;

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

AccountCreationResult SQLiteDatabase::CreateAccountNew(const char * Username,
	const char * PasswordData, size_t PasswordSize, const char * Email)
try
{
	auto stmt = ExecuteSQL("SELECT AID FROM Account WHERE UserID = ?", Username);

	if (stmt.HasRow())
		return AccountCreationResult::UsernameAlreadyExists;

	stmt = ExecuteSQL("INSERT INTO Account (UserID, UGradeID, PGradeID, RegDate, Email) VALUES (?, 0, 0, date('now'), ?)",
		Username, Email);

	stmt = ExecuteSQL("SELECT AID FROM Account WHERE UserID = ?",
		Username);

	if (!stmt.HasRow())
		return AccountCreationResult::DBError;

	auto AID = stmt.Get<int>(0);

	stmt = ExecuteSQL("INSERT INTO Login(UserID, AID, PasswordData) VALUES(?, ?, ?)",
		Username, AID, StringView{ PasswordData, PasswordSize });

	return AccountCreationResult::Success;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return AccountCreationResult::DBError;
}

bool SQLiteDatabase::UpdateCharLevel(int CID, int Level)
try
{
	auto stmt = ExecuteSQL("UPDATE Character SET Level = ? WHERE CID = ?", Level, CID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::InsertLevelUpLog(int nCID, int nLevel, int nBP, int nKillCount, int nDeathCount, int nPlayTime)
{
	return true;
}

bool SQLiteDatabase::UpdateLastConnDate(const char * UserID, const char * IP)
try
{
	auto stmt = ExecuteSQL("UPDATE Login SET LastConnDate = date('now'), LastIP = ? WHERE UserID = ?", IP, UserID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::BanPlayer(int nAID, const char* Reason, const time_t& UnbanTime)
try
{
	auto stmt = ExecuteSQL("UPDATE Account SET UGradeID = ? WHERE AID = ?", MMUG_BLOCKED, nAID);

	ExecuteSQL("INSERT INTO Blocks (AID, Type, Reason, EndDate) VALUES (?, ?, ?, ?)",
		nAID, MMBT_BANNED, Reason, UnbanTime);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

int SQLiteDatabase::CreateCharacter(int AID, const char * NewName, int CharIndex, int Sex, int Hair, int Face, int Costume)
try
{
	auto stmt = ExecuteSQL("SELECT COUNT(*) AS NUM FROM Character WHERE Name = ?", NewName);

	if (!stmt.HasRow())
		return MERR_UNKNOWN;

	if (stmt.Get<int>() > 0)
		return MERR_CLIENT_EXIST_CHARNAME;

	ExecuteSQL("INSERT INTO Character (AID, Name, CharNum, Level, Sex, Hair, Face, XP, BP, \
		GameCount, KillCount, DeathCount, RegDate, PlayTime, DeleteFlag) \
		Values(?, ?, ?, 1, ?, ?, ?, 0, 0, \
		0, 0, 0, date('now'), 0, 0)",
		AID, NewName, CharIndex, Sex, Hair, Face);

	stmt = ExecuteSQL("SELECT CID FROM Character WHERE AID = ? AND CharNum = ?\n", AID, CharIndex);

	if (!stmt.HasRow() || stmt.IsNull())
	{
		Log("SQLiteDatabase::CreateCharacter - Weirdness while creating character named %s for AID %d\n",
			NewName, AID);
		return false;
	}

	auto CID = stmt.Get<int>();

	std::pair<int, int> GenderedCostume[2][2] = { { { MMCIP_CHEST, 21001 },{ MMCIP_LEGS, 23001 }},
	{{ MMCIP_CHEST, 21501 },{ MMCIP_LEGS, 23501 }} };
	std::pair<int, int> UnisexCostume[] = { { MMCIP_MELEE, 2 }, { MMCIP_PRIMARY, 5002 } };

	ItemBlob Items;
	for (auto& e : Items.ItemIDs)
		e = 0;
	for (auto& e : Items.CIIDs)
		e = 0;

	auto SetItem = [&](auto& Parts)
	{
		size_t Index = Parts.first;
		auto ItemID = Parts.second;
		Items.ItemIDs[Index] = ItemID;
		ExecuteSQL("INSERT INTO CharacterItem (CID, ItemID) VALUES (?, ?)", CID, ItemID);
		auto CIID = sqlite3_last_insert_rowid(sqlite);
		Items.CIIDs[Index] = CIID;
	};

	for (auto& Pair : GenderedCostume[Sex])
		SetItem(Pair);
	for (auto& Pair : UnisexCostume)
		SetItem(Pair);

	ExecuteSQL("UPDATE Character SET Items = ? WHERE AID = ? AND CID = ?",
		Blob{ &Items, sizeof(Items) }, AID, CID);

	return MOK;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::DeleteCharacter(int AID, int CharIndex, const char * CharName)
try
{
	auto stmt = ExecuteSQL("SELECT CID FROM Character WHERE AID = ? AND CharNum = ?", AID, CharIndex);

	if (!stmt.HasRow())
		return false;

	auto CID = stmt.Get<int>();

	stmt = ExecuteSQL("SELECT COUNT(*) AS CashItemCount FROM CharacterItem WHERE CID = ? AND ItemID >= 500000", CID);

	if (stmt.HasRow())
	{
		auto CashItemCount = stmt.Get<int>();

		if (CashItemCount > 0)
			return false;
	}

	stmt = ExecuteSQL("UPDATE Character SET CharNum = -1, DeleteFlag = 1, Name = '', DeleteName = ? \
		WHERE AID = ? AND CharNum = ?",
		CharName, AID, CharIndex);

	InsertCharMakingLog(AID, CharName, CharMakingType::Delete);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::InsertCharMakingLog(unsigned int AID, const char * CharName, CharMakingType Type)
try
{
	ExecuteSQL("INSERT INTO CharacterMakingLog(AID, CharName, Type, Date) \
		VALUES(?, ?, ?, date('now'))",
		AID, CharName, Type);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetAccountCharList(int AID, MTD_AccountCharInfo * outCharList, int * outCharCount)
try
{
	auto stmt = ExecuteSQL("SELECT Name, CharNum, Level \
		FROM Character \
		WHERE AID = ? AND DeleteFlag = 0",
		AID);

	int i = 0;
	while (stmt.HasRow() && i < MAX_CHAR_COUNT)
	{
		strcpy_safe(outCharList[i].szName, stmt.Get<StringView>().Ptr);
		outCharList[i].nCharNum = stmt.Get<int>();
		outCharList[i].nLevel = stmt.Get<int>();
		stmt.Step();
		i++;
	}

	*outCharCount = i;

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetAccountCharInfo(int AID, int CharIndex, MTD_CharInfo * outCharInfo)
try
{
	auto stmt = ExecuteSQL("SELECT CID FROM Character WHERE AID = ? and CharNum = ?",
		AID, CharIndex);

	if (!stmt.HasRow())
		return false;

	auto CID = stmt.Get<int>();

	stmt = ExecuteSQL("SELECT Name, CharNum, Level, Sex, Hair, Face, XP, BP, \
		(SELECT cl.Name FROM Clan cl, ClanMember cm WHERE cm.cid = ?1 AND cm.CLID = cl.CLID) AS ClanName, \
		Items \
		FROM Character \
		WHERE CID = ?1",
		CID);

	if (!stmt.HasRow())
		return false;

	strcpy_safe(outCharInfo->szName, stmt.Get<StringView>().Ptr);
	outCharInfo->nCharNum = stmt.Get<int>();
	outCharInfo->nLevel = stmt.Get<int>();
	outCharInfo->nSex = stmt.Get<int>();
	outCharInfo->nHair = stmt.Get<int>();
	outCharInfo->nFace = stmt.Get<int>();
	outCharInfo->nXP = stmt.Get<int>();
	outCharInfo->nBP = stmt.Get<int>();

	if (!stmt.IsNull())
	{
		strcpy_safe(outCharInfo->szClanName, stmt.Get<StringView>().Ptr);
	}
	else
	{
		for (auto& e : outCharInfo->szClanName)
			e = 0;

		stmt.NextColumn();
	}

	if (!stmt.IsNull())
	{
		auto ItemsBlob = stmt.Get<Blob>();
		auto& Items = *static_cast<const ItemBlob*>(ItemsBlob.Ptr);
		std::copy(std::begin(Items.ItemIDs), std::end(Items.ItemIDs), outCharInfo->nEquipedItemDesc);
	}
	else
	{
		std::fill(std::begin(outCharInfo->nEquipedItemDesc), std::end(outCharInfo->nEquipedItemDesc), 0);

		stmt.NextColumn();
	}

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetAccountInfo(int AID, MMatchAccountInfo * outAccountInfo)
try
{
	auto stmt = ExecuteSQL("SELECT UserID, UGradeID \
		FROM Account WHERE AID = ?",
		AID);

	if (!stmt.HasRow())
		return false;

	outAccountInfo->m_nAID = AID;
	strcpy_safe(outAccountInfo->m_szUserID, stmt.Get<StringView>().Ptr);
	outAccountInfo->m_nUGrade = static_cast<MMatchUserGradeID>(stmt.Get<int>());

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetCharInfoByAID(int AID, int CharIndex, MMatchCharInfo * outCharInfo, int & nWaitHourDiff)
try
{
	auto stmt = ExecuteSQL("SELECT CID FROM Character WHERE AID = ? and CharNum = ?",
		AID, CharIndex);

	if (!stmt.HasRow())
		return false;

	auto CID = stmt.Get<int>();

	stmt = ExecuteSQL("SELECT Name, Level, Sex, CharNum, Hair, Face, \
		XP, BP, GameCount, KillCount, DeathCount, PlayTime, Items \
		FROM Character \
		WHERE CID = ?",
		CID);

	if (!stmt.HasRow())
		return false;

	outCharInfo->m_nCID = CID;
	if (!stmt.IsNull())
		strcpy_safe(outCharInfo->m_szName, stmt.Get<StringView>().Ptr);
	else
		stmt.NextColumn();
	outCharInfo->m_nLevel = stmt.Get<int>();
	outCharInfo->m_nSex = static_cast<MMatchSex>(stmt.Get<int>());
	outCharInfo->m_nCharNum = stmt.Get<int>();
	outCharInfo->m_nHair = stmt.Get<int>();
	outCharInfo->m_nFace = stmt.Get<int>();
	outCharInfo->m_nXP = stmt.Get<int>();
	outCharInfo->m_nBP = stmt.Get<int>();
	stmt.NextColumn();
	outCharInfo->m_nTotalKillCount = stmt.Get<int>();
	outCharInfo->m_nTotalDeathCount = stmt.Get<int>();
	outCharInfo->m_nTotalPlayTimeSec = stmt.Get<int>();
	if (!stmt.IsNull())
	{
		auto ItemsBlob = stmt.Get<Blob>();
		auto& Items = *reinterpret_cast<const ItemBlob*>(ItemsBlob.Ptr);
		std::copy(std::begin(Items.CIIDs), std::end(Items.CIIDs), outCharInfo->m_nEquipedItemCIID);
	}
	else
	{
		std::fill(std::begin(outCharInfo->m_nEquipedItemCIID), std::end(outCharInfo->m_nEquipedItemCIID), 0);

		stmt.NextColumn();
	}

	stmt = ExecuteSQL("SELECT cl.CLID, cl.Name, cm.Grade, cm.ContPoint \
		FROM ClanMember cm, Clan cl \
		WHERE cm.cid = ? AND cm.CLID = cl.CLID",
		CID);

	if (stmt.HasRow())
	{
		outCharInfo->m_ClanInfo.m_nClanID = stmt.Get<int>();
		if (!stmt.IsNull())
		{
			strcpy_safe(outCharInfo->m_ClanInfo.m_szClanName, stmt.Get<StringView>().Ptr);
		}
		else
		{
			for (auto& e : outCharInfo->m_ClanInfo.m_szClanName)
				e = 0;
			stmt.NextColumn();
		}
		outCharInfo->m_ClanInfo.m_nGrade = static_cast<MMatchClanGrade>(stmt.Get<int>());
		outCharInfo->m_ClanInfo.m_nContPoint = stmt.Get<int>();
	}
	else
	{
		outCharInfo->m_ClanInfo.m_nClanID = 0;
		for (auto& c : outCharInfo->m_ClanInfo.m_szClanName)
			c = 0;
		outCharInfo->m_ClanInfo.m_nGrade = MCG_NONE;
		outCharInfo->m_ClanInfo.m_nContPoint = 0;
	}

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetCharCID(const char * Name, int * outCID)
try
{
	auto stmt = ExecuteSQL("SELECT CID \
		FROM Character \
		WHERE Name = ?",
		Name);

	if (!stmt.HasRow())
		return false;

	*outCID = stmt.Get<int>();

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::SimpleUpdateCharInfo(const MMatchCharInfo& CharInfo)
try
{
	ExecuteSQL("UPDATE Character \
		SET Level = ?, XP = ?, BP = ? \
		WHERE CID = ?",
		CharInfo.m_nLevel, CharInfo.m_nXP, CharInfo.m_nBP,
		CharInfo.m_nCID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::UpdateCharBP(int CID, int BPInc)
try
{
	ExecuteSQL("UPDATE Character \
		SET BP = BP + ? \
		WHERE CID = ?",
		BPInc, CID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::UpdateCharInfoData(int CID, int AddedXP, int AddedBP, int AddedKillCount, int AddedDeathCount)
try
{
	ExecuteSQL("UPDATE Character \
		SET XP = XP + ?, BP = BP + ?, KillCount = KillCount + ?, DeathCount = DeathCount + ? \
		WHERE CID = ?",
		AddedXP, AddedBP, AddedKillCount, AddedDeathCount,
		CID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::InsertCharItem(unsigned int CID, int ItemID, bool RentItem, int RentPeriodHour,
	unsigned long * outCIID)
try
{
	ExecuteSQL("INSERT INTO CharacterItem (CID, ItemID, RegDate) \
		Values (?, ?, date('now')))",
		CID, ItemID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::DeleteCharItem(unsigned int CID, int CIID)
try
{
	ExecuteSQL("UPDATE CharacterItem SET CID = NULL \
		WHERE CID = ? AND CIID = ?",
		CID, CIID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

#define RENT_MINUTE_PERIOD_UNLIMITED	(525600)	// 클라이언트한테는 기간제 아이템 기간을 minute단위로 보낸다. 525600이면 무제한(1년)

bool SQLiteDatabase::GetCharItemInfo(MMatchCharInfo& CharInfo)
try
{
	auto stmt = ExecuteSQL("SELECT CIID, ItemID, \
		(RentHourPeriod*60) - CAST((JulianDay(datetime('now')) - JulianDay(RentDate)) * 24 * 60 As Integer) \
		AS RentPeriodRemainder \
		FROM CharacterItem \
		WHERE CID = ? ORDER BY CIID",
		CharInfo.m_nCID);

	while (stmt.HasRow())
	{
		auto CIID = stmt.Get<int>();
		auto ItemDescID = stmt.Get<int>();

		auto IsRentItem = !stmt.IsNull();
		auto RentMinutePeriodRemainder = 0;
		if (IsRentItem)
		{
			RentMinutePeriodRemainder = stmt.Get<int>();
		}
		else
		{
			RentMinutePeriodRemainder = RENT_MINUTE_PERIOD_UNLIMITED;
			stmt.NextColumn();
		}

		MUID uidNew = MMatchItemMap::UseUID();
		CharInfo.m_ItemList.CreateItem(uidNew, CIID, ItemDescID, IsRentItem, RentMinutePeriodRemainder);

		stmt.Step();
	}

	CharInfo.m_ItemList.SetDbAccess();

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetAccountItemInfo(int AID, MAccountItemNode * outItemNode,
	int * outNodeCount, int MaxNodeCount, MAccountItemNode * outExpiredItemList,
	int * outExpiredItemCount, int MaxExpiredItemCount)
try
{
	auto stmt = ExecuteSQL("SELECT AIID, ItemID, \
		(RentHourPeriod*60) - CAST((JulianDay(datetime('now')) - JulianDay(RentDate)) * 24 * 60 As Integer) \
		 AS RentPeriodRemainder \
		FROM AccountItem \
		WHERE AID = ? ORDER BY AIID",
		AID);

	int NodeCount;
	int ExpiredItemCount;

	while (stmt.HasRow())
	{
		auto aiid = stmt.Get<int>();
		unsigned long itemid = stmt.Get<int>();

		auto IsRentItem = !stmt.IsNull();
		auto RentMinutePeriodRemainder = 0;
		if (IsRentItem)
		{
			RentMinutePeriodRemainder = stmt.Get<int>();
		}
		else
		{
			RentMinutePeriodRemainder = RENT_MINUTE_PERIOD_UNLIMITED;
			stmt.NextColumn();
		}

		if (IsRentItem && RentMinutePeriodRemainder <= 0)
		{
			if (ExpiredItemCount < MaxExpiredItemCount)
			{
				outExpiredItemList[ExpiredItemCount].nAIID = aiid;
				outExpiredItemList[ExpiredItemCount].nItemID = itemid;
				ExpiredItemCount++;
			}
		}
		else
		{
			outItemNode[NodeCount].nAIID = aiid;
			outItemNode[NodeCount].nItemID = itemid;
			outItemNode[NodeCount].nRentMinutePeriodRemainder = RentMinutePeriodRemainder;

			NodeCount++;
			if (NodeCount >= MaxNodeCount) break;
		}

		stmt.Step();
	}

	*outNodeCount = NodeCount;
	*outExpiredItemCount = ExpiredItemCount;

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::UpdateEquipedItem(unsigned long CID, MMatchCharItemParts parts, unsigned long int CIID,
	unsigned long int ItemID)
try
{
	auto stmt = ExecuteSQL("SELECT Items FROM Character WHERE CID = ?", CID);

	ItemBlob Items;

	if (stmt.HasRow() && !stmt.IsNull())
	{
		auto Blob = stmt.Get<StringView>();
		if (Blob.Size != sizeof(ItemBlob))
		{
			Log("SQLiteDatabase::UpdateEquipedItem - Items blob has wrong size %d, expected %d\n",
				Blob.Size, sizeof(ItemBlob));
			return false;
		}
		memcpy(&Items, Blob.Ptr, sizeof(Items));
	}
	else
	{
		for (auto& e : Items.CIIDs)
			e = 0;
		for (auto& e : Items.ItemIDs)
			e = 0;
	}

	Items.CIIDs[parts] = CIID;
	Items.ItemIDs[parts] = ItemID;

	ExecuteSQL("UPDATE Character SET Items = ? WHERE CID = ?", Blob{ &Items, sizeof(Items) }, CID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::ClearAllEquipedItem(unsigned long CID)
try
{
	ExecuteSQL("UPDATE Character SET Items = NULL WHERE CID = ?", CID);
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::DeleteExpiredAccountItem(int AIID)
try
{
	ExecuteSQL("DELETE FROM AccountItem WHERE AIID = ? AND RentDate IS NOT NULL", AIID);
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::BuyBountyItem(unsigned int CID, int ItemID, int Price, unsigned long * outCIID)
try
{
	auto stmt = ExecuteSQL("SELECT BP FROM Character WHERE CID = ?", CID);
	if (!stmt.HasRow() || stmt.IsNull() || stmt.Get<int>() < Price)
		return false;

	ExecuteSQL("UPDATE Character SET BP = BP - ? WHERE CID = ?", Price, CID);
	ExecuteSQL("INSERT INTO CharacterItem (CID, ItemID, RegDate) Values (?, ?, date('now'))", CID, ItemID);

	// TODO: Insert purchase log

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::SellBountyItem(unsigned int CID, unsigned int ItemID, unsigned int CIID, int Price, int CharBP)
try
{
	ExecuteSQL("UPDATE CharacterItem SET CID = NULL WHERE CID = ? AND CIID = ?", CID, CIID);
	ExecuteSQL("UPDATE Character SET BP = BP + ? WHERE CID = ?", Price, CID);

	// TODO: Insert sell log

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::UpdateQuestItem(int nCID, MQuestItemMap & rfQuestIteMap, MQuestMonsterBible & rfQuestMonster)
try
{
	// TODO: Implement

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetCharQuestItemInfo(MMatchCharInfo * pCharInfo)
{
	// TODO: Implement

	return true;
}

bool SQLiteDatabase::InsertQuestGameLog(const char * pszStageName, int nScenarioID, int nMasterCID, int nPlayer1, int nPlayer2, int nPlayer3, int nTotalRewardQItemCount, int nElapsedPlayTime, int & outQGLID)
{
	return true;
}

bool SQLiteDatabase::InsertQUniqueGameLog(int nQGLID, int nCID, int nQIID)
{
	// TODO: Implement

	return true;
}

bool SQLiteDatabase::InsertConnLog(int nAID, const char * szIP, const std::string & strCountryCode3)
{
	// TODO: Implement

	return true;
}

bool SQLiteDatabase::InsertGameLog(const char * szGameName, const char * szMap, const char * GameType, int nRound, unsigned int nMasterCID, int nPlayerCount, const char * szPlayers)
{
	// TODO: Implement

	return true;
}

bool SQLiteDatabase::InsertKillLog(unsigned int nAttackerCID, unsigned int nVictimCID)
{
	// TODO: Implement

	return true;
}

bool SQLiteDatabase::InsertChatLog(unsigned long int nCID, const char * szMsg, unsigned long int nTime)
{
	// TODO: Implement

	return true;
}

bool SQLiteDatabase::InsertServerLog(int nServerID, int nPlayerCount, int nGameCount, uint32_t dwBlockCount, uint32_t dwNonBlockCount)
{
	// TODO: Implement

	return true;
}

bool SQLiteDatabase::InsertPlayerLog(unsigned long int nCID, int nPlayTime, int nKillCount, int nDeathCount, int nXP, int nTotalXP)
{
	// TODO: Implement

	return true;
}

bool SQLiteDatabase::UpdateCharPlayTime(unsigned long CID, unsigned long PlayTime)
try
{
	ExecuteSQL("UPDATE Character SET PlayTime = PlayTime + ?, LastTime = date('now') WHERE CID = ?", CID);
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::BringAccountItem(int AID, int CID, int AIID, unsigned int * outCIID, unsigned long int * outItemID, bool * outIsRentItem, int * outRentMinutePeriodRemainder)
try
{
	auto stmt = ExecuteSQL("SELECT ItemID, RentDate, RentHourPeriod, Cnt \
		FROM AccountItem WHERE AIID = ?", AIID);
	if (!stmt.HasRow())
		return false;

	auto ItemID = stmt.Get<int>();
	auto RentDate = stmt.Get<StringView>();
	auto RentHourPeriod = stmt.Get<int>();
	auto Cnt = stmt.Get<int>();

	ExecuteSQL("DELETE FROM AccountItem WHERE AIID = ?", AIID);

	ExecuteSQL("INSERT INTO CharacterItem(CID, ItemID, RegDate, RentDate, RentHourPeriod, Cnt) \
		VALUES(?, ?, date('now'), ?, ?, ?)",
		CID, ItemID, RentDate, RentHourPeriod, Cnt);

	// TODO: Log

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::BringBackAccountItem(int AID, int CID, int CIID)
try
{
	BeginTransaction();

	// TODO: Check that the item isn't equipped

	auto stmt = ExecuteSQL("SELECT ItemID, RentDate, @RentHourPeriod=RentHourPeriod, Cnt \
		FROM CharacterItem WHERE CIID = ? AND CID = ?", CIID, CID);

	if (!stmt.HasRow())
	{
		RollbackTransaction();
		return false;
	}

	auto ItemID = stmt.Get<int>();
	auto RentDate = stmt.Get<int>();
	auto RentHourPeriod = stmt.Get<int>();
	auto Cnt = stmt.Get<int>();

	ExecuteSQL("UPDATE CharacterItem SET CID = NULL WHERE CIID = ? AND CID = ?", CIID, CID);

	ExecuteSQL("INSERT INTO AccountItem(AID, ItemID, RentDate, RentHourPeriod, Cnt) \
		VALUES(?, ?, ?, ?, ?)",
		AID, ItemID, RentDate, RentHourPeriod, Cnt);

	CommitTransaction();
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::FriendAdd(int CID, int FriendCID, int Favorite)
try
{
	ExecuteSQL("INSERT INTO Friend(CID, FriendCID, Favorite, DeleteFlag, Type) \
		Values (?, ?, ?, 0, 1)",
		CID, FriendCID, Favorite);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::FriendRemove(int CID, int FriendCID)
try
{
	ExecuteSQL("UPDATE Friend \
		SET DeleteFlag = 1 \
		WHERE CID = ? AND FriendCID = ?",
		CID, FriendCID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::FriendGetList(int CID, MMatchFriendInfo * FriendInfo)
try
{
	auto stmt = ExecuteSQL("SELECT f.FriendCID, f.Favorite, c.Name \
		FROM Friend f, Character c \
		WHERE f.CID = ? AND f.FriendCID = c.CID AND f.DeleteFlag = 0 AND f.Type = 1",
		CID);

	while (stmt.HasRow())
	{
		auto FriendCID = stmt.Get<int>();
		auto Favorite = stmt.Get<int>();
		auto Name = stmt.Get<StringView>();
		FriendInfo->Add(FriendCID, Favorite, Name.Ptr);
		stmt.Step();
	};

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetCharClan(int CID, int * outClanID, char * outClanName, int maxlen)
try
{
	auto stmt = ExecuteSQL("SELECT cl.CLID AS CLID, cl.Name AS ClanName \
		FROM ClanMember cm, Clan cl \
		WHERE cm.cid = ? AND cm.CLID = cl.CLID",
		CID);

	if (!stmt.HasRow())
		return false;

	*outClanID = stmt.Get<int>();
	strcpy_safe(outClanName, maxlen, stmt.Get<StringView>().Ptr);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetClanIDFromName(const char * ClanName, int * outCLID)
try
{
	auto stmt = ExecuteSQL("SELECT CLID FROM Clan WHERE Name = ?", ClanName);

	if (!stmt.HasRow())
		return false;

	*outCLID = stmt.Get<int>();

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::CreateClan(const char * ClanName, int MasterCID, int Member1CID, int Member2CID, int Member3CID, int Member4CID, bool * outRet, int * outNewCLID)
try
{
	auto stmt = ExecuteSQL("SELECT CLID FROM Clan WHERE Name = ?",
		ClanName);

	if (stmt.HasRow())
		return false;

	stmt = ExecuteSQL("SELECT COUNT(*) FROM ClanMember cm Character c \
		WHERE((cm.CID = ?) OR(cm.CID = ?) OR(cm.CID = ?) OR(cm.CID = ?) OR \
		(cm.CID = ?)) AND cm.CID = c.CID AND c.DeleteFlag = 0",
		MasterCID, Member1CID, Member2CID, Member3CID, Member4CID);

	if (stmt.HasRow())
		return false;

	BeginTransaction();

	ExecuteSQL("INSERT INTO Clan(Name, MasterCID, RegDate) VALUES(?, ?, date('now'))",
		ClanName, MasterCID);

	stmt = ExecuteSQL("SELECT CLID FROM Clan WHERE MasterCID = ?",
		MasterCID);

	if (!stmt.HasRow())
	{
		RollbackTransaction();
		return false;
	}

	auto CLID = stmt.Get<int>();

	ExecuteSQL("INSERT INTO ClanMember(CLID, CID, Grade, RegDate) VALUES(?, ?, 1, date('now'))",
		CLID, MasterCID);

	int MemberCIDs[] = { Member1CID, Member2CID, Member3CID, Member4CID };

	for (auto CID : MemberCIDs)
		ExecuteSQL("INSERT INTO ClanMember(CLID, CID, Grade, RegDate) VALUES(?, ?, 9, date('now'))",
			CLID, CID);

	CommitTransaction();

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::DeleteExpiredClan(uint32_t dwCID, uint32_t dwCLID, const std::string & strDeleteName, uint32_t dwWaitHour)
try
{
	// Unimplemented in MSSQL

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::SetDeleteTime(uint32_t dwMasterCID, uint32_t dwCLID, const std::string & strDeleteDate)
try
{
	// Unimplemented in MSSQL

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::ReserveCloseClan(int CLID, const char * ClanName, int MasterCID, const std::string & strDeleteDate)
try
{
	ExecuteSQL("UPDATE Clan SET DeleteFlag = 2 WHERE CLID = ? AND Name = ? AND MasterCID = ?",
		CLID, ClanName, MasterCID);
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::AddClanMember(int CLID, int JoinerCID, int ClanGrade, bool * outRet)
try
{
	ExecuteSQL("INSERT INTO ClanMember(CLID, CID, Grade, RegDate) VALUES(?, ?, ?, date('now'))",
		CLID, JoinerCID, ClanGrade);
	*outRet = true;
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::RemoveClanMember(int CLID, int LeaverCID)
try
{
	ExecuteSQL("DELETE FROM ClanMember WHERE CLID = ? AND CID = ? AND Grade != 1", CLID, LeaverCID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::UpdateClanGrade(int CLID, int MemberCID, int ClanGrade)
try
{
	ExecuteSQL("UPDATE ClanMember SET Grade = ? WHERE CLID = ? AND CID = @CID", ClanGrade, CLID, MemberCID);
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::ExpelClanMember(int CLID, int AdminGrade, char * MemberName, int * outRet)
try
{
	auto stmt = ExecuteSQL("SELECT c.cid, cm.Grade FROM Character c, ClanMember cm \
		WHERE cm.clid = ? AND c.cid = cm.cid AND c.Name = ? AND DeleteFlag = 0",
		CLID, MemberName);

	if (!stmt.HasRow())
	{
		*outRet = 0;
		return false;
	}

	if (stmt.IsNull())
	{
		*outRet = 0;
		return false;
	}

	auto CID = stmt.Get<int>();
	auto Grade = stmt.Get<int>();

	if (Grade >= AdminGrade)
	{
		*outRet = 2;
		return false;
	}

	ExecuteSQL("DELETE FROM ClanMember WHERE CLID = ? AND CID = ? AND Grade != 1", CLID, CID);

	*outRet = 1;
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	*outRet = 3;
	return false;
}

bool SQLiteDatabase::GetClanInfo(int CLID, MDB_ClanInfo * outClanInfo)
try
{
	auto stmt = ExecuteSQL("SELECT cl.Name AS Name, cl.TotalPoint AS TotalPoint, cl.Level AS Level, cl.Ranking AS Ranking, \
		cl.Point AS Point, cl.Wins AS Wins, cl.Losses AS Losses, cl.Draws AS Draws, \
		c.Name AS ClanMaster, \
		(SELECT COUNT(*) FROM ClanMember WHERE CLID = ?1) AS MemberCount, \
		cl.EmblemUrl AS EmblemUrl, cl.EmblemChecksum AS EmblemChecksum \
		FROM Clan cl, Character c \
		WHERE cl.CLID = ?1 and cl.MasterCID = c.CID",
		CLID);

	if (!stmt.HasRow())
		return false;

	outClanInfo->nCLID = CLID;
	strcpy_safe(outClanInfo->szClanName, stmt.Get<StringView>().Ptr);
	outClanInfo->nTotalPoint = stmt.Get<int>();
	outClanInfo->nLevel = stmt.Get<int>();
	outClanInfo->nRanking = stmt.Get<int>();
	outClanInfo->nPoint = stmt.Get<int>();
	outClanInfo->nWins = stmt.Get<int>();
	outClanInfo->nLosses = stmt.Get<int>();
	strcpy_safe(outClanInfo->szMasterName, stmt.Get<StringView>().Ptr);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::UpdateCharClanContPoint(int CID, int CLID, int AddedContPoint)
try
{
	ExecuteSQL("UPDATE ClanMember SET ContPoint = ContPoint + ? WHERE CID = ? AND CLID = ?", AddedContPoint, CID, CLID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetLadderTeamID(const int nTeamTableIndex, const int * pnMemberCIDArray, int nMemberCount, int * pnoutTID)
try
{
	// Unimplemented in MSSQL

	return false;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::LadderTeamWinTheGame(int nTeamTableIndex, int nWinnerTID, int nLoserTID, bool bIsDrawGame, int nWinnerPoint, int nLoserPoint, int nDrawPoint)
try
{
	// Unimplemented in MSSQL

	return false;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetLadderTeamMemberByCID(const int nCID, int * poutTeamID, char ** ppoutCharArray, int maxlen, int nCount)
try
{
	// Unimplemented in MSSQL

	return false;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::WinTheClanGame(int WinnerCLID, int LoserCLID, bool IsDrawGame, int WinnerPoint, int LoserPoint,
	const char * WinnerClanName, const char * LoserClanName, int RoundWins, int RoundLosses, int MapID,
	int GameType, const char * WinnerMembers, const char * LoserMembers)
	try
{
	if (IsDrawGame)
	{
		return true;
	}

	BeginTransaction();

	ExecuteSQL("UPDATE Clan SET Wins = Wins + 1, Point = Point + ?1, TotalPoint = TotalPoint + ?1 WHERE CLID = ?2", WinnerPoint, WinnerCLID);
	ExecuteSQL("UPDATE Clan SET Losses = Losses + 1, Point = max(0, Point + ?) WHERE CLID = ?", LoserPoint, LoserCLID);
	ExecuteSQL("INSERT INTO ClanGameLog(WinnerCLID, LoserCLID, WinnerClanName, LoserClanName, RoundWins, RoundLosses, \
		MapID, GameType, RegDate, WinnerMembers, LoserMembers, WinnerPoint, LoserPoint) \
		VALUES(?, ?, ?, ?, ?, ?, \
		?, ?, date('now'), ?, ?, ?, ?)",
		WinnerCLID, LoserCLID, WinnerClanName, LoserClanName, RoundWins, RoundLosses,
		MapID, GameType, WinnerMembers, LoserMembers, WinnerPoint, LoserPoint);

	CommitTransaction();

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::UpdateCharLevel(int CID, int NewLevel, int BP, int KillCount, int DeathCount, int PlayTime, bool IsLevelUp)
try
{
	ExecuteSQL("UPDATE Character SET Level = ? WHERE CID = ?", NewLevel, CID);
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::EventJjangUpdate(int AID, bool Jjang)
try
{
	auto UGradeID = Jjang ? MMUG_STAR : MMUG_FREE;
	ExecuteSQL("UPDATE Account SET UGradeID = ? WHERE AID = ?", UGradeID, AID);
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::CheckPremiumIP(const char * szIP, bool & outbResult)
try
{
	// Unimplemented in MSSQL

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetCID(const char * CharName, int & outCID)
try
{
	auto stmt = ExecuteSQL("SELECT CID FROM Character WHERE Name = ?", CharName);

	if (!stmt.HasRow())
		return false;

	outCID = stmt.Get<int>();

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetCharName(int CID, std::string & outCharName)
try
{
	auto stmt = ExecuteSQL("SELECT Name FROM Character WHERE CID = ?", CID);

	if (!stmt.HasRow() || stmt.IsNull())
		return false;

	outCharName = stmt.Get<StringView>().Ptr;

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetIPCountryCode(const std::string & strIP, uint32_t & dwOutIPFrom, uint32_t & dwOutIPTo, std::string & strOutCountryCode)
try
{
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetBlockCountryCodeList(BlockCountryCodeList & rfBlockCountryCodeList)
try
{
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetIPtoCountryList(IPtoCountryList & rfIPtoCountryList)
try
{
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetCustomIP(const std::string & strIP, uint32_t & dwIPFrom, uint32_t & dwIPTo, bool & bIsBlock, std::string & strCountryCode3, std::string & strComment)
try
{
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetCustomIPList(CustomIPList & rfCustomIPList)
try
{
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::InsertEvent(uint32_t dwAID, uint32_t dwCID, const std::string & strEventName)
try
{
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::SetBlockAccount(uint32_t dwAID, uint32_t dwCID, uint8_t btBlockType, const std::string & strComment, const std::string & strIP, const std::string & strEndHackBlockerDate)
try
{
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::ResetAccountBlock(uint32_t dwAID, uint8_t btBlockType)
try
{
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::InsertBlockLog(uint32_t dwAID, uint32_t dwCID, uint8_t btBlockType, const std::string & strComment, const std::string & strIP)
try
{
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::AdminResetAllHackingBlock()
try
{
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}