#include "StdAfx.h"
#include ".\abuseworddb.h"
#include <algorithm>

#pragma comment( lib, "libmysql.lib" )




bool CAbuseList::InsertAbuseWord( const ABUSE_WORD_INFO& AbuseWordInfo )
{
	return InsertAbuseWord( AbuseWordInfo.m_strKeyAbuseWord, 
							AbuseWordInfo.m_strReplaceWord, 
							AbuseWordInfo.m_nCheckState );
}


bool CAbuseList::InsertAbuseWord( const string& strKeyAbuseWord, const string& strReplaceWord, const AW_CHECK_STATE nCheckState )
{
	// 리스트에 중복되는 단어가 없어야 함.
	const ABUSE_WORD_INFO* pDupAbuseWord = GetAbuseWordByKeyAbuseWord( strKeyAbuseWord );

	ASSERT( (0 == pDupAbuseWord) && "CAbuseList::InsertAbuseWord -> 리스트에 중족된 단어가 있으면 않됨." );

	if( 0 == pDupAbuseWord )
	{
		ABUSE_WORD_INFO AbuseWordInfo;

		AbuseWordInfo.m_strKeyAbuseWord = strKeyAbuseWord;
		AbuseWordInfo.m_strReplaceWord	= strReplaceWord;
		AbuseWordInfo.m_nCheckState		= nCheckState;

		if( AW_USED == nCheckState )
			m_AbuseWordList.push_back( AbuseWordInfo );
		else
			m_AbuseWordList.push_front( AbuseWordInfo );

		return true;
	}

	return false;
}


AbuseWordDeq::iterator CAbuseList::Find( const string& strKeyAbuseWord )
{
	if( strKeyAbuseWord.empty() || m_AbuseWordList.empty() )
		return m_AbuseWordList.end();

	return find_if( m_AbuseWordList.begin(), 
					m_AbuseWordList.end(), 
					CAbuseWordFinder<ABUSE_WORD_INFO>(strKeyAbuseWord) );
}

AbuseWordDeq::iterator CAbuseList::FindFirstUncheckedPos()
{
	if( m_AbuseWordList.empty() )
		return m_AbuseWordList.end();

	return find_if( m_AbuseWordList.begin(), 
					m_AbuseWordList.end(), 
					CUncheckedFinder<ABUSE_WORD_INFO>() );
}

const ABUSE_WORD_INFO* CAbuseList::FindFirstUnchecked()
{
	AbuseWordDeq::iterator itFind = FindFirstUncheckedPos();
	if( m_AbuseWordList.end() != itFind )
		return &(*itFind);
	
	return 0;
}	


const ABUSE_WORD_INFO* CAbuseList::GetAbuseWordByPos( const unsigned int nPos )
{
	if( nPos < m_AbuseWordList.size() )
		return &m_AbuseWordList[ nPos ];

	return 0;
}


const ABUSE_WORD_INFO* CAbuseList::GetAbuseWordByKeyAbuseWord( const string& strKeyAbuseWord )
{
	AbuseWordDeq::iterator itFind = Find( strKeyAbuseWord );

	if( m_AbuseWordList.end() != itFind )
		return &(*itFind);
	
	return 0;
}


void CAbuseList::Check( const string& strKeyAbuseWord, const AW_CHECK_STATE nCheckState )
{
	if( !m_AbuseWordList.empty() )
	{
		AbuseWordDeq::iterator itFind = Find( strKeyAbuseWord );

		if( m_AbuseWordList.end() != itFind )
			(*itFind).m_nCheckState = nCheckState;
	}
}


void CAbuseList::Delete( const string& strKeyAbuseWord )
{
	AbuseWordDeq::iterator itFind = Find( strKeyAbuseWord );
	if( m_AbuseWordList.end() != itFind )
		m_AbuseWordList.erase( itFind );
}


