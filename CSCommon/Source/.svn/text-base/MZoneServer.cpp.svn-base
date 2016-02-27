#include "MZoneServer.h"
#include "MMap.h"
#include "MSharedCommandTable.h"
#include "MMaster.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MObjectTypes.h"
#include "MObjectItem.h"
#include "MCharacterDesc.h"
#include "MSkillDesc.h"
#include "MAction.h"
#include "MActionPipe.h"
#include "MActParam.h"

using namespace RealSpace2;
#include "RBspObject.h"
//#include "RPathFinder.h"


MZoneServer* MZoneServer::m_pInstance = NULL;

////////////////////////////////////
void RcpLog(const char *pFormat,...)
{
	char szBuf[256];

	va_list args;

	va_start(args,pFormat);
	vsprintf(szBuf, pFormat, args);
	va_end(args);

	int nEnd = (int)strlen(szBuf)-1;
	if ((nEnd >= 0) && (szBuf[nEnd] == '\n')) {
		szBuf[nEnd] = NULL;
		strcat(szBuf, "\n");
	}
	OutputDebugString(szBuf);
}
////////////////////////////////////

bool MZoneServer::OnCreate(void)
{
	return true;
}
void MZoneServer::OnDestroy(void)
{
}

void MZoneServer::OnRegisterCommand(MCommandManager* pCommandManager)
{
	MCommandCommunicator::OnRegisterCommand(pCommandManager);
	MAddSharedCommandTable(pCommandManager, MSCT_ZONESERVER);
	Log("Command registeration completed");
}

bool MZoneServer::OnCommand(MCommand* pCommand)
{
	switch(pCommand->GetID()){
		case MC_LOCAL_LOGIN:
			{
				MUID uidComm, uidPlayer;
				pCommand->GetParameter(&uidComm, 0, MPT_UID);
				pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				OnLogin(uidComm, uidPlayer);
				return true;
			}
		case MC_DEBUG_TEST:
			{
				DebugTest();
				return true;
			}
		case MC_NET_REQUEST_INFO:
			{
				ResponseInfo(pCommand->m_Sender);
				return true;
			}
		case MC_NET_ECHO:
			{
				char szMessage[256];
				if (pCommand->GetParameter(szMessage, 0, MPT_STR)==false) break;

				MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_ECHO), pCommand->m_Sender, m_This);
				pNew->AddParameter(new MCommandParameterString(szMessage));
				Post(pNew);
				return true;
			}
		case MC_NET_RESPONSE_UID:
			{
				MUID uidStart, uidEnd;
				pCommand->GetParameter(&uidStart, 0, MPT_UID);
				pCommand->GetParameter(&uidEnd, 1, MPT_UID);
				m_CommAllocRange.Start = uidStart;
				m_CommAllocRange.End = uidEnd;
				LOG("UID-space reservation completed (%u:%u ~ %u:%u)", m_CommAllocRange.Start.High, m_CommAllocRange.Start.Low, m_CommAllocRange.End.High, m_CommAllocRange.End.Low);
				OnInitialize();
				return true;
			}
		case MC_NET_CLEAR:
			{
				MUID uid;
				if (pCommand->GetParameter(&uid, 0, MPT_UID)==false) break;

				ReleasePlayerCharacter(uid);

				LOG("Release communication UID (%u:%u)", uid.High, uid.Low);
				return true;
			}
		case MC_NET_CHECKPING:
			{
				MUID uid;
				if (pCommand->GetParameter(&uid, 0, MPT_UID)==false) break;
				MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_PING), uid, m_This);
				pNew->AddParameter(new MCommandParameterUInt(timeGetTime()));
				Post(pNew);
				return true;
			}
		case MC_NET_PING:
			{
				unsigned int nTimeStamp;
				if (pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT)==false) break;
				MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_PONG), pCommand->m_Sender, m_This);
				pNew->AddParameter(new MCommandParameterUInt(nTimeStamp));
				Post(pNew);
				return true;
			}
		case MC_NET_PONG:
			{
				int nTimeStamp;
				pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT);
				LOG("Ping from (%u:%u) = %d", pCommand->GetSenderUID().High, pCommand->GetSenderUID().Low, timeGetTime()-nTimeStamp);
			}
			break;
		case MC_ZONESERVER_REQUEST_MAPLIST:
			ResponseMapList(pCommand->m_Sender);
			break;
		case MC_MESSAGE_SAY:
			{
				MUID uid;
				char szSay[256];
				pCommand->GetParameter(&uid, 0, MPT_UID);
				pCommand->GetParameter(szSay, 1, MPT_STR);
				int nErrCode = MessageSay(uid, szSay);
				if(nErrCode!=MOK) LOG(MGetErrorString(nErrCode));
				return true;
			}
			
		case MC_OBJECT_ADD:
			{
				int nObjID, nMapID, nNodeID;
				rvector Pos;
				pCommand->GetParameter(&nObjID, 0, MPT_INT);
				pCommand->GetParameter(&nMapID, 1, MPT_INT);
				pCommand->GetParameter(&nNodeID, 2, MPT_INT);
				pCommand->GetParameter(&Pos, 3, MPT_POS);
				MUID ObjUID;
				int nErrCode = ObjectAdd(&ObjUID, nObjID, nMapID, nNodeID, Pos);
				if(nErrCode!=MOK) LOG(MGetErrorString(nErrCode));
			}
			break;
		case MC_OBJECT_SET_CONTROL:
			{
				MUID uid;
				pCommand->GetParameter(&uid, 0, MPT_UID);

			}
			break;
		case MC_OBJECT_MOVEMODE:
			{
				MUID uid;
				int nMoveMode;
				pCommand->GetParameter(&uid, 0, MPT_UID);
				pCommand->GetParameter(&nMoveMode, 0, MPT_INT);
				ObjectMoveMode(uid, nMoveMode);
			}
			break;
		case MC_OBJECT_MODE:
			{
				MUID uid;
				int nMode;
				pCommand->GetParameter(&uid, 0, MPT_UID);
				pCommand->GetParameter(&nMode, 0, MPT_INT);
				ObjectMode(uid, nMode);
			}
			break;
		case MC_OBJECT_STATE:
			{
				MUID uid;
				int nState;
				pCommand->GetParameter(&uid, 0, MPT_UID);
				pCommand->GetParameter(&nState, 0, MPT_INT);
				ObjectState(uid, nState);
			}
			break;

			/*
		case MC_OBJECT_MOVE:
			{
				MUID uid;
				unsigned long int StartTime, EndTime;
				rvector StartPos, EndPos;
				pCommand->GetParameter(&uid, 0, MPT_UID);
				pCommand->GetParameter(&StartTime, 1, MPT_UINT);
				pCommand->GetParameter(&EndTime, 2, MPT_UINT);
				pCommand->GetParameter(&StartPos, 3, MPT_POS);
				pCommand->GetParameter(&EndPos, 4, MPT_POS);

				int nErrCode = ObjectMove(uid, StartTime, EndTime, StartPos, EndPos);
				if(nErrCode!=MOK) LOG(MGetErrorString(nErrCode));
			}
			break;
			*/
		case MC_OBJECT_MOVEARRAY:
			{
				MUID uid;
				pCommand->GetParameter(&uid, 0, MPT_UID);
				void* pPathArray = pCommand->GetParameter(1)->GetPointer();

				int nErrCode = ObjectMove(uid, pPathArray);
				if(nErrCode!=MOK) LOG(MGetErrorString(nErrCode));
			}
			break;
		case MC_OBJECT_ATTACK:
			{
				MUID uid, TargetUID;
				pCommand->GetParameter(&uid, 0, MPT_UID);
				pCommand->GetParameter(&TargetUID, 1, MPT_UID);

				int nErrCode = ObjectAttack(uid, TargetUID);
				if(nErrCode!=MOK) LOG(MGetErrorString(nErrCode));
			}
			break;
		case MC_OBJECT_STOPATTACK:
			{
				MUID uid;
				pCommand->GetParameter(&uid, 0, MPT_UID);
				MObjectCharacter* pActor = (MObjectCharacter*)GetObject(uid);
				if(!ISEQUALCLASS(MObjectCharacter, pActor)) 
					LOG(MGetErrorString(MERR_OBJECT_INVALID));
				else
					pActor->StopAttack();
			}
			break;
		case MC_OBJECT_SKILL:
			{
				MUID uid, uidTarget;
				int nSkillID;
				pCommand->GetParameter(&uid, 0, MPT_UID);
				pCommand->GetParameter(&uidTarget, 1, MPT_UID);
				pCommand->GetParameter(&nSkillID, 2, MPT_INT);
				ObjectSkill(uid, uidTarget, nSkillID);
			}
			break;
		case MC_MAP_REQUEST_OBJECTLIST:
			{
				int nMapID;
				int nNodeID;
				pCommand->GetParameter(&nMapID, 0, MPT_INT);
				pCommand->GetParameter(&nNodeID, 1, MPT_INT);
				ResponseObjectList(pCommand->m_Sender, nMapID, nNodeID);
			}
			break;
		case MC_MAP_REQUEST_OBJECTLIST2:
			{
				int nMapID;
				pCommand->GetParameter(&nMapID, 0, MPT_INT);
				ResponseObjectList(pCommand->m_Sender, nMapID);
			}
			break;
		case MC_OBJECT_REQUEST_BASICATTR:
			{
				MUID uid;
				pCommand->GetParameter(&uid, 0, MPT_UID);
				ResponseCharacterBasicAttr(pCommand->m_Sender, uid);
			}
			break;
		case MC_ADMIN_CREATE_ITEM:
			{
				MUID uid;
				unsigned int nItemIndex;
				pCommand->GetParameter(&uid, 0, MPT_UID);
				pCommand->GetParameter(&nItemIndex, 1, MPT_UINT);

				int nErrCode = AdminCreateItem(uid, nItemIndex);
				if(nErrCode!=MOK) LOG(MGetErrorString(nErrCode));
			}
			break;
		case MC_ADMIN_REQUEST_CHARACTER_INFO:
			{
				MUID uid;
				pCommand->GetParameter(&uid, 0, MPT_UID);
				ResponseAdminCharacterInfo(pCommand->m_Sender, uid);
			}
			break;
		case MC_OBJECT_REQUEST_EQUIP_ITEM:
			{
				MUID ChrUID, ItemUID;
				int nEquipmentSlot;

				pCommand->GetParameter(&ChrUID, 0, MPT_UID);
				pCommand->GetParameter(&ItemUID, 1, MPT_UID);
				pCommand->GetParameter(&nEquipmentSlot, 2, MPT_INT);
				ObjectEquipItem(pCommand->m_Sender, ChrUID, ItemUID, MEquipmentSlot(nEquipmentSlot));
			}
			break;
		case MC_OBJECT_REQUEST_SKILL:
			{
				MUID ChrUID;
				pCommand->GetParameter(&ChrUID, 0, MPT_UID);
				ResponseCharacterSkillList(pCommand->m_Sender, ChrUID);
			}
			break;
		default:
			_ASSERT(0);	// 아직 핸들러가 없다.
			return false;
	}
	return false;
}

