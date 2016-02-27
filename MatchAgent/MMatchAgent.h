#ifndef MMATCHAGENT_H
#define MMATCHAGENT_H

#include "winsock2.h"
#include "MServer.h"
#include "MSafeUDP.h"
#include "MAgentClient.h"
#include "MStageAgent.h"
#include "MPacketCrypter.h"


class MMatchAgent : public MServer {
private:
	static MMatchAgent*		m_pInstance;		///< 전역 인스턴스

protected:
	MUID				m_uidMatchServer;

	MUID				m_uidNextAlloc;
	
	MAgentClients		m_Clients;
	MStageAgents		m_Stages;

	MSafeUDP			m_SafeUDP;
	MPacketCrypter		m_MatchServerPacketCrypter;					///< 암호화 클래스
protected:
	char	m_szIP[64];
	int		m_nTCPPort;
	int		m_nUDPPort;

	bool	m_bMatchServerConnectTrying;
	bool	m_bMatchServerConnected;
	UINT	m_nConnectedTime;
	char	m_szMatchServerIP[64];
	int		m_nMatchServerTCPPort;
	
	UINT	m_nLastLiveCheckSendTime;
	UINT	m_nLastLiveCheckRecvTime;

public:
	void SetIP(char* pszIP)					{ strcpy(m_szIP, pszIP); }
	char* GetIP()							{ return m_szIP; }
	void SetTCPPort(int nPort)				{ m_nTCPPort = nPort; }
	int GetTCPPort()						{ return m_nTCPPort; }
	void SetUDPPort(int nPort)				{ m_nUDPPort = nPort; }
	int GetUDPPort()						{ return m_nUDPPort; }

	void SetMatchServerTrying(bool bVal)	{ m_bMatchServerConnectTrying = bVal; }
	bool GetMatchServerTrying()				{ return m_bMatchServerConnectTrying; }
	void SetMatchServerConnected(bool bVal)	{ m_bMatchServerConnected = bVal; }
	bool GetMatchServerConnected()			{ return m_bMatchServerConnected; }
	void SetConnectedTime(UINT nTime)		{ m_nConnectedTime = nTime; }
	UINT GetConnectedTime()					{ return m_nConnectedTime; }
	void SetMatchServerIP(char* pszIP)		{ strcpy(m_szMatchServerIP, pszIP); }
	char* GetMatchServerIP()				{ return m_szMatchServerIP; }
	void SetMatchServerTCPPort(int nPort)	{ m_nMatchServerTCPPort = nPort; }
	int GetMatchServerTCPPort()				{ return m_nMatchServerTCPPort; }

	void SetLastLiveCheckSendTime(unsigned long nTime)	{ m_nLastLiveCheckSendTime = nTime; }
	unsigned long GetLastLiveCheckSendTime()			{ return m_nLastLiveCheckSendTime; }
	void SetLastLiveCheckRecvTime(unsigned long nTime)	{ m_nLastLiveCheckRecvTime = nTime; }
	unsigned long GetLastLiveCheckRecvTime()			{ return m_nLastLiveCheckRecvTime; }

	unsigned long GetStageCount()			{ return (unsigned long)m_Stages.size(); }
	unsigned long GetClientCount()			{ return (unsigned long)m_Clients.size(); }

public:
	MMatchAgent(void);
	virtual ~MMatchAgent(void);

	/// 전역 인스턴스 얻기
	static MMatchAgent* GetInstance(void)	{ return m_pInstance; }

	/// 초기화
	bool Create(int nPort);
	/// 해제
	void Destroy(void);

	/// MatchServer와 연결한다.
	void ConnectToMatchServer(char* pszAddr, int nPort);
	/// MatchServer와 연결을 해제한다.
	void DisconnectFromMatchServer();

protected:
	/// Create()호출시에 불리는 함수
	virtual bool OnCreate(void);
	/// Destroy()호출시에 불리는 함수
	virtual void OnDestroy(void);
	/// 사용자 커맨드 등록
	virtual void OnRegisterCommand(MCommandManager* pCommandManager);
	/// 사용자 커맨드 처리
	virtual bool OnCommand(MCommand* pCommand);
	/// 사용자 루프
	virtual void OnRun(void);

