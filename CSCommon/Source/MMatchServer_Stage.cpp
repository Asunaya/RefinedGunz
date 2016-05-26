#include "stdafx.h"
#include "MMatrix.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchItem.h"
#include "MAgentObject.h"
#include "MMatchNotify.h"
#include "Msg.h"
#include "MMatchObjCache.h"
#include "MMatchStage.h"
#include "MMatchTransDataType.h"
#include "MMatchFormula.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MMatchShop.h"
#include "MMatchTransDataType.h"
#include "MDebug.h"
#include "MMatchAuth.h"
#include "MMatchStatus.h"
#include "MAsyncDBJob.h"
#include "MVoteDiscussImpl.h"
#include "MUtil.h"
#include "MMatchGameType.h"
#include "MMatchRuleBaseQuest.h"
#include "MMatchRuleQuest.h"
#include "MMatchRuleBerserker.h"
#include "MMatchRuleDuel.h"

static bool StageShowInfo(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat);


MMatchStage* MMatchServer::FindStage(const MUID& uidStage)
{
	MMatchStageMap::iterator i = m_StageMap.find(uidStage);
	if(i==m_StageMap.end()) return NULL;

	MMatchStage* pStage = (*i).second;
	return pStage;
}

bool MMatchServer::StageAdd(MMatchChannel* pChannel, const char* pszStageName, bool bPrivate, const char* pszStagePassword, MUID* pAllocUID)
{
	// 클랜전은 pChannel이 NULL이다.

	MUID uidStage = m_StageMap.UseUID();
	
	MMatchStage* pStage= new MMatchStage;
	if (pChannel && !pChannel->AddStage(pStage)) 
	{
		delete pStage;
		return false;
	}

	if (!pStage->Create(uidStage, pszStageName, bPrivate, pszStagePassword))
	{
		if (pChannel)
		{
			pChannel->RemoveStage(pStage);
		}

		delete pStage;
		return false;
	}

	m_StageMap.Insert(uidStage, pStage);

	*pAllocUID = uidStage;

	return true;
}


bool MMatchServer::StageRemove(const MUID& uidStage, MMatchStageMap::iterator* pNextItor)
{
	MMatchStageMap::iterator i = m_StageMap.find(uidStage);
	if(i==m_StageMap.end()) 
	{
		return false;
	}

	MMatchStage* pStage = (*i).second;

	MMatchChannel* pChannel = FindChannel(pStage->GetOwnerChannel());
	if (pChannel)
	{
		pChannel->RemoveStage(pStage);
	}

	pStage->Destroy();
	delete pStage;

	MMatchStageMap::iterator itorTemp = m_StageMap.erase(i);
	if (pNextItor)
		*pNextItor = itorTemp;

	return true;
}


bool MMatchServer::StageJoin(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;

	if (pObj->GetStageUID() != MUID(0,0))
		StageLeave(pObj->GetUID(), pObj->GetStageUID());


	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	int ret = ValidateStageJoin(uidPlayer, uidStage);
	if (ret != MOK)
	{
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, ret);
		return false;
	}
	pObj->OnStageJoin();

	// Cache Add
	MMatchObjectCacheBuilder CacheBuilder;
	CacheBuilder.AddObject(pObj);
	MCommand* pCmdCacheAdd = CacheBuilder.GetResultCmd(MATCHCACHEMODE_ADD, this);
	RouteToStage(pStage->GetUID(), pCmdCacheAdd);

	// Join
	pStage->AddObject(uidPlayer, pObj);
	pObj->SetStageUID(uidStage);
	pObj->SetStageState(MOSS_NONREADY);
	pObj->SetTeam(pStage->GetRecommandedTeam());

	// Cast Join
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_JOIN), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
	pNew->AddParameter(new MCommandParameterUInt(pStage->GetIndex()+1));
	pNew->AddParameter(new MCommandParameterString((char*)pStage->GetName()));
	if (pStage->GetState() == STAGE_STATE_STANDBY)
		RouteToStage(pStage->GetUID(), pNew);
	else
		RouteToListener(pObj, pNew);


	// Cache Update
	CacheBuilder.Reset();
	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pScanObj = (MMatchObject*)GetObject(uidObj);
		if (pScanObj) {
			CacheBuilder.AddObject(pScanObj);
		} else {
			LOG(LOG_ALL, "MMatchServer::StageJoin - Invalid ObjectMUID(%u:%u) exist in Stage(%s)",
				uidObj.High, uidObj.Low, pStage->GetName());
			pStage->RemoveObject(uidObj);
			return false;
		}
	}
    MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_UPDATE, this);
	RouteToListener(pObj, pCmdCacheUpdate);


	// Cast Master(방장)
	MUID uidMaster = pStage->GetMasterUID();
	MCommand* pMasterCmd = CreateCommand(MC_MATCH_STAGE_MASTER, MUID(0,0));
	pMasterCmd->AddParameter(new MCommandParameterUID(uidStage));
	pMasterCmd->AddParameter(new MCommandParameterUID(uidMaster));
	RouteToListener(pObj, pMasterCmd);


#ifdef _QUEST_ITEM
	if (MGetServerConfig()->GetServerMode() == MSM_TEST)
	{
		MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if( 0 == pNode )
		{
			mlog( "MMatchServer::StageJoin - 스테이지 셋팅 노드 찾기 실패.\n" );
			return false;
		}

		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
		{
			MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
			if( 0 == pRuleQuest )
			{
				mlog( "MMatchServer::StageJoin - 포인터 형변환 실패.\n" );
				return false;
			}

			pRuleQuest->OnChangeCondition();
			//pRuleQuest->OnResponseQL_ToStage( pObj->GetStageUID() );
			// 동환씨께서 처음 스테이지 조인시는 이전에 설정이 퀘스트로 되있어도 
			//  처음 조인한 유저는 퀘스트 타입인지 알수가 없기에,
			//	클라이언트가 스테이지 타입이 퀘스트인지를 인식하는 시점에서
			//  이 정보를 요청을 하는 방향으로 수정함. - 05/04/14 by 추교성.
			// pStage->GetRule()->OnResponseSacrificeSlotInfoToStage( uidPlayer );
		}
	}
#endif


	// Cast Character Setting
	StageTeam(uidPlayer, uidStage, pObj->GetTeam());
	StagePlayerState(uidPlayer, uidStage, pObj->GetStageState());


	// 방송 관계자면 방장권한을 자동으로 빼앗는다. - 온게임넷 비비빅 요청
	if (MMUG_EVENTMASTER == pObj->GetAccountInfo()->m_nUGrade) {
		OnEventChangeMaster(pObj->GetUID());
	}

	return true;
}

bool MMatchServer::StageLeave(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	bool bLeaverMaster = false;
	if (uidPlayer == pStage->GetMasterUID()) bLeaverMaster = true;

#ifdef _QUEST_ITEM
	if (MGetServerConfig()->GetServerMode() == MSM_TEST)
	{
		MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if( 0 != pNode )
		{
			if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
			{
				MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
				if(pRuleQuest)
				{
					pRuleQuest->PreProcessLeaveStage( uidPlayer );
				} else {
					LOG(LOG_ALL, "StageLeave:: MMatchRule to MMatchRuleBaseQuest FAILED \n");
				}
			}
		}
	}
#endif

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_LEAVE), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
	RouteToStage(pStage->GetUID(), pNew);

	pStage->RemoveObject(uidPlayer);

	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj)
	{
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.AddObject(pObj);
		MCommand* pCmdCache = CacheBuilder.GetResultCmd(MATCHCACHEMODE_REMOVE, this);
		RouteToStage(uidStage, pCmdCache);
	}

	// cast Master
	if (bLeaverMaster) StageMaster(uidStage);

#ifdef _QUEST_ITEM
	// 유저가 스테이지에서 나간후에 QL을 다시 계산해 줘야 함.
	if (MGetServerConfig()->GetServerMode() == MSM_TEST)
	{
		MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if( 0 == pNode )
		{
			mlog( "MMatchServer::StageLeave - 스테이지 셋팅 노드 찾기 실패.\n" );
			return false;
		}

		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
		{
			MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
			if( 0 == pRuleQuest )
			{
				mlog( "MMatchServer::StageLeave - 포인터 형변환 실패.\n" );
				return false;
			}

			if( STAGE_STATE_STANDBY == pStage->GetState() )
				pRuleQuest->OnChangeCondition();
				//pRuleQuest->OnResponseQL_ToStage( uidStage );
		}
	}
#endif


	return true;
}

bool MMatchServer::StageEnterBattle(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	pObj->SetPlace(MMP_BATTLE);

	MCommand* pNew = CreateCommand(MC_MATCH_STAGE_ENTERBATTLE, MUID(0,0));
	//pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	//pNew->AddParameter(new MCommandParameterUID(uidStage));

	unsigned char nParam = MCEP_NORMAL;
	if (pObj->IsForcedEntried()) nParam = MCEP_FORCED;
	pNew->AddParameter(new MCommandParameterUChar(nParam));


	void* pPeerArray = MMakeBlobArray(sizeof(MTD_PeerListNode), 1);
	MTD_PeerListNode* pNode = (MTD_PeerListNode*)MGetBlobArrayElement(pPeerArray, 0);
	pNode->uidChar = pObj->GetUID();

	pNode->dwIP = pObj->GetIP();
	pNode->nPort = pObj->GetPort();

	CopyCharInfoForTrans(&pNode->CharInfo, pObj->GetCharInfo(), pObj);

	ZeroMemory(&pNode->ExtendInfo, sizeof(MTD_ExtendInfo));
	//if (pStage->GetStageSetting()->IsTeamPlay())
		pNode->ExtendInfo.nTeam = (char)pObj->GetTeam();
	//else
		//pNode->ExtendInfo.nTeam = 0;
	pNode->ExtendInfo.nPlayerFlags = pObj->GetPlayerFlags();

	pNew->AddParameter(new MCommandParameterBlob(pPeerArray, MGetBlobArraySize(pPeerArray)));
	MEraseBlobArray(pPeerArray);

	RouteToStage(uidStage, pNew);


	// 라우팅 후에 넣어야 한다.
	pStage->EnterBattle(pObj);


	return true;
}

