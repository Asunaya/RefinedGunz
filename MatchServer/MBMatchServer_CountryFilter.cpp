#include "stdafx.h"
#include "MMatchConfig.h"
#include "MAsyncDBJob_CountryFilter.h"
#include "MBMatchServer.h"


void MBMatchServer::OnLocalUpdateUseCountryFilter()
{
	MGetServerConfig()->SetUseFilterState( !MGetServerConfig()->IsUseFilter() );
}


void MBMatchServer::OnLocalGetDBIPtoCountry()
{
	MAsyncDBJob_UpdateIPtoCountryList* pAsyncJob = new MAsyncDBJob_UpdateIPtoCountryList;
	if( 0 != pAsyncJob )
		PostAsyncJob( pAsyncJob );
}


void MBMatchServer::OnLocalGetDBBlockCountryCode()
{
	MAsyncDBJob_UpdateBlockCountryCodeList* pAsyncJob = new MAsyncDBJob_UpdateBlockCountryCodeList;
	if( 0 != pAsyncJob )
		PostAsyncJob( pAsyncJob );
}


void MBMatchServer::OnLocalGetDBCustomIP()
{
	MAsyncDBJob_UpdateCustomIPList* pAsyncJob = new MAsyncDBJob_UpdateCustomIPList;
	if( 0 != pAsyncJob )
		PostAsyncJob( pAsyncJob );
}


void MBMatchServer::OnLocalUpdateIPtoCountry()
{
	if( GetCountryFilter().InitIPtoCountryList(m_TmpIPtoCountryList) )
		mlog( "success to update IPtoCountryList\n" );
	else
		mlog( "fail to update IPtoCountryList\n" );
}


void MBMatchServer::OnLocalUpdateBlockCountryCode()
{
	if( !GetCountryFilter().InitBlockCountryCodeList(m_TmpBlockCountryCodeList) )
		mlog( "success to update BlockCounryCode\n" );
	else
		mlog( "fail to update BlockCountryCode\n" );
}


void MBMatchServer::OnLocalUpdateCustomIP()
{
	if( GetCountryFilter().InitCustomIPList(m_TmpCustomIPList) )
		mlog( "success to update CustomIP\n" );
	else
		mlog( "fail to updaet CustomIP\n" );
}

void MBMatchServer::OnLocalUpdateAcceptInvaildIP()
{
	MGetServerConfig()->SetAcceptInvalidIPState( !MGetServerConfig()->IsAcceptInvalidIP() );
}