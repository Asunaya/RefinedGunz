#ifndef _MCHARACTERDESC_H
#define _MCHARACTERDESC_H

#include "winsock2.h"
#include "MObjectTypes.h"
#include "MXml.h"

#include <map>
using namespace std;

#define FILENAME_CHARACTER_DESC		"CharInitialData.xml"

class MCharacterDesc
{
private:
protected:
	char					m_szName[64];
	int						m_nDescID;
	MCharacterBasicAttr		m_BasicAttr;
public:
	MCharacterDesc(int id, char* name);
	virtual ~MCharacterDesc();

	MCharacterBasicAttr*	GetDesc() { return &m_BasicAttr; }
	int						GetID()   { return m_nDescID; }
	const char*				GetName() { return m_szName; }

	void SetDesc(MCharacterBasicAttr* pAttr) { m_BasicAttr = *pAttr; }
};

class MCharacterDescManager : public map<int, MCharacterDesc*>
{
private:
protected:

public:
	MCharacterDescManager();
	virtual ~MCharacterDescManager();
	bool Read(const char* szFileName);
	void Clear();
	MCharacterDesc* Find(int nDescID);
};

extern MCharacterDescManager	g_CharacterDescManager;

#endif