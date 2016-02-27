#include "stdafx.h"
#include "MCountryCodeFilter.h"
#include "MDebug.h"
#include <windows.h>
#include <mmsystem.h>

#include <algorithm>

MCountryCodeFilter::MCountryCodeFilter() : m_dwLastUpdatedTime( timeGetTime() )
{
}


MCountryCodeFilter::~MCountryCodeFilter()
{
}


bool MCountryCodeFilter::Create( const BlockCountryCodeInfoList& rfBlockCountryCodeInfoList, const IPtoCountryList& rfIPtoCountryList )
{
	if( rfBlockCountryCodeInfoList.empty() ) return false;

	if( !InitContryCodeTableList(rfBlockCountryCodeInfoList) ) 
		return false;

	if( !InitIPtoCountryList(rfIPtoCountryList) )
		return false;

	return true;
}


bool MCountryCodeFilter::AddIPtoCountry( const DWORD dwIPFrom, const DWORD dwIPTo, const string& strCode )
{
	if( CheckIsInverseRange(dwIPFrom, dwIPTo) )
		return false;

	if( CheckIsDuplicatedRange(dwIPFrom, dwIPTo, m_IPtoCountryList) )
		return false;

	IPtoCountry ipc;

	ipc.nIPFrom			= dwIPFrom;
	ipc.nIPTo			= dwIPTo;
	ipc.strCountryCode3 = strCode; 

	m_IPtoCountryList.push_back( ipc );

	sort( m_IPtoCountryList.begin(), m_IPtoCountryList.end(), SortCmp() );

	return true;
}


bool MCountryCodeFilter::InitContryCodeTableList( const BlockCountryCodeInfoList& rfBlockCountryCodeInfoList )
{
	if( rfBlockCountryCodeInfoList.empty() ) return false;

	m_BlockCountryCodeInfoList.clear();

	string strRoutingURL;
	BlockCountryCodeInfoList::const_iterator it, end;

	end = rfBlockCountryCodeInfoList.end();
	for( it = rfBlockCountryCodeInfoList.begin(); it != end; ++it )
	{
		if( !IsValidContryCode((*it).strCountryCode, strRoutingURL, m_BlockCountryCodeInfoList) )
		{
			m_BlockCountryCodeInfoList.push_back( (*it) );
		}
		else
		{
			ASSERT( 0 && "중복된 코드." );
			return false;
		}
	}

	return true;
}


bool MCountryCodeFilter::IsValidContryCode( const string& strCountryCode, string& strOutRoutingURL, BlockCountryCodeInfoList& bcil )
{
	if( 3 != strCountryCode.length() )
		return false;

	BlockCountryCodeInfoList::iterator itFind = find_if( bcil.begin(), 
														 bcil.end(), 
														 StrICmp(strCountryCode) );

	if( bcil.end() != itFind )
	{
		if( itFind->bIsBlock )
		{
			strOutRoutingURL = itFind->strRoutingURL;
			return false;
		}
	}
	else
	{
		strOutRoutingURL = "NO_";
		return false;
	}
	
	return true;
}


bool MCountryCodeFilter::IsValidContryCode( const string& strCountryCode, string& strOutRoutingURL )
{
	return IsValidContryCode( strCountryCode, strOutRoutingURL, m_BlockCountryCodeInfoList );
}


bool MCountryCodeFilter::InitIPtoCountryList( const IPtoCountryList& rfIPtoCountryList )
{
	if( rfIPtoCountryList.empty() )
		return false;

	m_IPtoCountryList.clear();

	IPtoCountryList::const_iterator it, end;

	end = rfIPtoCountryList.end();
	for( it = rfIPtoCountryList.begin(); it != end; ++it )
	{
		if( !CheckIPtoCountryRange(it->nIPFrom, it->nIPTo, m_IPtoCountryList) )
		{
			ASSERT( 0 && "중복되는 구간이 있음.\n" );
			return false;
		}

		m_IPtoCountryList.push_back( (*it) );
	}

	return true;
}


const DWORD MCountryCodeFilter::inet_aton( const string& strIP )
{
	if( strIP.empty() ) 
		return 0;

	size_t a, b, c;
	char szPos1[ 4 ] = {0,};
	char szPos2[ 4 ] = {0,};
	char szPos3[ 4 ] = {0,};
	char szPos4[ 4 ] = {0,};

	a = strIP.find( "." );
	if( string::npos == a ) 
		return 0;

	b = strIP.find( ".", a + 1 );
	if( string::npos == b ) 
		return 0;

	c = strIP.find( ".", b + 1 );
	if( string::npos == c )
		return 0;

	strncpy( szPos1, &strIP[0], a );
	strncpy( szPos2, &strIP[a + 1], b - a - 1 );
	strncpy( szPos3, &strIP[b + 1], c - b - 1 );
	strncpy( szPos4, &strIP[c + 1], strIP.length() - c - 1 );

	DWORD dwPos1, dwPos2, dwPos3, dwPos4;

	dwPos1 = static_cast< DWORD >( atoi(szPos1) );
	dwPos2 = static_cast< DWORD >( atoi(szPos2) );
	dwPos3 = static_cast< DWORD >( atoi(szPos3) );
	dwPos4 = static_cast< DWORD >( atoi(szPos4) );

	DWORD dwTotal = (dwPos1 * 16777216) + (dwPos2 * 65536) + (dwPos3 * 256) + dwPos4;

	return dwTotal;
}