	/// 초기화
	void OnInitialize();

	MUID GetMatchServerUID()	{ return m_uidMatchServer; }

	/// 새로운 UID 얻어내기
	virtual MUID UseUID(void);

	/// UDP
	MSafeUDP* GetSafeUDP() { return &m_SafeUDP; }
	void SendCommandByUDP(MCommand* pCommand, char* szIP, int nPort);
	static bool UDPSocketRecvEvent(DWORD dwIP, WORD wRawPort, char* pPacket, DWORD dwSize);
	void ParseUDPPacket(char* pData, MPacketHeader* pPacketHeader, DWORD dwIP, WORD wRawPort);
	void ParsePacket(char* pData, MPacketHeader* pPacketHeader, DWORD dwIP, WORD wRawPort);

	/// 클라이언트 생성
	int ClientAdd(const MUID& uid);
	/// 클라이언트 제거
	int ClientRemove(const MUID& uid, MAgentClients::iterator* pNextItor);

	/// UID로 오브젝트 얻어내기
	MAgentClient* GetClient(const MUID& uid);
	/// CommUID로 오브젝트 얻어내기
	MAgentClient* GetPlayerByCommUID(const MUID& uid);

	MStageAgent* FindStage(const MUID& uidStage);
	bool StageAdd(const MUID& uidStage);
	bool StageRemove(const MUID& uidStage, MStageAgents::iterator* pNextItor);
	bool StageJoin(const MUID& uidPlayer, const MUID& uidStage);
	bool StageLeave(const MUID& uidPlayer, const MUID& uidStage);

protected:
	/// 커넥션을 받아들인 경우
	virtual int OnAccept(MCommObject* pCommOBj);
	/// 커넥션이 이루어진 경우
	virtual int OnConnected(MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp, MCommObject* pCommObj);
	/// 커넥션이 종료된 경우
	virtual int OnDisconnect(MCommObject* pCommObj);
	virtual void OnAgentLocalLogin(const MUID& uidComm, const MUID& uidPlayer);

	/// MatchServer와 연결을 확인한다.
	void OnResponseLiveCheck(unsigned long nTimeStamp);

	void DebugTest();
	void OnDebugTest(const MUID& uidComm, const char* pszMsg);

	void OnStageReserve(const MUID& uidStage);
	void OnRelayPeer(const MUID& uidChar, const MUID& uidPeer, const MUID& uidStage);
	void OnPeerBindTCP(const MUID& uidComm, const MUID& uidChar);
	void OnPeerBindUDP(const MUID& uidChar, char* szLocalIP, int nLocalPort, char* szIP, int nPort);
	void OnPeerUnbind(const MUID& uidComm, const MUID& uidChar);
	void OnTunnelingTCP(const MUID& uidSender, const MUID& uidReceiver, void* pBlob, int nCount);
	void OnTunnelingUDP(const MUID& uidSender, const MUID& uidReceiver, void* pBlob, int nCount);
	void SendPeerTunnel(MAgentClient* pClient, MAgentClient* pTarget, void* pBlob,int nCount);
	void RoutePeerTunnel(MAgentClient* pClient,const MUID& uidReceiver,void* pBlob,int nCount);

public:
	void RouteToListener(MObject* pObject, MCommand* pCommand);
	/// Command를 지정 Stage 참가자에게 전송
	//void RouteToStage(const MUID& uidStage, MCommand* pCommand);

	/// MatchServer와 연결을 확인한다.
	void RequestLiveCheck();

	/// 현재 클럭 얻어내기
	static unsigned long int GetGlobalClockCount(void);
	/// Local Clock을 Global Clock으로 변환
	static unsigned long int ConvertLocalClockToGlobalClock(unsigned long int nLocalClock, unsigned long int nLocalClockDistance);
	/// Global Clock을 Local Clock으로 변환
	static unsigned long int ConvertGlobalClockToLocalClock(unsigned long int nGlobalClock, unsigned long int nLocalClockDistance);

public: // For Debug
	friend void MDebugUtil_PrintStagePeerList();
};


#endif
