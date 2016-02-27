#include "stdafx.h"
#include "MMatchConfig.h"
#include <windows.h>
#include "MMatchMap.h"
#include "MLex.h"
#include "MZFileSystem.h"

#include "MErrorTable.h"
#include "MMatchServer.h"


bool MMatchConfig::GetPrivateProfileBool(const char* szAppName, const char* szKeyName, 
						   bool bDefault, const char* szFileName)
{
	int nDefault = bDefault ? 1 : 0;

	int n;
	n = GetPrivateProfileInt(szAppName, szKeyName, nDefault, szFileName);
	if (n == 0) return false;
	return true;
}

MMatchConfig::MMatchConfig()
{
	m_nMaxUser					= 0;
	m_szDB_DNS[0]				= '\0';
	m_szDB_UserName[0]			= '\0';
	m_szDB_Password[0]			= '\0';
	m_nServerID					= 0;
	m_szServerName[0]			= '\0';
	m_nServerMode				= MSM_NORMAL;
	m_bRestrictionMap			= false;
	m_bCheckPremiumIP			= false;
	m_bUseFilter				= false;
	m_bAcceptInvalidIP			= false;
	m_bIsDebugServer			= false;
	m_bEnabledCreateLadderGame	= true;
	m_bIsComplete				= false;
}

MMatchConfig::~MMatchConfig()
{

}

MMatchConfig* MMatchConfig::GetInstance()
{
	static MMatchConfig m_MatchConfig;
	return &m_MatchConfig;
}

