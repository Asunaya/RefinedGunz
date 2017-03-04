#include "stdafx.h"
#include "MBMatchServer.h"
#include "MMatchConfig.h"

static std::string Line;
static std::vector<std::string> Splits;
// Doesn't count the command.
static int NumArguments;

static constexpr int NoArgumentLimit = -1;

void MBMatchServer::InitConsoleCommands()
{
	auto AddConsoleCommand = [&](const char* Name,
		int MinArgs, int MaxArgs,
		std::string Description, ::string Usage, std::string Help,
		auto&& Callback)
	{
		ConsoleCommand Command;
		Command.MinArgs = MinArgs;
		Command.MaxArgs = MaxArgs;
		Command.Description = std::move(Description);
		Command.Usage = std::move(Usage);
		Command.Help = std::move(Help);
		Command.Callback = std::move(Callback);
		ConsoleCommandMap.emplace(Name, std::move(Command));
	};

	AddConsoleCommand("hello",
		NoArgumentLimit, NoArgumentLimit,
		"Says hi.", "hello", "",
		[]{ MLog("Hi! ^__^\n"); });

	AddConsoleCommand("argv",
		NoArgumentLimit, NoArgumentLimit,
		"Prints all input.", "argv [Arg1 [Arg2 ... [ArgN]]]", "",
		[] {
		MLog("Line = %s\n", Line.c_str());
		MLog("NumArguments = %d\n", NumArguments);
		for (int i = 0; i < int(Splits.size()); ++i)
			MLog("Splits[%d] = \"%s\"\n", i, Splits[i].c_str());
	});

	AddConsoleCommand("help",
		NoArgumentLimit, 1,
		"Provides information about commands.",
		"help [command name]",
		"",
		[&] {
		if (NumArguments == 0)
		{
			// Print all the commands.
			for (auto&& Pair : ConsoleCommandMap)
			{
				MLog("%s: %s\n", Pair.first.c_str(), Pair.second.Description.c_str());
			}
		}
		else
		{
			auto it = ConsoleCommandMap.find(Splits[1]);
			if (it == ConsoleCommandMap.end())
			{
				MLog("help: Unknown command \"%s\"\n", Splits[1].c_str());
				return;
			}

			MLog("help: %s\n"
				"Description: %s\n"
				"Usage: %s\n"
				"Help: %s\n",
				it->first.c_str(),
				it->second.Description.c_str(),
				it->second.Usage.c_str(),
				it->second.Help.c_str());
		}
	});

	AddConsoleCommand("setversion", 1, 1,
		"Sets the expected client version.",
		"setversion <version number>",
		"Sets the client version number, MMatchConfig::Version, to the specified value.\n"
		"This value is used when players log in, to check if their clients are up to date.\n",
		[] {
		const auto NewVersion = atoi(Splits[1].c_str());
		const auto OldVersion = MGetServerConfig()->Version;
		MGetServerConfig()->Version = NewVersion;
		MLog("Set version to %d! Previous version was %d.\n", NewVersion, OldVersion);
	});
}

void MBMatchServer::OnInput(const std::string & Input)
{
	Line = Input;

	Splits.clear();

	size_t LastPos = 0;
	for (size_t Pos = 0; Pos < Input.length(); ++Pos)
	{
		if (Input[Pos] == ' ') {
			Splits.emplace_back(Input.begin() + LastPos, Input.begin() + Pos);
			while (Input[Pos] == ' ' && Pos < Input.length())
				++Pos;
			LastPos = Pos;
		}
	}

	if (LastPos < Input.length())
		Splits.emplace_back(Input.begin() + LastPos, Input.end());

	if (Splits.empty())
		return;

	auto it = ConsoleCommandMap.find(Splits[0]);
	if (it == ConsoleCommandMap.end())
	{
		MLog("Unknown command \"%s\"\n", Splits[0].c_str());
		return;
	}

	auto&& Command = it->second;

	NumArguments = int(Splits.size()) - 1;
	if ((Command.MinArgs != NoArgumentLimit && NumArguments < Command.MinArgs) ||
		(Command.MaxArgs != NoArgumentLimit && NumArguments > Command.MaxArgs))
	{
		MLog("Incorrect number of arguments to %s, valid range is %d-%d, got %d\n",
			it->first.c_str(), Command.MinArgs, Command.MaxArgs, NumArguments);
		return;
	}

	Command.Callback();
}
