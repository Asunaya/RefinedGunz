#pragma once
#include <cstdio>
#include <tuple>
#include "MDebug.h"

template <int = 0>
struct TestState
{
	static bool AssertionFailed;
	static bool BreakOnFailedAssertion;
};

template <> bool TestState<>::AssertionFailed = false;
template <> bool TestState<>::BreakOnFailedAssertion = true;

#define TestAssert(pred)\
	do {\
		if (!(pred)) {\
			OnFailedAssertion(#pred, __FILE__, __LINE__);\
			if (TestState<>::BreakOnFailedAssertion)\
				__asm { int 3 };\
		}\
	} while(false)

#define TestFail(msg) TestAssert(!(msg))

inline void OnFailedAssertion(const char* str, const char *File, int Line)
{
	auto Log = [](auto&&... a) {
		printf(a...);
		MLog(a...);
	};
	
	Log("Assertion %s failed!\n"
		"File: %s\nLine: %d\n\n",
		str,
		File, Line);

	TestState<>::AssertionFailed = true;
}