void MZoneServer::OnRun(void)
{
	unsigned long int nGlobalClock = GetGlobalClockCount();
	for(MObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end();){
		MObject* pObj = (*i).second;
		pObj->Tick(nGlobalClock);

		if (pObj->CheckDestroy(nGlobalClock) == true) {
			ObjectRemove(pObj->GetUID(), &i);
		} else {
			i++;
		}
	}

	if(m_SpawnManager.IsNewTick(nGlobalClock))
		m_SpawnManager.Tick(nGlobalClock);
}

void MZoneServer::UpdateObjectRecognition(MObject* pObject)
{
	pObject->InvalidateObjectCache();
	for(MObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++){
		MObject* pScanObj = (MObject*)((*i).second);
		rvector Sub;
		D3DXVec3Subtract(&Sub, &pObject->GetPos(), &pScanObj->GetPos());
		if (D3DXVec3Length(&Sub) < 2000.0f)
			pObject->UpdateObjectCache(pScanObj);
	}
	if (pObject->HasCommListener()) {
		for (list<MUID>::iterator itorUID=pObject->m_CommListener.begin(); itorUID!=pObject->m_CommListener.end(); itorUID++) {
			MUID TargetUID = *itorUID;
			ResponseObjectUpdate(TargetUID, pObject);
		}
	}
	pObject->RemoveObjectCacheExpired();
}

void MZoneServer::ResponseObjectUpdate(MUID& TargetUID, MObject* pObject)
{
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_OBJECT_CACHE_UPDATE), TargetUID, m_This);
	int nObjectCount = pObject->GetObjectCacheUpdateCount();
	if (nObjectCount <= 0) return;

	void* pObjectDescs = MMakeBlobArray(sizeof(MOBJECTCACHEINFO), nObjectCount);
	int i = 0;
	for (MObjectCache::iterator itor=pObject->m_ObjectCache.begin(); itor!=pObject->m_ObjectCache.end(); itor++) {
		MObjectCacheNode* pNode = *itor;
		if ((pNode->m_CacheState == OBJECTCACHESTATE_NEW) || 
			(pNode->m_CacheState == OBJECTCACHESTATE_EXPIRE) ) {
			// Make Response
			MOBJECTCACHEINFO* pObjDesc = (MOBJECTCACHEINFO*)MGetBlobArrayElement(pObjectDescs, i++);
			MObject* pObj = pNode->m_pObject;
			pObjDesc->uid = pObj->GetUID();
			pObjDesc->x = pObj->GetPos().x;
			pObjDesc->y = pObj->GetPos().y;
			pObjDesc->z = pObj->GetPos().z;
			pObjDesc->nObjectType = pObj->GetObjectType();
			pObjDesc->nState = pNode->m_CacheState;
		}			
	}
	pNew->AddParameter(new MCommandParameterBlob(pObjectDescs, MGetBlobArraySize(pObjectDescs)));
	MEraseBlobArray(pObjectDescs);
	Post(pNew);

