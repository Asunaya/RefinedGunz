#include <thread>
#include <chrono>
#include <atomic>
#include "MAsyncProxy.h"
#include "TestAssert.h"

void TestMAsyncProxy()
{
	MAsyncProxy map;
	map.Create(2, [] { return nullptr; });
	std::atomic<int> i{0};
	constexpr int JobID = 1234;
	struct MAsyncDBJob_Inc : MAsyncJob {
		MAsyncDBJob_Inc() : MAsyncJob(JobID) {}
		std::atomic<int>* i;
		void Run(void*) override
		{
			++(*i);
		}
	};
	MAsyncDBJob_Inc Jobs[2];
	for (auto& Job : Jobs)
	{
		Job.i = &i;
		map.PostJob(&Job);
	}
	bool Got[2]{};
	while (true)
	{
		auto p = map.GetJobResult();
		if (!p)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		
		if (p != &Jobs[0] && p != &Jobs[1])
		{
			TestFail("GetJobResult returned unrecognized pointer");
			return;
		}

		int Idx = int(p == &Jobs[1]);

		if (Got[Idx])
		{
			TestFail("GetJobResult returned duplicated pointer");
			return;
		}

		Got[Idx] = true;
		if (Got[!Idx])
			break;
	}
	TestAssert(i == 2);
	map.Destroy();
	std::this_thread::sleep_for(std::chrono::seconds(1));
}
