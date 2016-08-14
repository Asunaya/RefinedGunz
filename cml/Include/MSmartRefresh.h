#pragma once

#include <map>
#include <string>

#include "GlobalTypes.h"


class MRefreshCategory {
protected:
	int				m_nCategory;
	unsigned long	m_nChecksum;

	u64				m_nLastUpdateTick;

protected:

	void SetChecksum(unsigned long nChecksum)	{ m_nChecksum = nChecksum; }

	auto GetLastUpdateTick()			{ return m_nLastUpdateTick; }
	void SetLastUpdateTick(u64 nTick)	{ m_nLastUpdateTick = nTick; }
	
	virtual bool OnUpdateChecksum(u64 nTick) = 0;

public:
	MRefreshCategory(int nCategory);
	virtual ~MRefreshCategory();

	int GetCategory()							{ return m_nCategory; }
	unsigned long GetChecksum()					{ return m_nChecksum; }
	bool UpdateChecksum(u64 nTick);
};
class MRefreshCategoryMap : public std::map<int, MRefreshCategory*>{};


class MRefreshClient {
protected:
	int				m_nCategory;
	unsigned long	m_nChecksum;
	bool			m_bEnable;
	u64				m_tmLastUpdated;

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

	auto GetLastUpdatedTime()				{ return m_tmLastUpdated; }
	void SetLastUpdatedTime(u64 tmTime)	{ m_tmLastUpdated = tmTime; }

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
	void UpdateCategory(u64 nTick);

	bool SyncClient(MRefreshClient* pClient);
};