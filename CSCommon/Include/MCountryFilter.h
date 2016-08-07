#pragma once


#include <map>
#include <string>
#include <vector>
#include <utility>
#include <cstdint>

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
	IPRange( const uint32_t dwIPFrom, const uint32_t dwIPTo ) 
		: m_dwIPFrom( dwIPFrom ), m_dwIPTo( dwIPTo )
	{
	}

	uint32_t m_dwIPFrom;
	uint32_t m_dwIPTo;
};

struct CustomIP : public IPRange
{
	CustomIP( const uint32_t dwIPFrom, const uint32_t dwIPTo, const bool bIsBlock, const string& strCountryCode3, const string& strComment ) 
		: IPRange( dwIPFrom, dwIPTo ), m_strCountryCode3( strCountryCode3 ), m_strComment( strComment ), m_bIsBlock( bIsBlock )
	{
	}
	
	bool	m_bIsBlock;
	string	m_strCountryCode3;
	string	m_strComment;
};


struct IPtoCountry : public IPRange
{
	IPtoCountry( const uint32_t dwIPFrom, const uint32_t dwIPTo, const string& strCountryCode3 ) 
		: IPRange( dwIPFrom, dwIPTo ), m_strCountryCode3( strCountryCode3 )
	{
	}

	string	m_strCountryCode3;
};



typedef vector< BlockCountryCodeInfo* >	BlockCountryCodeList;
typedef vector< IPtoCountry* >			IPtoCountryList;
typedef vector< CustomIP* >				CustomIPList;
typedef map< uint32_t, string >			InvalidIPList;


class StrICmp
{
public :
	StrICmp( const string& strSrc ) : m_strSrc( strSrc ) {}

	bool operator () ( const BlockCountryCodeInfo* pBlockCountryCodeInfo )
	{
		return 0 == _stricmp( m_strSrc.c_str(), pBlockCountryCodeInfo->m_strCountryCode.c_str() );
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

	const int			BinarySearch( const uint32_t dwIP, const T& tVector ) const;
	const int			Search( const uint32_t dwIP, const T& tVector ) const;
	const bool			CheckIsDuplicateRange( const uint32_t dwIPFrom, const uint32_t dwIPTo, const T& tVector ) const;
	inline const bool	IsInverse( const uint32_t dwIPFrom, const uint32_t dwIPTo ) { return dwIPFrom > dwIPTo; }
	const bool			IsLast( const uint32_t dwIPFrom, const T& tVector ) const;
	void				SortIncrease( T& tVector );
	const int			MakeEmptyBlock( const uint32_t dwInsertIP, T& tVector );

#ifdef _FILTER_TEST
private :
	uint32_t m_dwTraceCount;
	uint32_t m_dwMaxTraceCount;
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

	uint32_t GetLastUpdatedTime()						{ return m_dwLastUpdatedTime; }
	void SetLastUpdatedTime( const uint32_t dwTime )	{ m_dwLastUpdatedTime = dwTime; }

	const BlockCountryCodeList&	GetBlockCountryCodeList() const { return m_BlockCountryCodeList; }
	const IPtoCountryList&		GetIPtoCountryList() const		{ return m_IPtoCountryList; }
	const CustomIPList&			GetCustomIPList() const			{ return m_CustomIPList; }

	void ReleaseCountryCode();
	void ReleaseBlockCountryCode();
	void ReleaseCustomIP();

public : // IPtoCountry
	bool AddIPtoCountry( const uint32_t dwIPFrom, const uint32_t dwIPTo, const string& strCode );
	bool GetIPCountryCode( const string& strIP, string& strCountryCode );
	bool AddInvalidIP( const uint32_t dwIP, const string& strIP );
	bool InitIPtoCountryList( IPtoCountryList& rfIPtoCountryList );
	// IPtoCountry

public : // BlockCountryCode
	bool IsNotBlockCode( const string& strCountryCode, string& strRoutingURL );
	bool InitBlockCountryCodeList( BlockCountryCodeList& rfBlockCountryCodeList );
	// BlockCountryCode

public : // CustomIP
	bool AddCustomIP( const uint32_t dwIPFrom, const uint32_t dwIPTo, const bool bIsBlock, const string& strCountryCode3, const string& strComment );
	bool GetCustomIP( const string& strIP, bool& bIsBlock, string& strCountryCode3, string& strComment );
	bool InitCustomIPList( CustomIPList& rfCustomIPList );
	// CustomIP

private : // Common
	const uint32_t inet_aton( const string& strIP );
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

	uint32_t					m_dwLastUpdatedTime;

	IPRangeAlgorithm< IPtoCountryList >	m_IPtoCountryAlgorithm;
	IPRangeAlgorithm< CustomIPList >	m_CustomIPAlgorithm;
};

bool SplitStrIP( const string& strIP, vector<uint8_t>& vIP );