bool MMatchServer::StageLeaveBattle(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	if (pObj->GetPlace() != MMP_BATTLE) 
	{
//		_ASSERT(0);
		return false;
	}

	pStage->LeaveBattle(pObj);
	pObj->SetPlace(MMP_STAGE);
	
	// 디비 캐슁 업데이트
	UpdateCharDBCachingData(pObj);

	// 레벨에 안맞는 장비아이템 체크
#define LEGAL_ITEMLEVEL_DIFF		3
	bool bIsCorrect = true;
	for (int i = 0; i < MMCIP_END; i++)
	{
		if (CorrectEquipmentByLevel(pObj, MMatchCharItemParts(i), LEGAL_ITEMLEVEL_DIFF))
		{
			bIsCorrect = false;
		}
	}
	if (!bIsCorrect)
	{
		MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_RESULT, MUID(0,0));
		pNewCmd->AddParameter(new MCommandParameterInt(MERR_TAKEOFF_ITEM_BY_LEVELDOWN));
		RouteToListener(pObj, pNewCmd);
	}

	// 기간 만료 아이템이 있는지 체크
	CheckExpiredItems(pObj);


	MCommand* pNew = CreateCommand(MC_MATCH_STAGE_LEAVEBATTLE, MUID(0,0));
	pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	pNew->AddParameter(new MCommandParameterUID(uidStage));
	RouteToStage(uidStage, pNew);

	if (pObj->GetRelayPeer()) {
		MAgentObject* pAgent = GetAgent(pObj->GetAgentUID());
		if (pAgent) {
			MCommand* pCmd = CreateCommand(MC_AGENT_PEER_UNBIND, pAgent->GetCommListener());
			pCmd->AddParameter(new MCmdParamUID(uidPlayer));
			Post(pCmd);
		}
	}

	return true;
}

bool MMatchServer::StageChat(const MUID& uidPlayer, const MUID& uidStage, char* pszChat)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL)	return false;
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) return false;

	if (pObj->GetAccountInfo()->m_nUGrade == MMUG_CHAT_LIMITED) return false;

//	InsertChatDBLog(uidPlayer, pszChat);

	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_CHAT), MUID(0,0), m_This);
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterString(pszChat));
	RouteToStage(uidStage, pCmd);
	return true;
}

bool MMatchServer::StageTeam(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	pStage->PlayerTeam(uidPlayer, nTeam);

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_TEAM, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterUInt(nTeam));

	RouteToStageWaitRoom(uidStage, pCmd);
	return true;
}

bool MMatchServer::StagePlayerState(const MUID& uidPlayer, const MUID& uidStage, MMatchObjectStageState nStageState)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	pStage->PlayerState(uidPlayer, nStageState);
	
	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_PLAYER_STATE, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterInt(nStageState));
	RouteToStage(uidStage, pCmd);
	return true;
}

bool MMatchServer::StageMaster(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	MUID uidMaster = pStage->GetMasterUID();

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_MASTER, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterUID(uidMaster));
	RouteToStage(uidStage, pCmd);

	return true;
}

void MMatchServer::StageLaunch(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	ReserveAgent(pStage);

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_LAUNCH, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidStage));
	pCmd->AddParameter(new MCmdParamStr( const_cast<char*>(pStage->GetMapName()) ));
	RouteToStage(uidStage, pCmd);
	return;
}

void MMatchServer::StageFinishGame(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_FINISH_GAME, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	RouteToStage(uidStage, pCmd);
	return;
}


MCommand* MMatchServer::CreateCmdResponseStageSetting(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return NULL;

	MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_STAGESETTING, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(pStage->GetUID()));

	MMatchStageSetting* pSetting = pStage->GetStageSetting();

	// Param 1 : Stage Settings
	void* pStageSettingArray = MMakeBlobArray(sizeof(MSTAGE_SETTING_NODE), 1);
	MSTAGE_SETTING_NODE* pNode = (MSTAGE_SETTING_NODE*)MGetBlobArrayElement(pStageSettingArray, 0);
	CopyMemory(pNode, pSetting->GetStageSetting(), sizeof(MSTAGE_SETTING_NODE));
	pCmd->AddParameter(new MCommandParameterBlob(pStageSettingArray, MGetBlobArraySize(pStageSettingArray)));
	MEraseBlobArray(pStageSettingArray);

	// Param 2 : Char Settings
	int nCharCount = (int)pStage->GetObjCount();
	void* pCharArray = MMakeBlobArray(sizeof(MSTAGE_CHAR_SETTING_NODE), nCharCount);
	int nIndex=0;
	for (MUIDRefCache::iterator itor=pStage->GetObjBegin(); itor!=pStage->GetObjEnd(); itor++) {
		MSTAGE_CHAR_SETTING_NODE* pCharNode = (MSTAGE_CHAR_SETTING_NODE*)MGetBlobArrayElement(pCharArray, nIndex++);
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		pCharNode->uidChar = pObj->GetUID();
		pCharNode->nTeam = pObj->GetTeam();
		pCharNode->nState = pObj->GetStageState();
	}
	pCmd->AddParameter(new MCommandParameterBlob(pCharArray, MGetBlobArraySize(pCharArray)));
	MEraseBlobArray(pCharArray);

	// Param 3 : Stage State
	pCmd->AddParameter(new MCommandParameterInt((int)pStage->GetState()));

	// Param 4 : Stage Master
	pCmd->AddParameter(new MCommandParameterUID(pStage->GetMasterUID()));

	return pCmd;
}



void MMatchServer::OnStageCreate(const MUID& uidChar, char* pszStageName, bool bPrivate, char* pszStagePassword)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;
	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
	if (pChannel == NULL) return;

	MUID uidStage;
	
	if (!StageAdd(pChannel, pszStageName, bPrivate, pszStagePassword, &uidStage))
	{
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_CREATE, MERR_CANNOT_CREATE_STAGE);
		return;
	}
	StageJoin(uidChar, uidStage);

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage)
		pStage->SetFirstMasterName(pObj->GetCharInfo()->m_szName);
}


void MMatchServer::OnStageJoin(const MUID& uidChar, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MMatchStage* pStage = NULL;

	if (uidStage == MUID(0,0)) {
		return;
	} else {
		pStage = FindStage(uidStage);
	}

	if (pStage == NULL) {
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_STAGE_NOT_EXIST);
		return;
	}

	if ((IsAdminGrade(pObj) == false) && pStage->IsPrivate())
	{
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_CANNOT_JOIN_STAGE_BY_PASSWORD);
		return;
	}

	StageJoin(uidChar, pStage->GetUID());
}

void MMatchServer::OnPrivateStageJoin(const MUID& uidPlayer, const MUID& uidStage, char* pszPassword)
{
	if (strlen(pszPassword) > STAGEPASSWD_LENGTH) return;

	MMatchStage* pStage = NULL;

	if (uidStage == MUID(0,0)) 
	{
		return;
	} 
	else 
	{
		pStage = FindStage(uidStage);
	}

	if (pStage == NULL) 
	{
		MMatchObject* pObj = GetObject(uidPlayer);
		if (pObj != NULL)
		{
			RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_STAGE_NOT_EXIST);
		}

		return;
	}

	// 영자나 개발자면 무시..

	bool bSkipPassword = false;

	MMatchObject* pObj = GetObject(uidPlayer);

	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) 
		return;

	MMatchUserGradeID ugid = pObj->GetAccountInfo()->m_nUGrade;

	if (ugid == MMUG_DEVELOPER || ugid == MMUG_ADMIN) 
		bSkipPassword = true;

	// 비밀방이 아니거나 패스워드가 틀리면 패스워드가 틀렸다고 응답한다.
	if(bSkipPassword==false) {
		if ((!pStage->IsPrivate()) || (strcmp(pStage->GetPassword(), pszPassword)))
		{
			MMatchObject* pObj = GetObject(uidPlayer);
			if (pObj != NULL)
			{
				RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_CANNOT_JOIN_STAGE_BY_PASSWORD);
			}

			return;
		}
	}

	StageJoin(uidPlayer, pStage->GetUID());
}

void MMatchServer::OnStageFollow(const MUID& uidPlayer, const char* pszTargetName)
{
	MMatchObject* pPlayerObj = GetObject(uidPlayer);
	if (pPlayerObj == NULL) return;

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) return;

	// 자기 자신을 따라 가려고 했을경우 검사.
	if (pPlayerObj->GetUID() == pTargetObj->GetUID()) return;

	// 스테이트가 잘못되어 있는지 검사.
	if (!pPlayerObj->CheckEnableAction(MMatchObject::MMOA_STAGE_FOLLOW)) return;


	// 서로 다른 채널인지 검사.
	if (pPlayerObj->GetChannelUID() != pTargetObj->GetChannelUID()) {

#ifdef _VOTESETTING
		RouteResponseToListener( pPlayerObj, MC_MATCH_RESPONSE_STAGE_FOLLOW, MERR_CANNOT_FOLLOW );
#endif
		return;
	}

	if ((IsAdminGrade(pTargetObj) == true)) {
		NotifyMessage(pPlayerObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	MMatchStage* pStage = FindStage(pTargetObj->GetStageUID());
	if (pStage == NULL) return;

	// 클랜전게임은 따라갈 수 없음
	if (pStage->GetStageType() != MST_NORMAL) return;

	if (pStage->IsPrivate()==false) {
		if ((pStage->GetStageSetting()->GetForcedEntry()==false) && pStage->GetState() != STAGE_STATE_STANDBY) {
			// Deny Join

#ifdef _VOTESETTING
			RouteResponseToListener( pPlayerObj, MC_MATCH_RESPONSE_STAGE_FOLLOW, MERR_CANNOT_FOLLOW );
#endif
		} else {
			StageJoin(uidPlayer, pTargetObj->GetStageUID());
		}
	}
	else {
		// 따라가려는 방이 비밀번호를 필요로 할경우는 따라갈수 없음.
		//RouteResponseToListener( pPlayerObj, MC_MATCH_RESPONSE_STAGE_FOLLOW, MERR_CANNOT_FOLLOW_BY_PASSWORD );

		// 해당방이 비밀방이면 비밀번호를 요구한다.
		MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_REQUIRE_PASSWORD), MUID(0,0), m_This);
		pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
		pNew->AddParameter(new MCommandParameterString((char*)pStage->GetName()));
		RouteToListener(pPlayerObj, pNew);
	}
}

void MMatchServer::OnStageLeave(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	StageLeave(uidPlayer, uidStage);
}

