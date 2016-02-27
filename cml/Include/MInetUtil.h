#ifndef _MINETUTIL_H
#define _MINETUTIL_H



/// . 있는 IP 문자열을 . 없는 IP 문자열(12바이트)로 변환
void MConvertCompactIP(char* szOut, int maxlen, const char* szInputDottedIP);


void GetLocalIP( char* szOutIP, int nSize );


#endif