void CAbuseList::DeleteUncheckedAbuseWord()
{
	AbuseWordDeq::iterator itFind;
	for( itFind = FindFirstUncheckedPos(); 
		 m_AbuseWordList.end() != itFind; 
		 itFind = FindFirstUncheckedPos() )
	{
		m_AbuseWordList.erase( itFind );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CAbuseWordDB::CAbuseWordDB(void) : m_bIsConnected( false )
{
	mysql_init( &m_MySql );
}

CAbuseWordDB::~CAbuseWordDB(void)
{
}


bool CAbuseWordDB::ConnectDB()
{
	if( !m_bIsConnected )
	{
		if( !mysql_real_connect(&m_MySql, 
								m_strHost.c_str(), 
								m_strUser.c_str(), 
								m_strPassword.c_str(), 
								m_strDB.c_str(), 
								0, 
								(char*)NULL, 
								0) )
		{
#ifdef _DEBUG
			const char* pszError = mysql_error( &m_MySql );
			OutputDebugString( pszError );
#endif
			return false;
		}
		
		m_bIsConnected = true;
	}
	
	return m_bIsConnected;
}


void CAbuseWordDB::DisconnectDB()
{
	if( m_bIsConnected )
	{
		mysql_close( &m_MySql );

		m_bIsConnected = false;
	}
}


bool CAbuseWordDB::GetDBAbuseList()
{
	if( m_bIsConnected )
	{
		string strQuery = "SELECT * FROM abuse_word";
		if( Query(strQuery) )
		{
			MYSQL_RES* pRes;

			pRes = mysql_store_result( &m_MySql );
			if( 0 != pRes )
			{
				MYSQL_ROW Row;
				
				while( (Row = mysql_fetch_row(pRes)) )
				{
					const int nFieldNum = static_cast<int>( mysql_num_fields(pRes) );
					ASSERT( COLUMN_COUNT == nFieldNum );

					m_AbuseWordList.InsertAbuseWord( Row[0], Row[1], ((1 == atoi(Row[2])) ? AW_USED : AW_NEW) );
				}
			}
		}
	}

	return false;
}


bool CAbuseWordDB::UpdateDB()
{
	// 업데이트는 CheckState가 AW_ADD인것만 새로 디비에 저장을 함.
	AW_CHECK_STATE nCheckState;
	const int nAbuseListCount = m_AbuseWordList.Size();
	for( int i = 0; i < nAbuseListCount; ++i )
	{
		const ABUSE_WORD_INFO* pAbuseWordInfo = m_AbuseWordList.GetAbuseWordByPos( i );

		ASSERT( 0 != pAbuseWordInfo );

		nCheckState = pAbuseWordInfo->m_nCheckState;

		if( (0 != pAbuseWordInfo) && (AW_USED != nCheckState) && (AW_NEW != nCheckState) )
			if( !UpdateOneAbuseWordDB(pAbuseWordInfo->m_strKeyAbuseWord) )
				return false;
	}

	return true;
}


bool CAbuseWordDB::UpdateOneAbuseWordDB( const string& strKeyAbuseWord )
{
	if( m_bIsConnected && !strKeyAbuseWord.empty() )
	{
		const ABUSE_WORD_INFO* pAbuseWordInfo = m_AbuseWordList.GetAbuseWordByKeyAbuseWord( strKeyAbuseWord );
		if( 0 != pAbuseWordInfo ) 
		{
			const AW_CHECK_STATE nCheckState = pAbuseWordInfo->m_nCheckState;

			if( AW_ADD == nCheckState )
			{
				const string strQuery( "UPDATE abuse_word SET Checked = 1 WHERE AbuseWord LIKE BINARY('" + strKeyAbuseWord + "')" );
				if( !Query(strQuery) )
					return false;

				return true;
			}
			else if( AW_DEL == nCheckState )
			{
				const string strQuery( "UPDATE abuse_word SET Checked = 0 WHERE AbuseWord LIKE BINARY('" + strKeyAbuseWord + "')" );
				if( !Query(strQuery) )
				{
					ASSERT( "CAbuseWordDB::UpdateOneAbuseWordDB -> 'Checked = 0'쿼리 실패." );
					return false;
				}

				return true;
			}
			else
			{
				ASSERT( 0 && "정의되지 않음" );
			}
		}
	}

	return false;
}


void CAbuseWordDB::Check( const string& strKeyString, const AW_CHECK_STATE nCheckState )
{
	if( !strKeyString.empty() || (0 != m_AbuseWordList.Size()) )
		m_AbuseWordList.Check( strKeyString, nCheckState );
}


bool CAbuseWordDB::DeleteUnchecked()
{
	// 디비와 같이 제거가 되어야 함.

	const unsigned int nAbuseWordCount = m_AbuseWordList.Size();
	for( unsigned int i = 0; i < nAbuseWordCount; ++i )
	{
		const ABUSE_WORD_INFO* pAbuseWordInfo = m_AbuseWordList.GetAbuseWordByPos( i );
		if( 0 != pAbuseWordInfo )
		{
			const AW_CHECK_STATE nCheckState = pAbuseWordInfo->m_nCheckState;
			if( (AW_NEW == nCheckState) || (AW_DEL == nCheckState) )
			{
				if( ConnectDB() )
				{
					if( DeleteAbuseWord(pAbuseWordInfo->m_strKeyAbuseWord) )
					{
						// 반복문 중간에 리스트 삭제가 이루어지면 Index가 유효한지 보장을 할수 없음.
						// 루프가 다 끝나고 일관적으로 리스트에서 Checked가 AW_DEL인 모든 개체를 제거함.
					}
					else 
					{
						DisconnectDB();
						return false;
					}
				}
				else
					return false;
				DisconnectDB();
			}
		}
		else
		{
			ASSERT( (0 != pAbuseWordInfo) && "잘못된 인덱스 참조." );
			return false;
		}
	}

	// 여기서 Checked가 AW_DEL인 개체를 모두 제거함.
	m_AbuseWordList.DeleteUncheckedAbuseWord();

	return true;
}


bool CAbuseWordDB::InsertAbuseWord( const string& strKeyAbuseWord, const string& strReplaceWord, const AW_CHECK_STATE nCheckState )
{
	if( m_AbuseWordList.InsertAbuseWord(strKeyAbuseWord, strReplaceWord, nCheckState) )
		return true;
	return false;
}


bool CAbuseWordDB::DeleteAbuseWord( const string& strKeyAbuseWord )
{
	const string strQuery( "DELETE FROM abuse_word  WHERE AbuseWord LIKE BINARY('" + strKeyAbuseWord + "')" );
	if( Query(strQuery) )
		return true;

	return false;
}


void CAbuseWordDB::SetReplaceWord( const string& strKeyAbuseWord, const string& strReplaceWord )
{
	ASSERT( 0 && "현제 지원하는 기능이 아님." );

	// 추후에 사용을 하게된다면 기능 추가함.
	// 현제는 사용할지 확정되지 않아서 인터페이스만 만들어 놓음.
}


const ABUSE_WORD_INFO* CAbuseWordDB::GetAbuseWordByPos( const unsigned int nPos )
{
	ASSERT( (nPos < m_AbuseWordList.Size()) && "CAbuseWordDB::GetAbuseWordByPos -> 찾는 위치가 유효범위 밖임." );

	if( nPos < static_cast<unsigned int>(m_AbuseWordList.Size()) )
	{
		const ABUSE_WORD_INFO* pAbuseWordInfo = m_AbuseWordList.GetAbuseWordByPos( nPos );
		if( 0 != pAbuseWordInfo )
			return pAbuseWordInfo;
	}

	return 0;
}


const ABUSE_WORD_INFO* CAbuseWordDB::GetAbuseWordByKeyAbuseWord( const string& strKeyAbuseWord )
{
	ASSERT( !strKeyAbuseWord.empty() && "CAbuseWordDB::GetAbuseWordByKeyAbuseWord -> 문자 길이가 0되면 않됨." );

	if( !strKeyAbuseWord.empty() )
	{
		const ABUSE_WORD_INFO* pAbuseWordInfo = m_AbuseWordList.GetAbuseWordByKeyAbuseWord( strKeyAbuseWord );
		if( 0 != pAbuseWordInfo )
			return pAbuseWordInfo;
	}

	return 0;
}


bool CAbuseWordDB::Query( const string& strQuery )
{
	if( 0 == mysql_real_query(&m_MySql, strQuery.c_str(), static_cast<unsigned long>(strQuery.length())) )
		return true;
	
	return false;
}


bool CAbuseWordDB::InsertAbuseWordDirectDB( const string& strKeyAbuseWord , const string& strReplaceWord, const AW_CHECK_STATE nCheckState )
{
	if( !strKeyAbuseWord.empty() )
	{
		const string strQuery( "INSERT INTO abuse_word( AbuseWord, ReplaceWord, Checked ) VALUES ('" +
			strKeyAbuseWord + "', '" + strReplaceWord + "', 0)" );
		if( Query(strQuery) )
		{
			if( InsertAbuseWord(strKeyAbuseWord, string("NULL"), AW_NEW) )
				return true;
		}
	}

	return false;
}


bool CAbuseWordDB::SaveLocalFile( const string& strLocalFileName )
{
	if( !strLocalFileName.empty() )
	{
		// 저장할 리스트 정보 수집.
		const string strBuf = MakeLocalFileInfo();
		if( !strBuf.empty() )
		{
			FILE* fp = fopen( strLocalFileName.c_str(), "w" );
			if( 0 != fp )
			{
				try
				{
					fprintf( fp, "%s", strBuf.c_str() );
				}
				catch( ... )
				{
					if( 0 != fp )
						fclose( fp );

					ASSERT( 0 && "로컬파일에 저장에서 Exception발생" );

					return false;
				}
			}
			fclose( fp );
		}
	}

	return true;
}


const string CAbuseWordDB::MakeLocalFileInfo()
{
	SYSTEMTIME SystemTime;
	GetSystemTime( &SystemTime );

	char szDate[ 1024 ];
	_snprintf( szDate, 1023, "%d-%d-%d %d:%d:%d", 
		SystemTime.wYear,
		SystemTime.wMonth,
		SystemTime.wDay,
		SystemTime.wHour + 9,
		SystemTime.wMinute,
		SystemTime.wSecond );
	
	string strBuf( "ABUSE_BEGIN\t" + string(szDate) + "\n" );

	const unsigned int nAbuseListCount = m_AbuseWordList.Size();
	for( unsigned int i = 0; i < nAbuseListCount; ++i )
	{
		const ABUSE_WORD_INFO* pAbuseWordInfo = m_AbuseWordList.GetAbuseWordByPos( i );
		if( 0 != pAbuseWordInfo )
		{
			strBuf += pAbuseWordInfo->m_strKeyAbuseWord + " " + 
					  pAbuseWordInfo->m_strReplaceWord + "\n";
		}
		else
		{
			ASSERT( (0 != pAbuseWordInfo) && ("잘못된 포인터.") );
		}
	}

	strBuf += string( "END" );

	return strBuf;
}