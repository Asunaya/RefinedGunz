#pragma once

#include <cstdio>

#define TestAssert(pred)\
	do {\
		if (!(pred)) {\
			log_failed_assert(pred, #pred, __FILE__, __LINE__);\
			__asm { int 3 };\
		}\
	} while(false)

inline void log_failed_assert(bool pred, const char* str, const char *File, int Line)
{
	if (pred)
		return;

	printf("%s failed!\nFile: %s\nLine: %d\n\n", str, File, Line);
}