#include "stdafx.h"
#include <winsock2.h>
#include "MInet.h"
#include <windows.h> 
#include <wininet.h> 
#include <stdio.h> 


bool MHTTP_Get(const char *szUrl, char *out, int nOutLen)
{
    HINTERNET h, h2; 
    unsigned long len, i = 0; 

    h = InternetOpen("Microsoft Internet Explorer", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL); 
    h2 = InternetOpenUrl(h, szUrl, NULL, 0, INTERNET_FLAG_DONT_CACHE|INTERNET_FLAG_NO_CACHE_WRITE, 0); 

    if (h2) 
	{
        do 
		{
			InternetReadFile(h2, out+i, nOutLen-1-i, &len); 
            i += len; 
        } while(len > 0 && i<len-1); 
        out[i] = '\0'; 

        InternetCloseHandle(h2); 
		return true;
    } 

    InternetCloseHandle(h); 
	return false;
}




//////////////////////////////////////////////////////////////////////////////
// MHttpThread ///////////////////////////////////////////////////////////////
MHttpThread::MHttpThread()
{
	m_bActive = false;
	m_pCallbackContext = NULL;
	m_fnRecvCallback = NULL;
}

MHttpThread::~MHttpThread()
{


}

void MHttpThread::Run()
{
	m_bActive = true;

	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
	WORD wEventIndex = 0;

	DWORD result;

	EventArray[wEventIndex++] = m_QueryEvent.GetEvent();
	EventArray[wEventIndex++] = m_KillEvent.GetEvent();

	bool bEnd = false;
	while (!bEnd)
	{
		result = WSAWaitForMultipleEvents(wEventIndex, EventArray, FALSE, WSA_INFINITE, FALSE);
		if ((result == WSA_WAIT_FAILED) || (result == WSA_WAIT_TIMEOUT)) continue;

		switch (result)
		{
		case WAIT_OBJECT_0:			//  Query Event
			{
				FlushQuery();
				m_QueryEvent.ResetEvent();
			}
			break;

		case WAIT_OBJECT_0 + 1:		// Kill Event
			{
				bEnd = true;
				m_KillEvent.ResetEvent();
			}
			break;

		default:
			{
				OutputDebugString("<SOCKETTHREAD_ERROR> exceptional case </SOCKETTHREAD_ERROR>\n");
				bEnd = true;
			}
			break;
		}	// switch

	}	// while

	m_bActive = false;

	ExitThread(0);
}

void MHttpThread::Create()
{
	InitializeCriticalSection(&m_csQueryLock);

	MThread::Create();

	memset(m_szRecvBuf, 0, sizeof(m_szRecvBuf));
}

void MHttpThread::Destroy()
{
	m_QueryList.clear();
	m_KillEvent.SetEvent(); 

	MThread::Destroy();

	DeleteCriticalSection(&m_csQueryLock);
}

void MHttpThread::Query(char* szQuery)
{
	if (!m_bActive) return;

	LockQuery();
	m_TempQueryList.push_back(string(szQuery));
	UnlockQuery();

	m_QueryEvent.SetEvent();
}

void MHttpThread::FlushQuery()
{
	if (!m_bActive) return;

	LockQuery();
	while (!m_TempQueryList.empty())
	{
		list<string>::iterator itor = m_TempQueryList.begin();
		m_QueryList.push_back(*itor);
		m_TempQueryList.erase(itor);
	}
	UnlockQuery();


	while(!m_QueryList.empty())
	{
		list<string>::iterator itor = m_QueryList.begin();
		const char* szQuery = (*itor).c_str();

		MHTTP_Get(szQuery, m_szRecvBuf, HTTP_BUFSIZE);

		if (m_fnRecvCallback)
		{
			m_fnRecvCallback(m_pCallbackContext, m_szRecvBuf);
		}

		m_QueryList.erase(m_QueryList.begin());
	}
}
void MHttpThread::ClearQuery()
{
	LockQuery();
	m_TempQueryList.clear();
	m_QueryList.clear();
	UnlockQuery();
}
//////////////////////////////////////////////////////////////////////////////
// MHttp /////////////////////////////////////////////////////////////////////
MHttp::MHttp()
{

}

MHttp::~MHttp()
{
	Destroy();
}


bool MHttp::Create()
{
	m_HttpThread.Create();

	return true;
}

void MHttp::Destroy()
{
	m_HttpThread.Destroy();
}


void MHttp::Query(char* szQuery)
{
//	char szRealQuery[1024];
//	strcpy_safe(szRealQuery, SERVER_URL);
//	strcat(szRealQuery, szQuery);

	m_HttpThread.Query(szQuery);
}

void MHttp::Clear()
{
	m_HttpThread.m_pCallbackContext = NULL;
	m_HttpThread.m_fnRecvCallback = NULL;

	m_HttpThread.ClearQuery();

}

void MHttp::ReplaceBlank(char* szOut, char* szSrc)
{
	int nLen = (int)strlen(szSrc);
	int nOutIdx = 0;
	for(int i = 0; i < nLen; i++)
	{
		if (szSrc[i] == ' ')
		{
			szOut[nOutIdx++] = '%';
			szOut[nOutIdx++] = '2';
			szOut[nOutIdx++] = '0';
		}
		else
		{
			szOut[nOutIdx++] = szSrc[i];
		}
	}
	szOut[nOutIdx] = '\0';
}