void MMatchServer::OnStageRequestPlayerList(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	// 방인원 목록
	MMatchObjectCacheBuilder CacheBuilder;
	CacheBuilder.Reset();
	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MMatchObject* pScanObj = (MMatchObject*)(*i).second;
		CacheBuilder.AddObject(pScanObj);
	}
    MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_UPDATE, this);
	RouteToListener(pObj, pCmdCacheUpdate);

	// Cast Master(방장)
	MUID uidMaster = pStage->GetMasterUID();
	MCommand* pMasterCmd = CreateCommand(MC_MATCH_STAGE_MASTER, MUID(0,0));
	pMasterCmd->AddParameter(new MCommandParameterUID(uidStage));
	pMasterCmd->AddParameter(new MCommandParameterUID(uidMaster));
	RouteToListener(pObj, pMasterCmd);

	// Cast Character Setting
	StageTeam(uidPlayer, uidStage, pObj->GetTeam());
	StagePlayerState(uidPlayer, uidStage, pObj->GetStageState());
}

void MMatchServer::OnStageEnterBattle(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	StageEnterBattle(uidPlayer, uidStage);
}

void MMatchServer::OnStageLeaveBattle(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	StageLeaveBattle(uidPlayer, uidStage);
}


#include "CMLexicalAnalyzer.h"
// 강퇴 임시코드
bool StageKick(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat)
{
	MMatchObject* pChar = pServer->GetObject(uidPlayer);
	if (pChar == NULL)	return false;
	MMatchStage* pStage = pServer->FindStage(uidStage);
	if (pStage == NULL) return false;
	if (uidPlayer != pStage->GetMasterUID()) return false;

	bool bResult = false;
	CMLexicalAnalyzer lex;
	lex.Create(pszChat);

	if (lex.GetCount() >= 1) {
		char* pszCmd = lex.GetByStr(0);
		if (pszCmd) {
			if (_stricmp(pszCmd, "/kick") == 0) {
				if (lex.GetCount() >= 2) {
					char* pszTarget = lex.GetByStr(1);
					if (pszTarget) {
						for (MUIDRefCache::iterator itor = pStage->GetObjBegin(); 
							itor != pStage->GetObjEnd(); ++itor)
						{
							MMatchObject* pTarget = (MMatchObject*)((*itor).second);
							if (_stricmp(pszTarget, pTarget->GetName()) == 0) {
								if (pTarget->GetPlace() != MMP_BATTLE) {
									pServer->StageLeave(pTarget->GetUID(), uidStage);
									bResult = true;
								}
								break;
							}
						}
					}
				}
			}	// Kick
		}
	}

	lex.Destroy();
	return bResult;
}

// 방장확인 임시코드
bool StageShowInfo(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat)
{
	MMatchObject* pChar = pServer->GetObject(uidPlayer);
	if (pChar == NULL)	return false;
	MMatchStage* pStage = pServer->FindStage(uidStage);
	if (pStage == NULL) return false;
	if (uidPlayer != pStage->GetMasterUID()) return false;

	bool bResult = false;
	CMLexicalAnalyzer lex;
	lex.Create(pszChat);

	if (lex.GetCount() >= 1) {
		char* pszCmd = lex.GetByStr(0);
		if (pszCmd) {
			if (_stricmp(pszCmd, "/showinfo") == 0) {
				char szMsg[256]="";
				sprintf_safe(szMsg, "FirstMaster : (%s)", pStage->GetFirstMasterName());
				pServer->Announce(pChar, szMsg);
				bResult = true;
			}	// ShowInfo
		}
	}

	lex.Destroy();
	return bResult;
}
void MMatchServer::OnStageChat(const MUID& uidPlayer, const MUID& uidStage, char* pszChat)
{
	// RAONHAJE : 강퇴 임시코드
	if (pszChat[0] == '/') {
		if (StageKick(this, uidPlayer, uidStage, pszChat))
			return;
		if (StageShowInfo(this, uidPlayer, uidStage, pszChat))
			return;
	}

	StageChat(uidPlayer, uidStage, pszChat);
}

void MMatchServer::OnStageStart(const MUID& uidPlayer, const MUID& uidStage, int nCountdown)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetMasterUID() != uidPlayer) return;

	if (pStage->StartGame() == true) {
		MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_START), MUID(0,0), m_This);
		pNew->AddParameter(new MCommandParameterUID(uidPlayer));
		pNew->AddParameter(new MCommandParameterUID(uidStage));
		pNew->AddParameter(new MCommandParameterInt(min(nCountdown,3)));
		RouteToStage(uidStage, pNew);

		// 디비에 로그를 남긴다.
		int nMapID = pStage->GetStageSetting()->GetMapIndex();
		int nGameType = (int)pStage->GetStageSetting()->GetGameType();
		MMatchObject* pMaster = GetObject(pStage->GetMasterUID());

		// test 맵등은 로그 남기지 않는다.
		if ( (MIsCorrectMap(nMapID)) && (pMaster) && (MGetGameTypeMgr()->IsCorrectGameType(nGameType)) )
		{
			char szPlayers[1024] = "";
			for (MUIDRefCache::iterator itor = pStage->GetObjBegin(); 
				itor != pStage->GetObjEnd(); ++itor)
			{
				MMatchObject* pObject = (MMatchObject*)((*itor).second);
				strcat_safe(szPlayers, pObject->GetCharInfo()->m_szName);
				strcat_safe(szPlayers, " ");
			}

			if (pStage->GetStageType() != MST_LADDER)
			{
				MAsyncDBJob_InsertGameLog* pJob=new MAsyncDBJob_InsertGameLog();
				pJob->Input(pStage->GetName(), 
							g_MapDesc[nMapID].szMapName, 
							MGetGameTypeMgr()->GetInfo(MMATCH_GAMETYPE(nGameType))->szGameTypeStr,
							pStage->GetStageSetting()->GetRoundMax(),
							pMaster->GetCharInfo()->m_nCID,
							(int)pStage->GetObjCount(),
							szPlayers);
				PostAsyncJob(pJob);

/*
				m_MatchDBMgr.InsertGameLog(pStage->GetName(), g_MapDesc[nMapID].szMapName, 
										MGetGameTypeMgr()->GetInfo(MMATCH_GAMETYPE(nGameType))->szGameTypeStr,
										pStage->GetStageSetting()->GetRoundMax(),
										pMaster->GetCharInfo()->m_nCID,
										(int)pStage->GetObjCount(),
										szPlayers);
*/
			}
		}

	}
}

void MMatchServer::OnStartStageList(const MUID& uidComm)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->SetStageListTransfer(true);
}

void MMatchServer::OnStopStageList(const MUID& uidComm)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->SetStageListTransfer(false);
}

void MMatchServer::OnStagePlayerState(const MUID& uidPlayer, const MUID& uidStage, MMatchObjectStageState nStageState)
{
	StagePlayerState(uidPlayer, uidStage, nStageState);
}


void MMatchServer::OnStageTeam(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	MMatchObject* pChar = GetObject(uidPlayer);
	if (pChar == NULL) return;

	StageTeam(uidPlayer, uidStage, nTeam);
}

void MMatchServer::OnStageMap(const MUID& uidStage, char* pszMapName)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetState() != STAGE_STATE_STANDBY) return;	// 대기상태에서만 바꿀수 있다
	if (strlen(pszMapName) < 2) return;


	pStage->SetMapName(pszMapName);


	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_MAP), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidStage));
	pNew->AddParameter(new MCommandParameterString(pszMapName));
	RouteToStage(uidStage, pNew);
}

void MMatchServer::OnStageSetting(const MUID& uidPlayer, const MUID& uidStage, void* pStageBlob, int nStageCount)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetState() != STAGE_STATE_STANDBY) return;	// 대기상태에서만 바꿀수 있다
	if (nStageCount <= 0) return;

	// 방장이거나 운영자가 아닌데 세팅을 바꾸면 그냥 리턴
	if (pStage->GetMasterUID() != uidPlayer)
	{
		MMatchObject* pObjMaster = GetObject(uidPlayer);
		if (!IsAdminGrade(pObjMaster)) return;
	}


	MSTAGE_SETTING_NODE* pNode = (MSTAGE_SETTING_NODE*)MGetBlobArrayElement(pStageBlob, 0);

	MMatchStageSetting* pSetting = pStage->GetStageSetting();
	MMatchChannel* pChannel = FindChannel(pStage->GetOwnerChannel());

	bool bCheckChannelRule = true;

#ifdef _QUEST
	if (QuestTestServer())
	{
		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
		{
			bCheckChannelRule = false;
		}
	}
#endif

	if ((pChannel) && (bCheckChannelRule))
	{
		// 세팅할 수 있는 맵, 게임타입인지 체크
		MChannelRule* pRule = MGetChannelRuleMgr()->GetRule(pChannel->GetRuleType());
		if (pRule)
		{
			if (!pRule->CheckGameType(pNode->nGameType))
			{
				pNode->nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
			}

			bool bDuelMode = false;
			if ( pNode->nGameType == MMATCH_GAMETYPE_DUEL)
				bDuelMode = true;

			if (!pRule->CheckMap(pNode->nMapIndex, bDuelMode))
			{
				pNode->nMapIndex = 0;
			}
		}
	}
	
	
		


	pNode->nMapIndex = pSetting->GetMapIndex();

#ifdef _QUEST
	MMATCH_GAMETYPE nLastGameType = pSetting->GetGameType();

	// 퀘스트 모드이면 무조건 난입불가, 최대인원 4명으로 세팅한다.
	if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
	{
		if (pNode->bForcedEntryEnabled == true) pNode->bForcedEntryEnabled = false;
		pNode->nMaxPlayers = STAGE_QUEST_MAX_PLAYER;
		pNode->nLimitTime = STAGESETTING_LIMITTIME_UNLIMITED;


		// 퀘스트 서버가 아닌데 퀘스트 게임이면 솔로데스매치로 바꾼다.
		if (!QuestTestServer())
		{
			pNode->nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
		}
	}

	// 퀘스트 모드였다가 다른 모드가 되면 '난입불가'를 허용으로 변경
	if ( (nLastGameType == MMATCH_GAMETYPE_QUEST) && (pNode->nGameType != MMATCH_GAMETYPE_QUEST))
		pNode->bForcedEntryEnabled = true;
