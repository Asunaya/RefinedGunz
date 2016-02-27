#ifndef _MMATCHUTIL_H
#define _MMATCHUTIL_H

#include <limits.h>
#include "MPacketCrypter.h"
#include "MUID.h"

// 유틸리티 펑션들
unsigned long int MGetTimeDistance(unsigned long int nTimeA, unsigned long int nTimeB);

class MZFileSystem;
unsigned long MGetMemoryChecksum(char *pBuffer, int nLen);
unsigned long MGetMZFileChecksum(const char* pszFileName);
void MMakeSeedKey(MPacketCrypterKey* pKey, const MUID& uidServer, const MUID& uidClient, unsigned int nTimeStamp);

struct MShortVector
{
	short x;
	short y;
	short z;
};

inline float ShortToDirElement(short x)
{
	return ((1.f/32000.f) * x);
}

inline short DirElementToShort(float x)
{
	return (short)(32000*x);
}

#endif