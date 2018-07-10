#include <vector>
#include <algorithm>
#include <random>
#include <array>
#include <chrono>
#include "IDatabase.h"
#include "SQLiteDatabase.h"
#include "MSSQLDatabase.h"
#include "MMatchConfig.h"
#include "MMatchGlobal.h"
#include "MMatchObject.h"
#include "MMatchTransDataType.h"
#include "MFile.h"
#include "StringView.h"
#include "ArrayView.h"
#include "optional.h"
#include "reinterpret.h"
#include "IniParser.h"
#include "TestAssert.h"

namespace TestDBInternal {
namespace {

#include "TestRandom.h"

// These are chosen to match the database's schema restrictions.
constexpr size_t UsernameLength = 16;
// While the length of the char name field in the schema is 24, the length of the
// MDB_ClanInfo::szMasterName data member is CLAN_NAME_LENGTH (which is lower).
constexpr size_t CharNameLength = CLAN_NAME_LENGTH;
constexpr size_t ClanNameLength = CLAN_NAME_LENGTH;

struct AccountCharList
{
	MTD_AccountCharInfo CharList[MAX_CHAR_COUNT];
	int CharCount;
};

optional<AccountCharList> GetCharList(IDatabase* DB, u32 AID)
{
	AccountCharList Ret;
	bool x = DB->GetAccountCharList(AID, Ret.CharList, &Ret.CharCount);
	if (!x)
		return nullopt;
	return Ret;
};

u32 CreateAccount(IDatabase* DB)
{
	char Username[UsernameLength];
	char Password[32];
	char Email[32];

	for (auto String : std::initializer_list<ArrayView<char>>{Username, Password, Email})
		RandomString(String);

	u32 AID;
	char RetrievedPassword[256];
	auto GetLogin = [&] {
		return DB->GetLoginInfo(Username, &AID, RetrievedPassword);
	};

	auto Create = [&] {
		return DB->CreateAccountNew(Username, Password, std::size(Password), Email);
	};

	TestAssert(!GetLogin());
	TestAssert(Create() == AccountCreationResult::Success);
	TestAssert(GetLogin());
	TestAssert(equals(Password, RetrievedPassword));
	TestAssert(Create() == AccountCreationResult::UsernameAlreadyExists);

	MMatchAccountInfo AccountInfo{};
	auto GetAccountInfo = [&] {
		return DB->GetAccountInfo(AID, &AccountInfo);
	};

	TestAssert(GetAccountInfo());
	TestAssert(AccountInfo.m_nAID == AID);
	TestAssert(equals(AccountInfo.m_szUserID, Username));

	auto&& CharList = GetCharList(DB, AID);
	TestAssert(CharList);
	TestAssert(CharList->CharCount == 0);

	return AID;
};

struct CharAttributes
{
	int CharIndex, Sex, Hair, Face, Costume;
};

CharAttributes RandomCharAttributes()
{
	CharAttributes Attr;
	Attr.CharIndex = 0;
	Attr.Sex = RandomNumber(1);
	Attr.Hair = RandomNumber(MAX_COSTUME_HAIR - 1);
	Attr.Face = RandomNumber(MAX_COSTUME_FACE - 1);
	Attr.Costume = RandomNumber(MAX_COSTUME_TEMPLATE - 1);
	return Attr;
}

MMatchCharInfo CreateChar(IDatabase* DB, u32 AID, const CharAttributes& Attr)
{
	auto GetCharInfo = [&] {
		MTD_CharInfo ci{};
		return DB->GetAccountCharInfo(AID, Attr.CharIndex, &ci);
	};

	MMatchCharInfo MCharInfo;
	auto GetCharInfoByAID = [&] {
		int WaitHourDiff;
		return DB->GetCharInfoByAID(AID, Attr.CharIndex, &MCharInfo, WaitHourDiff);
	};
	
	char CharName[CharNameLength];
	RandomString(CharName);
	int CID;
	auto GetCharCID = [&] {
		return DB->GetCharCID(CharName, &CID);
	};
	auto CreateChar = [&] {
		return DB->CreateCharacter(AID, CharName, Attr.CharIndex, Attr.Sex, Attr.Hair,
			Attr.Face, Attr.Costume);
	};

	TestAssert(!GetCharInfo());
	TestAssert(!GetCharInfoByAID());
	TestAssert(!GetCharCID());
	TestAssert(CreateChar() == MOK);
	TestAssert(GetCharCID());
	TestAssert(CreateChar() == MERR_CLIENT_EXIST_CHARNAME);
	TestAssert(GetCharInfo());
	TestAssert(GetCharInfoByAID());

	return MCharInfo;
};

void CheckCharInfo(IDatabase* DB, u32 AID, int CharIndex, const MMatchCharInfo& e)
{
	auto List = GetCharList(DB, AID);
	TestAssert(List);
	TestAssert(List->CharCount == 1);
	TestAssert(equals(List->CharList[0].szName, e.m_szName));
	TestAssert(List->CharList[0].nCharNum == e.m_nCharNum);
	TestAssert(List->CharList[0].nLevel == e.m_nLevel);

	MTD_CharInfo CharInfo;
	TestAssert(DB->GetAccountCharInfo(AID, CharIndex, &CharInfo));
	TestAssert(equals(CharInfo.szName, e.m_szName));
	TestAssert(equals(CharInfo.szClanName, ""));
	TestAssert(CharInfo.nCharNum == e.m_nCharNum);
	TestAssert(CharInfo.nLevel == e.m_nLevel);
	TestAssert(CharInfo.nSex == e.m_nSex);
	TestAssert(CharInfo.nHair == e.m_nHair);
	TestAssert(CharInfo.nFace == e.m_nFace);
	TestAssert(CharInfo.nXP == e.m_nXP);
	TestAssert(CharInfo.nBP == e.m_nBP);

	int WaitHourDiff;
	MMatchCharInfo MCharInfo;
	TestAssert(DB->GetCharInfoByAID(AID, CharIndex, &MCharInfo, WaitHourDiff));
	TestAssert(MCharInfo.m_nCID == e.m_nCID);
	TestAssert(MCharInfo.m_nCharNum == e.m_nCharNum);
	TestAssert(equals(MCharInfo.m_szName, e.m_szName));
	TestAssert(MCharInfo.m_nLevel == e.m_nLevel);
	TestAssert(MCharInfo.m_nSex == e.m_nSex);
	TestAssert(MCharInfo.m_nHair == e.m_nHair);
	TestAssert(MCharInfo.m_nFace == e.m_nFace);
	TestAssert(MCharInfo.m_nXP == e.m_nXP);
	TestAssert(MCharInfo.m_nBP == e.m_nBP);
	TestAssert(MCharInfo.m_nTotalKillCount == e.m_nTotalKillCount);
	TestAssert(MCharInfo.m_nTotalDeathCount == e.m_nTotalDeathCount);
}

void TestCharAttributes(IDatabase* DB, u32 AID, int CharIndex, int CID,
	const MMatchCharInfo& InitialAttributes)
{
	auto ExpectedAttributes = InitialAttributes;
	auto CheckCharInfo = [&] {
		return TestDBInternal::CheckCharInfo(DB, AID, CharIndex, ExpectedAttributes);
	};

	CheckCharInfo();

	TestAssert(DB->UpdateCharLevel(CID, 42));
	ExpectedAttributes.m_nLevel = 42;
	CheckCharInfo();

	{
		auto AddedBP = 42;
		TestAssert(DB->UpdateCharBP(CID, AddedBP));
		ExpectedAttributes.m_nBP += AddedBP;
		CheckCharInfo();
	}

	{
		auto AddedXP = 10, AddedBP = 20, AddedKillCount = 30, AddedDeathCount = 40;
		TestAssert(DB->UpdateCharInfoData(CID, AddedXP, AddedBP, AddedKillCount, AddedDeathCount));
		ExpectedAttributes.m_nXP += AddedXP;
		ExpectedAttributes.m_nBP += AddedBP;
		ExpectedAttributes.m_nTotalKillCount += AddedKillCount;
		ExpectedAttributes.m_nTotalDeathCount += AddedDeathCount;
		CheckCharInfo();
	}
}

void TestItems(IDatabase* DB, u32 AID, int CharIndex, int CID)
{
	MMatchCharInfo MCharInfo;
	MCharInfo.m_nCID = CID;
	MCharInfo.m_nCharNum = CharIndex;

	struct ItemIDs
	{
		int ID{};
		u32 CIID{};

		auto tied() const { return std::tie(ID, CIID); }
		bool operator==(ItemIDs rhs) const { return tied() == rhs.tied(); }
		bool operator< (ItemIDs rhs) const { return tied() <  rhs.tied(); }
	};
	using ItemList = std::vector<ItemIDs>;
	ItemList ExpectedItems;
	ItemList ActualItems;

	auto SortedInsert = [](ItemList& Vec, ItemIDs Value) {
		Vec.insert(std::lower_bound(Vec.begin(), Vec.end(), Value), Value);
	};

	auto GetItemsFromDB = [&](ItemList& Output) {
		MCharInfo.m_EquipedItem.Clear();
		MCharInfo.m_ItemList.Clear();
		TestAssert(DB->GetCharItemInfo(MCharInfo));
		Output.clear();
		for (auto&& Pair : MCharInfo.m_ItemList)
		{
			auto& Item = Pair.second;
			SortedInsert(Output, {int(Item->GetDescID()), Item->GetCIID()});
		}
	};

	auto CheckItems = [&] {
		GetItemsFromDB(ActualItems);
		TestAssert(ExpectedItems == ActualItems);
	};

	auto CreateItem = [&](int ItemID) {
		auto Desc = new MMatchItemDesc{};
		Desc->m_nID = ItemID;
		MGetMatchItemDescMgr()->insert({ItemID, Desc});
	};

	auto AddItem = [&](int ItemID) {
		CreateItem(ItemID);
		u32 CIID;
		TestAssert(DB->InsertCharItem(CID, ItemID, false, 0, &CIID));
		SortedInsert(ExpectedItems, {ItemID, CIID});
	};

	for (auto&& CostumePair : g_InitialCostume)
		for (auto&& Costume : CostumePair)
			for (int ItemID : reinterpret<std::array<int, 9>>(Costume))
				if (ItemID)
					CreateItem(ItemID);

	GetItemsFromDB(ExpectedItems);

	CheckItems();

	auto RandomItem   = [&] { return RandomIterator(ExpectedItems); };
	auto RandomItemID = [&] { return RandomNumber(1, INT_MAX); };

	for (int i = 0; i < 10; ++i)
	{
		AddItem(RandomItemID());
		CheckItems();
	}

	using EquippedItemList = std::array<ItemIDs, MMCIP_END>;

	auto GetEquippedItems = [&] {
		EquippedItemList Ret;
		int WaitHourDiff;
		TestAssert(DB->GetCharInfoByAID(AID, CharIndex, &MCharInfo, WaitHourDiff));
		MCharInfo.EquipFromItemList();
		for (size_t i = 0; i < std::size(Ret); ++i)
		{
			auto ItemPtr = MCharInfo.m_EquipedItem.GetItem(MMatchCharItemParts(i));
			auto ID = ItemPtr ? int(ItemPtr->GetDescID()) : 0;
			Ret[i] = {ID, MCharInfo.m_nEquipedItemCIID[i]};
		}
		return Ret;
	};

	auto ExpectedEquippedItems = GetEquippedItems();

	for (auto&& _ : ExpectedItems)
	{
		static const MMatchCharItemParts CheckPairs[][2]{
			{MMCIP_FINGERL, MMCIP_FINGERR},
			{MMCIP_PRIMARY, MMCIP_SECONDARY},
			{MMCIP_CUSTOM1, MMCIP_CUSTOM2},
		};
		bool Unequip = RandomNumber(2) == 0;
		auto Item = Unequip ? ItemIDs{0, 0} : *RandomItem();
		auto Parts = MMatchCharItemParts(RandomNumber(int(MMCIP_END) - 1));
		bool AlreadyEquipped = false;
		if (!Unequip)
		{
			for (auto&& Pair : CheckPairs)
			{
				if (Parts == Pair[0])
				{
					AlreadyEquipped = ExpectedEquippedItems[Pair[1]] == Item;
					break;
				}
				else if (Parts == Pair[1])
				{
					AlreadyEquipped = ExpectedEquippedItems[Pair[0]] == Item;
					break;
				}
			}
		}
		TestAssert(DB->UpdateEquipedItem(CID, Parts, Item.CIID, Item.ID) == !AlreadyEquipped);
		if (!AlreadyEquipped)
			ExpectedEquippedItems[Parts] = Item;
		TestAssert(GetEquippedItems() == ExpectedEquippedItems);
	}

	CheckItems();

	TestAssert(DB->ClearAllEquipedItem(CID));
	ExpectedEquippedItems = {};
	TestAssert(GetEquippedItems() == ExpectedEquippedItems);

	auto GetBP = [&] {
		int WaitHourDiff;
		TestAssert(DB->GetCharInfoByAID(AID, CharIndex, &MCharInfo, WaitHourDiff));
		return MCharInfo.m_nBP;
	};
	auto BP = GetBP();
	for (auto&& _ : ExpectedItems)
	{
		auto ItemID = RandomItemID();
		CreateItem(ItemID);
		auto Price = RandomNumber(1, 20);
		u32 CIID;
		bool ShouldSucceed = BP >= Price;
		TestAssert(DB->BuyBountyItem(CID, ItemID, Price, &CIID) == ShouldSucceed);
		if (ShouldSucceed)
		{
			BP -= Price;
			SortedInsert(ExpectedItems, {ItemID, CIID});
		}
		else
		{
			auto AddedBP = RandomNumber(1, 20);
			TestAssert(DB->UpdateCharBP(CID, AddedBP));
			BP += AddedBP;
		}
		TestAssert(BP == GetBP());
	}

	CheckItems();

	for (size_t i = 0; i < ExpectedItems.size() / 2; ++i)
	{
		auto Item = RandomItem();
		auto Price = RandomNumber(1, 20);
		auto Sell = [&] {
			return DB->SellBountyItem(CID, Item->ID, Item->CIID, Price, BP);
		};
		TestAssert(Sell());
		TestAssert(!Sell());
		ExpectedItems.erase(Item);
		CheckItems();
		BP += Price;
		TestAssert(BP == GetBP());
	}

	for (auto&& _ : ExpectedItems)
	{
		auto Item = RandomItem();
		// DeleteCharItem never fails.
		auto Delete = [&] { return DB->DeleteCharItem(CID, Item->CIID); };
		TestAssert(Delete());
		TestAssert(Delete());
		ExpectedItems.erase(Item);
		CheckItems();
	}

	MQuestItemMap ExpectedQuestItems;
	auto CheckQuestItems = [&] {
		TestAssert(DB->GetCharQuestItemInfo(&MCharInfo));
		TestAssert(std::equal(ExpectedQuestItems.begin(), ExpectedQuestItems.end(),
			MCharInfo.m_QuestItemList.begin(), MCharInfo.m_QuestItemList.end(),
			[](std::pair<const u32, MQuestItem*> a, std::pair<const u32, MQuestItem*> b) {
			return a.first == b.first;
		}));
	};
	CheckQuestItems();
	MQuestMonsterBible MonsterBible;
	MonsterBible.Clear();
	auto UpdateAndCheck = [&] {
		TestAssert(DB->UpdateQuestItem(CID, ExpectedQuestItems, MonsterBible));
		CheckQuestItems();
	};
	int QuestItemIDsStorage[MAXID_QITEM_LIMIT - MINID_QITEM_LIMIT + 1];
	for (int i = 0; i < int(std::size(QuestItemIDsStorage)); ++i)
		QuestItemIDsStorage[i] = i + MINID_QITEM_LIMIT;
	ArrayView<int> QuestItemIDs = QuestItemIDsStorage;
	for (int i = 0; i < 10; ++i)
	{
		auto ItemID = std::exchange(RandomElement(QuestItemIDs), QuestItemIDs.back());
		QuestItemIDs.remove_suffix(1);
		auto Desc = new MQuestItemDesc{};
		Desc->m_nItemID = ItemID;
		GetQuestItemDescMgr().insert({ItemID, Desc});
		auto Count = RandomNumber(1, 5);
		ExpectedQuestItems.CreateQuestItem(ItemID, Count);
		UpdateAndCheck();
	}
	ExpectedQuestItems.Clear();
	UpdateAndCheck();
}

void TestDeleteChar(IDatabase* DB, u32 AID, int CharIndex, const char* Name)
{
	auto DeleteChar = [&] {
		return DB->DeleteCharacter(AID, 0, Name);
	};
	TestAssert(DeleteChar());
	auto List = GetCharList(DB, AID);
	TestAssert(List);
	TestAssert(List->CharCount == 0);
	int DummyInt;
	TestAssert(!DB->GetCharCID(Name, &DummyInt));
	MTD_CharInfo DummyCharInfo;
	TestAssert(!DB->GetAccountCharInfo(AID, CharIndex, &DummyCharInfo));
	MMatchCharInfo DummyMCharInfo;
	TestAssert(!DB->GetCharInfoByAID(AID, CharIndex, &DummyMCharInfo, DummyInt));
	TestAssert(!DeleteChar());
}

struct TestCharInfo
{
	u32 AID;
	int CharIndex;
	int CID;
	char Name[MATCHOBJECT_NAME_LENGTH];
};

bool operator==(const TestCharInfo& a, const TestCharInfo& b) { return a.CID == b.CID; }
bool operator< (const TestCharInfo& a, const TestCharInfo& b) { return a.CID <  b.CID; }

constexpr auto NumTestChars = 10;
using TestCharList = std::array<TestCharInfo, NumTestChars>;

TestCharList TestFriends(IDatabase* DB, int CID)
{
	TestCharList FriendsStorage;
	ArrayView<TestCharInfo> Friends;
	auto CheckFriends = [&] {
		MMatchFriendInfo FriendInfo;
		TestAssert(DB->FriendGetList(CID, &FriendInfo));
		FriendInfo.m_FriendList.sort([](MMatchFriendNode* a, MMatchFriendNode* b) {
			return a->nFriendCID < b->nFriendCID;
		});
		TestAssert(std::equal(Friends.begin(), Friends.end(),
			FriendInfo.m_FriendList.begin(), FriendInfo.m_FriendList.end(),
			[](const TestCharInfo& a, MMatchFriendNode* b) {
			return a.CID == b->nFriendCID;
		}));
	};
	CheckFriends();
	for (size_t i = 0; i < NumTestChars; ++i)
	{
		auto& Friend = FriendsStorage[i];
		Friend.AID = CreateAccount(DB);
		auto Attr = RandomCharAttributes();
		Friend.CharIndex = Attr.CharIndex;
		auto FriendCharInfo = CreateChar(DB, Friend.AID, Attr);
		Friend.CID = FriendCharInfo.m_nCID;
		strcpy_safe(Friend.Name, FriendCharInfo.m_szName);
		int Favorite = RandomNumber(UCHAR_MAX);
		// Never fails, just adds more duplicate friends, which we don't want.
		TestAssert(DB->FriendAdd(CID, Friend.CID, Favorite));
		Friends = {FriendsStorage.data(), i + 1};
		// Sort friend into the right spot.
		auto it = std::lower_bound(Friends.begin(), Friends.end() - 1, Friend);
		std::rotate(it, Friends.end() - 1, Friends.end());
		CheckFriends();
	}
	for (int i = 0; i < NumTestChars; ++i)
	{
		auto& Friend = RandomElement(Friends);
		auto Remove = [&] { return DB->FriendRemove(CID, Friend.CID); };
		TestAssert(Remove());
		// Never fails.
		TestAssert(Remove());
		// Remove friend from list while keeping order.
		auto it = std::find(Friends.begin(), Friends.end(), Friend);
		std::rotate(it, it + 1, Friends.end());
		Friends.remove_suffix(1);
		CheckFriends();
	}
	return FriendsStorage;
}

void TestClan(IDatabase* DB, ArrayView<TestCharInfo> Chars)
{
	auto OriginalChars = Chars;
	auto GetChar = [&]() -> auto& {
		std::swap(RandomElement(Chars), Chars.back());
		auto& Char = Chars.back();
		Chars.remove_suffix(1);
		return Char;
	};
	auto& MasterChar = GetChar();
	char ClanName[ClanNameLength];
	RandomString(ClanName);
	bool Ret;
	int CLID;
	TestAssert(DB->CreateClan(ClanName, MasterChar.CID, &Ret, &CLID) && Ret);
	auto CheckExpelFails = [&](const TestCharInfo& Char, MMatchClanGrade Grade) {
		auto Ret = DB->ExpelClanMember(CLID, Grade, Char.Name);
		TestAssert(Ret == ExpelResult::TooLowGrade);
	};
	for (auto Grade : {MCG_MEMBER, MCG_ADMIN, MCG_MASTER})
		CheckExpelFails(MasterChar, Grade);
	auto CheckClanInfo = [&](const TestCharInfo& Char, bool InClan,
		MMatchClanGrade Grade) {
		MMatchCharInfo MCharInfo;
		int WaitHourDiff;
		TestAssert(DB->GetCharInfoByAID(Char.AID, Char.CharIndex, &MCharInfo, WaitHourDiff));
		TestAssert(equals(MCharInfo.m_ClanInfo.m_szClanName, InClan ? ClanName : ""));
		if (InClan)
			TestAssert(MCharInfo.m_ClanInfo.m_nGrade == Grade);
	};
	for (int i = 0; i < NumTestChars - 1; ++i)
	{
		auto& Char = GetChar();
		auto Grade = MCG_MEMBER;
		// Never fails, just adds more duplicate members.
		TestAssert(DB->AddClanMember(CLID, Char.CID, Grade, &Ret) && Ret);
		MDB_ClanInfo ClanInfo;
		TestAssert(DB->GetClanInfo(CLID, &ClanInfo));
		TestAssert(ClanInfo.nCLID == CLID);
		TestAssert(equals(ClanInfo.szClanName, ClanName));
		TestAssert(equals(ClanInfo.szMasterName, MasterChar.Name));
		TestAssert(ClanInfo.nTotalMemberCount == i + 2);
		CheckClanInfo(Char, true, Grade);
		CheckExpelFails(Char, MCG_MEMBER);
		Grade = MCG_ADMIN;
		TestAssert(DB->UpdateClanGrade(CLID, Char.CID, Grade));
		CheckClanInfo(Char, true, Grade);
		CheckExpelFails(Char, MCG_MEMBER);
		CheckExpelFails(Char, MCG_ADMIN);
	}
	enum {
		RemoveFlag   = 1,
		ByMasterFlag = 2,
		MaxFlag      = 4,
	};
	int RemoveFlags = 0;
	Chars = OriginalChars;
	Chars.remove_suffix(1);
	for (int i = 0; i < NumTestChars - 1; ++i)
	{
		auto& Char = GetChar();
		auto AdminGrade = RemoveFlags & ByMasterFlag ? MCG_MASTER : MCG_ADMIN;
		auto Remove = [&] {
			if (RemoveFlags & RemoveFlag)
				return DB->RemoveClanMember(CLID, Char.CID);
			else
				return DB->ExpelClanMember(CLID, AdminGrade, Char.Name) == ExpelResult::OK;
		};
		if (AdminGrade == MCG_ADMIN)
			TestAssert(DB->UpdateClanGrade(CLID, Char.CID, MCG_MEMBER));
		TestAssert(Remove());
		// RemoveClanMember never fails.
		TestAssert(Remove() == bool(RemoveFlags & RemoveFlag));
		CheckClanInfo(Char, false, MCG_NONE);
		++RemoveFlags;
		RemoveFlags %= MaxFlag;
	}
	auto Close = [&] { return DB->CloseClan(CLID, ClanName, MasterChar.CID); };
	TestAssert(Close());
	// Never fails.
	TestAssert(Close());
}

void TestDB(IDatabase* DB)
{
	auto AID = CreateAccount(DB);
	auto Attr = RandomCharAttributes();
	auto CharInfo = CreateChar(DB, AID, Attr);
	auto CID = CharInfo.m_nCID;
	const int CharIndex = 0;
	TestCharAttributes(DB, AID, Attr.CharIndex, CID, CharInfo);
	TestItems(DB, AID, Attr.CharIndex, CID);
	auto Chars = TestFriends(DB, CID);
	TestClan(DB, Chars);
	for (auto&& Char : Chars)
		TestDeleteChar(DB, Char.AID, Char.CharIndex, Char.Name);
	TestDeleteChar(DB, AID, Attr.CharIndex, CharInfo.m_szName);
}

} // namespace
} // namespace TestDBInternal

void TestDB()
{
	using namespace TestDBInternal;
	struct Timer
	{
		const char* Name;
		using clock = std::chrono::high_resolution_clock;
		clock::time_point Start = clock::now();

		~Timer()
		{
			using namespace std::chrono;
			float Secs = duration_cast<duration<float>>(clock::now() - Start).count();
			MLog("%s tests took %.3f seconds\n", Name, Secs);
		}
	};
	[] {
		Timer timer{"MSSQL"};
		MDatabase::ConnectionDetails ConnDetails{};
		IniParser ini;
		if (!ini.Parse("TestDB.ini") || !GetDBConnDetails(ini, ConnDetails))
		{
			TestFail("Failed to parse DB settings from TestDB.ini");
			return;
		}

		MSSQLDatabase DB{ConnDetails};
		if (!DB.DeleteAllRows())
		{
			TestFail("Failed to delete rows in MSSQL DB");
			return;
		}

		TestDB(&DB);
	}();
	{
		Timer timer{"SQLite"};
		SQLiteDatabase DB{":memory:"};
		TestDB(&DB);
	}
}