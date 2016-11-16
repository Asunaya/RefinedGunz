#pragma once

#include "MCommandCommunicator.h"
#include "RealCPNet.h"

#include <list>

class MCommand;

class MServer : public MCommandCommunicator {
protected:
	MRealCPNet					m_RealCPNet;

	list<MCommObject*>			m_AcceptWaitQueue;
	CRITICAL_SECTION			m_csAcceptWaitQueue;

	void LockAcceptWaitQueue()		{ EnterCriticalSection(&m_csAcceptWaitQueue); }
	void UnlockAcceptWaitQueue()		{ LeaveCriticalSection(&m_csAcceptWaitQueue); }

	MUIDRefCache<MCommObject>			m_CommRefCache;
	CRITICAL_SECTION			m_csCommList;

	void LockCommList()			{ EnterCriticalSection(&m_csCommList); }
	void UnlockCommList()		{ LeaveCriticalSection(&m_csCommList); }

	MCommandList				m_SafeCmdQueue;
	CRITICAL_SECTION			m_csSafeCmdQueue;
	void LockSafeCmdQueue()		{ EnterCriticalSection(&m_csSafeCmdQueue); }
	void UnlockSafeCmdQueue()	{ LeaveCriticalSection(&m_csSafeCmdQueue); }

	virtual MUID UseUID() = 0;

	void AddCommObject(const MUID& uid, MCommObject* pCommObj);
	void RemoveCommObject(const MUID& uid);
	void InitCryptCommObject(MCommObject* pCommObj, unsigned int nTimeStamp);

	void PostSafeQueue(MCommand* pNew);

	void SendCommand(MCommand* pCommand);
	void ParsePacket(MCommObject* pCommObj, MPacketHeader* pPacket);

	virtual void OnPrepareRun();
	virtual void OnRun();
	virtual bool OnCommand(MCommand* pCommand);

	virtual void OnNetClear(const MUID& CommUID);
	virtual void OnNetPong(const MUID& CommUID, unsigned int nTimeStamp);

	bool SendMsgReplyConnect(MUID* pHostUID, MUID* pAllocUID, unsigned int nTimeStamp,
		MCommObject* pCommObj);
	bool SendMsgCommand(DWORD nClientKey, char* pBuf, int nSize,
		unsigned short nMsgHeaderID, MPacketCrypterKey* pCrypterKey);

	static void RCPCallback(void* pCallbackContext, RCP_IO_OPERATION nIO,
		DWORD nKey, MPacketHeader* pPacket, DWORD dwPacketLen);	// Thread not safe

public:	// For Debugging
	char m_szName[128];
	void SetName(char* pszName) { strcpy_safe(m_szName, pszName); }
	void DebugLog(char* pszLog) {
		char szLog[128];
		wsprintf(szLog, "[%s] %s \n", m_szName, pszLog);
		OutputDebugString(szLog);
	}

public:
	MServer();
	~MServer();

	bool Create(int nPort, const bool bReuse = false );
	void Destroy();
	int GetCommObjCount();

	virtual int Connect(MCommObject* pCommObj);
	int ReplyConnect(MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp, MCommObject* pCommObj);
	virtual int OnAccept(MCommObject* pCommObj);
	virtual void OnLocalLogin(MUID CommUID, MUID PlayerUID);
	virtual void Disconnect(MUID uid);
	virtual int OnDisconnect(const MUID& uid);	// Thread not safe

	virtual void Log(unsigned int nLogLevel, const char* szLog) = 0;

	void LogF(unsigned int Level, const char* Format, ...);
};
