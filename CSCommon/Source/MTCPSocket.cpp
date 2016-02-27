#include "stdafx.h"
#include "MTCPSocket.h"
#include "MDebug.h"

#define MAX_RECVBUF_LEN						4096
#define TCPSOCKET_MAX_SENDQUEUE_LEN			5120
#define MAX_CLIENTSOCKET_LEN				60

////////////////////////////////////////////////////////////////////////////////////////////
// MTCPSocketThread ///////////////////////////////////////////////////////////////////////////
MTCPSocketThread::MTCPSocketThread(MTCPSocket* pTCPSocket)
{
	m_nTotalSend = 0;
	m_nTotalRecv = 0;

	m_pTCPSocket = pTCPSocket;
	m_fnSocketErrorCallback = NULL;
	m_pCallbackContext = NULL;
}

MTCPSocketThread::~MTCPSocketThread()
{
	
}

void MTCPSocketThread::Create()
{
	if (m_bActive) Destroy();

	InitializeCriticalSection(&m_csSendLock);
	MThread::Create(); 
	m_bActive = true;

}

void MTCPSocketThread::Destroy()
{

	m_bActive = false;

	m_KillEvent.SetEvent(); 
	MThread::Destroy();		// Wait for Thread Death

	DeleteCriticalSection(&m_csSendLock); 

}

void MTCPSocketThread::Run()
{

}

void MTCPSocketThread::OnSocketError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode)
{
	if (m_fnSocketErrorCallback)
		m_fnSocketErrorCallback(m_pCallbackContext, sock, ErrorEvent, ErrorCode);
}
////////////////////////////////////////////////////////////////////////////////////////////
// MClientSocketThread /////////////////////////////////////////////////////////////////////
bool MClientSocketThread::OnConnect(SOCKET sock)
{
	m_bActive = true;

	if (m_fnConnectCallback)
		return m_fnConnectCallback(m_pCallbackContext, sock);

	return false;
}

bool MClientSocketThread::OnDisconnect(SOCKET sock)
{
	m_bActive = false;

	if (m_fnDisconnectCallback)
		return m_fnDisconnectCallback(m_pCallbackContext, sock);

	return false;
}

void MClientSocketThread::ClearSendList()
{
	LockSend();
	for (TCPSendListItor itor = m_SendList.begin(); itor != m_SendList.end(); )
	{
		MTCPSendQueueItem*		pItem = (*itor);
		delete [] pItem->pPacket;
		delete pItem;
		++itor;
	}
	m_SendList.clear();

	for (TCPSendListItor itor = m_TempSendList.begin(); itor != m_TempSendList.end(); )
	{
		MTCPSendQueueItem*		pItem = (*itor);
		delete [] pItem->pPacket;
		delete pItem;
		++itor;
	}
	m_TempSendList.clear();

	UnlockSend();

}

bool MClientSocketThread::FlushSend()
{
	TCPSendListItor	SendItor;

	LockSend();
	while (!m_TempSendList.empty())
	{
		TCPSendListItor itor = m_TempSendList.begin();
		m_SendList.push_back(*itor);
		m_TempSendList.erase(itor);
	}
	UnlockSend();

	while(!m_SendList.empty())
	{
		SendItor = m_SendList.begin();
		MTCPSendQueueItem* pSendItem = (MTCPSendQueueItem*)(*SendItor);

		unsigned int nTransBytes = 0;
		while(TRUE) {
			int nSent = send(m_pTCPSocket->GetSocket(), (char*)pSendItem->pPacket+nTransBytes,
							pSendItem->dwPacketSize-nTransBytes, 0);
			if (nSent == SOCKET_ERROR) {
				int nErrCode = WSAGetLastError();
					char szBuf[64]; sprintf_s(szBuf, "FLUSHSEND> FlushSend Error(%d)\n", nErrCode);
					OutputDebugString(szBuf);

				if ( (nErrCode == WSAECONNABORTED) ||
					 (nErrCode == WSAECONNRESET) ||
					 (nErrCode == WSAETIMEDOUT) ) 
				{
					 OutputDebugString("FLUSHSEND> Connection ERROR Closed!!!!!!!!!! \n");
					 return false;
				}
				
				if (nErrCode == WSAEWOULDBLOCK)
					continue;
			} else {
				nTransBytes += nSent;
				if (nTransBytes >= pSendItem->dwPacketSize)
					break;
			}
		}

		m_nTotalSend += nTransBytes;
		m_SendTrafficLog.Record(m_nTotalSend);

		LockSend();
		if (pSendItem != NULL)
		{
			delete [] pSendItem->pPacket;
			delete pSendItem;
		}
		m_SendList.erase(SendItor);

		UnlockSend();
	}

/*
	char buf[255];
	sprintf_s(buf, "Flush : %d 남았다\n", m_SendList.size());
	OutputDebugString(buf);
*/

	return true;
}

