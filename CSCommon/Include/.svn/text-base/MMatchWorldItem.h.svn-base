#ifndef _MMATCHWORLDITEM_H
#define _MMATCHWORLDITEM_H



#include <vector>
#include <list>
#include <map>
using namespace std;

class MMatchStage;
class MMatchObject;
class MZFileSystem;


#define WORLDITEM_EXTRAVALUE_NUM		2

// 맵에 생성된 아이템
struct MMatchWorldItem
{
	unsigned short		nUID;
	unsigned short		nItemID;
	short				nStaticSpawnIndex;
	float				x;
	float				y;
	float				z;
	int					nLifeTime;			// 아이템 활성 시간( -1이면 무한 )

	union {
		struct {
		    int			nDropItemID;		// 만약 퀘스트일 경우 QuestItem 또는 일반 아이템의 ID
			int			nRentPeriodHour;	// 만약 일반 아이템일 경우 Item ID
		} ;
		int				nExtraValue[WORLDITEM_EXTRAVALUE_NUM];
	};
};

typedef map<unsigned short, MMatchWorldItem*> MMatchWorldItemMap;


// 맵의 스폰 정보
struct MMatchWorldItemSpawnInfo
{
	unsigned short		nItemID;
	unsigned long int	nCoolTime;
	unsigned long int	nElapsedTime;
	float x;
	float y;
	float z;
	bool				bExist;
	bool				bUsed;
};


class MMatchWorldItemManager
{
private:
	MMatchStage*						m_pMatchStage;
	MMatchWorldItemMap					m_ItemMap;				// 맵에 존재하고 있는 아이템 리스트

	vector<MMatchWorldItemSpawnInfo>	m_SpawnInfos;			// 맵의 스폰 아이템 정보
	int									m_nSpawnItemCount;		// 스폰 아이템 정보 개수
	unsigned long int					m_nLastTime;

	short								m_nUIDGenerate;
	bool								m_bStarted;

	void AddItem(const unsigned short nItemID, short nSpawnIndex, 
				 const float x, const float y, const float z);
	void AddItem(const unsigned short nItemID, short nSpawnIndex, 
				 const float x, const float y, const float z, int nLifeTime, int* pnExtraValues );
	void DelItem(short nUID);
	void Spawn(int nSpawnIndex);
	void Clear();
	void SpawnInfoInit();
	void ClearItems();

	void RouteSpawnWorldItem(MMatchWorldItem* pWorldItem);
	void RouteObtainWorldItem(const MUID& uidPlayer, int nWorldItemUID);
	void RouteRemoveWorldItem(int nWorldItemUID);
public:
	MMatchWorldItemManager();
	virtual ~MMatchWorldItemManager();

	// MMatchStage에서 관리하는 함수
	bool Create(MMatchStage* pMatchStage);
	void Destroy();

	void OnRoundBegin();
	void OnStageBegin(MMatchStageSetting* pStageSetting);
	void OnStageEnd();
	void Update();

	bool Obtain(MMatchObject* pObj, short nItemUID, int* poutItemID, int* poutExtraValues);
	void SpawnDynamicItem(MMatchObject* pObj, const int nItemID, const float x, const float y, const float z);
	void SpawnDynamicItem(MMatchObject* pObj, const int nItemID, const float x, const float y, const float z, 
						  int nLifeTime, int* pnExtraValues );
	void RouteAllItems(MMatchObject* pObj);

};



#endif