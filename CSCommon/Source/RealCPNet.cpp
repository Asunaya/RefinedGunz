#include "stdafx.h"
//////////////////////////////////////////////////////////////////
// Class:	MRealCPNet class (2001/10/25)
// File:	RealCPNet.cpp
// Author:	Kim young ho (moanus@maiet.net)
//
// Implements Scalable Network Module with I/O Competion Port.
// Code based on MS iocpserver/iocpclient example
////////////////////////////////////////////////////////////////// 
#define REALCPNET_LINK_SOCKET_LIBS
#include "RealCPNet.h"
#include <windowsx.h>
#include <crtdbg.h>
#include <time.h>
#include "Shlwapi.h"
#include "MCrashDump.h"
#include "MInetUtil.h"

#ifndef _PUBLISH
	#include "MProcessController.h"
#endif




static RCPLOGFUNC*	g_RCPLog = NULL;
void SetupRCPLog(RCPLOGFUNC* pFunc) { g_RCPLog = pFunc; }
#define RCPLOG if(g_RCPLog) g_RCPLog


static int g_LogSessionCreated = 0;
static int g_LogSessionDestroyed = 0;

MRealCPNet::MRealCPNet()
{
	m_nPort				= DEFAULT_PORT;
	m_bEndServer		= FALSE;            
	m_bRestart			= TRUE;             
	m_bVerbose			= FALSE;
	m_hIOCP				= NULL;
	m_sdListen			= INVALID_SOCKET;
	m_dwThreadCount		= 0;
	ZeroMemory(m_ThreadHandles, sizeof(HANDLE)*MAX_WORKER_THREAD);
	m_hCleanupEvent		= NULL;
	m_fnCallback	= NULL;
	m_pListenSession = NULL;	// 초기화 추가. - by 추교성.
}

MRealCPNet::~MRealCPNet()
{
}

bool MRealCPNet::Create(int nPort, const bool bReuse )
{
	SYSTEM_INFO         systemInfo;
	WSADATA             wsaData;
	int                 nRet;

	GetSystemInfo(&systemInfo);
	m_dwThreadCount = systemInfo.dwNumberOfProcessors * 2;

	m_hCleanupEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hCleanupEvent == NULL) {
		RCPLOG("RealCPNet> CreateEvent(CleanupEvent) failed: %d\n",GetLastError());
		return false;
	}

	if ((nRet = WSAStartup(0x202, &wsaData)) != 0) {
		RCPLOG("RealCPNet> WSAStartup failed: %d\n", nRet);
		CloseHandle(m_hCleanupEvent);
		return false;
	}

	InitializeCriticalSection(&m_csCrashDump);
	//////////////////////////////////////////////

	if (nPort != 0)
		m_nPort = nPort;

	m_bRestart = FALSE;
	m_bEndServer = FALSE;
	ResetEvent(m_hCleanupEvent);

	// notice that we will create more worker threads (dwThreadCount) than 
	// the thread concurrency limit on the IOCP.
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == m_hIOCP) {
		RCPLOG("RealCPNet> CreateIoCompletionPort failed to create I/O completion port: %d\n", GetLastError());
        return false;
	}

	for (DWORD dwCPU=0; dwCPU < m_dwThreadCount; dwCPU++) {
		// Create worker threads to service the overlapped I/O requests.  The decision
		// to create 2 worker threads per CPU in the system is a heuristic.  Also,
		// note that thread handles are closed right away, because we will not need them
		// and the worker threads will continue to execute.
		HANDLE  hThread;
		DWORD   dwThreadId;

		hThread = CreateThread(NULL, 0, WorkerThread, this, 0, &dwThreadId);
		if (hThread == NULL) {
			RCPLOG("RealCPNet> CreateThread failed to create worker thread: %d\n", GetLastError());
			return false;
		}

		m_ThreadHandles[dwCPU] = hThread;
	}
    
	if (!CreateListenSocket(bReuse))
		return false;

    if (!CreateAcceptSocket(TRUE))
		return false;

	return true;
}

