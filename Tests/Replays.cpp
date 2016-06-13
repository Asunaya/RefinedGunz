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

#define ASSERT(pred) assert(pred, #pred)

void assert(bool pred, const char* str)
{
	if (pred)
		return;

	printf_s("%s failed!\n\n", str);

	system("pause");

	exit(0);
}

struct ReplayData
{
	ReplayVersion Version;
	REPLAY_STAGE_SETTING_NODE StageSetting;
	decltype(ZReplayLoader().GetCharInfo()) Players;
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

		if (Output.StageSetting.nGameType == MMATCH_GAMETYPE_DUEL)
		{
			Loader.GetDuelQueueInfo(nullptr);
		}

		Output.Players = Loader.GetCharInfo();

		Loader.GetCommands([](auto, auto) {}, true);
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

int main()
{
	ReplayData Data;
	ASSERT(LoadFile(Data, "ReplayFlies/GLT[001]_Alwaysssss_2015-11-15_21-22-36.gzr"));

	auto it = std::find_if(Data.Players.begin(), Data.Players.end(), [](ReplayPlayerInfo& Player) {
		return strcmp(Player.Info.szName, "Alwaysssss") == 0;
	});

	ASSERT(it != Data.Players.end());

	ASSERT(it->IsHero);
	ASSERT(it->Info.nLevel == 40);

	printf_s("Freestyle Gunz V9 OK\n\n");
}