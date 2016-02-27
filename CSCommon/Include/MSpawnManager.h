#ifndef MSPAWNMANAGER_H
#define MSPAWNMANAGER_H


#include "RTypes.h"
using namespace std;
#include <map>


#define RESPAWN_TICK_INTERVAL	1000


class MSpawnData {
	int			m_nSpawnID;			/// 자동생성되는 ID
	char		m_szSpawnName[24];

	int			m_nObjID;

	int			m_nMapID;
	int			m_nCellID;
	rvector		m_Pos;

	int			m_nMaxCount;
	int			m_nCurrentCount;	/// 현재 Spawn되어있는 Object Count

	unsigned long	m_nRespawnTime;		/// 초단위
	unsigned long	m_nLastSpawnTick;

public:
	MSpawnData(int nSpawnID, char* pszSpawnName) {
		m_nObjID=0; m_nMapID=0; m_nCellID=0; m_Pos=rvector(.0,.0,.0); m_nMaxCount=0; m_nCurrentCount=0; m_nRespawnTime=0; m_nLastSpawnTick=0;

		m_nSpawnID = nSpawnID;
		strcpy(m_szSpawnName, pszSpawnName);
	}
	int GetSpawnID() { return m_nSpawnID; }
	char* GetSpawnName() { return m_szSpawnName; }
	int GetObjID() { return m_nObjID; }
	void SetObjID(int nObjID) {	m_nObjID = nObjID; }
	int GetMapID() { return m_nMapID; }
	void SetMapID(int nMapID) { m_nMapID = nMapID; }
	int GetCellID() { return m_nCellID; }
	void SetCellID(int nCellID) { m_nCellID = nCellID; }
	rvector& GetPos() { return m_Pos; }
	void SetPos(const rvector& Pos) { m_Pos = Pos; }
	int GetMaxCount() { return m_nMaxCount; }
	void SetMaxCount(int nMaxCount) { m_nMaxCount = nMaxCount; }
	int GetCurrentCount() { return m_nCurrentCount; }
	void SetCurrentCount(int nCount) { m_nCurrentCount = nCount; }
	unsigned long GetRespawnTime() { return m_nRespawnTime; }
	void SetRespawnTime(unsigned long nSeconds) { m_nRespawnTime = nSeconds; }
	unsigned long GetLastSpawnTime() { return m_nLastSpawnTick; }
	void SetLastSpawnTime(unsigned long nTick) { m_nLastSpawnTick = nTick; }
};


class MSpawnList : public map<int, MSpawnData*> {};
class MZoneServer;


class MSpawnManager {
	int				m_nNextSpawnID;
	MSpawnList		m_SpawnList;
	unsigned long	m_nLastTickTime; 

	MZoneServer*	m_pZoneServer;

public:
	MSpawnManager();
	virtual ~MSpawnManager();

	void SetZoneServer(MZoneServer* pServer) { m_pZoneServer = pServer; }

	MSpawnData* FindSpawnByName(char* pszSpawnName);
	MSpawnData* FindSpawnByID(int nSpawnID) { 	
		MSpawnList::iterator i = m_SpawnList.find(nSpawnID);
		if(i!=m_SpawnList.end()) return (MSpawnData*)(*i).second;
		else return NULL;
	}

	int CreateSpawnID()							{ return m_nNextSpawnID++; }
	MSpawnData* AddSpawnData(char* szSpawnName, const rvector& Pos);

	bool ReadSpawnPosition(char* pszMapFile);
	bool ReadSpawnData(char* pszSpawnFile);

	void Spawn(MSpawnData* pSpawn, unsigned long nTime);
	void DeSpawn(int nSpawnID);

	bool IsNewTick(unsigned long int nTime) { 
		if (nTime-m_nLastTickTime >= RESPAWN_TICK_INTERVAL) return true;
		else return false;
	}
	void UpdateTick(unsigned long int nTime) { m_nLastTickTime = nTime; }
	void Tick(unsigned long int nTime);

	void LOG(const char *pFormat,...);
};


#endif