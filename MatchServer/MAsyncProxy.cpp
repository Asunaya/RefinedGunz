#include "stdafx.h"
#include "MAsyncProxy.h"
#include "MMatchConfig.h"
#include "MMatchServer.h"
#include "MCrashDump.h"
#ifndef _PUBLISH
	#include "MProcessController.h"
#endif

bool MAsyncProxy::Create(int ThreadCount)
{
	return Create(ThreadCount, MakeDatabaseFromConfig);
}

bool MAsyncProxy::Create(int ThreadCount, function_view<IDatabase*()> GetDatabase)
{
	ThreadCount = min(ThreadCount, MAX_THREADPOOL_COUNT);

	for (int i = 0; i < ThreadCount; i++)
	{
		std::thread{ [this, Database = GetDatabase()] { WorkerThread(Database); } }.detach();
	}

	return true;
}

void MAsyncProxy::Destroy()
{
	EventShutdown.SetEvent();
}

void MAsyncProxy::PostJob(MAsyncJob* pJob)
{
	WaitQueue.Lock();
		pJob->SetPostTime(GetGlobalTimeMS());
		WaitQueue.AddUnsafe(pJob);	
	WaitQueue.Unlock();

	EventFetchJob.SetEvent();
}

void MAsyncProxy::WorkerThread(IDatabase* Database)
{
#ifdef _WIN32
	__try
	{
#endif
		OnRun(Database);
#ifdef _WIN32
	}
	__except(CrashDump(GetExceptionInformation())) 
	{
	}
#endif
}

void MAsyncProxy::OnRun(IDatabase* Database)
{
	MSignalEvent* EventArray[]{
		&EventShutdown,
		&EventFetchJob,
	};

	bool bShutdown = false;

	while (!bShutdown)
	{
		const auto Timeout = 1000; // Milliseconds
		const auto WaitResult = WaitForMultipleEvents(EventArray, Timeout);

		if (WaitResult == MSync::WaitTimeout) {
			if (WaitQueue.GetCount() > 0) {
				EventFetchJob.SetEvent();
			}
			continue;
		}

		switch(WaitResult)
		{
		case 0: // Shutdown
			bShutdown = true;
			break;
		case 1:	// Fetch Job
			{
				WaitQueue.Lock();
					MAsyncJob* pJob = WaitQueue.GetJobUnsafe();
				WaitQueue.Unlock();

				if (pJob) {
					pJob->Run(Database);
					pJob->SetFinishTime(GetGlobalTimeMS());

					ResultQueue.Lock();
						ResultQueue.AddUnsafe(pJob);
					ResultQueue.Unlock();
				}

				if (WaitQueue.GetCount() > 0) {
					EventFetchJob.SetEvent();
				}
			}
			break;
		};
	};
}

#ifdef _WIN32
u32 MAsyncProxy::CrashDump(EXCEPTION_POINTERS* ExceptionInfo)
{
	mlog("CrashDump Entered 1\n");
	std::lock_guard<MCriticalSection> lock{ csCrashDump };

	if (PathIsDirectory("Log") == FALSE)
		CreateDirectory("Log", NULL);

	time_t		tClock;
	struct tm	tmTime;

	time(&tClock);
	localtime_s(&tmTime, &tClock);

	char szFileName[_MAX_DIR];

	int nFooter = 1;
	while(TRUE) {
		sprintf_safe(szFileName, "Log/MAsyncProxy_%02d-%02d-%02d-%d.dmp", 
			tmTime.tm_year+1900, tmTime.tm_mon+1, tmTime.tm_mday, nFooter);

		if (PathFileExists(szFileName) == FALSE)
			break;

		nFooter++;
		if (nFooter > 100) 
		{
			return false;
		}
	}

	auto ret = CrashExceptionDump(ExceptionInfo, szFileName);

	mlog("CrashDump Leaving\n");

	return ret;
}
#endif
