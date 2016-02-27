#ifndef MAGENTCLIENT_H
#define MAGENTCLIENT_H


#include <map>
using namespace std;
#include "MUID.h"
#include "MObject.h"


enum AGENT_PEERTYPE {
	AGENT_PEERTYPE_IDLE,
	AGENT_PEERTYPE_TCP,
	AGENT_PEERTYPE_UDP
};

class MAgentClient : public MObject {
protected:
	AGENT_PEERTYPE	m_nPeerType;
	char 			m_szIP[64];
	unsigned int	m_nPort;

	MUID			m_uidStage;
	list<MUID>		m_PeerRouteList;

public:
	MAgentClient() {}
	MAgentClient(const MUID& uid) : MObject(uid) { 
		SetPeerType(AGENT_PEERTYPE_IDLE);
		ZeroMemory(m_szIP, sizeof(char)*64);
		m_nPort=0;

		m_uidStage = MUID(0,0);
	}
	virtual ~MAgentClient() {};

	AGENT_PEERTYPE GetPeerType()				{ return m_nPeerType; }
	void SetPeerType(AGENT_PEERTYPE nType)		{ m_nPeerType = nType; }
	void SetPeerAddr(char* szIP, unsigned short nPort)	{ strcpy(m_szIP, szIP); m_nPort = nPort; }
	char* GetIP()								{ return m_szIP; }
	unsigned short GetPort()					{ return m_nPort; }

	MUID GetStageUID()							{ return m_uidStage; }
	void SetStageUID(const MUID& uid)			{ m_uidStage = uid; }

	void AddPeerRoute(const MUID& uid);
	list<MUID>::iterator RemovePeerRoute(const MUID& uid);
	bool ExamPeerRoute(const MUID& uid);
	size_t GetPeerRouteCount()					{ return m_PeerRouteList.size(); }
	list<MUID>::iterator GetPeerRouteBegin()	{ return m_PeerRouteList.begin(); }
	list<MUID>::iterator GetPeerRouteEnd()		{ return m_PeerRouteList.end(); }

	/// Æ½ Ã³¸®
	virtual void Tick(unsigned long int nTime);
	virtual bool CheckDestroy(int nTime);
};

class MAgentClients : public map<MUID, MAgentClient*>{};


#endif