/*	char Buf[1024];
	int nSize = pNew->GetData(Buf, 1024);

	MCommand* pDebug = new MCommand();
	pDebug->SetData(Buf, &m_CommandManager);*/
}

void MZoneServer::ResponseInfo(MUID& Target)
{
	char szTemp[1024];
	sprintf(szTemp, "MAIET Zone-Server\n UID : %u:%u\n UID Remained Count : %u:%u ~ %u:%u", m_This.High, m_This.Low, m_CommAllocRange.Start.High, m_CommAllocRange.Start.Low, m_CommAllocRange.End.High, m_CommAllocRange.End.Low);

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_RESPONSE_INFO), Target, m_This);
	pNew->AddParameter(new MCommandParameterString(szTemp));
	Post(pNew);
}

void MZoneServer::ResponseMapList(MUID& Target)
{
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_ZONESERVER_RESPONSE_MAPLIST), Target, m_This);

	int nMapCount = (int)m_Maps.size();
	void* pMapDescs = MMakeBlobArray(sizeof(MMAPDESCRIPTION), nMapCount);
	for(int i=0; i<nMapCount; i++){
		MMAPDESCRIPTION* pMapDesc = (MMAPDESCRIPTION*)MGetBlobArrayElement(pMapDescs, i);
		strcpy(pMapDesc->szName, m_Maps[i]->GetName());
	}
	pNew->AddParameter(new MCommandParameterBlob(pMapDescs, MGetBlobArraySize(pMapDescs)));
	MEraseBlobArray(pMapDescs);

	Post(pNew);
}

inline void MZoneServer::RouteToListener(MObject* pObject, MCommand* pCommand)
{
	size_t nListenerCount = pObject->m_CommListener.size();
	if (nListenerCount <= 0) {
		delete pCommand;
		return;
	} else if (nListenerCount == 1) {
		MUID TargetUID = *pObject->m_CommListener.begin();
		pCommand->m_Receiver = TargetUID;
		Post(pCommand);
	} else {
		int nCount = 0;
		for (list<MUID>::iterator itorUID=pObject->m_CommListener.begin(); itorUID!=pObject->m_CommListener.end(); itorUID++) {
			MUID TargetUID = *itorUID;

			MCommand* pSendCmd;
			if (nCount<=0)
				pSendCmd = pCommand;
			else
				pSendCmd = pCommand->Clone();
			pSendCmd->m_Receiver = TargetUID;
			Post(pSendCmd);
			nCount++;
		}
	}
}

void MZoneServer::RouteToAllClient(MCommand* pCommand)
{
	int nCount = 0;
	for(MUIDRefCache::iterator i=m_CommRefCache.begin(); i!=m_CommRefCache.end(); i++){
		MCommObject* pCommObj = (MCommObject*)((*i).second);
		if (pCommObj->GetUID() < MUID(0,3)) continue;	// MUID로 Client인지 판별할수 있는 코드 필요함
		
		MCommand* pSendCmd;
		if (nCount<=0)
			pSendCmd = pCommand;
		else
			pSendCmd = pCommand->Clone();
		pSendCmd->m_Receiver = pCommObj->GetUID();
		Post(pSendCmd);
		nCount++;
	}	
	if (nCount == 0) delete pCommand;
}

void MZoneServer::RouteToAllExcludeSender(MUID& uidSender, MCommand* pCommand)
{
	int nCount = 0;
	for(MUIDRefCache::iterator i=m_CommRefCache.begin(); i!=m_CommRefCache.end(); i++){
		MCommObject* pCommObj = (MCommObject*)((*i).second);
		if (pCommObj->GetUID() == uidSender) continue;
		
		MCommand* pSendCmd;
		if (nCount<=0)
			pSendCmd = pCommand;
		else
			pSendCmd = pCommand->Clone();
		pSendCmd->m_Receiver = pCommObj->GetUID();
		Post(pSendCmd);
		nCount++;
	}	
	if (nCount == 0) delete pCommand;
}

void MZoneServer::RouteToRegion(int nMapID, const rvector& Pos, float fRange, MCommand* pCommand)
{
	if(nMapID>=(int)m_Maps.size()){
		LOG("ObjectAdd : Unknown map index %d", nMapID);
		return;
	}
	
	list<MObject*> ObjList;

	MMap* pMap = m_Maps[nMapID];
	pMap->Get(&ObjList, Pos.x, Pos.y, Pos.z, fRange);

	int nCount = 0;
	for (list<MObject*>::iterator i=ObjList.begin(); i!=ObjList.end(); i++) {
		MObject* pObj = *i;
		if (pObj->HasCommListener()) {
			MCommand* pSendCmd;
			if (nCount<=0)
				pSendCmd = pCommand;
			else
				pSendCmd = pCommand->Clone();
			RouteToListener(pObj, pSendCmd);
			nCount++;
		}
	}
	if (nCount == 0) delete pCommand;
}

void MZoneServer::RequestUIDToMaster(int nSize)
{
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_REQUEST_UID), m_Master, m_This);
	pNew->AddParameter(new MCommandParameterInt(nSize));
	Post(pNew);

	Log("Request UID-space reservation to Master-Controller");
}

MObject* MZoneServer::LoadCharacter(MUID& uid)
{
	// Load Data from DB

	MObjectCharacter* pObj = new MObjectCharacter(uid, m_Maps[0], rvector(0, 0, 0));

	m_Objects.insert(MObjectList::value_type(uid, pObj));

	return pObj;
}

void MZoneServer::UnloadCharacter(MUID& uid)
{
	// Save Data to DB

	MObjectList::iterator i = m_Objects.find(uid);
	if(i==m_Objects.end()) return;

	MObject* pObj = (*i).second;
	delete (*i).second;

	m_Objects.erase(i);
}

