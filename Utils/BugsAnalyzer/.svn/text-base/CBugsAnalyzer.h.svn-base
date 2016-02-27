#ifndef _CBUGSANALYZER_H
#define _CBUGSANALYZER_H

#include <list>
#include <map>
#include <string>
using namespace std;

struct BugInfo
{
	char szFuncName[1024];
	char szSrcFileName[1024];
	char szLogFileName[1024];
	int nLineNumber;
};

struct BugAnalysisItem
{
	int	nCount;
	char szFuncName[1024];
	char szSampleLogFileName[1024];
};

typedef	map<string, BugAnalysisItem*>	BugAnalysisMap;
typedef list<BugAnalysisItem*>	BugAnalysisList;
typedef list<BugInfo*>			BugInfoList;

class CBugsAnalyzer
{
private:
	char			m_szFolderPath[256];
	BugInfoList		m_BugInfoList;
	BugAnalysisList	m_BugAnalysisList;
	
	void ParseFile(const char* szFileName);
	void Analyze();
public:
	CBugsAnalyzer();
	virtual ~CBugsAnalyzer();
	bool Create(const char* szFolderPath);
	void Execute();

	BugAnalysisList* GetAnalysisList() { return &m_BugAnalysisList; }
	BugInfoList* GetBugInfoList() { return &m_BugInfoList; }
};





#endif