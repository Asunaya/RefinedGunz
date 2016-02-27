#pragma once

#include <string>
#include <vector>
#include <deque>
#include <winsock.h>
#include "mysql.h"


#define COLUMN_COUNT 3


using std::string;
using std::vector;
using std::deque;

enum AW_CHECK_STATE
{
	AW_NEW = 0,	// 디비에 처음 등록되어 감사할 단어.
	AW_ADD,		// 금지단어로 사용하기로 함.
	AW_USED,	// 이전에 이미 사용하고 있던 단어.
	AW_DEL,		// 사용하던 단어를 제거하려고 함.
};


struct ABUSE_WORD_INFO
{
	string			m_strKeyAbuseWord;
	string			m_strReplaceWord;
	AW_CHECK_STATE	m_nCheckState;
};


typedef deque< ABUSE_WORD_INFO > AbuseWordDeq;


template< typename T >
class CAbuseWordFinder
{
public :
	CAbuseWordFinder( const string& strKeyAbuseWord ) : m_strKeyAbuseWord( strKeyAbuseWord ) {}
	CAbuseWordFinder( const T& AbuseWordTemp ) : m_strKeyAbuseWord( AbuseWordTemp.m_strKeyAbuseWord ) {}

	bool operator() ( const T& tElement ) const { return (tElement.m_strKeyAbuseWord == m_strKeyAbuseWord); }

private :
	string m_strKeyAbuseWord;
};


template< typename T >
class CUncheckedFinder
{
public :
	bool operator() ( const T& tElement ) const { return (AW_NEW == tElement.m_nCheckState); }
};


class CAbuseList
{
public :
	bool InsertAbuseWord( const ABUSE_WORD_INFO& AbuseWordInfo );
	bool InsertAbuseWord( const string& strKeyAbuseWord, const string& strReplaceWord, const AW_CHECK_STATE nCheckState );

	const ABUSE_WORD_INFO* GetAbuseWordByPos( const unsigned int nPos );
	const ABUSE_WORD_INFO* GetAbuseWordByKeyAbuseWord( const string& strKeyAbuseWord );
	const unsigned int Size() { return static_cast<unsigned int>(m_AbuseWordList.size()); }

	void Check( const string& strKeyAbuseWord, const AW_CHECK_STATE nCheckState );

	void Delete( const string& strKeyAbuseWord );
	void DeleteUncheckedAbuseWord();

	const ABUSE_WORD_INFO* FindFirstUnchecked();

	void Clear() { m_AbuseWordList.clear(); }

private :
	AbuseWordDeq::iterator Find( const string& strKeyAbuseWord );
	AbuseWordDeq::iterator FindFirstUncheckedPos();

private :
	 AbuseWordDeq m_AbuseWordList;
};


class CAbuseWordDB
{
public:
	CAbuseWordDB(void);
	~CAbuseWordDB(void);

	void ClearList() { m_AbuseWordList.Clear(); }

	bool ConnectDB();
	void DisconnectDB();
	bool UpdateDB();
	bool UpdateOneAbuseWordDB( const string& strKeyAbuseWord );
	bool InsertAbuseWordDirectDB( const string& strKeyAbuseWord , const string& strReplaceWord, const AW_CHECK_STATE nCheckState );

	bool GetDBAbuseList();
	int	 GetAbuseListSize() { return m_AbuseWordList.Size(); }
	const ABUSE_WORD_INFO*	GetAbuseWordByPos( const unsigned int nPos );
	const ABUSE_WORD_INFO*	GetAbuseWordByKeyAbuseWord( const string& strKeyAbuseWord );

	const string& GetLastSaveDir() { return m_strLastSaveDir; }

	void Check( const string& strKeyString, const AW_CHECK_STATE nCheckState );
	bool DeleteUnchecked();
	bool InsertAbuseWord( const string& strKeyAbuseWord, const string& strReplaceWord, const AW_CHECK_STATE nCheckState );
	bool DeleteAbuseWord( const string& strKeyAbuseWord );
	void SetReplaceWord( const string& strKeyAbuseWord, const string& strReplaceWord );

	bool SaveLocalFile( const string& strLocalFileName );

	void SetHost( const char* pszHost )			{ m_strHost = pszHost; }
	void SetUser( const char* pszUser )			{ m_strUser = pszUser; }
	void SetPassword( const char* pszPassword ) { m_strPassword = pszPassword; }
	void SetDB( const char* pszDB )				{ m_strDB = pszDB; }

	void SetLastSaveDir( const char* pszLastSaveDir ) { m_strLastSaveDir = pszLastSaveDir; }

private :
	bool Query( const string& strQuery );
	const string MakeLocalFileInfo();

private :
	MYSQL m_MySql;

	string m_strHost;
	string m_strUser;
	string m_strPassword;
	string m_strDB;

	string m_strLastSaveDir;

	bool m_bIsConnected;
	CAbuseList m_AbuseWordList;
};
