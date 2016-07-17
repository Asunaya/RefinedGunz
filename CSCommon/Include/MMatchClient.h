#ifndef MMATCHCLIENT_H
#define MMATCHCLIENT_H

#include <list>
#include <map>
using namespace std;
#include "MMatchGlobal.h"
#include "MCommandCommunicator.h"
#include "MClient.h"
#include "MSafeUDP.h"
#include "MMatchObjCache.h"
#include "MMatchObject.h"
#include "MMatchTransDataType.h"
#include "MMatchStage.h"
#include "MMatchGlobal.h"
#include "MPacketCrypter.h"


#define MATCHCLIENT_DEFAULT_UDP_PORT	10000
#define MAX_PING						999

class MMatchPeerInfo
{
public:
	MUID	uidChar;
	char	szIP[64];
	DWORD	dwIP;
	int		nPort;
	MTD_CharInfo		CharInfo;
	MTD_ExtendInfo		ExtendInfo;
protected:
	bool				m_bUDPTestResult;
	bool				m_bUDPTestProcess;
	int					m_nUDPTestCount;

	bool				m_bOpened;
	int					m_nPing;
	int					m_nPingTryCount;
	unsigned int		m_nLastPingTime;
	unsigned int		m_nLastPongTime;
	MCommandSNChecker	m_CommandSNChecker;
public:
	MMatchPeerInfo() {
		uidChar = MUID(0,0);
		szIP[0] = NULL;
		dwIP = 0;
		nPort = 0;
		memset(&CharInfo, 0, sizeof(MTD_CharInfo));

		m_bUDPTestResult = false;
		m_bUDPTestProcess = false;
		m_nUDPTestCount = 0;

		m_bOpened = false;
		m_nPing = 0;
		m_nLastPingTime = 0;
		m_nLastPongTime = 0;
		m_nPingTryCount = 0;
	}
	virtual ~MMatchPeerInfo()			{}
	bool GetUDPTestResult()				{ return m_bUDPTestResult; }
	void SetUDPTestResult(bool bResult)	{ m_bUDPTestResult = bResult; }
	void StartUDPTest()					{ m_bUDPTestProcess = true; m_nUDPTestCount = 10; }
	void StopUDPTest()					{ m_bUDPTestProcess = false; m_nUDPTestCount = 0; }
	bool GetProcess()					{ return m_bUDPTestProcess; }
	int GetTestCount()					{ return m_nUDPTestCount; }
	void UseTestCount()					{ m_nUDPTestCount--; }

	bool IsOpened()						{ return m_bOpened; }
	void SetOpened(bool bVal)			{ m_bOpened = bVal; }
	int GetPing(unsigned int nCurrTime);
	void UpdatePing(unsigned int nTime, int nPing);
	void SetLastPingTime(unsigned int nTime);
	bool CheckCommandValidate(MCommand* pCmd)
	{
		return m_CommandSNChecker.CheckValidate(pCmd->m_nSerialNumber);
	}
};


class MMatchPeerInfoList : public map<MUID, MMatchPeerInfo*>
{
private:
	map<MUID, MMatchPeerInfo*>		m_IPnPortMap;
	CRITICAL_SECTION				m_csLock;
	void Lock()			{ EnterCriticalSection(&m_csLock); }
	void Unlock()		{ LeaveCriticalSection(&m_csLock); }
public:
	MMatchPeerInfoList();
	virtual ~MMatchPeerInfoList();
	void Clear();
	void Add(MMatchPeerInfo* pPeerInfo);
	bool Delete(MMatchPeerInfo* pPeerInfo);
	MMatchPeerInfo* Find(const MUID& uidChar);
	MUID FindUID(DWORD dwIP, int nPort);
};

/// 게임 클라이언트
class MMatchClient : public MClient
{
protected:
	MUID				m_uidServer;
	// Note that this is equivalent to m_This
	MUID				m_uidPlayer;
	MUID				m_uidChannel;
	MUID				m_uidStage;

