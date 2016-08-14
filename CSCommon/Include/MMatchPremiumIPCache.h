#ifndef _MMATCHPREMIUMIPCACHE_H
#define _MMATCHPREMIUMIPCACHE_H


#include <map>

class MMatchPremiumIPNode
{
private:
	DWORD	IP;
	u64		Time;	// Last updated time
public:
	MMatchPremiumIPNode(u32 IP, u64 tmTime) : IP(IP), Time(Time) {}
	auto GetIP() const			{ return IP; }
	void SetIP(DWORD dwIP)		{ IP = dwIP; }
	auto GetTime() const		{ return Time; }
	void SetTime(u64 tmTime)	{ Time = tmTime; }
};

class MMatchPremiumIPMap : public std::map<DWORD, MMatchPremiumIPNode> {};

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