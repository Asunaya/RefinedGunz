// datasocket.cpp : implementation file
//

#include "stdafx.h"
#include "Downloader.h"
#include "datasocket.h"

DWORD DataSocket::m_SocketCount = 0;
HWND DataSocket::m_SocketWnd = NULL;
List<DataSocket *> DataSocket::m_Sockets;

DataSocket::DataSocket()
{
	m_hSocket = INVALID_SOCKET;

	m_Accept = NULL;
	m_Close = NULL;
	m_Connect = NULL;
	m_Receive = NULL;
	m_Send = NULL;
	m_Custom = 0;

	m_RecvBuf = MAlloc(BYTE, 0);
	m_RecvLen = 0;
	m_RecvMax = 0;
	m_RecvBytes = 0;
	m_BpsIn = 0;

	m_SendBuf = MAlloc(BYTE, 0);
	m_SendLen = 0;
	m_SendMax = 0;
	m_SendBytes = 0;
	m_BpsOut = 0;
	m_MaxBpsOut = 5 * 1024;
}

DataSocket::~DataSocket()
{
	MFree(m_RecvBuf);
	MFree(m_SendBuf);

	Close();
}

bool DataSocket::Create(WORD Port, int Type)
{
	SOCKADDR_IN Addr;

	if(m_hSocket != INVALID_SOCKET) Close();

	// Create the Socket
	m_hSocket = socket(AF_INET, Type, 0);

	if(m_hSocket == INVALID_SOCKET) return false;
	DEBUG_SOCKET("Socket 0x%.8X Created\n", m_hSocket);

	// Check for Socket Window
	InitWindow();

	// Bind to Socket Window
	WSAAsyncSelect(m_hSocket, m_SocketWnd, WM_USER, FD_READ | FD_WRITE | FD_ACCEPT | FD_CONNECT | FD_CLOSE);

	// Bind the Socket
	ZeroMemory(&Addr, sizeof(Addr));

	Addr.sin_family = AF_INET;
	Addr.sin_addr.s_addr = htonl(INADDR_ANY);
	Addr.sin_port = htons(Port);

	if(bind(m_hSocket, (SOCKADDR *)&Addr, sizeof(Addr)) == SOCKET_ERROR)
	{
		Close();
		return false;
	}
	
	// Add to Sockets List
	m_Sockets.PushBack(this);

	return true;
}

void DataSocket::Close(void)
{
	DWORD i;

	if(m_hSocket == INVALID_SOCKET) return;

	closesocket(m_hSocket);
	m_hSocket = INVALID_SOCKET;

	// Remove from Sockets List
	for(i = 0; i < m_Sockets.GetSize(); i++)
	{
		if(m_Sockets[i] == this)
		{
			m_Sockets.Delete(i);
			break;
		}
	}

	KillWindow();
}

void DataSocket::Listen(void)
{
	listen(m_hSocket, SOMAXCONN);
}

void DataSocket::Accept(DataSocket & s)
{
	// Add to Sockets List
	m_Sockets.PushBack(&s);

	// Check for Socket Window
	s.InitWindow();

	s.m_hSocket = accept(m_hSocket, NULL, NULL);
}

bool DataSocket::Connect(const char * Address, WORD Port)
{
	sockaddr_in Addr;
	HOSTENT * host;

	ZeroMemory(&Addr, sizeof(Addr));

	Addr.sin_family = AF_INET;
	Addr.sin_addr.s_addr = inet_addr(Address);

	if(Addr.sin_addr.s_addr == INADDR_NONE)
	{
		host = gethostbyname(Address);
		if(host == NULL) return false;
		Addr.sin_addr.s_addr = ((IN_ADDR *)host->h_addr)->s_addr;
	}

	Addr.sin_port = htons(Port);

	connect(m_hSocket, (sockaddr *)&Addr, sizeof(Addr));

	return true;
}

void DataSocket::GetPeerName(CString & Address, WORD & Port)
{
	SOCKADDR_IN sockAddr;
	int nSockAddrLen;

	memset(&sockAddr, 0, sizeof(sockAddr));

	nSockAddrLen = sizeof(sockAddr);

	getpeername(m_hSocket, (SOCKADDR*)&sockAddr, &nSockAddrLen);

	Port = ntohs(sockAddr.sin_port);
	Address = inet_ntoa(sockAddr.sin_addr);
}

