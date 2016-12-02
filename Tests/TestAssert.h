#pragma once

#include <cstdio>

#undef ASSERT
#undef assert

#define ASSERT(pred) assert_(pred, #pred, __FILE__, __LINE__)
#define assert(pred) do { if (!(pred)) __asm { int 3 }; } while(false)

inline void assert_(bool pred, const char* str, const char *File, int Line)
{
	if (pred)
		return;

	printf_s("%s failed!\nFile: %s\nLine: %d\n\n", str, File, Line);

	getchar();
}