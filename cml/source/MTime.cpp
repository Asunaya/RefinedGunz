#include "stdafx.h"
#include "MTime.h"

unsigned long long GetGlobalTimeMS()
{
	return timeGetTime();
}

float GetGlobalTime()
{
	return GetGlobalTimeMS() / 1000.f;
}
