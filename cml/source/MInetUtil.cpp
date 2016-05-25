#include "stdafx.h"
#include "Winsock2.h"
#include "MInetUtil.h"
#include <iostream>

void MConvertCompactIP(char* szOut, int maxlen, const char* szInputDottedIP)
{
	in_addr addr;
	addr.S_un.S_addr = inet_addr(szInputDottedIP);
	sprintf_s(szOut, maxlen, "%03u%03u%03u%03u", addr.S_un.S_un_b.s_b1, addr.S_un.S_un_b.s_b2, 
											addr.S_un.S_un_b.s_b3, addr.S_un.S_un_b.s_b4);
}