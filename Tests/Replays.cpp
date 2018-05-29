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
#include "reinterpret.h"
#include "MFile.h"
#include "TestAssert.h"

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
		{
			TestFail("ZReplayLoader::LoadFile failed");
			return false;
		}

		Output.Version = Loader.GetVersion();

		auto VersionString = Output.Version.GetVersionString();

		MLog("Replay header loaded for %s: %s\n", Filename, VersionString.c_str());

		if (Output.Version.Server == ServerType::None)
		{
			TestFail("Unrecognized replay server");
			return false;
		}

		Loader.GetStageSetting(Output.StageSetting);

		MLog("Character position: %d\n", Loader.GetPosition());

		if (Output.StageSetting.nGameType == MMATCH_GAMETYPE_DUEL)
		{
			Loader.GetDuelQueueInfo(nullptr);
		}

		Output.Players = Loader.GetCharInfo();

		MLog("Command stream position: %d\n", Loader.GetPosition());

		Output.CommandStreamPos = Loader.GetPosition();

		//Loader.GetCommands([&](auto, auto) { Output.NumCommands++; }, false);
	}
	catch (EOFException& e)
	{
		MLog("Unexpected EOF while reading replay %s at position %d\n", Filename, e.GetPosition());
		TestFail("EOFException");
		return false;
	}
	catch (...)
	{
		MLog("Something went wrong while reading replay %s\n", Filename);
		TestFail("Unknown exception");
		return false;
	}

	return true;
}

void PrintPlayers(ReplayData& Data)
{
	for (auto& Player : Data.Players)
	{
		MLog("IsHero: %d\n", Player.IsHero);
		MLog("Name: %s\n", Player.Info.szName);
		MLog("Level: %d\n", Player.Info.nLevel);
		MLog("Clan name: %s\n", Player.Info.szClanName);
		MLog("Clan grade: %d\n", Player.Info.nClanGrade);
		MLog("Clan cont point: %d\n", Player.Info.nClanContPoint);
		MLog("Position: %f, %f, %f / %08X, %08X, %08X\n",
			Player.State.Position.x, Player.State.Position.y, Player.State.Position.z,
			reinterpret<u32>(Player.State.Position.x), reinterpret<u32>(Player.State.Position.y), reinterpret<u32>(Player.State.Position.z));

		MLog("Property name: %s\n", Player.State.Property.szName);
		MLog("Property clan name: %s\n", Player.State.Property.szClanName);
		MLog("Property sex: %d\n", Player.State.Property.nSex);
		MLog("Property hair: %d\n", Player.State.Property.nHair);
		MLog("Property face: %d\n", Player.State.Property.nFace);
	}
}

struct ReplayFile : ReplayData
{
	std::string Name;
};

