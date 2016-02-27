#ifndef SAFEUDP_H
#define SAFEUDP_H

/////////////////////////////////////////////////////////////
//	SafeUDP.h	- SafeUDP 1.9.2
//								 Programmed by Kim Young-Ho 
//								    LastUpdate : 2000/07/25
/////////////////////////////////////////////////////////////

#pragma warning(disable:4786)
#include <map>
#include <list>
using namespace std;
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <winsock2.h>
#include "MSync.h"
#include "MThread.h"
#include "MBasePacket.h"
#include "MTrafficLog.h"


class MSafeUDP;


// INNER CLASS //////////////////////////////////////////////////////////////////////////
struct MSendQueueItem {
	DWORD			dwIP;
	WORD			wRawPort;
	MBasePacket*	pPacket;
	DWORD			dwPacketSize;
};

struct MACKQueueItem {
	DWORD			dwIP;
	WORD			wRawPort;
	BYTE			nSafeIndex;
};

struct MACKWaitItem {
	MSafePacket*	pPacket;
	DWORD			dwPacketSize;
	timeval			tvFirstSent;
	timeval			tvLastSent;
	BYTE			nSendCount;
	MACKWaitItem() {}
	~MACKWaitItem() { if (pPacket) delete pPacket; }
};

// OUTER CLASS //////////////////////////////////////////////////////////////////////////
class MNetLink {
public:
	enum LINKSTATE {
		LINKSTATE_CLOSED,
		LINKSTATE_ESTABLISHED,
		LINKSTATE_SYN_SENT,
		LINKSTATE_SYN_RCVD,
		LINKSTATE_FIN_SENT,
		LINKSTATE_FIN_RCVD
	};

	typedef list<MACKWaitItem*>		ACKWaitList;
	typedef ACKWaitList::iterator	ACKWaitListItor;

private:
	MSafeUDP*		m_pSafeUDP;
	bool			m_bConnected;
	LINKSTATE		m_nLinkState;
	sockaddr_in		m_Address;		// IP and Port
	BYTE			m_nNextReadIndex;
	BYTE			m_nNextWriteIndex;

	DWORD			m_dwAuthKey;
	void*			m_pUserData;

public:
	timeval			m_tvConnectedTime;
	timeval			m_tvLastPacketRecvTime;

public:
	ACKWaitList		m_ACKWaitQueue;	// Safe Sent queue, Wait for ACK

private:
	void Setconnected(bool bConnected)	{ m_bConnected = bConnected; }
	void CreateAuthKey() { 	
		srand((unsigned)time(NULL));
		m_dwAuthKey = rand()*rand();
	}
	BYTE GetNextReadIndex()				{ return m_nNextReadIndex++; }
	BYTE GetNextWriteIndex()			{ return m_nNextWriteIndex++; }

public:
	MNetLink();
	~MNetLink();

	bool SendControl(MControlPacket::CONTROL nControl);
	bool OnRecvControl(MControlPacket* pPacket);

	bool SetACKWait(MSafePacket* pPacket, DWORD dwPacketSize);
	bool ClearACKWait(BYTE nSafeIndex);

	void SetSafeUDP(MSafeUDP* pSafeUDP)	{ m_pSafeUDP = pSafeUDP; }
	MSafeUDP* GetSafeUDP()				{ return m_pSafeUDP; }
	bool IsConnected()					{ return m_bConnected; }
	timeval GetConnectedTime()			{ return m_tvConnectedTime; }
	void SetLinkState(MNetLink::LINKSTATE nState);
	MNetLink::LINKSTATE GetLinkState()	{ return m_nLinkState; }
	static bool MakeSockAddr(char* pszIP, int nPort, sockaddr_in* pSockAddr);
	bool SetAddress(char* pszIP, int nPort);
	char* GetIPString()					{ return inet_ntoa(m_Address.sin_addr); }
	DWORD GetIP()						{ return m_Address.sin_addr.S_un.S_addr; }
	WORD GetRawPort()					{ return m_Address.sin_port; }
	int GetPort()						{ return ntohs(m_Address.sin_port); }
	sockaddr_in* GetSockAddr()			{ return &m_Address; }
	__int64 GetMapKey();
	static __int64 GetMapKey(sockaddr_in* pSockAddr);
	timeval GetLastPacketRecvTime()		{ return m_tvLastPacketRecvTime; }

