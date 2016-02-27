#pragma once

#include <windows.h>

// Referenced from http://www.gamedev.net/reference/articles/article1941.asp


class MCRC32 {
private:
	static DWORD CRC32Table[256];
	static inline void LookupCRC32( const BYTE byte, DWORD &dwCRC32 );

public:
	typedef DWORD crc_t;

	enum CRC
	{
		SIZE = sizeof(DWORD),
	};

	static DWORD BuildCRC32(BYTE* pData, DWORD dwSize); 
};
