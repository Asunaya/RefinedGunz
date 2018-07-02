#include "stdafx.h"
#include "MCrashDump.h"
#include "MFile.h"

#ifdef _WIN32

#include "MDebug.h"
#include "Shlwapi.h"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4091)
#endif
#include <imagehlp.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#pragma comment(lib, "dbghelp.lib") 

constexpr auto HandlerReturnValue = EXCEPTION_EXECUTE_HANDLER;

static void CrashExceptionDump(PEXCEPTION_POINTERS ExceptionInfo, const char* szDumpFileName)
{
	HANDLE hFile = CreateFileA(
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
	}

	CloseHandle(hFile);

	if (IsLogAvailable())
		MFilterException(ExceptionInfo);
}

void MCrashDump::WriteDump(uintptr_t ExceptionInfo, const char* Filename)
{
	CrashExceptionDump(reinterpret_cast<PEXCEPTION_POINTERS>(ExceptionInfo), Filename);
}

static thread_local std::function<void(uintptr_t)> UserCallback;

static LONG __stdcall ExceptionFilter(_EXCEPTION_POINTERS* p)
{
	if (UserCallback)
		UserCallback(reinterpret_cast<uintptr_t>(p));
	return HandlerReturnValue;
}

void MCrashDump::SetCallback(std::function<void(uintptr_t)> CrashCallback)
{
	UserCallback = std::move(CrashCallback);
	SetUnhandledExceptionFilter(ExceptionFilter);
}

void MCrashDump::Try(function_view<void()> Func, function_view<void(uintptr_t)> CrashCallback)
{
	__try
	{
		Func();
	}
	__except((CrashCallback(reinterpret_cast<uintptr_t>(GetExceptionInformation())), HandlerReturnValue))
	{
	}
}

#else

void MCrashDump::SetCallback(std::function<void(uintptr_t)>){}
void MCrashDump::Try(function_view<void()> Func, function_view<void(uintptr_t)>)
{
	Func();
}
void MCrashDump::WriteDump(uintptr_t, const char*){}

#endif