std::vector<ReplayFile> GetFileInfo()
{
	std::vector<ReplayFile> Files;

	ReplayFile* File;

	auto AddFile = [&] {
		Files.emplace_back();
		return &Files.back();
	};

	auto AddPlayer = [&](const char* Name, int Level, bool IsHero) {
		File->Players.emplace_back();
		auto&& Player = File->Players.back();
		strcpy_safe(Player.Info.szName, Name);
		Player.Info.nLevel = Level;
		Player.IsHero = IsHero;
	};

	File = AddFile();
	File->Version = { ServerType::FreestyleGunz, 7, 0 };
	File->Name = "fg-v7_0.gzr";
	strcpy_safe(File->StageSetting.szMapName, "Dojo");
	File->StageSetting.nGameType = MMATCH_GAMETYPE_GLADIATOR_TEAM;
	File->StageSetting.bForcedEntryEnabled = true;
	AddPlayer("swiftie", 62, true);
	AddPlayer("dydu", 70, false);
	File->CommandStreamPos = 1530;

	File = AddFile();
	File->Version = { ServerType::FreestyleGunz, 7, 1 };
	File->Name = "fg-v7_1.gzr";
	strcpy_safe(File->StageSetting.szMapName, "Town");
	File->StageSetting.nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
	File->StageSetting.bForcedEntryEnabled = true;
	AddPlayer("toxen", 40, true);
	File->CommandStreamPos = 1133;

	File = AddFile();
	File->Version = { ServerType::FreestyleGunz, 8, 0 };
	File->Name = "fg-v8.gzr";
	strcpy_safe(File->StageSetting.szMapName, "Dojo");
	File->StageSetting.nGameType = MMATCH_GAMETYPE_GLADIATOR_TEAM;
	File->StageSetting.bForcedEntryEnabled = true;
	AddPlayer("^5Alexstrasza", 82, true);
	AddPlayer("DUCKONWHEELS", 65, false);
	File->CommandStreamPos = 2186;

	File = AddFile();
	File->Version = { ServerType::FreestyleGunz, 9, 0 };
	File->Name = "fg-v9.gzr";
	strcpy_safe(File->StageSetting.szMapName, "Dojo");
	File->StageSetting.nGameType = MMATCH_GAMETYPE_GLADIATOR_TEAM;
	File->StageSetting.bForcedEntryEnabled = true;
	AddPlayer("Alwaysssss", 1, true);
	AddPlayer("AMITYVILLE", 63, false);
	File->CommandStreamPos = 2250;

	File = AddFile();
	File->Version = { ServerType::FreestyleGunz, 10, 0 };
	File->Name = "fg-v10.gzr";
	strcpy_safe(File->StageSetting.szMapName, "Mansion");
	File->StageSetting.nGameType = MMATCH_GAMETYPE_GLADIATOR_TEAM;
	AddPlayer("swiftie", 73, true);
	File->CommandStreamPos = 1241;

	File = AddFile();
	File->Version = { ServerType::DarkGunz, 6, 0 };
	File->Name = "dg-v6.gzr";
	strcpy_safe(File->StageSetting.szMapName, "Battle Arena");
	File->StageSetting.nGameType = MMATCH_GAMETYPE_GLADIATOR_TEAM;
	AddPlayer("Rinnema", 63, true);
	AddPlayer("bestgladna", 46, false);
	File->CommandStreamPos = 1562;

	File = AddFile();
	File->Version = { ServerType::Official, 4, 0 };
	File->Name = "official-v4.gzr";
	strcpy_safe(File->StageSetting.szMapName, "Battle Arena");
	File->StageSetting.nGameType = MMATCH_GAMETYPE_DEATHMATCH_TEAM;
	AddPlayer("Devilito", 50, false);
	AddPlayer("Tenalady", 62, false);
	AddPlayer("\x83" "achion", 67, false);
	AddPlayer("Confucious", 65, false);
	AddPlayer("Horror", 69, false);
	AddPlayer("Alinda", 62, false);
	AddPlayer("Shot", 65, false);
	AddPlayer("Pallero", 71, false);
	AddPlayer("GM_DeviIito", 56, true);
	AddPlayer("ApoIlo", 56, false);
	File->CommandStreamPos = 4330;

	File = AddFile();
	File->Version = { ServerType::Official, 6, 0 };
	File->Name = "official-v6.gzr";
	strcpy_safe(File->StageSetting.szMapName, "Battle Arena");
	File->StageSetting.nGameType = MMATCH_GAMETYPE_GLADIATOR_TEAM;
	AddPlayer("PwnHighLevs", 55, false);
	AddPlayer("notitiot1052", 32, true);
	AddPlayer("ImpactedQQ", 45, false);
	AddPlayer("Mikius", 30, false);
	AddPlayer("SortaDrunk", 59, false);
	AddPlayer("[Kor]Apple", 27, false);
	AddPlayer("^5Ruin^9", 7, false);
	AddPlayer("Baoser", 26, false);
	AddPlayer("Joiku", 24, false);
	AddPlayer("iDeviok", 28, false);
	AddPlayer("iDeviok", 28, false);
	File->CommandStreamPos = 7703;

	File = AddFile();
	File->Version = { ServerType::Official, 7, 0 };
	File->Name = "official-v7.gzr";
	strcpy_safe(File->StageSetting.szMapName, "Battle Arena");
	File->StageSetting.nGameType = MMATCH_GAMETYPE_GLADIATOR_TEAM;
	AddPlayer("Bewowzers", 13, true);
	AddPlayer("NUTTER", 1, false);
	AddPlayer("BadHeros", 26, false);
	AddPlayer("Dyriel", 43, false);
	File->CommandStreamPos = 2884;

	File = AddFile();
	File->Version = { ServerType::Official, 11, 0 };
	File->Name = "official-v11.gzr";
	strcpy_safe(File->StageSetting.szMapName, "Battle Arena");
	File->StageSetting.nGameType = MMATCH_GAMETYPE_GLADIATOR_TEAM;
	AddPlayer("dystopiz", 8, true);
	AddPlayer("MeetraSurik", 25, false);
	AddPlayer("Banner", 90, false);
	File->CommandStreamPos = 2623;

	std::string cwd(MFile::MaxPath, 0);
	MFile::GetCWD({&cwd[0], cwd.size()});
	cwd.resize(cwd.find_first_of('\0'));
	MLog("cwd: %s\n", cwd.c_str());
	auto pos = cwd.find_last_of("\\/");
	if (pos != std::string::npos)
	{
		cwd.assign(cwd.begin() + pos + 1, cwd.end());
	}

	if (cwd == "Tests")
	{
		for (auto& File : Files)
		{
			File.Name = "ReplayFiles/" + File.Name;
		}
	}
	else
	{
		for (auto& File : Files)
		{
			File.Name = "../ReplayFiles/" + File.Name;
		}
	}

	return Files;
}

