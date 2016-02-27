#ifndef _ZNETMARBLE_H
#define _ZNETMARBLE_H

#include "ZBaseAuthInfo.h"

#define NM_BUFSIZE	2048

class ZNetmarbleAuthInfo : public ZBaseAuthInfo
{
private:
	char			m_AuthCookie[NM_BUFSIZE];
	char			m_DataCookie[NM_BUFSIZE];
	char			m_SpareParam[NM_BUFSIZE];
protected:

public:
	ZNetmarbleAuthInfo();
	virtual ~ZNetmarbleAuthInfo();

	const char* GetAuthCookie()				{ return m_AuthCookie;			}
	void SetAuthCookie(const char* pszVal)	{ strcpy(m_AuthCookie, pszVal);	}
	const char* GetDataCookie()				{ return m_DataCookie;			}
	void SetDataCookie(const char* pszVal)	{ strcpy(m_DataCookie, pszVal);	}
	const char* GetSpareParam()				{ return m_SpareParam;			}
	void SetSpareParam(const char* pszVal)	{ strcpy(m_SpareParam, pszVal);	}
};

#ifdef LOCALE_KOREA

bool NetmarbleParseArguments(const char* pszArgs);

#endif


#endif