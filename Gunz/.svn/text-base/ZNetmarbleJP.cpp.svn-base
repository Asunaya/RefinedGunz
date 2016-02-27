#include "stdafx.h"
#include "ZNetmarbleJP.h"
#include "ZLocale.h"


ZNetmarbleJPAuthInfo::ZNetmarbleJPAuthInfo() : ZBaseAuthInfo()
{

}

ZNetmarbleJPAuthInfo::~ZNetmarbleJPAuthInfo()
{

}

#ifdef LOCALE_JAPAN
//////////////////////////////////////////////////////////////////////////////////////////
static char* StrTok( char *lpStr, char chSeparator )
{
	static const int TOK_BUFSIZE = 1024*40;

	static char chBuf[TOK_BUFSIZE];
	static char chToken[TOK_BUFSIZE];

	static char *lpNow = NULL;

	if( lpStr )
	{
		int nlen = (int)strlen( lpStr );
		_ASSERT( nlen < TOK_BUFSIZE );

		strcpy( chBuf, lpStr );
		lpNow = chBuf;
	}

	if( !lpNow || *lpNow == 0 ) return NULL;

	int nTLen = 0;
	char *porg = lpNow;
	while( *lpNow != 0 )
	{
		if( *lpNow == chSeparator )
		{
			++lpNow;
			break;
		}
		++nTLen;
		++lpNow;
	}

	if( nTLen > 0 )
	{
		strncpy( chToken, porg, nTLen );
		chToken[nTLen] = 0;
	}
	else
	 chToken[0] = 0;

  return chToken;
}


static BOOL AnalizeArgument( char *lpArg )
{
	if( strlen( lpArg ) >= ( 1024 * 4 ) ) return FALSE;

	char szKey[1024] = {0,};
	char szBuf[1024*4] = {0,};

	if( !GetCryptKey( szKey, 1024-1, true ) ) return FALSE;
	if( CRYPT_SUCCESS != DecryptString( lpArg, szKey, szBuf ) ) return FALSE;

	char szGetArg[5][1024] = {0,};
	char* token = StrTok( szBuf, ',' );

	int nArgCount = 0;
	while( token && nArgCount < 5 )
	{
		strcpy( szGetArg[nArgCount], token );
		nArgCount++;
		token = StrTok( NULL, ',' );
	}

	if( nArgCount < 4 ) return FALSE;
	if( strlen( szGetArg[0] ) > 16 ) return FALSE;

	//--------------------------------------------------------------------------------------------|

	char* pszServIP		= szGetArg[0];
	UINT nServPort		= (UINT)::atoi( szGetArg[1] );
	char* pszLoginID	= szGetArg[2];
	char* pszLoginPW	= szGetArg[3];
	char* pszExtra		= szGetArg[4];

	ZNetmarbleJPAuthInfo* pMNInfo = (ZNetmarbleJPAuthInfo*)ZGetLocale()->GetAuthInfo();
	pMNInfo->SetServerIP(pszServIP);
	pMNInfo->SetServerPort(nServPort);
	pMNInfo->SetLoginID(pszLoginID);
	pMNInfo->SetLoginPW(pszLoginPW);
	pMNInfo->SetExtra(pszExtra);

//#ifdef _DEBUG
/*
	pMNInfo->SetServerIP("192.168.0.100");
	pMNInfo->SetServerPort(6000);
	pMNInfo->SetLoginID("JM0000726991");
	pMNInfo->SetLoginPW("skarlfyd");


mlog("ServerIP: '%s' \n", pszServIP);
mlog("ServPort: '%u' \n", nServPort);
mlog("LoginID: '%s' \n", pszLoginID);
mlog("LoginPW: '%s' \n", pszLoginPW);
mlog("Extra: '%s' \n", pszExtra);
*/
//#endif
	//--------------------------------------------------------------------------------------------|

	return TRUE;
}

bool NetmarbleJPParseArguments(const char* pszArgs)
{
	if (AnalizeArgument((char*)pszArgs) == TRUE)
		return true;
	else
		return false;
}


#endif // LOCALE_JAPAN