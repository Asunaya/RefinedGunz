#ifndef _MEMBLEMMGR_H
#define _MEMBLEMMGR_H

//#pragma once


#include "MHttpSpooler.h"
#include <string>
#include <map>
using namespace std;


class MEmblemNode {
protected:
	unsigned int	m_nCLID;
	unsigned long	m_nChecksum;
	char			m_szURL[256];
	time_t			m_tmLastUsed;
public:
	MEmblemNode()			{ m_nCLID=0; m_nChecksum=0; m_szURL[0]=NULL; m_tmLastUsed=0; }
	virtual ~MEmblemNode()	{}

	unsigned int GetCLID()			{ return m_nCLID; }
	void SetCLID(unsigned int nCLID){ m_nCLID = nCLID; }
	const char* GetURL()			{ return m_szURL; }
	void SetURL(const char* pszURL)	{ strcpy(m_szURL, pszURL); }
	unsigned long GetChecksum()		{ return m_nChecksum; }
	void SetChecksum(unsigned long nChecksum)	{ m_nChecksum = nChecksum; }

	time_t GetTimeLastUsed()		{ return m_tmLastUsed; }
	void SetTimeLastUsed(time_t tm)	{ m_tmLastUsed = tm; }
	void UpdateTimeLastUsed()		{ time(&m_tmLastUsed); }
};
class MEmblemMap : public map<unsigned int, MEmblemNode*> {};


class MEmblemMgr {
protected:
	MHttpSpooler	m_HttpSpooler;
	
	MEmblemMap		m_EmblemMap;

	char			m_szEmblemBaseDir[_MAX_DIR];
	char			m_szEmblemDataFile[_MAX_DIR];

	bool			m_bSave;
	unsigned long	m_tmLastSavedTick;

	int				m_nTotalRequest;
	int				m_nCachedRequest;
	
protected:
	bool InitDefaut();
	const char* GetEmblemBaseDir()				{ return m_szEmblemBaseDir; }
	const char* GetEmblemDataFile()				{ return m_szEmblemDataFile; }

	bool CheckSaveFlag()						{ return m_bSave; }
	void SetSaveFlag(bool bSave)				{ m_bSave = bSave; }
	unsigned long GetLastSavedTick()			{ return m_tmLastSavedTick; }
	void SetLastSavedTick(unsigned long nTick)	{ m_tmLastSavedTick = nTick; }

	bool CreateCache();
	bool LoadCache();
	bool SaveCache();
	void ClearCache();

	void PostDownload(unsigned int nCLID, unsigned int nChecksum, const char* pszURL);
	bool RegisterEmblem(unsigned int nCLID, const char* pszURL, unsigned long nChecksum, time_t tmLastUsed=0);
	void NotifyDownloadDone(unsigned int nCLID, const char* pszURL);

public:
	void Create();
	void Destroy();

	bool GetEmblemPath(char* pszFilePath, const char* pszURL);
	bool GetEmblemPathByCLID(unsigned int nCLID, char* poutFilePath);

	int GetTotalRequest()	{ return m_nTotalRequest; }
	int GetCachedRequest()	{ return m_nCachedRequest; }

	bool PrepareCache();

	bool CheckEmblem(unsigned int nCLID, unsigned long nChecksum);
	bool ProcessEmblem(unsigned int nCLID, const char* pszURL, unsigned long nChecksum);

	void Tick(unsigned long nTick);
};


#endif