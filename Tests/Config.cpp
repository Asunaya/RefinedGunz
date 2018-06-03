#include <cctype>
#include <vector>
#include <functional>
#include <array>
#include <random>
#include "ArrayView.h"
#include "MFile.h"
#include "MMatchConfig.h"
#include "MLocatorConfig.h"
#include "TestAssert.h"

namespace TestConfigInternal {
namespace {

#include "TestRandom.h"

template <typename T>
std::enable_if_t<std::is_integral<T>::value, bool> eq(T a, StringView b)
{
	auto i = StringToInt<T>(b);
	if (!i)
		return false;
	return a == *i;
}

template <typename T>
std::enable_if_t<std::is_enum<T>::value, bool> eq(T a, StringView b)
{
	return iequals(ToString(a), b);
}

template <typename A, typename B>
std::enable_if_t<!std::is_integral<A>::value && !std::is_enum<A>::value, bool>
eq(const A& a, const B& b)
{
	return a == b;
}

template <typename T>
struct ConfigTestState
{
	MFile::RWFile File;
	std::vector<std::function<void(T&)>> TestFuncs;
};

enum EntryType { Bool, Int, String, IP, Port };

template <typename T>
void AddSection(ConfigTestState<T>& State, const char* Name)
{
	char Str[1024];
	auto Size = sprintf_safe(Str, "[%s]\n", Name);
	State.File.write(Str, Size);
}

void RandomizeCapitalization(ArrayView<char> String)
{
	for (char& c : String)
	{
		if (!c)
			break;
		if (RandomNumber(1) && isalpha(c))
			c = islower(c) ? toupper(c) : tolower(c);
	}
};

template <typename T, typename F>
void Write(ConfigTestState<T>& State, StringView Name, StringView Value, F&& TestFunc)
{
	char NameP[256];
	strcpy_safe(NameP, Name);
	RandomizeCapitalization(NameP);
	char Str[1024];
	auto Size = sprintf_safe(Str, "%s=\"%.*s\"\n", NameP, Value.size(), Value.data());
	State.File.write(Str, Size);
	State.File.flush();
	State.TestFuncs.push_back(std::move(TestFunc));
}

void GetValue(ArrayView<char> Value, EntryType Type)
{
	switch (Type)
	{
	case EntryType::Bool:
		itoa_safe(RandomNumber(1), Value);
		break;
	case EntryType::Int:
		itoa_safe(RandomNumber(0, INT_MAX), Value);
		break;
	case EntryType::String:
		RandomString(Value);
		break;
	case EntryType::IP:
	{
		auto f = [] { return RandomNumber(255); };
		sprintf_safe(Value, "%d.%d.%d.%d", f(), f(), f(), f());
	}
	case EntryType::Port:
		itoa_safe(RandomNumber(0, USHRT_MAX), Value);
		break;
	break;
	}
}

template <typename T, typename F>
auto GetFunction(F f)
{
	struct {
		F f;
		char Value[32];
		void operator()(T& Config)
		{
			TestAssert(eq(std::mem_fn(f)(Config), StringView(Value)));
		}
	} TestFunc{f};
	return TestFunc;
}

template <typename T, typename F>
void Add(ConfigTestState<T>& State, F f, StringView Name, EntryType Type)
{
	auto TestFunc = GetFunction<T>(f);
	GetValue(TestFunc.Value, Type);
	Write(State, Name, TestFunc.Value, TestFunc);
}

template <typename T, typename F, typename E>
void AddEnum(ConfigTestState<T>& State, F f, StringView Name, E Max)
{
	auto TestFunc = GetFunction<T>(f);
	strcpy_safe(TestFunc.Value, ToString(E(RandomNumber(int(Max) - 1))));
	Write(State, Name, TestFunc.Value, TestFunc);
}

template <typename T, typename F>
void AddWithFunction(ConfigTestState<T>& State, F f, const char* Name, const char* Value)
{
	Write(State, Name, Value, f);
}

template <typename T, typename F>
void AddWithFunction(ConfigTestState<T>& State, F f, const char* Name, EntryType Type)
{
	char Value[32];
	GetValue(Value, Type);
	auto Func = [f, Value](T& Config) {
		return f(Value, Config);
	};
	Write(State, Name, Value, Func);
}

template <typename T, typename F>
void AddWithValue(ConfigTestState<T>& State, F f, const char* Name, const char* Value)
{
	auto TestFunc = GetFunction<T>(f);
	strcpy_safe(TestFunc.Value, Value);
	Write(State, Name, Value, TestFunc);
}

template <typename T>
void Test(ConfigTestState<T>& State, T& Config)
{
	for (auto&& TestFunc : State.TestFuncs)
	{
		TestFunc(Config);
	}
}

} // namespace
} // namespace TestConfigInternal

