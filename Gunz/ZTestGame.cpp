#include "stdafx.h"
#include "ZApplication.h"
#include "ZEnemy.h"
#include "ZObject.h"
#include "ZObjectManager.h"
#include "Physics.h"
#include "ZReplay.h"
#include "ZTestGame.h"
#include "ZGameClient.h"
#include "MMath.h"
#include "ZMapDesc.h"

#include "ZModule_Skills.h"

void CreateTestGame(char *mapname, int nDummyCharacterCount, bool bShot,bool bAITest, int nParam1)
{
	ZApplication::GetStageInterface()->SetMapName(mapname);
	ZGetGameClient()->GetMatchStageSetting()->SetMapName(mapname);
	ZGetGameInterface()->SetState(GUNZ_GAME);

	if(!g_pGame) return;

	g_pGame->SetReadyState(ZGAME_READYSTATE_RUN);

	g_pGame->GetMatch()->SetRoundState(MMATCH_ROUNDSTATE_PLAY);

	MTD_CharInfo info;
	strcpy_safe(info.szName,"Maiet");
	info.szClanName[0]=0;
	info.nSex=1;
	info.nHP = 100;
	info.nAP = 100;

	for(int i=0;i<MMCIP_END;i++)
		info.nEquipedItemDesc[i]=0;
	//info.nEquipedItemDesc[MMCIP_MELEE] = 1;		// 구식단검
	info.nEquipedItemDesc[MMCIP_MELEE] = 2;		// 구식장검
	//	info.nEquipedItemDesc[MMCIP_MELEE] = 31;		// 용월랑
	//	info.nEquipedItemDesc[MMCIP_PRIMARY] = 5018;
	//	info.nEquipedItemDesc[MMCIP_PRIMARY] = 4514;	// 자우르스 B x2
	info.nEquipedItemDesc[MMCIP_PRIMARY] = 7006;	// LX 44
	info.nEquipedItemDesc[MMCIP_PRIMARY] = 9003;	// 월콤 L1 mk. II
	//	info.nEquipedItemDesc[MMCIP_SECONDARY] = 6001;	// 샷건
	info.nEquipedItemDesc[MMCIP_SECONDARY] = 8003;	// 머신건
	info.nEquipedItemDesc[MMCIP_CUSTOM1] = 400007;
	//info.nEquipedItemDesc[MMCIP_CUSTOM2] = 600005;	// 카노푸스의 눈

	g_pGame->CreateMyCharacter(&info);
	g_pGame->m_pMyCharacter->Revival();

	rvector pos=rvector(0,0,0), dir=rvector(0,1,0);

	ZMapSpawnData* pSpawnData = ZApplication::GetGame()->GetMapDesc()->GetSpawnManager()->GetSoloRandomData();
	if (pSpawnData != NULL)
	{
		pos = pSpawnData->m_Pos;
		dir = pSpawnData->m_Dir;
	}

	g_pGame->m_pMyCharacter->SetPosition(pos);
	g_pGame->m_pMyCharacter->SetDirection(dir);

#ifndef _PUBLISH

	// 테스트 코드
	ZModule_Skills *pModule = new ZModule_Skills;
	g_pGame->m_pMyCharacter->AddModule(pModule);
	const int ids[] = { 2 };
	pModule->Init(1,ids);

	if (nDummyCharacterCount != 0)
	{
		for (int i = 0; i < nDummyCharacterCount; i++)
		{

			ZCharacter *pChar= new ZDummyCharacter();
			pChar->SetUID(MUID(0,i+2));
			if (bShot) 
			{
				((ZDummyCharacter*)(pChar))->SetShotEnable(true);
			}

			g_pGame->m_CharacterManager.Add(pChar);

			rvector ranpos = rvector(RandomNumber(-500.0f, 500.0f), RandomNumber(-500.0f, 500.0f), 0);
			pChar->SetPosition(ranpos);
		}
	}
#endif // #ifndef _PUBLISH
}

