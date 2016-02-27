#ifndef _ZBASEAUTHINFO_H
#define _ZBASEAUTHINFO_H


#define AUTHINFO_BUFSIZE	2048

class ZBaseAuthInfo
{
protected:
	bool					m_bTeenMode;
	char					m_ServerIP[AUTHINFO_BUFSIZE];
	unsigned int			m_nServerPort;
	char					m_CpCookie[AUTHINFO_BUFSIZE];
public:
	ZBaseAuthInfo();
	virtual ~ZBaseAuthInfo() {}

	bool GetTeenMode()						{ return m_bTeenMode;			}
	const char* GetServerIP()				{ return m_ServerIP;			}
	unsigned int GetServerPort()			{ return m_nServerPort;			}
	const char* GetCpCookie()				{ return m_CpCookie;			}

	void SetTeenMode(bool bMode)			{ m_bTeenMode = bMode;			}
	void SetServerIP(const char* pszIP)		{ strcpy(m_ServerIP, pszIP);	}
	void SetServerPort(unsigned int nPort)	{ m_nServerPort = nPort;		}
	void SetCpCookie(const char* pszVal)	{ strcpy(m_CpCookie, pszVal);	}
};




#endif