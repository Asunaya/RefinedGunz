#ifndef _ZITEMLIST_H
#define _ZITEMLIST_H

#include "ZPrerequisites.h"
#include "MMatchItem.h"
#include "MBaseItem.h"
#include <list>
#include <map>
using namespace std;


class ZMyItemNode : public MBaseItem
{
protected:
	unsigned long int		m_nItemID;
	MUID					m_UID;
	DWORD					m_dwWhenReceivedClock;

public:
	
	ZMyItemNode() : MBaseItem(), m_nItemID(0), m_UID(MUID(0,0)) { }

	virtual	~ZMyItemNode() { }
	void Create(MUID& uidItem, unsigned long int nItemID, 
				bool bIsRentItem=false, int nRentMinutePeriodRemainder=RENT_MINUTE_PERIOD_UNLIMITED)
	{
		m_UID = uidItem;
		m_nItemID = nItemID;
		m_bIsRentItem = bIsRentItem;
		m_nRentMinutePeriodRemainder = nRentMinutePeriodRemainder;
		m_dwWhenReceivedClock = timeGetTime();
	}
	void Create(unsigned long int nItemID, 
				bool bIsRentItem=false, int nRentMinutePeriodRemainder=RENT_MINUTE_PERIOD_UNLIMITED)
	{
		m_nItemID = nItemID;
		m_bIsRentItem = bIsRentItem;
		m_nRentMinutePeriodRemainder = nRentMinutePeriodRemainder;
		m_dwWhenReceivedClock = timeGetTime();
	}
	DWORD GetWhenReceivedClock()
	{
		return m_dwWhenReceivedClock;
	}

	unsigned long int GetItemID()	{ return m_nItemID; }
	MUID& GetUID()					{ return m_UID; }

};

typedef map<MUID, ZMyItemNode*> MITEMNODEMAP;
typedef map<int, ZMyItemNode*> MACCOUNT_ITEMNODEMAP;

#ifdef _QUEST_ITEM
#define MAX_ZQUEST_ITEM_COUNT 99
#define MIN_ZQUEST_ITEM_COUNT 0

// Description은 MQuestItem에 있는걸 사용.
class ZMyQuestItemNode : MBaseQuestItem
{
public :
	ZMyQuestItemNode() : m_nCount( 0 ), m_nItemID( 0 )
	{
	}

	virtual ~ZMyQuestItemNode()
	{
	}

	unsigned long int	GetItemID()	{ return m_nItemID; }
	int					GetCount()	{ return m_nCount; }
	MQuestItemDesc*		GetDesc()	{ return m_pDesc; }

	void Increase( const int nCount = 1 ) 
	{
		m_nCount += nCount;
		if( m_nCount >= MAX_ZQUEST_ITEM_COUNT )
			m_nCount = MAX_ZQUEST_ITEM_COUNT;

	}

	void Decrease( const int nCount = 1 )
	{
		m_nCount -= nCount;
		if( MIN_ZQUEST_ITEM_COUNT > m_nCount )
			m_nCount = MIN_ZQUEST_ITEM_COUNT;
	}

	void SetItemID( const unsigned long int nItemID )	{ m_nItemID = nItemID; }
	void SetCount( const int nCount )					{ m_nCount = nCount; }
	void SetDesc( MQuestItemDesc* pDesc )				{ m_pDesc = pDesc; }

	void Create( const unsigned long int nItemID, const int nCount, MQuestItemDesc* pDesc )
	{
		m_nItemID	= nItemID;
		m_nCount	= nCount;
		m_pDesc		= pDesc;
	}

public :
	unsigned long int	m_nItemID;
	int					m_nCount;
	MQuestItemDesc*		m_pDesc;
};


class ZMyQuestItemMap : public map< unsigned long int, ZMyQuestItemNode* >
{
public :
	ZMyQuestItemMap();
	~ZMyQuestItemMap();
	
	bool Add( const unsigned long int nItemID, ZMyQuestItemNode* pQuestItem );

	bool CreateQuestItem( const unsigned long int nItemID, const int nCount, MQuestItemDesc* pDesc );

	void Clear();

