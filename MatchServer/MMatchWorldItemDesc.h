#ifndef _MMATCHWORLDITEMDESC_H
#define _MMATCHWORLDITEMDESC_H

#include "MXml.h"
#include <map>
using namespace std;

#define WORLDITEM_NAME_LENGTH		256

enum MMATCH_WORLD_ITEM_TYPE
{
	WIT_HP			= 0,
	WIT_AP			= 1,
	WIT_BULLET		= 2,
	WIT_HPAP		= 3,
	WIT_CLIENT		= 4,	// 클라이언트 전용 월드아이템

	WIT_QUEST		= 5,	// 퀘스트 아이템 박스
	WIT_BOUNTY		= 6,	// 퀘스트에서 나오는 바운티 아이템

	WIT_END
};


struct MMatchWorldItemDesc
{
	short					m_nID;
	MMATCH_WORLD_ITEM_TYPE	m_nItemType;
	float					m_fAmount;
	unsigned long int		m_nTime;
	char					m_szModelName[WORLDITEM_NAME_LENGTH];
	char					m_szDescName[WORLDITEM_NAME_LENGTH];
};

class MMatchWorldItemDescMgr;
class MZFileSystem;


/// 월드 아이템 타입 목록
class MMatchWorldItemDescMgr : public map<short, MMatchWorldItemDesc*>
{
private:
protected:
	void ParseWorldItem(MXmlElement& element);
public:
	MMatchWorldItemDescMgr();
	virtual ~MMatchWorldItemDescMgr();
	bool ReadXml(const char* szFileName);
	bool ReadXml(MZFileSystem* pFileSystem, const char* szFileName);
	void Clear();
	MMatchWorldItemDesc* GetItemDesc(short nID);
	static MMatchWorldItemDescMgr* GetInstance();
};

inline MMatchWorldItemDescMgr* MGetMatchWorldItemDescMgr() 
{ 
	return MMatchWorldItemDescMgr::GetInstance();
}


/////////////////////////////////////////////////////////////////////////////////////


#include "MMatchWorldItem.h"
#include "MMatchStage.h"

#define MAX_WORLDITEM_SPAWN		100

// 한 맵에서 가지고 있는 스폰정보
struct MMatchMapsWorldItemSpawnInfoSet
{
	MMatchWorldItemSpawnInfo	SoloSpawnInfo[MAX_WORLDITEM_SPAWN];
	MMatchWorldItemSpawnInfo	TeamSpawnInfo[MAX_WORLDITEM_SPAWN];
	int							m_nSoloSpawnCount;
	int							m_nTeamSpawnCount;
};

// 전체 맵의 스폰정보
class MMatchMapsWorldItemSpawnInfo
{
private:
	void ParseSpawnInfo(MXmlElement& element, int nMapID);
	void SetMapsSpawnInfo(int nMapID, char* szGameTypeID, int nItemID, float x, float y, float z, 
						  unsigned long int nCoolTime);
	bool ReadXml(const char* szFileName, int nMapID);
protected:
public:
	MMatchMapsWorldItemSpawnInfoSet		m_MapsSpawnInfo[MMATCH_MAP_COUNT];
	MMatchMapsWorldItemSpawnInfo();
	virtual ~MMatchMapsWorldItemSpawnInfo();
	
	bool Read();
	void Clear();
	static MMatchMapsWorldItemSpawnInfo* GetInstance();
};



inline MMatchMapsWorldItemSpawnInfo* MGetMapsWorldItemSpawnInfo() 
{ 
	return MMatchMapsWorldItemSpawnInfo::GetInstance();
}

#endif