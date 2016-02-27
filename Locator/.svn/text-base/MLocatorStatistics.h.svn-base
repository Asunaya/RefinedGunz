#ifndef _LOCATOR_STATISTICS
#define _LOCATOR_STATISTICS

#include <map>
#include <string>
using namespace std;


typedef map< string, DWORD >	CountryStatisticsMap;
typedef map< int, DWORD >		DeadServerStatisticsMap;


class MLocatorStatistics
{
public :
	~MLocatorStatistics();

	void Reset();

	void InitInsertCountryCode( const string& strCountryCode3 );

	void IncreaseCountryStatistics( const string& strCountryCode3, const int nCount = 1 );
	void IncreaseDeadServerStatistics( const int nServerID );
	void IncreaseCountryCodeCacheHitMissCount() { ++m_dwCountryCodeCacheHitMissCount; }
	void IncreaseInvalidIPCount()				{ ++m_dwInvalidIPCount; }
	void IncreaseBlockCount()					{ ++m_dwBlockCount; }
	void IncreaseBlockCountryCodeHitCount()		{ ++m_dwBlockCountryCodeHitCount; }
	
	const CountryStatisticsMap& GetCountryStatistics() const	{ return m_CountryStatistics; }
	const DWORD GetLastUpdatedTime()							{ return m_dwLastUpdatedTime; }
	
	void SetLastUpdatedTime( const DWORD dwTime )	{ m_dwLastUpdatedTime = dwTime; }
	void SetDeadServerCount( const DWORD dwCount )	{ m_dwDeadServerCount = dwCount; }
	void SetLiveServerCount( const DWORD dwCount )	{ m_dwLiveServerCount = dwCount; }

	static MLocatorStatistics& GetInstance() 
	{
		static MLocatorStatistics LocatorStatistics;
		return LocatorStatistics;
	}

private :
	MLocatorStatistics();

	void IncreaseCountryCodeCheckCount() { ++m_dwCountryCodeCheckCount; }

	const DWORD GetCountryCodeCacheHitMissCount() const { return m_dwCountryCodeCacheHitMissCount; }
	const DWORD GetInvalidIPCount() const				{ return m_dwInvalidIPCount; }
	const DWORD GetCountryCodeCheckCount() const		{ return m_dwCountryCodeCheckCount; }
	const DWORD GetBlockCount() const					{ return m_dwBlockCount; }
	const DWORD GetDeadServerCount() const				{ return m_dwDeadServerCount; }
	const DWORD GetLiveServerCount() const				{ return m_dwLiveServerCount; }
	const DWORD GetBlockCountryCodeHitCount() const		{ return m_dwBlockCountryCodeHitCount; }

	void ResetCountryStatistics();
	void ResetDeadServerStatistics();
	void ResetCountryCodeCacheHitMissCount()	{ m_dwCountryCodeCacheHitMissCount = 0; }
	void ResetInvalidIPCount()					{ m_dwInvalidIPCount = 0; }
	void ResetCountryCodeCheckCount()			{ m_dwCountryCodeCheckCount = 0; }
	void ResetBlockCount()						{ m_dwBlockCount = 0; }
	void ResetBlockCountryCodeHitCount()		{ m_dwBlockCountryCodeHitCount = 0; }

	void DumpSelfLog();
	
private :
	CountryStatisticsMap	m_CountryStatistics;
	DeadServerStatisticsMap m_DeadServerStatistics;
	DWORD					m_dwCountryCodeCacheHitMissCount;
	DWORD					m_dwInvalidIPCount;
	DWORD					m_dwCountryCodeCheckCount;
	DWORD					m_dwBlockCount;
	DWORD					m_dwLiveServerCount;
	DWORD					m_dwDeadServerCount;
	DWORD					m_dwBlockCountryCodeHitCount;

	DWORD					m_dwLastUpdatedTime;
};

MLocatorStatistics& GetLocatorStatistics();

#endif
