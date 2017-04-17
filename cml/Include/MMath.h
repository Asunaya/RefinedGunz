#pragma once

#include <cmath>
#include <cstdlib>
#include <cassert>

#ifndef PI
#define PI 3.14159265358979323846
#endif
#define PI_FLOAT static_cast<float>(PI)
#define TAU 6.28318530717958647692
#define TAU_FLOAT static_cast<float>(TAU)


inline int RandomNumber(int nMin, int nMax)
{
	assert((nMax-nMin) < 131072);
	return (rand() * (nMax - nMin + 1) / (RAND_MAX+1) + nMin );
}

inline float RandomNumber(float fMin, float fMax)
{
	float fRandom = (float)rand() / (float)RAND_MAX;
	return(fRandom*(fMax-fMin)+fMin);
}

inline long Dice(unsigned char n, unsigned char sides, short mod)
{
    int result = mod;
    for(int i = 0; i < n; i++)
    {
		result += RandomNumber(1, sides);
    }
    return result;
}

inline float Roundf(float x)
{
	return floorf(x + .5f);
}

#define ToRadian( degree ) ((degree) * (PI_FLOAT / 180.0f))
#define ToDegree( radian ) ((radian) * (180.0f / PI_FLOAT))
