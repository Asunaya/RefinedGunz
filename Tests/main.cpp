#include "MDebug.h"
#include <cstdio>
#include "TestAssert.h"

int main()
{
	InitLog();

	bool TestReplays();
	TestAssert(TestReplays());
	bool TestMath();
	TestAssert(TestMath());
	bool TestLauncher();
	TestAssert(TestLauncher());

	printf("\n\n\nAll OK!\n\n");

	getchar();
}