void DataSocket::InitWindow(void)
{
	WNDCLASS wc;

	if(m_SocketCount == 0)
	{
		ZeroMemory(&wc, sizeof(wc));

		wc.hInstance = AfxGetInstanceHandle();
		wc.lpfnWndProc = WindowProc;
		wc.lpszClassName = "DataSocket";

		RegisterClass(&wc);

		m_SocketWnd = CreateWindowEx(0, "DataSocket", "DataSocket", 0, 0, 0, 0, 0, NULL, NULL, AfxGetInstanceHandle(), NULL);
		SetTimer(m_SocketWnd, 0, DATASOCKET_SEND_INTERVAL, NULL);
	}

	m_SocketCount++;
}

void DataSocket::KillWindow(void)
{
	m_SocketCount--;

	if(m_SocketCount == 0)
	{
		DestroyWindow(m_SocketWnd);
	}
}

LRESULT CALLBACK DataSocket::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	SOCKET s;
	WORD Event;
	WORD Error;
	DataSocket * ds;
	DWORD i;

	switch(Msg)
	{
	case WM_TIMER:
		for(i = 0; i < m_Sockets.GetSize(); i++)
		{
			if(m_Sockets[i]->m_MaxBpsOut != 0) m_Sockets[i]->OnSend(0);
		}
		break;

	case WM_USER:
		s = (SOCKET)wParam;
		Event = LOWORD(lParam);
		Error = HIWORD(lParam);

		ds = NULL;
		for(i = 0; i < m_Sockets.GetSize(); i++)
		{
			if(m_Sockets[i]->m_hSocket == s)
			{
				ds = m_Sockets[i];
			}
		}

		DEBUG_SOCKET("SocketEvent: 0x%.2X <%.8X>\n", Event, s);

		if(ds)
		{
			switch(Event)
			{
			case FD_READ:
				ds->OnReceive(Error);
				break;

			case FD_WRITE:
				if(ds->m_MaxBpsOut == 0) ds->OnSend(Error);
				break;

			case FD_ACCEPT:
				ds->OnAccept(Error);
				break;

			case FD_CONNECT:
				ds->OnConnect(Error);
				break;

			case FD_CLOSE:
				ds->OnClose(Error);
				break;

			}
		}

		break;

	default:
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}

	return 0;
}

void DataSocket::OnAccept(int nErrorCode)
{
	DEBUG_SOCKET("OnAccept (%d)\n", nErrorCode);

	if(m_Accept) m_Accept(this, m_Custom, nErrorCode);
}

void DataSocket::OnClose(int nErrorCode)
{
	DEBUG_SOCKET("OnClose (%d)\n", nErrorCode);

	if(m_Close) m_Close(this, m_Custom, nErrorCode);
}

void DataSocket::OnConnect(int nErrorCode)
{
	DEBUG_SOCKET("OnConnect (%d)\n", nErrorCode);

	if(m_Connect) m_Connect(this, m_Custom, nErrorCode);
}

void DataSocket::OnReceive(int nErrorCode)
{
	DEBUG_SOCKET("OnReceive (%d)\n", nErrorCode);

	BYTE * Buffer;
	BYTE * Data;
	DWORD Result;

	if(nErrorCode == 0)
	{
		Buffer = MAlloc(BYTE, 1024);

		while(1)
		{
			Result = recv(m_hSocket, (char *)Buffer, 1024, 0);

			if(Result == 0) break;	// Todo: Disconnect? OnClose? (Seems to not be needed)
			if(Result == SOCKET_ERROR) break;

#ifdef DEBUG_SOCKET_DATA
			DWORD i;
			TRACE("<< ");
			for(i = 0; i < Result; i++)
			{
				TRACE("%.2X ", Buffer[i]);
			}
			TRACE("\n");
#endif

			if(m_RecvLen + Result > m_RecvMax)
			{
				Data = MAlloc(BYTE, m_RecvLen + Result);
				m_RecvMax = m_RecvLen + Result;

				memcpy(Data, m_RecvBuf, m_RecvLen);
				memcpy(&Data[m_RecvLen], Buffer, Result);

				MFree(m_RecvBuf);
				m_RecvBuf = Data;
			}else{
				memcpy(&m_RecvBuf[m_RecvLen], Buffer, Result);
			}
			m_RecvLen += Result;
			m_RecvBytes += Result;
		}

		MFree(Buffer);
	}

	if(m_Receive) m_Receive(this, m_Custom, nErrorCode);
}

