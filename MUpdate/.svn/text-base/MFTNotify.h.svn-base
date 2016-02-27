#pragma once


#include <list>
using namespace std;


//// ABSTRACT LISTENER ////////////////////////////////
class MFTListener {
public:
	virtual void Notify(MFTNotify* pNotify)=0;
};

//// NOTIFY ///////////////////////////////////////////
class MFTNotify {
protected:
	unsigned long	m_nContext;
	bool			m_bResult;
	unsigned long	m_nValue;
	char			m_szString[_MAX_DIR];

public:
	MFTNotify(unsigned long nContext, bool bResult, const char* pszString=NULL) {
		m_nContext = nContext;
		m_bResult = bResult;
		m_szString[0] = NULL;
		if (pszString && pszString[0] != NULL)
			strcpy(m_szString, pszString);
	}
	virtual ~MFTNotify() {}
	void SetValue(unsigned long nVal)	{ m_nValue = nVal; }
	unsigned long GetContext()			{ return m_nContext; }
	bool GetResult()					{ return m_bResult; }
	unsigned long GetValue()			{ return m_nValue; }
	char* GetString()					{ return m_szString; }

	virtual MFTNotify* Clone(void) {
		MFTNotify* pNotify = new MFTNotify(GetContext(), GetResult(), GetString());
		pNotify->SetValue(GetValue());
		return ( pNotify );
	}
};

class MFTNotifyProgress : public MFTNotify {
protected:
	unsigned long	m_nReadBytes;
	unsigned long	m_nTransSumBytes;
	unsigned long	m_nMaxBytes;

public:
	MFTNotifyProgress(unsigned long nContext, bool bResult, const char* pszString=NULL) : MFTNotify(nContext, bResult, pszString) {}
	virtual ~MFTNotifyProgress() {}

	void SetProgress(unsigned long nReadBytes, unsigned long nTransSumBytes, unsigned long nMaxBytes) {
		m_nReadBytes = nReadBytes;
		m_nTransSumBytes = nTransSumBytes;
		m_nMaxBytes = nMaxBytes;
	}
	void GetProgress(unsigned long* pReadBytes, unsigned long* pTransBytes, unsigned long* pMaxBytes) {
		*pReadBytes = m_nReadBytes;
		*pTransBytes = m_nTransSumBytes;
		*pMaxBytes = m_nMaxBytes;
	}
	virtual MFTNotify* Clone(void) {
		MFTNotifyProgress* pNotify = new MFTNotifyProgress(GetContext(), GetResult(), GetString());
		pNotify->SetProgress(m_nReadBytes, m_nTransSumBytes, m_nMaxBytes);
		return ( pNotify );
	}
};

//// NOTIFY LIST //////////////////////////////////////
typedef list<MFTNotify*>	MFTNotifyList;