void TestConfig()
{
	using namespace TestConfigInternal;
	{
		enum { UseODBC = 1, UseSQLServer = 2, Max = 4 };
		using C = MMatchConfig;
		auto EnableMapsFunc = [](MMatchConfig& Config) {
			for (int i = 0; i < int(MMATCH_MAP_MAX); ++i)
			{
				bool ShouldSucceed = i == MMATCH_MAP_MANSION || i == MMATCH_MAP_BATTLE_ARENA;
				TestAssert(Config.IsEnableMap(MMATCH_MAP(i)) == ShouldSucceed);
			}
		};
		auto Filename = SERVER_CONFIG_FILENAME;
		MFile::Delete(Filename);
		MFile::CreateFile(Filename);
		TestAssert(C{}.Create());
		for (int i = 0; i < 10; ++i)
		{
			ConfigTestState<C> s{MFile::RWFile{Filename, MFile::Clear | MFile::Text}};
			AddSection(s, "SERVER");
			Add(s, &C::GetMaxUser, "MAXUSER", EntryType::Int);
			Add(s, &C::GetServerID, "SERVERID", EntryType::Int);
			Add(s, &C::GetServerName, "SERVERNAME", EntryType::String);
			AddEnum(s, &C::GetServerMode, "MODE", MSM_MAX);
			Add(s, &C::GetGameDirectory, "game_dir", EntryType::String);
			Add(s, &C::IsMasterServer, "is_master_server", EntryType::Bool);
			AddWithFunction(s, EnableMapsFunc, "EnableMap", "Mansion;Battle Arena");
			AddSection(s, "LOCALE");
			Add(s, &C::GetNJDBAgentIP, "DBAgentIP", EntryType::IP);
			Add(s, &C::GetNJDBAgentPort, "DBAgentPort", EntryType::Port);
			Add(s, &C::GetNJDBAgentGameCode, "GameCode", EntryType::Int);
			AddSection(s, "FILTER");
			Add(s, &C::IsUseFilter, "USE", EntryType::Bool);
			Add(s, &C::IsAcceptInvalidIP, "ACCEPT_INVALID_IP", EntryType::Bool);
			AddSection(s, "ENVIRONMENT");
			Add(s, &C::IsUseEvent, "USE_EVENT", EntryType::Bool);
			Add(s, &C::IsUseFileCrc, "USE_FILECRC", EntryType::Bool);

			AddSection(s, "DB");
			int DBIteration = i % Max;
			bool UseSQLite = bool(RandomNumber(1));
			AddWithValue(s, &C::GetDatabaseType, "database_type",
				UseSQLite ? "sqlite" : "mssql");
			if (!UseSQLite)
			{
				auto Driver = DBIteration & UseODBC ?
					MDatabase::DBDriver::ODBC : MDatabase::DBDriver::SQLServer;
				auto Auth = DBIteration & UseSQLServer ?
					MDatabase::DBAuth::SQLServer : MDatabase::DBAuth::Windows;
				auto AddEntry = [&](const char* Name, auto&& Value, auto&& f) {
					AddWithFunction(s, [=](MMatchConfig& Config) {
						TestAssert(eq(Config.GetDBConnectionDetails().*f, Value));
					}, Name, Value);
				};
				AddEntry("driver", ToString(Driver), &MDatabase::ConnectionDetails::Driver);
				char Str[32];
				if (Driver == MDatabase::DBDriver::SQLServer)
				{
					RandomString(Str);
					AddEntry("server", Str, &MDatabase::ConnectionDetails::Server);
					RandomString(Str);
					AddEntry("database", Str, &MDatabase::ConnectionDetails::Database);
				}
				else
				{
					RandomString(Str);
					AddEntry("DNS", Str, &MDatabase::ConnectionDetails::DSN);
				}

				AddEntry("auth", ToString(Auth), &MDatabase::ConnectionDetails::Auth);
				if (Auth == MDatabase::DBAuth::SQLServer)
				{
					RandomString(Str);
					AddEntry("username", Str, &MDatabase::ConnectionDetails::Username);
					RandomString(Str);
					AddEntry("password", Str, &MDatabase::ConnectionDetails::Password);
				}
			}
			C Config;
			TestAssert(Config.Create());
			Test(s, Config);
		}
	}
	{
		using C = MLocatorConfig;
		auto Filename = LOCATOR_CONFIG;
		MFile::Delete(Filename);
		MFile::CreateFile(Filename);
		TestAssert(C{}.LoadConfig());
		for (int i = 0; i < 10; ++i)
		{
			ConfigTestState<C> s{MFile::RWFile{Filename, MFile::Clear | MFile::Text}};
			auto AddLocatorUIDPart = [&](bool High, const char* Name) {
				auto Func = [=](StringView Value, MLocatorConfig& Config) {
					auto UID = Config.GetLocatorUID();
					auto Part = High ? UID.High : UID.Low;
					return eq(Part, Value);
				};
				return AddWithFunction(s, Func, Name, EntryType::Int);
			};
			AddSection(s, "DB");
			Add(s, &C::GetDBDSN, "DNS", EntryType::String);
			Add(s, &C::GetDBUserName, "USERNAME", EntryType::String);
			Add(s, &C::GetDBPassword, "PASSWORD", EntryType::String);
			AddSection(s, "NETWORK");
			Add(s, &C::GetLocatorIP, "IP", EntryType::IP);
			Add(s, &C::GetLocatorPort, "PORT", EntryType::Port);
			AddSection(s, "ENV");
			AddLocatorUIDPart(false, "LOCATOR_UID_HIGH");
			AddLocatorUIDPart(true, "LOCATOR_UID_LOW");
			Add(s, &C::GetLocatorID, "ID", EntryType::Int);
			Add(s, &C::GetMaxElapsedUpdateServerStatusTime, "MAX_ELAPSED_UPDATE_SERVER_STATUS_TIME",
				EntryType::Int);
			Add(s, &C::GetUDPLiveTime, "UDP_LIVE_TIME", EntryType::Int);
			Add(s, &C::GetMaxFreeUseCountPerLiveTime, "MAX_FREE_RECV_COUNT_PER_LIVE_TIME",
				EntryType::Int);
			Add(s, &C::GetBlockTime, "BLOCK_TIME", EntryType::Int);
			Add(s, &C::GetUpdateUDPManagerElapsedTime, "UPDATE_UDP_MANAGER_ELAPSED_TIME",
				EntryType::Int);
			Add(s, &C::GetMarginOfErrorMin, "MARGIN_OF_ERROR_MIN", EntryType::Int);
			Add(s, &C::GetElapsedTimeUpdateLocatorLog, "ELAPSED_TIME_UPDATE_LOCATOR_LOG",
				EntryType::Int);
			Add(s, &C::IsUseCountryCodeFilter, "USE_COUNTRY_CODE_FILTER", EntryType::Bool);
			Add(s, &C::IsAcceptInvalidIP, "ACCEPT_INVALID_IP", EntryType::Bool);
			Add(s, &C::IsTestServerOnly, "TEST_SERVER", EntryType::Bool);
			C Config;
			TestAssert(Config.LoadConfig());
			Test(s, Config);
		}
	}
}