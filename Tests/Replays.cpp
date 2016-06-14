#include "stdafx.h"
#include "MTypes.h"
#include "ZReplay.h"
#include "ZApplication.h"
#include "ZFile.h"
#include "ZGameClient.h"
#include "ZReplay.h"
#include "ZGame.h"
#include "ZNetCharacter.h"
#include "ZMyCharacter.h"
#include "ZPost.h"
#include "ZRuleDuel.h"
#include "RGMain.h"
#include "ZReplay.inl"

#undef ASSERT
#undef assert

#define ASSERT(pred) assert(pred, #pred, __FILE__, __LINE__)

void assert(bool pred, const char* str, const char *File, int Line)
{
	if (pred)
		return;

	printf_s("%s failed!\nFile: %s\nLine: %d\n\n", str, File, Line);

	system("pause");

	//exit(0);
}

struct ReplayData
{
	ReplayVersion Version;
	REPLAY_STAGE_SETTING_NODE StageSetting;
	decltype(ZReplayLoader().GetCharInfo()) Players;
	int NumCommands = 0;
	int CommandStreamPos = 0;
};

bool LoadFile(ReplayData& Output, const char* Filename)
{
	ZReplayLoader Loader;

	try
	{
		if (!Loader.LoadFile(Filename))
			return false;

		Output.Version = Loader.GetVersion();

		auto VersionString = Output.Version.GetVersionString();

		printf_s("Replay header loaded for %s: %s\n", Filename, VersionString.c_str());

		Loader.GetStageSetting(Output.StageSetting);

		printf_s("Character position: %d\n", Loader.GetPosition());

		if (Output.StageSetting.nGameType == MMATCH_GAMETYPE_DUEL)
		{
			Loader.GetDuelQueueInfo(nullptr);
		}

		Output.Players = Loader.GetCharInfo();

		printf_s("Command stream position: %d\n", Loader.GetPosition());

		Output.CommandStreamPos = Loader.GetPosition();

		//Loader.GetCommands([&](auto, auto) { Output.NumCommands++; }, false);
	}
	catch (EOFException& e)
	{
		printf_s("Unexpected EOF while reading replay %s at position %d\n", Filename, e.GetPosition());
		return false;
	}
	catch (...)
	{
		printf_s("Something went wrong while reading replay %s\n", Filename);
		return false;
	}

	return true;
}

void PrintPlayers(ReplayData& Data)
{
	for (auto& Player : Data.Players)
	{
		printf_s("IsHero: %d\n", Player.IsHero);
		printf_s("Name: %s\n", Player.Info.szName);
		printf_s("Clan name: %s\n", Player.Info.szClanName);
		printf_s("Clan grade: %d\n", Player.Info.nClanGrade);
		printf_s("Clan cont point: %d\n", Player.Info.nClanContPoint);
		printf_s("Position: %f, %f, %f / %08X, %08X, %08X\n",
			Player.State.Position.x, Player.State.Position.y, Player.State.Position.z,
			reinterpret<u32>(Player.State.Position.x), reinterpret<u32>(Player.State.Position.y), reinterpret<u32>(Player.State.Position.z));

		printf_s("Property name: %s\n", Player.State.Property.szName);
		printf_s("Property clan name: %s\n", Player.State.Property.szClanName);
		printf_s("Property sex: %d\n", Player.State.Property.nSex);
		printf_s("Property hair: %d\n", Player.State.Property.nHair);
		printf_s("Property face: %d\n", Player.State.Property.nFace);
	}
}

int main()
{
	InitLog();

	ReplayData Data;

	ASSERT(LoadFile(Data, "ReplayFiles/GLT[001]_Alwaysssss_2015-11-15_21-22-36.gzr"));

	printf_s("Map name: %s\n", Data.StageSetting.szMapName);
	printf_s("Stage name: %s\n", Data.StageSetting.szStageName);
	printf_s("Gametype: %d\n", Data.StageSetting.nGameType);
	printf_s("Max players: %d\n", Data.StageSetting.nMaxPlayers);
	printf_s("Team kill enabled: %d\n", Data.StageSetting.bTeamKillEnabled);
	printf_s("Team win the point: %d\n", Data.StageSetting.bTeamWinThePoint);
	printf_s("Forced entry enabled: %d\n", Data.StageSetting.bForcedEntryEnabled);
	printf_s("Num commands = %d\n", Data.NumCommands);

	PrintPlayers(Data);

	ASSERT(Data.StageSetting.nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM);
	ASSERT(Data.StageSetting.nMaxPlayers == 2);
	ASSERT(Data.StageSetting.bTeamKillEnabled == false);
	ASSERT(Data.StageSetting.bTeamWinThePoint == false);
	ASSERT(Data.StageSetting.bForcedEntryEnabled == true);

	ASSERT(Data.Players.size() == 2);

	ASSERT(Data.CommandStreamPos == 2250);

	decltype(ReplayData::Players)::iterator it;

	auto FindPlayer = [&](const char* Name)
	{
		it = std::find_if(Data.Players.begin(), Data.Players.end(), [&](ReplayPlayerInfo& Player) {
			return strcmp(Player.Info.szName, Name) == 0;
		});

		ASSERT(it != Data.Players.end());
	};

	FindPlayer("Alwaysssss");

	ASSERT(it->IsHero);
	ASSERT(it->Info.nLevel == 1);

	FindPlayer("AMITYVILLE");

	ASSERT(!it->IsHero);
	ASSERT(it->Info.nLevel == 63);

	printf_s("\nFreestyle Gunz V9 OK\n\n");

	ASSERT(LoadFile(Data, "ReplayFiles/GLT_Rinnema_20160612_175131.gzr"));

	PrintPlayers(Data);

	ASSERT(Data.Players.size() == 2);

	ASSERT(Data.CommandStreamPos == 1562);

	FindPlayer("Rinnema");
	FindPlayer("bestgladna");

	printf_s("\nDark Gunz V6 OK\n\n");

	ASSERT(LoadFile(Data, "ReplayFiles/GLT_dystopiz_20130316_003344.gzr"));

	ASSERT(Data.Players.size() == 3);
	ASSERT(Data.CommandStreamPos == 2623);

	FindPlayer("dystopiz");
	FindPlayer("MeetraSurik");
	FindPlayer("Banner");

	printf_s("\nAeria Gunz V11 OK\n\n");

	system("pause");
}