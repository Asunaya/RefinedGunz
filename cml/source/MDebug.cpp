#include "stdafx.h"
#include <stdio.h>
#include <signal.h>
#include "fileinfo.h"
#include "MDebug.h"
#include <string>
#include "MPdb.h"
#include <windows.h>
#include <mutex>
#include "../../sdk/dx9/Include/d3dx9.h"
#include <cassert>

static char logfilename[256];
static int g_nLogMethod=MLOGSTYLE_DEBUGSTRING;

static bool g_bLogInitialized = false;

bool IsLogAvailable()
{
	return g_bLogInitialized;
}

void InitLog(int logmethodflags, const char* pszLogFileName)
{
	g_nLogMethod=logmethodflags;

	if(g_nLogMethod&MLOGSTYLE_FILE)
	{
		GetFullPath(logfilename, sizeof(logfilename), pszLogFileName);
		FILE *pFile = nullptr;
		fopen_s(&pFile, logfilename,"w+");
		if( !pFile ) return;
		fclose(pFile);
	}

	g_bLogInitialized = true;
}

void DMLog(const char* Format, ...)
{
	char temp[16 * 1024];

	va_list args;

	va_start(args, Format);
	vsprintf_safe(temp, Format, args);
	va_end(args);

	/*FILE *pFile = nullptr;
	fopen_s(&pFile, logfilename, "a");

	if (!pFile)
		fopen_s(&pFile, logfilename, "w");

	if (pFile == nullptr)
		return;

	fprintf(pFile, "%s", temp);
	fclose(pFile);*/

	OutputDebugString(temp);
}

#ifdef _DEBUG
void DLogMatrix(matrix& mat)
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
#endif

void MLogFile(const char* Msg)
{
	FILE *pFile = nullptr;
	fopen_s(&pFile, logfilename, "a");

	if (!pFile)
		fopen_s(&pFile, logfilename, "w");

	if (pFile == nullptr)
		return;

	fprintf(pFile, "%s", Msg);
	fclose(pFile);
}

void MLog(const char *pFormat,...)
{
	char temp[16 * 1024];

	va_list args;

	va_start(args,pFormat);
	vsprintf_safe(temp,pFormat,args);
	va_end(args);

	if (g_nLogMethod & MLOGSTYLE_FILE)
	{
		MLogFile(temp);
	}
	if (g_nLogMethod & MLOGSTYLE_DEBUGSTRING)
	{
		OutputDebugString(temp);
	}

	CustomLog(temp);
}

extern "C" void CustomLogDefault(const char* Msg) {}

#ifdef _WIN32
#include <windows.h>
#include <crtdbg.h>

void __cdecl MMsg(const char *pFormat,...)
{
    char buff[256];

    wvsprintf(buff, pFormat, (char *)(&pFormat+1));
    lstrcat(buff, "\r\n");
    MessageBox( NULL, buff, "RealSpace Message", MB_OK );
	mlog(buff);mlog("\n");
}
#endif

////////////////////////////////////////////////////////////////////////////////
// exception handler

void MShowContextRecord(CONTEXT* p)
{
#ifndef _WIN64
	mlog("[Context]\n");

	mlog("GS : %08x  FS : %08x  ES : %08x  DS : %08x\n", p->SegGs, p->SegFs, p->SegEs, p->SegDs);
	mlog("EDI: %08x  ESI: %08x  EBX: %08x  EDX: %08x\n", p->Edi, p->Esi, p->Ebx, p->Edx);
	mlog("ECX: %08x  EAX: %08x  EBP: %08x  EIP: %08x\n", p->Ecx, p->Eax, p->Ebp, p->Eip);
	mlog("CS : %08x  Flg: %08x  ESP: %08x  SS : %08x\n", p->SegCs, p->EFlags, p->Esp, p->SegSs);

	mlog("\n");
#endif
}

void MShowStack(DWORD* sp, int nSize)
{
	mlog("[Stack]");

	for(int i=0; i<nSize; i++){
		if((i%8)==0) mlog("\n");
		mlog("%08x ", *(sp+i));
	}

	mlog("\n");
}

