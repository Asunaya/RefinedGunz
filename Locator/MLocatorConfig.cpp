#include "StdAfx.h"
#include ".\mlocatorconfig.h"




MLocatorConfig* GetLocatorConfig()
{
	return MLocatorConfig::GetLocatorConfigInst();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


MLocatorConfig::MLocatorConfig(void) : m_bIsInitCompleted( false )
{
}

MLocatorConfig::~MLocatorConfig(void)
{
}


bool MLocatorConfig::LoadConfig()
{
	if( !LoadDBConfig() )	return false;
	if( !LoadNetConfig() )	return false;
	if( !LoadEnvConfig() )	return false;

	m_bIsInitCompleted = true;

	return true;
}


bool MLocatorConfig::LoadDBConfig()
{
	char szVal[ 256 ];

	GetPrivateProfileString( "DB", "DNS", "", szVal, 255, LOCATOR_CONFIG );
	if( (0 == strlen(szVal)) || !SetDBDSN(szVal) ) return false;
	
	GetPrivateProfileString( "DB", "USERNAME", "", szVal, 255, LOCATOR_CONFIG );
	if( (0 == strlen(szVal)) || !SetDBUserName(szVal) ) return false;
	
	GetPrivateProfileString( "DB", "PASSWORD", "", szVal, 255, LOCATOR_CONFIG );
	if( (0 == strlen(szVal)) || !SetDBPassword(szVal) ) return false;
	
	return true;
}


bool MLocatorConfig::LoadNetConfig()
{
	char szVal[ 256 ];

	GetPrivateProfileString( "NETWORK", "IP", "", szVal, 255, LOCATOR_CONFIG );
	if( 0 == strlen(szVal) ) return false;
	SetLocatorIP( szVal );

	GetPrivateProfileString( "NETWORK", "PORT", "", szVal, 255, LOCATOR_CONFIG );
	if( 0 == strlen(szVal) ) return false;
	SetLocatorPort( atoi(szVal) );

	return true;
}


bool MLocatorConfig::LoadEnvConfig()
{
	char szVal[ 256 ];
	GetPrivateProfileString( "ENV", "ID", "", szVal, 255, LOCATOR_CONFIG );
	if( 0 == strlen(szVal) ) return false;
	SetLocatorID( atoi(szVal) );

	GetPrivateProfileString( "ENV", "LOCATOR_UID_HIGH", "", szVal, 255, LOCATOR_CONFIG );
	if( 0 == strlen(szVal) ) return false;
	unsigned long nHighUID = static_cast< unsigned long >( atol(szVal) );

	GetPrivateProfileString( "ENV", "LOCATOR_UID_LOW", "", szVal, 255, LOCATOR_CONFIG );
	if( 0 == strlen(szVal) ) return false;
	unsigned long nLowUID = static_cast< unsigned long >( atol(szVal) );

	SetLocatorUID( MUID(nHighUID, nLowUID) );

	GetPrivateProfileString( "ENV", "MAX_ELAPSED_UPDATE_SERVER_STATUS_TIME", "", szVal, 255, LOCATOR_CONFIG );
	if( 0 == strlen(szVal) ) return false;
	SetMaxElapsedUpdateServerSTatusTime( static_cast<unsigned long>(atol(szVal)) );

	GetPrivateProfileString( "ENV", "UDP_LIVE_TIME", "", szVal, 255, LOCATOR_CONFIG );
	if( 0 == strlen(szVal) ) return false;
	SetUDPLiveTime( static_cast<unsigned long>(atol(szVal)) );

	GetPrivateProfileString( "ENV", "MAX_FREE_RECV_COUNT_PER_LIVE_TIME", "", szVal, 255, LOCATOR_CONFIG );
	if( 0 == strlen(szVal) ) return false;
	SetMaxFreeUseCountPerLiveTime( atoi(szVal) );

	GetPrivateProfileString( "ENV", "BLOCK_TIME", "", szVal, 255, LOCATOR_CONFIG );
	if( 0 == strlen(szVal) ) return false;
	SetBlockTime( static_cast<unsigned long>(atol(szVal)) );

	GetPrivateProfileString( "ENV", "UPDATE_UDP_MANAGER_ELAPSED_TIME", "", szVal, 255, LOCATOR_CONFIG );
	if( 0 == strlen(szVal) ) return false;
	SetUpdateUDPManagerElapsedTime( static_cast<unsigned long>(atol(szVal)) );

	GetPrivateProfileString( "ENV", "MARGIN_OF_ERROR_MIN", "", szVal, 255, LOCATOR_CONFIG );
	if( 0 == strlen(szVal) ) return false;
	SetMarginOfErrorMin( static_cast<unsigned long>(atoi(szVal)) );

	GetPrivateProfileString( "ENV", "USE_COUNTRY_CODE_FILTER", "", szVal, 255, LOCATOR_CONFIG );
	if( 0 == strlen(szVal) ) return false;
	SetCountryCodeFilterStatus( 0 == stricmp("yes", szVal) );

	GetPrivateProfileString( "ENV", "ELAPSED_TIME_UPDATE_LOCATOR_LOG", "", szVal, 255, LOCATOR_CONFIG );
	if( 0 == strlen(szVal) ) return false;
	SetElapsedTimeUpdateLocaorLog( static_cast<DWORD>(atoi(szVal)) );

	GetPrivateProfileString( "ENV", "ACCEPT_INVALID_IP", "1", szVal, 255, LOCATOR_CONFIG );
	if( 0 == strlen(szVal) ) return false;
	SetAcceptInvalidIP( static_cast<bool>(atoi(szVal)) );

	GetPrivateProfileString( "ENV", "TEST_SERVER", "0", szVal, 255, LOCATOR_CONFIG );
	if( 0 == strlen(szVal) ) return false;
	SetTestServerOnly( static_cast<bool>(atoi(szVal)) );

	return true;
}


bool MLocatorConfig::SetDBDSN( const char* pszDSN )
{
	if( (0 != pszDSN) && (0 < strlen(pszDSN)) ) 
	{
		m_strDBDSN = pszDSN;
		return true;
	}
	return false;
}


bool MLocatorConfig::SetDBUserName( const char* pszUserName )
{
	if( (0 != pszUserName) && (0 < strlen(pszUserName)) )
	{
		m_strDBUserName = pszUserName;
		return true;
	}
	return false;
}


bool MLocatorConfig::SetDBPassword( const char* pszPassword )
{
	if( (0 != pszPassword) && (0 < strlen(pszPassword)) )
	{
		m_strDBPassword = pszPassword;
		return true;
	}
	return false;
}