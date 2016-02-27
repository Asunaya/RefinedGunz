#ifndef _MSMARTREFRESH_H
#define _MSMARTREFRESH_H

//#pragma once

#include <map>
#include <string>
using namespace std;


class MRefreshCategory {
protected:
	int				m_nCategory;
	unsigned long	m_nChecksum;

	unsigned long	m_nLastUpdateTick;

protected:

	void SetChecksum(unsigned long nChecksum)	{ m_nChecksum = nChecksum; }

	unsigned long GetLastUpdateTick()			{ return m_nLastUpdateTick; }
	void SetLastUpdateTick(unsigned long nTick)	{ m_nLastUpdateTick = nTick; }
	
	virtual bool OnUpdateChecksum(unsigned long nTick) = 0;

public:
	MRefreshCategory(int nCategory);
	virtual ~MRefreshCategory();

	int GetCategory()							{ return m_nCategory; }
	unsigned long GetChecksum()					{ return m_nChecksum; }

	inline bool UpdateChecksum(unsigned long nTick);
};
class MRefreshCategoryMap : public map<int, MRefreshCategory*>{};


class MRefreshClient {
protected:
	int				m_nCategory;
	unsigned long	m_nChecksum;
	bool			m_bEnable;
	unsigned long	m_tmLastUpdated;

protected:
	virtual bool OnSync(unsigned long nChecksum) = 0;

public:
	MRefreshClient();
	virtual ~MRefreshClient();
	
	int GetCategory()								{ return m_nCategory; }
	void SetCategory(int nCategory)					{ m_nCategory = nCategory; }

	unsigned long GetChecksum()						{ return m_nChecksum; }
	void SetChecksum(unsigned long nChecksum)		{ m_nChecksum = nChecksum; }

	bool IsEnable()									{ return m_bEnable; }
	void Enable(bool bEnable)						{ m_bEnable = bEnable; }

	unsigned long GetLastUpdatedTime()				{ return m_tmLastUpdated; }
	void SetLastUpdatedTime(unsigned long tmTime)	{ m_tmLastUpdated = tmTime; }

	bool Sync(unsigned long nChecksum);
};


class MSmartRefresh {
protected:
	MRefreshCategoryMap		m_CategoryMap;

public:
	MSmartRefresh();
	virtual ~MSmartRefresh();
	void Clear();

	MRefreshCategory* GetCategory(int nCategory);
	void AddCategory(MRefreshCategory* pCategory);
	void UpdateCategory(unsigned int nTick);

//	void AddClient(MRefreshClient* pClient);
//	void RemoveClient(MRefreshClient* pClient);
	bool SyncClient(MRefreshClient* pClient);	// Refresh되면:True, 변동없으면:False
};


#endif