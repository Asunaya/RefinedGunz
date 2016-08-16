#ifndef _MMATCH_ANTIHACK_H
#define _MMATCH_ANTIHACK_H


#include "MUID.h"

#include <list>
#include <string>
using namespace std;

/// 오브젝트의 XTrap 관련 정보
struct MMatchObjectAntiHackInfo {
	char		m_szRandomValue[32];
				MMatchObjectAntiHackInfo()	{ Clear(); }
	void		Clear()						{ memset(m_szRandomValue, 0, sizeof(m_szRandomValue));	}
};


class MMatchAntiHack
{
private:
	static list<string>			m_ClientHashValueList;
	static list<unsigned int>	m_clientFileListCRC;

public:
	MMatchAntiHack()		{}
	~MMatchAntiHack()		{}

	static size_t			GetHashMapSize();
	static size_t			GetFielCRCSize();

	static bool				CrackCheck(char* szSerialKey, char* szRandomValue);
	static bool				AddNewHashValue( const string& strNewHashValue );

	static void				ClearHashMap() { m_ClientHashValueList.clear(); }
	static void				InitHashMap();

	static void				ClearClientFileList() { m_clientFileListCRC.clear(); }
	static void				InitClientFileList();
	static bool				CheckClientFileListCRC(unsigned int crc, const MUID& uidUser );
};

#endif