void MRealCPNet::Destroy()
{
	mlog("RealCPNet> SessionCreated=%d, SessionDestroyed=%d \n", 
			g_LogSessionCreated, g_LogSessionDestroyed);

	RCPLOG("RealCPNet> Begin Destroy \n");

	m_bEndServer = TRUE;
	// Cause worker threads to exit
    if (m_hIOCP) {
		for (DWORD i = 0; i < m_dwThreadCount; i++)
			PostQueuedCompletionStatus(m_hIOCP, 0, 0, NULL);
	}

	RCPLOG("RealCPNet> Destroy Process 1/4 \n");
            
	//Make sure worker threads exits.
	if (WAIT_OBJECT_0 != WaitForMultipleObjects(m_dwThreadCount,  m_ThreadHandles, TRUE, 1000)) {
		RCPLOG("RealCPNet> WaitFor Threads Exits failed: %d\n", GetLastError());
    } else {
		for (DWORD i=0; i<m_dwThreadCount; i++) {
			if (m_ThreadHandles[i] != INVALID_HANDLE_VALUE) 
				CloseHandle(m_ThreadHandles[i]);
			m_ThreadHandles[i] = INVALID_HANDLE_VALUE;
		}
	}

	RCPLOG("RealCPNet> Destroy Process 2/4 \n");

	if (m_sdListen != INVALID_SOCKET) {
		closesocket(m_sdListen);                                
		m_sdListen = INVALID_SOCKET;
	}
                
	if (m_pListenSession) {
		if (m_pListenSession->GetSocket() != INVALID_SOCKET) 
			closesocket(m_pListenSession->GetSocket());
		m_pListenSession->SetSocket(INVALID_SOCKET);

		if (m_pListenSession)
			delete m_pListenSession;
		m_pListenSession = NULL;
	}

	RCPLOG("RealCPNet> Destroy Process 3/4 \n");

	DeleteAllSession();

	RCPLOG("RealCPNet> Destroy Process 4/4 \n");
   
	if (m_hIOCP) {
		CloseHandle(m_hIOCP);
		m_hIOCP = NULL;
	}
	
	DeleteCriticalSection(&m_csCrashDump);

	WSACleanup();
	CloseHandle(m_hCleanupEvent);

	RCPLOG("RealCPNet> Destroy End \n");
}

// Create a socket with all the socket options we need, namely disable buffering
// and set linger.
SOCKET MRealCPNet::CreateSocket(void)
{
	int         nRet;
	int         nZero = 0;
	LINGER      lingerStruct;
	SOCKET      sdSocket = INVALID_SOCKET;


	sdSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED); 
	if (INVALID_SOCKET == sdSocket) {
		RCPLOG("RealCPNet> WSASocket(sdSocket): %d\n", WSAGetLastError());
		return(sdSocket);
	}
/*
	// Disable send buffering on the socket.  Setting SO_SNDBUF
	// to 0 causes winsock to stop bufferring sends and perform
	// sends directly from our buffers, thereby save one memory copy.
	nZero = 0;
	nRet = setsockopt(sdSocket, SOL_SOCKET, SO_SNDBUF, (char *)&nZero, sizeof(nZero));
	if (SOCKET_ERROR == nRet) {
		RCPLOG("RealCPNet> setsockopt(SNDBUF): %d\n", WSAGetLastError());
		return(sdSocket);
	}

	// Disable receive buffering on the socket.  Setting SO_RCVBUF 
	// to 0 causes winsock to stop bufferring receive and perform
	// receives directly from our buffers, thereby save one memory copy.
	nZero = 0;
	nRet = setsockopt(sdSocket, SOL_SOCKET, SO_RCVBUF, (char *)&nZero, sizeof(nZero));
	if (SOCKET_ERROR == nRet) {
		RCPLOG("RealCPNet> setsockopt(SO_RCVBUF): %d\n", WSAGetLastError());
		return(sdSocket);
	}
*/
	BOOL val = TRUE;
	nRet = setsockopt(sdSocket, IPPROTO_TCP, TCP_NODELAY, (const char FAR *)&val, sizeof(BOOL) );
	if (SOCKET_ERROR == nRet) {
		RCPLOG("RealCPNet> setsockopt(TCP_NODELAY): %d\n", WSAGetLastError());
		return(sdSocket);
	}

	lingerStruct.l_onoff = 1;
	lingerStruct.l_linger = 0;
	nRet = setsockopt(sdSocket, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct));
	if (SOCKET_ERROR == nRet) {
		RCPLOG("RealCPNet> setsockopt(SO_LINGER): %d\n", WSAGetLastError());
		return(sdSocket);
	}

	return(sdSocket);
}

