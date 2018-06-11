#include "MProcess.h"
#include "MFile.h"
#include "TestAssert.h"

void TestMProcess()
{
	auto Start = [] {
		auto Name = "ping";
#ifdef _WIN32
		auto Count = "-n";
#else
		auto Count = "-c";
#endif
		const char* Args[] = {"ping", "localhost", Count, "1", nullptr};
		return MProcess::Start(Name, Args);
	};
	auto Handle = Start();
	TestAssert(Handle);
	auto AwaitRet = MProcess::Await(Handle);
	TestAssert(!AwaitRet.TimedOut && AwaitRet.ExitCode == 0);
	Handle = Start();
	TestAssert(Handle);
	TestAssert(MProcess::Terminate(Handle));
	AwaitRet = MProcess::Await(Handle, 100);
	TestAssert(!AwaitRet.TimedOut);
}
