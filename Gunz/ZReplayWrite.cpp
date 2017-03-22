#include "stdafx.h"
#include "ZReplay.h"
#include "RGVersion.h"
#include "ZRuleDuel.h"

// Returns false from the enclosing function if the expression is false.
#define V(expr) do { if (!(expr)) return false; } while (false)
// Attempts to write a value to the file, returning false from the enclosing function if it fails.
#define WRITE(expr) V(File.Write((expr)))

static bool WriteHeader(ZFile& File)
{
	REPLAY_HEADER_RG Header;
	Header.Header = RG_REPLAY_MAGIC_NUMBER;
	Header.ReplayBinaryVersion = RG_REPLAY_BINARY_VERSION;
	Header.Timestamp = static_cast<i64>(time(nullptr));
	Header.ClientVersionMajor = RGUNZ_VERSION_MAJOR;
	Header.ClientVersionMinor = RGUNZ_VERSION_MINOR;
	Header.ClientVersionPatch = RGUNZ_VERSION_PATCH;
	Header.ClientVersionRevision = RGUNZ_VERSION_REVISION;

	return File.Write(Header);
}

static bool WriteStageSetting(ZFile& File)
{
	REPLAY_STAGE_SETTING_NODE rssn;
	GetReplayStageSetting(rssn, *ZGetGameClient()->GetMatchStageSetting()->GetStageSetting());
	WRITE(rssn);

	return true;
}

static bool WriteDuelInfo(ZFile& File)
{
	assert(ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUEL);
	assert(dynamic_cast<ZRuleDuel*>(ZGetGameInterface()->GetGame()->GetMatch()->GetRule()) != nullptr);

	auto* RuleDuel = static_cast<ZRuleDuel*>(ZGetGameInterface()->GetGame()->GetMatch()->GetRule());
	WRITE(RuleDuel->QInfo);

	return true;
}

static bool WriteCharacters(ZFile& File, ZCharacterManager& CharacterManager)
{
	auto CharacterCount = i32(CharacterManager.size());
	WRITE(CharacterCount);

	for (auto& Item : CharacterManager)
	{
		auto Char = Item.second;
		ReplayPlayerInfo Info;
		Char->Save(Info);
		WRITE(Info);
	}

	return true;
}

static bool WriteTime(ZFile& File, float Time)
{
	static_assert(std::is_same<decltype(Time), float>::value, "Type of Time is wrong");
	WRITE(Time);

	return true;
}

bool WriteReplayStart(ZFile& File, float Time, ZCharacterManager& CharacterManager)
{
	V(WriteHeader(File));
	V(WriteStageSetting(File));
	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUEL) {
		V(WriteDuelInfo(File));
	}
	V(WriteCharacters(File, CharacterManager));
	V(WriteTime(File, Time));

	return true;
}

bool WriteReplayEnd(ZFile& File, ZObserverCommandList& ReplayCommandList)
{
	// Write the commands
	for (auto* pItem : ReplayCommandList)
	{
		auto* pCommand = pItem->pCommand;

		constexpr int BUF_SIZE = 1024;
		char CommandBuffer[BUF_SIZE];
		auto Size = pCommand->GetData(CommandBuffer, BUF_SIZE);
		if (Size <= 0)
		{
			MLog("WriteReplayEnd -- Invalid command!\n");
			continue;
		}

		WRITE(pItem->fTime);
		WRITE(pCommand->m_Sender);
		WRITE(Size);
		File.Write(CommandBuffer, Size);
	}

	return true;
}

#undef V
#undef WRITE
