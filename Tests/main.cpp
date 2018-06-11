#include "MDebug.h"
#include <cstdio>
#include <vector>
#include "TestAssert.h"

int main()
{
	InitLog(MLOGSTYLE_DEBUGSTRING | MLOGSTYLE_FILE);

	using TestFuncType = void();
	std::vector<std::pair<const char*, TestFuncType*>> TestFuncs;
#define ADD(x) TestFuncType x; TestFuncs.push_back({#x, x})
#ifdef _WIN32
	ADD(TestReplays);
	ADD(TestMath);
#endif
	ADD(TestMUtil);
	ADD(TestStringView);
	ADD(TestSafeString);
	ADD(TestConfig);
	ADD(TestMFile);
	ADD(TestMProcess);
	ADD(TestMAsyncProxy);
	ADD(TestDB);
	ADD(TestLauncher);
#undef ADD

	int NumFail = 0;
	for (auto&& TestFunc : TestFuncs)
	{
		TestFunc.second();
		auto Result = !TestState<>::AssertionFailed;
		TestState<>::AssertionFailed = false;
		printf("%s %s\n", TestFunc.first, Result ? "succeeded" : "failed");
		if (!Result)
			++NumFail;
	}

	printf("\n\n");
	if (NumFail)
	{
		printf("%d tests failed\n", NumFail);
	}
	else
	{
		printf("All OK!");
	}

	return NumFail;
}