void MClientSocketThread::Run()
{
	while(true) 
	{	
		DWORD dwVal = WaitForSingleObject(m_KillEvent.GetEvent(), 100);
		if (dwVal == WAIT_OBJECT_0) 
		{
			return;
		} 
		else if (dwVal == WAIT_TIMEOUT) 
		{
			if (m_pTCPSocket)
				break;
		}
	}

	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
	WORD wEventIndex = 0;

	WSANETWORKEVENTS NetEvent;
	WSAEVENT hFDEvent = WSACreateEvent();
	DWORD result;

	result = WSAEventSelect(m_pTCPSocket->GetSocket(), hFDEvent, FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE);
	if (SOCKET_ERROR == result)
	{
		OutputDebugString("<SOCKETTHREAD_ERROR> WSAEventSelect Error </SOCKETTHREAD_ERROR>\n");
		return;
	}

	EventArray[wEventIndex++] = hFDEvent;
	EventArray[wEventIndex++] = m_SendEvent.GetEvent();
	EventArray[wEventIndex++] = m_KillEvent.GetEvent();

	bool bEnd = false;
	while (!bEnd)
	{
		result = WSAWaitForMultipleEvents(wEventIndex, EventArray, FALSE, 100, FALSE);
		if (result == WSA_WAIT_FAILED) continue;

		switch (result)
		{
		case WAIT_OBJECT_0:		// Socket Event
			{
				result = WSAEnumNetworkEvents(m_pTCPSocket->GetSocket(), hFDEvent, &NetEvent);

				if (SOCKET_ERROR == result)
				{
					OutputDebugString("<SOCKETTHREAD_ERROR> WSAEnumNetworkEvents Error </SOCKETTHREAD_ERROR>\n");
					ResetEvent(hFDEvent);
					continue;
				}
				
				if (NetEvent.lNetworkEvents & FD_CONNECT)
				{
					if (NetEvent.iErrorCode[FD_CONNECT_BIT] == 0)
					{
						OnConnect(m_pTCPSocket->GetSocket());
					}
					else
					{
						OnSocketError(m_pTCPSocket->GetSocket(), eeConnect, NetEvent.iErrorCode[FD_CONNECT_BIT]);
						m_KillEvent.SetEvent();
					}
				}
				
				if (NetEvent.lNetworkEvents & FD_READ)
				{
					if (NetEvent.iErrorCode[FD_READ_BIT] == 0)
					{
						if (m_bActive) Recv();
					}
					else
					{
						OnSocketError(m_pTCPSocket->GetSocket(), eeReceive, NetEvent.iErrorCode[FD_READ_BIT]);
					}
					
				}

				if (NetEvent.lNetworkEvents & FD_WRITE)
				{
					if (NetEvent.iErrorCode[FD_WRITE_BIT] == 0)
					{
//						OutputDebugString("FlushSend 수행했다.\n");
						if (m_bActive) FlushSend();
					}
					else
					{
						OnSocketError(m_pTCPSocket->GetSocket(), eeSend, NetEvent.iErrorCode[FD_WRITE_BIT]);
					}
				}

				if (NetEvent.lNetworkEvents & FD_CLOSE)
				{
					OnDisconnect(m_pTCPSocket->GetSocket());
					
					if (NetEvent.iErrorCode[FD_CLOSE_BIT] != 0)
					{
						OnSocketError(m_pTCPSocket->GetSocket(), eeDisconnect, NetEvent.iErrorCode[FD_CLOSE_BIT]);
					}
					m_KillEvent.SetEvent();
				}
			}
			break;
		case WAIT_OBJECT_0 + 1:		// Send Event
			{
				if (m_bActive) FlushSend();
				m_SendEvent.ResetEvent();
			}
			break;
		case WAIT_OBJECT_0 + 2:		// Kill Event
			{
				bEnd = true;
				m_KillEvent.ResetEvent();
			}
			break;
		case WSA_WAIT_TIMEOUT:
			{
				if (GetSendWaitQueueCount() > 0)
					FlushSend();
			}
			break;
		default:
			{
				OutputDebugString("<SOCKETTHREAD_ERROR> exceptional case </SOCKETTHREAD_ERROR>\n");
				bEnd = true;
			}
			break;
		}	// switch

	}	// while

	WSACloseEvent(hFDEvent);

	ClearSendList();

	m_bActive = false;
}

