#ifndef _MCUSTOMCLIENT_H
#define _MCUSTOMCLIENT_H


/// 클라이언트
class MCustomClient
{
private:

protected:
	MClientSocket		m_ClientSocket;			///< 클라이언트 소켓용 클래스
	CRITICAL_SECTION	m_csRecvLock;			///< CommandQueue critical section
protected:
	void LockRecv() { EnterCriticalSection(&m_csRecvLock); }
	void UnlockRecv() { LeaveCriticalSection(&m_csRecvLock); }

	// Socket Event
	virtual bool OnSockConnect(SOCKET sock);
	virtual bool OnSockDisconnect(SOCKET sock);
	virtual bool OnSockRecv(SOCKET sock, char* pPacket, DWORD dwSize);
	virtual void OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode);
public:
	MCustomClient();
	virtual ~MCustomClient();
	MClientSocket* GetClientSocket()						{ return &m_ClientSocket; }

	virtual int Connect(char* szIP, int nPort);
	void Send(char* pBuf, DWORD nSize);

	static bool SocketRecvEvent(void* pCallbackContext, SOCKET sock, char* pPacket, DWORD dwSize);
	static bool SocketConnectEvent(void* pCallbackContext, SOCKET sock);
	static bool SocketDisconnectEvent(void* pCallbackContext, SOCKET sock);
	static void SocketErrorEvent(void* pCallbackContext, SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode);

	bool IsConnected() { return m_ClientSocket.IsActive(); }
	MClientSocket* GetSock() { return &m_ClientSocket; }
};









#endif