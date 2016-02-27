#include "stdafx.h"
#include "MMatchWorldItem.h"
#include "MMatchServer.h"
#include "MMatchStage.h"
#include "MMatchObject.h"
#include "MSharedCommandTable.h"
#include "MBlobArray.h"
#include "MZFileSystem.h"
#include "MUID.h"
#include "MMatchWorldItemDesc.h"
#include "RTypes.h"

MMatchWorldItemManager::MMatchWorldItemManager()
{
	m_pMatchStage = NULL;
	m_bStarted = false;
	m_nSpawnItemCount = 0;
	m_nLastTime = 0;
	m_nUIDGenerate = 0;
}

MMatchWorldItemManager::~MMatchWorldItemManager()
{
	ClearItems();
}

void MMatchWorldItemManager::ClearItems()
{
	m_nLastTime = 0;
	m_nUIDGenerate = 0;
	for (MMatchWorldItemMap::iterator itor = m_ItemMap.begin(); itor != m_ItemMap.end(); ++itor)
	{
		delete (*itor).second ;
	}
	m_ItemMap.clear();
}

void MMatchWorldItemManager::Clear()
{
	ClearItems();
	m_SpawnInfos.clear();
	m_nSpawnItemCount = 0;
}

void MMatchWorldItemManager::OnRoundBegin()
{
	ClearItems();
	SpawnInfoInit();
}

void MMatchWorldItemManager::SpawnInfoInit()
{
	if (m_pMatchStage == NULL) return;

	int nSize = (int)m_SpawnInfos.size();
	for (int i = 0; i < m_nSpawnItemCount; i++)
	{
		m_SpawnInfos[i].nElapsedTime = 0;
		m_SpawnInfos[i].bExist = false;
		m_SpawnInfos[i].bUsed = true;

		// 글래디에어터 모드는 Bullet를 스폰안시킨다
		if (IsGameRuleGladiator(m_pMatchStage->GetStageSetting()->GetGameType()))
		{
			MMatchWorldItemDesc* wi = MGetMatchWorldItemDescMgr()->GetItemDesc(m_SpawnInfos[i].nItemID);
			if ((wi) && (wi->m_nItemType == WIT_BULLET))
			{
				m_SpawnInfos[i].bUsed = false;
			}
		}
	}
}

void MMatchWorldItemManager::Update()
{
	if (!m_bStarted) return;
	if (m_pMatchStage == NULL) return;

	unsigned long int nNowTime = timeGetTime();
	unsigned long int nDeltaTime = 0;

	if (m_nLastTime != 0) nDeltaTime = nNowTime - m_nLastTime;	

	// 스폰할 꺼 있나 검색
	for (int i = 0; i < m_nSpawnItemCount; i++)
	{
		if ((m_SpawnInfos[i].bUsed == true) && (m_SpawnInfos[i].bExist == false))
		{
			m_SpawnInfos[i].nElapsedTime += nDeltaTime;

			if (m_SpawnInfos[i].nElapsedTime >= m_SpawnInfos[i].nCoolTime)
			{
				// 여기서 스폰
				Spawn(i);
			}
		}
	}

	// 이미 스폰된 아이템중 제거될 꺼 있나 검색
	for (MMatchWorldItemMap::iterator itor = m_ItemMap.begin(); itor != m_ItemMap.end(); )
	{
		MMatchWorldItem* pWorldItem = (*itor).second;

		// dynamic 월드아이템만 제거함
		if ((pWorldItem->nStaticSpawnIndex < 0) && (pWorldItem->nLifeTime > 0))
		{
			pWorldItem->nLifeTime -= (int)nDeltaTime;

			if (pWorldItem->nLifeTime <= 0)
			{
				RouteRemoveWorldItem(pWorldItem->nUID);

				delete pWorldItem;
				itor = m_ItemMap.erase(itor);
				continue;
			}
		}
		
		++itor;
	}
	


	m_nLastTime = nNowTime;

}

void MMatchWorldItemManager::RouteAllItems(MMatchObject* pObj)
{
	int nItemSize = (int)m_ItemMap.size();
	if (nItemSize <= 0) return;

	void* pItemArray = MMakeBlobArray(sizeof(MTD_WorldItem), nItemSize);
	

	int nIndex = 0;
	for (MMatchWorldItemMap::iterator itor = m_ItemMap.begin(); itor != m_ItemMap.end(); ++itor)
	{
		MMatchWorldItem* pWorldItem = (*itor).second;
		MTD_WorldItem* pNode = (MTD_WorldItem*)MGetBlobArrayElement(pItemArray, nIndex++);

		Make_MTDWorldItem(pNode, pWorldItem);

		MMatchRule* pRule = m_pMatchStage->GetRule();					// 갓뎀 -_-;
		if (pRule->GetGameType() == MMATCH_GAMETYPE_DUEL && pWorldItem->nItemID < 100)
			return;

	}

	// 난입한 넘에게 아이템 정보 보내준다
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_SPAWN_WORLDITEM, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterBlob(pItemArray, MGetBlobArraySize(pItemArray)));
	MEraseBlobArray(pItemArray);

	MMatchServer::GetInstance()->RouteToListener(pObj, pCmd);
}

