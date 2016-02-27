#include <crtdbg.h>

#include "stdafx.h"
#include "MZFileSystem.h"
#include "MMessenger.h"
#include "MLanguageConf.h"

MLanguageCode::MLanguageCode() : m_nLangCode( -1 ), m_isCreated( false )
{
}

MLanguageCode::~MLanguageCode()
{
}

string MLanguageCode::ConvCodeToStrLangType( const int nLangCode )
{
	if( LANGUAGE_CODE::KOR == nLangCode )
		return string( "kor" );

	if( LANGUAGE_CODE::ENG == nLangCode )
		return string( "eng" );

	if( LANGUAGE_CODE::JPN == nLangCode )
		return string( "jpn" );

	if( LANGUAGE_CODE::INTERNATIONAL == nLangCode )
		return string( "international" );
	
	return string( 0 );
}

int	MLanguageCode::ConvStrLangTypeToCode( const char* pszLangType )
{
	if( 0 == strcmp("kor", pszLangType) )
		return LANGUAGE_CODE::KOR;

	if( 0 == strcmp("eng", pszLangType) )
		return LANGUAGE_CODE::ENG;

	if( 0 == strcmp("jpn", pszLangType) )
		return LANGUAGE_CODE::JPN;

	if( 0 == strcmp("international", pszLangType) )
		return LANGUAGE_CODE::INTERNATIONAL;

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////

MMessageNameGroup::MMessageNameGroup()
{
}

MMessageNameGroup::~MMessageNameGroup()
{
}

string MMessageNameGroup::GetErrMessageDir()
{
	MsgGroupIter it = GetGroup().find( CSC_ERR_MESSAGE_TYPE );
	if( GetGroup().end() == it )
		return string( 0 );

	return it->second;
}

////////////////////////////////////////////////////////////////////////////////

MLanguageConf::MLanguageConf() : m_pMsgNameGroup( 0 )
{
}

MLanguageConf::~MLanguageConf()
{
}

bool MLanguageConf::Init( MZFileSystem* pfs, MMessageNameGroup* pMsgNameGroup, const char* pszRootDir )
{
	if( (0 == pszRootDir) || (0 == pMsgNameGroup) ) 
		return false;

	m_strRootDir	= pszRootDir;			// 상대적 최상의 디렉토리.
	// m_strLangType	= pszLangType;
	m_strLangType	= m_LangCode.ConvCodeToStrLangType( m_LangCode.GetLangCode() );

	// 외부에서 설정해주기로 바꿔줌.
	// 언어 타입 로드와 설정 시점 분리.
	// 반드시 SetLangTypeByStr( const char* pszLangType )함수가 먼저 불러져야함.
	// 그렇지 않을경우 정상적인 동작을 보장못함.
	// m_LangCode.SetLangCode( pszLangType );

	MakeCompleteDir();

	if( 0 != m_pMsgNameGroup ){
		delete m_pMsgNameGroup;
		m_pMsgNameGroup = 0;
	}

	m_pMsgNameGroup = pMsgNameGroup;
	
	if( !CheckFile(pfs, m_pMsgNameGroup) ){
		delete m_pMsgNameGroup;
		m_pMsgNameGroup = 0;

		return false;
	}

	return true;
}

bool MLanguageConf::CheckFile( MZFileSystem* pfs, MMessageNameGroup* pMsgNameGroup )
{
	if( (0 == pMsgNameGroup) )
		return false;

	// 서버와 클라이언트가 공통적으로 사용하는 부분은 여기에서 등록시킴.

	MZFile mzf;

	string strErrMessage( m_strLangDir + "/" + CSC_ERR_MESSAGE_FILE_NAME );
	if( mzf.Open(strErrMessage.c_str(), pfs) ){
		mzf.Close();
		pMsgNameGroup->GetGroup().insert( MsgGroupPair(CSC_ERR_MESSAGE_TYPE, strErrMessage) );
	}
	else{
		return false;
	}

	// 상속 받은 클래스 수행.
	if( !CheckSubTaskFile(pfs, pMsgNameGroup) )
		return false;

	return true;
}

void MLanguageConf::Release()
{
	m_strRootDir.clear();
	m_strLangType.clear();
	m_strLangDir.clear();

	if( 0 != m_pMsgNameGroup ){
		m_pMsgNameGroup->Release();
		delete m_pMsgNameGroup;
		m_pMsgNameGroup = 0;
	}
}

bool MLanguageConf::SetLangTypeByStr( const char* pszLangType )
{
	if( 0 == pszLangType )
		return false;

	m_LangCode.SetLangCode( pszLangType );
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////

MMessengerManager::MMessengerManager() : m_pErrMessenger( 0 )
{
}

MMessengerManager::~MMessengerManager()
{
}

bool MMessengerManager::Init( MMessageNameGroup* pMMsgNameGroup, MZFileSystem* pfs )
{
	if( 0 == pMMsgNameGroup )
		return false;

	// 사용하기로 했다면은 적어도 하나는 등록이 되어 있어야 한다.
	// size가 0이면 MMessageNameGroup를 초기화 하는 부분에서 문제가 있는 것이다.
	if( 0 == pMMsgNameGroup->GetGroup().size() )
		return false;
	
	if( !LoadFile(pMMsgNameGroup, pfs) )
		return false;

	return true;
}

bool MMessengerManager::LoadFile( MMessageNameGroup* pMMsgNameGroup, MZFileSystem* pfs )
{
	if( 0 == pMMsgNameGroup )
		return false;

	MsgGroupIter it, end;
	end = pMMsgNameGroup->GetGroup().end();
	for( it = pMMsgNameGroup->GetGroup().begin(); it != end; ++it ){
		if( !Allocate(it->first, it->second, pfs) )
			return false;
	}

	return true;
}

bool MMessengerManager::Allocate( const string& strKey, const string& strFile, MZFileSystem* pfs )
{
	// 클라이언트와 서버가 같이 사용되는 메신저를 등록시키려면 여기서 함.

	if( strKey.empty() || strFile.empty() )
		return false;

	// error mesasge load.
	if( 0 == strcmp(CSC_ERR_MESSAGE_TYPE, strKey.c_str()) ){
		if( 0 != m_pErrMessenger ) 
			delete m_pErrMessenger;

		m_pErrMessenger = new MMessenger;
		if( 0 == m_pErrMessenger )
			return false;

		if( !m_pErrMessenger->InitializeMessage(strFile.c_str(), pfs) ){
			delete m_pErrMessenger;
			m_pErrMessenger = 0;
			return false;
		}

		return true;
	}

	// 상속한 클래스 수행.
	if( !AllocateSubTask(strKey, strFile, pfs) )
		return false;

	return true;
}

void MMessengerManager::Release()
{
	if( 0 != m_pErrMessenger ){
		m_pErrMessenger->Release();
		delete m_pErrMessenger;
		m_pErrMessenger = 0;
	}

	ReleaseSubTask();
}
