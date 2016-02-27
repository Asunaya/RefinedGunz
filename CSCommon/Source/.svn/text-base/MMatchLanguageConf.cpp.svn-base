#include <crtdbg.h>

#include "stdafx.h"
#include "MMessenger.h"
#include "MMatchLanguageConf.h"


MMatchLanguageConf::MMatchLanguageConf()
{
}

MMatchLanguageConf::~MMatchLanguageConf()
{
}

void MMatchLanguageConf::MakeCompleteDir()
{
	m_strLangDir = /*m_strRootDir + "/" +*/ string(SERV_LANG_DIR) + "/" + m_strLangType;
}

bool MMatchLanguageConf::CheckSubTaskFile( MZFileSystem* pfs, MMessageNameGroup* pMsgNameGroup )
{
	if( (0 == pMsgNameGroup) || (0 == pMsgNameGroup) )
		return false;

	return true;
}

/////////////////////////////////////////////////////////////

MMatchMessengerManager::MMatchMessengerManager()
{
}

MMatchMessengerManager::~MMatchMessengerManager()
{
}

bool MMatchMessengerManager::AllocateSubTask( const string& strKey, const string& strFile, MZFileSystem* pfs )
{
	if( strKey.empty() || strFile.empty() ){
		_ASSERT( 0 );
		return false;
	}

	return true;
}

void MMatchMessengerManager::ReleaseSubTask()
{
}