	char				m_szServerName[64];
	char				m_szServerIP[32];
	int					m_nServerPort;
	int					m_nServerPeerPort;
	MMatchServerMode	m_nServerMode;				///< 서버모드

protected:
	MMatchObjCacheMap	m_ObjCacheMap;
	MSafeUDP			m_SafeUDP;
	MMatchPeerInfoList	m_Peers;
	bool				m_bBridgePeerFlag;
	bool				m_bUDPTestProcess;
	MPacketCrypter		m_AgentPacketCrypter;		///< 에이전트 암호화 키
	MPacketCrypter		m_PeerPacketCrypter;		///< 클라이언트끼리 통신할때의 암호화 키
protected:
	MClientSocket		m_AgentSocket;

	MUID				m_uidAgentServer;
	MUID				m_uidAgentClient;

	char				m_szAgentIP[32];
	int					m_nAgentPort;
	int					m_nAgentPeerPort;

	bool				m_bAgentPeerFlag;	// Agent와 UDP연결여부
	int					m_nAgentPeerCount;	// Agent와 UDP연결시 Retry를 위한 정보

	bool				m_bAllowTunneling;

	bool PeerToPeer = true;

public:
	MCommand* MakeCmdFromTunnelingBlob(const MUID& uidSender, void* pBlob, int nBlobArrayCount);
	MCommand* MakeCmdFromSaneTunnelingBlob(const MUID& uidSender, void* pBlob, size_t Size);
	bool MakeTunnelingCommandBlob(MCommand* pWrappingCmd, MCommand* pSrcCmd);
protected:
	bool GetAgentPeerFlag()				{ return m_bAgentPeerFlag; }
	void SetAgentPeerFlag(bool bVal)	{ m_bAgentPeerFlag = bVal; }
	int GetAgentPeerCount()				{ return m_nAgentPeerCount; }
	void SetAgentPeerCount(int nCount)	{ m_nAgentPeerCount = nCount; }
	void StartAgentPeerConnect();
	void CastAgentPeerConnect();
	void StartUDPTest(const MUID& uidChar);
	void InitPeerCrypt(const MUID& uidStage, unsigned int nChecksum);
protected:
	// tcp socket event
	virtual bool OnSockConnect(SOCKET sock);
	virtual bool OnSockDisconnect(SOCKET sock);
	virtual bool OnSockRecv(SOCKET sock, char* pPacket, DWORD dwSize);
	virtual void OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode);

	virtual int OnConnected(SOCKET sock, MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp);
	virtual void OnRegisterCommand(MCommandManager* pCommandManager);
	virtual bool OnCommand(MCommand* pCommand);
	virtual int OnResponseMatchLogin(const MUID& uidServer, 
									 int nResult, 
									 const char* szServerName, 
		                             const MMatchServerMode nServerMode, 
									 const char* szAccountID, 
                                     const MMatchUserGradeID nUGradeID, 
                                     const MMatchPremiumGradeID nPGradeID,
									 const MUID& uidPlayer,
									 const char* szRandomValue,
									 unsigned char* pbyGuidReqMsg);
	virtual void OnObjectCache(unsigned int nType, void* pBlob, int nCount);
	virtual void OnUDPTest(const MUID& uidChar);
	virtual void OnUDPTestReply(const MUID& uidChar);
	virtual void OnResponseAgentLogin();
	virtual void OnLocateAgentToClient(const MUID& uidAgent, char* szIP, int nPort, int nUDPPort);
	virtual void OnTunnelingTCP(const MUID& uidSender, void* pBlob, int nCount);
	virtual void OnTunnelingUDP(const MUID& uidSender, void* pBlob, int nCount);	
	virtual void OnAllowTunnelingTCP();
	virtual void OnAllowTunnelingUDP();	
	virtual void OnAgentConnected(const MUID& uidAgentServer, const MUID& uidAlloc);
	virtual void OnAgentError(int nError);

	void OutputLocalInfo(void);

	virtual void SendCommand(MCommand* pCommand);
	bool SendCommandToAgent(MCommand* pCommand);
	void SendCommandByTunneling(MCommand* pCommand);
	void SendCommandByMatchServerTunneling(MCommand* pCommand);
	void ParseUDPPacket(char* pData,MPacketHeader* pPacketHeader,DWORD dwIP,unsigned int nPort);
public:
	void SendCommandByUDP(MCommand* pCommand, char* szIP, int nPort);
/*
public :
	void SendCommandByUDP(MCommand* pCommand, char* szIP, int nPort);
	*/

