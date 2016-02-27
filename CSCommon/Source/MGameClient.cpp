#include "MGameClient.h"
#include "MErrorTable.h"
#include "MSharedCommandTable.h"

bool MakeGameClientCommuncationInfo(MCommObject* pCommObj, const char* szCommunicatorName)
{
	if(stricmp(szCommunicatorName, "Server")==0)
	{
		pCommObj->SetAddress("127.0.0.1", 6000);
		return true;
	}
	return false;
}


MGameClient::MGameClient()
{

}
MGameClient::~MGameClient()
{

}

bool MGameClient::OnSockConnect(SOCKET sock)
{
	MClient::OnSockConnect(sock);

	return true;
}
bool MGameClient::OnSockDisconnect(SOCKET sock)
{
	MClient::OnSockDisconnect(sock);

	OutputMessage("Communicator disconnected.", MZMOM_LOCALREPLY);

	return true;
}
bool MGameClient::OnSockRecv(SOCKET sock, char* pPacket, DWORD dwSize)
{
	MClient::OnSockRecv(sock, pPacket, dwSize);

	return true;
}
void MGameClient::OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode)
{
	MClient::OnSockError(sock, ErrorEvent, ErrorCode);

	OutputMessage(MZMOM_LOCALREPLY, "Socket Error(Code =  %d)", ErrorCode);	

}

bool MGameClient::OnCommand(MCommand* pCommand)
{
	bool ret = MClient::OnCommand(pCommand);

	switch(pCommand->GetID()){
		case MC_VERSION:
			OutputMessage("MAIET GameClient Abstract Version", MZMOM_LOCALREPLY);
			break;
		case MC_NET_ENUM:
			break;
		case MC_NET_RESPONSE_INFO:
			break;
		case MC_ZONESERVER_REQUEST_MAPLIST:
			break;

		default:
			if (!ret)
			{
//				OutputMessage(MZMOM_LOCALREPLY, "Command(%s) handler not found", pCommand->m_pCommandDesc->GetName());
				return false;
			}
	}
	return true;
}

void MGameClient::OnRegisterCommand(MCommandManager* pCommandManager)
{
	MClient::OnRegisterCommand(pCommandManager);
	MAddSharedCommandTable(pCommandManager, MSCT_CLIENT);
}

void MGameClient::OutputLocalInfo(void)
{
	OutputMessage("MAIET Game Client", MZMOM_LOCALREPLY);
	OutputMessage(MZMOM_LOCALREPLY, "UID : %u:%u", m_This.High, m_This.Low);
	OutputMessage(MZMOM_LOCALREPLY, "Connected Communicator : %u:%u", m_Server.High, m_Server.Low);
}

int MGameClient::OnConnected(MUID* pTargetUID, MUID* pAllocUID, MCommObject* pCommObj)
{
	int ret = MClient::OnConnected(pTargetUID, pAllocUID, pCommObj);

	OutputMessage(MZMOM_LOCALREPLY, "Communicator connected.");
	OutputMessage(MZMOM_LOCALREPLY, "Your Communicator UID is allocated = %d%d", m_This.High, m_This.Low);

	return ret;
}