void DataSocket::OnSend(int nErrorCode)
{
//	DEBUG_SOCKET("OnSend (%d)\n", nErrorCode);

	int Result;
	DWORD MaxOut;

	if(m_SendLen > 0)
	{
		if(m_MaxBpsOut == 0)
		{
			Result = send(m_hSocket, (char *)m_SendBuf, m_SendLen, 0);
		}else{
			MaxOut = m_MaxBpsOut / (1000 / DATASOCKET_SEND_INTERVAL);
			if(MaxOut > m_SendLen) MaxOut = m_SendLen;
			Result = send(m_hSocket, (char *)m_SendBuf, MaxOut, 0);
		}

		if(Result != 0 && Result != SOCKET_ERROR)
		{
			memmove(m_SendBuf, &m_SendBuf[Result], m_SendLen - Result);
			m_SendLen -= Result;

			m_SendBytes += Result;

			if(m_SendLen == 0)
			{
				if(m_Send)
				{
					m_Send(this, m_Custom, nErrorCode);
				}
			}
		}
	}
}

void DataSocket::SetCallbacks(DATASOCKET_CALLBACK cbAccept, DATASOCKET_CALLBACK cbClose, DATASOCKET_CALLBACK cbConnect, DATASOCKET_CALLBACK cbReceive, DATASOCKET_CALLBACK cbSend, DWORD_PTR Custom)
{
	m_Accept = cbAccept;
	m_Close = cbClose;
	m_Connect = cbConnect;
	m_Receive = cbReceive;
	m_Send = cbSend;
	m_Custom = Custom;
}

void DataSocket::SendString(const char * Str)
{
	Send(Str, (DWORD)strlen(Str));
}

void DataSocket::Send(const void * Buf, DWORD BufLen)
{
	BYTE * Data;

#ifdef DEBUG_SOCKET_DATA
	DWORD i;
	TRACE(">> ");
	for(i = 0; i < BufLen; i++)
	{
		TRACE("%.2X ", ((BYTE *)Buf)[i]);
	}
	TRACE("\n");
#endif

	if(m_SendLen + BufLen > m_SendMax)
	{
		Data = MAlloc(BYTE, m_SendLen + BufLen);
		m_SendMax = m_SendLen + BufLen;

		memcpy(Data, m_SendBuf, m_SendLen);
		memcpy(&Data[m_SendLen], Buf, BufLen);

		m_SendLen += BufLen;
		MFree(m_SendBuf);
		m_SendBuf = Data;
	}else{
		memcpy(&m_SendBuf[m_SendLen], Buf, BufLen);

		m_SendLen += BufLen;
	}

	if(m_MaxBpsOut == 0) OnSend(0);
}

BYTE * DataSocket::GetData(void)
{
	return m_RecvBuf;
}

DWORD DataSocket::GetDataLen(void)
{
	return m_RecvLen;
}

void DataSocket::Discard(DWORD Bytes)
{
	memmove(m_RecvBuf, &m_RecvBuf[Bytes], m_RecvLen - Bytes);
	m_RecvLen -= Bytes;
}

DWORD DataSocket::GetSendDataLen(void)
{
	return m_SendLen;
}

DWORD DataSocket::GetBpsIn(void)
{
	return m_BpsIn;
}

DWORD DataSocket::GetBpsOut(void)
{
	return m_BpsOut;
}

void DataSocket::UpdateBps(void)
{
	m_BpsIn = m_RecvBytes;
	m_BpsOut = m_SendBytes;

	m_RecvBytes = 0;
	m_SendBytes = 0;
}

void DataSocket::SetMaxOut(DWORD Bps)
{
	m_MaxBpsOut = Bps;
}

DWORD DataSocket::GetMaxOut(void)
{
	return m_MaxBpsOut;
}
