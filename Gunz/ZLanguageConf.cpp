#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <crtdbg.h>
#include "MXml.h"
#include "MZFileSystem.h"
//#include "MMessenger.h"
#include "ZLanguageConf.h"
#include "ZFilePath.h"

/*
ZMessageNameGroup::ZMessageNameGroup()
{
}

ZMessageNameGroup::~ZMessageNameGroup()
{
}

const string ZMessageNameGroup::GetMessageDir()
{
	map< string, string >::iterator it = m_mMsgGroup.find( CLI_ERRMESSAGE_TYPE );
	if( m_mMsgGroup.end() == it ) 
		return string( 0 );

	return it->second;
}

///////////////////////////////////////////////////////////////

ZLanguageConf::ZLanguageConf()
{
}

ZLanguageConf::~ZLanguageConf()
{
}


void ZLanguageConf::MakeCompleteDir()
{
	m_strLangDir =  string(CLI_LANG_DIR) + "/" + m_strLangType;
}


bool ZLanguageConf::CheckSubTaskFile( MZFileSystem* pfs, MMessageNameGroup* pMsgNameGroup )
{
	MZFile mzf;

	string strMessage( m_strLangDir + "/" + CLI_MESSAGE_FILE_NAME );
	if( mzf.Open(strMessage.c_str(), pfs) ){
		mzf.Close();
		pMsgNameGroup->GetGroup().insert( MsgGroupPair(CLI_MESSAGE_TYPE, strMessage) );
	}
	else{
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

ZMessengerManager::ZMessengerManager() : m_pMessenger( 0 )
{
}

ZMessengerManager::~ZMessengerManager()
{
}

bool ZMessengerManager::AllocateSubTask( const string& strKey, const string& strFile, MZFileSystem* pfs )
{
	if( strKey.empty() || strFile.empty() ) 
		return false;

	if( 0 == strcmp(CLI_MESSAGE_TYPE, strKey.c_str()) ){
		m_pMessenger = new MMessenger;
		if( 0 == m_pMessenger ) 
			return false;

		if( !m_pMessenger->InitializeMessage(strFile.c_str(), pfs) ){
			delete m_pMessenger;
			m_pMessenger = 0;
			return false;
		}

		return true;
	}

	return true;
}

void ZMessengerManager::ReleaseSubTask()
{
	if( 0 != m_pMessenger ){
		m_pMessenger->Release();
		delete m_pMessenger;
		m_pMessenger = 0;
	}
}
*/