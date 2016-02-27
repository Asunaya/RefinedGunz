#include "stdafx.h"
#include "MBaseStringResManager.h"

#define TOK_XML_STRING_HEADER		"STR:"


MBaseStringResManager* MBaseStringResManager::m_pInstance = NULL;

MBaseStringResManager* MBaseStringResManager::GetInstance()
{
	_ASSERT(m_pInstance != NULL);

	return m_pInstance;
}

void MBaseStringResManager::FreeInstance()
{
	if (m_pInstance)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

MBaseStringResManager::MBaseStringResManager()
{

}

MBaseStringResManager::~MBaseStringResManager()
{
	
}

bool MBaseStringResManager::Init(const char* szPath, const int nLangID, MZFileSystem* pfs )
{
	m_strPath = szPath;	
	m_pFS = pfs;

	string strFileName;

	// string table
	strFileName = m_strPath + FILENAME_STRING_TABLE;
	if (!m_StringTable.Initialize(strFileName.c_str(), nLangID, pfs))
	{
		_ASSERT(0);
		mlog("Error!! - StringTable Initalize Failed\n");
		return false;
	}

	// error table
	strFileName = m_strPath + FILENAME_ERROR_TABLE;
	if (!m_ErrorTable.Initialize(strFileName.c_str(), nLangID, pfs))
	{
		_ASSERT(0);
		mlog("Error!! - ErrorTable Initalize Failed\n");
		return false;
	}

	bool ret = OnInit();
	return ret;
}


const char* MBaseStringResManager::GetErrorStr(int nID)
{
	return m_ErrorTable.GetStr(nID);
}

const char* MBaseStringResManager::GetString(string& key)
{
	return m_StringTable.GetStr(key);
}

const char* MBaseStringResManager::GetStringFromXml(const char* str)
{
	int nHeaderLen = (int)strlen(TOK_XML_STRING_HEADER);
	int nStrLen = (int)strlen(str);

	if (nStrLen <= nHeaderLen) return str;
	if (strnicmp(str, TOK_XML_STRING_HEADER, nHeaderLen)) return str;

	string key;
	key = &str[nHeaderLen];

	return GetString(key);
}