void MZoneServer::BuildActionPipe(MObjectCharacter* pCharacter)
{
	MActionAttack* pActAttack = new MActionAttack;
	MActionDefense* pActDefense = new MActionDefense;
	MActionSkill* pActSkill = new MActionSkill;
	pCharacter->AddAction(pActAttack);
	pCharacter->AddAction(pActDefense);
	pCharacter->AddAction(pActSkill);

	MActivePipe* pActivePipe = new MActivePipe();
	pActivePipe->AddAction(pActAttack);
	pCharacter->AddActionPipe(pActivePipe);

	MDefensePipe* pDefensePipe = new MDefensePipe();
	pDefensePipe->AddAction(pActDefense);
	pCharacter->AddActionPipe(pDefensePipe);

	MSkillPipe* pSkillPipe = new MSkillPipe();

	pCharacter->AddActionPipe(pSkillPipe);
}

int MZoneServer::MessageSay(MUID& uid, char* pszSay)
{
	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MESSAGE_SAY), MUID(0,0), m_This);
	pCmd->AddParameter(new MCommandParameterUID(uid));
	pCmd->AddParameter(new MCommandParameterString(pszSay));

	MObjectList::iterator i = m_Objects.find(uid);
	if(i==m_Objects.end()) return MERR_OBJECT_INVALID;

	MObject* pObj = (*i).second;
	//_ASSERT(pObj->m_pMap!=NULL);

	// MMap->GetID() 필요
	int nMapID = pObj->GetMap()->GetID();

	RouteToRegion(nMapID, pObj->GetPos(), 2000.0f, pCmd);

	LOG("Message Say (UID:%d%d) : %s", uid.High, uid.Low, pszSay);

	return MOK;
}

int MZoneServer::ObjectAdd(MUID* pAllocUID, int nObjID, int nMapID, int nNodeID, rvector& Pos)
{
	if(nMapID>=(int)m_Maps.size()){
		LOG("ObjectAdd : Unknown map index %d", nMapID);
		return MERR_UNKNOWN;
	}
	if(nObjID!=0){
		LOG("ObjectAdd : Unknown object ID(%d). Only object ID 0 is prepared.", nObjID);
		return MERR_UNKNOWN;
	}
	MMap* pMap = m_Maps[nMapID];

	MObjectCharacter* pChar = new MObjectCharacter(UseUID(), pMap, Pos);
	BuildActionPipe(pChar);	

	m_Objects.insert(MObjectList::value_type(pChar->GetUID(), pChar));
	*pAllocUID = pChar->GetUID();

	LOG("Character Added (UID:%d%d)", pChar->GetUID().High, pChar->GetUID().Low);

	return MOK;
}

int MZoneServer::ObjectRemove(const MUID& uid, MObjectList::iterator* pNextItor)
{
	MObjectList::iterator i = m_Objects.find(uid);
	if(i==m_Objects.end()) return MERR_OBJECT_INVALID;

	MObject* pObj = (*i).second;
	if (pObj->GetSpawnID())
		m_SpawnManager.DeSpawn(pObj->GetSpawnID());

	delete pObj;

	*pNextItor = m_Objects.erase(i);
	return MOK;
}

int MZoneServer::ObjectMoveMode(const MUID& uid, int nMoveMode)
{
	MObjectCharacter* pChar = FindCharacter(uid);
	if (pChar == NULL) return MERR_OBJECT_INVALID;
	pChar->SetMoveMode((MCharacterMoveMode)nMoveMode);
	return MOK;
}

int MZoneServer::ObjectMode(const MUID& uid, int nMode)
{
	MObjectCharacter* pChar = FindCharacter(uid);
	if (pChar == NULL) return MERR_OBJECT_INVALID;
	pChar->SetMode((MCharacterMode)nMode);
	return MOK;
}

int MZoneServer::ObjectState(const MUID& uid, int nState)
{
	MObjectCharacter* pChar = FindCharacter(uid);
	if (pChar == NULL) return MERR_OBJECT_INVALID;
	pChar->SetState((MCharacterState)nState);
	return MOK;
}

/*
int MZoneServer::ObjectMove(MUID& uid, unsigned long int nStartTime, unsigned long int nEndTime, rvector& StartPos, rvector& EndPos)
{
	MObjectCharacter* pObj = GetObjectCharacter(uid);
	if (pObj == NULL) return MERR_OBJECT_INVALID;

	if ( (pObj->GetMode() == MCM_DEAD) || (pObj->GetMode() == MCM_SIT) )
		return MERR_OBJECT_INVALID_MODE;

	// 유효성 검사후 이동


	pObj->SetPos(nStartTime, nEndTime, StartPos, EndPos);

	MCommand* pRouteCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_OBJECT_MOVE), MUID(0,0), m_This);
	pRouteCmd->AddParameter(new MCommandParameterUID(uid));
	pRouteCmd->AddParameter(new MCommandParameterUInt(nStartTime));
	pRouteCmd->AddParameter(new MCommandParameterUInt(nEndTime));
	pRouteCmd->AddParameter(new MCommandParameterPos(StartPos.x, StartPos.y, StartPos.z));
	pRouteCmd->AddParameter(new MCommandParameterPos(EndPos.x, EndPos.y, EndPos.z));

	float fSharedVisionRadius = m_fSharedVisionRadius;
	rvector Diff = EndPos - StartPos;
	float fDiff = D3DXVec3Length(&Diff);
	//_ASSERT(fDiff<fSharedVisionRadius);
	if(fDiff>fSharedVisionRadius) fSharedVisionRadius  = fDiff;
	// 지금은 m_fSharedVisionRadius 이상의 이동이면 그만큼 추가해서 알려주지만,
	// 패스의 최대 길이 제한을 두어 Move가 생략되는 오브젝트가 없도록 해줘야 한다.

	RouteToRegion(0, StartPos, fSharedVisionRadius, pRouteCmd);
	//RouteToAllClient(pRouteCmd);

	LOG("Object Move (UID:%d%d, Pos:%.2f,%.2f,%.2f)", uid.High, uid.Low, EndPos.x, EndPos.y, EndPos.z);

	return MOK;
}
*/

// 직선 경로 검증: 시작점과 끝점의 거리가 움직일 수 있는 거리인가?
bool ValidateStaightPathLength(rvector* pRecommendedPos, rvector& Start, rvector& End, unsigned long int nTimeInterval)
{
	return true;
}

