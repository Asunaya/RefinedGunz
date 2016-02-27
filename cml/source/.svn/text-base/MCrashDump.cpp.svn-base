#include "stdafx.h"
#include "MCrashDump.h"

// CrashExceptionDump 함수를 사용하려면 dbghelp.dll이 필요하다.
// 해당 dll은 http://www.microsoft.com/whdc/ddk/debugging/ 에서 구할 수 있다.
#pragma comment(lib, "dbghelp.lib") 


DWORD CrashExceptionDump(PEXCEPTION_POINTERS ExceptionInfo, const char* szDumpFileName, bool bMLog)
{
	HANDLE hFile = CreateFile(
		szDumpFileName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile)
	{
		MINIDUMP_EXCEPTION_INFORMATION eInfo;
		eInfo.ThreadId = GetCurrentThreadId();
		eInfo.ExceptionPointers = ExceptionInfo;
		eInfo.ClientPointers = FALSE;

		MINIDUMP_CALLBACK_INFORMATION cbMiniDump;
		//cbMiniDump.CallbackRoutine = miniDumpCallback;
		cbMiniDump.CallbackRoutine = NULL;
		cbMiniDump.CallbackParam = 0;

		MiniDumpWriteDump(
			GetCurrentProcess(),
			GetCurrentProcessId(),
			hFile,
			MiniDumpNormal,
			ExceptionInfo ? &eInfo : NULL,
			NULL,
			NULL);
			//&cbMiniDump);
	}

	CloseHandle(hFile);

	if (bMLog)
	{
		MFilterException(ExceptionInfo);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}
