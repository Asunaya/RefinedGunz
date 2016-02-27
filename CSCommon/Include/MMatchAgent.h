#ifndef MMATCHAGENT_H
#define MMATCHAGENT_H


#include "MObject.h"


class MMatchAgent : public MObject {
protected:
	char 			m_szIP[64];
	unsigned int	m_nTCPPort;
	unsigned int	m_nUDPPort;

public:
	MMatchAgent() {}
	MMatchAgent(const MUID& uid) : MObject(uid) { 
		ZeroMemory(m_szIP, sizeof(char)*64);	
		m_nTCPPort=0;
		m_nUDPPort=0;
	}
	virtual ~MMatchAgent() {};

	void SetAddr(char* szIP, unsigned short nTCPPort, unsigned short nUDPPort)	{ 
		strcpy(m_szIP, szIP); m_nTCPPort = nTCPPort; m_nUDPPort = nUDPPort;
	}
	char* GetIP()					{ return m_szIP; }
	unsigned short GetTCPPort()		{ return m_nTCPPort; }
	unsigned short GetUDPPort()		{ return m_nUDPPort; }

	int GetStageCount()			{ return 0; }
	int GetAssignCount()		{ return 0; }
};


class MMatchAgentMap : public map<MUID, MMatchAgent*>{};


#endif