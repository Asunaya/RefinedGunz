#include "stdafx.h"
#include "MLocatorConfig.h"
#include "ini.h"
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

using MapType = std::unordered_map<std::string, std::unordered_map<std::string, std::string>>;

struct IniContext
{
	MapType Map;
	optional<StringView> GetString(const char* arg_section, const char* arg_name) const
	{
		auto it = Map.find(arg_section);
		if (it == Map.end())
			return nullopt;

		auto it2 = it->second.find(arg_name);
		if (it2 == it->second.end())
			return nullopt;

		return it2->second;
	}
	optional<int> GetInt(const char* arg_section, const char* arg_name) const
	{
		auto Str = GetString(arg_section, arg_name);
		if (!Str)
			return nullopt;
		return StringToInt(*Str);
	};
};

extern "C" {
static int IniCallbackFwd(void* user, const char* section, const char* name, const char* value)
{
	auto& Map = *static_cast<MapType*>(user);
	StringView v = value;
	if (starts_with(v, "\""))
		v.remove_prefix(1);
	if (ends_with(v, "\""))
		v.remove_suffix(1);
	Map[section][name] = v.str();
	return 0;
}
}

bool MLocatorConfig::LoadConfig()
{
	IniContext ini;
	if (!ini_parse(LOCATOR_CONFIG, IniCallbackFwd, &ini.Map))
		return false;
#ifdef LOCATOR_FREESTANDING
	if( !LoadDBConfig(ini) )	return false;
#endif
	if( !LoadNetConfig(ini) )	return false;
	if( !LoadEnvConfig(ini) )	return false;

	m_bIsInitCompleted = true;

	return true;
}

bool Set(const IniContext& ini, const char* section, const char* name, MLocatorConfig* conf,
	bool (MLocatorConfig::*func)(const char*))
{
	auto Str = ini.GetString(section, name);
	if (!Str || Str->empty())
		return false;
	return (conf->*func)(Str->data());
}

template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
bool Set(const IniContext& ini, const char* section, const char* name, MLocatorConfig* conf,
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

bool MLocatorConfig::LoadDBConfig(const IniContext& ini)
{
	SET("DB", "DNS", SetDBDSN);
	SET("DB", "USERNAME", SetDBUserName);
	SET("DB", "PASSWORD", SetDBPassword);
	return true;
}


bool MLocatorConfig::LoadNetConfig(const IniContext& ini)
{
#ifdef LOCATOR_FREESTANDING
	SET("NETWORK", "IP", SetLocatorIP);
#endif
	SET("NETWORK", "PORT", SetLocatorPort);
	return true;
}

bool MLocatorConfig::LoadEnvConfig(const IniContext& ini)
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