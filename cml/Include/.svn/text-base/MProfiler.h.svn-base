#ifndef _MPROFILER_H
#define _MPROFILER_H

#pragma warning( disable : 4786 )

#include <stack>
#include <list>

using namespace std;

#define MPROFILE_ITEM_NAME_LENGTH	64

// One Profile Item
struct MPROFILEITEM{
	char	szName[MPROFILE_ITEM_NAME_LENGTH];
	int		nStartTime;
	int		nEndTime;
};

// Accumulated Profile Log
struct MPROFILELOG{
	char	szName[MPROFILE_ITEM_NAME_LENGTH];
	int		nCount;
	int		nDepth;
	int		nTotalTime;
	int		nMaxTime;
	int		nMinTime;
};

// Temporary Profile Call Stack
class MProfileStack : public stack<MPROFILEITEM*>{
public:
	virtual ~MProfileStack(void);
};

// One Loop Log
class MProfileLoop : public list<MPROFILELOG*>{
public:
	/*
	void AddProfile(char* szName, int nDepth);
	void SetProfile(int nTime);
	*/
	void AddProfile(MPROFILELOG* pPL);

	int GetTotalTime(void);
};


// Profiler
class MProfiler{
protected:
	MProfileStack	m_ProfileStack;
	MProfileLoop	m_ProfileLoop;

	bool			m_bEnableOneLoopProfile;
	MProfileLoop*	m_pOneLoopProfile;
	MProfileLoop*	m_pOneLoopProfileResult;

	char*			m_szFirstProfileName;		// 맨처음 시작한 프로파일 이름 ( Depth 0 )

public:
	MProfiler(void);
	virtual ~MProfiler(void);

	void BeginProfile(char* szProfileName);
	void EndProfile(char* szProfileName);

	bool FinalAnalysis(char* szFileName);

	int GetTotalTime(void);

	// One Loop Profiling
	void EnableOneLoopProfile(bool bEnable);
	bool IsOneLoopProfile(void);
	// if not enabled, return NULL
	MProfileLoop* GetOneLoopProfile(void);

	// Accumulated Profile Result
	MProfileLoop* GetProfile(void);
};


// Global Profiler
extern MProfiler	g_DefaultProfiler;

// Instance 형태의 Profile
class MProfileInstance{
	char	m_szProfileName[MPROFILE_ITEM_NAME_LENGTH];
public:
	MProfileInstance(char* szProfileName){
		g_DefaultProfiler.BeginProfile(szProfileName);
		strcpy(m_szProfileName, szProfileName);
	}
	virtual ~MProfileInstance(void){
		g_DefaultProfiler.EndProfile(m_szProfileName);
	}
};


#ifdef _DO_NOT_USE_PROFILER

#define BEGINPROFILE(_szProfileName)	;
#define ENDPROFILE(_szProfileName)		;
#define FINALANALYSIS(_szFileName)		;
#define RUNWITHPROFILER(_Function)		_Function;
#define ENABLEONELOOPPROFILE(_bEnable)	;
#define GETONELOOPPROFILE()	0;
#define GETPROFILE();
#define PROFILEINSTANCE(_szProfileName)	;

#else

// Macro for Global Profiler
#define BEGINPROFILE(_szProfileName)	g_DefaultProfiler.BeginProfile(_szProfileName)
#define ENDPROFILE(_szProfileName)		g_DefaultProfiler.EndProfile(_szProfileName)
#define FINALANALYSIS(_szFileName)		g_DefaultProfiler.FinalAnalysis(_szFileName)
#define RUNWITHPROFILER(_Function)		BEGINPROFILE(#_Function); _Function; ENDPROFILE(#_Function);
#define ENABLEONELOOPPROFILE(_bEnable)	g_DefaultProfiler.EnableOneLoopProfile(_bEnable)
#define GETONELOOPPROFILE()				g_DefaultProfiler.GetOneLoopProfile()
#define GETPROFILE()					g_DefaultProfiler.GetProfile()

#define PROFILEINSTANCE(_szProfileName)	MProfileInstance __ProfileInstance(_szProfileName);

#endif

// Simple Macro
#define _BP(_szProfileName)				BEGINPROFILE(_szProfileName)
#define _EP(_szProfileName)				ENDPROFILE(_szProfileName)
#define _RP(_Function)					RUNWITHPROFILER(_Function)
#define _FA(_szFileName)				FINALANALYSIS(_szFileName)
#define _PI(_szProfileName)				PROFILEINSTANCE(_szProfileName)


#endif
