#pragma once

#include <cstdio>

#undef ASSERT
#undef assert

#define assert(pred)\
	do {\
		if (!(pred)) {\
			log_failed_assert(pred, #pred, __FILE__, __LINE__);\
			__asm { int 3 };\
		}\
	} while(false)
#define ASSERT assert

inline void log_failed_assert(bool pred, const char* str, const char *File, int Line)
{
	if (pred)
		return;

	printf("%s failed!\nFile: %s\nLine: %d\n\n", str, File, Line);
}