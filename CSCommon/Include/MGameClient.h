#ifndef MGAMECLIENT_H
#define MGAMECLIENT_H

#include "MCommandCommunicator.h"
#include "MClient.h"

/// 게임 클라이언트
class MGameClient : public MClient
{
private:
protected:
	// socket event
	virtual bool OnSockConnect(SOCKET sock);
	virtual bool OnSockDisconnect(SOCKET sock);
	virtual bool OnSockRecv(SOCKET sock, char* pPacket, DWORD dwSize);
	virtual void OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode);

	virtual int OnConnected(MUID* pTargetUID, MUID* pAllocUID, MCommObject* pCommObj);
	virtual void OnRegisterCommand(MCommandManager* pCommandManager);
	virtual bool OnCommand(MCommand* pCommand);

	void OutputLocalInfo(void);
	
public:
	MGameClient();
	virtual ~MGameClient();
	
};



#endif