void MFilterException(LPEXCEPTION_POINTERS p)
{
	char tmpbuf[128];
	_strtime_s(tmpbuf);

	mlog("Crash (%s)\n",tmpbuf);

	mlog("Build " __DATE__ " " __TIME__ "\n\n");

	mlog("\n[Exception]\n");
	mlog("Address	:	%08x\n", p->ExceptionRecord->ExceptionAddress);
	mlog("ExpCode	:	%08x\n", p->ExceptionRecord->ExceptionCode);
	mlog("Flags	:	%08x\n", p->ExceptionRecord->ExceptionFlags);
	mlog("#Param	:	%08x\n", p->ExceptionRecord->NumberParameters);
	mlog("other	:	%08x\n", p->ExceptionRecord->ExceptionRecord);
	mlog("\n");
	
	MShowContextRecord(p->ContextRecord);
#ifndef _WIN64
	MShowStack((DWORD*)p->ContextRecord->Esp, 128);
#endif

	mlog("\n");

	string str;

	GetCrashInfo(p,str);

	mlog(str.c_str());
	mlog("\n");
}

void MSEHTranslator(UINT nSeCode, _EXCEPTION_POINTERS* pExcPointers)
{
	MFilterException(pExcPointers);

	raise(SIGABRT);     // raise abort signal 

	// We usually won't get here, but it's possible that
	// SIGABRT was ignored.  So hose the program anyway.
	_exit(3);
}

#ifndef _PUBLISH

#define MAX_PROFILE_COUNT	10000

struct MPROFILEITEM {
	u64 dwStartTime;
	u64 dwTotalTime;
	DWORD dwCalledCount;
};
#include <unordered_map>
#include <stack>
static std::unordered_map<std::string, MPROFILEITEM> ProfileItems;
static u64 g_dwEnableTime;
static std::stack<std::string> Stack;

void MInitProfile()
{
	g_dwEnableTime=GetGlobalTimeMS();
}

ProfilerGuard MBeginProfile(const char * szName)
{
	std::string strName{ szName };
	auto& ProfileItem = ProfileItems[strName];
	ProfileItem.dwStartTime = GetGlobalTimeMS();
	ProfileItem.dwCalledCount++;
	Stack.push(std::move(strName));
	return{};
}

void MEndProfile(ProfilerGuard& Guard)
{
	Guard.Active = false;
	auto& Name = Stack.top();
	auto& ProfileItem = ProfileItems[Name];
	ProfileItem.dwTotalTime += GetGlobalTimeMS() - ProfileItem.dwStartTime;
	Stack.pop();
}

void MBeginProfile(int nIndex, const char *szName)
{
	auto lvalue = MBeginProfile(szName);
	lvalue.Active = false;
}

void MEndProfile(int nIndex)
{
	ProfilerGuard Guard;
	MEndProfile(Guard);
}

#include <algorithm>

void MSaveProfile(const char *filename)
{
	auto dwTotalTime = GetGlobalTimeMS()-g_dwEnableTime;

	assert(Stack.empty());

	FILE *file = nullptr;
	auto ret = fopen_s(&file, filename, "w+");
	if (file == nullptr || ret != 0)
	{
		MLog("Failed to open file %s to save profile stats\n", filename);
		return;
	}

	std::vector<typename decltype(ProfileItems)::iterator> vec;
	for (auto it = ProfileItems.begin(); it != ProfileItems.end(); it++)
		vec.emplace_back(it);

	std::sort(vec.begin(), vec.end(), [&](auto&& lhs, auto&& rhs) { return lhs->second.dwTotalTime > rhs->second.dwTotalTime; });

	fprintf(file," total time = %f seconds \n", (float)dwTotalTime*0.001f);
	fprintf(file,"id   (loop ms)  seconds     %%        calledcount   name \n");
	fprintf(file,"=========================================================\n");

	int i = 0;
	for (auto it : vec)
	{
		auto& Name = it->first;
		auto& ProfileItem = it->second;
		fprintf(file,"(%05d) %8.3f %8.3f ( %6.3f %% , %6u) %s \n",
			i,
			0.f,
			0.001f * ProfileItem.dwTotalTime,
			100.0f * ProfileItem.dwTotalTime / dwTotalTime,
			ProfileItem.dwCalledCount,
			Name.c_str());
		i++;
	}
	fclose(file);
}
#else
void MInitProfile() {}
void MBeginProfile(int nIndex, const char *szName) {}
void MEndProfile(int nIndex) {}
void MSaveProfile(const char *file) {}
void MBeginProfile(const char * szName) {}
void MEndProfile() {}
#endif

ProfilerGuard::~ProfilerGuard()
{
	if (Active)
		MEndProfile(*this);
}