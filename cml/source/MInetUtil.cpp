#include "stdafx.h"
#include "Winsock2.h"
#include "MInetUtil.h"
#include <iostream>
#include <string>

using std::string;

void MConvertCompactIP(char* szOut, int maxlen, const char* szInputDottedIP)
{
	in_addr addr;
	addr.S_un.S_addr = inet_addr(szInputDottedIP);
	sprintf_s(szOut, maxlen, "%03u%03u%03u%03u", addr.S_un.S_un_b.s_b1, addr.S_un.S_un_b.s_b2, 
											addr.S_un.S_un_b.s_b3, addr.S_un.S_un_b.s_b4);
}



void GetLocalIP( char* szOutIP, int nSize )
{
	if( (0 == szOutIP) || (0 >= nSize) )
		return;

    unsigned int optval = 1 ;

    string ip;
    char szHostName[256];
    PHOSTENT pHostInfo;

    if( gethostname(szHostName,sizeof(szHostName)) ==0)
    {
        if((pHostInfo = gethostbyname(szHostName)) != NULL)
        {
            ip = inet_ntoa(*(struct in_addr *)*pHostInfo->h_addr_list); 
			if( ip.length() <= nSize )
				strncpy( szOutIP, ip.c_str(), ip.length() );
        }
    }
}