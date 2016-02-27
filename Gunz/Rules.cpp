#include "stdafx.h"
#include "Rules.h"
#include "Events.h"
#include "ZGameClient.h"

Rules::Rules()
{
	g_OnGameCreate.AddMethod(this, &Rules::OnGameCreate);
}

bool Rules::GetRoomTag(const char* szTag)
{
	return strstr(ZGetGameClient()->GetStageName(), szTag) ? true : false;
}

void Rules::OnGameCreate()
{
	HP = -1;
	AP = -1;
	NoFlip = false;
	NoElements = false;
	NoTipslash = false;
	NoGuns = false;
	NoMassiveThrow = false;
	NoMassiveEffect = false;
	VanillaMode = false;

	if (GetRoomTag("[glad]"))
	{
		MLog("Roomtag: [glad]\n");
		HP = 100;
		AP = 50;
		NoFlip = true;
		NoGuns = true;
	}

	if (GetRoomTag("[f]"))
	{
		NoFlip = true;
	}

	if (GetRoomTag("[skillmap]"))
	{
		NoFlip = true;
	}

	if (GetRoomTag("[notipslash]"))
	{
		NoTipslash = true;
	}

	if (GetRoomTag("[nm]"))
	{
		NoMassiveThrow = true;
	}

	if (GetRoomTag("[nm2]"))
	{
		NoMassiveEffect = true;
	}

	if (GetRoomTag("[hitbox]"))
	{
		ShowHitboxes = true;
	}
}

Rules g_Rules;