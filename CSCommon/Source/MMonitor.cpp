#include "MMonitor.h"
#include "MSharedCommandTable.h"
#include "MZoneServer.h"
#include <stdarg.h>
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"

int MMonitor::OnConnected(MUID* pTargetUID, MUID* pAllocUID, MCommObject* pCommObj)
{
	int ret = MClient::OnConnected(pTargetUID, pAllocUID, pCommObj);
	
	OutputMessage("Communicator connected.", MZMOM_LOCALREPLY);
	OutputMessage(MZMOM_LOCALREPLY, "Your Communicator UID is allocated = %d%d", m_This.High, m_This.Low);

	return ret;
}

bool MMonitor::OnSockConnect(SOCKET sock)
{
	MClient::OnSockConnect(sock);

	return true;
}

bool MMonitor::OnSockDisconnect(SOCKET sock)
{
	MClient::OnSockDisconnect(sock);

	OutputMessage("Communicator disconnected.", MZMOM_LOCALREPLY);
	return true;
}

bool MMonitor::OnSockRecv(SOCKET sock, char* pPacket, DWORD dwSize)
{
	MClient::OnSockRecv(sock, pPacket, dwSize);

	return true;
}

void MMonitor::OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode)
{
	MClient::OnSockError(sock, ErrorEvent, ErrorCode);

	OutputMessage(MZMOM_LOCALREPLY, "Socket Error(Code =  %d)", ErrorCode);	
}

void MMonitor::OnRegisterCommand(MCommandManager* pCommandManager)
{
	MCommandCommunicator::OnRegisterCommand(pCommandManager);
	MAddSharedCommandTable(pCommandManager, MSCT_MONITOR);
}

bool MakeCommuncationInfo(MCommObject* pCommObj, const char* szCommunicatorName)
{
	if(stricmp(szCommunicatorName, "Master")==0){
		pCommObj->SetUID( MUID(0,1) );
		pCommObj->SetAddress("127.0.0.1", 5000);
		//pCI->pDirectConnection = MTempSetMaster();
		return true;
	}
	else if(stricmp(szCommunicatorName, "ZoneServer")==0){
		pCommObj->SetUID( MUID(0,2) );
		pCommObj->SetAddress("127.0.0.1", 6000);
		//pCI->pDirectConnection = MTempGetZoneServer();
		return true;
	}
	return false;
}

bool MMonitor::OnCommand(MCommand* pCommand)
{
	bool ret = MClient::OnCommand(pCommand);

	switch(pCommand->GetID()){
		case MC_VERSION:
			OutputMessage("MAIET Monitor Abstract Version", MZMOM_LOCALREPLY);
			break;
		case MC_NET_ENUM:
			OutputMessage("Master", MZMOM_LOCALREPLY);
			OutputMessage("ZoneServer", MZMOM_LOCALREPLY);
			break;
		case MC_NET_RESPONSE_INFO:
			{
				char szInfo[1024];
				pCommand->GetParameter(szInfo, 0, MPT_STR);
				OutputMessage(szInfo, MZMOM_LOCALREPLY);
			}
			break;
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
				OutputMessage(MZMOM_LOCALREPLY, "Ping from (%u:%u) = %d", pCommand->GetSenderUID().High, pCommand->GetSenderUID().Low, timeGetTime()-nTimeStamp);
			}
			break;
		case MC_ZONESERVER_RESPONSE_MAPLIST:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				int nCount = MGetBlobArrayCount(pBlob);
				for(int i=0; i<nCount; i++){
					MMAPDESCRIPTION* pMapDesc = (MMAPDESCRIPTION*)MGetBlobArrayElement(pBlob, i);
					OutputMessage(pMapDesc->szName);
				}
			}
			break;
		case MC_MONITOR_BROWSE_MAP:
			{
				char szMapName[256];
				pCommand->GetParameter(szMapName, 0, MPT_STR);
				BrowseMap(szMapName);
				OutputMessage(MZMOM_LOCALREPLY, "%s map window created.", szMapName);
			}
			break;
		case MC_MAP_RESPONSE_OBJECTLIST:
			{
				int nMapID, nCellID;
				pCommand->GetParameter(&nMapID, 0, MPT_INT);
				pCommand->GetParameter(&nCellID, 1, MPT_INT);
				MCommandParameter* pParam = pCommand->GetParameter(2);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				OnResponseObjectList(nMapID, nCellID, (MOBJECTBASICINFO*)MGetBlobArrayPointer(pBlob), MGetBlobArrayCount(pBlob));
			}
			break;
		case MC_MAP_RESPONSE_OBJECTLIST2:
			{
				int nMapID;
				pCommand->GetParameter(&nMapID, 0, MPT_INT);
				MCommandParameter* pParam = pCommand->GetParameter(1);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				OnResponseObjectList(nMapID, (MOBJECTBASICINFO*)MGetBlobArrayPointer(pBlob), MGetBlobArrayCount(pBlob));
			}
			break;
		case MC_OBJECT_ASSIGN_PLAYER:
			{
				MUID uid;
				rvector Pos;
				pCommand->GetParameter(&uid, 0, MPT_UID);
				pCommand->GetParameter(&Pos, 1, MPT_POS);
				OutputMessage(MZMOM_LOCALREPLY, "Player assigned %u:%u, (%f, %f, %f).", uid.High, uid.Low, Pos.x, Pos.y, Pos.z);
			}
			break;
		case MC_OBJECT_CACHE_UPDATE:{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				OnResponseObjectCacheUpdate((MOBJECTCACHEINFO*)MGetBlobArrayPointer(pBlob), MGetBlobArrayCount(pBlob));
			}
			break;
		case MC_OBJECT_BASICINFO_UPDATE:{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				OnResponseObjectBasicInfoUpdate((MBLOB_BASICINFO_UPDATE*)MGetBlobArrayPointer(pBlob), MGetBlobArrayCount(pBlob));
			}
			break;
		case MC_MESSAGE_SAY:
			{
				MUID uid;
				char szMessage[1024];
				pCommand->GetParameter(&uid, 0, MPT_UID);
				pCommand->GetParameter(szMessage, 1, MPT_STR);
				OutputMessage(MZMOM_LOCALREPLY, "%u:%u say \"%s\"", uid.High, uid.Low, szMessage);
			}
			break;
		default:
			if (!ret)
			{
				OutputMessage(MZMOM_LOCALREPLY, "Command(%s) handler not found", pCommand->m_pCommandDesc->GetName());
				return false;
			}
	}
	return true;
}

