#pragma once

#include <string>
#include <list>
#include <vector>
#include <map>

#define		ZCHATCMD_TEXSIZE	2048
#define		ZCHATCMD_NAMESIZE	256
#define		ZCHATCMD_LIMIT_REPEAT_COUNT 3

#define ARGVNoMin	(-1)
#define ARGVNoMax	(-1)

struct ZChatCmdArgvInfo
{
	char	*cargv[256];
	int		cargc;
	char	argbuf[2048];
};

class ZChatCmd;
class ZChatCmdManager;

typedef std::map<std::string, ZChatCmd*>	ZChatCmdMap;
typedef void(ZChatCmdProc)(const char* line, const int argc, char **const argv);

enum ZChatCmdFlag
{
	CCF_NONE	= 0,
	CCF_LOBBY	= 0x01,
	CCF_STAGE	= 0x02,
	CCF_GAME	= 0x04,
	CCF_ALL		= 0x0F,

	CCF_TEST	= 0x40,
	CCF_ADMIN	= 0x80	
};



class ZChatCmd
{
private:
	int					m_nID;
	char				m_szName[ZCHATCMD_NAMESIZE];
	char				m_szUsage[ZCHATCMD_TEXSIZE];
	char				m_szHelp[ZCHATCMD_TEXSIZE];
	int					m_nMinArgs;
	int					m_nMaxArgs;
	bool				m_bRepeatEnabled;
	unsigned long int	m_nFlag;
	ZChatCmdProc*		m_fnProc;
protected:
public:
	ZChatCmd(int nID, const char* szName, unsigned long int flag,
		int nMinArgs = ARGVNoMin, int nMaxArgs=ARGVNoMax, bool bRepeatEnabled=true,
		const char* szUsage=NULL, const char* szHelp=NULL);
	virtual ~ZChatCmd() { }

	void OnProc(const char* line, const int argc, char **const argv);
	void SetProc(ZChatCmdProc* fnProc) { m_fnProc = fnProc; }
	const int GetID()				{ return m_nID; }
	const char* GetName()			{ return m_szName; }
	const char* GetUsage()			{ return m_szUsage; }
	const char* GetHelp()			{ return m_szHelp; }
	ZChatCmdProc* GetProc()			{ return m_fnProc; }
	int GetMinArgs()				{ return m_nMinArgs; }
	int GetMaxArgs()				{ return m_nMaxArgs; }
	unsigned long int GetFlag()		{ return m_nFlag; }
	bool GetRepeatEnabled()			{ return m_bRepeatEnabled; }
};

class ZChatCmdManager
{
private:
	ZChatCmdMap						m_CmdMap;
	map<std::string, std::string>	m_AliasMap;
	ZChatCmd* MakeArgv(const char* szLine, ZChatCmdArgvInfo* pAI);
public:
	enum CmdInputFlag
	{
		CIF_NORMAL	= 0x1,
		CIF_ADMIN	= 0x2,
		CIF_TESTER	= 0x4,
	};

	ZChatCmdManager();
	virtual ~ZChatCmdManager();

	void AddCommand(int					nID,
					const char*			szName, 
					ZChatCmdProc*		fnProc, 
					unsigned long int	flag,
					int					nMinArgs=ARGVNoMin, 
					int					nMaxArgs=ARGVNoMax,
					bool				bRepeatEnabled=true,
					const char*			szUsage=NULL, 
					const char*			szHelp=NULL);
	void AddAlias(const char* szNewCmdName, const char* szTarCmdName);
	bool IsRepeatEnabled(const char* szLine);
	bool DoCommand(const char* szLine, ZChatCmdFlag nCurrFlag, unsigned long nInputFlag=CIF_NORMAL);
	ZChatCmd* GetCommandByName(const char* szName);
	ZChatCmd* GetCommandByID(int nID);
	ZChatCmdMap::iterator GetCmdBegin() { return m_CmdMap.begin(); }
	ZChatCmdMap::iterator GetCmdEnd()	{ return m_CmdMap.end(); }
	int GetCmdCount() const { return (int)m_CmdMap.size(); }
};

void ZImplodeChatCmdArgs(char* szOut, size_t maxlen, int argc, char **const argv, int nFirstIndex = 0);
template <size_t size>
void ZImplodeChatCmdArgs(char (&szOut)[size], int argc, char **const argv, int nFirstIndex = 0) {
	return ZImplodeChatCmdArgs(szOut, size, argc, argv, nFirstIndex);
}


#include "MXmlParser.h"

class ZCmdXmlParser : public MXmlParser
{
public:
	struct _CmdStr
	{
		char			szName[ZCHATCMD_NAMESIZE];
		char			szUsage[ZCHATCMD_TEXSIZE];
		char			szHelp[ZCHATCMD_TEXSIZE];
		vector<string>	vecAliases;
		_CmdStr()
		{
			szName[0] = szUsage[0] = szHelp[0] = 0;
		}
	};
private:
	map<int, _CmdStr*>		m_CmdMap;

	virtual void ParseRoot(const char* szTagName, MXmlElement* pElement);
	void ParseCmd(MXmlElement* pElement);
public:
	ZCmdXmlParser();
	virtual ~ZCmdXmlParser();
	void Clear();
	_CmdStr* GetStr(int nID);
};