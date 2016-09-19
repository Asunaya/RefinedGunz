#pragma once

#include <cstdio>

#undef ASSERT
#undef assert

#define ASSERT(pred) assert(pred, #pred, __FILE__, __LINE__)

inline void assert(bool pred, const char* str, const char *File, int Line)
{
	if (pred)
		return;

	printf_s("%s failed!\nFile: %s\nLine: %d\n\n", str, File, Line);

	getchar();
}