#include "stdafx.h"
#include "MProcess.h"

#ifdef WIN32
#include "MWindows.h"
#endif

uintptr_t MProcess::Start(const char* Commandline)
{
#ifdef WIN32

	// CreateProcess can modify the string, so need to make our own copy.
	char LocalCommandLine[1024];
	strcpy_safe(LocalCommandLine, Commandline);

	STARTUPINFO si{};
	PROCESS_INFORMATION pi{};

	si.cb = sizeof(si);

	auto CreateProcessResult = CreateProcessA(
		NULL,  // Application name
		LocalCommandLine, // Command line
		NULL,  // Process attributes
		NULL,  // Thread attributes
		FALSE, // Inherit handles
		0,     // Creation flags
		NULL,  // Environment
		NULL,  // Current directory
		&si,   // Startup info
		&pi);  // Process info

	if (CreateProcessResult == FALSE)
	{
		return 0;
	}

	auto Handle = reinterpret_cast<uintptr_t>(pi.hProcess);
	return Handle;

#else

	static_assert(false, "Port me!");

#endif
}

MProcess::AwaitResult MProcess::Await(uintptr_t Handle, u32 Timeout)
{
	AwaitResult ret{};

#ifdef WIN32

	auto WinHandle = reinterpret_cast<HANDLE>(Handle);

	auto WaitResult = WaitForSingleObject(WinHandle, Timeout);
	if (WaitResult == INFINITE)
	{
		ret.TimedOut = true;
		ret.ExitCode = 0;
		return ret;
	}
	
	ret.TimedOut = false;

	DWORD ExitCode = 0;
	auto GotExitCode = GetExitCodeProcess(WinHandle, &ExitCode) != FALSE;
	ret.ExitCode = ExitCode;

#else

	static_assert(false, "Port me!");

#endif

	return ret;
}

bool MProcess::Terminate(uintptr_t Handle, u32 ExitCode)
{
#ifdef WIN32

	return ::TerminateProcess(reinterpret_cast<HANDLE>(Handle), ExitCode) != FALSE;

#else

	static_assert(false, "Port me!");

#endif
}