// Create a listening socket, bind, and set up its listening backlog.
BOOL MRealCPNet::CreateListenSocket( const bool bReuse )
{
	SOCKADDR_IN	si_addrlocal;
	int			nRet;
	LINGER		lingerStruct;

	lingerStruct.l_onoff = 1;
	lingerStruct.l_linger = 0;


	m_sdListen = CreateSocket();
	if (INVALID_SOCKET == m_sdListen) {
		return(FALSE);
	}

	if (bReuse) {
		int opt = 1;
		if (setsockopt(m_sdListen, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR)
			return FALSE;
	}

	si_addrlocal.sin_family = AF_INET;
	si_addrlocal.sin_port = htons(m_nPort);
	si_addrlocal.sin_addr.s_addr = htonl(INADDR_ANY);       
	nRet = ::bind(m_sdListen, (struct sockaddr *)&si_addrlocal, sizeof(si_addrlocal));
	if (SOCKET_ERROR == nRet) {
		RCPLOG("RealCPNet> bind: %d\n", WSAGetLastError());
		return(FALSE);
	}

	nRet = listen(m_sdListen, 16);
	if (SOCKET_ERROR == nRet) {
		RCPLOG("RealCPNet> listen: %d\n", WSAGetLastError());
		return(FALSE);
	}

	return(TRUE);
}

// Create a socket and invoke AcceptEx.  Only the original call to to this
// function needs to be added to the IOCP.
//
// If the expected behaviour of connecting client applications is to NOT
// send data right away, then only posting one AcceptEx can cause connection
// attempts to be refused if a client connects without sending some initial
// data (notice that the associated iocpclient does not operate this way 
// but instead makes a connection and starts sending data write away).  
// This is because the IOCP packet does not get delivered without the initial
// data (as implemented in this sample) thus preventing the worker thread 
// from posting another AcceptEx and eventually the backlog value set in 
// listen() will be exceeded if clients continue to try to connect.
//
// One technique to address this situation is to simply cause AcceptEx
// to return right away upon accepting a connection without returning any
// data.  This can be done by setting dwReceiveDataLength=0 when calling AcceptEx.
//
// Another technique to address this situation is to post multiple calls 
// to AcceptEx.  Posting multiple calls to AcceptEx is similar in concept to 
// increasing the backlog value in listen(), though posting AcceptEx is 
// dynamic (i.e. during the course of running your application you can adjust 
// the number of AcceptEx calls you post).  It is important however to keep
// your backlog value in listen() high in your server to ensure that the 
// stack can accept connections even if your application does not get enough 
// CPU cycles to repost another AcceptEx under stress conditions.
// 
// This sample implements neither of these techniques and is therefore
// susceptible to the behaviour described above.
//
BOOL MRealCPNet::CreateAcceptSocket(BOOL fUpdateIOCP)
{
	int		nRet;
	DWORD	dwRecvNumBytes = 0;     

	//The context for listening socket uses the SockAccept member to store the
	//socket for client connection. 
	if (fUpdateIOCP) {
		m_pListenSession = UpdateCompletionPort(m_sdListen, RCP_IO_ACCEPT, FALSE);
		if (m_pListenSession == NULL) {
			RCPLOG("failed to update listen socket to IOCP\n");
			return(FALSE);
		}
	}

	SOCKET sdAccept = CreateSocket();
	if (INVALID_SOCKET == sdAccept) {
		RCPLOG("failed to create new accept socket\n");
		delete m_pListenSession;
		return(FALSE);
	}

	RCPOverlappedAccept* pRCPAccept = new RCPOverlappedAccept();
	pRCPAccept->SetBuffer(m_pListenSession->m_RecvBuffer, MAX_BUFF_SIZE);
	pRCPAccept->SetSocket(sdAccept);

	// pay close attention to these parameters and buffer lengths
	nRet = AcceptEx(m_sdListen, 
					pRCPAccept->GetSocket(),
					(LPVOID)pRCPAccept->GetBuffer(),
					0,
					sizeof(SOCKADDR_IN) + 16,
					sizeof(SOCKADDR_IN) + 16,
					&dwRecvNumBytes,
					(LPOVERLAPPED)pRCPAccept	);
/*	nRet = AcceptEx(m_sdListen, 
					m_pListenSession->m_RecvIOContext.SocketAccept,
					(LPVOID)(m_pListenSession->m_RecvIOContext.Buffer),
					MAX_BUFF_SIZE - (2 * (sizeof(SOCKADDR_IN) + 16)),
					sizeof(SOCKADDR_IN) + 16,
					sizeof(SOCKADDR_IN) + 16,
					&dwRecvNumBytes,
					(LPOVERLAPPED)&(m_pListenSession->m_RecvIOContext.Overlapped)
				   );*/
	if (nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError())) {
		RCPLOG("AcceptEx Failed: %d\n", WSAGetLastError());
		return(FALSE);
	}

	return(TRUE);
}

//  Allocate a context structures for the socket and add the socket to the IOCP.  
//  Additionally, add the context structure to the global list of context structures.
MRealSession* MRealCPNet::UpdateCompletionPort(SOCKET sd, RCP_IO_OPERATION nOperation, BOOL bAddToList)
{
	MRealSession* pSession;
    
	pSession = CreateSession(sd, nOperation);
	if (pSession == NULL) 
		return(NULL);

	m_hIOCP = CreateIoCompletionPort((HANDLE)sd, m_hIOCP, (DWORD_PTR)pSession, 0);
	if (NULL == m_hIOCP) {
		RCPLOG("CreateIoCompletionPort: %d\n", GetLastError());

		delete pSession;
		return(NULL);
	}

	//The listening socket context (bAddToList is FALSE) is not added to the list.
	//All other socket contexts are added to the list.
	if (bAddToList) 
		m_SessionMap.Add(pSession);

	if (m_bVerbose)
		RCPLOG("UpdateCompletionPort: Socket(%d) added to IOCP\n", pSession->GetSocket());

	return pSession;
}

