#include "stdafx.h"

#include "ZCharacterItem.h"
#include "crtdbg.h"

ZCharacterItem::ZCharacterItem()
{
	memset(&m_Items, 0, sizeof(m_Items));
	m_nSelectedWeapon = MMCIP_END;

	// 우선 임의로 아이템 지급

#ifndef _PUBLISH
	EquipItem(MMCIP_MELEE,		2);				// knife
	EquipItem(MMCIP_PRIMARY,	4);			// rifle
	EquipItem(MMCIP_SECONDARY,	7);
	EquipItem(MMCIP_CUSTOM1,	11);			// grenade

//	SelectWeapon(MMCIP_PRIMARY);

	m_Items[MMCIP_PRIMARY].InitBullet(999);
	m_Items[MMCIP_SECONDARY].InitBullet(999);
	m_Items[MMCIP_CUSTOM1].InitBullet(10);

#endif
}
ZCharacterItem::~ZCharacterItem()
{

}

bool ZCharacterItem::Confirm(MMatchCharItemParts parts, MMatchItemDesc* pDesc)
{
	if (pDesc == NULL) return false;


	switch (parts)
	{
	case MMCIP_HEAD:
	case MMCIP_CHEST:
	case MMCIP_HANDS:
	case MMCIP_LEGS:
	case MMCIP_FEET:
	case MMCIP_FINGERL:
	case MMCIP_FINGERR:
		{
			if (pDesc->m_nType != MMIT_EQUIPMENT) 
			{
				return false;
			}
		}
		break;		
	case MMCIP_MELEE:
		{
			if (pDesc->m_nType != MMIT_MELEE) 
			{
				return false;
			}
		}
		break;
	case MMCIP_PRIMARY:
	case MMCIP_SECONDARY:
		{
			if (pDesc->m_nType != MMIT_RANGE) 
			{
				return false;
			}
		}
		break;
	case MMCIP_CUSTOM1:
	case MMCIP_CUSTOM2:
		{
			if (pDesc->m_nType != MMIT_CUSTOM) 
			{
				return false;
			}
		}
	}

	return true;
}

bool ZCharacterItem::EquipItem(MMatchCharItemParts parts, int nItemDescID)
{
	if (nItemDescID == 0) 
	{
		m_Items[parts].Create(MUID(0,0), NULL, 0);
		return true;
	}

	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemDescID);

	if (pDesc == NULL)
	{
//		_ASSERT(0);
		return false;
	}
	if (!Confirm(parts, pDesc)) 
	{
//		_ASSERT(0);
		return false;
	}

	m_Items[parts].Create(MUID(0,0), pDesc, 1);

	return true;
}

void ZCharacterItem::SelectWeapon(MMatchCharItemParts parts)
{
	_ASSERT( (parts == MMCIP_MELEE) || (parts == MMCIP_PRIMARY) || (parts == MMCIP_SECONDARY) ||
		 (parts == MMCIP_CUSTOM1) || (parts == MMCIP_CUSTOM2) );
	if (!IsWeaponItem(parts)) return;

	if (m_nSelectedWeapon == parts) return;
	m_nSelectedWeapon = parts;
}

bool ZCharacterItem::Reload()
{
	if ((m_nSelectedWeapon != MMCIP_PRIMARY) && (m_nSelectedWeapon != MMCIP_SECONDARY)) return false;

	ZItem* pItem = &m_Items[m_nSelectedWeapon];

	if (pItem->GetDesc() == NULL) return false;
	return pItem->Reload();
}


ZItem* ZCharacterItem::GetSelectedWeapon()
{
	if(m_nSelectedWeapon<MMCIP_MELEE || m_nSelectedWeapon>MMCIP_CUSTOM2) return NULL;
	return &m_Items[(int)m_nSelectedWeapon]; 
}

bool ZCharacterItem::IsWeaponItem(MMatchCharItemParts parts)
{
	if ((parts == MMCIP_MELEE) ||
		(parts == MMCIP_PRIMARY) ||
		(parts == MMCIP_SECONDARY) ||
		(parts == MMCIP_CUSTOM1 ) ||
		(parts == MMCIP_CUSTOM2 )
		) return true;

	return false;
}

bool ZCharacterItem::Save(ZFile *file)
{
	size_t n;
	for(int i=0;i<MMCIP_END;i++)
	{
		ZItem *pItem=GetItem(MMatchCharItemParts(i));
		int nBullet=pItem->GetBullet();
		n=zfwrite(&nBullet,sizeof(nBullet),1,file);
		if(n!=1) return false;

		int nBulletAMagazine=pItem->GetBulletAMagazine();
		n=zfwrite(&nBulletAMagazine,sizeof(nBulletAMagazine),1,file);
		if(n!=1) return false;
	}
	return true;
}

bool ZCharacterItem::Load(ZFile *file)
{
	size_t n;
	for(int i=0;i<MMCIP_END;i++)
	{
		ZItem *pItem=GetItem(MMatchCharItemParts(i));
		int nBullet;
		n=zfread(&nBullet,sizeof(nBullet),1,file);
		pItem->SetBullet(nBullet);
		if(n!=1) return false;

		int nBulletAMagazine;
		n=zfread(&nBulletAMagazine,sizeof(nBulletAMagazine),1,file);
		pItem->SetBulletAMagazine(nBulletAMagazine);
		if(n!=1) return false;
	}

	return true;
}