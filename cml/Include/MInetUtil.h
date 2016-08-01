#ifndef _MINETUTIL_H
#define _MINETUTIL_H

#include <string>
#include "SafeString.h"


/// . 있는 IP 문자열을 . 없는 IP 문자열(12바이트)로 변환
void MConvertCompactIP(char* szOut, int maxlen, const char* szInputDottedIP);


template <size_t size>
void GetLocalIP(char (&szOutIP)[size])
{
	char szHostName[256];
	PHOSTENT pHostInfo;

	if (gethostname(szHostName, sizeof(szHostName)) == 0)
	{
		if ((pHostInfo = gethostbyname(szHostName)) != NULL)
		{
			auto ip = inet_ntoa(*(struct in_addr *)*pHostInfo->h_addr_list);
			strcpy_safe(szOutIP, ip);
		}
	}
}

template <size_t size>
void GetIPv4String(in_addr addr, char(&ip_string)[size])
{
	sprintf_safe(ip_string, "%d.%d.%d.%d",
		addr.S_un.S_un_b.s_b1,
		addr.S_un.S_un_b.s_b2,
		addr.S_un.S_un_b.s_b3,
		addr.S_un.S_un_b.s_b4);
}



#endif