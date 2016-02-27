#ifndef MMAP_H
#define MMAP_H

#include "MGridMap.h"
#include <vector>
using namespace std;


class MObject;
namespace RealSpace2{
	class RBspObject;
}
class MZFileSystem;

#define MMAP_NAME_LENGTH	64

typedef MGridMap<MObject*>	MObjectGridMap;

/// Map
class MMap : public MObjectGridMap{
protected:
	char	m_szName[MMAP_NAME_LENGTH];	///< Map Name
	int		m_nID;
	/*
	MGridMap<MObjectCharacter*>	m_CharacterGridMap;	///< Grid Map for Referencing
	MGridMap<MObjectItem*>		m_ItemGridMap;		///< Grid Map for Referencing
	*/
	RealSpace2::RBspObject*	m_pBspObj;

public:
	MMap(int nID, const char* szName);
	virtual ~MMap(void);

	int GetID(void){ return m_nID; }
	const char* GetName(void){ return m_szName; }

	int Create(const char* szFileName, MZFileSystem* pZFS=NULL);
	void Destroy(void);

	/*
	void AddRef(MObject* pObj);
	void DelRef(MObject* pObj);
	*/

	void RandomLocate(void);

	RealSpace2::RBspObject*	GetBspObject(void){ return m_pBspObj; }
};

/// ¸Ê ÄÁÅ×ÀÌ³Ê
class MMapManager : public vector<MMap*>{
public:
	MMapManager(void);
	virtual ~MMapManager(void);

	MMap* Get(const char* szName);
	int GetID(const char* szName);
};


#endif