const int MCountryCodeFilter::GetIPCountryCode( const string& strIP, string& strOutCountryCode )
{
	const DWORD dwIP = inet_aton( strIP );
	if( 0 == dwIP )
		return false;

	m_IPCountryCodeSearch.SetIP( dwIP );
	const int idx = m_IPCountryCodeSearch.BinarySearch( m_IPtoCountryList );
	if( -1 != idx )
		strOutCountryCode = m_IPtoCountryList[ idx ].strCountryCode3;
	return idx;
/*
	size_t nHead = 0;
	size_t nTail = m_IPtoCountryList.size() - 1;
	size_t nMiddle = nTail / 2;

	DWORD dwIPFrom;
	DWORD dwIPTo;

	while( true )
	{
		dwIPFrom = m_IPtoCountryList[ nMiddle ].nIPFrom;
		dwIPTo   = m_IPtoCountryList[ nMiddle ].nIPTo;

		if( dwIPFrom <= dwIP && dwIPTo >= dwIP )
		{
			strOutCountryCode = m_IPtoCountryList[ nMiddle ].strCountryCode3;
#ifdef _LOCATOR_TEST
			if( -1 == idx ) 
				mlog( "어흑 틀리다...(%u)\n", dwIP );
#endif
			return true;
		}
		else if( (0 == nMiddle) || (nHead == nTail) || (nHead > nTail) )
		{
			break;;
		}
		else if( dwIPFrom < dwIP )
		{
			nHead = nMiddle + 1;
			nMiddle = (nHead + nTail) / 2;
		}
		else if( dwIPFrom > dwIP )
		{
			nTail = nMiddle - 1;
			nMiddle = (nHead + nTail) / 2;
		}
	}

	return false;
	*/
}


const int MCountryCodeFilter::GetCustomIP( const string& strIP, string& strOutCountryCode, bool& bIsBlock, string& strComment )
{
	const DWORD dwIP = inet_aton( strIP );
	if( 0 == dwIP )
		return false;

	m_CustomIPSearch.SetIP( dwIP );
	const int idx = m_CustomIPSearch.BinarySearch( m_CustomIPList );
	if( -1 != idx )
	{
		strOutCountryCode	= m_CustomIPList[ idx ].strCountryCode3;
		bIsBlock			= m_CustomIPList[ idx ].bIsBlock;
		strComment			= m_CustomIPList[ idx ].strComment;
	}
	return idx;
}


bool MCountryCodeFilter::CheckIPtoCountryRange( const DWORD dwIPFrom, const DWORD dwIPTo, const IPtoCountryList& icl )
{
	if( CheckIsInverseRange(dwIPFrom, dwIPTo) )
		return false;

	if( !CheckIsLast(dwIPFrom, dwIPTo, icl) )
		return false;

	if( CheckIsDuplicatedRange(dwIPFrom, dwIPTo, icl) )
		return false;
	
	return true;
}


bool MCountryCodeFilter::CheckIsInverseRange( const DWORD dwIPFrom, const DWORD dwIPTo )
{
	return (dwIPFrom > dwIPTo) ;
}


bool MCountryCodeFilter::CheckIsLast( const DWORD dwIPFrom, const DWORD dwIPTo, const IPtoCountryList& icl )
{
	if( !icl.empty() )
	{
		size_t idx = icl.size();
		IPtoCountry itc = icl[ idx - 1 ];
		if( itc.nIPTo >= dwIPFrom )
			return false;
	}

	return true;
}


bool MCountryCodeFilter::CheckIsDuplicatedRange( const DWORD dwIPFrom, const DWORD dwIPTo, const IPtoCountryList& icl )
{
	IPtoCountryList::const_iterator it, end;
	end = icl.end();
	for( it = icl.begin(); it != end; ++it )
	{
		if( it->nIPFrom <= dwIPFrom )
		{
			if( it->nIPFrom <= dwIPFrom && it->nIPTo >= dwIPFrom )
				return true;
			break;
		}

		if( it->nIPTo >= dwIPTo )
		{
			if( it->nIPFrom <= dwIPTo && it->nIPTo >= dwIPTo )
				return true;
			break;
		}
	}

	return false;
}


