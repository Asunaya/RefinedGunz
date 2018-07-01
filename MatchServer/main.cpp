#include "stdafx.h"
#include "MBMatchServer.h"
#include <iostream>
#include <atomic>
#include "MFile.h"
#include "MCrashDump.h"

static std::mutex InputMutex;
static std::vector<std::string> InputQueue;
static std::atomic<bool> HasInput;

static void InputThreadProc()
{
	std::string Input;

	while (true)
	{
		Input.clear();
		std::getline(std::cin, Input);
		if (Input.empty())
			continue;
		{
			std::lock_guard<std::mutex> Lock{ InputMutex };
			InputQueue.emplace_back(Input);
			HasInput = true;
		}
	}
}

static void HandleInput(MBMatchServer& MatchServer)
{
	if (!HasInput)
		return;

	std::lock_guard<std::mutex> Lock{ InputMutex };

	for (auto&& Input : InputQueue)
		MatchServer.OnInput(Input);

	InputQueue.clear();
	HasInput = false;
}

int main(int argc, char** argv)
try
{
	char LogFileName[MFile::MaxPath];
	GetLogFilename(LogFileName, "MatchLog", "txt");
	InitLog(MLOGSTYLE_DEBUGSTRING | MLOGSTYLE_FILE, LogFileName);
	void MatchServerCustomLog(const char*);
	CustomLog = MatchServerCustomLog;

	MCrashDump::SetCallback([](uintptr_t ExceptionInfo) {
		char Filename[MFile::MaxPath];
		GetLogFilename(Filename, "MatchServer", "dmp");
		MCrashDump::WriteDump(ExceptionInfo, Filename);
	});

	MBMatchServer MatchServer;

	if (!MatchServer.Create(6000))
	{
		MLog("MMatchServer::Create failed\n");
		return -1;
	}

	MatchServer.InitLocator();

	std::thread{ [&] { InputThreadProc(); } }.detach();

	while (true)
	{
		MatchServer.Run();
		HandleInput(MatchServer);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
catch (std::runtime_error& e)
{
	MLog("Uncaught std::runtime_error: %s\n", e.what());
	throw;
}