MClientSocketThread::MClientSocketThread(MTCPSocket* pTCPSocket): MTCPSocketThread(pTCPSocket)
{
	m_bActive = false;
	m_fnRecvCallback = NULL;
	m_fnConnectCallback = NULL;
	m_fnDisconnectCallback = NULL;
}

MClientSocketThread::~MClientSocketThread()
{

}

bool MClientSocketThread::Recv()
{
	char			RecvBuf[MAX_RECVBUF_LEN];
	int				nRecv = 0;

	while(TRUE) 
	{
		nRecv = recv(m_pTCPSocket->GetSocket(), RecvBuf, MAX_RECVBUF_LEN, 0);
		if (nRecv != SOCKET_ERROR) {
			m_nTotalRecv += nRecv;
			m_RecvTrafficLog.Record(m_nTotalRecv);
		}

		if (nRecv <= 0) break;

		if (m_fnRecvCallback && OnRecv(m_pTCPSocket->GetSocket(), RecvBuf, nRecv) == true)
		{
			continue;
		}
	}

	return true;
}

bool MClientSocketThread::OnRecv(SOCKET socket, char *pPacket, DWORD dwSize)
{
	if (m_fnRecvCallback) return m_fnRecvCallback(m_pCallbackContext, socket, pPacket, dwSize);

	return false;
}

