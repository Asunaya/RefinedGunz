#pragma once

#include <vector>
#include <string>

using std::vector;
using std::string;

class MZFileSystem;

#define LOCATOR_PORT 8900

class ZLocatorList : public map< int, string >
{
public:
	ZLocatorList(void);
	virtual ~ZLocatorList(void);

	void Clear() { m_LocatorIPList.clear(); }
	const int GetSize() const { return static_cast< int >( m_LocatorIPList.size() ); }
	const string& GetIPByPos( const int nPos ) { return m_LocatorIPList[ nPos ]; }

//	bool Init( MZFileSystem* pFileSystem, const char* pszListFile );

	bool ParseLocatorList( MXmlElement& element );

private :
//	bool LoadXML( MZFileSystem* pFileSystem, const char* pszListFile );
	bool ParseLocator( MXmlElement& element );

private :
	vector< string > m_LocatorIPList;
};