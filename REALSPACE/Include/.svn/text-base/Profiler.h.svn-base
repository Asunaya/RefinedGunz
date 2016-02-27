#ifndef PROFILER_H
#define PROFILER_H

#include <windows.h>
#include <stdio.h>
#include "CMList.h"

#define BUF_SIZE 1024

struct PROFILENODE {
public:
	char* pName;
	char* pValue;
	char* pArg;
	
public:
	PROFILENODE() { memset(this, 0, sizeof(PROFILENODE)); };
	~PROFILENODE() { 
		if (pName) delete [] pName; 
		if (pValue) delete [] pValue; 
		if (pArg) delete [] pArg; 
	};
};

class Profiler {
public:
	char m_strFileName[256];
	CMLinkedList<PROFILENODE>	m_ProfileList;
	int	m_IdxFinded;

public:
	Profiler() { memset(this, 0, sizeof(Profiler)); };
	~Profiler() { m_ProfileList.DeleteAll(); };
	void CleanUp() { m_ProfileList.DeleteAll(); };

	char* GetFileName() { return m_strFileName; }
	void SetFileName(char* szFileName) { strcpy(m_strFileName, szFileName); }
	BOOL Load(char* szFileName);
	BOOL LoadPak(char* szPakFileName,char* szFileName);
	BOOL Save();
	void Parse(char* strBuf);

	BOOL Write(const char* pName, const char* pValue);
	BOOL Read(const char* pName, char* pBuf, int nBufSize);
	BOOL ReadArg(const char* pName, char* pBuf, int nBufSize);
	BOOL Read(const char* pName, char** pValue);
	BOOL Delete(const char* pName);

// Don't use Find yet...
	int  Find(const char* pName, const char* pValue);	// return List index
	int  FindNext(const char* pName, const char* pValue);	// return -1 when No found
};

BOOL ReadLine(FILE* fl, char *buf);

#endif