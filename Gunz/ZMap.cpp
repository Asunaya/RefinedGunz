#include "stdafx.h"
#include "ZMap.h"
#include "ZApplication.h"
#include "MComboBox.h"
#include "ZChannelRule.h"

#include "ZGameClient.h"

void ZGetCurrMapPath(char* outPath, int maxlen)
{
// 나중에 랜덤맵 구현할때까지 그냥 맵 하나만 사용
#ifdef _QUEST


	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
	{
		strcpy_safe(outPath, maxlen, PATH_QUEST_MAPS);
		return;
	}
#endif

	if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_QUEST)
	{
		strcpy_safe(outPath, maxlen, PATH_QUEST_MAPS);
	}
	else
	{
		strcpy_safe(outPath, maxlen, PATH_GAME_MAPS);
	}
}

bool InitMaps(MWidget *pWidget)
{
	if(!pWidget) return false;

	MComboBox* pCombo=(MComboBox*)pWidget;
	pCombo->RemoveAll();

	// 일단 임시 하드코딩(우에엥~ ㅠ.ㅠ)
	if ((ZGetGameClient()) && ( ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())))
	{
		pCombo->Add( "Mansion");
//		pCombo->Add( "Factory");
//		pCombo->Add( "Prison");
//		pCombo->Add( "Town");
//		pCombo->Add( "Castle");
//		pCombo->Add( "Dungeon");

		return true;
	}

	MChannelRule* pRule = ZGetChannelRuleMgr()->GetCurrentRule();
	if (pRule == NULL) {
		mlog("::InitMaps() > No Current ChannelRule \n");
		return false;
	}

	MZFileSystem *pFS=ZApplication::GetFileSystem();
#define MAP_EXT	".rs"

	int nExtLen = (int)strlen(MAP_EXT);
	for(int i=0; i<pFS->GetFileCount(); i++)
	{
		const char* szFileName = pFS->GetFileName(i);
		const MZFILEDESC* desc = pFS->GetFileDesc(i);
		int nLen = (int)strlen(szFileName);

		char MAPDIRECTORY[64];
		ZGetCurrMapPath(MAPDIRECTORY);

		if( _strnicmp(desc->m_szFileName,MAPDIRECTORY,strlen(MAPDIRECTORY))==0 && nLen>nExtLen && _stricmp(szFileName+nLen-nExtLen, MAP_EXT)==0 )
		{
			char drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
			_splitpath(szFileName,drive,dir,fname,ext);

/*
			if ((ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_DEVELOP) ||
				(ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_QUEST) ) 
			{
				pCombo->Add(fname);
			} 

#ifdef _QUEST
			// 퀘스트 모드이면 채널룰이랑 상관없다.
			else if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
			{
				pCombo->Add(fname);
			}
#endif
*/

#ifdef _DEBUG
			pCombo->Add(fname);

			continue;
#endif

			bool bDuelMode = false;
			if ( ZGetGameClient() && (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUEL))
				bDuelMode = true;

			if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_SKILLMAP)
			{
				char lwrfname[_MAX_FNAME];
				strcpy(lwrfname, fname);
				strlwr(lwrfname);
				if (strstr(lwrfname, "skill") || !_stricmp(fname, "Superflip"))
					pCombo->Add(fname);
			}
			else
			{
				if (pRule->CheckMap(fname, bDuelMode))
					pCombo->Add(fname);
			}
		}
	}

	return true;
}
