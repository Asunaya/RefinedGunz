#include "stdafx.h"
#include "MLocatorConfig.h"
#include "IniParser.h"
#include "MUtil.h"
#include "StringView.h"
#include "ArrayView.h"
#include <unordered_map>

MLocatorConfig* GetLocatorConfig()
{
	return MLocatorConfig::GetLocatorConfigInst();
}

MLocatorConfig::MLocatorConfig(void) : m_bIsInitCompleted( false )
{
}

MLocatorConfig::~MLocatorConfig(void)
{
}

bool MLocatorConfig::LoadConfig()
{
	IniParser ini;
	if (!ini.Parse(LOCATOR_CONFIG))
		return false;
#ifdef LOCATOR_FREESTANDING
	if( !LoadDBConfig(ini) )	return false;
#endif
	if( !LoadNetConfig(ini) )	return false;
	if( !LoadEnvConfig(ini) )	return false;

	m_bIsInitCompleted = true;

	return true;
}

bool Set(const IniParser& ini, const char* section, const char* name, MLocatorConfig* conf,
	bool (MLocatorConfig::*func)(const char*))
{
	auto Str = ini.GetString(section, name);
	if (!Str || Str->empty())
		return false;
	return (conf->*func)(Str->data());
}

template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
bool Set(const IniParser& ini, const char* section, const char* name, MLocatorConfig* conf,
	void (MLocatorConfig::*func)(T))
{
	auto Value = ini.GetInt(section, name);
	if (Value)
		(conf->*func)(*Value);
	return Value.has_value();
}

#define SET(section, name, func) do {\
		if (!Set(ini, section, name, this, &MLocatorConfig::func)) return false;\
	} while (false)

bool MLocatorConfig::LoadDBConfig(const IniParser& ini)
{
	SET("DB", "DNS", SetDBDSN);
	SET("DB", "USERNAME", SetDBUserName);
	SET("DB", "PASSWORD", SetDBPassword);
	return true;
}


bool MLocatorConfig::LoadNetConfig(const IniParser& ini)
{
#ifdef LOCATOR_FREESTANDING
	SET("NETWORK", "IP", SetLocatorIP);
#endif
	SET("NETWORK", "PORT", SetLocatorPort);
	return true;
}

bool MLocatorConfig::LoadEnvConfig(const IniParser& ini)
{
	auto High = ini.GetInt("ENV", "LOCATOR_UID_HIGH");
	auto Low = ini.GetInt("ENV", "LOCATOR_UID_LOW");
	if (!High || !Low)
		return false;
	SetLocatorUID(MUID(*High, *Low));
	SET("ENV", "ID", SetLocatorID);
	SET("ENV", "MAX_ELAPSED_UPDATE_SERVER_STATUS_TIME", SetMaxElapsedUpdateServerSTatusTime);
	SET("ENV", "UDP_LIVE_TIME", SetUDPLiveTime);
	SET("ENV", "MAX_FREE_RECV_COUNT_PER_LIVE_TIME", SetMaxFreeUseCountPerLiveTime);
	SET("ENV", "BLOCK_TIME", SetBlockTime);
	SET("ENV", "UPDATE_UDP_MANAGER_ELAPSED_TIME", SetUpdateUDPManagerElapsedTime);
	SET("ENV", "MARGIN_OF_ERROR_MIN", SetMarginOfErrorMin);
	SetCountryCodeFilterStatus(bool(ini.GetInt("ENV", "USE_COUNTRY_CODE_FILTER").value_or(0)));
	SET("ENV", "ELAPSED_TIME_UPDATE_LOCATOR_LOG", SetElapsedTimeUpdateLocaorLog);
	SetAcceptInvalidIP(bool(ini.GetInt("ENV", "ACCEPT_INVALID_IP").value_or(1)));
	SetTestServerOnly(bool(ini.GetInt("ENV", "TEST_SERVER").value_or(0)));

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