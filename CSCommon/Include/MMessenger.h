#pragma once

#include <map>
#include <string>
using std::map;
using std::string;
using std::pair;

class MZFileSystem;

/// 메세지를 담고 있는 기본 클래스
class MMessenger
{
public :
	MMessenger( const char* pszTOK_MESSAGE = "MESSAGE", const char* pszTOK_CID = "cid" );
	virtual ~MMessenger();

	typedef map< int, string >	MsgMap;
	typedef MsgMap::iterator	MsgIter;
	typedef pair< int, string >	MsgPair;

	bool	InitializeMessage( const char* pszFileName, MZFileSystem *pfs = 0 );

	bool TranslateMessage( char* poutStr, const int nStringCode, const int argnum = 0, const char* arg1 = NULL, ... );
	const char* GetStr( const int nStringCode );

	void Release();

private :
	string m_strTOK_MESSAGE;
	string m_strTOK_CID;
	MsgMap m_MessageMap;
};