void MMatchWorldItemManager::AddItem(const unsigned short nItemID, short nSpawnIndex, 
									 const float x, const float y, const float z)
{
	if (m_pMatchStage == NULL) return;
	m_nUIDGenerate++;

	MMatchWorldItem* pNewWorldItem = new MMatchWorldItem;

	// 세팅
	pNewWorldItem->nUID = m_nUIDGenerate;
	pNewWorldItem->nItemID = nItemID;
	pNewWorldItem->nStaticSpawnIndex = nSpawnIndex;
	pNewWorldItem->x = x;
	pNewWorldItem->y = y;
	pNewWorldItem->z = z;
	pNewWorldItem->nLifeTime = -1;
	for (int i = 0; i < WORLDITEM_EXTRAVALUE_NUM; i++) pNewWorldItem->nExtraValue[i] = 0;

	m_ItemMap.insert(MMatchWorldItemMap::value_type(m_nUIDGenerate, pNewWorldItem));

	// 방인원에게 라우팅
	RouteSpawnWorldItem(pNewWorldItem);

}

void MMatchWorldItemManager::AddItem(const unsigned short nItemID, short nSpawnIndex, 
									 const float x, const float y, const float z, int nLifeTime, 
									 int* pnExtraValues )
{
	if (m_pMatchStage == NULL) return;
	m_nUIDGenerate++;

	MMatchWorldItem* pNewWorldItem = new MMatchWorldItem;

	// 세팅
	pNewWorldItem->nUID					= m_nUIDGenerate;
	pNewWorldItem->nItemID				= nItemID;
	pNewWorldItem->nStaticSpawnIndex	= nSpawnIndex;
	pNewWorldItem->x					= x;
	pNewWorldItem->y					= y;
	pNewWorldItem->z					= z;
	pNewWorldItem->nLifeTime			= nLifeTime;

	for (int i = 0; i < WORLDITEM_EXTRAVALUE_NUM; i++)
	{
		pNewWorldItem->nExtraValue[i] = pnExtraValues[i];
	}

	m_ItemMap.insert(MMatchWorldItemMap::value_type(m_nUIDGenerate, pNewWorldItem));

	// 방인원에게 라우팅
	RouteSpawnWorldItem(pNewWorldItem);
}


void MMatchWorldItemManager::OnStageBegin(MMatchStageSetting* pStageSetting)
{
	if (m_pMatchStage == NULL) return;

	int nMapID = pStageSetting->GetMapIndex();
	bool bIsTeamPlay = pStageSetting->IsTeamPlay();

	Clear();
	
	m_SpawnInfos.clear();

	if ((nMapID < 0) || (nMapID >= MMATCH_MAP_COUNT)) return;
	if (MGetGameTypeMgr()->IsWorldItemSpawnEnable(pStageSetting->GetGameType()))
	{
		MMatchMapsWorldItemSpawnInfoSet* pSpawnInfoSet = &MGetMapsWorldItemSpawnInfo()->m_MapsSpawnInfo[nMapID];

		if (bIsTeamPlay)
		{
			int nSpawnCount = MGetMapsWorldItemSpawnInfo()->m_MapsSpawnInfo[nMapID].m_nTeamSpawnCount;
			if (nSpawnCount > 0)
			{
				m_SpawnInfos.reserve(nSpawnCount);
				
				for (int i = 0; i < nSpawnCount; i++)
				{
					m_SpawnInfos[i].x = pSpawnInfoSet->TeamSpawnInfo[i].x;
					m_SpawnInfos[i].y = pSpawnInfoSet->TeamSpawnInfo[i].y;
					m_SpawnInfos[i].z = pSpawnInfoSet->TeamSpawnInfo[i].z;
					m_SpawnInfos[i].nCoolTime = pSpawnInfoSet->TeamSpawnInfo[i].nCoolTime;
					m_SpawnInfos[i].nItemID = pSpawnInfoSet->TeamSpawnInfo[i].nItemID;
				}
			}
			m_nSpawnItemCount = nSpawnCount;
		}
		else
		{
			int nSpawnCount = MGetMapsWorldItemSpawnInfo()->m_MapsSpawnInfo[nMapID].m_nSoloSpawnCount;

			if (nSpawnCount > 0)
			{
				m_SpawnInfos.reserve(nSpawnCount);

				for (int i = 0; i < nSpawnCount; i++)
				{
					m_SpawnInfos[i].x = pSpawnInfoSet->SoloSpawnInfo[i].x;
					m_SpawnInfos[i].y = pSpawnInfoSet->SoloSpawnInfo[i].y;
					m_SpawnInfos[i].z = pSpawnInfoSet->SoloSpawnInfo[i].z;
					m_SpawnInfos[i].nCoolTime = pSpawnInfoSet->SoloSpawnInfo[i].nCoolTime;
					m_SpawnInfos[i].nItemID = pSpawnInfoSet->SoloSpawnInfo[i].nItemID;
				}
			}

			m_nSpawnItemCount = nSpawnCount;
		}
	}

	SpawnInfoInit();

	m_bStarted = true;
}

