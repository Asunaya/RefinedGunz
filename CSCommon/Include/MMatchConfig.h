#ifndef _MMATCHCONFIG_H
#define _MMATCHCONFIG_H

#include <string>
#include <list>
#include <set>
using namespace std;

#include "MMatchMap.h"
#include "MMatchGlobal.h"


class MMatchConfig
{
private:
	char				m_szDB_DNS[64];				///< DB DNS
	char				m_szDB_UserName[64];		///< DB Username
	char				m_szDB_Password[64];		///< DB Password

	int					m_nMaxUser;					///< 최대 접속자
	int					m_nServerID;
	char				m_szServerName[256];		///< 서버이름

	MMatchServerMode	m_nServerMode;				///< 서버모드
	bool				m_bRestrictionMap;			///< 맵제한이 있는지 여부 - default : false
	set<int>			m_EnableMaps;				///< 맵제한이 있을경우 가능한 맵
	list<string>		m_FreeLoginIPList;			///< 접속인원 무시 IP
	bool				m_bCheckPremiumIP;			///< 프리미엄 IP 체크
	string				m_strCountry;				
	string				m_strLanguage;

	// enabled 씨리즈 - ini에서 관리하지 않는다.
	bool				m_bEnabledCreateLadderGame;	///< 클랜전 생성가능한지 여부

	// -- 일본 넷마블 전용
	char				m_NJ_szDBAgentIP[64];
	int					m_NJ_nDBAgentPort;
	int					m_NJ_nGameCode;

	// filter 사용 설정.
	bool				m_bUseFilter;				/// 필터 사용을 설정함(0:사용않함. 1:사용.)
	bool				m_bAcceptInvalidIP;			/// 리스트에 없는 IP허용 여부.(0:허용하지 않음. 1:허용.)

	// evironment.
	bool				m_bIsUseHShield;
	bool				m_bIsUseXTrap;
	bool				m_bIsUseEvent;
	bool				m_bIsUseFileCrc;

	// debug.
	list<string>		m_DebugLoginIPList;			///< Debug용 IP.
	bool				m_bIsDebugServer;

	// keeper ip.
	string				m_strKeeperIP;				/// Keeper와 server와의 통신에서 키퍼의 요청인지 검사하기 위해서.
	
	std::string GameDirectory = "";
	bool bIsMasterServer = true;

	bool				m_bIsComplete;

private:
	bool GetPrivateProfileBool(const char* szAppName, const char* szKeyName, 
							   bool bDefault, const char* szFileName);
	void AddFreeLoginIP(const char* szIP);
	void AddDebugLoginIP( const char* szIP );
	void ReadEnableMaps();
	void TrimStr(const char* szSrcStr, char* outStr, int maxlen);

public:
	MMatchConfig();
	virtual ~MMatchConfig();
	static MMatchConfig* GetInstance();
	bool Create();
	void Destroy();
	void Clear();

	// get
	const char* GetDB_DNS()							{ return m_szDB_DNS; }
	const char* GetDB_UserName()					{ return m_szDB_UserName; }
	const char* GetDB_Password()					{ return m_szDB_Password; }
	const int GetMaxUser()							{ return m_nMaxUser; }
	const int GetServerID()							{ return m_nServerID; }
	const char* GetServerName()						{ return m_szServerName; }
	const MMatchServerMode		GetServerMode()		{ return m_nServerMode; }
	bool IsResMap()									{ return m_bRestrictionMap; }	// 맵제한이 있는지 여부
	bool IsEnableMap(const MMATCH_MAP nMap)										// 플레이가능한 맵인지 여부
	{
		if (!m_bRestrictionMap) return true;
		if (m_EnableMaps.find(nMap) != m_EnableMaps.end()) return true;
		return false;
	}
	const bool IsDebugServer()							{ return m_bIsDebugServer; }
	bool CheckFreeLoginIPList(const char* pszIP);
	bool IsDebugLoginIPList( const char* pszIP );
	bool CheckPremiumIP()							{ return m_bCheckPremiumIP; }

	bool IsEnabledCreateLadderGame()				{ return m_bEnabledCreateLadderGame; }
	void SetEnabledCreateLadderGame(bool bVal)		{ m_bEnabledCreateLadderGame = bVal; }

	const char* GetNJDBAgentIP()					{ return m_NJ_szDBAgentIP; }
	int GetNJDBAgentPort()							{ return m_NJ_nDBAgentPort; }
	int GetNJDBAgentGameCode()						{ return m_NJ_nGameCode; }

	const bool IsUseFilter() const						{ return m_bUseFilter; }
	const bool IsAcceptInvalidIP() const				{ return m_bAcceptInvalidIP; }
	void SetUseFilterState( const bool bUse )			{ m_bUseFilter = bUse; }
	void SetAcceptInvalidIPState( const bool bAccept )	{ m_bAcceptInvalidIP = bAccept; }

	const bool IsUseHShield() const	{ return m_bIsUseHShield; }
	const bool IsUseXTrap() const	{ return m_bIsUseXTrap; }
	const bool IsUseEvent() const	{ return m_bIsUseEvent; }
	const bool IsUseFileCrc() const { return m_bIsUseFileCrc; }

	bool IsKeeperIP( const string& strIP )				{ return m_strKeeperIP == strIP; }

	const string& GetKeeperIP() { return m_strKeeperIP; }

	const string& GetCountry()	{ return m_strCountry; }
	const string& GetLanguage() { return m_strLanguage; }

	const bool IsComplete() { return m_bIsComplete; }

	const char* GetGameDirectory() const { return GameDirectory.c_str(); }

	bool IsMasterServer() const { return bIsMasterServer; }
};

inline MMatchConfig* MGetServerConfig() { return MMatchConfig::GetInstance(); }

inline bool QuestTestServer() { return (MGetServerConfig()->GetServerMode() == MSM_TEST); }


#define SERVER_CONFIG_FILENAME			"./server.ini"


#define SERVER_CONFIG_SERVERMODE_NORMAL			"normal"
#define SERVER_CONFIG_SERVERMODE_CLAN			"clan"
#define SERVER_CONFIG_SERVERMODE_LADDER			"ladder"
#define SERVER_CONFIG_SERVERMODE_EVENT			"event"
#define SERVER_CONFIG_SERVERMODE_TEST			"test"

#define SERVER_CONFIG_DEFAULT_NJ_DBAGENT_IP			"210.174.197.180"
#define SERVER_CONFIG_DEFAULT_NJ_DBAGENT_PORT		7500
#define SERVER_CONFIG_DEFAULT_NJ_DBAGENT_GAMECODE	1013

#define SERVER_CONFIG_DEFAULT_USE_HSHIELD	"0"
#define SERVER_CONFIG_DEFAULT_USE_XTRAP		"0"
#define SERVER_CONFIG_DEFAULT_USE_EVENT		"1"
#define SERVER_CONFIG_DEFAULT_USE_FILECRC	"0"

#define SERVER_CONFIG_DEBUG_DEFAULT			"0"

#endif
