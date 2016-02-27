#ifndef _MSYNC_H
#define _MSYNC_H

//#pragma once


class MCriticalSection {
	CRITICAL_SECTION	m_cs;

public:
	MCriticalSection() { InitializeCriticalSection(&m_cs);	}
	~MCriticalSection() { DeleteCriticalSection(&m_cs); }
	void Lock() { EnterCriticalSection(&m_cs); }
	void Unlock() { LeaveCriticalSection(&m_cs); }
};


class MSignalEvent {
	HANDLE		m_hEvent;

public:
	MSignalEvent() { m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL); }
	~MSignalEvent() { if (m_hEvent) { CloseHandle(m_hEvent); m_hEvent = NULL; } }
	HANDLE GetEvent() { return m_hEvent; }
	BOOL SetEvent() { return ::SetEvent(m_hEvent); }
	BOOL ResetEvent() { return ::ResetEvent(m_hEvent); }
};


// 어플리케이션이 동시에 두개뜨는것을 막거나 기다리게 도와주는 클래스
class MSingleRunController {
protected:
	char	m_szAppName[_MAX_DIR];
	HANDLE	m_Mutex;
public:
	//Usage : 	if (g_SingleRunController.Create(true) == false) return FALSE;
	MSingleRunController(const char* pszAppName)	{ 
		m_Mutex = NULL;
		strcpy(m_szAppName, pszAppName); 
	}
	virtual ~MSingleRunController()	{ 
		Destroy();
	}	
	bool Create(bool bWaitMode=false) {	// bMode=true(실행가능때까지 블럭대기), bMode==false(늦게 실행된 인스턴스 바로종료)
		if (bWaitMode == true) {
			int tmTotalWait = 0;
			while(true) {
				m_Mutex = CreateMutex(NULL, TRUE, m_szAppName);
				if (GetLastError() == ERROR_ALREADY_EXISTS)	{
					CloseHandle(m_Mutex);

					Sleep(100);
					tmTotalWait += 100;
					if (tmTotalWait >= 5*60*1000) // 5분이상은 강제종료
						return false;
				} else {
					return true;
				}
			}
		} else {
			m_Mutex = CreateMutex(NULL, TRUE, m_szAppName);
			if (GetLastError() == ERROR_ALREADY_EXISTS)
				return false;
			else
				return true;
		}
	}
	void Destroy() {
		if (m_Mutex) {
			ReleaseMutex(m_Mutex);	
			m_Mutex = NULL;
		}
	}
};

#endif