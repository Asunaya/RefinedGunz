#include "MVersion.h"


#pragma comment ( lib, "Version.lib" )


//// Version /////////////////////////
bool MGetFileVersion(unsigned long* pMajorVersion, unsigned long* pMinorVersion, char* szFileName)
{
	DWORD dwVersionInfoSize = GetFileVersionInfoSize(szFileName, 0);
	if(dwVersionInfoSize<=0){
		*pMajorVersion = *pMinorVersion = 0;
		return false;
	}
	char* pBlock = new char[dwVersionInfoSize];
	if(GetFileVersionInfo(szFileName, NULL, dwVersionInfoSize, pBlock)==0){
		*pMajorVersion = *pMinorVersion = 0;
		return false;
	}

	VS_FIXEDFILEINFO* pFFI;
	unsigned int nVLenght;
	if(VerQueryValue(pBlock, "\\", (void**)&pFFI, &nVLenght)==0){
		*pMajorVersion = *pMinorVersion = 0;
		return false;
	}

	*pMajorVersion = pFFI->dwFileVersionMS;
	*pMinorVersion = pFFI->dwFileVersionLS;

	delete[] pBlock;
	return true;
}
DWORD MMakeVersion(const char* pszServiceVersion)
{
	DWORD dwVersion = 0;
	char szToken[] = "., ";
	char szBuf[_MAX_DIR];
	strcpy(szBuf, pszServiceVersion);
	int nOffset = 0;

	char* pszNumber = strtok(szBuf, szToken);
	while(pszNumber && nOffset<4) {
		int nNumber = atoi(pszNumber);

		nNumber = nNumber << (3-nOffset)*8;
		dwVersion |= nNumber;
		++nOffset;

		pszNumber = strtok(NULL, szToken);
	}

	return dwVersion;
}