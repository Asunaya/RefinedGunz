//////////////////////////////////////////////////
//
// 지역화 관련 코드 모음
//
// 
#ifndef _MLOCALE_H
#define _MLOCALE_H

#include <windows.h>
#include <string>

class MLocale
{
private:
	static int			GetCodePageFromCharset(int nCharset);
	static std::string	TransCode(const wchar_t *pwszString, int nCodePage = CP_UTF8);
	static std::string	TransCode(const char *pszString, int nOldCodePage = CP_ACP, int nNewCodePage=CP_UTF8);
public:
	static int			GetCharsetFromLang(LANGID langid);
	static int			GetCodePageFromLang(LANGID langid);
	static std::string	ConvUTF8ToAnsi(const char* pszString, LANGID langid = LANG_KOREAN);
	static std::string	ConvAnsiToUTF8(const char* pszString, LANGID langid = LANG_KOREAN);
};


#endif