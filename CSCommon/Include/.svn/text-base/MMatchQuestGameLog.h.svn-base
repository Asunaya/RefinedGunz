#ifndef _MMATCH_QUESTGAMELOG_H
#define _MMATCH_QUESTGAMELOG_H


typedef map< unsigned long int, int >	QItemLogMap;
typedef QItemLogMap::iterator			QItemLogMapIter;

typedef pair< int, unsigned long >		SacriItemOwnerPair;
typedef vector< SacriItemOwnerPair >	SacriSlotLogVec;
typedef SacriSlotLogVec::iterator		SacrislotLogVecIter;


class MMatchObject;


class MQuestPlayerLogInfo
{
public :
	MQuestPlayerLogInfo() : m_nCID( 0 )
	{
		m_UniqueItemList.clear();
	}
	~MQuestPlayerLogInfo() 
	{}

	void AddUniqueItem( const unsigned long int nItemID, int nCount );

	void ClearQItemInfo() 
	{
		m_UniqueItemList.clear();
	}

	QItemLogMap&	GetUniqueItemList()	{ return m_UniqueItemList; }
	int				GetCID()			{ return m_nCID; }

	void SetCID( const int nCID )		{ m_nCID = nCID; }

private :
	int			m_nCID;
	QItemLogMap	m_UniqueItemList;
};



class MMatchQuestGameLogInfoManager : public map< MUID, MQuestPlayerLogInfo* >
{
private :
	MQuestPlayerLogInfo* Find( const MUID& uidPlayer );
public :
	MMatchQuestGameLogInfoManager();
	virtual ~MMatchQuestGameLogInfoManager();

	void AddQuestPlayer( const MUID& uidPlayer, MMatchObject* pPlayer );
	bool AddRewardQuestItemInfo( const MUID& uidPlayer, MQuestItemMap* pObtainQuestItemList );
	bool AddRewardZItemInfo( const MUID& uidPlayer, MQuestRewardZItemList* pObtainZItemList );
	void Clear();

	void SetMasterCID( const int nMasterCID )		{ m_nMasterCID = nMasterCID; }
	void SetStageName( const char* pszStageName );
	void SetStartTime( const DWORD dwStartTime )	{ m_dwStartTime = dwStartTime; }
	void SetEndTime( const DWORD dwEndTime )		{ m_dwEndTime = dwEndTime; }
	void SetScenarioID( const int nScenarioID )		{ m_nScenarioID = nScenarioID; }

	bool PostInsertQuestGameLog();
private :
	int		m_nMasterCID;
	int		m_nScenarioID;
	DWORD	m_dwStartTime;
	DWORD	m_dwEndTime;
	char	m_szStageName[ 64 ];
	int		m_nTotalRewardQItemCount;	// 하나의 퀘스트 게임동안 획득한 총 아이템 수( 일반 퀘스트 아이템 + 유니크 아이템 )

	
};

#endif