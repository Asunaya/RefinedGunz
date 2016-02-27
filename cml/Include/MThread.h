#ifndef MTHREAD_H
#define MTHREAD_H


#include <windows.h>
#include <MMsystem.h>


class MThread {
protected:
	HANDLE		m_hThread;
	DWORD		m_idThread;

public:
	MThread();
	virtual ~MThread();
	void Create();
	void Destroy();

	HANDLE GetThreadHandle()	{ return m_hThread; }
	DWORD GetThreadID()			{ return m_idThread; }

	static DWORD WINAPI ThreadProc(LPVOID lpParameter);

	virtual void OnCreate()		{}
	virtual void OnDestroy()	{}
	virtual void Run()			{}
};

class MTime {
	DWORD	m;
	DWORD	q;
	DWORD	a;
	DWORD	r;
	DWORD	seed;

public:
	MTime() {
		m=2147483647; q=127773; a=16807; r=2836;
		seed = timeGetTime();
	}
	unsigned long Random(void);
	int MakeNumber(int nFrom, int nTo);
	static void GetTime(struct timeval *t);
	static struct timeval TimeSub(struct timeval Src1, struct timeval Src2);
	static struct timeval TimeAdd(struct timeval Src1, struct timeval Src2);
};

#pragma comment(lib, "winmm.lib")

#endif
