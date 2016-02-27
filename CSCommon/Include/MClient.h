#ifndef MCLIENT_H
#define MCLIENT_H

#include "MCommandCommunicator.h"
#include "MPacketCrypter.h"

class MClientCommandProcessor;
class MCommand;

#define RINGBUFSIZE 16384

// 나중에 패킷 버퍼 대체로 사용할 목적으로 우선 만들어둔다.
class MRingBuffer {
private:
	int			m_iBufSize;
	char*		m_Buf;
	char*		m_cpBegin, m_cpEnd;
protected:
public:
	MRingBuffer();
	MRingBuffer(int iBufSize);
	virtual ~MRingBuffer();
	void Reserve(int iBufSize); 
	bool Enqueue(char* cp, int iDataSize);
	bool Dequeue(char* cpOut, int iDataSize);
	int GetSize()		{ return m_iBufSize; }
};

/// Message Type
enum MZMOMType{
	MZMDM_GENERAL,			///< 일반 메세지
	MZMOM_USERCOMMAND,		///< 사용자 입력 커맨드
	MZMOM_ERROR,			///< 에러
	MZMOM_LOCALREPLY,		///< 로컬 응답 메세지
	MZMOM_SERVERREPLY,		///< 서버 응답 메세지
};


/// 클라이언트
class MClient : public MCommandCommunicator{
private:
	static MClient*	m_pInstance;				///< 전역 인스턴스
protected:
	MUID				m_Server;				///< 연결된 커뮤니케이터

	MClientSocket		m_ClientSocket;			///< 클라이언트 소켓용 클래스
	CRITICAL_SECTION	m_csRecvLock;			///< CommandQueue critical section

	MCommandBuilder*	m_pCommandBuilder;
	MPacketCrypter		m_ServerPacketCrypter;	///< MatchServer와의 암호화 클래스

protected:
	void LockRecv() { EnterCriticalSection(&m_csRecvLock); }
	void UnlockRecv() { LeaveCriticalSection(&m_csRecvLock); }

	virtual void OnRegisterCommand(MCommandManager* pCommandManager);
	virtual bool OnCommand(MCommand* pCommand);

	/// 로컬 정보를 출력한다.
	virtual void OutputLocalInfo(void) = 0;
	/// 기본 출력
	virtual void OutputMessage(const char* szMessage, MZMOMType nType=MZMDM_GENERAL) = 0;

	MCommandBuilder* GetCommandBuilder()	{ return m_pCommandBuilder; }
	virtual void SendCommand(MCommand* pCommand);
	virtual MCommand* GetCommandSafe();

	virtual int OnConnected(MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp, MCommObject* pCommObj);
	virtual int OnConnected(SOCKET sock, MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp);
	int MakeCmdPacket(char* pOutPacket, int iMaxPacketSize, MPacketCrypter* pPacketCrypter, MCommand* pCommand);
	

	// Socket Event
	virtual bool OnSockConnect(SOCKET sock);
	virtual bool OnSockDisconnect(SOCKET sock);
	virtual bool OnSockRecv(SOCKET sock, char* pPacket, DWORD dwSize);
	virtual void OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode);

public:
	MClient();
	virtual ~MClient();

	/// 전역 인스턴스 얻기
	static MClient* GetInstance(void);

	MUID GetServerUID(void){ return m_Server; }
	virtual MUID GetSenderUIDBySocket(SOCKET socket);
	MClientSocket* GetClientSocket()						{ return &m_ClientSocket; }
	void GetTraffic(int* nSendTraffic, int* nRecvTraffic)	{ return m_ClientSocket.GetTraffic(nSendTraffic, nRecvTraffic); }

	virtual bool Post(MCommand* pCommand);
	virtual bool Post(char* szErrMsg, int nErrMsgCount, const char* szCommand);

	virtual int Connect(MCommObject* pCommObj);
	virtual int Connect(SOCKET* pSocket, char* szIP, int nPort);
	virtual void Disconnect(MUID uid);
	virtual void Log(const char* szLog){}

	/// 파라미터 출력
	void OutputMessage(MZMOMType nType, const char *pFormat,...);

	static bool SocketRecvEvent(void* pCallbackContext, SOCKET sock, char* pPacket, DWORD dwSize);
	static bool SocketConnectEvent(void* pCallbackContext, SOCKET sock);
	static bool SocketDisconnectEvent(void* pCallbackContext, SOCKET sock);
	static void SocketErrorEvent(void* pCallbackContext, SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode);

	bool IsConnected() { return m_ClientSocket.IsActive(); }
};

void SplitIAddress(char* szIP, int maxlen, int* pPort, const char* szAddress);

#endif
