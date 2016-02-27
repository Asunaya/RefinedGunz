#pragma once


#include <list>
using namespace std;


// MAIET FILE TRANSFER //

//// COMMAND ID ///////////////////////////////////////
enum MFT_CMDID {	
	MFT_CMDID_SET_PROGRESS_CONTEXT,
	MFT_CMDID_CHDIR,
	MFT_CMDID_REQUESTLIST,
	MFT_CMDID_REQUESTFILE,
	MFT_CMDID_END,
};


//// ABSTRACT COMMAND /////////////////////////////////
class MFTCmd {
protected:
	unsigned int	m_nID;
	unsigned long	m_nContext;
public:
	MFTCmd()					{ m_nID = 0; m_nContext = 0; }
	virtual ~MFTCmd()			{ }

	int GetID()					{ return m_nID; }
	int GetContext()			{ return m_nContext; }
	void SetContext(unsigned long nContext)	{ m_nContext = nContext; }
};
typedef list<MFTCmd*>	MFTCmdList;


//// COMMANDS /////////////////////////////////////////
class MFTCmd_SETPROGRESSCONTEXT : public MFTCmd {
public:
	MFTCmd_SETPROGRESSCONTEXT() {
		m_nID = MFT_CMDID_SET_PROGRESS_CONTEXT;
	}
};

class MFTCmd_CHDIR : public MFTCmd {
	char	m_szDir[_MAX_DIR];
public:
	MFTCmd_CHDIR(const char* pszDir) {
		m_nID = MFT_CMDID_CHDIR;
		strcpy(m_szDir, pszDir);
	}
	const char* GetDir()	{ return m_szDir; }
};

class MFTCmd_REQUESTLIST : public MFTCmd {
public:
	MFTCmd_REQUESTLIST() {
		m_nID = MFT_CMDID_REQUESTLIST;
	}
};

class MFTCmd_REQUESTFILE : public MFTCmd {
	char	m_szRemoteFileName[_MAX_DIR];
	char	m_szLocalFileName[_MAX_DIR];
public:
	MFTCmd_REQUESTFILE(const char* pszRemoteFileName, const char* pszLocalFileName) {
		m_nID = MFT_CMDID_REQUESTFILE;
		strcpy(m_szRemoteFileName, pszRemoteFileName);
		strcpy(m_szLocalFileName, pszLocalFileName);
	}
	const char* GetRemoteFileName()	{ return m_szRemoteFileName; }
	const char* GetLocalFileName()	{ return m_szLocalFileName; }
};
