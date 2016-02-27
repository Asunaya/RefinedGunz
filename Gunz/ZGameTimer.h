#ifndef _ZGAMETIMER_H
#define _ZGAMETIMER_H

//#pragma once


// GT = LT+Offset (-> Offset = GT-LT)

class ZGameTimer {
protected:
	unsigned long	m_nTickTime;
	int				m_nGlobalOffset;
public:
	DWORD			m_dwUpdateCnt;
public:
	ZGameTimer()			{ m_nTickTime=0; m_nGlobalOffset=0; m_dwUpdateCnt=0;}
	virtual ~ZGameTimer()	{}

	void Reset()			{ m_nTickTime = timeGetTime(); m_dwUpdateCnt = 0;}
	void UpdateTick(unsigned long nTickTime) {
		m_nTickTime = nTickTime;
		m_dwUpdateCnt++;
	}
	void SetGlobalOffset(int nOffset) { 
		m_nGlobalOffset = nOffset; 
	}
	unsigned long GetGlobalTick() {
		return (m_nTickTime + m_nGlobalOffset);
	}
};

#endif