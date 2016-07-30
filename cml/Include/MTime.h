#pragma once

unsigned long long GetGlobalTimeMS();
float GetGlobalTime();

#pragma comment(linker, "/alternatename:?GetGlobalTimeMS@@YA_KXZ=_GetGlobalTimeMSDefault")