#include "MDebug.h"
#include <cstdio>
#include <cassert>

int main()
{
	InitLog();

	bool TestReplays();
	assert(TestReplays());
	bool TestMath();
	assert(TestMath());

	printf("\n\n\nAll OK!\n\n");

	getchar();
}