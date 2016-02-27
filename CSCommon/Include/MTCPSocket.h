#ifndef	MTCPSOCKET_H
#define MTCPSOCKET_H

#pragma warning(disable:4786)
#include <list>
#include <vector>
#include <deque>
#include <algorithm>
using namespace std;

#include <winsock2.h>
#include <windows.h>
#include <crtdbg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "MSync.h"
#include "MThread.h"
#include "MPacket.h"
#include "MCommand.h"
#include "MTrafficLog.h"


class MTCPSocket;
class MServerSocket;
class MClientSocket;

struct MTCPSendQueueItem
{
	char*			pPacket;
	DWORD			dwPacketSize;
};

typedef list<MTCPSendQueueItem*>	TCPSendList;
typedef TCPSendList::iterator			TCPSendListItor;

struct MSocketObj
{
	SOCKET				sock;
	HANDLE				event;
	TCPSendList			sendlist;
};

typedef list<MSocketObj*>			SocketList;
typedef SocketList::iterator		SocketListItor;

enum SOCKET_ERROR_EVENT {eeGeneral, eeSend, eeReceive, eeConnect, eeDisconnect, eeAccept};

// general callback
typedef void(MSOCKETERRORCALLBACK)(void* pCallbackContext, SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode);
// client callback
typedef bool(MCLIENTRECVCALLBACK)(void* pCallbackContext, SOCKET socket, char* pPacket, DWORD dwSize);
typedef bool(MCONNECTCALLBACK)(void* pCallbackContext, SOCKET sock);
typedef bool(MDISCONNECTCALLBACK)(void* pCallbackContext, SOCKET sock);
// server callback
typedef bool(MSERVERRECVCALLBACK)(MSocketObj* pSocketObj, char* pPacket, DWORD dwSize);
typedef bool(MACCEPTCALLBACK)(MSocketObj* pSocketObj);
typedef bool(MDISCONNECTCLIENTCALLBACK)(MSocketObj* pSocketObj);

/// 소켓 쓰레드
class MTCPSocketThread : public MThread 
{
private:
protected:
	MTCPSocket*				m_pTCPSocket;
	MSignalEvent			m_SendEvent;
	MSignalEvent			m_KillEvent;
	CRITICAL_SECTION		m_csSendLock;
	bool					m_bActive;

	DWORD					m_nTotalSend;
	DWORD					m_nTotalRecv;
	MTrafficLog				m_SendTrafficLog;
	MTrafficLog				m_RecvTrafficLog;

	virtual void OnSocketError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode);
public:
	MTCPSocketThread(MTCPSocket* pTCPSocket);
	~MTCPSocketThread();
	virtual void Run();
	virtual void Create();
	virtual void Destroy();
	void LockSend()			{ EnterCriticalSection(&m_csSendLock); }
	void UnlockSend()		{ LeaveCriticalSection(&m_csSendLock); }
	bool IsActive()			{ return m_bActive; }

	int GetSendTraffic()	{ return m_SendTrafficLog.GetTrafficSpeed(); }
	int GetRecvTraffic()	{ return m_RecvTrafficLog.GetTrafficSpeed(); }

	void*						m_pCallbackContext;
	MSOCKETERRORCALLBACK*		m_fnSocketErrorCallback;
};

/// 클라이언트용 소켓 쓰레드
class MClientSocketThread : public MTCPSocketThread 
{
private:
protected:
	TCPSendList				m_SendList;			// Sending priority Low	(Safe|Normal) Packet
	TCPSendList				m_TempSendList;		// Temporary Send List for Sync

	size_t GetSendWaitQueueCount()	{ return m_TempSendList.size(); }

	bool OnConnect(SOCKET sock);
	bool OnRecv(SOCKET socket, char* pPacket, DWORD dwSize);
	bool FlushSend();
	bool Recv();
	void ClearSendList();
public:
	MClientSocketThread(MTCPSocket* pTCPSocket);
	~MClientSocketThread();
	virtual void Run();

	bool PushSend(char* pPacket, DWORD dwPacketSize);

	bool OnDisconnect(SOCKET sock);
	int GetSendItemCount()	{ return (int)m_SendList.size(); }

	MCLIENTRECVCALLBACK*	m_fnRecvCallback;
	MCONNECTCALLBACK*		m_fnConnectCallback;
	MDISCONNECTCALLBACK*	m_fnDisconnectCallback;
};

/// 서버용 소켓 쓰레드
class MServerSocketThread : public MTCPSocketThread 
{
private:
	WSAEVENT				m_EventArray[WSA_MAXIMUM_WAIT_EVENTS];
protected:
	CRITICAL_SECTION		m_csSocketLock;

	bool OnRecv(MSocketObj* pSocketObj, char* pPacket, DWORD dwPacketSize);
	bool OnAccept(MSocketObj* pSocketObj);
	bool OnDisconnectClient(MSocketObj* pSocketObj);

