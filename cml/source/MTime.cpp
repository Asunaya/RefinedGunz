#include "stdafx.h"
#include "MTime.h"

extern "C" unsigned long long GetGlobalTimeMSDefault()
{
	return timeGetTime();
}

float GetGlobalTime()
{
	return GetGlobalTimeMS() / 1000.f;

}