#include "stdafx.h"
#include "ZLocale.h"
#include "ZConfiguration.h"
#include "ZNetmarble.h"			// test
#include "ZApplication.h"
#include "ZGlobal.h"
#include "ZSecurity.h"
#include "ZPost.h"

ZLocale* ZLocale::GetInstance()
{
	static ZLocale m_stLocale;
	return &m_stLocale;
}

ZLocale::ZLocale() : MBaseLocale(), m_bTeenMode(false), m_pAuthInfo(NULL)
{

}

ZLocale::~ZLocale()
{
	if(m_pAuthInfo) delete m_pAuthInfo;
}

bool ZLocale::IsTeenMode()
{
	if (m_pAuthInfo) return m_pAuthInfo->GetTeenMode();

	return false;
}

void ZLocale::SetTeenMode(bool bTeenMode)
{
	if (m_pAuthInfo) m_pAuthInfo->SetTeenMode(bTeenMode);
	else _ASSERT(0);
}

bool ZLocale::OnInit()
{
	CreateAuthInfo();

	return true;
}

bool ZLocale::ParseArguments(const char* pszArgs)
{
	switch (m_nCountry)
	{
	case MC_KOREA:
		{
			mlog("LOCALE:KOREA \n");
#ifdef LOCALE_KOREA
			if ( NetmarbleParseArguments( pszArgs))
			{
				ZApplication::GetInstance()->SetLaunchMode(ZApplication::ZLAUNCH_MODE_NETMARBLE);
				return true;
			}
			else
			{
				mlog( "Netmarble Certification Failed. Shutdown. \n");
				return false;
			}
#endif // LOCALE_KOREA
		}
		break;

	case MC_JAPAN:
		{
			mlog("LOCALE:JAPAN \n");
#ifdef LOCALE_JAPAN
			if ( NetmarbleJPParseArguments( pszArgs))
			{
				ZApplication::GetInstance()->SetLaunchMode(ZApplication::ZLAUNCH_MODE_NETMARBLE);
				return true;
			}
			else
			{
				mlog( "Netmarble Certification Failed. Shutdown. \n");
				return false;
			}
#endif // LOCALE_JAPAN
		}
		break;
	default:
		{
			mlog("LOCALE:UNKNOWN \n");
		}
	};

	return true;
}

void ZLocale::CreateAuthInfo()
{
	if (m_pAuthInfo) delete m_pAuthInfo;
	m_pAuthInfo = NULL;

	switch (m_nCountry)
	{
	case MC_KOREA:
		{
#ifdef LOCALE_KOREA
			m_pAuthInfo = new ZNetmarbleAuthInfo();
#endif // LOCALE_KOREA
		}
		break;
	case MC_JAPAN:
		{
#ifdef LOCALE_JAPAN
			m_pAuthInfo = new ZNetmarbleJPAuthInfo();
#endif // LOCALE_JAPAN
		}
		break;
	default:
		{

		}
	};
}

void ZLocale::RouteToWebsite()
{
	if (strlen(Z_LOCALE_HOMEPAGE_URL) > 0)
	{
		ShellExecute(NULL, "open", Z_LOCALE_HOMEPAGE_URL, NULL, NULL, SW_SHOWNORMAL);

		char szMsgWarning[128]="";
		char szMsgCertFail[128]="";
		ZTransMsg(szMsgWarning,MSG_WARNING);
		ZTransMsg(szMsgCertFail,MSG_REROUTE_TO_WEBSITE);
		MessageBox(g_hWnd, szMsgCertFail, szMsgWarning, MB_OK);
	}
}

void ZLocale::PostLoginViaHomepage(MUID* pAllocUID)
{
//	unsigned long nChecksum = MGetMatchItemDescMgr()->GetChecksum();
//	unsigned long nChecksum = ZGetMZFileChecksum(FILENAME_ZITEM_DESC);
//	unsigned long nChecksum = MGetMatchItemDescMgr()->GetChecksum();
//	unsigned long nChecksum = ZGetMZFileChecksum(FILENAME_ZITEM_DESC);
	unsigned long nChecksum = ZGetApplication()->GetFileListCRC();
	nChecksum = nChecksum ^ (*pAllocUID).High ^ (*pAllocUID).Low;

	switch (m_nCountry)
	{
#ifdef LOCALE_KOREA
	case MC_KOREA:
		{
			char szSpareParam[3] = {1,2,0};
			ZPostNetmarbleLogin((char*)m_pAuthInfo->GetCpCookie(), szSpareParam, nChecksum);
		}
		break;
#endif //LOCALE_KOREA

#ifdef LOCALE_JAPAN
	case MC_JAPAN:
		{

			ZNetmarbleJPAuthInfo* pNMAuth = (ZNetmarbleJPAuthInfo*)m_pAuthInfo;
			ZPostNetmarbleJPLogin((char*)pNMAuth->GetLoginID(), (char*)pNMAuth->GetLoginPW(), nChecksum);
		}
		break;
#endif
	default:
		mlog("LoginViaHomepage - Unknown Locale \n");
		break;
	};
}


