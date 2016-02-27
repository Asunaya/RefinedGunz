#pragma once


#include <windows.h>


class MSafeThread {
protected:
	HANDLE		m_hThread;
	DWORD		m_idThread;

	virtual void OnRun() = 0;

public:
	MSafeThread();
	~MSafeThread();
	virtual void Create();
	virtual void Destroy();
	static DWORD WINAPI ThreadProc(LPVOID lpParameter);
};
