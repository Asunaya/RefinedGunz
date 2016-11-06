#pragma once

#include <string>
#include "SafeString.h"
#include <WinSock2.h>

template <size_t size>
bool GetLocalIP(char (&szOutIP)[size])
{
	char szHostName[256];
	PHOSTENT pHostInfo;

	if (gethostname(szHostName, sizeof(szHostName)) != 0)
		return false;

	pHostInfo = gethostbyname(szHostName);
	if (!pHostInfo)
		return false;

	auto ip = inet_ntoa(*(struct in_addr *)*pHostInfo->h_addr_list);
	strcpy_safe(szOutIP, ip);
	return true;
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

inline std::string GetIPv4String(in_addr addr)
{
	char buf[32];
	GetIPv4String(addr, buf);
	return buf;
}