bool MRealCPNet::MakeSockAddr(char* pszIP, int nPort, sockaddr_in* pSockAddr)
{
	sockaddr_in 	RemoteAddr;
	memset((char*)&RemoteAddr, 0, sizeof(sockaddr_in));

	//	Set Dest IP and Port 
	RemoteAddr.sin_family = AF_INET;
	RemoteAddr.sin_port = htons(nPort);
	DWORD dwAddr = inet_addr(pszIP);
	if (dwAddr != INADDR_NONE) {
		memcpy(&(RemoteAddr.sin_addr), &dwAddr, 4);
	} else {		// 연결할 host name을 입력한 경우
		HOSTENT* pHost = gethostbyname(pszIP);
		if (pHost == NULL) {	// error
			OutputDebugString("<REALCP_ERROR> Can't resolve hostname </REALCP_ERROR>\n");
			return false;
		}
		memcpy((char FAR *)&(RemoteAddr.sin_addr), pHost->h_addr, pHost->h_length);
	}
	memcpy(pSockAddr, &RemoteAddr, sizeof(sockaddr_in));
	return true;
}

bool MRealCPNet::CheckIPFloodAttack(sockaddr_in* pRemoteAddr, int* poutIPCount)
{
	bool bResult = false;
	int nIPCount = 0;

	m_SessionMap.Lock();
		for (MSessionMap::iterator i=m_SessionMap.begin(); i!=m_SessionMap.end(); i++) {
			MRealSession* pSession = (*i).second;
			if (pSession->GetSockAddr()->sin_addr.S_un.S_addr == pRemoteAddr->sin_addr.S_un.S_addr)
				nIPCount++;
			if (nIPCount > 32)
				bResult = true;
		}
	m_SessionMap.Unlock();

	*poutIPCount = nIPCount;
	return bResult;
}

bool MRealCPNet::Connect(SOCKET* pSocket, char* pszAddress, int nPort)
{
	SOCKET sdConnect = CreateSocket();
	if (INVALID_SOCKET == sdConnect) {
		RCPLOG("Can't Create Socket \n");
		*pSocket = INVALID_SOCKET;
		return false;
	}
	
	SOCKADDR_IN	ConnectAddr;
	if (MakeSockAddr(pszAddress, nPort, &ConnectAddr) == false) {
		RCPLOG("Can't resolve Address %s:%n", pszAddress, nPort);
		closesocket(sdConnect);
		*pSocket = INVALID_SOCKET;
		return false;
	}

	MRealSession* pContextConnect = UpdateCompletionPort(sdConnect, RCP_IO_CONNECT, TRUE);
	if (pContextConnect == NULL) {
		RCPLOG("failed to update listen socket to IOCP\n");
		closesocket(sdConnect);
		*pSocket = INVALID_SOCKET;
		return false;
	}
	pContextConnect->SetSockAddr(&ConnectAddr, sizeof(SOCKADDR_IN));

/*	int	nRet;
	DWORD dwRecvNumBytes = 0;
	nRet = ConnectEx(sdConnect, &ConnectAddr, sizeof(SOCKADDR_IN), 
				(LPVOID)(m_pListenSession->pIOContext->Buffer),
				MAX_BUFF_SIZE - (2 * (sizeof(SOCKADDR_IN) + 16)),
				&dwRecvNumBytes;
				(LPOVERLAPPED) &(m_pListenSession->pIOContext->Overlapped)
			 );
	if (nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError())) {
		Disconnect(pContextConnect->GetSocket(), FALSE);
		RCPLOG("ConnectEx Failed: %d\n", WSAGetLastError());
		return NULL;
	}*/

	int	nRet;
	nRet = connect(sdConnect, (LPSOCKADDR)&ConnectAddr, sizeof(ConnectAddr));
	if (SOCKET_ERROR == nRet) {
		RCPLOG("RCPLOG> Failed to Connect (%s:%d)\n", pszAddress, nPort);
		CloseSession(pContextConnect, TRUE);
		*pSocket = INVALID_SOCKET;
		return false;
	} else {
		*pSocket = sdConnect;
		PostIORecv(pContextConnect->GetSocket());

		if (m_fnCallback)
			m_fnCallback(m_pCallbackContext, RCP_IO_CONNECT, (DWORD)sdConnect, NULL, 0);

		return true;
	}
}