/*
// 직선 경로 검증
bool ValidateStraightPath(rvector* pRollBackPos, RPathNode** ppRollBackNode, rvector& Pos, rvector& NextPos, RPathNode* pNode, RPathNode* pNextNode, unsigned long int nTimeInterval, RPathList* pPathList)
{
	// 디폴트는 시작지점으로 롤백
	*pRollBackPos = Pos;
	*ppRollBackNode = pNode;
*/
	// 시작점이 시작노드에 속해있는가?
	// 이전 단계에서 테스트해보았을것으로 간주, 스킵한다.
	/*
	if(IsPosInNode(pRecommendedPos, Pos, pNode)==false){
		return false;
	}
	*/

	/*
	// 끝점이 끝노드에 속해있는가?
	if(IsPosInNode(NextPos, pNextNode)==false){
		return false;
	}

	// 시작 노드에서 끝노드가 연결되어 있는가?
	int nNodeNeighborhoodID = IsNodeConnection(pNode, pNextNode, pPathList);
	if(nNodeNeighborhoodID==-1){
		return false;
	}

	// 시작점과 끝점의 거리가 움직일 수 있는 거리인가?
	if(ValidateStaightPathLength(pRollBackPos, Pos, NextPos, nTimeInterval)==false){
		return false;
	}

	// 포탈을 통과하는가?
	if(IsPathAcrossPortal(Pos, NextPos, pNode, pNextNode, nNodeNeighborhoodID, pPathList)==false){
		return false;
	}

	// 시작점과 끝점의 연결선이 시작노드와 끝노드안에서 벽과 인터섹션이 발생하지 않는가?
	if(IsPathInNode(Pos, NextPos, pNode, pNextNode, nNodeNeighborhoodID, -1, pPathList)==false){
		return false;
	}
	return true;
}
*/

int MZoneServer::ObjectMove(MUID& uid, void* pPathArray)
{
	MObjectCharacter* pObj = GetObjectCharacter(uid);
	if (pObj == NULL) return MERR_OBJECT_INVALID;

	if ( (pObj->GetState() == MCS_DEAD) || (pObj->GetState() == MCS_SIT) )
		return MERR_OBJECT_INVALID_MODE;


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// 유효성 검사후 이동
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//ValidateStraightPath();

	// 위치에 대한 유효성 검사



	// 시간 대비 거리에 대한 유효성 검사



	//pObj->SetPos(nStartTime, nEndTime, StartPos, EndPos);
	pObj->SetPath((MOBJECTPATH*)MGetBlobArrayPointer(pPathArray), MGetBlobArrayCount(pPathArray));

	MCommand* pRouteCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_OBJECT_MOVEARRAY), MUID(0,0), m_This);
	pRouteCmd->AddParameter(new MCommandParameterUID(uid));
	pRouteCmd->AddParameter(new MCommandParameterBlob(pPathArray, MGetBlobArraySize(pPathArray)));

	rvector StartPos;
	float fSharedVisionRadius = m_fSharedVisionRadius;

	// 패스가 설정된 전 범위 계산. m_fSharedVisionRadius보다 크면 새로운 범위로 재설정.
	int pPathCount = MGetBlobArrayCount(pPathArray);
	_ASSERT(pPathCount>0);
	for(int i=0; i<pPathCount; i++){
		MOBJECTPATH* pObjectMove = (MOBJECTPATH*)MGetBlobArrayElement(pPathArray, i);
		if(i==0) StartPos = pObjectMove->Pos;
		else{
			rvector Diff = pObjectMove->Pos - StartPos;
			float fDiff = D3DXVec3Length(&Diff);

			//_ASSERT(fDiff<fSharedVisionRadius);
			if(fDiff>fSharedVisionRadius) fSharedVisionRadius  = fDiff;
		}
	}

	// 지금은 m_fSharedVisionRadius 이상의 이동이면 그만큼 추가해서 알려주지만,
	// 패스의 최대 길이 제한을 두어 Move가 생략되는 오브젝트가 없도록 해줘야 한다.
	RouteToRegion(0, StartPos, fSharedVisionRadius, pRouteCmd);
	//RouteToAllClient(pRouteCmd);

	LOG("Object Move (UID:%d%d, Path Array)", uid.High, uid.Low);

	return MOK;
}

int MZoneServer::ObjectAttack(MUID& uid, MUID& uidTarget)
{
	MObjectCharacter* pActor = GetObjectCharacter(uid);
	if (pActor == NULL) return MERR_OBJECT_INVALID;
	if(!ISEQUALCLASS(MObjectCharacter, pActor)) return MERR_OBJECT_INVALID;

	if ( (pActor->GetState() == MCS_DEAD) || (pActor->GetState() == MCS_SIT) )
		return MERR_OBJECT_INVALID_MODE;

	MObjectCharacter* pTarget = GetObjectCharacter(uidTarget);
	if(pTarget == NULL) return MERR_OBJECT_INVALID;
	if(!ISEQUALCLASS(MObjectCharacter, pTarget)) return MERR_OBJECT_INVALID;

	if (pActor->Attack(pTarget) == false) return MERR_OBJECT_INVALID;
	return MOK;
}

int MZoneServer::ObjectSkill(const MUID& uid, const MUID& uidTarget, int nSkillID)
{
	MObjectCharacter* pActor = GetObjectCharacter(uid);
	if (pActor == NULL) return MERR_OBJECT_INVALID;
	if(!ISEQUALCLASS(MObjectCharacter, pActor)) return MERR_OBJECT_INVALID;

	if ( (pActor->GetState() == MCS_DEAD) || (pActor->GetState() == MCS_SIT) )
		return MERR_OBJECT_INVALID_MODE;

	MObjectCharacter* pTarget = GetObjectCharacter(uidTarget);
	if(pTarget == NULL) return MERR_OBJECT_INVALID;
	if(!ISEQUALCLASS(MObjectCharacter, pTarget)) return MERR_OBJECT_INVALID;

	if (pActor->Skill(nSkillID, pTarget) == false) return MERR_OBJECT_INVALID;
	return MOK;
}

void MZoneServer::ResponseObjectList(MUID& Target, int nMapID, int nNodeID)
{
	char szTemp[1024];
	sprintf(szTemp, "MAIET Zone-Server\n UID : %u:%u\n UID Remained Count : %u:%u ~ %u:%u", m_This.High, m_This.Low, m_CommAllocRange.Start.High, m_CommAllocRange.Start.Low, m_CommAllocRange.End.High, m_CommAllocRange.End.Low);

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MAP_RESPONSE_OBJECTLIST), Target, m_This);
	pNew->AddParameter(new MCmdParamInt(nMapID));
	pNew->AddParameter(new MCmdParamInt(nNodeID));

	MMap::MRefCell* pRefCell = m_Maps[nMapID]->GetCell(nNodeID);
	int nObjectCount = (int)pRefCell->size();
	void* pObjectDescs = MMakeBlobArray(sizeof(MOBJECTBASICINFO), nObjectCount);
	int i = 0;
	for(MMap::MRefCell::iterator it=pRefCell->begin(); it!=pRefCell->end(); it++){
		MOBJECTBASICINFO* pObjDesc = (MOBJECTBASICINFO*)MGetBlobArrayElement(pObjectDescs, i);
		MObject* pObj = (*it).Obj;
		pObjDesc->uid = pObj->GetUID();
		pObjDesc->x = pObj->GetPos().x;
		pObjDesc->y = pObj->GetPos().y;
		pObjDesc->z = pObj->GetPos().z;

		i++;
	}
	pNew->AddParameter(new MCommandParameterBlob(pObjectDescs, MGetBlobArraySize(pObjectDescs)));
	MEraseBlobArray(pObjectDescs);

	Post(pNew);
}

