#pragma once


#include "MMatchAuth.h"
#include <string.h>


class MBMatchAuthInfo : public MMatchAuthInfo {
protected:
	char	m_szUserID[MAUTHINFO_BUFLEN];
	char	m_szUniqueID[MAUTHINFO_BUFLEN];
	char	m_szCertificate[MAUTHINFO_BUFLEN];
	char	m_szName[MAUTHINFO_BUFLEN];
	int		m_nAge;
	int		m_nSex;

public:
	MBMatchAuthInfo()
	{
		m_szUserID[0] = 0;
		m_szUniqueID[0] = 0;
		m_szCertificate[0] = 0;
		m_szName[0] = 0;
		m_nAge = 1;
		m_nSex = 1;
	}

	virtual ~MBMatchAuthInfo()	{}


	virtual const char* GetUserID()			{ return m_szUserID; }
	virtual const char* GetUniqueID()		{ return m_szUniqueID; }
	virtual const char* GetCertificate()	{ return m_szCertificate; }
	virtual const char* GetName()			{ return m_szName; }
	virtual int GetAge()					{ return m_nAge; }
	virtual int GetSex()					{ return m_nSex; }

	void SetUserID(const char* pszVal)		{ strcpy(m_szUserID, pszVal); }
	void SetUniqueID(const char* pszVal)	{ strcpy(m_szUniqueID, pszVal); }
	void SetCertificate(const char* pszVal)	{ strcpy(m_szCertificate, pszVal); }
	void SetName(const char* pszVal)		{ strcpy(m_szName, pszVal); }
	void SetAge(int nAge)					{ m_nAge = nAge; }
	void SetSex(int nSex)					{ m_nSex = nSex; }
};

class MBMatchAuthBuilder : public MMatchAuthBuilder {
public:
	MBMatchAuthBuilder()				{}
	virtual ~MBMatchAuthBuilder()		{}

	virtual bool ParseAuthInfo(const char* pszData, MMatchAuthInfo** ppoutAutoInfo);
};
