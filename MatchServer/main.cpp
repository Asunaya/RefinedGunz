#include "stdafx.h"
#include "MBMatchServer.h"
#include <Shlwapi.h>

static bool GetLogFileName(char* pszBuf)
{
	if (PathIsDirectory("Log") == FALSE)
		CreateDirectory("Log", NULL);

	time_t		tClock;
	struct tm*	ptmTime;

	time(&tClock);
	ptmTime = localtime(&tClock);

	char szFileName[_MAX_DIR];

	int nFooter = 1;
	while (TRUE) {
		sprintf(szFileName, "Log/MatchLog_%02d-%02d-%02d-%d.txt",
			ptmTime->tm_year + 1900, ptmTime->tm_mon + 1, ptmTime->tm_mday, nFooter);

		if (PathFileExists(szFileName) == FALSE)
			break;

		nFooter++;
		if (nFooter > 100) return false;
	}
	strcpy(pszBuf, szFileName);
	return true;
}

int main(int argc, char** argv)
{
	SetCurrentDirectory("./Runtime");

	char LogFileName[256];
	GetLogFileName(LogFileName);
	InitLog(MLOGSTYLE_DEBUGSTRING | MLOGSTYLE_FILE, LogFileName);

	char cwd[256];
	GetCurrentDirectory(ArraySize(cwd), cwd);
	MLog("cwd: %s\n", cwd);

	MBMatchServer MatchServer;

	if (!MatchServer.Create(6000))
	{
		MLog("MMatchServer::Create failed\n");
		return -1;
	}

	MatchServer.InitLocator();

	while (true)
	{
		MatchServer.Run();
		Sleep(1);
	}
}