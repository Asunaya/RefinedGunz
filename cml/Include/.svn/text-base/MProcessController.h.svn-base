#pragma once


#include <windows.h>

#define _MPROCESS_CONTROLLER

#ifdef _MPROCESS_CONTROLLER
///////////////////////////

#include <Tlhelp32.h>

class MProcessController {
public:
	static bool FindProcessByName(const char* pszProcessName, PROCESSENTRY32* pOutPE32); // Win95 Compatible
	static HANDLE OpenProcessHandleByFilePath(const char* pszFilePath);	// CloseHandle 필요, Win2K 이상
	static HANDLE OpenProcessHandleByName(const char* pszFilePath); // CloseHandle 필요, Win2K 이상
	static bool StartProcess(const char* pszProcessPath, const BOOL bInheritHandles = TRUE);
	static bool StopProcess(HANDLE hProcess);
};

/// 현재 프로세서의 메모리를 반환한다.
int MGetCurrProcessMemory();

///////////////////////////
#endif
