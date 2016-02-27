#ifndef _ZNETMARBLEJP_H
#define _ZNETMARBLEJP_H


#include "ZBaseAuthInfo.h"
#include <string>
using namespace std;

class ZNetmarbleJPAuthInfo : public ZBaseAuthInfo
{
private:
	string	m_strLoginID;
	string	m_strLoginPW;
	string	m_strExtra;
public:
	ZNetmarbleJPAuthInfo();
	virtual ~ZNetmarbleJPAuthInfo();

	const char* GetLoginID()					{ return m_strLoginID.c_str(); }
	const char* GetLoginPW()					{ return m_strLoginPW.c_str(); }
	const char* GetExtra()						{ return m_strExtra.c_str(); }
	void SetLoginID(const char* pszLoginID)		{ m_strLoginID = pszLoginID; }
	void SetLoginPW(const char* pszLoginPW)		{ m_strLoginPW = pszLoginPW; }
	void SetExtra(const char* pszExtra)			{ m_strExtra = pszExtra; }
};


#ifdef LOCALE_JAPAN

bool NetmarbleJPParseArguments(const char* pszArgs);

#endif


#endif