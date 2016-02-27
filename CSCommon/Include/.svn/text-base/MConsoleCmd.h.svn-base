#ifndef _MCONSOLECMD_H
#define _MCONSOLECMD_H

#include <string>
#include <list>
#include <vector>
#include <map>
using namespace std;

#define		MCONSOLE_TEXSIZE		4096
#define		MCONSOLE_NAMESIZE		256

#define ARGVNoMin	(-1)
#define ARGVNoMax	(-1)

struct MConsoleArgvInfo
{
	char	*cargv[256];
	int		cargc;
	char	argbuf[2048];		// 실제 argv 값이 들어가는 buf
};

class MConsoleCmd;
class MConsoleCmdManager;

typedef void(MConsoleCmdProc)(const int argc, char **const argv);
typedef map<string, MConsoleCmd*>	MConsoleCmdMap;

class MConsoleCmdMap
{
private:
	char		m_szName[MCONSOLE_NAMESIZE];
	char		m_szUsage[MCONSOLE_TEXSIZE];
	char		m_szHelp[MCONSOLE_TEXSIZE];
	int			m_nMinArgs;
	int			m_nMaxArgs;
	MConsoleCmdProc*	m_fnProc;
protected:
public:
	MConsoleCmd(const char* szName, int nMinArgs = ARGVNoMin, int nMaxArgs=ARGVNoMax,
		const char* szUsage=NULL, const char* szHelp=NULL);
	virtual ~MConsoleCmd() { }

	void OnProc(const int argc, char **const argv);
	void SetProc(MConsoleCmdProc* fnProc) { m_fnProc = fnProc; }
	const char* GetName()		{ return m_szName; }
	const char* GetUsage()		{ return m_szUsage; }
	const char* GetHelp()		{ return m_szHelp; }
	int GetMinArgs()			{ return m_nMinArgs; }
	int GetMaxArgs()			{ return m_nMaxArgs; }
};


class MConsoleCmdManager
{
private:
	MConsoleCmdMap		m_CmdMap;
	bool MakeArgv(char* szLine, MConsoleArgvInfo* pAI);
public:
	MConsoleCmdManager()		{ }
	virtual ~MConsoleCmdManager()	{ }

	void AddCommand(const char* szName, MConsoleCmdProc* fnProc,
		int nMinArgs=ARGVNoMin, int nMaxArgs=ARGVNoMax,
		const char* szUsage=NULL, const char* szHelp=NULL);
	void DoCommand(char* szLine);
	MConsoleCmd* GetCommandByName(const char* szName);
};
#endif