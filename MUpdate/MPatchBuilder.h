#pragma once


#include "MPatchNode.h"

struct _finddata_t;


class MPatchBuilder {
protected:
	MPatchList	m_PatchList;

protected:
	void ScanDirStack(const char* szDir);	// prefer method
	void ScanDirRCS(const char* szDir);		// ugly scan order :(
	bool TraverseTest();					// just for test
	void TRACEFILE(const _finddata_t* pFile, bool bBegin=false);

public:
	MPatchBuilder();
	virtual ~MPatchBuilder();

	void Clear();
	bool GetVersion(char* pszVersion);
	bool Build(const char* pszFileName, const char* pszEncoding);
};
