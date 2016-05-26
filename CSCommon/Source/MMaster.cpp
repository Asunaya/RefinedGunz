#include "stdafx.h"
#include "MMaster.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MCommandBuilder.h"

MMaster* MMaster::m_pInstance = NULL;

MUID MMaster::UseUID(void)
{
	// 마스터에서는 UseUID를 쓸 이유가 없다.
	// m_GlobalUIDRefMap.Generate(pCommInfo); 하는 형태만 허용된다.
	_ASSERT(FALSE);
	return MUID::Invalid();
}

int MMaster::OnAccept(MCommObject* pCommObj)
{
	// 새로 할당
	pCommObj->SetUID( m_GlobalUIDRefMap.Generate(pCommObj) );
	MUID AllocUID = pCommObj->GetUID();

	MCommandBuilder* pCmdBuilder = pCommObj->GetCommandBuilder();
	pCmdBuilder->SetUID(GetUID(), AllocUID);

	m_CommRefCache.insert(MUIDRefCache::value_type(pCommObj->GetUID(), pCommObj));

	LOG(LOG_DEBUG, "New Communicator connected. UID %u:%u assigned", AllocUID.High, AllocUID.Low);

	return ReplyConnect(&m_This, &AllocUID, 0, pCommObj);
}

void MMaster::OnRegisterCommand(MCommandManager* pCommandManager)
{
	MCommandCommunicator::OnRegisterCommand(pCommandManager);
	MAddSharedCommandTable(pCommandManager, MSCT_MASTER);
	Log(LOG_DEBUG, "Command registeration completed");
}

bool MMaster::OnCommand(MCommand* pCommand)
{
	switch(pCommand->GetID()){
		/*
		case MC_NET_ECHO:
			{
				SetReceiver(pCommand->m_Sender);
				MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_LOCAL_ECHO));
				pNew->AddParameter(new MCommandParameterString((char*)pCommand->GetParameter(0)->GetPointer()));
				SendCommand(pNew);
				return true;
			}
		*/
		case MC_NET_REQUEST_INFO:
			{
				ResponseInfo(pCommand->m_Sender);
				return true;
			}
		case MC_NET_REQUEST_UID:
			{
				int nSize;
				pCommand->GetParameter(&nSize, 0, MPT_INT);
				MUIDRANGE r = m_GlobalUIDRefMap.Reserve(nSize);
				MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_RESPONSE_UID), pCommand->m_Sender, m_This);
				pNew->AddParameter(new MCommandParameterUID(r.Start));
				pNew->AddParameter(new MCommandParameterUID(r.End));
				Post(pNew);
				return true;
			}
		default:
			_ASSERT(0);	// 아직 핸들러가 없다.
			return false;
	}
	return false;
}

void MMaster::ResponseInfo(MUID& Target)
{
	char szTemp[1024];
	MUIDRANGE r = m_GlobalUIDRefMap.GetReservedCount();
	sprintf_safe(szTemp, "MAIET Master-Controller\n UID : %u:%u\n UID Reserved Count : %u:%u ~ %u:%u", m_This.High, m_This.Low, r.Start.High, r.Start.Low, r.End.High, r.End.Low);

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_RESPONSE_INFO), Target, m_This);
	pNew->AddParameter(new MCommandParameterString(szTemp));
	Post(pNew);
}

MMaster::MMaster(void)
{
	_ASSERT(m_pInstance==NULL);
	m_pInstance = this;

	m_This = m_GlobalUIDRefMap.Generate(this);	// 마스터의 MUID는 무조건 1
	_ASSERT(m_This==MASTER_UID);

	SetName("MASTER");	// For Debug
}

MMaster::~MMaster(void)
{
}

MMaster* MMaster::GetInstance(void)
{
	return m_pInstance;
}

bool MMaster::Create(int nPort)
{
	if(MServer::Create(nPort)==false) return false;

	return true;
}

void MMaster::Destroy(void)
{
	MServer::Destroy();
}
