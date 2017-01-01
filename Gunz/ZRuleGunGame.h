#pragma once
#include "ZRule.h"

class ZRuleGunGame : public ZRule
{
	//TODO:
private:

public:
	ZRuleGunGame(ZMatch* pMatch);
	virtual ~ZRuleGunGame();
	virtual bool OnCommand(MCommand* pCommand);

	void SetPlayerWeapons(const MUID& uidPlayer, const UINT WeaponSetArray[]);
};