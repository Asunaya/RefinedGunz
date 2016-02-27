#include "MSafeThread.h"


// MSafeThread class /////////////////////////////////////////////////////////////////////
MSafeThread::MSafeThread() 
{ 
	m_hThread = NULL;
	m_idThread = 0;
}

MSafeThread::~MSafeThread()
{
	if (m_hThread)
		Destroy();
}

void MSafeThread::Create()
{
	m_hThread = CreateThread(NULL, 0, ThreadProc, this, 0, &m_idThread); 
}

void MSafeThread::Destroy()
{
	if (m_hThread) {
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

DWORD WINAPI MSafeThread::ThreadProc(LPVOID pParam)
{
	OutputDebugString("<MSAFETHREAD_BEGIN>\n");
	MSafeThread* pThread = (MSafeThread*)pParam;

	pThread->OnRun();

	OutputDebugString("</MSAFETHREAD_END>\n");
	ExitThread(0);
	return (0);
}