void MMonitor::OnResponseObjectList(int nMapID, int nCellID, MOBJECTBASICINFO* pObjBIArray, int nObjBIArrayCount)
{
	OutputMessage(MZMOM_LOCALREPLY, "Objects in Map %d Cell %d : ", nMapID, nCellID);
//	for(int i=0; i<nObjBIArrayCount; i++){
//		MOBJECTBASICINFO* pObjBI = &(pObjBIArray[i]);
//		OutputMessage(MZMOM_LOCALREPLY, "%u:%u", pObjBI->uid.High, pObjBI->uid.Low);
//	}
}

void MMonitor::OnResponseObjectList(int nMapID, MOBJECTBASICINFO* pObjBIArray, int nObjBIArrayCount)
{
	OutputMessage(MZMOM_LOCALREPLY, "Objects in Map %d", nMapID);
//	for(int i=0; i<nObjBIArrayCount; i++){
//		MOBJECTBASICINFO* pObjBI = &(pObjBIArray[i]);
//		OutputMessage(MZMOM_LOCALREPLY, "%u:%u", pObjBI->uid.High, pObjBI->uid.Low);
//	}
}

void MMonitor::OnResponseObjectCacheUpdate(MOBJECTCACHEINFO* pObjBIArray, int nObjBIArrayCount)
{
	OutputMessage(MZMOM_LOCALREPLY, "Objects Cache Updated");
//	for(int i=0; i<nObjBIArrayCount; i++){
//		MOBJECTCACHEINFO* pObjCI = &(pObjBIArray[i]);
//		OutputMessage(MZMOM_LOCALREPLY, "%u:%u", pObjCI->uid.High, pObjCI->uid.Low);
//	}
}

void MMonitor::OnResponseObjectBasicInfoUpdate(MBLOB_BASICINFO_UPDATE* pObjBIArray, int nObjBIArrayCount)
{
	OutputMessage(MZMOM_LOCALREPLY, "Objects BasicInfo Updated");
//	for(int i=0; i<nObjBIArrayCount; i++){
//		MBLOB_BASICINFO_UPDATE* pObjCI = &(pObjBIArray[i]);
//		OutputMessage(MZMOM_LOCALREPLY, "BasicInfo(%u:%u) HP=%.1f EN=%.1f", pObjCI->uid.High, pObjCI->uid.Low, pObjCI->fHP, pObjCI->fEN);
//	}
}


void MMonitor::OutputLocalInfo(void)
{
	OutputMessage("MAIET Monitor", MZMOM_LOCALREPLY);
	OutputMessage(MZMOM_LOCALREPLY, "UID : %u:%u", m_This.High, m_This.Low);
	OutputMessage(MZMOM_LOCALREPLY, "Connected Communicator : %u:%u", m_Server.High, m_Server.Low);
}

MMonitor::MMonitor(void)
{
	m_Server.SetInvalid();
}

/*
bool MMonitor::Create(int nPort)
{
	if(MClient::Create(nPort)==false) return false;

	return true;
}

void MMonitor::Destroy(void)
{
	MClient::Destroy();
}
*/




MCommandCommunicator* g_pServer = NULL;
MCommandCommunicator* g_pMaster = NULL;
void MTempSetZoneServer(MCommandCommunicator* pServer)
{
	g_pServer = pServer;
}
MCommandCommunicator* MTempGetZoneServer(void)
{
	return g_pServer;
}
void MTempSetMaster(MCommandCommunicator* pMaster)
{
	g_pMaster = pMaster;
}
MCommandCommunicator* MTempSetMaster(void)
{
	return g_pMaster;
}
