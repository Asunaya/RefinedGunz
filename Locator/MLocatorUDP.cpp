#include "StdAfx.h"
#include "MLocatorUDP.h"
#include "mmsystem.h"
#include "MDebug.h"

#include <winsock2.h>

#include <utility>
#include <algorithm>
using namespace std;

#pragma comment( lib, "winmm.lib" )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const DWORD MLocatorUDPInfo::CalcuUseElapsedTime()
{
	return (timeGetTime() - GetUseStartTime());
}


void MLocatorUDPInfo::SetInfo( const DWORD dwIP, 
							   const int nPort, 
							   const unsigned int nUseCount, 
							   const DWORD dwUseStartTime, 
							   const string& strIP )
{
	SetIP( dwIP );
	SetStrIP( strIP );
	SetPort( nPort );
	SetUseCount( nUseCount );
	SetUseStartTime( dwUseStartTime );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MUDPManager::MUDPManager()
{
	InitializeCriticalSection( &m_csLock );
}


MUDPManager::~MUDPManager()
{
	SafeDestroy();

	DeleteCriticalSection( &m_csLock );
}


bool MUDPManager::Insert( const DWORD dwIPKey, const int nPort, const DWORD dwUseStartTime )
{
	const_iterator itFind = find( dwIPKey );
	if( itFind == end() )
	{
		MLocatorUDPInfo* pRecvUDPInfo = new MLocatorUDPInfo;
		if( 0 != pRecvUDPInfo )
		{
			in_addr addr;
			addr.S_un.S_addr = dwIPKey;

			pRecvUDPInfo->SetInfo( dwIPKey, nPort, 1, dwUseStartTime, inet_ntoa(addr) );

			insert( pair<DWORD, MLocatorUDPInfo*>(dwIPKey, pRecvUDPInfo) );

			return true;
		}
		else
		{
			mlog( "MUDPManager::Insert - memory error\n" );
		}
	}
	
	return false;
}


bool MUDPManager::Insert( const DWORD dwIPKey, MLocatorUDPInfo* pRecvUDPInfo )
{
	const_iterator itFind = find( dwIPKey );
	if( itFind == end() )
	{
		insert( pair<DWORD, MLocatorUDPInfo*>(dwIPKey, pRecvUDPInfo) );
		return true;
	}
	return false;
}


bool MUDPManager::SafeInsert( const DWORD dwIPKey, const int nPort, const DWORD dwUseStartTime )
{
	Lock();
	const bool bResult = Insert( dwIPKey, nPort, dwUseStartTime );
	Unlock();

	return bResult;
}


bool MUDPManager::SafeInsert( const DWORD dwIPKey, MLocatorUDPInfo* pRecvUDPInfo )
{
	Lock();
	const bool bResult = Insert( dwIPKey, pRecvUDPInfo );
	Unlock();
	
	return bResult;
}


MLocatorUDPInfo* MUDPManager::PopFirst()
{
	if( empty() ) return 0;

	iterator			itBegin			= begin();
	MLocatorUDPInfo*	pRecvUDPInfo	= itBegin->second;

	erase( itBegin );

	return pRecvUDPInfo;
}


MLocatorUDPInfo* MUDPManager::SafePopFirst()
{
	Lock();
	MLocatorUDPInfo* pRecvUDPInfo = PopFirst();
	Unlock();

	return pRecvUDPInfo;
}


MLocatorUDPInfo* MUDPManager::PopByIPKey( const DWORD dwIPKey )
{
	iterator itFind = find( dwIPKey );
	if( end() != itFind )
	{
		MLocatorUDPInfo* pRecvUDPInfo = itFind->second;

		erase( itFind );

		return pRecvUDPInfo;
	}

	return 0;
}


MLocatorUDPInfo* MUDPManager::SafePopByIPKey( const DWORD dwIPKey )
{
	Lock();
	MLocatorUDPInfo* pRecvUDPInfo = PopByIPKey( dwIPKey );
	Unlock();

	return pRecvUDPInfo;
}


bool MUDPManager::CheckWasInserted( const DWORD dwIPKey )
{
	const_iterator itFind = find( dwIPKey );
	if( itFind != end() )
	{
		itFind->second->IncreaseUseCount();
		return true;
	}

	return false;
}


bool MUDPManager::SafeCheckWasInserted( const DWORD dwIPKey )
{
	Lock();
	const bool bResult = CheckWasInserted( dwIPKey );
	Unlock();

	return bResult;
}


const unsigned char MUDPManager::GetUseCount( const DWORD dwIPKey )
{
	const_iterator itFind = find( dwIPKey );
	if( end() != itFind )
		return itFind->second->GetUseCount();
	
	return 0;
}


const unsigned char MUDPManager::SafeGetUseCount( const DWORD dwIPKey )
{
	Lock();
	const unsigned char nUseCount = GetUseCount( dwIPKey );
	Unlock();

	return nUseCount;
}


void MUDPManager::SafeDestroy()
{
	Lock();
	iterator It, End;
	for( It = begin(), End = end(); It != End; ++It )
		delete It->second;
	clear();
	Unlock();
}


MLocatorUDPInfo* MUDPManager::Find( const DWORD dwIPKey )
{
	const_iterator itFind = find( dwIPKey );
	if( end() != itFind )
		return itFind->second;

	return 0;
}


MLocatorUDPInfo* MUDPManager::SafeFind( const DWORD dwIPKey )
{
	Lock();
	MLocatorUDPInfo* pRecvUDPInfo = Find( dwIPKey );
	Unlock();

	return pRecvUDPInfo;
}


void MUDPManager::Delete( const DWORD dwIPKey )
{
	MLocatorUDPInfo* pRecvUDPInfo = PopByIPKey( dwIPKey );
	if( 0 != pRecvUDPInfo )
		delete pRecvUDPInfo;
}


void MUDPManager::SafeDelete( const DWORD dwIPKey )
{
	Lock();
	Delete( dwIPKey );
	Unlock();
}


void MUDPManager::ClearElapsedLiveTimeUDP( const DWORD dwLiveTime, const DWORD dwEventTime )
{
	iterator itDeadUDP;
	while( true )
	{
		itDeadUDP = find_if( begin(), end(), 
			MDeadUDPFinder< pair<DWORD, MLocatorUDPInfo*> >(dwLiveTime, dwEventTime) );

		if( end() == itDeadUDP )
			break;

		delete itDeadUDP->second;
		erase( itDeadUDP );
	}
}


void MUDPManager::SafeClearElapsedLiveTimeUDP( const DWORD dwLiveTime, const DWORD dwEventTime )
{
	Lock();
	ClearElapsedLiveTimeUDP( dwLiveTime, dwEventTime );
	Unlock();
}


void MUDPManager::DumpStatusInfo()
{
	char szDbgInfo[ 1024 ];

	_snprintf( szDbgInfo, 1023, "Size:%d\n", size() );

	mlog( szDbgInfo );
}


void MUDPManager::DumpUDPInfo()
{
	char szDbgInfo[ 1024 ];
	iterator It, End;

	for( It = begin(), End = end(); It != End; ++It )
	{
		in_addr addr;
		addr.S_un.S_addr = It->second->GetIP();
		_snprintf( szDbgInfo, 1023, "IP:%s, Port:%d, Count:%d\n", 
			inet_ntoa(addr), 
			It->second->GetPort(), 
			It->second->GetUseCount() );
		mlog( szDbgInfo );
	}
}


void MUDPManager::InitRecvUDPInfoMemPool()
{
	InitMemPool( MLocatorUDPInfo );
}


void MUDPManager::ReleaseRecvUDPInfoMemPool()
{
	ReleaseMemPool( MLocatorUDPInfo );

	UninitMemPool( MLocatorUDPInfo );
}

void MUDPManager::SetBegin()
{
	m_itBegin = begin();
}


MLocatorUDPInfo* MUDPManager::GetCurPosUDP()
{
	if( end() != m_itBegin )
		return m_itBegin->second;
	return 0;
}


bool MUDPManager::MoveNext()
{
	++m_itBegin;
	if( end() == m_itBegin ) return false;
	return true;
}