void MZoneServer::ResponseObjectList(MUID& Target, int nMapID)
{
	char szTemp[1024];
	sprintf(szTemp, "MAIET Zone-Server\n UID : %u:%u\n UID Remained Count : %u:%u ~ %u:%u", m_This.High, m_This.Low, m_CommAllocRange.Start.High, m_CommAllocRange.Start.Low, m_CommAllocRange.End.High, m_CommAllocRange.End.Low);

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MAP_RESPONSE_OBJECTLIST2), Target, m_This);
	pNew->AddParameter(new MCmdParamInt(nMapID));

	int nObjectCount = (int)m_Objects.size();
	void* pObjectDescs = MMakeBlobArray(sizeof(MOBJECTBASICINFO), nObjectCount);
	int i = 0;
	for(MObjectList::iterator it=m_Objects.begin(); it!=m_Objects.end(); it++){
		MOBJECTBASICINFO* pObjDesc = (MOBJECTBASICINFO*)MGetBlobArrayElement(pObjectDescs, i);
		MObject* pObj = (*it).second;
		pObjDesc->uid = pObj->GetUID();
		pObjDesc->x = pObj->GetPos().x;
		pObjDesc->y = pObj->GetPos().y;
		pObjDesc->z = pObj->GetPos().z;

		i++;
	}
	pNew->AddParameter(new MCommandParameterBlob(pObjectDescs, MGetBlobArraySize(pObjectDescs)));
	MEraseBlobArray(pObjectDescs);

	Post(pNew);
}

void MZoneServer::ResponseCharacterSkillList(const MUID& CommUID, const MUID& CharUID)
{
	MObjectCharacter* pChar = GetObjectCharacter(CharUID);
	if (pChar == NULL) return;

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_OBJECT_REQUEST_SKILL), CommUID, m_This);
	pNew->AddParameter(new MCommandParameterUID(CharUID));
	
	int index = 0;
	void* pSkillArray = MMakeBlobArray(sizeof(int), pChar->GetActionCount());
	for (list<MAction*>::iterator i=pChar->GetActionBeginItor(); i!=pChar->GetActionEndItor(); i++) {
		MAction* pAct = *i;
		int* pID = (int*)MGetBlobArrayElement(pSkillArray, index++);
		*pID = pAct->GetID();
	}
	pNew->AddParameter(new MCommandParameterBlob(pSkillArray, MGetBlobArraySize(pSkillArray)));
	MEraseBlobArray(pSkillArray);

	Post(pNew);
}

unsigned long int MZoneServer::GetGlobalClockCount(void)
{
	unsigned long int i = timeGetTime();
	return i;
}

MObject* MZoneServer::GetObject(const MUID& uid)
{
	MObjectList::iterator i = m_Objects.find(uid);
	if(i==m_Objects.end()) return NULL;
	return (*i).second;
}

MObjectCharacter* MZoneServer::GetObjectCharacter(const MUID& uid)
{
	MObject* pObj = GetObject(uid);
	if(pObj==NULL) return NULL;
	if(ISEQUALCLASS(MObjectCharacter, pObj)) return (MObjectCharacter*)pObj;
	return NULL;
}

MObjectItem* MZoneServer::GetObjectItem(const MUID& uid)
{
	MObject* pObj = GetObject(uid);
	if(pObj==NULL) return NULL;
	if(ISEQUALCLASS(MObjectItem, pObj)) return (MObjectItem*)pObj;
	return NULL;
}

MObjectCharacter* MZoneServer::GetPlayerByCommUID(const MUID& uid)
{
	for(MObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++){
		MObject* pObj = (MObject*)((*i).second);
		for (list<MUID>::iterator j=pObj->m_CommListener.begin(); j!=pObj->m_CommListener.end(); j++){
			MUID TargetUID = *j;
			if (TargetUID == uid)
				return (MObjectCharacter*)pObj;
		}
	}
	return NULL;
}

MUID MZoneServer::UseUID(void)
{
	if(m_CommAllocRange.IsEmpty()==true){
		if(m_NextCommAllocRange.IsEmpty()==true){
			return MUID::Invalid();
		}
		else{
			m_CommAllocRange = m_NextCommAllocRange;
			m_NextCommAllocRange.Empty();
		}
	}

	MUID ret = m_CommAllocRange.Start;
	m_CommAllocRange.Start.Increase();
	return ret;
}

void MZoneServer::OnInitialize()
{
	m_SpawnManager.ReadSpawnPosition("map.xml");
	m_SpawnManager.ReadSpawnData("spawn.xml");
	g_CharacterDescManager.Read(FILENAME_CHARACTER_DESC);
	g_SkillDescManager.Read(FILENAME_SKILL_DESC);
	g_ItemCatalog.Read(FILENAME_ITEM_DESC);
	Log("Initilization completed");
}

// Login절차가 완성되면 Player고유의 PlayerUID사용. 임시로 생성된 AllocUID사용중
void MZoneServer::OnLogin(MUID CommUID, MUID PlayerUID)
{
	// 초기 위치의 노드는 검색해서 얻어낸다.
	int nMapID = 0;
	MMap* pMap = m_Maps[nMapID];
	RBspObject* pBspObj = pMap->GetBspObject();

	rvector Pos(0,0,0);
	rvector PosTo = Pos;
	PosTo.z -= 0.1f;

	int nNodeID = 0;
	/*
	RPathNode* pPathNode;
	rvector ColPos;
	int nNodeID = 0;
	if (pBspObj->PickPathNode(Pos, PosTo, &pPathNode, &ColPos)==false){
		LOG("OnLogin : Path node not found on Pos(%f %f %f) in map index %d", Pos.x, Pos.y, Pos.z, nMapID);
	}
	else{
		nNodeID = pPathNode->m_nIndex;
	}
	*/

	MUID AllocUID;
	int nErrCode = ObjectAdd(&AllocUID, 0, 0, nNodeID, rvector(0,0,0));
	if(nErrCode!=MOK) LOG(MGetErrorString(nErrCode));

	MObject* pObj = GetObject(AllocUID);
	pObj->SetObjectType(MOT_PC);

	SetClientClockSynchronize(CommUID);
	AssignPlayerCharacter(CommUID, AllocUID);
}

void MZoneServer::SetClientClockSynchronize(MUID& CommUID)
{
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_CLOCK_SYNCHRONIZE), CommUID, m_This);
	pNew->AddParameter(new MCommandParameterUInt(GetGlobalClockCount()));
	Post(pNew);
}

