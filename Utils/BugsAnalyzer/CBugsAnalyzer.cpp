#include "stdafx.h"
#include "CBugsAnalyzer.h"

CBugsAnalyzer::CBugsAnalyzer()
{
	m_szFolderPath[0] = 0;
}

CBugsAnalyzer::~CBugsAnalyzer()
{
}


bool CBugsAnalyzer::Create(const char* szFolderPath)
{
	m_BugInfoList.clear();

	strcpy(m_szFolderPath, szFolderPath);
	return true;
}

void CBugsAnalyzer::Execute()
{
	CString strFindPath, strReadPath;
	BOOL bContinue;

	strFindPath = CString(m_szFolderPath);
	strFindPath.Append("*.txt");
	CFileFind filefind;
	bContinue = filefind.FindFile(strFindPath);

	if(!bContinue)
	{
		filefind.Close();
		return;
	}

	while(bContinue)
	{
		bContinue = filefind.FindNextFile();
		strReadPath = filefind.GetFileName();

		if(!filefind.IsDirectory())
		{
			strReadPath = filefind.GetFileName();
			CString strFullFileName = CString(m_szFolderPath);
			strFullFileName.Append(strReadPath);

			ParseFile(strFullFileName);
		}
	}
	filefind.Close();
	
	Analyze();
}


void CBugsAnalyzer::ParseFile(const char* szFileName)
{
	FILE* fp = fopen(szFileName, "rt");
	BugInfo* pBugInfo = new BugInfo;
	ZeroMemory(pBugInfo, sizeof(BugInfo));

	char szLine[1024];
	bool bDone = false;
	if (fp)
	{
		strcpy(pBugInfo->szLogFileName, szFileName);
		while (fgets(szLine, 1024, fp))
		{
			if (!strnicmp("     Function Name : ", szLine, 20))
			{
				if (bDone)
				{
					m_BugInfoList.push_back(pBugInfo);
					return;				
				}

				strcpy(pBugInfo->szFuncName, &szLine[20]);
				pBugInfo->szFuncName[strlen(pBugInfo->szFuncName)-1] = 0;
				bDone = true;
			}
			if (!strnicmp("     File Name : ", szLine, 16))
			{
				strcpy(pBugInfo->szSrcFileName, &szLine[16]);
				pBugInfo->szSrcFileName[strlen(pBugInfo->szSrcFileName)-1] = 0;
			}
			if (!strnicmp("     Line Number : ", szLine, 19))
			{
				char szNum[256] = "";
				strcpy(szNum, &szLine[19]);
				try
				{
					pBugInfo->nLineNumber = atoi(szNum);
				}
				catch(...)
				{
					pBugInfo->nLineNumber = 0;
				}
			}
		}

		fclose(fp);
	}
}

bool static CompareBugsCount(BugAnalysisItem* a, BugAnalysisItem* b) 
{
	if(a->nCount > b->nCount) return false;
	if(a->nCount < b->nCount) return true;

	return false;
}


void CBugsAnalyzer::Analyze()
{
	BugAnalysisMap	m_BugAnalysisMap;

	for (list<BugInfo*>::iterator itor = m_BugInfoList.begin();
		itor != m_BugInfoList.end(); ++itor)
	{
		BugInfo* pBugInfo = (*itor);

		BugAnalysisMap::iterator MapItor = m_BugAnalysisMap.find(string(pBugInfo->szFuncName));
		if (MapItor != m_BugAnalysisMap.end())
		{
			BugAnalysisItem* pItem = (*MapItor).second;
			pItem->nCount++;
		}
		else
		{
			BugAnalysisItem* pNewItem = new BugAnalysisItem;
			pNewItem->nCount = 1;
			strcpy(pNewItem->szSampleLogFileName, pBugInfo->szLogFileName);
			strcpy(pNewItem->szFuncName, pBugInfo->szFuncName);
			
			m_BugAnalysisMap.insert(BugAnalysisMap::value_type(string(pBugInfo->szFuncName), pNewItem));
		}
	}

	m_BugAnalysisList.clear();

	for (BugAnalysisMap::iterator itor = m_BugAnalysisMap.begin();
		itor != m_BugAnalysisMap.end(); ++itor)
	{
		BugAnalysisItem* pItem = (*itor).second;

		m_BugAnalysisList.push_back(pItem);
	}

	m_BugAnalysisList.sort(CompareBugsCount);
}