#endif

	if (!MGetGameTypeMgr()->IsTeamGame(pNode->nGameType))
	{
		pNode->bAutoTeamBalancing = true;
	}

	pSetting->UpdateStageSetting(pNode);
	pStage->ChangeRule(pNode->nGameType);


	MCommand* pCmd = CreateCmdResponseStageSetting(uidStage);
	RouteToStage(uidStage, pCmd);


	// 게임 모드가 변경되었을경우
	if (nLastGameType != pSetting->GetGameType())
	{
		char szNewMap[256];

		if ( (nLastGameType != MMATCH_GAMETYPE_QUEST) && ( pSetting->GetGameType() == MMATCH_GAMETYPE_QUEST))
		{
			OnStageMap(uidStage, GetQuest()->GetSurvivalMapInfo(MSURVIVAL_MAP(0))->szName);

			MMatchRuleQuest* pQuest = reinterpret_cast< MMatchRuleQuest* >( pStage->GetRule() );
			pQuest->RefreshStageGameInfo();
		}
		else if ( (nLastGameType != MMATCH_GAMETYPE_DUEL) && ( pSetting->GetGameType() == MMATCH_GAMETYPE_DUEL))
		{
			strcpy_safe( szNewMap, MGetMapName( MMATCH_MAP_HALL));
			OnStageMap(uidStage, szNewMap);
		}
		else if ( ((nLastGameType == MMATCH_GAMETYPE_QUEST) || (nLastGameType == MMATCH_GAMETYPE_DUEL)) &&
			      ((pSetting->GetGameType() != MMATCH_GAMETYPE_QUEST) && ( pSetting->GetGameType() != MMATCH_GAMETYPE_DUEL)))
		{
			strcpy_safe( szNewMap, MGetMapName( MMATCH_MAP_MANSION));
			OnStageMap(uidStage, szNewMap);
		}
	}
}

void MMatchServer::OnRequestStageSetting(const MUID& uidComm, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	MCommand* pCmd = CreateCmdResponseStageSetting(uidStage);
	pCmd->m_Receiver = uidComm;
	Post(pCmd);

	MMatchObject* pChar = GetObject(uidComm);
	if (pChar && (MMUG_EVENTMASTER == pChar->GetAccountInfo()->m_nUGrade)) 	{
		// 이벤트 마스터에게 처음 방만들었던 사람을 알려준다
		StageShowInfo(this, uidComm, uidStage, "/showinfo");
	}
}

void MMatchServer::OnRequestPeerList(const MUID& uidChar, const MUID& uidStage)
{
	ResponsePeerList(uidChar, uidStage);
}

void MMatchServer::OnRequestGameInfo(const MUID& uidChar, const MUID& uidStage)
{
	ResponseGameInfo(uidChar, uidStage);
}

void MMatchServer::ResponseGameInfo(const MUID& uidChar, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage); if (pStage == NULL) return;
	MMatchObject* pObj = GetObject(uidChar); if (pObj == NULL) return;
	if (pStage->GetRule() == NULL) return;

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_GAME_INFO, MUID(0,0));
	pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));

	// 게임정보
	void* pGameInfoArray = MMakeBlobArray(sizeof(MTD_GameInfo), 1);
	MTD_GameInfo* pGameItem = (MTD_GameInfo*)MGetBlobArrayElement(pGameInfoArray, 0);
	memset(pGameItem, 0, sizeof(MTD_GameInfo));
	
	if (pStage->GetStageSetting()->IsTeamPlay())
	{
		pGameItem->nRedTeamScore = static_cast<char>(pStage->GetTeamScore(MMT_RED));
		pGameItem->nBlueTeamScore = static_cast<char>(pStage->GetTeamScore(MMT_BLUE));

		pGameItem->nRedTeamKills = static_cast<short>(pStage->GetTeamKills(MMT_RED));
		pGameItem->nBlueTeamKills = static_cast<short>(pStage->GetTeamKills(MMT_BLUE));
	}

	pNew->AddParameter(new MCommandParameterBlob(pGameInfoArray, MGetBlobArraySize(pGameInfoArray)));
	MEraseBlobArray(pGameInfoArray);

	// 룰정보
	void* pRuleInfoArray = NULL;
	if (pStage->GetRule())
		pRuleInfoArray = pStage->GetRule()->CreateRuleInfoBlob();
	if (pRuleInfoArray == NULL)
		pRuleInfoArray = MMakeBlobArray(0, 0);
	pNew->AddParameter(new MCommandParameterBlob(pRuleInfoArray, MGetBlobArraySize(pRuleInfoArray)));
	MEraseBlobArray(pRuleInfoArray);

	// Battle에 들어간 사람만 List를 만든다.
	int nPlayerCount = pStage->GetObjInBattleCount();

	void* pPlayerItemArray = MMakeBlobArray(sizeof(MTD_GameInfoPlayerItem), nPlayerCount);
	int nIndex=0;
	for (MUIDRefCache::iterator itor=pStage->GetObjBegin(); itor!=pStage->GetObjEnd(); itor++) 
	{
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		if (pObj->GetEnterBattle() == false) continue;

		MTD_GameInfoPlayerItem* pPlayerItem = (MTD_GameInfoPlayerItem*)MGetBlobArrayElement(pPlayerItemArray, nIndex++);
		pPlayerItem->uidPlayer = pObj->GetUID();
		pPlayerItem->bAlive = pObj->CheckAlive();
		pPlayerItem->nKillCount = pObj->GetAllRoundKillCount();
		pPlayerItem->nDeathCount = pObj->GetAllRoundDeathCount();
	}
	pNew->AddParameter(new MCommandParameterBlob(pPlayerItemArray, MGetBlobArraySize(pPlayerItemArray)));
	MEraseBlobArray(pPlayerItemArray);

	RouteToListener(pObj, pNew);
}

void MMatchServer::OnMatchLoadingComplete(const MUID& uidPlayer, int nPercent)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MCommand* pCmd = CreateCommand(MC_MATCH_LOADING_COMPLETE, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidPlayer));
	pCmd->AddParameter(new MCmdParamInt(nPercent));
	RouteToStage(pObj->GetStageUID(), pCmd);	
}


void MMatchServer::OnGameRoundState(const MUID& uidStage, int nState, int nRound)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	pStage->RoundStateFromClient(uidStage, nState, nRound);
}

void MMatchServer::OnGameKill(const MUID& uidAttacker, const MUID& uidVictim)
{
	MMatchObject* pVictim = GetObject(uidVictim);
	MMatchObject* pAttacker = GetObject(uidAttacker);
	if (pVictim == NULL) return;

	MMatchStage* pStage = FindStage(pVictim->GetStageUID());
	if (pStage == NULL) return;

	// 서버는 죽은줄 알고있었는데 또 죽었다고 신고들어온경우 죽었다는 메시지만 라우팅한다
	if (pVictim->CheckAlive() == false) {	
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SUICIDE, MUID(0,0));
		int nResult = MOK;
		pNew->AddParameter(new MCommandParameterInt(nResult));
		pNew->AddParameter(new MCommandParameterUID(pVictim->GetUID()));
		RouteToBattle(pStage->GetUID(), pNew);
		return;
	}

	MUID TheAttackerUID = uidAttacker;

	if (pAttacker)
	{
		if (pAttacker->GetStageUID() != pVictim->GetStageUID())
		{
			pAttacker = NULL;
			TheAttackerUID = MUID(0,0);
		}
	}

	pVictim->OnDead();

	if (TheAttackerUID != uidVictim) 
	{
		if (pAttacker != NULL) 
		{
			pAttacker->OnKill();
		}
	}

	if ((pAttacker != NULL) && (pVictim != NULL))
	{
		ProcessOnGameKill(pStage, pAttacker, pVictim);
	}

	pStage->OnGameKill(TheAttackerUID, uidVictim);	
}


void MMatchServer::OnDuelSetObserver(const MUID& uidChar)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MCommand* pCmd = CreateCommand(MC_MATCH_SET_OBSERVER, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidChar));
	RouteToBattle(pObj->GetStageUID(), pCmd);
}

void MMatchServer::OnRequestSpawn(const MUID& uidChar, const MVector& pos, const MVector& dir)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	if ((IsAdminGrade(pObj) && pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
		|| pObj->GetTeam() == MMT_SPECTATOR) {
		// Do not spawn when admin-hiding or spectating
		return;
	}

	// 마지막 죽었던 시간과 새로 리스폰을 요청한 시간 사이에 5초 이상의 시간이 있었는지 검사한다.
	DWORD dwTime = timeGetTime() - pObj->GetLastSpawnTime();;
	if ( dwTime < 5000)
		return;
	pObj->SetLastSpawnTime( timeGetTime());


	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;
	if ( (pStage->GetRule()->GetRoundState() != MMATCH_ROUNDSTATE_PREPARE) &&
		 (!pObj->IsEnabledRespawnDeathTime(GetTickTime()))  )
		 return;

	MMatchRule* pRule = pStage->GetRule();					// 이런 식의 코드는 마음에 안들지만 -_-; 게임타입 보고 예외처리.
	MMATCH_GAMETYPE gameType = pRule->GetGameType();		// 다른 방법 있나요.
	if (gameType == MMATCH_GAMETYPE_DUEL)
	{
		MMatchRuleDuel* pDuel = (MMatchRuleDuel*)pRule;		// RTTI 안써서 dynamic cast는 패스.. 예외처리도 짜증나고 -,.-
		if (uidChar != pDuel->uidChampion && uidChar != pDuel->uidChallenger)
		{
			OnDuelSetObserver(uidChar);
			return;
		}
	}


	pObj->SetAlive(true);

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_RESPONSE_SPAWN, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidChar));
	pCmd->AddParameter(new MCmdParamShortVector(pos.x, pos.y, pos.z));
	pCmd->AddParameter(new MCmdParamShortVector(DirElementToShort(dir.x), DirElementToShort(dir.y), DirElementToShort(dir.z)));
	RouteToBattle(pObj->GetStageUID(), pCmd);
}

void MMatchServer::OnGameRequestTimeSync(const MUID& uidComm, unsigned long nLocalTimeStamp)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	MMatchTimeSyncInfo* pSync = pObj->GetSyncInfo();
	pSync->Update(GetGlobalClockCount());

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_RESPONSE_TIMESYNC, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUInt(nLocalTimeStamp));
	pCmd->AddParameter(new MCmdParamUInt(GetGlobalClockCount()));
	RouteToListener(pObj, pCmd);
}

