#pragma once


#include <map>
#include <string>
#include <vector>
#include <utility>

using std::string;
using std::vector;
using std::pair;
using std::map;


struct BlockCountryCodeInfo
{
	BlockCountryCodeInfo( const string& strCountryCode, const string& strRoutingURL, const bool bIsBlock ) 
		: m_strCountryCode( strCountryCode ), m_strRoutingURL( strRoutingURL ), m_bIsBlock( bIsBlock )
	{
	}
	
	string	m_strCountryCode;
	string	m_strRoutingURL;
	bool	m_bIsBlock;
};


struct IPRange
{
	IPRange( const DWORD dwIPFrom, const DWORD dwIPTo ) 
		: m_dwIPFrom( dwIPFrom ), m_dwIPTo( dwIPTo )
	{
	}

	DWORD m_dwIPFrom;
	DWORD m_dwIPTo;
};

struct CustomIP : public IPRange
{
	CustomIP( const DWORD dwIPFrom, const DWORD dwIPTo, const bool bIsBlock, const string& strCountryCode3, const string& strComment ) 
		: IPRange( dwIPFrom, dwIPTo ), m_strCountryCode3( strCountryCode3 ), m_strComment( strComment ), m_bIsBlock( bIsBlock )
	{
	}
	
	bool	m_bIsBlock;
	string	m_strCountryCode3;
	string	m_strComment;
};


struct IPtoCountry : public IPRange
{
	IPtoCountry( const DWORD dwIPFrom, const DWORD dwIPTo, const string& strCountryCode3 ) 
		: IPRange( dwIPFrom, dwIPTo ), m_strCountryCode3( strCountryCode3 )
	{
	}

	string	m_strCountryCode3;
};



typedef vector< BlockCountryCodeInfo* >	BlockCountryCodeList;
typedef vector< IPtoCountry* >			IPtoCountryList;
typedef vector< CustomIP* >				CustomIPList;
typedef map< DWORD, string >			InvalidIPList;


class StrICmp
{
public :
	StrICmp( const string& strSrc ) : m_strSrc( strSrc ) {}

	bool operator () ( const BlockCountryCodeInfo* pBlockCountryCodeInfo )
	{
		return 0 == stricmp( m_strSrc.c_str(), pBlockCountryCodeInfo->m_strCountryCode.c_str() );
	}

private :
	StrICmp() {}

	string m_strSrc;
};

class SortCmp
{
public :
	bool operator () ( const IPRange* pRange1, const IPRange* pRange2 )
	{
		return pRange1->m_dwIPFrom < pRange2->m_dwIPFrom;
	}
};


template <typename T >
class IPRangeAlgorithm
{
public :
	IPRangeAlgorithm();

	const int			BinarySearch( const DWORD dwIP, const T& tVector ) const;
	const int			Search( const DWORD dwIP, const T& tVector ) const;
	const bool			CheckIsDuplicateRange( const DWORD dwIPFrom, const DWORD dwIPTo, const T& tVector ) const;
	inline const bool	IsInverse( const DWORD dwIPFrom, const DWORD dwIPTo ) { return dwIPFrom > dwIPTo; }
	const bool			IsLast( const DWORD dwIPFrom, const T& tVector ) const;
	void				SortIncrease( T& tVector );
	const int			MakeEmptyBlock( const DWORD dwInsertIP, T& tVector );

#ifdef _FILTER_TEST
private :
	DWORD m_dwTraceCount;
	DWORD m_dwMaxTraceCount;
#endif
};


class MCountryFilter
{
public: // Common
	MCountryFilter(void);
	virtual ~MCountryFilter(void);

	bool Create( BlockCountryCodeList& rfBlockCountryCodeList, 
				 IPtoCountryList& rfIPtoCountryList,
				 CustomIPList& rfCustomIPList );

	bool Update( BlockCountryCodeList& rfBlockCountryCodeList, 
				 IPtoCountryList& rfIPtoCountryList,
				 CustomIPList& rfCustomIPList );

	DWORD GetLastUpdatedTime()						{ return m_dwLastUpdatedTime; }
	void SetLastUpdatedTime( const DWORD dwTime )	{ m_dwLastUpdatedTime = dwTime; }

	const BlockCountryCodeList&	GetBlockCountryCodeList() const { return m_BlockCountryCodeList; }
	const IPtoCountryList&		GetIPtoCountryList() const		{ return m_IPtoCountryList; }
	const CustomIPList&			GetCustomIPList() const			{ return m_CustomIPList; }

	void ReleaseCountryCode();
	void ReleaseBlockCountryCode();
	void ReleaseCustomIP();

public : // IPtoCountry
	bool AddIPtoCountry( const DWORD dwIPFrom, const DWORD dwIPTo, const string& strCode );
	bool GetIPCountryCode( const string& strIP, string& strCountryCode );
	bool AddInvalidIP( const DWORD dwIP, const string& strIP );
	bool InitIPtoCountryList( IPtoCountryList& rfIPtoCountryList );
	// IPtoCountry

public : // BlockCountryCode
	bool IsNotBlockCode( const string& strCountryCode, string& strRoutingURL );
	bool InitBlockCountryCodeList( BlockCountryCodeList& rfBlockCountryCodeList );
	// BlockCountryCode

public : // CustomIP
	bool AddCustomIP( const DWORD dwIPFrom, const DWORD dwIPTo, const bool bIsBlock, const string& strCountryCode3, const string& strComment );
	bool GetCustomIP( const string& strIP, bool& bIsBlock, string& strCountryCode3, string& strComment );
	bool InitCustomIPList( CustomIPList& rfCustomIPList );
	// CustomIP

private : // Common
	const DWORD inet_aton( const string& strIP );
	// Common

private : // IPtoCountry
	// IPtoCountry

private : // BlockCountryCode
	bool IsNotBlockCode( const string& strCountryCode, string& strRoutingURL, BlockCountryCodeList& bcil );
	// BlockCountryCode

private : // CustomIP
	// CustomIP

private :
	BlockCountryCodeList	m_BlockCountryCodeList;
	IPtoCountryList			m_IPtoCountryList;
	CustomIPList			m_CustomIPList;
	InvalidIPList			m_InvalidIPList;

	DWORD					m_dwLastUpdatedTime;

	IPRangeAlgorithm< IPtoCountryList >	m_IPtoCountryAlgorithm;
	IPRangeAlgorithm< CustomIPList >	m_CustomIPAlgorithm;
};

bool SplitStrIP( const string& strIP, vector<BYTE>& vIP );