	DWORD GetAuthKey()					{ return m_dwAuthKey; }
	void SetUserData(void* pUserData)	{ m_pUserData = pUserData; }
	void* GetUserData()					{ return m_pUserData; }
};

typedef map<__int64, MNetLink*>	NetLinkMap;
typedef NetLinkMap::value_type	NetLinkType;
typedef NetLinkMap::iterator	NetLinkItor;


// INNER CLASS //////////////////////////////////////////////////////////////////////////
typedef void(MNETLINKSTATECALLBACK)(MNetLink* pNetLink, MNetLink::LINKSTATE nState);
typedef bool(MCUSTOMRECVCALLBACK)(DWORD dwIP, WORD wRawPort, char* pPacket, DWORD dwSize);	// Real UDP Packet
typedef void(MLIGHTRECVCALLBACK)(DWORD dwIP, WORD wRawPort, MLightPacket* pPacket, DWORD dwSize);
typedef void(MGENERICRECVCALLBACK)(MNetLink* pNetLink, MBasePacket* pPacket, DWORD dwSize);

class MSafeUDP;
class MSocketThread : public MThread {
public:
	typedef list<MACKQueueItem*>	ACKSendList;
	typedef ACKSendList::iterator	ACKSendListItor;
	typedef list<MSendQueueItem*>	SendList;
	typedef SendList::iterator		SendListItor;

private:
	MSafeUDP*				m_pSafeUDP;
	MSignalEvent			m_ACKEvent;
	MSignalEvent			m_SendEvent;
	MSignalEvent			m_KillEvent;

	ACKSendList				m_ACKSendList;		// Sending priority High
	ACKSendList				m_TempACKSendList;	// Temporary ACK List for Sync
	CRITICAL_SECTION		m_csACKLock;

	SendList				m_SendList;			// Sending priority Low	(Safe|Normal) Packet
	SendList				m_TempSendList;		// Temporary Send List for Sync
	CRITICAL_SECTION		m_csSendLock;

	DWORD					m_nTotalSend;
	DWORD					m_nTotalRecv;
	MTrafficLog				m_SendTrafficLog;
	MTrafficLog				m_RecvTrafficLog;

public:
	MCUSTOMRECVCALLBACK*	m_fnCustomRecvCallback;
	MLIGHTRECVCALLBACK*		m_fnLightRecvCallback;
	MGENERICRECVCALLBACK*	m_fnGenericRecvCallback;

private:
	void LockACK() { EnterCriticalSection(&m_csACKLock); }
	void UnlockACK() { LeaveCriticalSection(&m_csACKLock); }
	void LockSend() { EnterCriticalSection(&m_csSendLock); }
	void UnlockSend() { LeaveCriticalSection(&m_csSendLock); }

	bool PushACK(MNetLink* pNetLink, MSafePacket* pPacket);
	bool FlushACK();
	bool FlushSend();

	bool SafeSendManage();

	bool Recv();
	bool OnCustomRecv(DWORD dwIP, WORD wRawPort, char* pPacket, DWORD dwSize);
	bool OnControlRecv(DWORD dwIP, WORD wRawPort, MBasePacket* pPacket, DWORD dwSize);
	bool OnLightRecv(DWORD dwIP, WORD wRawPort, MLightPacket* pPacket, DWORD dwSize);
	bool OnACKRecv(DWORD dwIP, WORD wRawPort, MACKPacket* pPacket);
	bool OnGenericRecv(DWORD dwIP, WORD wRawPort, MBasePacket* pPacket, DWORD dwSize);

public:
	MSocketThread()						{ 
		m_pSafeUDP=NULL; 
		m_fnCustomRecvCallback = NULL; m_fnLightRecvCallback=NULL; m_fnGenericRecvCallback=NULL; 
		m_nTotalSend = 0;	m_nTotalRecv = 0;
	}
	~MSocketThread()					{ m_pSafeUDP=NULL; }
	void Create();						
	void Destroy();
	MSafeUDP* GetSafeUDP()				{ return m_pSafeUDP; }
	void SetSafeUDP(MSafeUDP* pSafeUDP)	{ m_pSafeUDP = pSafeUDP; }