void MMatchWorldItemManager::OnStageEnd()
{
	m_bStarted = false;
}

void MMatchWorldItemManager::Spawn(int nSpawnIndex)
{
	m_SpawnInfos[nSpawnIndex].bExist = true;
	m_SpawnInfos[nSpawnIndex].nElapsedTime = 0;

	AddItem(m_SpawnInfos[nSpawnIndex].nItemID, nSpawnIndex, 
		    m_SpawnInfos[nSpawnIndex].x, m_SpawnInfos[nSpawnIndex].y, m_SpawnInfos[nSpawnIndex].z);
}

void MMatchWorldItemManager::DelItem(short nUID)
{
	MMatchWorldItemMap::iterator itor = m_ItemMap.find(nUID);
	if (itor != m_ItemMap.end())
	{
		MMatchWorldItem* pWorldItem = (*itor).second;

		int nSpawnIndex = pWorldItem->nStaticSpawnIndex;

		if ((nSpawnIndex >= 0) && (nSpawnIndex < m_nSpawnItemCount))
		{
			m_SpawnInfos[nSpawnIndex].bExist = false;
			m_SpawnInfos[nSpawnIndex].nElapsedTime = 0;
		}

		delete pWorldItem;
		m_ItemMap.erase(itor);
	}
}


bool MMatchWorldItemManager::Obtain(MMatchObject* pObj, short nItemUID, int* poutItemID, int* poutExtraValues)
{
	if (m_pMatchStage == NULL) return false;

	MMatchWorldItemMap::iterator itor = m_ItemMap.find(nItemUID);
	if (itor != m_ItemMap.end())
	{
		MMatchWorldItem* pWorldItem = (*itor).second;

		*poutItemID = pWorldItem->nItemID;

		for (int i = 0; i < WORLDITEM_EXTRAVALUE_NUM; i++)
		{
			poutExtraValues[i] = pWorldItem->nExtraValue[i];
		}

		RouteObtainWorldItem(pObj->GetUID(), (int)nItemUID);
		DelItem(nItemUID);

		return true;
	}

	return false;
}

void MMatchWorldItemManager::SpawnDynamicItem(MMatchObject* pObj, const int nItemID, const float x, const float y, const float z)
{
	if (m_pMatchStage == NULL) return;

	// TODO: 여기서 만들수 있는 ItemID인지 검증해야한다.

	AddItem(nItemID, -1, x, y, z);
}

void MMatchWorldItemManager::SpawnDynamicItem(MMatchObject* pObj, const int nItemID, const float x, const float y, const float z, 
											  int nLifeTime, int* pnExtraValues )
{
	if (m_pMatchStage == NULL) return;

	// TODO: 여기서 만들수 있는 ItemID인지 검증해야한다.

	AddItem(nItemID, -1, x, y, z, nLifeTime, pnExtraValues );
}

bool MMatchWorldItemManager::Create(MMatchStage* pMatchStage)
{
	m_pMatchStage = pMatchStage;
	return true;
}

void MMatchWorldItemManager::Destroy()
{

}

void MMatchWorldItemManager::RouteObtainWorldItem(const MUID& uidPlayer, int nWorldItemUID)
{
	// 먹었다고 라우팅
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_OBTAIN_WORLDITEM, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidPlayer));
	pCmd->AddParameter(new MCmdParamInt(nWorldItemUID));
	MMatchServer::GetInstance()->RouteToBattle(m_pMatchStage->GetUID(), pCmd);
}

void MMatchWorldItemManager::RouteRemoveWorldItem(int nWorldItemUID)
{
	// 없어졌다고 라우팅
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_REMOVE_WORLDITEM, MUID(0,0));
	pCmd->AddParameter(new MCmdParamInt(nWorldItemUID));
	MMatchServer::GetInstance()->RouteToBattle(m_pMatchStage->GetUID(), pCmd);
}

void MMatchWorldItemManager::RouteSpawnWorldItem(MMatchWorldItem* pWorldItem)
{
	MMatchRule* pRule = m_pMatchStage->GetRule();					// 갓뎀 -_-;
	if (pRule->GetGameType() == MMATCH_GAMETYPE_DUEL && pWorldItem->nItemID < 100)
		return;



	// 방인원에게 라우팅
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_SPAWN_WORLDITEM, MUID(0,0));

	void* pItemArray = MMakeBlobArray(sizeof(MTD_WorldItem), 1);
	MTD_WorldItem* pNode = (MTD_WorldItem*)MGetBlobArrayElement(pItemArray, 0);

	Make_MTDWorldItem(pNode, pWorldItem);

	pCmd->AddParameter(new MCommandParameterBlob(pItemArray, MGetBlobArraySize(pItemArray)));
	MEraseBlobArray(pItemArray);

	MMatchServer::GetInstance()->RouteToBattle(m_pMatchStage->GetUID(), pCmd);
}












