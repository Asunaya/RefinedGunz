#ifndef _MPDB_H
#define _MPDB_H

#include <windows.h>
#include <string>

using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif

//	bool	GetCrashInfo(char* FullPathFileName, DWORD CrashAddress, char* OutInfo);
	DWORD	GetCrashInfo(LPEXCEPTION_POINTERS exceptionInfo,string& str);

#ifdef __cplusplus
}
#endif

#endif//_MPDB_H
