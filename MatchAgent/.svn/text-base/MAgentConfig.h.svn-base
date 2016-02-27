#ifndef MAGENTCONFIG_H
#define MAGENTCONFIG_H


class MAgentConfig
{
private:
	static MAgentConfig*	m_pInstance;

protected:
	char	m_szName[128];
	int		m_nMaxClient;

	char	m_szIP[64];
	int		m_nTCPPort;
	int		m_nUDPPort;

	char	m_szMatchServerIP[64];
	int		m_nMatchServerTCPPort;

public:
	char* GetName()				{ return m_szName; }
	int GetMaxClient()			{ return m_nMaxClient; }

	char* GetIP()				{ return m_szIP; }
	int GetTCPPort()			{ return m_nTCPPort; }
	int GetUDPPort()			{ return m_nUDPPort; }

	char* GetMatchServerIP()	{ return m_szMatchServerIP; }
	int GetMatchServerTCPPort()	{ return m_nMatchServerTCPPort; }

protected:
	MAgentConfig();

public:
	static MAgentConfig* GetInstance();
	void ReleaseInstance();
	virtual ~MAgentConfig();	// 안불리지 -_-
	bool LoadFromXML(const char* pszFileName);
};


#define ATOK_AGENTCONFIG		"AGENTCONFIG"
#define ATOK_NAME				"NAME"
#define ATOK_MAXCLIENT			"MAXCLIENT"
#define ATOK_ADDRESS			"ADDRESS"
#define ATOK_MATCHSERVERADDRESS	"MATCHSERVERADDRESS"
#define ATOK_ATTR_IP			"ip"
#define ATOK_ATTR_TCPPORT		"tcpport"
#define ATOK_ATTR_UDPPORT		"udpport"


#endif