#ifndef ZEQUIPMENTLISTBOX_H
#define ZEQUIPMENTLISTBOX_H

#include "ZPrerequisites.h"
#include "MListBox.h"

MBitmap* GetItemIconBitmap(MMatchItemDesc* pItemDesc, bool bSmallIcon = false);
bool ZGetIsCashItem(unsigned long nItemID);

class ZEquipmentListItem : public MListItem{
protected:
	MBitmap*			m_pBitmap;
	int					m_nAIID;		// 중앙은행에서 사용한다
	unsigned long		m_nItemID;
public:
	MUID				m_UID;
public:
	char	m_szName[256];
	char	m_szLevel[256];
//	char	m_szWeight[256];
//	char	m_szSlot[256];
	char	m_szPrice[256];
public:

//	ZEquipmentListItem(const MUID& uidItem, MBitmap* pBitmap, const char* szName, const char* szWeight, const char* szSlot, const char* szPrice)
	ZEquipmentListItem(const MUID& uidItem, const unsigned long nItemID, MBitmap* pBitmap, const char* szName, const char* szLevel, const char* szPrice)
	{
		m_nAIID = 0;
		m_nItemID = nItemID;
		m_pBitmap = pBitmap;
		m_UID = uidItem;
		strcpy_safe(m_szName, szName);
		strcpy_safe(m_szLevel, szLevel);
//		strcpy_safe(m_szWeight, szWeight);
//		strcpy_safe(m_szSlot, szSlot);
		strcpy_safe(m_szPrice, szPrice);
	}
	ZEquipmentListItem(const int nAIID, const unsigned long nItemID, MBitmap* pBitmap, const char* szName, const char* szLevel)
	{
		m_nAIID = nAIID;
		m_nItemID = nItemID;
		m_pBitmap = pBitmap;
		m_UID = MUID(0,0);
		strcpy_safe(m_szName, szName);
		strcpy_safe(m_szLevel, szLevel);
		m_szPrice[0] = 0;
	}

	ZEquipmentListItem(void)
	{
		m_nAIID = 0;
		m_nItemID = 0;
		m_pBitmap = NULL;
		m_UID = MUID(0,0);
		m_szName[0] = 0;
//		m_szWeight[0] = 0;
//		m_szSlot[0] = 0;
		m_szLevel[0] = 0;
		m_szPrice[0] = 0;
	}
	virtual const char* GetString(void)
	{
		return m_szName;
	}
	virtual const char* GetString(int i)
	{
		if(i==1) return m_szName;
		else if(i==2) return m_szLevel;
//		else if(i==2) return m_szWeight;
//		else if(i==3) return m_szSlot;
		else if(i==3) {
			if ( ZGetIsCashItem(GetItemID()) == true)
			{
				return ZMsg( MSG_WORD_CASH);
			}
			else
				return m_szPrice;
		}
		return NULL;
	}
	virtual bool GetDragItem(MBitmap** ppDragBitmap, char* szDragString, char* szDragItemString)
	{
		*ppDragBitmap = GetBitmap(0);
		strcpy(szDragString, m_szName);
		strcpy(szDragItemString, m_szName);

		return true;
	}
	virtual MBitmap* GetBitmap(int i)
	{
		if (i == 0) return m_pBitmap;
		return NULL;
	}
	MUID& GetUID() { return m_UID; }
	int GetAIID() { return m_nAIID; }
	unsigned long GetItemID() { return m_nItemID; }
};


class ZItemMenu;
//typedef void (*ZCB_ONDROP)(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);

class ZEquipmentListBox : public MListBox
{
protected:
	virtual bool IsDropable(MWidget* pSender);
//	virtual bool OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);

protected:
//	ZCB_ONDROP			m_pOnDropFunc;
	MWidget*			m_pDescFrame;
protected:
	ZItemMenu*			m_pItemMenu;	// ZEquipmentList가 Exclusive라서 Popup이 Child일때만 Show()가능하다
	ZItemMenu* GetItemMenu()	{ return m_pItemMenu; }

public:
	ZEquipmentListBox(const char* szName, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZEquipmentListBox(void);
	void AttachMenu(ZItemMenu* pMenu);

//	void Add(const MUID& uidItem, MBitmap* pIconBitmap, const char* szName, const char* szWeight, const char* szSlot, const char* szPrice);
//	void Add(const MUID& uidItem, MBitmap* pIconBitmap, const char* szName, int nWeight, MMatchItemSlotType nSlot, int nBountyPrice);

	void Add(const MUID& uidItem, unsigned long nItemID, MBitmap* pIconBitmap, const char* szName, const char* szLevel, const char* szPrice);
	void Add(const MUID& uidItem, unsigned long nItemID, MBitmap* pIconBitmap, const char* szName, int nLevel,int nBountyPrice);
	void Add(const int nAIID, unsigned long nItemID, MBitmap* pIconBitmap, const char* szName, int nLevel);

//	void SetOnDropCallback(ZCB_ONDROP pCallback) { m_pOnDropFunc = pCallback; }
	void SetDescriptionWidget(MWidget *pWidget)	{ m_pDescFrame = pWidget; }

public:
	#define MINT_EQUIPMENTLISTBOX	"EquipmentListBox"
	virtual const char* GetClassName(void){ return MINT_EQUIPMENTLISTBOX; }

	DWORD	m_dwLastMouseMove;
	int		m_nLastItem;
};

void ShopPurchaseItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);
void ShopSaleItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);
void CharacterEquipmentItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);

MListener* ZGetShopAllEquipmentFilterListener(void);
MListener* ZGetEquipAllEquipmentFilterListener(void);

MListener* ZGetShopSaleItemListBoxListener(void);
MListener* ZGetCashShopItemListBoxListener(void);
MListener* ZGetShopPurchaseItemListBoxListener(void);
MListener* ZGetEquipmentItemListBoxListener(void);
MListener* ZGetAccountItemListBoxListener(void);

#endif