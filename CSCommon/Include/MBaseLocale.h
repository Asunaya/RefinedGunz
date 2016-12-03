#pragma once

enum MCountry
{
	MC_INVALID			= 0,
	MC_KOREA			= 82,
	MC_US				= 1,
	MC_JAPAN			= 81,
	MC_BRAZIL			= 55,
	MC_INDIA			= 91,
};

enum MLanguage
{
	ML_INVALID				= 0x00,
	ML_CHINESE				= LANG_CHINESE,
	ML_CHINESE_TRADITIONAL	= SUBLANG_CHINESE_TRADITIONAL,
	ML_KOREAN				= LANG_KOREAN,
	ML_ENGLISH				= LANG_ENGLISH,
	ML_JAPANESE				= LANG_JAPANESE,
	ML_BRAZIL				= LANG_PORTUGUESE,
	ML_INDIA				= LANG_INDONESIAN,
};

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

	MCountry GetCountry()		{ return m_nCountry; }
	MLanguage GetLanguage()		{ return m_nLanguage; }

	bool bIsComplete()			{ return m_bIsComplete; }
};

MCountry GetCountryID( const char* pCountry );
MLanguage GetLanguageID( const char* pLanguage );