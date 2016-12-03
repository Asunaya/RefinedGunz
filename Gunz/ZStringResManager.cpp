#include "stdafx.h"
#include "ZStringResManager.h"
#include "ZConfiguration.h"

void ZStringResManager::MakeInstance()
{
	_ASSERT(m_pInstance == NULL);
	m_pInstance = new ZStringResManager();
}

ZStringResManager::ZStringResManager() = default;
ZStringResManager::~ZStringResManager() = default;

bool ZStringResManager::OnInit()
{
	string strFileName = m_strPath + FILENAME_MESSAGES;

	if (ZGetConfiguration()->IsComplete())
	{
		if (!m_Messages.Initialize(strFileName.c_str(), 0, m_pFS))
		{
			_ASSERT(0);
			mlog("Error!! - Messages Initalize Failed\n");
			return false;
		}
	}
	else
	{
		ASSERT( 0 );
		mlog( "'Configuration' or 'ZLocale' must be completed befor 'Message'.\n" );
		return false;
	}

	return true;
}

const char* ZStringResManager::GetMessageStr(int nID)
{
	return m_Messages.GetStr(nID);
}