void MMatchServer::OnGameReportTimeSync(const MUID& uidComm, unsigned long nLocalTimeStamp, unsigned int nDataChecksum)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->UpdateTickLastPacketRecved();	// Last Packet Timestamp

	if (pObj->GetEnterBattle() == false)
		return;

	//// SpeedHack Test ////
	MMatchTimeSyncInfo* pSync = pObj->GetSyncInfo();
	int nSyncDiff = nLocalTimeStamp - pSync->GetLastSyncClock();
	float fError = static_cast<float>(nSyncDiff) / static_cast<float>(MATCH_CYCLE_CHECK_SPEEDHACK);
	if (fError > 2.f) {	
		pSync->AddFoulCount();
		if (pSync->GetFoulCount() >= 3) {	// 3연속 스피드핵 검출 - 3진아웃

			#ifndef _DEBUG		// 디버그할때는 빼놓았음
				NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GAME_SPEEDHACK);
				StageLeave(pObj->GetUID(), pObj->GetStageUID());
				DisconnectObject(pObj->GetUID());
			#endif
			mlog("SPEEDHACK : User='%s', SyncRatio=%f (TimeDiff=%d) \n", 
				pObj->GetName(), fError, nSyncDiff);
			pSync->ResetFoulCount();
		}
	} else {
		pSync->ResetFoulCount();
	}
	pSync->Update(GetGlobalClockCount());

	//// MemoryHack Test ////
	if (nDataChecksum > 0) {	// 서버가 Client MemoryChecksum 모르므로 일단 클라이언트가 신고하는의미로 사용한다.
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GAME_MEMORYHACK);
		StageLeave(pObj->GetUID(), pObj->GetStageUID());
		DisconnectObject(pObj->GetUID());
		mlog("MEMORYHACK : User='%s', MemoryChecksum=%u \n", pObj->GetName(), nDataChecksum);
	}
}

void MMatchServer::OnUpdateFinishedRound(const MUID& uidStage, const MUID& uidChar, 
						   void* pPeerInfo, void* pKillInfo)
{

}

void MMatchServer::OnRequestForcedEntry(const MUID& uidStage, const MUID& uidChar)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	MMatchObject* pObj = GetObject(uidChar);	
	if (pObj == NULL) return;

	pObj->SetForcedEntry(true);

	RouteResponseToListener(pObj, MC_MATCH_STAGE_RESPONSE_FORCED_ENTRY, MOK);
}

void MMatchServer::OnRequestSuicide(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	
	OnGameKill(uidPlayer, uidPlayer);


	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SUICIDE, MUID(0,0));
	int nResult = MOK;
	pNew->AddParameter(new MCommandParameterInt(nResult));
	pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	RouteToBattle(pObj->GetStageUID(), pNew);
}

void MMatchServer::OnRequestObtainWorldItem(const MUID& uidPlayer, const int nItemUID)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	pStage->ObtainWorldItem(pObj, nItemUID);
}

void MMatchServer::OnRequestSpawnWorldItem(const MUID& uidPlayer, const int nItemID, const float x, const float y, const float z)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	pStage->RequestSpawnWorldItem(pObj, nItemID, x, y, z);
}

float MMatchServer::GetDuelVictoryMultiflier(int nVictorty)
{
	return 1.0f;
}

float MMatchServer::GetDuelPlayersMultiflier(int nPlayerCount)
{
	return 1.0f;
}

void MMatchServer::CalcExpOnGameKill(MMatchStage* pStage, MMatchObject* pAttacker, MMatchObject* pVictim, 
					   int* poutAttackerExp, int* poutVictimExp)
{
	bool bSuicide = false;		// 자살
	if (pAttacker == pVictim) bSuicide = true;		

	MMATCH_GAMETYPE nGameType = pStage->GetStageSetting()->GetGameType();
	float fGameExpRatio = MGetGameTypeMgr()->GetInfo(nGameType)->fGameExpRatio;

	// 게임타입이 Training이면 바로 0리턴
	if (nGameType == MMATCH_GAMETYPE_TRAINING)
	{
		*poutAttackerExp = 0;
		*poutVictimExp = 0;
		return;
	}
	// 게임타입이 버서커일 경우
	else if (nGameType == MMATCH_GAMETYPE_BERSERKER)
	{
		MMatchRuleBerserker* pRuleBerserker = (MMatchRuleBerserker*)pStage->GetRule();

		if (pRuleBerserker->GetBerserker() == pAttacker->GetUID())
		{
			if (pAttacker != pVictim)
			{
				// 버서커는 경험치를 80%만 획득한다.
				fGameExpRatio = fGameExpRatio * 0.8f;
			}
			else
			{
				// 버서커는 자살 또는 피가 줄어 죽는경우 손실 경험치는 없도록 한다.
				fGameExpRatio = 0.0f;
			}
		}
	}
	else if (nGameType == MMATCH_GAMETYPE_DUEL)
	{
		MMatchRuleDuel* pRuleDuel = (MMatchRuleDuel*)pStage->GetRule();
		if (pVictim->GetUID() == pRuleDuel->uidChallenger)
		{
			fGameExpRatio *= GetDuelVictoryMultiflier(pRuleDuel->GetVictory());
		}
		else
		{
			fGameExpRatio *= GetDuelVictoryMultiflier(pRuleDuel->GetVictory()) * GetDuelPlayersMultiflier(pStage->GetPlayers());

		}
//		if (pRuleDuel->GetVictory() <= 1)
//		{
//			fGameExpRatio = fGameExpRatio * GetDuelPlayersMultiflier(pStage->GetPlayers()) * GetDuelVictoryMultiflier()
//		}
	}

	// 맵, 게임타입에 대한 경험치 비율 적용
	int nMapIndex = pStage->GetStageSetting()->GetMapIndex();
	if ((nMapIndex >=0) && (nMapIndex < MMATCH_MAP_COUNT))
	{
		float fMapExpRatio = g_MapDesc[nMapIndex].fExpRatio;
		fGameExpRatio = fGameExpRatio * fMapExpRatio;
	}

	int nAttackerLevel = pAttacker->GetCharInfo()->m_nLevel;
	int nVictimLevel = pVictim->GetCharInfo()->m_nLevel;

	// 경험치 계산
	int nAttackerExp = (int)(MMatchFormula::GetGettingExp(nAttackerLevel, nVictimLevel) * fGameExpRatio);
	int nVictimExp = (int)(MMatchFormula::CalcPanaltyEXP(nAttackerLevel, nVictimLevel) * fGameExpRatio);


	// 클랜전일 경우는 획득 경험치가 1.5배, 손실경험치 없음
	if ((MGetServerConfig()->GetServerMode() == MSM_CLAN) && (pStage->GetStageType() == MST_LADDER))
	{
		nAttackerExp = (int)((float)nAttackerExp * 1.5f);
		nVictimExp = 0;
	}

	// 고수채널, 초고수채널일 경우에는 경치다운 없음(자살제외)
	MMatchChannel* pOwnerChannel = FindChannel(pStage->GetOwnerChannel());
	if ((pOwnerChannel) && (!bSuicide))
	{
		if ((pOwnerChannel->GetRuleType() == MCHANNEL_RULE_MASTERY) || 
			(pOwnerChannel->GetRuleType() == MCHANNEL_RULE_ELITE))
		{
			nVictimExp=0;
		}
	}

	// 죽은사람이 운영자, 개발자일 경우 경험치 두배
	if ((pVictim->GetAccountInfo()->m_nUGrade == MMUG_ADMIN) || 
		(pVictim->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER))
	{
		nAttackerExp = nAttackerExp * 2;
	}
	// 죽인사람이 운영자, 개발자일 경우 경치다운 없음
	if ((!bSuicide) &&
		((pAttacker->GetAccountInfo()->m_nUGrade == MMUG_ADMIN) || 
		(pAttacker->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER)))
	{
		nVictimExp = 0;
	}

	// 자살일 경우 경험치 손실이 두배
	if (bSuicide) 
	{
		nVictimExp = (int)(MMatchFormula::GetSuicidePanaltyEXP(nVictimLevel) * fGameExpRatio);
		nAttackerExp = 0;
	}

	// 팀킬인경우 경험치 제로
	if ((pStage->GetStageSetting()->IsTeamPlay()) && (pAttacker->GetTeam() == pVictim->GetTeam()))
	{
		nAttackerExp = 0;
	}


	// 팀전일 경우 경험치 배분
	if (pStage->IsApplyTeamBonus())
	{
		int nTeamBonus = 0;
		if (pStage->GetRule() != NULL)
		{
			int nNewAttackerExp = nAttackerExp;
			pStage->GetRule()->CalcTeamBonus(pAttacker, pVictim, nAttackerExp, &nNewAttackerExp, &nTeamBonus);
			nAttackerExp = nNewAttackerExp;
		}

		// 팀 경험치 적립
		pStage->AddTeamBonus(nTeamBonus, MMatchTeam(pAttacker->GetTeam()));
	}

	// xp 보너스 적용(넷마블 PC방, 경험치 반지)
	int nAttackerExpBonus = 0;
	if (nAttackerExp != 0)
	{
		MMatchItemBonusType nBonusType = GetStageBonusType(pStage->GetStageSetting());
		const float fAttackerExpBonusRatio = MMatchFormula::CalcXPBonusRatio(pAttacker, nBonusType);
		nAttackerExpBonus = (int)(nAttackerExp * fAttackerExpBonusRatio);
	}

	*poutAttackerExp = nAttackerExp + nAttackerExpBonus;
	*poutVictimExp = nVictimExp;
}


const int MMatchServer::CalcBPonGameKill( MMatchStage* pStage, MMatchObject* pAttacker, const int nAttackerLevel, const int nVictimLevel )
{
	if( (0 == pStage) || (0 == pAttacker) ) 
		return -1;

	const int	nAddedBP		= static_cast< int >( MMatchFormula::GetGettingBounty(nAttackerLevel, nVictimLevel) );
	const float fBPBonusRatio	= MMatchFormula::CalcBPBounsRatio( pAttacker, GetStageBonusType(pStage->GetStageSetting()) );
	const int	nBPBonus		= static_cast< int >( nAddedBP * fBPBonusRatio );

	return nAddedBP + nBPBonus;
}


