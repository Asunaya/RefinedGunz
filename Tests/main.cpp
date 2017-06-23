#include "MDebug.h"
#include <cstdio>
#include "TestAssert.h"

int main()
{
	InitLog();

	bool TestReplays();
	//assert(TestReplays());
	bool TestMath();
	assert(TestMath());
	bool TestLauncher();
	assert(TestLauncher());

	printf("\n\n\nAll OK!\n\n");

	getchar();
}