void MZoneServer::AssignPlayerCharacter(MUID& CommUID, MUID& PlayerUID)
{
	MObjectList::iterator itor = m_Objects.find(PlayerUID);
	if(itor==m_Objects.end()) return;
	MObject* pObj = (*itor).second;
	
	// Object에 CommListener 등록
	pObj->AddCommListener(CommUID);

	ResponsePlayerCharacterAssign(CommUID, pObj);
}

void MZoneServer::ResponsePlayerCharacterAssign(MUID& CommUID, MObject* pObject)
{
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_OBJECT_ASSIGN_PLAYER), CommUID, m_This);
	pNew->AddParameter(new MCommandParameterUID(pObject->GetUID()));
	pNew->AddParameter(new MCommandParameterPos(pObject->GetPos().x, pObject->GetPos().y, pObject->GetPos().z));
	Post(pNew);
}

void MZoneServer::ResponseCharacterBasicAttr(MUID& TargetUID, MUID& RequestUID)
{
	MObjectList::iterator itor = m_Objects.find(RequestUID);
	if (itor == m_Objects.end()) return;

	MObject* pObj = (*itor).second;
	if (!ISEQUALCLASS(MObjectCharacter, pObj))  return;

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_OBJECT_RESPONSE_BASICATTR), TargetUID, m_This);

	void* pAttrDescs = MMakeBlobArray(sizeof(MCharacterAttrBasicInfo), 1);
	MCharacterAttrBasicInfo* pAttrDesc = (MCharacterAttrBasicInfo*)MGetBlobArrayElement(pAttrDescs, 0);

	pAttrDesc->uid = RequestUID;
	pAttrDesc->nHP = ((MObjectCharacter*)pObj)->GetHP();
	pAttrDesc->nEN = ((MObjectCharacter*)pObj)->GetEN();

/*
	MCharacterAttrBasicInfo		chrAttrBasicInfo;
	chrAttrBasicInfo.uid = RequestUID;
	chrAttrBasicInfo.nHP = ((MObjectCharacter*)pObj)->GetHP();
	chrAttrBasicInfo.nEN = ((MObjectCharacter*)pObj)->GetEN();
*/
	pNew->AddParameter(new MCommandParameterBlob(pAttrDescs, MGetBlobArraySize(pAttrDescs)));
	MEraseBlobArray(pAttrDescs);

//	pNew->AddParameter(new MCommandParameterBlob(&chrAttrBasicInfo, sizeof(chrAttrBasicInfo)));
	Post(pNew);	
}



void MZoneServer::ReleasePlayerCharacter(MUID& uid)
{
	for(MObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++){
		MObject* pObj = (MObject*)((*i).second);
		for (list<MUID>::iterator j=pObj->m_CommListener.begin(); j!=pObj->m_CommListener.end(); j++){
			MUID TargetUID = *j;
			if (TargetUID == uid) {
				delete pObj;
				m_Objects.erase(i);
				return;
			}
		}
	}
}

MZoneServer::MZoneServer(void)
{
	_ASSERT(m_pInstance==NULL);
	m_pInstance = this;

	m_Master.SetInvalid();
	m_bRequestUIDToMaster = false;
	m_fSharedVisionRadius = 2000;

	SetName("ZONESERVER");	// For Debug

	m_CommAllocRange.SetZero();
	m_NextCommAllocRange.SetZero();

	m_SpawnManager.SetZoneServer(this);
}

MZoneServer::~MZoneServer(void)
{
	Destroy();
}

MZoneServer* MZoneServer::GetInstance(void)
{
	return m_pInstance;
}

bool MZoneServer::Create(int nPort)
{
	m_CommAllocRange.SetZero();
	m_NextCommAllocRange.SetZero();

	if(MServer::Create(nPort)==false) return false;

	if(OnCreate()==false){
		MServer::Destroy();
		return false;
	}

	/*
	LoadCharacter(MUID(0, 1000000));
	LoadCharacter(MUID(0, 1000001));
	LoadCharacter(MUID(0, 1000002));
	*/
	SetupRCPLog(RcpLog);
	m_RealCPNet.SetLogLevel(1);

	LOG("Zone Server Created (Port:%d)", nPort);

	return true;
}

void MZoneServer::Destroy(void)
{
	OnDestroy();

	MServer::Destroy();

	m_CommAllocRange.SetZero();
	m_NextCommAllocRange.SetZero();
}

int MZoneServer::OnAccept(MCommObject* pCommObj)
{
	int nErrCode = MServer::OnAccept(pCommObj);
	if (nErrCode != MOK) return nErrCode;

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_LOCAL_LOGIN), m_This, m_This);
	pNew->AddParameter(new MCommandParameterUID(pCommObj->GetUID()));
	pNew->AddParameter(new MCommandParameterUID(MUID(0,0)));
	Post(pNew);

	return MOK;
}

int MZoneServer::OnConnected(MUID* pTargetUID, MUID* pAllocUID, MCommObject* pCommObj)
{
	int nRetCode = MServer::OnConnected(pTargetUID, pAllocUID, pCommObj);
	if (nRetCode != MOK) return nRetCode;

	if (*pTargetUID == MASTER_UID) {
		m_Master = *pTargetUID;
		m_This = *pAllocUID;
		Log("Master-Controller connected");
	}

	// 마스터와 연결되지 않았다면 아무일도 하지 않는다.
	if(m_Master.IsInvalid()) return MERR_ZONESERVER_NOT_CONNECTED_TO_MASTER;

	// UID 할당 요청
	if(m_CommAllocRange.Start<=m_CommAllocRange.End){
		if(m_bRequestUIDToMaster==false){
			RequestUIDToMaster(DEFAULT_REQUEST_UID_SIZE);
			m_bRequestUIDToMaster = true;
		}
	}
	else{
		m_bRequestUIDToMaster = false;
	}

	return MOK;
}

int MZoneServer::OnDisconnect(MCommObject* pCommObj)
{
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_CLEAR), m_This, m_This);
	pNew->AddParameter(new MCommandParameterUID(pCommObj->GetUID()));
	Post(pNew);

	return MOK;
}

int MZoneServer::ConnectToMaster(MCommObject* pCommObj)
{
	m_Master = pCommObj->GetUID();
	int nErrCode = Connect(pCommObj);
	if(nErrCode!=MOK) return nErrCode;
	
	return MOK;
}

void MZoneServer::DisconnectToMaster(void)
{
	Disconnect(m_Master);
}


unsigned long int ConvertLocalClockToGlobalClock(unsigned long int nLocalClock, unsigned long int nLocalClockDistance)
{
	return (nLocalClock + nLocalClockDistance);
}