bool MMatchConfig::Create()
{
	int nTemp = 0;

	GetPrivateProfileString("DB", "DNS", "gunzdb", m_szDB_DNS, 64, SERVER_CONFIG_FILENAME);
	GetPrivateProfileString("DB", "USERNAME", "gunzdb", m_szDB_UserName, 64, SERVER_CONFIG_FILENAME);
	GetPrivateProfileString("DB", "PASSWORD", "gunzdb", m_szDB_Password, 64, SERVER_CONFIG_FILENAME);

	m_nMaxUser = GetPrivateProfileInt("SERVER", "MAXUSER", 1500, SERVER_CONFIG_FILENAME);
	m_nServerID = GetPrivateProfileInt("SERVER", "SERVERID", 1500, SERVER_CONFIG_FILENAME);
	GetPrivateProfileString("SERVER", "SERVERNAME", "matchserver", m_szServerName, 256, SERVER_CONFIG_FILENAME);

	char szServerMode[128] = "";
	GetPrivateProfileString("SERVER", "MODE", SERVER_CONFIG_SERVERMODE_NORMAL, szServerMode, 128, SERVER_CONFIG_FILENAME);

	if (!_stricmp(szServerMode, SERVER_CONFIG_SERVERMODE_NORMAL)) m_nServerMode = MSM_NORMAL;
	else if (!_stricmp(szServerMode, SERVER_CONFIG_SERVERMODE_CLAN)) m_nServerMode = MSM_CLAN;
	else if (!_stricmp(szServerMode, SERVER_CONFIG_SERVERMODE_LADDER)) m_nServerMode = MSM_LADDER;
	else if (!_stricmp(szServerMode, SERVER_CONFIG_SERVERMODE_EVENT)) m_nServerMode = MSM_EVENT;
	else if (!_stricmp(szServerMode, SERVER_CONFIG_SERVERMODE_TEST)) m_nServerMode = MSM_TEST;
	else { _ASSERT(0); }

	// 인원제한 무시하는 허용IP
	char szAllowIP[1024] = "";
	char* pNextArg = szAllowIP;
	GetPrivateProfileString("SERVER", "FREELOGINIP", "", szAllowIP, 1024, SERVER_CONFIG_FILENAME);
	MLex lex;
	while(true) {
		char szIP[128] = "";
		pNextArg = lex.GetOneArg(pNextArg, szIP);
		if (*szIP == NULL)
			break;
		AddFreeLoginIP(szIP);
	}

	char szDebug[4] = {0,};
	GetPrivateProfileString( "SERVER", "DEBUG", SERVER_CONFIG_DEBUG_DEFAULT, szDebug, 4, SERVER_CONFIG_FILENAME );
	if( 0 == _stricmp("0", szDebug) )
		m_bIsDebugServer = false;
	else
		m_bIsDebugServer = true;

	// Debug ip.
	char szDebugIP[ 1024 ] = {0,};
	char* pNextDbgIP = szDebugIP;
	GetPrivateProfileString( "SERVER", "DEBUGIP", "", szDebugIP, 1024, SERVER_CONFIG_FILENAME );
	while(true) {
		char szIP[128] = "";
		pNextDbgIP = lex.GetOneArg(pNextDbgIP, szIP);
		if (*szIP == NULL)
			break;
		AddDebugLoginIP(szIP);
	}

	char szKeeperIP[ 32 ] = "";
	GetPrivateProfileString( "SERVER", "KEEPERIP", "", szKeeperIP, 32, SERVER_CONFIG_FILENAME );
	if( 0 == strlen(szKeeperIP) )
	{
		mlog( "server.ini - Keeper ip not setting\n" );
		return false;
	}
	m_strKeeperIP = szKeeperIP;
	
	// 프리미엄 IP 체크
	int nCheckPremiumIP = GetPrivateProfileInt("SERVER", "CheckPremiumIP", 0, SERVER_CONFIG_FILENAME);
	if (nCheckPremiumIP != 0) m_bCheckPremiumIP = true;

	char szCountry[ 32 ] = "";
	GetPrivateProfileString( "SERVER", "COUNTRY", "", szCountry, 31, SERVER_CONFIG_FILENAME );
	if( 0 != strlen(szCountry) )
		m_strCountry = szCountry;
	else
	{
		ASSERT( 0 );
		mlog( "server.ini - Invalid country type.\n" );
		return false;
	}

	char szLanguage[ 32 ] = "";
	GetPrivateProfileString( "SERVER", "LANGUAGE", "", szLanguage, 31, SERVER_CONFIG_FILENAME );
	if( 0 != strlen(szLanguage) )
		m_strLanguage = szLanguage;
	else
	{
		ASSERT( 0 );
		mlog( "server.ini - Invalid language type.\n" );
		return false;
	}


	// 일본 넷마블 전용
	GetPrivateProfileString("LOCALE", "DBAgentIP",						SERVER_CONFIG_DEFAULT_NJ_DBAGENT_IP, m_NJ_szDBAgentIP, 64, SERVER_CONFIG_FILENAME);
	m_NJ_nDBAgentPort = GetPrivateProfileInt("LOCALE", "DBAgentPort",	SERVER_CONFIG_DEFAULT_NJ_DBAGENT_PORT, SERVER_CONFIG_FILENAME);
	m_NJ_nGameCode = GetPrivateProfileInt("LOCALE", "GameCode",			SERVER_CONFIG_DEFAULT_NJ_DBAGENT_GAMECODE, SERVER_CONFIG_FILENAME);


	ReadEnableMaps();

	// filer.
	char szUse[ 2 ] = {0,};
	GetPrivateProfileString( "FILTER", "USE", "0", szUse, 2, SERVER_CONFIG_FILENAME );
	SetUseFilterState( atoi(szUse) != 0 );
	
	char szAccept[ 2 ] = {0,};
	GetPrivateProfileString( "FILTER", "ACCEPT_INVALID_IP", "1", szAccept, 2, SERVER_CONFIG_FILENAME );
	SetAcceptInvalidIPState( atoi(szAccept) != 0 );

	// environment
	char szUseHShield[ 2 ] = {0,};
	GetPrivateProfileString("ENVIRONMENT", "USE_HSHIELD", SERVER_CONFIG_DEFAULT_USE_HSHIELD, szUseHShield, 2, SERVER_CONFIG_FILENAME);
	ASSERT( 0 != strlen(szUseHShield) );
	if( 0 == _stricmp("1", szUseHShield) )
		m_bIsUseHShield = true;
	else
		m_bIsUseHShield = false;

	char szUseXTrap[ 2 ] = {0,};
	GetPrivateProfileString("ENVIRONMENT", "USE_XTRAP", SERVER_CONFIG_DEFAULT_USE_XTRAP, szUseXTrap, 2, SERVER_CONFIG_FILENAME);
	ASSERT( 0 != strlen(szUseXTrap) );
	if( 0 == _stricmp("1", szUseXTrap) )
		m_bIsUseXTrap = true;
	else
		m_bIsUseXTrap = false;

	char szUseEvent[ 2 ] = {0,};
	GetPrivateProfileString("ENVIRONMENT", "USE_EVENT", SERVER_CONFIG_DEFAULT_USE_EVENT, szUseEvent, 2, SERVER_CONFIG_FILENAME);
	ASSERT( 0 != strlen(szUseEvent) );
	if( 0 == _stricmp("1", szUseEvent) )
		m_bIsUseEvent = true;
	else
		m_bIsUseEvent = false;

	char szUseFileCrc[ 2 ] = {0,};
	GetPrivateProfileString("ENVIRONMENT", "USE_FILECRC", SERVER_CONFIG_DEFAULT_USE_FILECRC, szUseFileCrc, 2, SERVER_CONFIG_FILENAME);
	ASSERT( 0 != strlen(szUseFileCrc) );
	if( 0 == _stricmp("1", szUseFileCrc) )
		m_bIsUseFileCrc = true;
	else
		m_bIsUseFileCrc = false;


	if( m_bIsUseHShield && m_bIsUseXTrap )
	{
		ASSERT( 0 && "hackshield와 x-trap은 같이 사용할 수 없다." );
		mlog( "server.ini - HackShield and XTrap is duplicated\n" );
		return false;
	}

	m_bIsComplete = true;
	return m_bIsComplete;
}
void MMatchConfig::Destroy()
{

}