void MRealCPNet::Disconnect(SOCKET sd)
{
/*	m_SessionMap.Lock();
		MRealSession* pSession = m_SessionMap.GetSessionUnsafe(sd);
		if (pSession) {
			pSession->SetSessionState(MRealSession::SESSIONSTATE_DEAD);
			if (pSession->GetSocket() != INVALID_SOCKET)
				closesocket(pSession->GetSocket());
//			pSession->m_RecvIOContext.IOOperation = RCP_IO_DISCONNECT;
//			PostQueuedCompletionStatus(m_hIOCP, 0, (ULONG_PTR)pSession, 
//									(LPOVERLAPPED)&pSession->m_RecvIOContext);
		}
	m_SessionMap.Unlock();
*/
	//// 2004-12-22 Leak Debug - RAONHAJE //////////////////////////
	MRealSession* pSession = m_SessionMap.GetSession(sd);
	if (pSession)
		CloseSession(pSession, FALSE);
}

//  Close down a connection with a client.  This involves closing the socket (when 
//  initiated as a result of a CTRL-C the socket closure is not graceful).  Additionally, 
//  any context data associated with that socket is free'd.
void MRealCPNet::CloseSession(MRealSession* pSession, BOOL bGraceful)
{
	m_SessionMap.Lock();
		if (m_SessionMap.IsExistUnsafe(pSession)) {
			SOCKET sd = pSession->GetSocket();
			if (m_bVerbose)
				RCPLOG("CloseSession: Socket(%d) connection closing (graceful=%s)\n",
						sd, (bGraceful?"TRUE":"FALSE"));
			if (sd != INVALID_SOCKET) {
				if (!bGraceful) {
					// force the subsequent closesocket to be abortative.
					LINGER  lingerStruct;
					lingerStruct.l_onoff = 1;
					lingerStruct.l_linger = 0;
					setsockopt(sd, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct));
				}
/*				if ( !CancelIo((HANDLE)sd) ) {
					DWORD dwErr = GetLastError();
					RCPLOG("CloseSession(%d): CancelIo(Error=%u) \n", sd, dwErr);
				}
				shutdown(sd, SD_BOTH);*/
				closesocket(sd);
				pSession->SetSocket(INVALID_SOCKET);
				pSession->SetSessionState(MRealSession::SESSIONSTATE_DEAD);	// Set Dead Mark
			}

			if (m_fnCallback)
				m_fnCallback(m_pCallbackContext, RCP_IO_DISCONNECT, (DWORD)sd, NULL, 0);
			pSession->SetUserContext(NULL);
			m_SessionMap.RemoveUnsafe(sd);
			g_LogSessionDestroyed++;
		}
	m_SessionMap.Unlock();
		
	return;    
} 

// Free all context structure in the global list of context structures.
VOID MRealCPNet::DeleteAllSession()
{
	m_SessionMap.RemoveAll();
	return;
}

bool MRealCPNet::GetAddress(SOCKET sd, char* pszAddress, int maxlen, int* pPort)
{
	bool bResult = false;
	m_SessionMap.Lock();
		MRealSession* pSession = m_SessionMap.GetSessionUnsafe(sd);
		if (pSession) {
			in_addr addr;
			addr.S_un.S_addr = pSession->GetIP();

			char ip[16];
			GetIPv4String(addr, ip);

			strcpy_safe(pszAddress, maxlen, ip);
			*pPort = pSession->GetPort();
			bResult = true;
		}
	m_SessionMap.Unlock();
	return bResult;
}

void* MRealCPNet::GetUserContext(SOCKET sd)
{
	void* pRet = NULL;
	m_SessionMap.Lock();
		MRealSession* pSession = m_SessionMap.GetSessionUnsafe(sd);
		if (pSession) {
			pRet = pSession->GetUserContext();
		}
	m_SessionMap.Unlock();
	return pRet;
}

void MRealCPNet::SetUserContext(SOCKET sd, void* pContext)
{
	m_SessionMap.Lock();
		MRealSession* pSession = m_SessionMap.GetSessionUnsafe(sd);
		if (pSession) {
			pSession->SetUserContext(pContext);
		}
	m_SessionMap.Unlock();
}

// pPacket은 malloc, free
bool MRealCPNet::Send(SOCKET sd, MPacketHeader* pPacket, int nSize)
{
	_ASSERT(nSize > 0);

	// Send Queue 구성하려면 여기에..

	m_SessionMap.Lock();
		MRealSession* pSession = m_SessionMap.GetSessionUnsafe(sd);
		if (pSession) {
			PostIOSend(sd, (char*)pPacket, nSize);
		} else {
			free(pPacket);
		}
	m_SessionMap.Unlock();
	return true;
}