unsigned long int ConvertGlobalClockToLocalClock(unsigned long int nGlobalClock, unsigned long int nLocalClockDistance)
{
	return (nGlobalClock - nLocalClockDistance);
}

//int MZoneServer::ObjectAdd(MUID* pAllocUID, int nObjID, int nMapID, int nNodeID, rvector& Pos)
int MZoneServer::SpawnObject(int nSpawnID, int nObjID, int nMapID, int nNodeID, rvector& Pos)
{
	if(nMapID>=(int)m_Maps.size()){
		LOG("SpawnObject : Unknown map index %d", nMapID);
		return MERR_UNKNOWN;
	}

	MMap* pMap = m_Maps[nMapID];
	RBspObject* pBspObj = pMap->GetBspObject();

	rvector PosTo = Pos; 
	PosTo.z -= 0.1f;

	/*
	RPathNode* pPathNode;
	rvector ColPos;
	if (pBspObj->PickPathNode(Pos, PosTo, &pPathNode, &ColPos) == false) {
		LOG("SpawnObject : Path node not found on Pos(%f %f %f) in map index %d", Pos.x, Pos.y, Pos.z, nMapID);
		return MERR_UNKNOWN;
	}

	MUID uid;
	int nErrCode = ObjectAdd(&uid, nObjID, nMapID, pPathNode->m_nIndex, Pos);
	if (nErrCode != MOK)
		return nErrCode;
		*/

	// 땜질.
	MUID uid;
	int nErrCode = ObjectAdd(&uid, nObjID, nMapID, 0 , Pos);
	if (nErrCode != MOK)
		return nErrCode;

	MObject* pObj = GetObject(uid);
	if (pObj == NULL)
		return MERR_OBJECT_INVALID;

	pObj->SetSpawnID(nSpawnID);
	
	pObj->SetObjectType(MOT_NPC);	// NPC

	return MOK;
}

void MZoneServer::DebugTest()
{
///////////
	LOG("DebugTest: Object List");
	for(MObjectList::iterator it=m_Objects.begin(); it!=m_Objects.end(); it++){
		MObject* pObj = (*it).second;
		LOG("DebugTest: Obj(%d%d) Pos(%f %f %f)", pObj->GetUID().High, pObj->GetUID().Low, pObj->GetPos().x, pObj->GetPos().y, pObj->GetPos().z);
	}
///////////
}

MObjectItem* MZoneServer::CreateObjectItem(int nItemDescID)
{
	MMap* pMap = m_Maps[0];
	MItemDesc* pItemDesc = g_ItemCatalog.Find(nItemDescID);

	if (pItemDesc != NULL)
	{
		MObject* pObjectItem = new MObjectItem(UseUID(), pMap, rvector(0,0,0));
		((MObjectItem*)pObjectItem)->SetDesc(pItemDesc);
		m_Objects.insert(MObjectList::value_type(pObjectItem->GetUID(), pObjectItem));

		return (MObjectItem*)pObjectItem;
	}
	else
	{
		LOG("CreateObjectItem : Unknown ItemDesc ID(%d).", nItemDescID);
		return NULL;
	}
}

int MZoneServer::AdminCreateItem(MUID uid, unsigned int nItemIndex)
{
	// 나중에 관리자 uid를 체크해야 한다.

	MObjectCharacter* pObjectCharacter = FindCharacter(uid);
	if (pObjectCharacter == NULL) return MERR_OBJECT_INVALID;

	MObjectItem* pObjectItem = CreateObjectItem(nItemIndex);

	if (pObjectItem!=NULL)
	{
		pObjectCharacter->ObtainItem(pObjectItem);
	}
	else { return MERR_OBJECT_INVALID; 	}


	return MOK;
}

void MZoneServer::ResponseAdminCharacterInfo(MUID& TargetUID, MUID& RequestUID)
{
	struct CHRINFO
	{
		MCharacterBasicAttr		BasicAttr;
		int						nEquipmentItem[MES_END];
	};

	MObjectList::iterator itor = m_Objects.find(RequestUID);
	if (itor == m_Objects.end()) return;

	MObject* pObj = (*itor).second;
	if (!ISEQUALCLASS(MObjectCharacter, pObj))  return;

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_ADMIN_RESPONSE_CHARACTER_INFO), 
								TargetUID, m_This);
	pNew->AddParameter(new MCommandParameterUID(RequestUID));

	CHRINFO info;
	memset(&info, 0, sizeof(info));
	
	MObjectCharacter* pObjCharacter = (MObjectCharacter*)pObj;

	info.BasicAttr = *pObjCharacter->GetBasicAttr();
	for (int i = 0; i < MES_END; i++)
	{
		if (pObjCharacter->GetEquipmentItem(MEquipmentSlot(i)) == NULL) info.nEquipmentItem[i] = -1;
		else info.nEquipmentItem[i] = pObjCharacter->GetEquipmentItem(MEquipmentSlot(i))->GetDescID();
	}

	pNew->AddParameter(new MCommandParameterBlob((char*)&info, sizeof(info)));

	Post(pNew);	
}

int MZoneServer::ObjectEquipItem(MUID& TargetUID, MUID& ChrUID, MUID& ItemUID, MEquipmentSlot slot)
{
	MObjectCharacter* pObjectCharacter = FindCharacter(ChrUID);
	if (pObjectCharacter == NULL) return MERR_OBJECT_INVALID;

	if (pObjectCharacter->EquipItem(ItemUID, slot))
	{
		MObjectItem* pObjectItem = pObjectCharacter->FindItem(ItemUID);
		ResponseEquipItem(TargetUID, ItemUID, slot);
	}

	return MOK;
}

MObjectCharacter* MZoneServer::FindCharacter(const MUID& uidChr)
{
	MObjectList::iterator i = m_Objects.find(uidChr);
	if(i==m_Objects.end()) return NULL;

	if (ISEQUALCLASS(MObjectCharacter, (*i).second))
	{
		return (MObjectCharacter*)(*i).second;
	}
	else { return NULL; }
}

MObjectItem* MZoneServer::FindItem(MUID& ItemUID)
{
	MObjectList::iterator i = m_Objects.find(ItemUID);
	if(i==m_Objects.end()) return NULL;

	if (ISEQUALCLASS(MObjectItem, (*i).second))
	{
		return (MObjectItem*)(*i).second;
	}
	else { return NULL; }
}

void MZoneServer::ResponseEquipItem(MUID& TargetUID, MUID& ItemUID, MEquipmentSlot slot)
{
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_OBJECT_RESPONSE_EQUIP_ITEM), TargetUID, m_This);
	pNew->AddParameter(new MCommandParameterUID(ItemUID));
	pNew->AddParameter(new MCommandParameterInt(int(slot)));
	Post(pNew);
}