// 적을 죽였을 경우 경험치 계산
void MMatchServer::ProcessOnGameKill(MMatchStage* pStage, MMatchObject* pAttacker, MMatchObject* pVictim)
{
	/*
		경험치 계산
		죽었다는 메세지 전송
		캐릭터에 경험치 적용
		레벨 계산
		Kill, Death, 바운티 계산, 적용
		DB캐싱 업데이트
		레벨업 메세지 전송
	*/

	bool bSuicide = false;		// 자살
	if (pAttacker == pVictim) bSuicide = true;		

	int nAttackerExp = 0;
	int nVictimExp = 0;
	int nAttackerLevel = pAttacker->GetCharInfo()->m_nLevel;
	int nVictimLevel = pVictim->GetCharInfo()->m_nLevel;
	MMATCH_GAMETYPE nGameType = pStage->GetStageSetting()->GetGameType();
	float fGameExpRatio = MGetGameTypeMgr()->GetInfo(nGameType)->fGameExpRatio;
	MUID uidStage = pAttacker->GetStageUID();

	// 경험치 계산
	CalcExpOnGameKill(pStage, pAttacker, pVictim, &nAttackerExp, &nVictimExp);

	// 죽었다는 메세지를 방전체에게 보낸다.
	PostGameDeadOnGameKill(uidStage, pAttacker, pVictim, nAttackerExp, nVictimExp);

	// 만약 게임모드가 Training이면 Xp등의 업데이트를 하지 않는다.
	if (nGameType == MMATCH_GAMETYPE_TRAINING) return;


	// 캐릭터 XP 업데이트
	pAttacker->GetCharInfo()->IncXP(nAttackerExp);
	pVictim->GetCharInfo()->DecXP(nVictimExp);

	// 레벨 계산
	int nNewAttackerLevel = -1, nNewVictimLevel = -1;
	if ((!bSuicide) && (pAttacker->GetCharInfo()->m_nLevel < MAX_LEVEL) &&
		(pAttacker->GetCharInfo()->m_nXP >= MMatchFormula::GetNeedExp(nAttackerLevel)))
	{
		nNewAttackerLevel = MMatchFormula::GetLevelFromExp(pAttacker->GetCharInfo()->m_nXP);
		if (nNewAttackerLevel != pAttacker->GetCharInfo()->m_nLevel) pAttacker->GetCharInfo()->m_nLevel = nNewAttackerLevel;
	}

	if ((pVictim->GetCharInfo()->m_nLevel > 0) &&
		(pVictim->GetCharInfo()->m_nXP < MMatchFormula::GetNeedExp(nVictimLevel-1)))
	{
		nNewVictimLevel = MMatchFormula::GetLevelFromExp(pVictim->GetCharInfo()->m_nXP);
		if (nNewVictimLevel != pVictim->GetCharInfo()->m_nLevel) pVictim->GetCharInfo()->m_nLevel = nNewVictimLevel;
	}

	if (!bSuicide)
	{
		// KillCount 계산
		pAttacker->GetCharInfo()->IncKill();

		// 바운티 추가해준다
		// int nAddedBP = (int)(MMatchFormula::GetGettingBounty(nAttackerLevel, nVictimLevel) * fGameExpRatio);
		const int nBPBonus = CalcBPonGameKill( pStage, pAttacker, nAttackerLevel, nVictimLevel );

		pAttacker->GetCharInfo()->IncBP(nBPBonus);
	}

	// DeathCount 계산
	pVictim->GetCharInfo()->IncDeath();

	// DB 캐슁 업데이트
	if (pAttacker->GetCharInfo()->GetDBCachingData()->IsRequestUpdate())
	{
		UpdateCharDBCachingData(pAttacker);
	}
	if (pVictim->GetCharInfo()->GetDBCachingData()->IsRequestUpdate())
	{
		UpdateCharDBCachingData(pAttacker);
	}

	// 만약 레벨이 바뀌면 따로 레벨업한다.
	if ((!bSuicide) && (nNewAttackerLevel >= 0) && (nNewAttackerLevel != nAttackerLevel))
	{
		// 레벨이 바뀌면 바로 캐슁 업데이트한다
		UpdateCharDBCachingData(pAttacker);

		pAttacker->GetCharInfo()->m_nLevel = nNewAttackerLevel;
		if (!m_MatchDBMgr.UpdateCharLevel(pAttacker->GetCharInfo()->m_nCID, 
										  nNewAttackerLevel, 
										  pAttacker->GetCharInfo()->m_nBP,
										  pAttacker->GetCharInfo()->m_nTotalKillCount, 
										  pAttacker->GetCharInfo()->m_nTotalDeathCount,
										  pAttacker->GetCharInfo()->m_nTotalPlayTimeSec,
										  true))
		{
			mlog("DB UpdateCharLevel Error : %s\n", pAttacker->GetCharInfo()->m_szName);
		}
	}
	if ((nNewVictimLevel >= 0) && (nNewVictimLevel != nVictimLevel))
	{
		// 레벨이 바뀌면 바로 캐슁 업데이트한다
		UpdateCharDBCachingData(pVictim);

		pVictim->GetCharInfo()->m_nLevel = nNewVictimLevel;
		if (!m_MatchDBMgr.UpdateCharLevel(pVictim->GetCharInfo()->m_nCID, 
			                              nNewVictimLevel,
										  pVictim->GetCharInfo()->m_nBP,
										  pVictim->GetCharInfo()->m_nTotalKillCount,
										  pVictim->GetCharInfo()->m_nTotalDeathCount,
										  pVictim->GetCharInfo()->m_nTotalPlayTimeSec,
										  false
										  ))
		{
			mlog("DB UpdateCharLevel Error : %s\n", pVictim->GetCharInfo()->m_szName);
		}
	}



	// 레벨업, 레벨 다운 메세지 보내기
	if ((!bSuicide) && (nNewAttackerLevel >= 0) && (nNewAttackerLevel > nAttackerLevel))
	{
		MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_UP, MUID(0,0));
		pCmd->AddParameter(new MCommandParameterUID(pAttacker->GetUID()));
		pCmd->AddParameter(new MCommandParameterInt(nNewAttackerLevel));
		RouteToBattle(uidStage, pCmd);	

	}
	if ((nNewVictimLevel >= 0) && (nNewVictimLevel < nVictimLevel))
	{
		MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_DOWN, MUID(0,0));
		pCmd->AddParameter(new MCommandParameterUID(pVictim->GetUID()));
		pCmd->AddParameter(new MCommandParameterInt(nNewVictimLevel));
		RouteToBattle(uidStage, pCmd);	
	}
}


void MMatchServer::ProcessPlayerXPBP(MMatchStage* pStage, MMatchObject* pPlayer, int nAddedXP, int nAddedBP)
{
	if (pStage == NULL) return;
	if (!IsEnabledObject(pPlayer)) return;

	/*
		경험치 계산
		캐릭터에 경험치 적용
		레벨 계산
		DB캐싱 업데이트
		레벨업,다운 메세지 전송
	*/

	MUID uidStage = pPlayer->GetStageUID();
	int nPlayerLevel = pPlayer->GetCharInfo()->m_nLevel;

	// 캐릭터 XP 업데이트
	pPlayer->GetCharInfo()->IncXP(nAddedXP);

	// 레벨 계산
	int nNewPlayerLevel = -1;
	if ((pPlayer->GetCharInfo()->m_nLevel < MAX_LEVEL) &&
		(pPlayer->GetCharInfo()->m_nXP >= MMatchFormula::GetNeedExp(nPlayerLevel)))
	{
		nNewPlayerLevel = MMatchFormula::GetLevelFromExp(pPlayer->GetCharInfo()->m_nXP);
		if (nNewPlayerLevel != pPlayer->GetCharInfo()->m_nLevel) pPlayer->GetCharInfo()->m_nLevel = nNewPlayerLevel;
	}

	// 바운티 추가해준다
	pPlayer->GetCharInfo()->IncBP(nAddedBP);


	// DB 캐슁 업데이트
	if (pPlayer->GetCharInfo()->GetDBCachingData()->IsRequestUpdate())
	{
		UpdateCharDBCachingData(pPlayer);
	}

	// 만약 레벨이 바뀌면 따로 레벨업한다.
	if ((nNewPlayerLevel >= 0) && (nNewPlayerLevel != nPlayerLevel))
	{
		// 레벨이 바뀌면 바로 캐슁 업데이트한다
		UpdateCharDBCachingData(pPlayer);

		pPlayer->GetCharInfo()->m_nLevel = nNewPlayerLevel;
		if (!m_MatchDBMgr.UpdateCharLevel(pPlayer->GetCharInfo()->m_nCID, 
										  nNewPlayerLevel, 
										  pPlayer->GetCharInfo()->m_nBP,
										  pPlayer->GetCharInfo()->m_nTotalKillCount, 
										  pPlayer->GetCharInfo()->m_nTotalDeathCount,
										  pPlayer->GetCharInfo()->m_nTotalPlayTimeSec,
										  true))
		{
			mlog("DB UpdateCharLevel Error : %s\n", pPlayer->GetCharInfo()->m_szName);
		}
	}

	// 레벨업, 레벨 다운 메세지 보내기
	if (nNewPlayerLevel > 0)
	{
		if (nNewPlayerLevel > nPlayerLevel)
		{
			MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_UP, MUID(0,0));
			pCmd->AddParameter(new MCommandParameterUID(pPlayer->GetUID()));
			pCmd->AddParameter(new MCommandParameterInt(nNewPlayerLevel));
			RouteToBattle(uidStage, pCmd);	
		}
		else if (nNewPlayerLevel < nPlayerLevel)
		{
			MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_DOWN, MUID(0,0));
			pCmd->AddParameter(new MCommandParameterUID(pPlayer->GetUID()));
			pCmd->AddParameter(new MCommandParameterInt(nNewPlayerLevel));
			RouteToBattle(uidStage, pCmd);	
		}
	}
}

