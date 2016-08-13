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
#ifdef LOCATOR_FREESTANDING
	if( !LoadDBConfig() )	return false;
#endif
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
	char szVal[256];

#ifdef LOCATOR_FREESTANDING
	GetPrivateProfileString( "NETWORK", "IP", "", szVal, 255, LOCATOR_CONFIG );
	if( 0 == strlen(szVal) ) return false;
	SetLocatorIP( szVal );
#endif

	GetPrivateProfileString( "NETWORK", "PORT", "", szVal, 255, LOCATOR_CONFIG );
	if (strlen(szVal) == 0)
	{
		MLog("MLocatorConfig::LoadNetConfig - Couldn't find port option in config\n");
		return false;
	}
	SetLocatorPort(atoi(szVal));

	return true;
}

static auto LoadField(const char* Field, const char* Default = "")
{
	char szVal[256];
	GetPrivateProfileString("ENV", Field, Default, szVal, 255, LOCATOR_CONFIG);
	if (strlen(szVal) == 0)
	{
		char err_msg[256];
		sprintf_safe(err_msg, "Couldn't find %s option in config", Field);
		throw std::runtime_error(err_msg);
	}
	return atoi(szVal);
}

bool MLocatorConfig::LoadEnvConfig()
{
	try
	{
		SetLocatorID(LoadField("ID"));
		auto High = LoadField("LOCATOR_UID_HIGH");
		auto Low = LoadField("LOCATOR_UID_LOW");
		SetLocatorUID(MUID(High, Low));
		SetMaxElapsedUpdateServerSTatusTime(LoadField("MAX_ELAPSED_UPDATE_SERVER_STATUS_TIME"));
		SetUDPLiveTime(LoadField("UDP_LIVE_TIME"));
		SetMaxFreeUseCountPerLiveTime(LoadField("MAX_FREE_RECV_COUNT_PER_LIVE_TIME"));
		SetBlockTime(LoadField("BLOCK_TIME"));
		SetUpdateUDPManagerElapsedTime(LoadField("UPDATE_UDP_MANAGER_ELAPSED_TIME"));
		SetMarginOfErrorMin(LoadField("MARGIN_OF_ERROR_MIN"));
		SetCountryCodeFilterStatus(LoadField("USE_COUNTRY_CODE_FILTER") != 0);
		SetElapsedTimeUpdateLocaorLog(LoadField("ELAPSED_TIME_UPDATE_LOCATOR_LOG"));
		SetAcceptInvalidIP(LoadField("ACCEPT_INVALID_IP", "1") != 0);
		SetTestServerOnly(LoadField("TEST_SERVER", "0") != 0);
	}
	catch (std::runtime_error& e)
	{
		MLog("MLocatorConfig::LoadEnvConfig - %s\n", e.what());
		return false;
	}

	return true;
}


bool MLocatorConfig::SetDBDSN( const char* pszDSN )
{
	if( (0 != pszDSN) && (0 < strlen(pszDSN)) ) 
	{
#ifdef MFC
		m_strDBDSN = pszDSN;
#endif
		return true;
	}
	return false;
}


bool MLocatorConfig::SetDBUserName( const char* pszUserName )
{
	if( (0 != pszUserName) && (0 < strlen(pszUserName)) )
	{
#ifdef MFC
		m_strDBUserName = pszUserName;
#endif
		return true;
	}
	return false;
}


bool MLocatorConfig::SetDBPassword( const char* pszPassword )
{
	if( (0 != pszPassword) && (0 < strlen(pszPassword)) )
	{
#ifdef MFC
		m_strDBPassword = pszPassword;
#endif
		return true;
	}
	return false;
}