bool MClientSocketThread::PushSend(char *pPacket, DWORD dwPacketSize)
{
	if ((!m_bActive) || (m_SendList.size() > TCPSOCKET_MAX_SENDQUEUE_LEN)) return false;
	

	_ASSERT(dwPacketSize > 0);

	MTCPSendQueueItem* pSendItem = new MTCPSendQueueItem;
	_ASSERT(pSendItem != NULL);

	pSendItem->dwPacketSize = 0;

/* 
	pSendItem->pPacket = new char[dwPacketSize];
	memcpy(pSendItem->pPacket, pPacket, dwPacketSize);
*/
	pSendItem->pPacket = pPacket;
	pSendItem->dwPacketSize = dwPacketSize;

	LockSend();
	m_TempSendList.push_back(pSendItem);
	UnlockSend();

	m_SendEvent.SetEvent();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
// MServerSocketThread /////////////////////////////////////////////////////////////////////
MServerSocketThread::MServerSocketThread(MTCPSocket* pTCPSocket): MTCPSocketThread(pTCPSocket)
{
	m_fnRecvCallback = NULL;
	m_fnAcceptCallback = NULL;
	m_fnDisconnectClientCallback = NULL;
}

MServerSocketThread::~MServerSocketThread()
{

}

void MServerSocketThread::Create()
{
	InitializeCriticalSection(&m_csSocketLock);

	MTCPSocketThread::Create();
}

void MServerSocketThread::Destroy()
{
	MTCPSocketThread::Destroy();

	DeleteCriticalSection(&m_csSocketLock);
}

bool MServerSocketThread::FlushSend()
{
	return false;

	int result = 0;
	MTCPSendQueueItem* pSendItem;
	TCPSendListItor	SendItor;
	MSocketObj*		pSocketObj;
	SocketListItor	SocketItor;

	for (SocketItor = m_SocketList.begin(); SocketItor != m_SocketList.end(); ++SocketItor)		
	{
		pSocketObj = *SocketItor;

		while(pSocketObj->sendlist.size() > 0) 
		{
			SendItor = pSocketObj->sendlist.begin();
			pSendItem = *SendItor;

			result = send(pSocketObj->sock, (char*)pSendItem->pPacket,
							pSendItem->dwPacketSize, 0);

			if (result == SOCKET_ERROR) {
				return false;
			} else {
				m_nTotalSend += result;
				m_SendTrafficLog.Record(m_nTotalSend);
			}
			
			delete [] pSendItem->pPacket;
			delete pSendItem;

			pSocketObj->sendlist.erase(SendItor);
		}
	}

	return true;
}

bool MServerSocketThread::Recv(MSocketObj* pSocketObj)
{
	char			RecvBuf[MAX_RECVBUF_LEN];
	int				nRecv = 0;

	while(pSocketObj->sock != INVALID_SOCKET)
	{
		nRecv = recv(pSocketObj->sock, RecvBuf, MAX_RECVBUF_LEN, 0);
		if (nRecv != SOCKET_ERROR) {
			m_nTotalRecv += nRecv;
			m_RecvTrafficLog.Record(m_nTotalRecv);
		}

		if (nRecv <= 0) break;
		if (m_fnRecvCallback && OnRecv(pSocketObj, RecvBuf, nRecv) == true)
		{
			continue;
		}

	}

	return true;
}

void MServerSocketThread::Run()
{
	SOCKET Accept;
	int Index = 0;
	bool bEnd = false;
	MSocketObj* pSocketObj;
	WSANETWORKEVENTS NetEvent;
	WSAEVENT hFDEvent, hNewEvent;
	DWORD result;

	hFDEvent = WSACreateEvent();
	result = WSAEventSelect(m_pTCPSocket->GetSocket(), hFDEvent, FD_ACCEPT | FD_CLOSE);

	if (SOCKET_ERROR == result)
	{
		OutputDebugString("<SOCKETTHREAD_ERROR> WSAEventSelect Error </SOCKETTHREAD_ERROR>\n");
		return;
	}

	m_EventArray[0] = hFDEvent;					// Accept 이벤트
	m_EventArray[1] = m_SendEvent.GetEvent();	// Send 이벤트
	m_EventArray[2] = m_KillEvent.GetEvent();	// Kill 이벤트

	while (!bEnd)
	{
		result = WaitForMultipleObjects((int)m_SocketList.size()+3, m_EventArray, FALSE, INFINITE);

        if (result == WAIT_FAILED)
        {
            OutputDebugString("<SOCKETTHREAD_ERROR> WaitForMultipleObjects Failed </SOCKETTHREAD_ERROR>\n");
            break;
        }


		switch (result)
		{
			case WAIT_OBJECT_0:		// Accept Event
				{
					WSAEnumNetworkEvents(m_pTCPSocket->GetSocket(), hFDEvent, &NetEvent);
					if (NetEvent.lNetworkEvents & FD_ACCEPT)
					{
						if (NetEvent.iErrorCode[FD_ACCEPT_BIT] == 0)
						{
							if (m_SocketList.size() >= MAX_CLIENTSOCKET_LEN) 
							{
								break;
							}

							Accept = accept(m_pTCPSocket->GetSocket(), NULL, NULL);

							hNewEvent = WSACreateEvent();
							WSAEventSelect(Accept, hNewEvent, FD_READ | FD_WRITE | FD_CLOSE);

							MSocketObj* pSocketObj;
							pSocketObj = InsertSocketObj(Accept, hNewEvent);

							// Accept Callback
							if (m_fnAcceptCallback && OnAccept(pSocketObj) == true)
							{

							}
						}
						else
						{
							OnSocketError(m_pTCPSocket->GetSocket(), eeDisconnect, NetEvent.iErrorCode[FD_ACCEPT_BIT]);
						}
					}
				}
				break;
			case WAIT_OBJECT_0 + 1:	// Send Event
				{
					FlushSend();
					m_SendEvent.ResetEvent();
				}
				break;

			case WAIT_OBJECT_0 + 2:	// Kill Event
				{
					bEnd = true;
					m_KillEvent.ResetEvent();
				}
				break;
			case WAIT_TIMEOUT:
				{
					FlushSend();
				}
				break;
		}	// switch

		// Client Socket Event
		for (SocketListItor itor = m_SocketList.begin(); itor != m_SocketList.end(); ++itor)
		{
			pSocketObj = *itor;

			result = WaitForSingleObject(pSocketObj->event, 0);
			if (result == WAIT_FAILED)
			{
				OutputDebugString("<SOCKETTHREAD_ERROR> WaitForSingleObject Failed </SOCKETTHREAD_ERROR>\n");
				ExitThread(-1);
			}
			else if (result == WAIT_TIMEOUT)
			{
				continue;
			}

			WSAEnumNetworkEvents(pSocketObj->sock, pSocketObj->event, &NetEvent);
			if (NetEvent.lNetworkEvents & FD_READ)
			{
				if (NetEvent.iErrorCode[FD_READ_BIT] == 0)
				{
					Recv(pSocketObj);
				}
				else
				{
					OnSocketError(pSocketObj->sock, eeReceive, NetEvent.iErrorCode[FD_READ_BIT]);
					Disconnect(pSocketObj);

					OutputDebugString("<SOCKETTHREAD_ERROR> FD_READ error </SOCKETTHREAD_ERROR>\n");
				}
			}
			if (NetEvent.lNetworkEvents & FD_WRITE)
			{
				if (NetEvent.iErrorCode[FD_WRITE_BIT] == 0)
				{
					FlushSend();
				}
				else
				{
					OnSocketError(pSocketObj->sock, eeSend, NetEvent.iErrorCode[FD_WRITE_BIT]);
					Disconnect(pSocketObj);

					OutputDebugString("<SOCKETTHREAD_ERROR> FD_WRITE error </SOCKETTHREAD_ERROR>\n");
				}
			}
			if (NetEvent.lNetworkEvents & FD_CLOSE)
			{
				Disconnect(pSocketObj);

				if (NetEvent.iErrorCode[FD_CLOSE_BIT] != 0)
				{
					OnSocketError(pSocketObj->sock, eeDisconnect, NetEvent.iErrorCode[FD_CLOSE_BIT]);
				}
			}

			// 안쓰는 소켓 해제
			if (pSocketObj->sock == INVALID_SOCKET)
			{
				FreeSocketObj(pSocketObj);
				itor = RemoveSocketObj(itor);
				RenumberEventArray();
//				break;
			}
		}	// for

	}	// while

}

bool MServerSocketThread::OnAccept(MSocketObj *pSocketObj)
{
	if (m_fnAcceptCallback)
		return m_fnAcceptCallback(pSocketObj);

	return false;
}

bool MServerSocketThread::OnDisconnectClient(MSocketObj* pSocketObj)
{
	if (m_fnDisconnectClientCallback)
		return m_fnDisconnectClientCallback(pSocketObj);
	else 
		return false;
}

bool MServerSocketThread::OnRecv(MSocketObj* pSocketObj, char* pPacket, DWORD dwPacketSize)
{
	if (m_fnRecvCallback)
		return m_fnRecvCallback(pSocketObj, pPacket, dwPacketSize);

	return false;
}

MSocketObj* MServerSocketThread::InsertSocketObj(SOCKET sock, HANDLE event)
{
	LockSocket();

	MSocketObj* pSocketObj = new MSocketObj;

	pSocketObj->sock = sock;
	pSocketObj->event = event;

	m_SocketList.push_back(pSocketObj);
	m_EventArray[m_SocketList.size()+2] = event;

	UnlockSocket();

	return pSocketObj;
}

void MServerSocketThread::RenumberEventArray()
{
	int nEventIndex = 1 + 2;
	MSocketObj* pSocketObj;

	for (SocketListItor itor = m_SocketList.begin(); itor != m_SocketList.end(); ++itor)
	{
		pSocketObj = *itor;
		m_EventArray[nEventIndex++] = pSocketObj->event;
	}
}

void MServerSocketThread::FreeSocketObj(MSocketObj *pSocketObj)
{
	MTCPSendQueueItem* pSendItem;
	TCPSendListItor	SendItor;

	// 버퍼에 있는 내용을 지운다.
	while(pSocketObj->sendlist.size() > 0) 
	{
		SendItor = pSocketObj->sendlist.begin();
		pSendItem = *SendItor;

		delete pSendItem->pPacket;
		delete pSendItem;

		pSocketObj->sendlist.erase(SendItor);
	}

	WSACloseEvent(pSocketObj->event);

	if (pSocketObj->sock != INVALID_SOCKET)
	{
		closesocket(pSocketObj->sock);
	}

	delete pSocketObj;	pSocketObj = NULL;
}

SocketListItor MServerSocketThread::RemoveSocketObj(SocketListItor itor)
{
	SocketListItor ret_itor;

	LockSocket();

	MSocketObj* pSocketObj = *itor;
	ret_itor = m_SocketList.erase(itor);

	WSASetEvent(m_EventArray);
 
	UnlockSocket();

	return ret_itor;
}

bool MServerSocketThread::PushSend(MSocketObj *pSocketObj, char *pPacket, DWORD dwPacketSize)
{
	if (pSocketObj->sendlist.size() > TCPSOCKET_MAX_SENDQUEUE_LEN) return false;

	MTCPSendQueueItem* pSendItem = new MTCPSendQueueItem;
	pSendItem->pPacket = new char[dwPacketSize];
	memcpy(pSendItem->pPacket, pPacket, dwPacketSize);
	pSendItem->dwPacketSize = dwPacketSize;

	LockSend();
	pSocketObj->sendlist.push_back(pSendItem);
	UnlockSend();

	m_SendEvent.SetEvent();

	return true;
}

void MServerSocketThread::Disconnect(MSocketObj *pSocketObj)
{
	closesocket(pSocketObj->sock);

	if (m_fnDisconnectClientCallback && OnDisconnectClient(pSocketObj) == true)
	{
	}

	pSocketObj->sock = INVALID_SOCKET;
}

////////////////////////////////////////////////////////////////////////////////////////////
// MTCPSocket //////////////////////////////////////////////////////////////////////////////
MTCPSocket::MTCPSocket()
{
	m_bInitialized = false;
	m_nPort = 0;
	m_Socket = INVALID_SOCKET;
	m_pSocketThread = NULL;
}

MTCPSocket::~MTCPSocket()
{

}

bool MTCPSocket::Initialize()
{
	WSADATA	wsaData;
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
		return false;

	if ( (LOBYTE( wsaData.wVersion ) != 2) || (HIBYTE( wsaData.wVersion ) != 2) ) 
	{
		OutputDebugString("<TCPSOCKET_ERROR> WinSock Version invalid </TCPSOCKET_ERROR>\n");
		return false;
	} 

	m_bInitialized = true;
	return true;
}

void MTCPSocket::Finalize()
{
	if (!m_bInitialized) return;

	WSACleanup();

	m_bInitialized = false;
}

bool MTCPSocket::OpenSocket()
{
	SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == INVALID_SOCKET) return false;

	m_Socket = sockfd;

	u_long option = 1;
	int result = ioctlsocket(sockfd, FIONBIO, (unsigned long*)&option);
	if (SOCKET_ERROR == result)
	{
		OutputDebugString("<TCPSOCKET_ERROR> ioctl fail </TCPSOCKET_ERROR>\n");
		return false;
	}

	BOOL val = TRUE;
	result = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (const char FAR *)&val, sizeof(BOOL) );
	if (SOCKET_ERROR == result) {
		OutputDebugString("<TCPSOCKET_ERROR> setsockopt(TCP_NODELAY) \n");
		return false;
	}

	return true;
}

