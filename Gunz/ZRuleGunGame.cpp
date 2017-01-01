#include "stdafx.h"
#include "ZRuleGunGame.h"

ZRuleGunGame::ZRuleGunGame(ZMatch* pMatch) : ZRule(pMatch)
{
}

ZRuleGunGame::~ZRuleGunGame()
{
}

bool ZRuleGunGame::OnCommand(MCommand* pCommand)
{
	if (!ZGetGame())
		return false;

	switch (pCommand->GetID())
	{
		case MC_MATCH_GUNGAME_SEND_NEW_WEAPON:
		{
			UINT WeaponSet[3];
			MUID uidPlayer;
			pCommand->GetParameter(&uidPlayer,0,MPT_UID);
			pCommand->GetParameter(&WeaponSet[0], 1,MPT_UINT);
			pCommand->GetParameter(&WeaponSet[1], 2, MPT_UINT);
			pCommand->GetParameter(&WeaponSet[2], 3, MPT_UINT);

			SetPlayerWeapons(uidPlayer, WeaponSet);
		}
		break;
	}
	return false;
}

void ZRuleGunGame::SetPlayerWeapons(const MUID& uidPlayer, const UINT WeaponSetArray[])
{
	ZCharacter* pChar = ZGetCharacterManager()->Find(uidPlayer);
	
	pChar->m_Items.EquipItem(MMCIP_MELEE, WeaponSetArray[0]);
	pChar->m_Items.EquipItem(MMCIP_PRIMARY, WeaponSetArray[1]);
	pChar->m_Items.EquipItem(MMCIP_SECONDARY, WeaponSetArray[2]);

	if (pChar == ZGetGame()->m_pMyCharacter)
	{
		pChar->ChangeWeapon(pChar->GetItems()->GetSelectedWeaponParts() == MMCIP_MELEE ? MMCIP_PRIMARY : MMCIP_MELEE);
	}
}