#include "stdafx.h"

#include <windows.h>
#include <crtdbg.h>
#include "ZNetmarble.h"
#include "ZLocale.h"
#include "MDebug.h"
#include "MLex.h"


//// ZNetmarbleAuthInfo ////////////////

ZNetmarbleAuthInfo::ZNetmarbleAuthInfo() : ZBaseAuthInfo()
{
	m_AuthCookie[0] = NULL;
	m_DataCookie[0] = NULL;
	strcpy( m_SpareParam, "12" );
}

ZNetmarbleAuthInfo::~ZNetmarbleAuthInfo()
{
}

////////////////ZNetmarbleAuthInfo ////

#ifdef LOCALE_KOREA

/* [ 클라이언트 작업 사항 ]
---------------------------------------------------------------------------------------------

1. 클라이언트에서는 클라이언트 복호화 모듈인 NMCrypt만 사용해야한다.

  주의) 서버쪽 복호화 모듈인 CPSSOLib 라이브러리와 DLL은 반드시 서버에서만 사용해야한다.
        절대로!! 클라이언트에 코드와 DLL을 포함시키면 안됨.
        (클라이언트의 코드에 포함되게 되면 disassemble등으로 코드 유출이 가능함)

        클라이언트에서 ID를 알고자 하는 경우라 하더라도 
        반드시 서버에서 복호화하여 클라이언트에 알려주도록 해야함

        보안에 관련된 아주 중요한 사항임.


2. 기존에 사용하던 아래의 두 함수를 이 코드로 교체한다.
  (단, 리턴되는 쿠키의 사이즈가 상당히 길기 때문에 버퍼를 충분히 여유있게 잡아야 함.)

3. 분석된 정보를 이용하여 서버에 접속하여

   g_CpCookie 를 서버에 보내어 인증을 거친다.

   클라이언트에서는 g_CpCookie 값이 암호화되어 있기 때문에 어떤 값이 들어있는지는 알 수 없다.

   g_AuthCookie, g_DataCookie는 넷마블 게임 전용 쿠키이기 때문에 CP업체는 관련이 없음(값을 넘기지 않을 수도 있다).
   (단, 카툰레이서와 같은 특정 게임은 3가지 쿠키 모두 필요함) 
*/

static char* _StrTok(const char* str, const char sep)
{
	// strtok()와 같은 기능의 함수이지만 separator를 1개만 받고
	// 인자가 비어있는 경우도 인자로 인정함

	static const int TOK_BUFSIZE = 1024*40;

	static char* pnow = NULL;
	static char buf[TOK_BUFSIZE];
	static char token[TOK_BUFSIZE];

	if(str) {
		size_t slen = strlen(str);
		_ASSERT(slen < TOK_BUFSIZE);		// ASSERT
		strcpy(buf, str);
		pnow = buf;
	}

	if(!pnow || *pnow == 0)
		return NULL;

	int tlen = 0;
	char* porg = pnow;
	while(*pnow != 0)
	{
		if(*pnow == sep) {
			++pnow;
			break;
		}

		++tlen;
		++pnow;
	}

	if(tlen > 0) {
		strncpy(token, porg, tlen);
		token[tlen] = 0;
	}
	else {
		token[0] = 0;
	}

	return token;
}

static BOOL AnalyzeArgument(char *key)
{
	//
	// 클립보드 인자 전달 규칙(각각의 인자는 컴마(,)로 구분되며 인자 사이의 공백은 없다)
	// 전달 인자의 순서 : ServerIP,Port,AuthCookie,DataCookie,CpCookie,SpareParam (인자는 암호화되어 있다)
	// - 넷마블 게임에서는 CpCookie를 전달하지 않는다
	// - CP 게임에서는 AuthCookie, DataCookie를 전달하지 않는다(몇몇 CP게임은 예외)
	// - Extra 인자는 생략되기도 한다
	//

	char buf[1024*20] = {0,};	// 버퍼는 충분히 크게 잡는다

	// 클립보드에서 인자 데이터를 읽어온다(한번 얻어오면 삭제됨)
	if(!GetNMClipData(buf, sizeof(buf)-1, key, true))
		return FALSE;

	if(strlen(buf)==0)
		return FALSE;

	char arg[6][1024*2] = {0,};

	int argcnt = 0;
	char* token = _StrTok(buf, ',');
	while(token && argcnt < 6)
	{
		strcpy(arg[argcnt], token);
		argcnt++;
		token = _StrTok(NULL, ',');
	}

	// 인자가 최소한 5개는 되어야 한다
	if(argcnt < 5)
		return FALSE;

	// IP주소가 올바른지 검사
	if(strlen(arg[0]) > 16)
		return FALSE;

	// CP 게임은 아래부분을 고쳐 쓸것
	char* pServIP     = arg[0];					// 서버 IP
	UINT  nServPort   = (UINT)::atoi(arg[1]);	// 서버 Port
	char* pAuthCookie = arg[2];					// 인증 쿠키	- 버퍼 크기를1024*2 정도로 잡아야 한다
	char* pDataCookie = arg[3];					// 데이터 쿠키	- 버퍼 크기를1024*2 정도로 잡아야 한다
	char* pCpCookie   = arg[4];					// CP쿠키		- 버퍼 크기를1024*2 정도로 잡아야 한다
	char* pSpareParam = arg[5];					// 여분의 인자(CP게임등에서 사용)

	ZNetmarbleAuthInfo* pMNInfo = (ZNetmarbleAuthInfo*)ZGetLocale()->GetAuthInfo();
	pMNInfo->SetServerIP(pServIP);
	pMNInfo->SetServerPort(nServPort);
	pMNInfo->SetAuthCookie(pAuthCookie);
	pMNInfo->SetDataCookie(pDataCookie);
	pMNInfo->SetCpCookie(pCpCookie);
	pMNInfo->SetSpareParam(pSpareParam);

	//mlog("NMSpareParam = %s \n", pSpareParam);

	char szField[64] = "";
	char szValue[64] = "";
	MLex lex;
	if(lex.SplitValue(pSpareParam, "=", szField, szValue)) {
		if (stricmp("age", szField) == 0) {
			int nAge = atoi(szValue);
			if (nAge < 18)
				pMNInfo->SetTeenMode(true);
			else
				pMNInfo->SetTeenMode(false);
		}
	}

//	pMNInfo->SetServerIP("192.168.0.30");	// RAON DEBUG

#ifndef _PUBLISH
mlog("ServerIP: '%s' \n", pServIP);
mlog("ServPort: '%d' \n", nServPort);
mlog("AuthCookie: '%s' \n", pAuthCookie);
mlog("DataCookie: '%s' \n", pDataCookie);
mlog("CpCookie: '%s' \n", pCpCookie);
mlog("SpareParam: '%s' \n", pSpareParam);
#endif
	
	return TRUE;
}


bool NetmarbleParseArguments(const char* pszArgs)
{
	if (AnalyzeArgument((char*)pszArgs) == TRUE)
		return true;
	else
		return false;
}

#endif // #ifdef LOCALE_KOREA