void TestReplays()
{
	const auto Files = GetFileInfo();

	ReplayData Data;

	for (auto& File : Files)
	{
		MLog("Loading file %s\n", File.Name.c_str());
		if (!LoadFile(Data, File.Name.c_str()))
		{
			TestFail(strprintf("Failed to load replay file %s").c_str());
			continue;
		}

		MLog("Map name: %s\n", Data.StageSetting.szMapName);
		MLog("Stage name: %s\n", Data.StageSetting.szStageName);
		MLog("Gametype: %d\n", Data.StageSetting.nGameType);
		MLog("Max players: %d\n", Data.StageSetting.nMaxPlayers);
		MLog("Team kill enabled: %d\n", Data.StageSetting.bTeamKillEnabled);
		MLog("Team win the point: %d\n", Data.StageSetting.bTeamWinThePoint);
		MLog("Forced entry enabled: %d\n", Data.StageSetting.bForcedEntryEnabled);
		MLog("Num commands = %d\n", Data.NumCommands);

		PrintPlayers(Data);

		TestAssert(Data.Version == File.Version);
		//TestAssert(memcmp(&Data.StageSetting, &File.StageSetting, sizeof(Data.StageSetting)) == 0);
		TestAssert(!strcmp(Data.StageSetting.szMapName, File.StageSetting.szMapName));
		TestAssert(Data.StageSetting.nGameType == File.StageSetting.nGameType);
		TestAssert(Data.StageSetting.bTeamKillEnabled == false);
		TestAssert(Data.StageSetting.bTeamWinThePoint == false);

		TestAssert(Data.Players.size() == File.Players.size());

		for (auto& ExpectedPlayer : File.Players)
		{
			auto FindPlayer = [&](const char* Name) -> auto&
			{
				auto it = std::find_if(Data.Players.begin(), Data.Players.end(), [&](ReplayPlayerInfo& Player) {
					return strcmp(Player.Info.szName, Name) == 0;
				});
				TestAssert(it != Data.Players.end());
				return *it;
			};

			auto& ActualPlayer = FindPlayer(ExpectedPlayer.Info.szName);

			TestAssert(ActualPlayer.IsHero == ExpectedPlayer.IsHero);
			TestAssert(ActualPlayer.Info.nLevel == ExpectedPlayer.Info.nLevel);
			//TestAssert(!strcmp(ActualPlayer.Info.szClanName, ExpectedPlayer.Info.szClanName));
		}

		TestAssert(Data.CommandStreamPos == File.CommandStreamPos);
	}
}