	bool FlushSend();
	bool Recv(MSocketObj* pSocketObj);
	void FreeSocketObj(MSocketObj* pSocketObj);
	SocketListItor RemoveSocketObj(SocketListItor itor);
	void RenumberEventArray();
	MSocketObj* InsertSocketObj(SOCKET sock, HANDLE event);
public:
	MServerSocketThread(MTCPSocket* pTCPSocket);
	~MServerSocketThread();	

	void Disconnect(MSocketObj* pSocketObj);
	bool PushSend(MSocketObj* pSocketObj, char *pPacket, DWORD dwPacketSize);
	virtual void Run();
	virtual void Destroy();
	virtual void Create();
	void LockSocket()	{ EnterCriticalSection(&m_csSocketLock); }
	void UnlockSocket()	{ LeaveCriticalSection(&m_csSocketLock); }

	SocketList					m_SocketList;

	MSERVERRECVCALLBACK*		m_fnRecvCallback;
	MACCEPTCALLBACK*			m_fnAcceptCallback;
	MDISCONNECTCLIENTCALLBACK*	m_fnDisconnectClientCallback;
};

/// TCP 소켓 상위 클래스
class MTCPSocket
{
private:
protected:
	bool						m_bInitialized;
	int							m_nPort;			// 포트
	SOCKET						m_Socket;			// My Socket
	MTCPSocketThread*			m_pSocketThread;

	virtual bool Initialize();
	virtual void Finalize();
	virtual bool OpenSocket();
	virtual void CloseSocket();
public:
	MTCPSocket();
	virtual ~MTCPSocket();
	SOCKET GetSocket()			{ return m_Socket; }
	int GetPort()				{ return m_nPort; }
	bool IsActive() { return m_pSocketThread->IsActive(); }
	void GetTraffic(int* nSendTraffic, int* nRecvTraffic) {
		*nSendTraffic = m_pSocketThread->GetSendTraffic();
		*nRecvTraffic = m_pSocketThread->GetRecvTraffic();
	}
	void SetSocketErrorCallback(MSOCKETERRORCALLBACK pCallback) 
					{ m_pSocketThread->m_fnSocketErrorCallback = pCallback; }
	void SetCallbackContext(void* pCallbackContext) 
					{ m_pSocketThread->m_pCallbackContext = pCallbackContext; }
};

/// TCP 소켓 Server 클래스
class MServerSocket: public MTCPSocket
{
private:
protected:
	sockaddr_in					m_LocalAddress;		// My IP and Port
	virtual void Finalize();
	virtual bool Initialize();
	virtual bool OpenSocket(int nPort);
	virtual void CloseSocket();
public:
	MServerSocket();
	virtual ~MServerSocket();

	bool Listen(int nPort);
	bool Close();
	bool Disconnect(MSocketObj* pSocketObj);		// Server에서만 사용 

	bool Send(MSocketObj* pSocketObj, char* pPacket, DWORD dwPacketSize);	


	void SetRecvCallback(MSERVERRECVCALLBACK pCallback) { ((MServerSocketThread*)(m_pSocketThread))->m_fnRecvCallback = pCallback; }
	void SetAcceptCallback(MACCEPTCALLBACK pCallback) { ((MServerSocketThread*)(m_pSocketThread))->m_fnAcceptCallback = pCallback; }
	void SetDisconnectClientCallback(MDISCONNECTCLIENTCALLBACK pCallback) { ((MServerSocketThread*)(m_pSocketThread))->m_fnDisconnectClientCallback = pCallback; }

	SocketList* GetClientList()	{ return &((MServerSocketThread*)(m_pSocketThread))->m_SocketList; }

	char* GetLocalIPString()	{ return inet_ntoa(m_LocalAddress.sin_addr); }
	DWORD GetLocalIP()			{ return m_LocalAddress.sin_addr.S_un.S_addr; }
};

/// TCP 소켓 Client 클래스
class MClientSocket: public MTCPSocket
{
private:
protected:
	char			m_szHost[255];		// Hpst IP
	virtual void Finalize();
	virtual bool Initialize();
	virtual bool OpenSocket();
	virtual void CloseSocket();

	virtual void SimpleCloseSocket();	// 쓰레드는 죽이지 않고, 소켓만 닫을때. - by 추교성.
public:
	MClientSocket();
	virtual ~MClientSocket();

	bool Connect(SOCKET* pSocket, char* szIP, int nPort);			// Client에서만 사용
	bool Disconnect();
	bool Send(char *pPacket, DWORD dwPacketSize);

	virtual bool SimpleDisconnect();	// -by 추교성.

	void SetRecvCallback(MCLIENTRECVCALLBACK pCallback) { ((MClientSocketThread*)(m_pSocketThread))->m_fnRecvCallback = pCallback; }
	void SetConnectCallback(MCONNECTCALLBACK pCallback) { ((MClientSocketThread*)(m_pSocketThread))->m_fnConnectCallback = pCallback; }
	void SetDisconnectCallback(MDISCONNECTCALLBACK pCallback) { ((MClientSocketThread*)(m_pSocketThread))->m_fnDisconnectCallback = pCallback; }

	const char* GetHost()	{ return m_szHost; }
};


#pragma comment(lib, "ws2_32.lib")

#endif