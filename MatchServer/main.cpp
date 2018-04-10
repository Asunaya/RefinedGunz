#include "stdafx.h"
#include "MBMatchServer.h"
#include <Shlwapi.h>
#include <iostream>
#include <atomic>

template <size_t size>
static bool GetLogFileName(char (&pszBuf)[size])
{
	if (PathIsDirectory("Log") == FALSE)
		CreateDirectory("Log", NULL);

	time_t		tClock;
	struct tm	tmTime;

	time(&tClock);
	auto err = localtime_s(&tmTime, &tClock);

	char szFileName[_MAX_DIR];

	int nFooter = 1;
	while (TRUE) {
		sprintf_safe(szFileName, "Log/MatchLog_%02d-%02d-%02d-%d.txt",
			tmTime.tm_year + 1900, tmTime.tm_mon + 1, tmTime.tm_mday, nFooter);

		if (PathFileExists(szFileName) == FALSE)
			break;

		nFooter++;
		if (nFooter > 100) return false;
	}
	strcpy_safe(pszBuf, szFileName);
	return true;
}

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
	SetCurrentDirectory("./Runtime");

	char LogFileName[256];
	GetLogFileName(LogFileName);
	InitLog(MLOGSTYLE_DEBUGSTRING | MLOGSTYLE_FILE, LogFileName);
	void MatchServerCustomLog(const char*);
	CustomLog = MatchServerCustomLog;

	char cwd[256];
	GetCurrentDirectory(std::size(cwd), cwd);
	MLog("cwd: %s\n", cwd);

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
		Sleep(1);
	}
}
catch (std::runtime_error& e)
{
	MLog("Uncaught std::runtime_error: %s\n", e.what());
	throw;
}