public:
	MMatchClient();
	virtual ~MMatchClient();

	bool Create(unsigned short nUDPPort);
	
	bool GetBridgePeerFlag()			{ return m_bBridgePeerFlag; }
	void SetBridgePeerFlag(bool bFlag)	{ m_bBridgePeerFlag = bFlag; }
	void AddPeer(MMatchPeerInfo* pPeerInfo);
	//bool DeletePeer(const char* szIP);
	bool DeletePeer(const MUID uid);
	MUID FindPeerUID(const DWORD dwIP, const int nPort);
	MMatchPeerInfo* FindPeer(const MUID& uidChar);
	void ClearPeers();
	void CastStageBridgePeer(const MUID& uidChar, const MUID& uidStage);	// UDP lost 대비해 Stage입장시 여러번 호출 필요

	bool GetUDPTestProcess()			{ return m_bUDPTestProcess; }
	void SetUDPTestProcess(bool bVal)	{ m_bUDPTestProcess = bVal; }
	void UpdateUDPTestProcess();
	void GetUDPTraffic(int* nSendTraffic, int* nRecvTraffic)	{ return m_SafeUDP.GetTraffic(nSendTraffic, nRecvTraffic); }

	void SetUDPPort(int nPort);
	MUID GetServerUID() { return m_uidServer; }
	MUID GetPlayerUID()	{ return m_uidPlayer; }
	MUID GetChannelUID() { return m_uidChannel; }
	MUID GetStageUID() { return m_uidStage; }
	virtual MUID GetSenderUIDBySocket(SOCKET socket);


	void SetServerAddr(const char* szIP, int nPort)	{ 
		strcpy_safe(m_szServerIP,szIP), m_nServerPort = nPort; 
	}
	char* GetServerIP() { return m_szServerIP; }
	int GetServerPort() { return m_nServerPort; }
	void SetServerPeerPort(int nPeerPort) { m_nServerPeerPort = nPeerPort; }
	int GetServerPeerPort() { return m_nServerPeerPort; }

	MMatchPeerInfoList* GetPeers() { return &m_Peers; }	
	MSafeUDP* GetSafeUDP() { return &m_SafeUDP; }
	string GetObjName(const MUID& uid);
	MMatchObjCache* FindObjCache(const MUID& uid);
	void ReplaceObjCache(MMatchObjCache* pCache);
	void UpdateObjCache(MMatchObjCache* pCache);
	void RemoveObjCache(const MUID& uid);
	void ClearObjCaches();

	static bool UDPSocketRecvEvent(DWORD dwIP, WORD wRawPort, char* pPacket, DWORD dwSize);

public:
	void SetAgentAddr(const char* szIP, int nPort)	{ 
		strcpy_safe(m_szAgentIP,szIP), m_nAgentPort = nPort; 
	}
	char* GetAgentIP() { return m_szAgentIP; }
	int GetAgentPort() { return m_nAgentPort; }
	void SetAgentPeerPort(int nPeerPort) { m_nAgentPeerPort = nPeerPort; }
	int GetAgentPeerPort() { return m_nAgentPeerPort; }
	const MUID& GetAgentServerUID() { return m_uidAgentServer; }
	const MUID& GetAgentClientUID() { return m_uidAgentClient; }
	bool GetAllowTunneling() { return m_bAllowTunneling; }
	void SetAllowTunneling(bool bAllow) { m_bAllowTunneling = bAllow; }

	int AgentConnect(SOCKET* pSocket, char* szIP, int nPort);
	void AgentDisconnect();

	MMatchObjCacheMap* GetObjCacheMap() { return &m_ObjCacheMap; }
	MMatchServerMode GetServerMode()	{ return m_nServerMode; }
	const char* GetServerName()			{ return m_szServerName; }

};

/* 
로컬에서 게임 2개 띄워놓고 테스트 하는 법
우선 ZoneServer를 띄운다.

Chat창에서 각각의 게임창마다
/con							 ,		/con
/peer.setport 10000				 ,		/peer.setport 10001 
/peer.addpeer 127.0.0.1 10001    ,		/peer.addpeer 127.0.0.1 10000 
이렇게 입력하고서 테스트 해야함.

왜냐하면 각 게임창마다 포트를 다르게 설정해야하기 때문..

*/

#endif