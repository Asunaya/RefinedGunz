#include "stdafx.h"
#include "ZTimer.h"
#include <Windows.h>

class ZTimerEvent
{
private:
	unsigned long int			m_nUpdatedTime;				///< 시간계산하기 위해 내부에서만 사용하는 변수
	unsigned long int			m_nElapse;					///< 사용자가 설정한 시간(1000 - 1초)
	bool						m_bOnce;					///< true로 설정되면 한번만 타이머 이벤트가 발생한다.
	void*						m_pParam;					///< Event Callback의 파라메터
public:
	ZTimerEvent() { m_nUpdatedTime = 0; m_nElapse = 0; m_bOnce = false; m_fnTimerEventCallBack = NULL; m_pParam=NULL; }
	bool UpdateTick(unsigned long int nDelta)
	{
		if (m_nElapse<0) return false;

		m_nUpdatedTime += nDelta;

		if (m_nUpdatedTime >= m_nElapse)
		{
			if (m_fnTimerEventCallBack)
			{
				m_fnTimerEventCallBack(m_pParam);
			}

			if (m_bOnce) return true;
		}

		return false;
	}
	void SetTimer(unsigned long int nElapse, ZGameTimerEventCallback* fnTimerEventCallback, void* pParam, bool bTimerOnce)
	{
		m_nElapse = nElapse;
		m_fnTimerEventCallBack = fnTimerEventCallback;
		m_pParam = pParam;
		m_bOnce = bTimerOnce;
	}

	ZGameTimerEventCallback*	m_fnTimerEventCallBack;		///< Event Callback 포인터
};


/////////////////////////////////////////////////////////////////////////////////////////////////////

ZTimer::ZTimer()
{
	m_bInitialized = false;
	m_nNowTime = 0;
	m_nLastTime = 0;
}

ZTimer::~ZTimer()
{
	for (list<ZTimerEvent*>::iterator itor = m_EventList.begin(); itor != m_EventList.end(); ++itor)
	{
		ZTimerEvent* pEvent = (*itor);
		delete pEvent;
	}

	m_EventList.clear();
}

void ZTimer::ResetFrame()
{
	m_bInitialized=false;
}

double ZTimer::UpdateFrame()
{
	static BOOL bUsingQPF=FALSE;
	static LONGLONG llQPFTicksPerSec  = 0;
	static LONGLONG llLastElapsedTime = 0;
	static DWORD thistime,lasttime,elapsed;

	LARGE_INTEGER qwTime;

	if(!m_bInitialized)
	{
		m_bInitialized = true;
		LARGE_INTEGER qwTicksPerSec;
		bUsingQPF = QueryPerformanceFrequency( &qwTicksPerSec );
		if( bUsingQPF )
		{
			llQPFTicksPerSec = qwTicksPerSec.QuadPart;

			QueryPerformanceCounter( &qwTime );

			llLastElapsedTime = qwTime.QuadPart;
		}
		else
		{
			lasttime = GetGlobalTimeMS();
		}
	}

	double fElapsed;

	if( bUsingQPF )
	{
		QueryPerformanceCounter( &qwTime );
		QueryPerformanceFrequency((PLARGE_INTEGER)&llQPFTicksPerSec);

		fElapsed = double( qwTime.QuadPart - llLastElapsedTime ) / llQPFTicksPerSec;
		llLastElapsedTime = qwTime.QuadPart;
	}
	else
	{
		thistime = GetGlobalTimeMS();
		elapsed = thistime - lasttime;
		lasttime = thistime;

		fElapsed=.001f*(float)elapsed;
	}

	
	UpdateEvents();			// 타이머 이벤트들 업데이트

	return fElapsed;
}

void ZTimer::UpdateEvents()
{
	m_nNowTime = GetGlobalTimeMS();
	unsigned long int nDeltaTime = m_nNowTime - m_nLastTime;
	m_nLastTime = m_nNowTime;

	if (m_EventList.empty()) return;

	for (list<ZTimerEvent*>::iterator itor = m_EventList.begin(); itor != m_EventList.end(); )
	{
		ZTimerEvent* pEvent = (*itor);
		bool bDone = pEvent->UpdateTick(nDeltaTime);
		if (bDone)
		{
			delete pEvent;
			itor = m_EventList.erase(itor);
		}
		else
		{
			++itor;
		}
	}
}

void ZTimer::SetTimerEvent(unsigned long int nElapsedTime, ZGameTimerEventCallback* fnTimerEventCallback, void* pParam, bool bTimerOnce)
{
	ZTimerEvent* pNewTimerEvent = new ZTimerEvent;
	pNewTimerEvent->SetTimer(nElapsedTime, fnTimerEventCallback, pParam, bTimerOnce);
	m_EventList.push_back(pNewTimerEvent);
}

void ZTimer::ClearTimerEvent(ZGameTimerEventCallback* fnTimerEventCallback)
{
	if (fnTimerEventCallback == NULL) return;

	for (list<ZTimerEvent*>::iterator itor = m_EventList.begin(); itor != m_EventList.end(); )
	{
		ZTimerEvent* pEvent = (*itor);

		if (pEvent->m_fnTimerEventCallBack == fnTimerEventCallback)
		{
			delete pEvent;
			itor = m_EventList.erase(itor);
		}
		else
		{
			++itor;
		}
	}

}