// 팀 보너스 적용
void MMatchServer::ApplyObjectTeamBonus(MMatchObject* pObject, int nAddedExp)
{
	if (!IsEnabledObject(pObject)) return;
	if (nAddedExp <= 0)
	{
		_ASSERT(0);
		return;
	}
	
	bool bIsLevelUp = false;

	// 보너스 적용
	if (nAddedExp != 0)
	{
		int nExpBonus = (int)(nAddedExp * MMatchFormula::CalcXPBonusRatio(pObject, MIBT_TEAM));
		nAddedExp += nExpBonus;
	}




	// 캐릭터 XP 업데이트
	pObject->GetCharInfo()->IncXP(nAddedExp);

	// 레벨 계산
	int nNewLevel = -1;
	int nCurrLevel = pObject->GetCharInfo()->m_nLevel;

	if (nNewLevel > nCurrLevel) bIsLevelUp = true;

	if ((pObject->GetCharInfo()->m_nLevel < MAX_LEVEL) &&
		(pObject->GetCharInfo()->m_nXP >= MMatchFormula::GetNeedExp(nCurrLevel)))
	{
		nNewLevel = MMatchFormula::GetLevelFromExp(pObject->GetCharInfo()->m_nXP);
		if (nNewLevel != nCurrLevel) pObject->GetCharInfo()->m_nLevel = nNewLevel;
	}

	// DB 캐슁 업데이트
	if (pObject->GetCharInfo()->GetDBCachingData()->IsRequestUpdate())
	{
		UpdateCharDBCachingData(pObject);
	}

	// 만약 레벨이 바뀌면 바로 레벨업한다.
	if ((nNewLevel >= 0) && (nNewLevel != nCurrLevel))
	{
		// 레벨이 바뀌면 바로 캐슁 업데이트한다
		UpdateCharDBCachingData(pObject);

		pObject->GetCharInfo()->m_nLevel = nNewLevel;
		nCurrLevel = nNewLevel;

		if (!m_MatchDBMgr.UpdateCharLevel(pObject->GetCharInfo()->m_nCID, 
			                              nNewLevel,
										  pObject->GetCharInfo()->m_nBP,
										  pObject->GetCharInfo()->m_nTotalKillCount,
										  pObject->GetCharInfo()->m_nTotalDeathCount,
										  pObject->GetCharInfo()->m_nTotalPlayTimeSec,
										  bIsLevelUp
										  ))
		{
			mlog("DB UpdateCharLevel Error : %s\n", pObject->GetCharInfo()->m_szName);
		}
	}


	MUID uidStage = pObject->GetStageUID();

	unsigned long int nExpArg;
	unsigned long int nChrExp;
	int nPercent;

	nChrExp = pObject->GetCharInfo()->m_nXP;
	nPercent = MMatchFormula::GetLevelPercent(nChrExp, nCurrLevel);
	// 상위 2바이트는 경험치, 하위 2바이트는 경험치의 퍼센트이다.
	nExpArg = MakeExpTransData(nAddedExp, nPercent);


	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_TEAMBONUS, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(pObject->GetUID()));
	pCmd->AddParameter(new MCommandParameterUInt(nExpArg));
	RouteToBattle(uidStage, pCmd);	


	// 레벨업 메세지 보내기
	if ((nNewLevel >= 0) && (nNewLevel > nCurrLevel))
	{
		MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_UP, MUID(0,0));
		pCmd->AddParameter(new MCommandParameterUID(pObject->GetUID()));
		pCmd->AddParameter(new MCommandParameterInt(nNewLevel));
		RouteToBattle(uidStage, pCmd);	
	}
}

void MMatchServer::PostGameDeadOnGameKill(MUID& uidStage, MMatchObject* pAttacker, MMatchObject* pVictim,
									int nAddedAttackerExp, int nSubedVictimExp)
{
	unsigned long int nAttackerArg = 0;
	unsigned long int nVictimArg =0;

	int nRealAttackerLevel = pAttacker->GetCharInfo()->m_nLevel;
	int nRealVictimLevel = pVictim->GetCharInfo()->m_nLevel;

	unsigned long int nChrExp;
	int nPercent;

	nChrExp = pAttacker->GetCharInfo()->m_nXP;
	nPercent = MMatchFormula::GetLevelPercent(nChrExp, nRealAttackerLevel);
	nAttackerArg = MakeExpTransData(nAddedAttackerExp, nPercent);

	nChrExp = pVictim->GetCharInfo()->m_nXP;
	nPercent = MMatchFormula::GetLevelPercent(nChrExp, nRealVictimLevel);
	nVictimArg = MakeExpTransData(nSubedVictimExp, nPercent);

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_DEAD, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(pAttacker->GetUID()));
	pCmd->AddParameter(new MCommandParameterUInt(nAttackerArg));
	pCmd->AddParameter(new MCommandParameterUID(pVictim->GetUID()));
	pCmd->AddParameter(new MCommandParameterUInt(nVictimArg));
	RouteToBattle(uidStage, pCmd);	
}

void MMatchServer::StageList(const MUID& uidPlayer, int nStageStartIndex, bool bCacheUpdate)
{
	MMatchObject* pChar = GetObject(uidPlayer);
	if (pChar == NULL) return;
	MMatchChannel* pChannel = FindChannel(pChar->GetChannelUID());
	if (pChannel == NULL) return;

	// 클랜서버인데 클랜채널일 경우에는 방 리스트대신 대기중 클랜 리스트를 보낸다.
	if ((MGetServerConfig()->GetServerMode() == MSM_CLAN) && (pChannel->GetChannelType() == MCHANNEL_TYPE_CLAN))
	{
		StandbyClanList(uidPlayer, nStageStartIndex, bCacheUpdate);
		return;
	}


	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_LIST), MUID(0,0), m_This);

	int nPrevStageCount = -1, nNextStageCount = -1;
	int nNextStageIndex = pChannel->GetMaxPlayers()-1;

	int nRealStageCount = 0;
	for (int i = nStageStartIndex; i < pChannel->GetMaxPlayers(); i++)
	{
		if (pChannel->IsEmptyStage(i)) continue;
		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;

		nRealStageCount++;
		if (nRealStageCount >= TRANS_STAGELIST_NODE_COUNT) 
		{
			nNextStageIndex = i;
			break;
		}
	}

	if (!bCacheUpdate)
	{
		nPrevStageCount = pChannel->GetPrevStageCount(nStageStartIndex);
		nNextStageCount = pChannel->GetNextStageCount(nNextStageIndex);
	}

	pNew->AddParameter(new MCommandParameterChar((char)nPrevStageCount));
	pNew->AddParameter(new MCommandParameterChar((char)nNextStageCount));


	void* pStageArray = MMakeBlobArray(sizeof(MTD_StageListNode), nRealStageCount);
	int nArrayIndex=0;

	for (int i = nStageStartIndex; i < pChannel->GetMaxPlayers(); i++)
	{
		if (pChannel->IsEmptyStage(i)) continue;
		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;

		if (nArrayIndex >= nRealStageCount) break;

		MTD_StageListNode* pNode = (MTD_StageListNode*)MGetBlobArrayElement(pStageArray, nArrayIndex++);
		pNode->uidStage = pStage->GetUID();
		strcpy_safe(pNode->szStageName, pStage->GetName());
		pNode->nNo = (unsigned char)(pStage->GetIndex() + 1);	// 사용자에게 보여주는 인덱스는 1부터 시작한다
		pNode->nPlayers = (char)pStage->GetPlayers();
		pNode->nMaxPlayers = pStage->GetStageSetting()->GetMaxPlayers();
		pNode->nState = pStage->GetState();
		pNode->nGameType = pStage->GetStageSetting()->GetGameType();
		pNode->nMapIndex = pStage->GetStageSetting()->GetMapIndex();
		
		pNode->nSettingFlag = 0;
		// 난입
		if (pStage->GetStageSetting()->GetForcedEntry())
		{
			pNode->nSettingFlag |= MSTAGENODE_FLAG_FORCEDENTRY_ENABLED;
		}
		// 비밀방
		if (pStage->IsPrivate())
		{
			pNode->nSettingFlag |= MSTAGENODE_FLAG_PRIVATE;
		}
		// 레벨제한
		pNode->nLimitLevel = pStage->GetStageSetting()->GetLimitLevel();
		pNode->nMasterLevel = 0;

		if (pNode->nLimitLevel != 0)
		{
			pNode->nSettingFlag |= MSTAGENODE_FLAG_LIMITLEVEL;

			;
			MMatchObject* pMaster = GetObject(pStage->GetMasterUID());
			if (pMaster)
			{
				if (pMaster->GetCharInfo())
				{
					pNode->nMasterLevel = pMaster->GetCharInfo()->m_nLevel;
				}
			}
		}
	}

	pNew->AddParameter(new MCommandParameterBlob(pStageArray, MGetBlobArraySize(pStageArray)));
	MEraseBlobArray(pStageArray);

	RouteToListener(pChar, pNew);	
}


void MMatchServer::OnStageRequestStageList(const MUID& uidPlayer, const MUID& uidChannel, const int nStageCursor)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return;

	pObj->SetStageCursor(nStageCursor);
	StageList(pObj->GetUID(), nStageCursor, false);
}


void MMatchServer::OnRequestQuickJoin(const MUID& uidPlayer, void* pQuickJoinBlob)
{
	MTD_QuickJoinParam* pNode = (MTD_QuickJoinParam*)MGetBlobArrayElement(pQuickJoinBlob, 0);
	ResponseQuickJoin(uidPlayer, pNode);
}

void MMatchServer::ResponseQuickJoin(const MUID& uidPlayer, MTD_QuickJoinParam* pQuickJoinParam)
{
	if (pQuickJoinParam == NULL) return;

	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;
	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
	if (pChannel == NULL) return;

	list<MUID>	recommended_stage_list;
	MUID uidRecommendedStage = MUID(0,0);
	int nQuickJoinResult = MOK;


	for (int i = 0; i < pChannel->GetMaxStages(); i++)
	{
		if (pChannel->IsEmptyStage(i)) continue;
		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;

		int ret = ValidateStageJoin(pObj->GetUID(), pStage->GetUID());
		if (ret == MOK)
		{
			if (pStage->IsPrivate()) continue;

			int nMapIndex = pStage->GetStageSetting()->GetMapIndex();
			int nGameType = pStage->GetStageSetting()->GetGameType();

			if (!CheckBitSet(pQuickJoinParam->nMapEnum, nMapIndex)) continue;
			if (!CheckBitSet(pQuickJoinParam->nModeEnum, nGameType)) continue;

			//if (((1 << nMapIndex) & (pQuickJoinParam->nMapEnum)) == 0) continue;
			//if (((1 << nGameType) & (pQuickJoinParam->nModeEnum)) == 0) continue;

			recommended_stage_list.push_back(pStage->GetUID());
		}
	}

	if (!recommended_stage_list.empty())
	{
		int nSize=(int)recommended_stage_list.size();
		int nIndex = rand() % nSize;

		int nCnt = 0;
		for (list<MUID>::iterator itor = recommended_stage_list.begin(); itor != recommended_stage_list.end(); ++itor)
		{
			if (nIndex == nCnt)
			{
				uidRecommendedStage = (*itor);
				break;
			}
			nCnt++;
		}
	}
	else
	{
		nQuickJoinResult = MERR_CANNOT_NO_STAGE;
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_RESPONSE_QUICKJOIN, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterInt(nQuickJoinResult));
	pCmd->AddParameter(new MCommandParameterUID(uidRecommendedStage));
	RouteToListener(pObj, pCmd);	
}

static int __cdecl _int_sortfunc(const void* a, const void* b)
{
	return *((int*)a) - *((int*)b);
}


