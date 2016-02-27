#ifndef _MMATCHPREMIUMIPCACHE_H
#define _MMATCHPREMIUMIPCACHE_H


#include <map>
using namespace std;

class MMatchPremiumIPNode
{
private:
	DWORD	m_dwIP;
	DWORD	m_tmTime;	// Last updated time
public:
	MMatchPremiumIPNode(DWORD dwIP, DWORD tmTime) { m_dwIP=dwIP; m_tmTime=tmTime; }
	DWORD GetIP()			{ return m_dwIP; }
	void SetIP(DWORD dwIP)	{ m_dwIP = dwIP; }
	DWORD GetTime()			{ return m_tmTime; }
	void SetTime(DWORD tmTime)	{ m_tmTime = tmTime; }
};

class MMatchPremiumIPMap : public map<DWORD, MMatchPremiumIPNode>{};

class MMatchPremiumIPCache
{
private:
	CRITICAL_SECTION			m_csLock;
	MMatchPremiumIPMap			m_PremiumIPMap;
	MMatchPremiumIPMap			m_NotPremiumIPMap;
	int							m_nDBFailedCount;
	int							m_nFailedCheckCount;

	void Lock()		{ EnterCriticalSection(&m_csLock); }
	void Unlock()	{ LeaveCriticalSection(&m_csLock); }
public:
	MMatchPremiumIPCache();
	~MMatchPremiumIPCache();
	static MMatchPremiumIPCache* GetInstance();

	bool CheckPremiumIP(DWORD dwIP, bool& outIsPremiumIP);
	void AddIP(DWORD dwIP, bool bPremiumIP);
	void OnDBFailed();
	void Update();
};

inline MMatchPremiumIPCache* MPremiumIPCache()
{
	return MMatchPremiumIPCache::GetInstance();
}




#endif