bool MRealCPNet::PostIOSend(SOCKET sd, char* pBuf, DWORD nBufLen)
{
//	_ASSERT(nBufLen < MAX_BUFF_SIZE);

	RCPOverlappedSend* pRCPSend = new RCPOverlappedSend();
	pRCPSend->SetData(pBuf, nBufLen);

	WSABUF wsaBuf;
	wsaBuf.buf = pBuf;
	wsaBuf.len = nBufLen;

	DWORD dwSendNumBytes = 0;
	DWORD dwFlags = 0;

	int nRet = WSASend(sd,
					&wsaBuf, 1, &dwSendNumBytes,
					dwFlags,
					pRCPSend, NULL);
//	_ASSERT(dwSendNumBytes==nBufLen);
	if (SOCKET_ERROR == nRet && (ERROR_IO_PENDING != WSAGetLastError())) {
		DWORD dwError = WSAGetLastError();
		RCPLOG("WSASend: %d\n", dwError);
//		Disconnect(sd);
		delete pRCPSend;
	} else {
//		if (m_bVerbose)
//			RCPLOG("<PostIOSend> WorkerThread(%d), Socket(%d) Send posted (%d bytes) \n", 
//					GetCurrentThreadId(), sd, nBufLen);		
	}

	return true;
}

void MRealCPNet::PostIORecv(SOCKET sd)
{
	int nRet = 0;
	m_SessionMap.Lock();
		MRealSession* pSession = m_SessionMap.GetSessionUnsafe(sd);
		if (pSession) {
			RCPOverlappedRecv* pRecv = new RCPOverlappedRecv();
			pRecv->SetBuffer(pSession->m_RecvBuffer, MAX_BUFF_SIZE);


			WSABUF	buffRecv;
			buffRecv.buf = pRecv->GetBuffer();
			buffRecv.len = pRecv->GetBufferSize();

			DWORD dwRecvNumBytes = 0;
			DWORD dwFlags = 0;

			nRet = WSARecv(sd, &buffRecv, 1, &dwRecvNumBytes, &dwFlags,
							pRecv, NULL);
			if (nRet == 0) {
//				if (m_bVerbose)
//					RCPLOG("<PostIORecv> WorkerThread(%d), Socket(%d) Recv completed (%d bytes)\n", 
//							GetCurrentThreadId(), sd, pSession->m_RecvIOContext.nTransBytes);
			}
		}
	m_SessionMap.Unlock();

	if (SOCKET_ERROR == nRet && (ERROR_IO_PENDING != WSAGetLastError())) {
		DWORD dwError = WSAGetLastError();
		RCPLOG("PostIORecv->WSARecv: %d\n", dwError);
	}
}

// Allocate a socket context for the new connection.  
MRealSession* MRealCPNet::CreateSession(SOCKET sd, RCP_IO_OPERATION ClientIO)
{
	MRealSession* pSession;
	pSession = new MRealSession;
	if (pSession) {
		pSession->SetSocket(sd);
//		pSession->m_RecvIOContext.Init(ClientIO);
	} else {
		RCPLOG("new MRealSession Failed: %d\n", GetLastError());
		return NULL;
	}
	g_LogSessionCreated++;
	return(pSession);
}

