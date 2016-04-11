#include "stdafx.h"
#include "MMatchConfig.h"


list<string> MMatchAntiHack::m_ClientHashValueList;

void MMatchAntiHack::InitHashMap()
{
#ifdef _XTRAP
	if( MGetServerConfig()->IsUseXTrap() )
	{
		FILE* fp;
		fopen_s(&fp, "hashmap.txt", "r");
		if (fp == NULL) return;

		char str[256];

		while (fgets(str, 256, fp) != NULL)
		{
			char szHash[256] = {0,};
			sscanf_s(str, "%s", szHash, 256);
			m_ClientHashValueList.push_back(szHash);
		}

		fclose(fp);
	}
#endif
}

bool MMatchAntiHack::CrackCheck(char* szSerialKey, char* szRandomValue)
{
#ifdef _XTRAP
	if( MGetServerConfig()->IsUseXTrap() )
	{
		if (m_ClientHashValueList.empty()) return true;

		for (list<string>::iterator itor = m_ClientHashValueList.begin(); itor != m_ClientHashValueList.end(); ++itor)
		{
			int rest = MGetMatchServer()->XTrap_XCrackCheckW(szSerialKey,		/* Input Verify Key(From Client)*/
				szRandomValue,		/* Input Generated First Key	*/
				(char*)(*itor).c_str());	/* Input Client Hash Value		*/

			if (rest != 0) return true;
		}
		return false;
	}
#endif

	return true;
}

bool MMatchAntiHack::AddNewHashValue( const string& strNewHashValue )
{
#ifdef _XTRAP
	if( MGetServerConfig()->IsUseXTrap() )
	{
		// if( !strNewHashValue.empty() )
		{
			string strRealHashValue;

			if( '@' == strNewHashValue[0] )
			{
				m_ClientHashValueList.clear();
				strRealHashValue = strNewHashValue.c_str() + 1;
			}
			else
			{
				strRealHashValue = strNewHashValue;
			}

			m_ClientHashValueList.push_back( strRealHashValue );

			FILE* fp;
			fopen_s(&fp, "hashmap.txt", "w");
			if( 0 == fp )
				return false;

			list<string>::iterator it, end;
			end = m_ClientHashValueList.end();
			for( it = m_ClientHashValueList.begin(); it != end; ++it )
				fprintf( fp, "%s\n", (*it).c_str() );
			fclose( fp );
		}
	}
#endif
	
	return true;
}

///////////////////// client file list

list<unsigned int>	MMatchAntiHack::m_clientFileListCRC;

void MMatchAntiHack::InitClientFileList()
{
	FILE* fp;
	fopen_s(&fp, "filelistcrc.txt", "r");
	if (fp == NULL) return;

	char str[256];

	while (fgets(str, 256, fp) != NULL)
	{
		unsigned int crc;
		sscanf_s(str, "%u", &crc);
		m_clientFileListCRC.push_back(crc);
	}

	fclose(fp);

	mlog("Inited client file list (%d)\n", (int)m_clientFileListCRC.size());
}

bool MMatchAntiHack::CheckClientFileListCRC( unsigned int crc, const MUID& uidUser )
{
	bool bFound = m_clientFileListCRC.end() != find(m_clientFileListCRC.begin(),m_clientFileListCRC.end(),crc);
	return bFound;
}


size_t MMatchAntiHack::GetHashMapSize() 
{ 
	return m_ClientHashValueList.size(); 
}


size_t MMatchAntiHack::GetFielCRCSize() 
{ 
	return m_clientFileListCRC.size(); 
}