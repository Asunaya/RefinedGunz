#ifndef _MCHATTINGFILTER_H
#define _MCHATTINGFILTER_H

#include <list>
#include <string>


typedef list<string>				STRFILTER_MAP;
typedef STRFILTER_MAP::iterator		STRFILTER_ITR;



class MChattingFilter
{
private :
	STRFILTER_MAP	m_AbuseMap;
	STRFILTER_MAP	m_InvalidNameMap;

	string			m_strRemoveTokSkip;
	string			m_strRemoveTokInvalid;

	char			m_szLastFilterdStr[ 256];


public :
	MChattingFilter();
	~MChattingFilter();

	static MChattingFilter* GetInstance()
	{
		static MChattingFilter ChattingFilter;
		return &ChattingFilter;
	}

	bool LoadFromFile( MZFileSystem* pfs, const char* szFileName);
	bool IsValidChatting( const char* strText);
	bool IsValidName( const char* strText);
	const char* GetLastFilteredStr()				{ return m_szLastFilterdStr; }
	bool FindInvalidChar( const string& strText);

protected:
	void GetLine( char*& prfBuf, char* szType, char* szText);
	void SkipBlock( char*& prfBuf);
	const string PreTranslate( const string& strText);
	bool FindInvalidWord( const string& strWord, const string& strText);
};

inline MChattingFilter* MGetChattingFilter()		{ return MChattingFilter::GetInstance(); }

#endif