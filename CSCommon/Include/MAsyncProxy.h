#pragma once

/////////////////////////////////////////////////////////////////////////
// Purpose		: Blocking 작업을 여러 쓰레드로 나눠 Async 스럽게 굴린다.
// Last Update	: 2004-02-04 

#include <deque>
#include <algorithm>
#include <functional>
#include <mutex>


enum MASYNC_RESULT {
	MASYNC_RESULT_SUCCEED,
	MASYNC_RESULT_FAILED,
	MASYNC_RESULT_TIMEOUT
};


class MAsyncJob {
protected:
	int				m_nJobID;	// Job Type ID

	u64	m_nPostTime;
	u64 m_nFinishTime;

	MASYNC_RESULT	m_nResult;

public:
	MAsyncJob(int nJobID) {
		m_nJobID = nJobID;
		m_nPostTime = 0;
		m_nFinishTime = 0;
	}
	virtual ~MAsyncJob()	{}

	int GetJobID()							{ return m_nJobID; }
	auto GetPostTime() const				{ return m_nPostTime; }
	void SetPostTime(u64 nTime)				{ m_nPostTime = nTime; }
	auto GetFinishTime() const				{ return m_nFinishTime; }
	void SetFinishTime(u64 nTime)			{ m_nFinishTime = nTime; }

	MASYNC_RESULT GetResult()				{ return m_nResult; }
	void SetResult(MASYNC_RESULT nResult)	{ m_nResult = nResult; }

	virtual void Run(void* pContext) = 0;
};

class MAsyncJobList final : private std::deque<MAsyncJob*> {
protected:
	CRITICAL_SECTION	m_csLock;
public:
	MAsyncJobList()				{ InitializeCriticalSection(&m_csLock); }
	~MAsyncJobList()	{ DeleteCriticalSection(&m_csLock); }

	void Lock()		{ EnterCriticalSection(&m_csLock); }
	void Unlock()	{ LeaveCriticalSection(&m_csLock); }

	auto GetBeginItorUnsafe()	{ return begin(); }
	auto GetEndItorUnsafe()		{ return end(); }

	void AddUnsafe(MAsyncJob* pJob) {
		push_back(pJob);
	}
	void RemoveUnsafe(MAsyncJob* pJob, MAsyncJobList::iterator* itorOut) {
		iterator i = find(begin(), end(), pJob);
		if (i != end()) {
			iterator itorTmp = erase(i);
			if (itorOut)
				*itorOut = itorTmp;
		}
	}
	MAsyncJob* GetJobUnsafe() {
		if (begin() == end()) return NULL;
		MAsyncJob* pReturn = *begin();
		pop_front();
		return pReturn;
	}
	int GetCount() { return (int)size(); }
};

#define MAX_THREADPOOL_COUNT 10

class MAsyncProxy final {
protected:
	HANDLE				m_hEventShutdown;
	HANDLE				m_hEventFetchJob;

	int					m_nThreadCount;
	HANDLE				m_ThreadPool[MAX_THREADPOOL_COUNT];

	MAsyncJobList		m_WaitQueue;
	MAsyncJobList		m_ResultQueue;

	std::deque<std::function<void()>> GenericJobs;
	std::mutex GenericJobMutex;

	CRITICAL_SECTION	m_csCrashDump;
	
protected:
	HANDLE GetEventShutdown()	{ return m_hEventShutdown; }
	HANDLE GetEventFetchJob()	{ return m_hEventFetchJob; }

	static DWORD WINAPI WorkerThread(LPVOID pJobContext);
	void OnRun();

public:
	MAsyncProxy();
	bool Create(int nThreadCount);
	void Destroy();
	
	int GetWaitQueueCount()		{ return m_WaitQueue.GetCount(); }
	int GetResultQueueCount()	{ return m_ResultQueue.GetCount(); }

	void PostJob(MAsyncJob* pJob);
	template <typename T>
	void PostJob(T&& fn)
	{
		std::lock_guard<std::mutex> lock(GenericJobMutex);
		GenericJobs.emplace_back(std::forward<T>(fn));
	}
	MAsyncJob* GetJobResult()	{
		MAsyncJob* pJob = NULL;
		m_ResultQueue.Lock();
			pJob = m_ResultQueue.GetJobUnsafe();
		m_ResultQueue.Unlock();
		return pJob;
	}

	DWORD CrashDump( PEXCEPTION_POINTERS ExceptionInfo );
};
