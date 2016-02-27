#include "stdafx.h"
#include "MAsyncDBJob_CountryFilter.h"


void MAsyncDBJob_UpdateIPtoCountryList::Run( void* pContext )
{
	MMatchServer* pMatchServer = MMatchServer::GetInstance();
	if( 0 == pMatchServer ) 
		return;

	MMatchDBMgr* pDBMgr = reinterpret_cast< MMatchDBMgr* >( pContext );
	if( 0 == pDBMgr ) 
		return;

	if( !pDBMgr->GetIPtoCountryList(pMatchServer->GetTmpIPtoCountryList()) )
	{
		SetResult( MASYNC_RESULT_FAILED );
		return;
	}

	SetResult( MASYNC_RESULT_SUCCEED );
}


void MAsyncDBJob_UpdateBlockCountryCodeList::Run( void* pContext )
{
	MMatchServer* pMatchServer = MMatchServer::GetInstance();
	if( 0 == pMatchServer ) 
		return;

	MMatchDBMgr* pDBMgr = reinterpret_cast< MMatchDBMgr* >( pContext );
	if( 0 == pDBMgr ) 
		return;

	if( !pDBMgr->GetBlockCountryCodeList(pMatchServer->GetTmpBlockCountryCodeList()) )
	{
		SetResult( MASYNC_RESULT_FAILED );
		return;
	}

	SetResult( MASYNC_RESULT_SUCCEED );
}


void MAsyncDBJob_UpdateCustomIPList::Run( void* pContext )
{
	MMatchServer* pMatchServer = MMatchServer::GetInstance();
	if( 0 == pMatchServer ) 
		return;

	MMatchDBMgr* pDBMgr = reinterpret_cast< MMatchDBMgr* >( pContext );
	if( 0 == pDBMgr ) 
		return;

	if( !pDBMgr->GetCustomIPList(pMatchServer->GetTmpCustomIPList()) )
	{
		SetResult( MASYNC_RESULT_FAILED );
		return;
	}
	
	SetResult( MASYNC_RESULT_SUCCEED );
}