#ifndef _MINET_H
#define _MINET_H

#include "MThread.h"
#include "MSync.h"

#pragma comment(lib, "Wininet.lib")

#include <list>
#include <string>
using namespace std;

/// HTTP프로토콜을 이용햐여 Url의 내용을 긁어온다.
bool MHTTP_Get(const char *szUrl, char *out, int nOutLen);

typedef bool(MHttpRecvCallback)(void* pCallbackContext, char* pRecvData);

#define HTTP_BUFSIZE 65536

/// HTTP 쓰레드
class MHttpThread : public MThread
{
private:
protected:
	MSignalEvent			m_QueryEvent;
	MSignalEvent			m_KillEvent;
	CRITICAL_SECTION		m_csQueryLock;
	bool					m_bActive;
	list<string>			m_QueryList;
	list<string>			m_TempQueryList;		// Temporary for Sync
	char					m_szRecvBuf[HTTP_BUFSIZE];
	void FlushQuery();
public:
	MHttpThread();
	virtual ~MHttpThread();
	virtual void Run();
	virtual void Create();
	virtual void Destroy();
	void ClearQuery();
	void Query(char* szQuery);
	void LockQuery() { EnterCriticalSection(&m_csQueryLock); }
	void UnlockQuery() { LeaveCriticalSection(&m_csQueryLock); }
	void*					m_pCallbackContext;
	MHttpRecvCallback*		m_fnRecvCallback;
};

/// HTTP 클래스
/// - 사용하려면 이 클래스를 상속받아 필요한 내용을 긁어오는 함수를 추가하면 된다.
class MHttp
{
private:
protected:
	MHttpThread		m_HttpThread;
	void ReplaceBlank(char* szOut, char* szSrc);
public:
	MHttp();
	virtual ~MHttp();
	
	bool Create();
	void Destroy();
	void Query(char* szQuery);

	void SetRecvCallback(void* pCallbackContext, MHttpRecvCallback pCallback)
	{
		m_HttpThread.m_pCallbackContext = pCallbackContext;
		m_HttpThread.m_fnRecvCallback = pCallback;
	}
	void Clear();
};



#endif