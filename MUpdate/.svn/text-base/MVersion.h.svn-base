#pragma once


#include <windows.h>


#define COMBINE_VERSION(M, m) (((M >> 16) & 0xFF)<<24) | (((M) & 0xFF)<<16) | (((m >> 16) & 0xFF)<<8) | ((m) & 0xFF)

bool MGetFileVersion(unsigned long* pMajorVersion, unsigned long* pMinorVersion, char* szFileName);
DWORD MMakeVersion(const char* pszServiceVersion);