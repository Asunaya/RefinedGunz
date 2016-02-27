#ifndef _ZLOCALE_H
#define _ZLOCALE_H

#include "MBaseLocale.h"
#include "ZBaseAuthInfo.h"
#include "ZLocaleDefine.h"


//#define ZTOK_LANGUAGE_KOREA					"kor"
//#define ZTOK_LANGUAGE_INTERNATIONAL			"international"
//#define ZTOK_LANGUAGE_JAPAN					"jpn"

class ZConfiguration;





/// 지역화 관련 관리 클래스 
class ZLocale : public MBaseLocale
{
private:
	bool				m_bTeenMode;			// 한국에서만 사용하는 틴모드인지 여부
	ZBaseAuthInfo*		m_pAuthInfo;			// 인증 정보
protected:
	virtual bool OnInit();
	void CreateAuthInfo();
public:
	ZLocale();
	virtual ~ZLocale();
	static ZLocale* GetInstance();

	bool IsTeenMode();
	void SetTeenMode(bool bTeenMode);

	/// 홈페이지에서만 실행해야하는 지역에서 실행파일을 바로 실행했을 경우에는 홈페이지를 띄워준다.
	void RouteToWebsite();
	void PostLoginViaHomepage(MUID* pAllocUID);

	bool ParseArguments(const char* pszArgs);
	ZBaseAuthInfo* GetAuthInfo()			{ return m_pAuthInfo; }
};

inline ZLocale* ZGetLocale()
{
	return ZLocale::GetInstance();
}





#endif