// Worker thread that handles all I/O requests on any socket handle added to the IOCP.
DWORD WINAPI MRealCPNet::WorkerThread(LPVOID WorkThreadContext)
{


	BOOL				bSuccess = FALSE;
	int					nRet;

	MRealCPNet*			pRealCPNet = (MRealCPNet*)WorkThreadContext;
	HANDLE				hIOCP = pRealCPNet->m_hIOCP;

	LPOVERLAPPED		lpOverlapped = NULL;
	MRealSession*		pSession = NULL;
	MRealSession*		lpAcceptSession = NULL;

	DWORD				dwRecvNumBytes = 0;
	DWORD				dwSendNumBytes = 0;
	DWORD				dwFlags = 0;
	DWORD				dwIoSize;

#ifndef _DEBUG
__try{
#endif
    while(TRUE) {
		// continually loop to service io completion packets
		bSuccess = GetQueuedCompletionStatus(
						hIOCP,
						&dwIoSize,
						(PDWORD_PTR)&pSession,
						&lpOverlapped,
						INFINITE 
						);
		if (!bSuccess) {
//			continue;	// 2005-08-01 RaonHaje
		/*	if (ERROR_OPERATION_ABORTED == GetLastError())
			{
				RCPLOG("GetQueuedCompletionStatus(%d): ERROR_OPERATION_ABORTED(995)\n", 
						pSession->GetSocket());
			}
			RCPLOG("GetQueuedCompletionStatus(%d): %d\n", pSession->GetSocket(), GetLastError());
		*/
		}

		if (pSession == NULL) {
			// CTRL-C handler used PostQueuedCompletionStatus to post an I/O packet with
			// a NULL CompletionKey (or if we get one for any reason).  It is time to exit.
			return 0;
		}

		if (pRealCPNet->m_bEndServer) {
			// main thread will do all cleanup needed - see finally block
			return 0;
		}

		RCPOverlapped* pRCPOverlapped = (RCPOverlapped*)lpOverlapped;

		//We should never skip the loop and not post another AcceptEx if the current
		//completion packet is for previous AcceptEx
		if (pRCPOverlapped->GetIOOperation() != RCP_IO_ACCEPT) {
			if ( !bSuccess || (bSuccess && (0 == dwIoSize)) ) {
				// client connection dropped, continue to service remaining (and possibly 
				// new) client connections
				#ifdef _DEBUG
				char szLog[128]="";
				wsprintf(szLog, "%u:RCP_CLOSE_SESSION(%d)\n", 
					GetTickCount(), pSession->GetSocket());
				OutputDebugString(szLog);
				#endif

				pRealCPNet->CloseSession(pSession, FALSE);	// RAON DEAD POINT
//				delete pRCPOverlapped;	// Session 중복삭제
				continue;
			}
		}

		// determine what type of IO packet has completed by checking the PER_IO_CONTEXT 
		// associated with this socket.  This will determine what action to take.
		switch (pRCPOverlapped->GetIOOperation()) {
		case RCP_IO_ACCEPT:
			{
				// When the AcceptEx function returns, the socket sAcceptSocket is 
				// in the default state for a connected socket. The socket sAcceptSocket 
				// does not inherit the properties of the socket associated with 
				// sListenSocket parameter until SO_UPDATE_ACCEPT_CONTEXT is set on 
				// the socket. Use the setsockopt function to set the SO_UPDATE_ACCEPT_CONTEXT 
				// option, specifying sAcceptSocket as the socket handle and sListenSocket 
				// as the option value. 

				RCPOverlappedAccept* pRCPAccept= (RCPOverlappedAccept*)lpOverlapped;

				if (pRCPAccept->GetSocket() == INVALID_SOCKET) {
					static int nInvalidAccept = 0;
					char szLog[64]; sprintf_safe(szLog, "Accept with INVALID_SOCKET (Count=%d) \n", nInvalidAccept++);
					OutputDebugString(szLog);
					delete pRCPAccept;
					pRealCPNet->CreateAcceptSocket(FALSE);
					continue;
				}

				// Get Address First //
				int locallen, remotelen;
				sockaddr_in *plocal = 0, *premote = 0;
				sockaddr_in LocalAddr, RemoteAddr;

				GetAcceptExSockaddrs((LPVOID)(pRCPAccept->GetBuffer()),
					0,
					sizeof(SOCKADDR_IN) + 16,
					sizeof(SOCKADDR_IN) + 16,
					(sockaddr **)&plocal,
					&locallen,
					(sockaddr **)&premote,
					&remotelen);

				memcpy(&LocalAddr, plocal, sizeof(sockaddr_in));
				memcpy(&RemoteAddr, premote, sizeof(sockaddr_in));
				// Get Address End //

				// Check Connection Flood Attack
				int nIPCount = 0;
				if (pRealCPNet->CheckIPFloodAttack(&RemoteAddr, &nIPCount) == true) {
					RCPLOG("Accept Detected CONNECTION FLOOD ATTACK (IP=%s , Count=%d) \n", 
							inet_ntoa(RemoteAddr.sin_addr), nIPCount);
					closesocket(pRCPAccept->GetSocket());
					pRealCPNet->CreateAcceptSocket(FALSE);
					delete pRCPAccept;
					continue;
				}

				nRet = setsockopt(
							pRCPAccept->GetSocket(), 
							SOL_SOCKET,
							SO_UPDATE_ACCEPT_CONTEXT,
							(char*)&pRealCPNet->m_sdListen,
							sizeof(pRealCPNet->m_sdListen)
						);

				if (nRet == SOCKET_ERROR) {
					int nError = WSAGetLastError();
					//just warn user here.
					RCPLOG("setsockopt(SO_UPDATE_ACCEPT_CONTEXT) failed to update accept socket. (Err:%d) \n", nError);


//					_ASSERT(0);
					closesocket(pRCPAccept->GetSocket());
					pRealCPNet->CreateAcceptSocket(FALSE);
					delete pRCPAccept;
					continue;
				}

				lpAcceptSession = pRealCPNet->UpdateCompletionPort(
												pRCPAccept->GetSocket(), 
												RCP_IO_ACCEPT, TRUE);
				if (lpAcceptSession == NULL) {
					//just warn user here.
					RCPLOG("failed to update accept socket to IOCP\n");
					SetEvent(pRealCPNet->m_hCleanupEvent);
					delete pRCPAccept;
					return 0;
				}

				lpAcceptSession->SetSockAddr(&RemoteAddr, remotelen);
		
/*				RCPLOG("WorkerThread %d: Socket(%d) AcceptEx completed (%d bytes)\n", 
					GetCurrentThreadId(), lpAcceptSession->GetSocket(), dwIoSize);	*/

				//Time to post another outstanding AcceptEx
				if (!pRealCPNet->CreateAcceptSocket(FALSE)) {
					RCPLOG("Please shut down and reboot the server.\n");
					SetEvent(pRealCPNet->m_hCleanupEvent);
					delete pRCPAccept;
					return 0;
				}               

				if (pRealCPNet->m_fnCallback)
					pRealCPNet->m_fnCallback(pRealCPNet->m_pCallbackContext, RCP_IO_ACCEPT, 
											(DWORD)lpAcceptSession->GetSocket(), 
											(MPacketHeader*)lpAcceptSession->m_RecvBuffer, (DWORD)dwIoSize);

				pRealCPNet->PostIORecv(lpAcceptSession->GetSocket());

				delete pRCPAccept;
			}
			break;
/*		case RCP_IO_DISCONNECT:
			{
				pRealCPNet->m_SessionMap.Lock();
					pRealCPNet->CloseSession(pSession, FALSE);
				pRealCPNet->m_SessionMap.Unlock();
			}
			break;*/
		case RCP_IO_READ:
			{
				RCPOverlappedRecv* pRCPRecv= (RCPOverlappedRecv*)lpOverlapped;
				delete pRCPRecv;

				SOCKET sdRecv = INVALID_SOCKET;

				// DEAD인데 IoSize있는경우있음
				pRealCPNet->m_SessionMap.Lock();
					if (pRealCPNet->m_SessionMap.IsExistUnsafe(pSession)) {	// Ensure exist Session 
						if (pSession->GetSessionState() != MRealSession::SESSIONSTATE_DEAD) {
							sdRecv = pSession->GetSocket();
							//lpIOContext->nTransBytes = dwIoSize;
							if (pRealCPNet->m_fnCallback)
								pRealCPNet->m_fnCallback(pRealCPNet->m_pCallbackContext, RCP_IO_READ, 
													(DWORD)pSession->GetSocket(), 
													(MPacketHeader*)pSession->m_RecvBuffer, (DWORD)dwIoSize);
						}
					}
				pRealCPNet->m_SessionMap.Unlock();

				if (INVALID_SOCKET != sdRecv)
					pRealCPNet->PostIORecv(sdRecv);
			}
			break;

		case RCP_IO_WRITE:	// Thread unsafe : 진입한 도중에 session delete 발생
			{
				RCPOverlappedSend* pRCPSend = (RCPOverlappedSend*)lpOverlapped;
				pRCPSend->AddTransBytes(dwIoSize);
				_ASSERT(pRCPSend->GetTransBytes() == pRCPSend->GetTotalBytes());
				delete pRCPSend;

/*				if (pRealCPNet->m_bVerbose) {
					RCPLOG("WorkerThread %d: Socket(%d) Send partially completed (%d bytes, %d/%d) (S=%d,C=%d) \n", 
							GetCurrentThreadId(), pSession->GetSocket(), dwIoSize, 
							pRCPSend->GetTransBytes(), pRCPSend->GetTotalBytes(),
							pSession->m_nSendCount, pSession->m_nSendCompleteCount
					);
				}	*/				
			}
			break;
		} //switch
	} //while
#ifndef _DEBUG
} __except(pRealCPNet->CrashDump(GetExceptionInformation())) 
{
// 서버만 실행하도록 하기위함이다.
#ifndef _PUBLISH
	char szFileName[_MAX_DIR];
	GetModuleFileName(NULL, szFileName, _MAX_DIR);
	HANDLE hProcess = MProcessController::OpenProcessHandleByFilePath(szFileName);
	TerminateProcess(hProcess, 0);
#endif
}
#endif

	return(0);
} 


DWORD MRealCPNet::CrashDump(PEXCEPTION_POINTERS ExceptionInfo)
{
	mlog("CrashDump Entered 1\n");
	EnterCriticalSection(&m_csCrashDump);
	mlog("CrashDump Entered 2\n");

	if (PathIsDirectory("Log") == FALSE)
		CreateDirectory("Log", NULL);

	time_t		tClock;
	tm	tmTime;

	time(&tClock);
	localtime_s(&tmTime, &tClock);

	char szFileName[_MAX_DIR];

	int nFooter = 1;
	while(TRUE) {
		sprintf_safe(szFileName, "Log/RealCPNet_%02d-%02d-%02d-%d.dmp", 
			tmTime.tm_year + 1900, tmTime.tm_mon + 1, tmTime.tm_mday, nFooter);

		if (PathFileExists(szFileName) == FALSE)
			break;

		nFooter++;
		if (nFooter > 100) 
		{
			LeaveCriticalSection(&m_csCrashDump);
			return false;
		}
	}

	DWORD ret = CrashExceptionDump(ExceptionInfo, szFileName);

	mlog("CrashDump Leaving\n");
	LeaveCriticalSection(&m_csCrashDump);
	mlog("CrashDump Leaved\n");

	return ret;
}
