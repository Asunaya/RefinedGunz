#include "MObjectItem.h"

IMPLEMENT_RTTI(MObjectItem)
IMPLEMENT_RTTI(MItemContainer)

MItemCatalog		g_ItemCatalog;

MItemCatalog::MItemCatalog()
{

}
MItemCatalog::~MItemCatalog()
{
	Clear();
}


bool MItemCatalog::Read(const char* szFileName)
{
	MXmlDocument	xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(szFileName))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement		rootElement, chrElement, attrElement;
	char szTagName[256];
	char szName[256];

	rootElement = xmlIniData.GetDocumentElement();

	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, MICTOK_ITEM))
		{
			int id = -1;
			memset(szName, 0, 256);

			chrElement.GetAttribute(&id, MICTOK_ID);

			MItemDesc* pItemDesc = new MItemDesc();
			memset(pItemDesc, 0, sizeof(MItemDesc));

			// 필요한 항목이 있을때마다 추가한다.
			if (chrElement.FindChildNode(MICTOK_IDENT, &attrElement))
			{
				attrElement.GetAttribute(pItemDesc->szName, MICTOK_NAME);
			}
			if (chrElement.FindChildNode(MICTOK_CLASS, &attrElement))
			{
				

			}
			if (chrElement.FindChildNode(MICTOK_slot, &attrElement))
			{
				int nSlotType = -1;
				attrElement.GetAttribute(&nSlotType, MICTOK_SLOTTYPE);
				SetSlotType(pItemDesc->SlotTypes, nSlotType);
			}
			if (chrElement.FindChildNode(MICTOK_ATTACK, &attrElement))
			{

			}
			if (chrElement.FindChildNode(MICTOK_DEFENSE, &attrElement))
			{

			}
			if (chrElement.FindChildNode(MICTOK_STAT, &attrElement))
			{

			}
			if (chrElement.FindChildNode(MICTOK_EFFECT, &attrElement))
			{

			}

			insert(value_type(id, pItemDesc));
		}
	}

	xmlIniData.Destroy();
	return true;

}

void MItemCatalog::Clear()
{
	if (!empty())
	{
		MItemDesc* pItemDesc = (*begin()).second;
		delete pItemDesc; pItemDesc = NULL;
		erase(begin());
	}

}

MItemDesc* MItemCatalog::Find(int nDescID)
{
	iterator itor = find(nDescID);
	if (itor != end())
	{
		return (*itor).second;
	}
	return NULL;
}


MObjectItem::MObjectItem()
{

}


MObjectItem::MObjectItem(MUID& uid, MMap* pMap, rvector& Pos) : MObject(uid, pMap, Pos)
{
	m_pItemDesc = NULL;
}

MObjectItem::MObjectItem(MUID& uid, int nDescID, MMap* pMap, rvector& Pos) : MObject(uid, pMap, Pos)
{
	SetDescID(nDescID);
}

void MObjectItem::SetDescID(int nDescID)
{
	m_pItemDesc = g_ItemCatalog.Find(nDescID);
}


//////////////////////////////////////////////////////////////////////////////
// MItemContainer ////////////////////////////////////////////////////////////
MItemContainer::MItemContainer(MUID& uid, MMap* pMap, rvector& Pos) : MObjectItem(uid, pMap, Pos)
{

}


MItemContainer::MItemContainer()
{

}

MItemContainer::~MItemContainer()
{

}

bool MItemContainer::Create(int nMaxSpace)
{
	m_nMaxSpace = nMaxSpace;
	m_ObjectItemVector.reserve(nMaxSpace);
	for (int i = 0; i < nMaxSpace; i++) m_ObjectItemVector[i] = NULL;

	return true;
}

void MItemContainer::Destroy()
{

}

void MItemContainer::Clear()
{

}

bool MItemContainer::Push(int nIndex, MObjectItem* pObjectItem)
{
	if (((nIndex < 0) && (nIndex >= m_nMaxSpace)) || 
		(m_ObjectItemVector[nIndex] != NULL)) return false;
	
	m_ObjectItemVector[nIndex] = pObjectItem;

	return true;
}

void MItemContainer::ForcePush(int nIndex, MObjectItem* pObjectItem, MObjectItem* pOutObjectItem)
{
	if ((nIndex < 0) && (nIndex >= m_nMaxSpace)) return;

	pOutObjectItem = m_ObjectItemVector[nIndex];
	m_ObjectItemVector[nIndex] = pObjectItem;
}

MObjectItem* MItemContainer::Pop(int nIndex)
{
	if ((nIndex < 0) && (nIndex >= m_nMaxSpace)) return NULL;
	
	MObjectItem* pObjectItem = m_ObjectItemVector[nIndex];
	m_ObjectItemVector[nIndex] = NULL;
	return pObjectItem;
}

MObjectItem* MItemContainer::GetItem(int nIndex)
{
	if ((nIndex < 0) && (nIndex >= m_nMaxSpace)) return NULL;

	MObjectItem* pObjectItem = m_ObjectItemVector[nIndex];
	return pObjectItem;
}


//////////////////////////////////////////////////////////////////////////////
// MEquipContainer ///////////////////////////////////////////////////////////
MEquipContainer::MEquipContainer()
{
	m_EquipItemVector.reserve(MES_END);
	for (int i = 0; i < MES_END; i++) m_EquipItemVector[i] = NULL;
}
MEquipContainer::~MEquipContainer()
{

}
