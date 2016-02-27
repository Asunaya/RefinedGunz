#include "winsock2.h"
#include "MXml.h"
#include "MSpawnManager.h"
#include "MZoneServer.h"


#define MTOK_SPAWNPOSITIONLIST	"SPAWNPOSITIONLIST"
#define MTOK_SPAWNPOSITION		"SPAWNPOSITION"
#define MTOK_NAME				"name"
#define MTOK_POSITION			"POSITION"
#define MTOK_SPAWNDATA			"SPAWNDATA"
#define MTOK_OBJECTID			"OBJECTID"
#define MTOK_MAXCOUNT			"MAXCOUNT"
#define MTOK_RESPAWNTIME		"RESPAWNTIME"


MSpawnManager::MSpawnManager()
{ 
	m_nNextSpawnID=1; m_pZoneServer=NULL; m_nLastTickTime=0; 
}

MSpawnManager::~MSpawnManager()
{
	while(m_SpawnList.begin() != m_SpawnList.end()) {
		MSpawnList::iterator i = m_SpawnList.begin();
		MSpawnData* pSpawnData = (MSpawnData*)((*i).second);
		delete pSpawnData;
		m_SpawnList.erase(i);
	}
}

MSpawnData* MSpawnManager::FindSpawnByName(char* pszSpawnName)
{
	for(MSpawnList::iterator i=m_SpawnList.begin(); i!=m_SpawnList.end(); i++){
		MSpawnData* pSpawnData = (MSpawnData*)((*i).second);
		if (strcmp(pSpawnData->GetSpawnName(), pszSpawnName) == 0)
			return pSpawnData;		
	}
	return NULL;
}

MSpawnData* MSpawnManager::AddSpawnData(char* szSpawnName, const rvector& Pos)
{
	int nSpawnID = CreateSpawnID();
	MSpawnData* pSpawnData = new MSpawnData(nSpawnID, szSpawnName);
	pSpawnData->SetPos(Pos);
	m_SpawnList.insert(MSpawnList::value_type(nSpawnID, pSpawnData));
	return pSpawnData;
}

bool MSpawnManager::ReadSpawnPosition(char* pszMapFile)
{
	MXmlDocument	aXml;
	aXml.Create();
	if (!aXml.LoadFromFile(pszMapFile)) {
		aXml.Destroy();
		return false;
	}

	char szBuf[65535];
	ZeroMemory(szBuf, 65535);

	MXmlElement aRoot;
	aRoot = aXml.GetDocumentElement();

	int nTotalRead = 0;

	int nSpawnListCount = aRoot.GetChildNodeCount();

	for (int i=0; i<nSpawnListCount; i++)
	{
		MXmlElement aSpawnList = aRoot.GetChildNode(i);
		aSpawnList.GetTagName(szBuf);
		if (szBuf[0] == '#') continue;

		if (!stricmp(szBuf, MTOK_SPAWNPOSITIONLIST)) {
			int nSpawnCount = aSpawnList.GetChildNodeCount();
			for (int j=0; j<nSpawnCount; j++) {
				MXmlElement aSpawn = aSpawnList.GetChildNode(j);
				aSpawn.GetTagName(szBuf);
				if (szBuf[0] == '#') continue;

				if (!strcmp(szBuf, MTOK_SPAWNPOSITION))
				{
					char szSpawnName[128];	ZeroMemory(szSpawnName, 128);
					rvector Pos;

					if (!aSpawn.GetAttribute(szSpawnName, MTOK_NAME)) continue;
            
					int nFieldCount = aSpawn.GetChildNodeCount();
					for (int k=0; k<nFieldCount; k++) {
						MXmlElement aField = aSpawn.GetChildNode(k);
						aField.GetTagName(szBuf);
						if (szBuf[0] == '#') continue;

						if (!stricmp(szBuf, MTOK_POSITION)) {
							aField.GetContents(szBuf);
							sscanf(szBuf, "%f %f %f", &Pos.x, &Pos.y, &Pos.z);
						}
					}

					MSpawnData* pSpawnData = FindSpawnByName(szSpawnName);
					if (pSpawnData) {
						LOG("Duplicated spawn name '%s' found in '%s' file", szSpawnName, pszMapFile);
					} else {
						AddSpawnData(szSpawnName, Pos);
						nTotalRead++;
					}
				}
			}
		}	// SPAWNLIST
	}

	aXml.Destroy();

	LOG("Read %d spawn position in map '%s'", nTotalRead, pszMapFile);

	return true;
}

