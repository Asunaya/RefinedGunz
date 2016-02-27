#include "StdAfx.h"
#include ".\mcountryfilter.h"

#include <algorithm>

bool SplitStrIP( const string& strIP, vector<BYTE>& vIP )
{
	if( strIP.empty() ) 
		return false;

	size_t a, b, c;
	char szPos1[ 4 ] = {0,};
	char szPos2[ 4 ] = {0,};
	char szPos3[ 4 ] = {0,};
	char szPos4[ 4 ] = {0,};

	a = strIP.find( "." );
	if( string::npos == a ) 
		return false;

	b = strIP.find( ".", a + 1 );
	if( string::npos == b ) 
		return false;

	c = strIP.find( ".", b + 1 );
	if( string::npos == c )
		return false;

	strncpy( szPos1, &strIP[0], a );
	strncpy( szPos2, &strIP[a + 1], b - a - 1 );
	strncpy( szPos3, &strIP[b + 1], c - b - 1 );
	strncpy( szPos4, &strIP[c + 1], strIP.length() - c - 1 );

	vIP.push_back( static_cast<BYTE>(atoi(szPos1)) );
	vIP.push_back( static_cast<BYTE>(atoi(szPos2)) );
	vIP.push_back( static_cast<BYTE>(atoi(szPos3)) );
	vIP.push_back( static_cast<BYTE>(atoi(szPos4)) );

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< typename T >
IPRangeAlgorithm<T>::IPRangeAlgorithm()
{
#ifdef _FILTER_TEST
	m_dwTraceCount = 0;
	m_dwMaxTraceCount = 10;
#endif
}


#ifdef _FILTER_TEST
struct MdlTrace 
{
	size_t head;
	size_t middle;
	size_t tail;
};
#endif


/*
 BinarySearch를 사용하기 위해서는 해당 컨테이너가 반드시 정렬이 되어 있어야 하며,
 검색하려는 IP가 SetIP( IP )로 먼서 설정되어야 한다. 
 ex).
  vector v;
  IPRangeAlgorithm< vector > ra;

  ra.SetIP( IP );
  const idx = rs.BinarySearch( v );
  if( -1 != idx ) v[ idx ];
  else return false;
 */
template< typename T >
const int IPRangeAlgorithm<T>::BinarySearch( const DWORD dwIP, const T& tVector ) const
{
	if( 2 > tVector.size() )
	{
		if( tVector.empty() ) 
			return -1;

		if( 1 == tVector.size() )
		{
			if( (tVector[0]->m_dwIPFrom <= dwIP) && (tVector[0]->m_dwIPTo >= dwIP) )
				return 0;
			else
				return -1;
		}
	}

	size_t nHead = 0;
	// [n]을 검사하기 위해서는 [n + 1]이 존재하여야 하기에, 마지막을 안전하게 검사하기 우해서.
	size_t nTail = tVector.size() - 2; 
	size_t nMiddle = nTail / 2;

	DWORD dwIPFrom;
	DWORD dwIPTo;

#ifdef _FILTER_TEST
	MdlTrace mt;
	vector< MdlTrace > vMdlTrace;
#endif

	while( true ){
		dwIPFrom = tVector[ nMiddle ]->m_dwIPFrom;
		dwIPTo   = tVector[ nMiddle ]->m_dwIPTo;

#ifdef _FILTER_TEST
		mt.head = nHead;
		mt.middle = nMiddle;
		mt.tail = nTail;
		vMdlTrace.push_back( mt );
#endif

		if( (nHead < nTail) && (0 != nMiddle) )
		{
			if( (dwIPFrom <= dwIP) && (tVector[nMiddle + 1]->m_dwIPFrom > dwIP) )
			{
				if( dwIPTo >= dwIP ) 
					return static_cast<int>(nMiddle);
			}

			if( dwIPFrom < dwIP )
			{
				nHead = nMiddle + 1;
			}
			else if( dwIPFrom > dwIP )
			{
				nTail = nMiddle - 1;
			}

			nMiddle = (nHead + nTail) / 2;
		}
		else{
			if( (dwIPFrom <= dwIP) && (dwIPTo >= dwIP) )
				return static_cast<int>(nMiddle);
			++nMiddle;
			if( (tVector[nMiddle]->m_dwIPFrom <= dwIP) && (tVector[nMiddle]->m_dwIPTo >= dwIP) )
				return static_cast<int>(nMiddle);

#ifdef _FILTER_TEST
			if( m_dwMaxTraceCount > m_dwTraceCount )
			{
				char szBuf[ 1024 ];
				DWORD i = 0;
                T::const_iterator it, end;
				end = tVector.end();
				for( it = tVector.begin(); it != end; ++it, ++i )
				{
					if( it->dwIPTo >= m_dwIP )
					{
						if( it->dwIPFrom <= m_dwIP )
						{
							memset( szBuf, 0, 1024 );
							sprintf( szBuf, "================== CountryCode miss hit trace ====================\n" );
							OutputDebugString( szBuf );
							memset( szBuf, 0, 1024 );
							sprintf( szBuf, "Index:%u, IP:%u\n", i, m_dwIP );
							OutputDebugString( szBuf );
							vector< MdlTrace >::iterator it2, end2;
							it2 = vMdlTrace.begin();
							end2 = vMdlTrace.end();
							for( ; it2 != end2; ++it2 )
							{
								memset( szBuf, 0, 1024 );
								sprintf( szBuf, "H:%u, M:%u, T:%u\n", it2->head, it2->middle, it2->tail );
								OutputDebugString( szBuf );
							}
							memset( szBuf, 0, 1024 );
							sprintf( szBuf, "\n==================================================================\n" );
							OutputDebugString( szBuf );
							++m_dwTraceCount;
						}
					}
				}
				memset( szBuf, 0, 1024 );
				sprintf( szBuf, "Can't find : %u\n", m_dwIP );
				OutputDebugString( szBuf );
			}
#endif
			return -1;
		}
	}
}


template< typename T >
const int IPRangeAlgorithm<T>::Search( const DWORD dwIP, const T& tVector ) const
{
	int i = 0;
	T::const_iterator it, end;
	end = tVector.end();
	for( it = tVector.begin(); it != end; ++it )
	{
		if( ((*it)->m_dwIPFrom <= dwIP) && ((*it)->m_dwIPTo >= dwIP) )
			return i;
		++i;
	}
	return -1;
}


template< typename T >
const bool IPRangeAlgorithm<T>::CheckIsDuplicateRange( const DWORD dwIPFrom, const DWORD dwIPTo, const T& tVector ) const
{
	if( -1 != BinarySearch(dwIPFrom, tVector) )
		return true;
	
	if( -1 != BinarySearch(dwIPTo, tVector) )
		return true;

	return false;
}

template< typename T >
const bool IPRangeAlgorithm<T>::IsLast( const DWORD dwIPFrom, const T& tVector ) const
{
	if( tVector.empty() ) return true;
	return tVector[ tVector.size() - 1 ]->m_dwIPTo < dwIPFrom;
}


template< typename T >
void IPRangeAlgorithm<T>::SortIncrease( T& tVector )
{
	sort( tVector.begin(), tVector.end(), SortCmp() );
}

template< typename T >
const int IPRangeAlgorithm<T>::MakeEmptyBlock( const DWORD dwInsertIP, T& tVector )
{
	const int size = static_cast< int >( tVector.size() );
	int i;

	for( i = 0; i < size; ++i )
	{
		if( dwInsertIP < tVector[i]->m_dwIPFrom )
			break;
	}

	if( size == i ) 
		return -1;

	T tmpVector;
	tmpVector.reserve( size + 1 );
	tmpVector.insert( tmpVector.begin(), &tVector[0], &tVector[i] );
	tmpVector.push_back( 0 );
	tmpVector.insert( tmpVector.end(), &tVector[i], &tVector[size] );
	tVector.swap( tmpVector );

	// 비어있는 인덱스를 외부에서 알아야 한다.
	return i;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MCountryFilter::MCountryFilter(void)
{
}

MCountryFilter::~MCountryFilter(void)
{
	ReleaseCountryCode();
	ReleaseBlockCountryCode();
	ReleaseCustomIP();
}


bool MCountryFilter::Create( BlockCountryCodeList& rfBlockCountryCodeList, 
							 IPtoCountryList& rfIPtoCountryList, 
							 CustomIPList& rfCustomIPList  )
{
	if( !InitBlockCountryCodeList(rfBlockCountryCodeList) ) 
		return false;

	if( !InitIPtoCountryList(rfIPtoCountryList) )
		return false;

	if( !InitCustomIPList(rfCustomIPList) )
		return false;

	return true;
}


bool MCountryFilter::AddIPtoCountry( const DWORD dwIPFrom, const DWORD dwIPTo, const string& strCode )
{
	if( m_IPtoCountryAlgorithm.IsInverse(dwIPFrom, dwIPTo) )
		return false;
	if( m_IPtoCountryAlgorithm.CheckIsDuplicateRange(dwIPFrom, dwIPTo, m_IPtoCountryList) )
		return false;

	if( m_IPtoCountryAlgorithm.IsLast(dwIPFrom, m_IPtoCountryList) )
	{
		m_IPtoCountryList.push_back( new IPtoCountry(dwIPFrom, dwIPTo, strCode) );
	}
	else
	{
		// m_IPtoCountryList.push_back( new IPtoCountry(dwIPFrom, dwIPTo, strCode) );
		// m_IPtoCountryAlgorithm.SortIncrease( m_IPtoCountryList );

		int idx = m_IPtoCountryAlgorithm.MakeEmptyBlock( dwIPFrom, m_IPtoCountryList );		
		if( -1 == idx )
			return false;
		IPtoCountry* pIPtoCountry = new IPtoCountry( dwIPFrom, dwIPTo, strCode );
		if( 0 == pIPtoCountry )
			return false;

		m_IPtoCountryList[ idx ] = pIPtoCountry;
	}

	return true;
}


// 비정상 IP만 따로 로그를 남기기 위해서.
bool MCountryFilter::AddInvalidIP( const DWORD dwIP, const string& strIP )
{
	pair< InvalidIPList::iterator, bool > pr;
	pr = m_InvalidIPList.insert( InvalidIPList::value_type(dwIP, strIP) );
	
	return pr.second;
}


bool MCountryFilter::AddCustomIP( const DWORD dwIPFrom, const DWORD dwIPTo, const bool bIsBlock, const string& strCountryCode3, const string& strComment )
{
	if( m_CustomIPAlgorithm.IsInverse(dwIPFrom, dwIPTo) )
		return false;
	if( m_CustomIPAlgorithm.CheckIsDuplicateRange(dwIPFrom, dwIPTo, m_CustomIPList) )
		return false;

	if( !m_CustomIPAlgorithm.IsLast(dwIPFrom, m_CustomIPList) )
		m_CustomIPList.push_back( new CustomIP(dwIPFrom, dwIPTo, bIsBlock, strCountryCode3, strComment) );
	else
	{
		m_CustomIPList.push_back( new CustomIP(dwIPFrom, dwIPTo, bIsBlock, strCountryCode3, strComment) );
		m_CustomIPAlgorithm.SortIncrease( m_CustomIPList );
	}
	return true;
}


bool MCountryFilter::InitBlockCountryCodeList( BlockCountryCodeList& rfBlockCountryCodeList )
{
	try
	{
		ReleaseBlockCountryCode();

		m_BlockCountryCodeList.swap( rfBlockCountryCodeList );
	}
	catch( ... )
	{
		return false;
	}

	return true;
}


bool MCountryFilter::InitCustomIPList( CustomIPList& rfCustomIPList )
{
	try
	{
		CustomIPList::const_iterator it, end;
		end = rfCustomIPList.end();
		for( it = rfCustomIPList.begin(); it != end; ++it )
		{
			if( m_CustomIPAlgorithm.IsInverse((*it)->m_dwIPFrom, (*it)->m_dwIPTo) )
				return false;
		}

		ReleaseCustomIP();

		m_CustomIPList.swap( rfCustomIPList );
	}
	catch( ... )
	{
		return false;
	}

	return true;
}


/*
 * 추가를 하기 위해서는 반드시 오름차순 정렬을 해서 넣어줘야함.
 * 그렇지 않으면 이진검색을 할수가 없음.
 */
bool MCountryFilter::InitIPtoCountryList( IPtoCountryList& rfIPtoCountryList )
{
	try
	{
		IPtoCountryList::const_iterator it, end;

		end = rfIPtoCountryList.end();
		for( it = rfIPtoCountryList.begin(); it != end; ++it )
		{
			// rfIPtoCountryList는 DB에서 오름차순으로 정렬해서 가져온다.
			if( m_IPtoCountryAlgorithm.IsInverse((*it)->m_dwIPFrom, (*it)->m_dwIPTo) )
			{
				ASSERT( 0 && "정상적인 범위가 아님." );
				return false;
			}
		}

		ReleaseCountryCode();
        
		m_IPtoCountryList.swap( rfIPtoCountryList );
	}
	catch( ... )
	{
		return false;
	}

	return true;
}


bool MCountryFilter::IsNotBlockCode( const string& strCountryCode, string& strRoutingURL, BlockCountryCodeList& bcil )
{
	if( 3 != strCountryCode.length() )
		return false;

	BlockCountryCodeList::iterator itFind = find_if( bcil.begin(), 
													 bcil.end(), 
													 StrICmp(strCountryCode) );

	if( bcil.end() != itFind )
	{
		if( (*itFind)->m_bIsBlock )
		{
			strRoutingURL = (*itFind)->m_strRoutingURL;
			return false;
		}
	}
	else
		return false;
	
	return true;
}


bool MCountryFilter::IsNotBlockCode( const string& strCountryCode, string& strRoutingURL )
{
	return IsNotBlockCode( strCountryCode, strRoutingURL, m_BlockCountryCodeList );
}


const DWORD MCountryFilter::inet_aton( const string& strIP )
{
	if( strIP.empty() ) 
		return false;

	vector< BYTE > vIP;
	if( !SplitStrIP(strIP, vIP) ) 
		return false;
	if( 4 != vIP.size() ) 
		return false;

	return (vIP[0] * 16777216) + (vIP[1] * 65536) + (vIP[2] * 256) + vIP[3];
}


bool MCountryFilter::GetIPCountryCode( const string& strIP, string& strCountryCode )
{
	const DWORD dwIP = inet_aton( strIP );
	if( 0 == dwIP )
		return false;

	const int nIdx = m_IPtoCountryAlgorithm.BinarySearch( dwIP, m_IPtoCountryList );
	if( -1 != nIdx )
		strCountryCode = m_IPtoCountryList[ nIdx ]->m_strCountryCode3;
	return (-1 != nIdx);
}

bool MCountryFilter::Update( BlockCountryCodeList& rfBlockCountryCodeList, 
							 IPtoCountryList& rfIPtoCountryList,
							 CustomIPList& rfCustomIPList )
{
	if( rfBlockCountryCodeList.empty() || 
		rfIPtoCountryList.empty() ) return false;

	// 이전 정보는 리셋하고 새로 리스트를 구성함.
	// 리스트외의 다른 정보는 수정되는것이 없어야 한다.

	if( !InitBlockCountryCodeList(rfBlockCountryCodeList) )
		return false;
	if( !InitIPtoCountryList(rfIPtoCountryList) )
		return false;
	if( !InitCustomIPList(rfCustomIPList) )
		return false;
	
	return true;
}


bool MCountryFilter::GetCustomIP( const string& strIP, bool& bIsBlock, string& strCountryCode3, string& strComment )
{
	const DWORD dwIP = inet_aton( strIP );
	if( 0 == dwIP )
		return false;
	
	const int nIdx = m_CustomIPAlgorithm.BinarySearch( dwIP, m_CustomIPList );
	if( -1 != nIdx )
	{
		bIsBlock		= m_CustomIPList[ nIdx ]->m_bIsBlock;
		strCountryCode3 = m_CustomIPList[ nIdx ]->m_strCountryCode3;
		strComment		= m_CustomIPList[ nIdx ]->m_strComment;
	}
	return (-1 != nIdx);
}


void MCountryFilter::ReleaseCountryCode()
{
	if( m_IPtoCountryList.empty() ) 
		return;

	IPtoCountryList::iterator it, end;
	end = m_IPtoCountryList.end();
	for( it = m_IPtoCountryList.begin(); it != end; ++it )
		delete (*it);
	m_IPtoCountryList.clear();
	
}


void MCountryFilter::ReleaseBlockCountryCode()
{
	if( m_BlockCountryCodeList.empty() )
		return;

	BlockCountryCodeList::iterator it, end;	
	end = m_BlockCountryCodeList.end();
	for( it = m_BlockCountryCodeList.begin(); it != end; ++it )
		delete (*it);
	m_BlockCountryCodeList.clear();
}


void MCountryFilter::ReleaseCustomIP()
{
	if( m_CustomIPList.empty() )
		return;

	CustomIPList::iterator it, end;
	end = m_CustomIPList.end();
	for( it = m_CustomIPList.begin(); it != end; ++it )
		delete (*it);
	m_CustomIPList.clear();
}