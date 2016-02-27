#include "stdafx.h"
#include "MBMatchAuth.h"
#include "Netmarble/CPSSOLib.h"
#include <stdlib.h>
#include "MDebug.h"
#include "MMatchStatus.h"

bool MBMatchAuthBuilder::ParseAuthInfo(const char* pszData, MMatchAuthInfo** ppoutAutoInfo)
{
	bool bResult = true;
	MBMatchAuthInfo* pAuthInfo = new MBMatchAuthInfo;

	char szBuf[MAUTHINFO_BUFLEN] = {0,};
	if ( bResult &= GetCPCookieValue(pszData, "UserID", szBuf) ) {
		pAuthInfo->SetUserID(szBuf);
	}
	if ( bResult &= GetCPCookieValue(pszData, "UniID", szBuf) ) {
		pAuthInfo->SetUniqueID(szBuf);
	}
	if ( bResult &= GetCPCookieValue(pszData, "Certificate", szBuf) ) {
		pAuthInfo->SetCertificate(szBuf);
	}
	if ( bResult &= GetCPCookieValue(pszData, "Name", szBuf) ) {
		pAuthInfo->SetName(szBuf);
	}
	if ( bResult &= GetCPCookieValue(pszData, "Age", szBuf) ) {
		pAuthInfo->SetAge(atoi(szBuf));
	}
	if ( bResult &= GetCPCookieValue(pszData, "Sex", szBuf) ) {
		pAuthInfo->SetSex(atoi(szBuf));
	}

	if (bResult) {
		*ppoutAutoInfo = pAuthInfo;
	}
	else {
		delete pAuthInfo;
		*ppoutAutoInfo = 0;
	}

	return bResult;
}