bool MSpawnManager::ReadSpawnData(char* pszSpawnFile)
{
	MXmlDocument	aXml;
	aXml.Create();
	if (!aXml.LoadFromFile(pszSpawnFile)) {
		aXml.Destroy();
		return false;
	}

	char szBuf[65535];
	ZeroMemory(szBuf, 65535);

	MXmlElement aRoot;
	aRoot = aXml.GetDocumentElement();
	
	int nTotalRead = 0;

	int nSpawnCount = aRoot.GetChildNodeCount();
	for (int i=0; i<nSpawnCount; i++)
	{
		MXmlElement aSpawn = aRoot.GetChildNode(i);
		aSpawn.GetTagName(szBuf);
		if (szBuf[0] == '#') continue;

		if (!stricmp(szBuf, MTOK_SPAWNDATA)) {
			char szSpawnName[128];	ZeroMemory(szSpawnName, 128);
			if (!aSpawn.GetAttribute(szSpawnName, "name")) continue;

			int nObjectID = 0;
			int nMaxCount = 0;
			int nRespawnTime = 0;

			int nFieldCount = aSpawn.GetChildNodeCount();
			for (int j=0; j<nFieldCount; j++) {
				MXmlElement aField = aSpawn.GetChildNode(j);
				aField.GetTagName(szBuf);
				if (szBuf[0] == '#') continue;

				if (!stricmp(szBuf, MTOK_OBJECTID))
					aField.GetContents(&nObjectID);
				else if (!stricmp(szBuf, MTOK_MAXCOUNT))
					aField.GetContents(&nMaxCount);
				else if (!stricmp(szBuf, MTOK_RESPAWNTIME))
					aField.GetContents(&nRespawnTime);
			}

			MSpawnData* pSpawnData = FindSpawnByName(szSpawnName);
			if (pSpawnData) {
				pSpawnData->SetObjID(nObjectID);
				pSpawnData->SetMaxCount(nMaxCount);
				pSpawnData->SetRespawnTime(nRespawnTime);
				nTotalRead++;
			} else {
				LOG("SpawnData '%s' has no SpawnPosition. (%s)", szSpawnName, pszSpawnFile);
			}
		}
	}

	aXml.Destroy();

	LOG("Read %d spawn data in file '%s'", nTotalRead, pszSpawnFile);

	return true;
}

void MSpawnManager::Spawn(MSpawnData* pSpawn, unsigned long nTime)
{
	m_pZoneServer->SpawnObject(pSpawn->GetSpawnID(), pSpawn->GetObjID(), pSpawn->GetMapID(), pSpawn->GetCellID(), pSpawn->GetPos());

	pSpawn->SetCurrentCount(pSpawn->GetCurrentCount()+1);
	pSpawn->SetLastSpawnTime(nTime);

	LOG("Spawn(%s) excuted (%d/%d)", pSpawn->GetSpawnName(), pSpawn->GetCurrentCount(), pSpawn->GetMaxCount());
}

void MSpawnManager::DeSpawn(int nSpawnID)
{
	MSpawnData* pSpawn = FindSpawnByID(nSpawnID);
	if (pSpawn)
		pSpawn->SetCurrentCount(max(0,pSpawn->GetCurrentCount()-1));
}

void MSpawnManager::Tick(unsigned long int nTime)
{
	for(MSpawnList::iterator i=m_SpawnList.begin(); i!=m_SpawnList.end(); i++){
		MSpawnData* pSpawn = (MSpawnData*)((*i).second);
		if ( (nTime - pSpawn->GetLastSpawnTime() >= pSpawn->GetRespawnTime()) &&
			 (pSpawn->GetCurrentCount() < pSpawn->GetMaxCount()) ) 
		{
			if ((pSpawn->GetRespawnTime() == 0) && (pSpawn->GetLastSpawnTime()!=0)) 
				continue;	// Respawntime==0 이면 단 1회만 Spawn

			Spawn(pSpawn, nTime);
		}
	}
	
	UpdateTick(nTime);	
}

void MSpawnManager::LOG(const char *pFormat,...)
{
	va_list args;
	static char temp[1024];

	va_start(args, pFormat);
	vsprintf(temp, pFormat, args);
	m_pZoneServer->Log(temp);
	va_end(args);
}