void MMatchConfig::AddFreeLoginIP(const char* pszIP)
{
	m_FreeLoginIPList.push_back(pszIP);
}


void MMatchConfig::AddDebugLoginIP( const char* szIP )
{
	m_DebugLoginIPList.push_back( szIP );
}


bool MMatchConfig::CheckFreeLoginIPList(const char* pszIP)
{
	list<string>::iterator end = m_FreeLoginIPList.end();
	for (list<string>::iterator i = m_FreeLoginIPList.begin(); i!= end; i++) {
		const char* pszFreeIP = (*i).c_str();
		if (strncmp(pszIP, pszFreeIP, strlen(pszFreeIP)) == 0) {
			return true;
		}
	}
	return false;
}


bool MMatchConfig::IsDebugLoginIPList( const char* pszIP )
{
	list< string >::iterator it, end;
	end = m_DebugLoginIPList.end();
	for( it = m_DebugLoginIPList.begin(); it != end; ++it )
	{
		const char* pszFreeIP = (*it).c_str();
		if (strncmp(pszIP, pszFreeIP, strlen(pszFreeIP)) == 0) {
			return true;
		}
	}
	return false;
}


void MMatchConfig::ReadEnableMaps()
{
	char szEnableMap[512];
	GetPrivateProfileString("SERVER", "EnableMap", "", szEnableMap, 512, SERVER_CONFIG_FILENAME);

	char seps[] = ";";
	char *token;

	int nMapCount = 0;

	char *context = nullptr;

	token = strtok_s(szEnableMap, seps, &context);
	while( token != NULL )
	{
		char szInputMapName[256] = "";
		TrimStr(token, szInputMapName, sizeof(szInputMapName));

		int nMapIndex = -1;
		for (int i = 0; i < MMATCH_MAP_MAX; i++)
		{
			if (!_stricmp(szInputMapName, g_MapDesc[i].szMapName))
			{
				nMapIndex = i;
				break;
			}
		}
		if (nMapIndex != -1)
		{
			nMapCount++;
			m_EnableMaps.insert(set<int>::value_type(nMapIndex));
		}

		// Get next token
		token = strtok_s(NULL, seps, &context);
   }

	if (nMapCount <= 0)
	{
		for (int i = 0; i < MMATCH_MAP_MAX; i++) m_EnableMaps.insert(set<int>::value_type(i));
		m_bRestrictionMap = false;
	}
	else
	{
		m_bRestrictionMap = true;
	}


}
void MMatchConfig::TrimStr(const char* szSrcStr, char* outStr, int maxlen)
{
	char szInputMapName[256] = "";

	// 왼쪽 공백제거
	int nSrcStrLen = (int)strlen(szSrcStr);
	for (int i = 0; i < nSrcStrLen; i++)
	{
		if (!isspace(szSrcStr[i]))
		{
			strcpy_safe(szInputMapName, &szSrcStr[i]);
			break;
		}
	}
	// 오른쪽 공백제거
	int nLen = (int)strlen(szInputMapName);
	for (int i = nLen-1; i >= 0; i--)
	{
		if (isspace(szInputMapName[i]))
		{
			szInputMapName[i] = '\0';
		}
		else
		{
			break;
		}
	}

	strcpy_safe(outStr, maxlen, szInputMapName);
}

void MMatchConfig::Clear()
{
	memset(m_szDB_DNS, 0, 64 );
	memset(m_szDB_UserName, 0, 64 );
	memset(m_szDB_Password, 0, 64 );		///< DB Password

	m_nMaxUser = 0;					///< 최대 접속자
	m_nServerID = 0;
	memset(m_szServerName, 0, 256 );		///< 서버이름

	// m_nServerMode;				///< 서버모드
	m_bRestrictionMap = false;			///< 맵제한이 있는지 여부 - default : false
	m_EnableMaps.clear();				///< 맵제한이 있을경우 가능한 맵
	m_FreeLoginIPList.clear();			///< 접속인원 무시 IP
	m_bCheckPremiumIP = true;			///< 프리미엄 IP 체크

	// enabled 씨리즈 - ini에서 관리하지 않는다.
	m_bEnabledCreateLadderGame = false;	///< 클랜전 생성가능한지 여부

	// -- 일본 넷마블 전용
	memset( m_NJ_szDBAgentIP, 0, 64 );
	m_NJ_nDBAgentPort = 0;
	m_NJ_nGameCode = 0;

	// filter 사용 설정.
	m_bUseFilter = false;				/// 필터 사용을 설정함(0:사용않함. 1:사용.)
	m_bAcceptInvalidIP = true;			/// 리스트에 없는 IP허용 여부.(0:허용하지 않음. 1:허용.)
}