#ifndef _ZCLAN_H
#define _ZCLAN_H

#include "ZPrerequisites.h"
#include "MMatchClan.h"
#include "ZMessages.h"

#include "ZApplication.h"
#include "ZLanguageConf.h"

inline const char* ZGetClanGradeStr(const MMatchClanGrade nGrade)
{
	switch (nGrade)
	{
	case MCG_NONE:		
		return ZMsg(MSG_WORD_CLAN_NONE); break;

	case MCG_MASTER:	
		return ZMsg(MSG_WORD_CLAN_MASTER); break;

	case MCG_ADMIN:		
		return ZMsg(MSG_WORD_CLAN_ADMIN); break;

	case MCG_MEMBER:	
		return ZMsg(MSG_WORD_CLAN_MEMBER); break;

	default:
		return "";
	}

}


class ZClan
{
private:
public:
	ZClan();
	virtual ~ZClan();
public:
};


/*
	case ZCMD_PLAYERMENU_CLAN_INVITE:
		ZPostRequestJoinClan(ZGetMyUID(),ZGetMyInfo()->GetClanName(),pMenu->GetTargetName());
		return true;

	case ZCMD_PLAYERMENU_CLAN_KICK:
		ZPostRequestExpelClanMember(ZGetMyUID(),pMenu->GetTargetName());
		return true;

	case ZCMD_PLAYERMENU_CLAN_GRADE_MASTER:
		ZPostRequestChangeClanGrade(ZGetMyUID(),pMenu->GetTargetName(),MCG_MASTER);
		return true;

	case ZCMD_PLAYERMENU_CLAN_GRADE_ADMIN:
		ZPostRequestChangeClanGrade(ZGetMyUID(),pMenu->GetTargetName(),MCG_ADMIN);
		return true;

	case ZCMD_PLAYERMENU_CLAN_GRADE_MEMBER:
		ZPostRequestChangeClanGrade(ZGetMyUID(),pMenu->GetTargetName(),MCG_MEMBER);
		return true;

	case ZCMD_PLAYERMENU_CLAN_LEAVE:
		ZPostRequestLeaveClan(ZGetMyUID());
		return true;
*/

#endif