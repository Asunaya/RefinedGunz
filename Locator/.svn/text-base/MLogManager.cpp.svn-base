#include "stdafx.h"
#include "MDebug.h"
#include "MLogManager.h"

MLogManager::MLogManager()
{
	InitializeCriticalSection( &m_csLock );
}


MLogManager::~MLogManager()
{
	DeleteCriticalSection( &m_csLock );
}


void MLogManager::InsertLog( const string& strLog )
{
	m_MLog.push_back( strLog );
}


void MLogManager::SafeInsertLog( const string& strLog )
{
	Lock();
	InsertLog( strLog );
	Unlock();
}


void MLogManager::WriteMLog()
{
	vector< string >::iterator it, end;
	for( it = m_MLog.begin(), end = m_MLog.end(); it != end; ++it )
		mlog( it->c_str() );
}


void MLogManager::SafeWriteMLog()
{
	Lock();
	WriteMLog();
	Unlock();
}


void MLogManager::SafeReset()
{
	Lock();
	Reset();
	Unlock();
}


MLogManager& GetLogManager()
{
	return MLogManager::GetInstance();
}