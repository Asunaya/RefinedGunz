#ifndef _MMATH_H
#define _MMATH_H

#include <math.h>
#include <crtdbg.h>

#define PI 3.141592f


/// min부터 max까지의 난수를 구한다. (nMin, nMax의 값도 포함된다.)
inline int RandomNumber(int nMin, int nMax)
{
	// 범위의 크기가 131,072이상이면 오버플로우가 발생한다.
	_ASSERT((nMax-nMin) < 131072);
	return (rand() * (nMax - nMin + 1) / (RAND_MAX+1) + nMin );
}

/// min부터 max까지의 난수를 구한다.
inline float RandomNumber(float fMin, float fMax)
{
	float fRandom = (float)rand() / (float)RAND_MAX;
	return(fRandom*(fMax-fMin)+fMin);
}

/// d&d식 주사위로 난수를 구한다. - sides면체 주사위 n개를 던지고 mod를 더해서 나오는 값 반환
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

#define ToRadian( degree ) ((degree) * (PI / 180.0f))
#define ToDegree( radian ) ((radian) * (180.0f / PI))


#endif