#ifndef __MDEBUG_H
#define __MDEBUG_H

#include <Windows.h>
#include <crtdbg.h>

#include "GlobalTypes.h"

#define MASSERT(x) _ASSERT(x)

#define MLOGSTYLE_FILE 0x0001
#define MLOGSTYLE_DEBUGSTRING 0x0002

#define MLOG_DEFAULT_HISTORY_COUNT	10

bool IsLogAvailable();

void InitLog(int logmethodflags=MLOGSTYLE_DEBUGSTRING, const char* pszLogFileName="mlog.txt");

#if defined(_DEBUG) || defined(DEBUG_FAST)
void DMLog(const char* Format, ...);

static inline void DLogMatrix(matrix& mat)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			DMLog("%f ", mat(i, j));
		}

		DMLog("\n");
	}
}
#else
static inline void DMLog(...)
{
}

static inline void DLogMatrix(matrix& mat)
{
}
#endif

void __cdecl MLog(const char *pFormat,...);
#define mlog MLog

/*
char *MGetLogHistory(int i);
int	MGetLogHistoryCount();
*/

#ifdef _WIN32
void __cdecl MMsg(const char *pFormat,...);
#endif

DWORD MFilterException(LPEXCEPTION_POINTERS p);

//void MInstallSEH();	// Compile Option에 /EHa 있어야함

void MInitProfile();
void MBeginProfile(int nIndex,const char *szName);
void MEndProfile(int nIndex);
void MSaveProfile(const char *file);

#endif