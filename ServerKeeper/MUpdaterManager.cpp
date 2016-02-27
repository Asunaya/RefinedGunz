#include "stdafx.h"
#include "MUpdater.h"
#include "MUpdaterManager.h"


MUpdaterManager::MUpdaterManager()
{
}


MUpdaterManager::~MUpdaterManager()
{
}


void MUpdaterManager::Clear()
{
	MUpdaterManager::iterator It, End;
	for( It = begin(), End = end(); It != End; ++It )
		delete It->second;
	clear();
}


bool MUpdaterManager::Insert( const UPDATER_TYPE nType, MUpdater* pUpdater )
{
	if( (nType < UT_END) && (0 != pUpdater) )
	{
		insert( MUpdaterManager::value_type(nType, pUpdater) );
		return true;
	}

	return false;
}


MUpdater* MUpdaterManager::Find( const UPDATER_TYPE nType )
{
	MUpdaterManager::iterator It = find( nType );
	if( end() == It )
		return 0;
	return It->second;
}