int MMatchServer::GetLadderTeamIDFromDB(const int nTeamTableIndex, const int* pnMemberCIDArray, const int nMemberCount)
{
	if ((nMemberCount <= 0) || (nTeamTableIndex != nMemberCount))
	{
		_ASSERT(0);
		return 0;
	}

	// cid 오름차순으로 소팅 - db상에 소팅되어 들어가있다. 
	int* pnSortedCIDs = new int[nMemberCount];
	for (int i = 0; i < nMemberCount; i++)
	{
		pnSortedCIDs[i] = pnMemberCIDArray[i];
	}
	qsort(pnSortedCIDs, nMemberCount, sizeof(int), _int_sortfunc);

	int nTID = 0;
	if (pnSortedCIDs[0] != 0)
	{
		if (!m_MatchDBMgr.GetLadderTeamID(nTeamTableIndex, pnSortedCIDs, nMemberCount, &nTID))
		{
			nTID = 0;
		}
	}
	

	delete[] pnSortedCIDs;

	return nTID;
}

void MMatchServer::SaveLadderTeamPointToDB(const int nTeamTableIndex, const int nWinnerTeamID, const int nLoserTeamID, const bool bIsDrawGame)
{
	// 포인트 계산 - 액션리그 전용
	int nWinnerPoint = 0, nLoserPoint = 0, nDrawPoint = 0;

	nLoserPoint = -1;
	switch (nTeamTableIndex)
	{
	case 2:	// 2대2
		{
			nWinnerPoint = 4;
			nDrawPoint = 1;
		}
		break;
	case 3:
		{
			nWinnerPoint = 6;
			nDrawPoint = 1;
		}
		break;
	case 4:
		{
			nWinnerPoint = 10;
			nDrawPoint = 2;
		}
		break;
	}

	if (!m_MatchDBMgr.LadderTeamWinTheGame(nTeamTableIndex, nWinnerTeamID, nLoserTeamID, bIsDrawGame,
		                                   nWinnerPoint, nLoserPoint, nDrawPoint))
	{
		mlog("DB Query(SaveLadderTeamPointToDB) Failed\n");
	}
}


void MMatchServer::OnVoteCallVote(const MUID& uidPlayer, const char* pszDiscuss, const char* pszArg)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	// 운영자가 강퇴투표하면 강제로 강퇴
	if (IsAdminGrade(pObj)) {
		MMatchStage* pStage = FindStage(pObj->GetStageUID());
		if (pStage)
			pStage->KickBanPlayer(pszArg, false);
		return;
	}

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// 운영자가 같이 게임중이면 투표 불가능
	for (MUIDRefCache::iterator itor = pStage->GetObjBegin(); itor != pStage->GetObjEnd(); itor++) {
		MUID uidObj = (MUID)(*itor).first;
		MMatchObject* pPlayer = (MMatchObject*)GetObject(uidObj);
		if ((pPlayer) && (IsAdminGrade(pPlayer)))
		{
			Announce(uidPlayer, (char*)MErrStr( MERR_CANNOT_VOTE));

			return;
		}
	}


	if( pObj->WasCallVote() )
	{
		Announce(uidPlayer, (char*)MErrStr( MERR_CANNOT_VOTE));

		return;
	}

	// 투표를 했다는걸 표시해놓음.
	pObj->SetVoteState( true );

	if (pStage->GetStageType() == MST_LADDER)
	{
		Announce(uidPlayer, (char*)MErrStr( MERR_CANNOT_VOTE_LADERGAME));

		return;
	}
#ifdef _VOTESETTING
	// 방 설정중 투표기능을 검사함.
	if( !pStage->GetStageSetting()->bVoteEnabled ) {
		VoteAbort( uidPlayer );
		return;
	}

	// 이번 게임에서 투표를 건의했는지 검사.
	if( pStage->WasCallVote() ) {
		VoteAbort( uidPlayer );
		return;
	}
	else {
		pStage->SetVoteState( true );
	}
#endif

	if (pStage->GetVoteMgr()->GetDiscuss())
	{
		Announce(uidPlayer, (char*)MErrStr( MERR_VOTE_ALREADY_START));

		return;
	}

	MVoteDiscuss* pDiscuss = MVoteDiscussBuilder::Build(uidPlayer, pStage->GetUID(), pszDiscuss, pszArg);
	if (pDiscuss == NULL) return;

	if (pStage->GetVoteMgr()->CallVote(pDiscuss)) {
		pDiscuss->Vote(uidPlayer, MVOTE_YES);	// 발의자 무조건 찬성

		MCommand* pCmd = CreateCommand(MC_MATCH_NOTIFY_CALLVOTE, MUID(0,0));
		pCmd->AddParameter(new MCmdParamStr(pszDiscuss));
		pCmd->AddParameter(new MCmdParamStr(pszArg));
		RouteToStage(pStage->GetUID(), pCmd);
		return;
	}
	else
	{
		Announce(uidPlayer, (char*)MErrStr( MERR_VOTE_FAILED));

		return;
	}
}

void MMatchServer::OnVoteYes(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	MVoteDiscuss* pDiscuss = pStage->GetVoteMgr()->GetDiscuss();
    if (pDiscuss == NULL) return;

	pDiscuss->Vote(uidPlayer, MVOTE_YES);
}

void MMatchServer::OnVoteNo(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	MVoteDiscuss* pDiscuss = pStage->GetVoteMgr()->GetDiscuss();
    if (pDiscuss == NULL) return;

	pDiscuss->Vote(uidPlayer, MVOTE_NO);
}

void MMatchServer::VoteAbort( const MUID& uidPlayer )
{
#ifndef MERR_CANNOT_VOTE
#define MERR_CANNOT_VOTE 120000
#endif

	MMatchObject* pObj = GetObject( uidPlayer );
	if( 0 == pObj )
		return;

	MCommand* pCmd = CreateCommand( MC_MATCH_VOTE_RESPONSE, MUID(0, 0) );
	if( 0 == pCmd )
		return;

	pCmd->AddParameter( new MCommandParameterInt(MERR_CANNOT_VOTE) );
	RouteToListener( pObj, pCmd );
}



void MMatchServer::OnEventChangeMaster(const MUID& uidAdmin)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// 관리자 권한을 가진 사람이 아니면 연결을 끊는다.
	if (!IsAdminGrade(pObj))
	{
//		DisconnectObject(uidAdmin);		
		return;
	}

	if (pStage->GetMasterUID() == uidAdmin)
		return;

	pStage->SetMasterUID(uidAdmin);
	StageMaster(pStage->GetUID());
}

void MMatchServer::OnEventChangePassword(const MUID& uidAdmin, const char* pszPassword)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// 관리자 권한을 가진 사람이 아니면 연결을 끊는다.
	if (!IsAdminGrade(pObj))
	{
//		DisconnectObject(uidAdmin);		
		return;
	}

	pStage->SetPassword(pszPassword);
	pStage->SetPrivate(true);
}

void MMatchServer::OnEventRequestJjang(const MUID& uidAdmin, const char* pszTargetName)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// 관리자 권한을 가진 사람이 아니면 무시
	if (!IsAdminGrade(pObj))
	{
		return;
	}

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) return;
	if (IsAdminGrade(pTargetObj)) return;		// 어드민 대상으로 짱불가
	if (MMUG_STAR == pTargetObj->GetAccountInfo()->m_nUGrade) return;	// 이미 짱

	pTargetObj->GetAccountInfo()->m_nUGrade = MMUG_STAR;

	if (m_MatchDBMgr.EventJjangUpdate(pTargetObj->GetAccountInfo()->m_nAID, true)) {
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.AddObject(pTargetObj);
		MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_REPLACE, this);
		RouteToStage(pStage->GetUID(), pCmdCacheUpdate);

		MCommand* pCmdUIUpdate = CreateCommand(MC_EVENT_UPDATE_JJANG, MUID(0,0));
		pCmdUIUpdate->AddParameter(new MCommandParameterUID(pTargetObj->GetUID()));
		pCmdUIUpdate->AddParameter(new MCommandParameterBool(true));
		RouteToStage(pStage->GetUID(), pCmdUIUpdate);
	}
}

void MMatchServer::OnEventRemoveJjang(const MUID& uidAdmin, const char* pszTargetName)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// 관리자 권한을 가진 사람이 아니면 연결을 끊는다.
	if (!IsAdminGrade(pObj))
	{
		return;
	}
	
	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) return;			// 어드민 대상으로 짱불가

	pTargetObj->GetAccountInfo()->m_nUGrade = MMUG_FREE;

	if (m_MatchDBMgr.EventJjangUpdate(pTargetObj->GetAccountInfo()->m_nAID, false)) {
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.AddObject(pTargetObj);
		MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_REPLACE, this);
		RouteToStage(pStage->GetUID(), pCmdCacheUpdate);

		MCommand* pCmdUIUpdate = CreateCommand(MC_EVENT_UPDATE_JJANG, MUID(0,0));
		pCmdUIUpdate->AddParameter(new MCommandParameterUID(pTargetObj->GetUID()));
		pCmdUIUpdate->AddParameter(new MCommandParameterBool(false));
		RouteToStage(pStage->GetUID(), pCmdUIUpdate);
	}
}


void MMatchServer::OnStageGo(const MUID& uidPlayer, unsigned int nRoomNo)
{
	MMatchObject* pChar = GetObject(uidPlayer);
	if (!IsEnabledObject(pChar)) return;
	if (pChar->GetPlace() != MMP_LOBBY) return;
	MMatchChannel* pChannel = FindChannel(pChar->GetChannelUID());
	if (pChannel == NULL) return;

	MMatchStage* pStage = pChannel->GetStage(nRoomNo-1);
	if (pStage) {
		MCommand* pNew = CreateCommand(MC_MATCH_REQUEST_STAGE_JOIN, GetUID());
		pNew->AddParameter(new MCommandParameterUID(uidPlayer));
		pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
		Post(pNew);
	}
}



void MMatchServer::OnDuelQueueInfo(const MUID& uidStage, const MTD_DuelQueueInfo& QueueInfo)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_DUEL_QUEUEINFO, MUID(0,0));
	pCmd->AddParameter(new MCmdParamBlob(&QueueInfo, sizeof(MTD_DuelQueueInfo)));
	RouteToBattle(uidStage, pCmd);
}


void MMatchServer::OnQuestSendPing(const MUID& uidStage, unsigned long int t)
{
	MCommand* pCmd = CreateCommand(MC_QUEST_PING, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUInt(t));
	RouteToBattle(uidStage, pCmd);
}