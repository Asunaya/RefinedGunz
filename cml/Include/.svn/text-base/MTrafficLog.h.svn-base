#ifndef _MTRAFFICLOG_H
#define _MTRAFFICLOG_H

//#pragma once


#include <Mmsystem.h>
// #pragma comment(lib,"winmm.lib")


class MTrafficNode {
protected:
	unsigned long	m_nTimeStamp;
	unsigned long	m_nTrafficSum;

public:
	unsigned long GetTimeStamp()	{ return m_nTimeStamp; }
	unsigned long GetTrafficSum()	{ return m_nTrafficSum; }
	void Record(unsigned long nTime, unsigned long nTrafficSum)	{ m_nTimeStamp = nTime; m_nTrafficSum = nTrafficSum; }
};


class MTrafficLog {
protected:
	int				m_nSamplingInterval;
	unsigned char	m_nTrafficLogCount;
	MTrafficNode*	m_pTrafficLogs;
	int				m_nCursor;			// 마지막으로 기록된곳 가리킴

public:
	#define DEFAILT_TRAFFICLOG_SAMPLING_INTERVAL	100
	#define DEFAULT_TRAFFICLOG_COUNT				10
	#define MAX_TRAFFICLOG_COUNT					256

	MTrafficLog() {
		m_nTrafficLogCount = 0;
		m_pTrafficLogs = NULL;
		m_nCursor = 0;

		SetSamplingInterval(DEFAILT_TRAFFICLOG_SAMPLING_INTERVAL);	// 0.1 sec
		SetLogSize(DEFAULT_TRAFFICLOG_COUNT);		
	}
	virtual ~MTrafficLog() {
		if (m_pTrafficLogs) {
			m_nTrafficLogCount = 0;
			delete [] m_pTrafficLogs;
			m_pTrafficLogs = NULL;		
		}
	}
	void SetSamplingInterval(int nInterval) { m_nSamplingInterval = nInterval; }
	void SetLogSize(int nSize) {
		if (m_pTrafficLogs)
			delete [] m_pTrafficLogs;

		m_nTrafficLogCount = DEFAULT_TRAFFICLOG_COUNT;
		m_pTrafficLogs = new MTrafficNode[m_nTrafficLogCount];
		ZeroMemory(m_pTrafficLogs, sizeof(MTrafficNode) * m_nTrafficLogCount);
	}
	void Record(unsigned long nTrafficSum) {	// Network 전송량의 누적치를 Arg로 사용
		unsigned long nTime = timeGetTime();
		if (nTime - m_pTrafficLogs[m_nCursor].GetTimeStamp() > (unsigned)m_nSamplingInterval) {
			if (++m_nCursor >= m_nTrafficLogCount)
				m_nCursor = 0;
			m_pTrafficLogs[m_nCursor].Record(nTime, nTrafficSum);
		}
	}
	int GetTrafficSpeed() {
		unsigned long nTimeStampCurrent = m_pTrafficLogs[m_nCursor].GetTimeStamp();
		unsigned long nTrafficCurrent = m_pTrafficLogs[m_nCursor].GetTrafficSum();

		int nLastCursor = m_nCursor + 1;
		if (nLastCursor >= m_nTrafficLogCount)
			nLastCursor = 0;
		unsigned long nTimeStampLast = m_pTrafficLogs[nLastCursor].GetTimeStamp();
		unsigned long nTrafficLast = m_pTrafficLogs[nLastCursor].GetTrafficSum();

		unsigned long nElapseTime = nTimeStampCurrent - nTimeStampLast;

		if ( (nTimeStampLast==0) || (nElapseTime==0) )
			return 0;

		return int( (nTrafficCurrent - nTrafficLast) / (nElapseTime/1000.0f) + 0.5f );
	}
};

#endif