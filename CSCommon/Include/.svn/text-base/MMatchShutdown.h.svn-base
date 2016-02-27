#pragma once


#include <vector>
using namespace std;


class MShutdownNotify {
protected:
	unsigned short	m_nDelay;
	char			m_szMessage[128];	
public:
	MShutdownNotify(unsigned short nDelay, const char* pszMsg) {
		// 공지를 저장할때 문자열 길이를 검사함. 
		// 만약 최대 길이를 넘어서면 복사할수 있는 길이만큼만 복사. - by 추교성.
		memset( m_szMessage, 0, 128 );
		m_nDelay = nDelay;
		const size_t len = strlen( pszMsg );
		ASSERT( (len < 128) && "문자열의 길이가 128보다 작아야 함." );
		strncpy( m_szMessage, pszMsg,  len < 128 ? len : 127 );
	}
	virtual ~MShutdownNotify() {}

	unsigned short GetDelay()	{ return m_nDelay; }
	char* GetString()			{ return m_szMessage; }
};

class MMatchShutdown {
protected:
	vector<MShutdownNotify*>	m_ShutdownNotifyArray;

	bool						m_bShutdown;
	unsigned short				m_nProgressIndex;
	unsigned long				m_nTimeLastProgress;

	unsigned short GetProgressIndex()		{ return m_nProgressIndex; }
	unsigned long GetTimeLastProgress()		{ return m_nTimeLastProgress; }
	void MMatchShutdown::SetProgress(int nIndex, unsigned long nClock);

public:
	MMatchShutdown() { m_bShutdown = false; }
	virtual ~MMatchShutdown();

	bool LoadXML_ShutdownNotify(const char* pszFileName);

	void Start(unsigned long nClock);
	void Notify(int nIndex);
	void Terminate();

	bool IsShutdown()	{ return m_bShutdown; }

	void OnRun(unsigned long nClock);
};