bool MCountryCodeFilter::Update( const BlockCountryCodeInfoList& rfBlockCountryCodeInfoList, 
								const IPtoCountryList& rfIPtoCountryList )
{
	if( rfBlockCountryCodeInfoList.empty() || 
		rfIPtoCountryList.empty() ) return false;

	// 이전 정보는 리셋하고 새로 리스트를 구성함.
	// 리스트외의 다른 정보는 수정되는것이 없어야 한다.

	InitContryCodeTableList( rfBlockCountryCodeInfoList );
	InitIPtoCountryList( rfIPtoCountryList );
	
	return true;
}


bool MCountryCodeFilter::UpdateCountryCodeTableList( const BlockCountryCodeInfoList& rfBlockCountryCodeInfoList )
{
	BlockCountryCodeInfoList bcil;

	try
	{
		if( rfBlockCountryCodeInfoList.empty() ) return false;

		string strRoutingURL;
		BlockCountryCodeInfoList::const_iterator it, end;

		bcil.clear();
		bcil.reserve( rfBlockCountryCodeInfoList.size() );

		end = rfBlockCountryCodeInfoList.end();
		for( it = rfBlockCountryCodeInfoList.begin(); it != end; ++it )
		{
			if( !IsValidContryCode((*it).strCountryCode, strRoutingURL, bcil) )
				bcil.push_back( (*it) );
			else
			{
				ASSERT( 0 && "중복된 코드." );
				return false;
			}
		}
	}
	catch( ... )
	{
		return false;
	}

	m_BlockCountryCodeInfoList.swap( bcil );
	bcil.clear();
	return true;
}


bool MCountryCodeFilter::UpdateIPtoCountryList( const IPtoCountryList& rfIPtoCountryList )
{
	IPtoCountryList icl;

	try
	{
		if( rfIPtoCountryList.empty() )
			return false;

		IPtoCountryList::const_iterator it, end;

		end = rfIPtoCountryList.end();
		for( it = rfIPtoCountryList.begin(); it != end; ++it )
		{
			if( !CheckIPtoCountryRange(it->nIPFrom, it->nIPTo, icl) )
			{
				ASSERT( 0 && "중복되는 구간이 있음.\n" );
				return false;
			}

			icl.push_back( (*it) );
		}
	}
	catch( ... )
	{
		return false;
	}

	m_IPtoCountryList.swap( icl );
	icl.clear();
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Test코드.


#ifdef _LOCATOR_TEST
void MCountryCodeFilter::DoTest()
{
	TestIP();
}


void MCountryCodeFilter::TestIP()
{
	FILE* fp = fopen( "testip.txt", "r" );
	if( 0 == fp )
	{
		ASSERT( 0 && "테스트용 ip리스트 파일 열기 실패." );
		return;
	}

	char szBuf[ 1024 ];
	char szLine[ 1024 ];
	string strCode;

	vector< string > ip;

	while( 0 != fgets(szBuf, 1023, fp) )
	{
		sscanf( szBuf, "%s", szLine );

		ip.push_back( szLine );
	}

	fclose( fp );

	vector< string >::iterator it, end;
	end = ip.end();
	for( it = ip.begin(); it != end; ++it )
	{
		if( !GetIPCountryCode((*it), strCode) )
		{
			string strBadIP = (*it);
			// ASSERT( 0 );
			mlog("Test fail IP(%s)\n", (*it).c_str() );
		}
	}
	mlog( "\n" );
}


void MCountryCodeFilter::TestAddIPtoCountry( const IPtoCountryList& rfIPtoCountryList )
{
	if( rfIPtoCountryList.empty() )
		return;

	IPtoCountryList::const_iterator it, end;
	end = rfIPtoCountryList.end();
	for( it = rfIPtoCountryList.begin(); it != end; ++it )
	{
		if( !AddIPtoCountry(it->nIPFrom, it->nIPTo, it->strCountryCode3) )
		{
			mlog( "MCountryCodeFilter::TestAddIPtoCountry - IPFrom:%u, IPTo:%u, Code:%s\n", 
				it->nIPFrom, it->nIPTo, it->strCountryCode3.c_str() );

			ASSERT( 0 );
		}
	}
}


bool MCountryCodeFilter::FindEqual( const DWORD dwIPFrom, const DWORD dwIPTo, const string& strCode )
{
	IPtoCountryList::iterator it, end;

	end = m_IPtoCountryList.end();
	for( it = m_IPtoCountryList.begin(); it != end; ++it )
	{
		if( (dwIPFrom == it->nIPFrom ) && (dwIPTo == it->nIPTo) && (strCode == it->strCountryCode3) )
			return true;
	}

	return false;
}
#endif

// Test코드.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