void MTCPSocket::CloseSocket()
{
//	shutdown(m_Socket, SD_SEND);
	closesocket(m_Socket);
}


////////////////////////////////////////////////////////////////////////////////////////////
// MServerSocket ///////////////////////////////////////////////////////////////////////////
MServerSocket::MServerSocket(): MTCPSocket()
{
	Initialize();
}

MServerSocket::~MServerSocket()
{
	Finalize();
}

bool MServerSocket::Initialize()
{
	MTCPSocket::Initialize();

	m_pSocketThread = new MServerSocketThread(this);

	m_bInitialized = true;

	return true;
}

void MServerSocket::Finalize()
{
	if (!m_bInitialized) return;
	Close();

	if (m_pSocketThread)
	{
		delete m_pSocketThread;	m_pSocketThread = NULL;
	}

	MTCPSocket::Finalize();
}

bool MServerSocket::Disconnect(MSocketObj *pSocketObj)
{
	((MServerSocketThread*)(m_pSocketThread))->Disconnect(pSocketObj);
	return true;
}

bool MServerSocket::Send(MSocketObj *pSocketObj, char *pPacket, DWORD dwPacketSize)
{
	return ((MServerSocketThread*)(m_pSocketThread))->PushSend(pSocketObj, pPacket, dwPacketSize);
}

