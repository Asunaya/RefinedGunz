#pragma once


#include <windows.h>
#include <list>
using namespace std;


class MPatchNode {
protected:
	FILETIME		m_tmWrite;
	unsigned long	m_nSize;
	char			m_szName[_MAX_DIR];
	unsigned long	m_nChecksum;
	bool			m_bValidate;

public:
	MPatchNode(const char* pszName, unsigned long nSize, FILETIME tmWrite, unsigned long nChecksum);
	virtual ~MPatchNode();

	FILETIME GetWriteTime()	{ return m_tmWrite; }
	unsigned long GetSize()	{ return m_nSize; }
	const char* GetName()	{ return m_szName; }

	unsigned long GetChecksum()	{ return m_nChecksum; }
	void MakeChecksum();

	bool IsValid()			{ return m_bValidate; }
	bool CheckValid();
	void ForcedSetValid(bool bVal)	{ m_bValidate = bVal; }
};
class MPatchList : public list<MPatchNode*>{};