	bool PushSend(MNetLink* pNetLink, MBasePacket* pPacket, DWORD dwpPacketSize, bool bRetransmit);	
	bool PushSend(char* pszIP, int nPort, char* pPacket, DWORD dwPacketSize);
	bool PushSend( DWORD dwIP, int nPort, char* pPacket, DWORD dwPacketSize );

	int GetSendTraffic()	{ return m_SendTrafficLog.GetTrafficSpeed(); }
	int GetRecvTraffic()	{ return m_RecvTrafficLog.GetTrafficSpeed(); }

	// Dbg code. don't use for release mode. - by SungE.
	int GetSendListSize() { return (int)m_SendList.size(); }
	int GetTempSendList() { return (int)m_TempSendList.size(); }
	// 

	virtual void Run();
	virtual void Debug();
};

// OUTER CLASS //////////////////////////////////////////////////////////////////////////
class MSafeUDP {
	bool						m_bBindWinsockDLL;		// Socket DLL Load
	SOCKET						m_Socket;			// My Socket
	sockaddr_in					m_LocalAddress;		// My IP and Port

	MSocketThread				m_SocketThread;

	CRITICAL_SECTION			m_csNetLink;

public:
	NetLinkMap					m_NetLinkMap;
	MNETLINKSTATECALLBACK*		m_fnNetLinkStateCallback;
	void LockNetLink()			{ EnterCriticalSection(&m_csNetLink); }
	void UnlockNetLink()		{ LeaveCriticalSection(&m_csNetLink); }

public:
	SOCKET GetLocalSocket()		{ return m_Socket; }
	char* GetLocalIPString()	{ return inet_ntoa(m_LocalAddress.sin_addr); }
	DWORD GetLocalIP()			{ return m_LocalAddress.sin_addr.S_un.S_addr; }
	WORD GetLocalPort()			{ return m_LocalAddress.sin_port; }

	MNetLink* FindNetLink(DWORD dwIP, WORD wRawPort);
	MNetLink* FindNetLink(__int64 nMapKey);

	void GetTraffic(int* nSendTraffic, int* nRecvTraffic) {
		*nSendTraffic = m_SocketThread.GetSendTraffic();
		*nRecvTraffic = m_SocketThread.GetRecvTraffic();
	}

private:
	bool OpenSocket(int nPort, bool bReuse=true);
	void CloseSocket();
	void OnConnect(MNetLink* pNetLink);
	void OnDisconnect(MNetLink* pNetLink);

public:
	bool Create(bool bBindWinsockDLL, int nPort, bool bReuse=true);
	void Destroy();

	void SetNetLinkStateCallback(MNETLINKSTATECALLBACK pCallback) { m_fnNetLinkStateCallback = pCallback; }
	void SetLightRecvCallback(MLIGHTRECVCALLBACK pCallback) { m_SocketThread.m_fnLightRecvCallback = pCallback; }
	void SetGenericRecvCallback(MGENERICRECVCALLBACK pCallback) { m_SocketThread.m_fnGenericRecvCallback = pCallback; }
	void SetCustomRecvCallback(MCUSTOMRECVCALLBACK pCallback) { m_SocketThread.m_fnCustomRecvCallback = pCallback; }

	MNetLink* OpenNetLink(char* szIP, int nPort);
	bool CloseNetLink(MNetLink* pNetLink);

	MNetLink* Connect(char* szIP, int nPort);
	void Reconnect(MNetLink* pNetLink);
	bool Disconnect(MNetLink* pNetLink);
	int DisconnectAll();

	bool Send(MNetLink* pNetLink, MBasePacket* pPacket, DWORD dwSize);
	bool Send(char* pszIP, int nPort, char* pPacket, DWORD dwSize);
	bool Send(DWORD dwIP, int nPort, char* pPacket, DWORD dwSize );

	void Debug() { m_SocketThread.Debug(); }

	// Dbg code. don't use for release mode. - by SungE.
	MSocketThread& GetSocketThread() { return m_SocketThread; }
	//
};


#pragma comment(lib, "ws2_32.lib")


#endif