bool MServerSocket::Listen(int nPort)
{
	if (!m_bInitialized) return false;

	if (!OpenSocket(nPort)) return false;

	listen(m_Socket, 5);

	return true;
}

bool MServerSocket::Close()
{
	CloseSocket();

	return true;
}

bool MServerSocket::OpenSocket(int nPort)
{
	if (!MTCPSocket::OpenSocket()) return false;

	m_pSocketThread->Create();

	sockaddr_in LocalAddress;
	LocalAddress.sin_family			= AF_INET;
	LocalAddress.sin_addr.s_addr	= htonl(INADDR_ANY);
	LocalAddress.sin_port			= htons(nPort);

	if (::bind(m_Socket, (struct sockaddr*)&LocalAddress, sizeof(LocalAddress)) == SOCKET_ERROR) 
	{
		closesocket(m_Socket);
		return false;
	}
	m_LocalAddress = LocalAddress;

	return true;
}

void MServerSocket::CloseSocket()
{
	MTCPSocket::CloseSocket();

	if (m_pSocketThread)
	{
		m_pSocketThread->Destroy();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// MClientSocket ///////////////////////////////////////////////////////////////////////////
MClientSocket::MClientSocket(): MTCPSocket()
{
	Initialize();
}

MClientSocket::~MClientSocket()
{
	Finalize();
}

bool MClientSocket::Initialize()
{
	MTCPSocket::Initialize();

	m_pSocketThread = new MClientSocketThread(this);
	m_bInitialized = true;

	return true;
}

void MClientSocket::Finalize()
{
	if (!m_bInitialized) return;
	CloseSocket();

	if (m_pSocketThread)
	{
		delete m_pSocketThread;	m_pSocketThread = NULL;
	}

	MTCPSocket::Finalize();
}

bool MClientSocket::Connect(SOCKET* pSocket, char *szIP, int nPort)
{
//	if (IsActive()) return false;

	if (GetSocket() != INVALID_SOCKET)
		CloseSocket();

	strcpy_safe(m_szHost, szIP);
	m_nPort = nPort;

	sockaddr_in RemoteAddr;
	memset((char*)&RemoteAddr, 0, sizeof(sockaddr_in));
	DWORD dwAddr = inet_addr(szIP);
	if (dwAddr != INADDR_NONE) {
		memcpy(&(RemoteAddr.sin_addr), &dwAddr, 4);
	} else {		// 연결할 host name을 입력한 경우
		HOSTENT* pHost = gethostbyname(szIP);
		if (pHost == NULL) {	// error
			OutputDebugString("<TCPSOCKET_ERROR> Can't resolve hostname </TCPSOCKET_ERROR>");
			return false;
		}
		memcpy((char FAR *)&(RemoteAddr.sin_addr), pHost->h_addr, pHost->h_length);
	}

	sockaddr_in addr;

	memset(&addr, 0, sizeof(sockaddr_in));
	memcpy(&addr, &RemoteAddr, sizeof(sockaddr_in));	//	addr.sin_addr.S_un.S_addr = inet_addr(szIP);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);

	OpenSocket();
	if (pSocket) 
		*pSocket = m_Socket;

	DWORD result = connect(m_Socket, (sockaddr*)&addr, sizeof(sockaddr_in));
	if (SOCKET_ERROR == result)	{
		DWORD dwError = WSAGetLastError();
		if (dwError != WSAEWOULDBLOCK) {
			char szLog[64];
			sprintf_s(szLog, "Connect Failed (ErrorCode=%d) \n", dwError);
			OutputDebugString(szLog);
			return false;
		}
	}

	char szLog[64];
	sprintf_s(szLog, "Connect Succeed \n");
	OutputDebugString(szLog);

	return true;
}

bool MClientSocket::Disconnect()
{
	if (!IsActive()) return false;

	CloseSocket();

	((MClientSocketThread*)m_pSocketThread)->OnDisconnect(m_Socket);

	return true;
}

bool MClientSocket::SimpleDisconnect()
{
	if( !IsActive() ) return false;

	SimpleCloseSocket();

	// ((MClientSocketThread*)m_pSocketThread)->OnDisconnect(m_Socket);
	return true;
}

bool MClientSocket::OpenSocket()
{
	if (!MTCPSocket::OpenSocket()) return false;

	m_pSocketThread->Create();

	return true;
}

void MClientSocket::CloseSocket()
{
	if (IsActive()) 
	{
		MTCPSocket::CloseSocket();

		m_pSocketThread->Destroy();
	}
}

void MClientSocket::SimpleCloseSocket()
{
	MTCPSocket::CloseSocket();
}

bool MClientSocket::Send(char *pPacket, DWORD dwPacketSize)
{
	if ((!m_bInitialized)) return false;
	return ((MClientSocketThread*)(m_pSocketThread))->PushSend(pPacket, dwPacketSize);
}
