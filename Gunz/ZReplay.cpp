#include "stdafx.h"
#include "ZApplication.h"
#include "ZFile.h"
#include "ZGameClient.h"
#include "ZReplay.h"
#include "ZGame.h"
#include "ZNetCharacter.h"
#include "ZMyCharacter.h"
#include "ZPost.h"
#include "MMatchUtil.h"
#include "ZRuleDuel.h"

#include "RGMain.h"

bool g_bTestFromReplay = false;

bool CreateReplayGame(char *filename)
{
	static char szLastFile[256] = "";
	char szBuf[256];
	if (filename != NULL) strcpy_safe(szBuf, filename);
	else strcpy_safe(szBuf, szLastFile);

	if (filename != NULL) strcpy_safe(szLastFile, filename);

	ZReplayLoader loader;
	if (!loader.Load(szBuf)) return false;
	g_pGame->OnLoadReplay(&loader);

	return true;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZReplayLoader::ZReplayLoader() : m_fGameTime(0.0f)
{
	Version.Server = SERVER_NONE;
	Version.nVersion = 0;
	Version.nSubVersion = 0;
	pFile = nullptr;
	FileBuffer = nullptr;
	memset(&m_StageSetting, 0, sizeof(REPLAY_STAGE_SETTING_NODE));
}

ZReplayLoader::~ZReplayLoader()
{
	delete[] FileBuffer;
}

bool ZReplayLoader::Load(const char* filename)
{
	ZFile* file = zfopen(filename);

	if(!file) return false;

	const int bufsize = 10000000;
	FileBuffer = new BYTE[bufsize];

	file->Read(FileBuffer, bufsize);

	zfclose(file);

	file = zfopen(filename);

	pFile = file;
	
	if (!LoadHeader()) return false;
	if (!LoadStageSetting()) return false;
	ChangeGameState();

	if (!LoadStageSettingEtc()) return false;

	if (!LoadCharInfo()) return false;
	if (!LoadCommandStream()) return false;

	zfclose(file);
	return true;
}

void ZReplayLoader::ChangeGameState()
{
	MSTAGE_SETTING_NODE stageSetting;
	memset(&stageSetting, 0, sizeof(MSTAGE_SETTING_NODE));
	ConvertStageSettingNode(&m_StageSetting, &stageSetting);
	ZGetGameClient()->GetMatchStageSetting()->UpdateStageSetting(&stageSetting);
	ZApplication::GetStageInterface()->SetMapName(ZGetGameClient()->GetMatchStageSetting()->GetMapName());
	ZGetGameInterface()->SetState(GUNZ_GAME);
	ZGetCharacterManager()->Clear();
	ZGetObjectManager()->Clear();
}

bool ZReplayLoader::LoadHeader()
{
	unsigned int version = 0;
	unsigned int header;
	int nRead;

	char szServer[32] = "Unknown";

	nRead = zfread(&header, sizeof(header), 1, pFile);
	if(nRead==0) return false;

	if (header == RG_REPLAY_MAGIC_NUMBER)
	{
		Version.Server = SERVER_REFINED_GUNZ;
		strcpy_safe(szServer, "Refined Gunz");
	}
	else if(header != GUNZ_REC_FILE_ID)
	{
		Version.Server = SERVER_NONE;
		return false;
	}

	nRead = zfread(&version, sizeof(version), 1, pFile);
	if (!nRead)// || ( version > GUNZ_REC_FILE_VERSION))
		return false;

	Version.nVersion = version;

	if (Version.nVersion >= 7)
	{
		Version.Server = SERVER_FREESTYLE_GUNZ;
		strcpy_safe(szServer, "Freestyle Gunz");
	}
	else
	{
		Version.Server = SERVER_OFFICIAL;
		strcpy_safe(szServer, "Official");
	}

	MLog("Replay header loaded -- Server: %s, version: %d\n", szServer, Version.nVersion);

	return true;
}

#define COPY_SETTING(member) m_StageSetting.member = Setting.member;
bool ZReplayLoader::LoadStageSetting()
{
	switch (Version.Server)
	{
	case SERVER_OFFICIAL:
	{
					 REPLAY_STAGE_SETTING_NODE_OLD Setting;
					 int nRead = zfread(&Setting, sizeof(REPLAY_STAGE_SETTING_NODE_OLD), 1, pFile);
					 if (nRead == 0) return false;

					 COPY_SETTING(uidStage);
					 m_StageSetting.szStageName[0] = 0;
					 strcpy_safe(m_StageSetting.szMapName, Setting.szMapName);
					 COPY_SETTING(nMapIndex);
					 COPY_SETTING(nGameType);
					 COPY_SETTING(nRoundMax);
					 COPY_SETTING(nLimitTime);
					 COPY_SETTING(nLimitLevel);
					 COPY_SETTING(nMaxPlayers);
					 COPY_SETTING(bTeamKillEnabled);
					 COPY_SETTING(bTeamWinThePoint);
					 COPY_SETTING(bForcedEntryEnabled);
	}
		break;
	case SERVER_REFINED_GUNZ:
	{
						 int nRead = zfread(&m_StageSetting, sizeof(REPLAY_STAGE_SETTING_NODE), 1, pFile);
						 if (nRead == 0) return false;

						 MLog("Read stage setting: stage name %s\n", m_StageSetting.szStageName);
	}
		break;
	case SERVER_FREESTYLE_GUNZ:
	{
								  REPLAY_STAGE_SETTING_NODE_FG Setting;
								  int nRead = pFile->Read(Setting);

								  COPY_SETTING(uidStage);
								  strcpy_safe(m_StageSetting.szStageName, Setting.szStageName);
								  strcpy_safe(m_StageSetting.szMapName, Setting.szMapName);
								  COPY_SETTING(nMapIndex);
								  COPY_SETTING(nGameType);
								  COPY_SETTING(nRoundMax);
								  COPY_SETTING(nLimitTime);
								  COPY_SETTING(nLimitLevel);
								  COPY_SETTING(nMaxPlayers);
								  COPY_SETTING(bTeamKillEnabled);
								  COPY_SETTING(bTeamWinThePoint);
								  COPY_SETTING(bForcedEntryEnabled);

								  // Position 527 holds the first char of the first ZCharacter's name in the proper V7 format, but holds an item count value in the interim V7 format.
								  int offset = 527;
								  if (m_StageSetting.nGameType == MMATCH_GAMETYPE_DUEL)
									  offset += sizeof(MTD_DuelQueueInfo);
								  if (Version.nVersion == 7 && (FileBuffer[offset] == 0x00 || FileBuffer[offset] == 0x01))
								  {
									  Version.nSubVersion = 1;
									  MLog("FG replay subversion 1\n");
								  }
	}
		break;
	};

	return true;
}
#undef COPY_SETTING

bool ZReplayLoader::LoadStageSettingEtc()
{
	if (Version.Server == SERVER_OFFICIAL && Version.nVersion < 4)
		return true;

	if(m_StageSetting.nGameType==MMATCH_GAMETYPE_DUEL)
	{
		ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		int nRead = zfread(&pDuel->QInfo,sizeof(MTD_DuelQueueInfo), 1, pFile);
		if(nRead==0) return false;
	}
	return true;
}


bool ZReplayLoader::LoadCharInfo()
{
	int nRead;

	// character info
	int nCharacterCount;
	zfread(&nCharacterCount, sizeof(nCharacterCount), 1, pFile);

	for(int i = 0; i < nCharacterCount; i++)
	{
		bool bHero;
		nRead = zfread(&bHero, sizeof(bHero), 1, pFile);
		if(nRead != 1) return false;

		MTD_CharInfo info;

		if (Version.Server == SERVER_OFFICIAL)
		{
			if (Version.nVersion < 2)
			{
				nRead = zfread(&info, sizeof(info)-4, 1, pFile);
				if (nRead != 1) return false;
				info.nClanCLID = 0;
			}
			else
			{
				nRead = zfread(&info, sizeof(info), 1, pFile);
				if (nRead != 1) return false;
			}
		}
		else if (Version.Server == SERVER_FREESTYLE_GUNZ)
		{
			if (Version.nVersion == 9)
			{
				MTD_CharInfo_FG_V9 charinfo;
				pFile->Read(charinfo);
				memcpy(&info, &charinfo, sizeof(info)); // Lazy, fix sometime
			}
			else if (Version.nVersion == 7)
			{
				if (Version.nSubVersion == 0)
				{
					MTD_CharInfo_FG_V7_0 charinfo;
					pFile->Read(charinfo);
					memcpy(&info, &charinfo, sizeof(info));
				}
				else if (Version.nSubVersion == 1)
				{
					MTD_CharInfo_FG_V7_1 charinfo;
					pFile->Read(charinfo);
					memcpy(&info, &charinfo, sizeof(info));
				}
			}

			MLog("HP/AP: %08X/%08X\n", info.nHP, info.nAP);

			info.nEquipedItemDesc[MMCIP_MELEE] = 2;
		}
		else if(Version.Server == SERVER_REFINED_GUNZ)
		{
			nRead = zfread(&info, sizeof(info), 1, pFile);
			if(nRead != 1) return false;
		}

		ZCharacter* pChar=NULL;
		if(bHero)
		{
			g_pGame->m_pMyCharacter=new ZMyCharacter;
			g_pGame->CreateMyCharacter(&info);
			pChar=g_pGame->m_pMyCharacter;
			pChar->Load(pFile, Version);
		}else
		{
			pChar=new ZNetCharacter;
			pChar->Load(pFile, Version);
			pChar->Create(&info);
		}
		ZGetCharacterManager()->Add(pChar);

		pChar->SetVisible(true);
	}

	return true;
}

bool ZReplayLoader::LoadCommandStream()
{
	float fGameTime;
	zfread(&fGameTime, sizeof(fGameTime), 1, pFile);
	m_fGameTime = fGameTime;

	int nCommandCount=0;

	int nSize;
	float fTime;
	while (zfread(&fTime, sizeof(fTime), 1, pFile))
	{
		nCommandCount++;

		char CommandBuffer[1024];

		MUID uidSender;
		zfread(&uidSender, sizeof(uidSender), 1, pFile);
		zfread(&nSize, sizeof(nSize), 1, pFile);

		if(nSize<=0 || nSize>sizeof(CommandBuffer)) {
			return false;
		}
		zfread(CommandBuffer, nSize, 1, pFile);


		ZObserverCommandItem *pZCommand=new ZObserverCommandItem;

		if (!CreateCommandFromStream(CommandBuffer, &pZCommand->pCommand))
			continue;

		pZCommand->pCommand->m_Sender=uidSender;

		pZCommand->fTime=fTime;

		if (Version.Server == SERVER_FREESTYLE_GUNZ && !strcmp(m_StageSetting.szMapName, "Dojo"))
		{
			auto Transform = [](float pos[3])
			{
				pos[0] -= 600;
				pos[1] += 2800;
				pos[2] += 400;
			};

			if (pZCommand->pCommand->GetID() == MC_PEER_BASICINFO)
			{
				MCommandParameter* pParam = pZCommand->pCommand->GetParameter(0);
				if (pParam->GetType() != MPT_BLOB)
					continue;

				ZPACKEDBASICINFO* ppbi = (ZPACKEDBASICINFO*)pParam->GetPointer();

				float pos[3] = { ppbi->posx, ppbi->posy, ppbi->posz };

				Transform(pos);

				ppbi->posx = pos[0];
				ppbi->posy = pos[1];
				ppbi->posz = pos[2];
			}
		}

		g_pGame->GetReplayCommandList()->push_back(pZCommand);
	}

	return true;
}


void ZReplayLoader::ConvertStageSettingNode(REPLAY_STAGE_SETTING_NODE* pSource, MSTAGE_SETTING_NODE* pTarget)
{
	pTarget->uidStage = pSource->uidStage;
	strcpy_safe(pTarget->szStageName, pSource->szStageName);
	strcpy_safe(pTarget->szMapName, pSource->szMapName);
	pTarget->nMapIndex = pSource->nMapIndex;
	pTarget->nGameType = pSource->nGameType;
	pTarget->nRoundMax = pSource->nRoundMax;
	pTarget->nLimitTime = pSource->nLimitTime;
	pTarget->nLimitLevel = pSource->nLimitLevel;
	pTarget->nMaxPlayers = pSource->nMaxPlayers;
	pTarget->bTeamKillEnabled = pSource->bTeamKillEnabled;
	pTarget->bTeamWinThePoint = pSource->bTeamWinThePoint;
	pTarget->bForcedEntryEnabled = pSource->bForcedEntryEnabled;
}

bool ZReplayLoader::CreateCommandFromStream(char* pStream, MCommand **pRetCommand)
{
	if (Version.Server == SERVER_OFFICIAL && Version.nVersion <= 2)
	{
		*pRetCommand = CreateCommandFromStreamVersion2(pStream);
		return true;
	}

	MCommand* pCommand = new MCommand;
	if (!pCommand->SetData(pStream, ZGetGameClient()->GetCommandManager()))
	{
		delete pCommand;
		return false;
	}

	*pRetCommand = pCommand;
	return true;
}


MCommand* ZReplayLoader::CreateCommandFromStreamVersion2(char* pStream)
{
	MCommandManager* pCM = ZGetGameClient()->GetCommandManager();

	MCommand* pCommand = new MCommand;
	
	BYTE nParamCount = 0;
	unsigned short int nDataCount = 0;

	// Get Total Size
	unsigned short nTotalSize = 0;
	memcpy(&nTotalSize, pStream, sizeof(nTotalSize));
	nDataCount += sizeof(nTotalSize);

	// Command
	unsigned short int nCommandID = 0;
	memcpy(&nCommandID, pStream+nDataCount, sizeof(nCommandID));
	nDataCount += sizeof(nCommandID);

	MCommandDesc* pDesc = pCM->GetCommandDescByID(nCommandID);
	if (pDesc == NULL)
	{
		mlog("Error(MCommand::SetData): Wrong Command ID(%d)\n", nCommandID);
		_ASSERT(0);

		return pCommand;
	}
	pCommand->SetID(pDesc);

	if (ParseVersion2Command(pStream+nDataCount, pCommand))
	{
		return pCommand;
	}

	// Parameters
	memcpy(&nParamCount, pStream+nDataCount, sizeof(nParamCount));
	nDataCount += sizeof(nParamCount);
	for(int i=0; i<nParamCount; i++)
	{
		BYTE nType;
		memcpy(&nType, pStream+nDataCount, sizeof(BYTE));
		nDataCount += sizeof(BYTE);

		MCommandParameter* pParam = MakeVersion2CommandParameter((MCommandParameterType)nType, pStream, &nDataCount);
		if (pParam == NULL) return false;
		
		pCommand->m_Params.push_back(pParam);
	}

	return pCommand;
}

bool ZReplayLoader::ParseVersion2Command(char* pStream, MCommand* pCmd)
{
	switch (pCmd->GetID())
	{
	case MC_PEER_HPINFO:
	case MC_PEER_HPAPINFO:
	case MC_MATCH_OBJECT_CACHE:
	case MC_MATCH_STAGE_ENTERBATTLE:
	case MC_MATCH_STAGE_LIST:
	case MC_MATCH_CHANNEL_RESPONSE_PLAYER_LIST:
	case MC_MATCH_GAME_RESPONSE_SPAWN:
	case MC_PEER_DASH:
	case MC_MATCH_BRIDGEPEER:
	case MC_MATCH_SPAWN_WORLDITEM:
		{

		}
		break;
	default:
		return false;
	};

	BYTE nParamCount = 0;
	unsigned short int nDataCount = 0;
	vector<MCommandParameter*> TempParams;

	// Count
	memcpy(&nParamCount, pStream+nDataCount, sizeof(nParamCount));
	nDataCount += sizeof(nParamCount);

	for(int i=0; i<nParamCount; i++)
	{
		BYTE nType;
		memcpy(&nType, pStream+nDataCount, sizeof(BYTE));
		nDataCount += sizeof(BYTE);

		MCommandParameter* pParam = MakeVersion2CommandParameter((MCommandParameterType)nType, pStream, &nDataCount);
		if (pParam == NULL) return false;
		
		TempParams.push_back(pParam);
	}


	switch (pCmd->GetID())
	{
	case MC_PEER_HPAPINFO:
		{
			void* pBlob = TempParams[1]->GetPointer();
			struct REPLAY2_HP_AP_INFO 
			{
				MUID muid;
				float fHP;
				float fAP;
			};

			REPLAY2_HP_AP_INFO* pBlobData = (REPLAY2_HP_AP_INFO*)MGetBlobArrayElement(pBlob, 0);
			pCmd->AddParameter(new MCmdParamFloat(pBlobData->fHP));
			pCmd->AddParameter(new MCmdParamFloat(pBlobData->fAP));
		}
		break;
	case MC_PEER_HPINFO:
		{
			void* pBlob = TempParams[1]->GetPointer();
			struct REPLAY2_HP_INFO 
			{
				MUID muid;
				float fHP;
			};

			REPLAY2_HP_INFO* pBlobData = (REPLAY2_HP_INFO*)MGetBlobArrayElement(pBlob, 0);
			pCmd->AddParameter(new MCmdParamFloat(pBlobData->fHP));
		}
		break;
	case MC_MATCH_OBJECT_CACHE:
		{
			unsigned int nType;
			TempParams[0]->GetValue(&nType);
			MCmdParamBlob* pBlobParam = ((MCmdParamBlob*)TempParams[1])->Clone();

			pCmd->AddParameter(new MCmdParamUChar((unsigned char)nType));
			pCmd->AddParameter(pBlobParam);
		}
		break;
	case MC_MATCH_STAGE_ENTERBATTLE:
		{
			MUID uidPlayer, uidStage;
			int nParam;
			
			TempParams[0]->GetValue(&uidPlayer);
			TempParams[1]->GetValue(&uidStage);
			TempParams[2]->GetValue(&nParam);

			struct REPLAY2_ExtendInfo
			{
				char			nTeam;
				unsigned char	nPlayerFlags;
				unsigned char	nReserved1;
				unsigned char	nReserved2;
			};

			struct REPLAY2_PeerListNode
			{
				MUID				uidChar;
				char				szIP[64];
				unsigned int		nPort;
				MTD_CharInfo		CharInfo;
				REPLAY2_ExtendInfo	ExtendInfo;
			};


			void* pBlob = TempParams[3]->GetPointer();
			//int nCount = MGetBlobArrayCount(pBlob);
			REPLAY2_PeerListNode* pNode = (REPLAY2_PeerListNode*)MGetBlobArrayElement(pBlob, 0);


			void* pNewBlob = MMakeBlobArray(sizeof(MTD_PeerListNode), 1);
			MTD_PeerListNode* pNewNode = (MTD_PeerListNode*)MGetBlobArrayElement(pNewBlob, 0);
			pNewNode->uidChar = pNode->uidChar;
			pNewNode->dwIP = inet_addr(pNode->szIP);
			pNewNode->nPort = pNode->nPort;
			memcpy(&pNewNode->CharInfo, &pNode->CharInfo, sizeof(MTD_CharInfo));
			pNewNode->ExtendInfo.nTeam = pNode->ExtendInfo.nTeam;
			pNewNode->ExtendInfo.nPlayerFlags = pNode->ExtendInfo.nPlayerFlags;
			pNewNode->ExtendInfo.nReserved1 = pNode->ExtendInfo.nReserved1;
			pNewNode->ExtendInfo.nReserved2 = pNode->ExtendInfo.nReserved1;
			

			pCmd->AddParameter(new MCmdParamUChar((unsigned char)nParam));
			pCmd->AddParameter(new MCommandParameterBlob(pNewBlob, MGetBlobArraySize(pNewBlob)));

			MEraseBlobArray(pNewBlob);
		}
		break;
	case MC_MATCH_STAGE_LIST:
		{
			_ASSERT(0);
		}
		break;
	case MC_MATCH_CHANNEL_RESPONSE_PLAYER_LIST:
		{
			_ASSERT(0);
		}
		break;
	case MC_MATCH_GAME_RESPONSE_SPAWN:
		{
			MUID uidChar;
			rvector pos, dir;

			TempParams[0]->GetValue(&uidChar);
			TempParams[1]->GetValue(&pos);
			TempParams[2]->GetValue(&dir);

			pCmd->AddParameter(new MCmdParamUID(uidChar));
			pCmd->AddParameter(new MCmdParamShortVector(pos.x, pos.y, pos.z));
			pCmd->AddParameter(new MCmdParamShortVector(DirElementToShort(dir.x), DirElementToShort(dir.y), DirElementToShort(dir.z)));
		}
		break;
	case MC_PEER_DASH:
		{
			rvector pos, dir;
			int nSelType;

			TempParams[0]->GetValue(&pos);
			TempParams[1]->GetValue(&dir);
			TempParams[2]->GetValue(&nSelType);

			ZPACKEDDASHINFO pdi;
			pdi.posx = Roundf(pos.x);
			pdi.posy = Roundf(pos.y);
			pdi.posz = Roundf(pos.z);

			pdi.dirx = DirElementToShort(dir.x);
			pdi.diry = DirElementToShort(dir.y);
			pdi.dirz = DirElementToShort(dir.z);

			pdi.seltype = (BYTE)nSelType;

			pCmd->AddParameter(new MCommandParameterBlob(&pdi,sizeof(ZPACKEDDASHINFO)));
		}
		break;
	case MC_MATCH_SPAWN_WORLDITEM:
		{
			struct REPLAY2_WorldItem
			{
				unsigned short	nUID;
				unsigned short	nItemID;
				unsigned short  nItemSubType;
				float			x;
				float			y;
				float			z;
			};


			void* pBlob = TempParams[0]->GetPointer();
			int nCount = MGetBlobArrayCount(pBlob);

			void* pNewBlob = MMakeBlobArray(sizeof(MTD_WorldItem), nCount);

			for (int i = 0; i < nCount; i++)
			{
				REPLAY2_WorldItem* pNode = (REPLAY2_WorldItem*)MGetBlobArrayElement(pBlob, i);
				MTD_WorldItem* pNewNode = (MTD_WorldItem*)MGetBlobArrayElement(pNewBlob, i);

				pNewNode->nUID = pNode->nUID;
				pNewNode->nItemID = pNode->nItemID;
				pNewNode->nItemSubType = pNode->nItemSubType;
				pNewNode->x = (short)Roundf(pNode->x);
				pNewNode->y = (short)Roundf(pNode->y);
				pNewNode->z = (short)Roundf(pNode->z);
			}
			pCmd->AddParameter(new MCommandParameterBlob(pNewBlob, MGetBlobArraySize(pNewBlob)));
			MEraseBlobArray(pNewBlob);

		}
		break;
	case MC_MATCH_BRIDGEPEER:
		{
			_ASSERT(0);
		}
		break;
	};


	for(int i=0; i<(int)TempParams.size(); i++){
		delete TempParams[i];
	}
	TempParams.clear();


	return true;
}


MCommandParameter* ZReplayLoader::MakeVersion2CommandParameter(MCommandParameterType nType, char* pStream, unsigned short int* pnDataCount)
{
	MCommandParameter* pParam = NULL;

	switch(nType) 
	{
	case MPT_INT:
		pParam = new MCommandParameterInt;
		break;
	case MPT_UINT:
		pParam = new MCommandParameterUInt;
		break;
	case MPT_FLOAT:
		pParam = new MCommandParameterFloat;
		break;
	case MPT_STR:
		{
			pParam = new MCommandParameterString;
			MCommandParameterString* pStringParam = (MCommandParameterString*)pParam;

			char* pStreamData = pStream+ *pnDataCount;

			int nValueSize = 0;
			memcpy(&nValueSize, pStreamData, sizeof(nValueSize));
			pStringParam->m_Value = new char[nValueSize];
			memcpy(pStringParam->m_Value, pStreamData+sizeof(nValueSize), nValueSize);
			int nParamSize = nValueSize+sizeof(nValueSize);

			*pnDataCount += nParamSize;
			return pParam;
		}
		break;
	case MPT_VECTOR:
		pParam = new MCommandParameterVector;
		break;
	case MPT_POS:
		pParam = new MCommandParameterPos;
		break;
	case MPT_DIR:
		pParam = new MCommandParameterDir;
		break;
	case MPT_BOOL:
		pParam = new MCommandParameterBool;
		break;
	case MPT_COLOR:
		pParam = new MCommandParameterColor;
		break;
	case MPT_UID:
		pParam = new MCommandParameterUID;
		break;
	case MPT_BLOB:
		pParam = new MCommandParameterBlob;
		break;
	case MPT_CHAR:
		pParam = new MCommandParameterChar;
		break;
	case MPT_UCHAR:
		pParam = new MCommandParameterUChar;
		break;
	case MPT_SHORT:
		pParam = new MCommandParameterShort;
		break;
	case MPT_USHORT:
		pParam = new MCommandParameterUShort;
		break;
	case MPT_INT64:
		pParam = new MCommandParameterInt64;
		break;
	case MPT_UINT64:
		pParam = new MCommandParameterUInt64;
		break;
	default:
		mlog("Error(MCommand::SetData): Wrong Param Type\n");
		_ASSERT(false);		// Unknow Parameter!!!
		return NULL;
	}

	*pnDataCount += pParam->SetData(pStream+ *pnDataCount);

	return pParam;
}