	ZMyQuestItemNode* Find( const unsigned long int nItemID )
	{
		ZMyQuestItemMap::iterator It;
		It = find( nItemID );
		if( end() != It )
		{
			return It->second;
		}

		return 0;
	}

private :

};
#endif

typedef map<unsigned long int, ZMyQuestItemNode*> MQUESTITEMNODEMAP;
/*
typedef map<MUID, unsigned long int> MITEMNODEMAP;
typedef map<int, unsigned long int> MACCOUNT_ITEMNODEMAP;
*/

enum ITEM_EQUIP_PARTS
{
	ITEM_EQUIP_PARTS_ALL = 0,
	ITEM_EQUIP_PARTS_HEAD,
	ITEM_EQUIP_PARTS_CHEST,
	ITEM_EQUIP_PARTS_HANDS,
	ITEM_EQUIP_PARTS_LEGS,
	ITEM_EQUIP_PARTS_FEET,
	ITEM_EQUIP_PARTS_MELEE,
	ITEM_EQUIP_PARTS_PRIMARY,
	ITEM_EQUIP_PARTS_SECONDARY,
	ITEM_EQUIP_PARTS_ITEM,

	ITEM_EQUIP_PARTS_END
};

class ZMyItemList
{
private:
protected:
	bool							m_bCreated;
	MUID							m_uidEquipItems[MMCIP_END];
	unsigned long int				m_nEquipItemID[MMCIP_END];
	MITEMNODEMAP					m_ItemMap;
	vector<MUID>					m_ItemIndexVectorEquip;
	vector<MUID>					m_ItemIndexVector;

	// Account Item 관련
	MACCOUNT_ITEMNODEMAP			m_AccountItemMap;
	vector<int>						m_AccountItemVector;

	void ClearItemMap();
	void ClearAccountItemMap();


#ifdef _QUEST_ITEM
protected :
	// quest item.
	ZMyQuestItemMap	m_QuestItemMap;

public :
	ZMyQuestItemMap&	GetQuestItemMap()	{ return m_QuestItemMap; }
	int					GetQuestItemCount()	{ return static_cast< int >( m_QuestItemMap.size() ); }

	void SetQuestItemsAll( MTD_QuestItemNode* pQuestItemNode, const int nQuestItemCount );
#endif


public:

	int		m_ListFilter;

public:
	ZMyItemList();
	virtual ~ZMyItemList();

	bool Create();
	void Destroy();
	void Clear();

	bool CheckAddType(int type);

	int GetItemCount() { return (int)m_ItemMap.size(); }

	void GetNamedComp( int nItemID, char* szBmpWidgetName, char* szBmpName, char* szLabelWidgetName);

	unsigned long int GetItemID(int nItemIndex);
	unsigned long int GetItemIDEquip(int nItemIndex);
	unsigned long int GetItemID(const MUID& uidItem);
	unsigned long int GetAccountItemID(int nPos);
	unsigned long int GetEquipedItemID(MMatchCharItemParts parts);

	ZMyItemNode* GetItem(int nItemIndex);
	ZMyItemNode* GetItemEquip(int nItemIndex);
	ZMyItemNode* GetItem(const MUID& uidItem);
	ZMyItemNode* GetEquipedItem(MMatchCharItemParts parts);
	ZMyItemNode* GetAccountItem(int nPos);


	MUID GetEquipedItemUID(MMatchCharItemParts parts);
	void SetEquipItemsAll(MUID* pnEquipItems);
	void SetEquipItemID(unsigned long int* pEquipItemID);
	void SetItemsAll(MTD_ItemNode* pItemNodes, const int nItemCount);
	bool IsCreated() { return m_bCreated; }

	MUID GetItemUID(int nItemIndex);
	MUID GetItemUIDEquip(int nItemIndex);

	void Serialize();
	void SerializeAccountItem();		// 창고 인터페이스를 동기화한다
	int GetEquipedTotalWeight();
	int GetEquipedHPModifier();
	int GetEquipedAPModifier();
	int GetMaxWeight();

	// AccountItem 관련
	void AddAccountItem(int nAIID, unsigned long int nItemID, int nRentMinutePeriodRemainder=RENT_MINUTE_PERIOD_UNLIMITED);
	void ClearAccountItems();
};

#endif