#ifndef _MBASELOCALE_H
#define _MBASELOCALE_H


#include "MLocaleDefine.h"

// 국가 코드
enum MCountry
{
	MC_INVALID			= 0,
	MC_KOREA			= 82,		// 한국
	MC_US				= 1,		// 미국(인터네셔날)
	MC_JAPAN			= 81,		// 일본
	MC_BRAZIL			= 55,		// 브라질
	MC_INDIA			= 91,		// 인도
};

//
//#ifdef LOCALE_US
//	#define DEFAULT_COUNTRY			MC_US
//#elif LOCALE_JAPAN
//	#define DEFAULT_COUNTRY			MC_JAPAN
//#elif LOCALE_BRAZIL
//	#define DEFAULT_COUNTRY			MC_BRAZIL
//#elif LOCALE_INDIA
//	#define DEFAULT_COUNTRY			MC_INDIA
//#else
//	#define DEFAULT_COUNTRY			MC_KOREA
//#endif
//

/* TODO: MLangageConf.h 와 통합해야함 - bird */

// LANG_xxx는 winnt.h에 정의되어 있음. - by SungE.
enum MLanguage
{
	ML_INVALID				= 0x00,
	ML_CHINESE				= LANG_CHINESE,					// 중국어
	ML_CHINESE_TRADITIONAL	= SUBLANG_CHINESE_TRADITIONAL,	// 중국어 같은데 -  _-)a;; 
	ML_KOREAN				= LANG_KOREAN,					// 한국어 (LANG_KOREAN값과 동일)
	ML_ENGLISH				= LANG_ENGLISH,					// 영어 (LANG_ENGLISH값과 동일)
	ML_JAPANESE				= LANG_JAPANESE,				// 일본어 (LANG_JAPANESE값과 동일)
	ML_BRAZIL				= LANG_PORTUGUESE,				// 브라질 (LANG_BRAZIL값과 동일)
	ML_INDIA				= LANG_INDONESIAN,				// 인도 (LANG_INDONESIAN값과 동일)
};

/// 지역화 관련 최상위 관리 클래스 
class MBaseLocale
{
private:
	void InitLanguageFromCountry();
protected:
	MCountry			m_nCountry;
	MLanguage			m_nLanguage;

	bool				m_bIsComplete;

	virtual bool OnInit() = 0;
public:
	MBaseLocale();
	virtual ~MBaseLocale();
	bool Init(MCountry nCountry);

	const MCountry	GetCountry()		{ return m_nCountry; }
	const MLanguage GetLanguage()		{ return m_nLanguage; }

	const bool bIsComplete()			{ return m_bIsComplete; }
};

const MCountry GetCountryID( const char* pCountry );
const MLanguage GetLanguageID( const char* pLanguage );

#endif