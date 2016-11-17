#include "stdafx.h"
#include "ZMap.h"
#include "ZApplication.h"
#include "MComboBox.h"
#include "ZChannelRule.h"
#include "ZGameClient.h"
#include "Config.h"

void ZGetCurrMapPath(char* outPath, int maxlen)
{
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
	if (!pWidget) return false;

	MComboBox* pCombo = (MComboBox*)pWidget;
	pCombo->RemoveAll();

	if ((ZGetGameClient()) && (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())))
	{
		pCombo->Add("Mansion");

		return true;
	}

	MChannelRule* pRule = ZGetChannelRuleMgr()->GetCurrentRule();
	if (pRule == NULL) {
		mlog("::InitMaps() > No Current ChannelRule \n");
		return false;
	}

	MZFileSystem *pFS = ZApplication::GetFileSystem();
#define MAP_EXT	".rs"

	int nExtLen = (int)strlen(MAP_EXT);
	for (int i = 0; i<pFS->GetFileCount(); i++)
	{
		const char* szFileName = pFS->GetFileName(i);
		const MZFILEDESC* desc = pFS->GetFileDesc(i);
		int nLen = (int)strlen(szFileName);

		char MAPDIRECTORY[64];
		ZGetCurrMapPath(MAPDIRECTORY);

		if (_strnicmp(desc->m_szFileName, MAPDIRECTORY, strlen(MAPDIRECTORY)) == 0 &&
			nLen>nExtLen &&
			_stricmp(szFileName + nLen - nExtLen, MAP_EXT) == 0)
		{
			char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
			_splitpath_s(szFileName, drive, dir, fname, ext);

#if defined(_DEBUG) || defined(ADD_ALL_MAPS)
			pCombo->Add(fname);

			continue;
#endif

			bool bDuelMode = false;
			if (ZGetGameClient() && (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUEL))
				bDuelMode = true;

			if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_SKILLMAP)
			{
				char lwrfname[_MAX_FNAME];
				strcpy_safe(lwrfname, fname);
				_strlwr_s(lwrfname);
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
