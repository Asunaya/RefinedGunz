#ifndef MSERVER_H
#define MSERVER_H

#include "MCommandCommunicator.h"
#include "RealCPNet.h"

#include <list>
using namespace std;

class MCommand;

/// 서버
class MServer : public MCommandCommunicator {
protected:
	MRealCPNet					m_RealCPNet;

	list<MCommObject*>			m_AcceptWaitQueue;
	CRITICAL_SECTION			m_csAcceptWaitQueue;

	void LockAcceptWaitQueue()		{ EnterCriticalSection(&m_csAcceptWaitQueue); }
	void UnlockAcceptWaitQueue()		{ LeaveCriticalSection(&m_csAcceptWaitQueue); }

	MUIDRefCache				m_CommRefCache;			///< 현재 연결이 설정된 다른 커뮤니케이터 캐쉬
	CRITICAL_SECTION			m_csCommList;

	void LockCommList()			{ EnterCriticalSection(&m_csCommList); }
	void UnlockCommList()		{ LeaveCriticalSection(&m_csCommList); }

	MCommandList				m_SafeCmdQueue;
	CRITICAL_SECTION			m_csSafeCmdQueue;
	void LockSafeCmdQueue()		{ EnterCriticalSection(&m_csSafeCmdQueue); }
	void UnlockSafeCmdQueue()	{ LeaveCriticalSection(&m_csSafeCmdQueue); }

	/// 새로운 UID 얻어내기
	virtual MUID UseUID(void) = 0;

	void AddCommObject(const MUID& uid, MCommObject* pCommObj);
	void RemoveCommObject(const MUID& uid);
	void InitCryptCommObject(MCommObject* pCommObj, unsigned int nTimeStamp);

	void PostSafeQueue(MCommand* pNew);

	/// Low-Level Command Transfer Function. 나중에 모아두었다가 블럭 전송등이 가능하게 해줄 수 있다.
	void SendCommand(MCommand* pCommand);
	void ParsePacket(MCommObject* pCommObj, MPacketHeader* pPacket);

	/// 커뮤니케이터 루프 전 준비
	virtual void  OnPrepareRun(void);
	/// 커뮤니케이터 루프
	virtual void OnRun(void);
	/// 사용자 커맨드 처리
	virtual bool OnCommand(MCommand* pCommand);

	virtual void OnNetClear(const MUID& CommUID);
	virtual void OnNetPong(const MUID& CommUID, unsigned int nTimeStamp);
	virtual void OnHShieldPong(const MUID& CommUID, unsigned int nTimeStamp) {};

	bool SendMsgReplyConnect(MUID* pHostUID, MUID* pAllocUID, unsigned int nTimeStamp, MCommObject* pCommObj);
	bool SendMsgCommand(DWORD nClientKey, char* pBuf, int nSize, unsigned short nMsgHeaderID, MPacketCrypterKey* pCrypterKey);

	static void RCPCallback(void* pCallbackContext, RCP_IO_OPERATION nIO, DWORD nKey, MPacketHeader* pPacket, DWORD dwPacketLen);	// Thread not safe

public:	// For Debugging
	char m_szName[128];
	void SetName(char* pszName) { strcpy_safe(m_szName, pszName); }
	void DebugLog(char* pszLog) {
		char szLog[128];
		wsprintf(szLog, "[%s] %s \n", m_szName, pszLog);
		OutputDebugString(szLog);
	}

public:
	MServer(void);
	~MServer(void);

	/// 초기화
	bool Create(int nPort, const bool bReuse = false );
	/// 해제
	void Destroy(void);
	int GetCommObjCount();


	/// 다른 커뮤티케이터로 연결 설정
	/// @param	pAllocUID	자기 Communicator가 배정받은 UID
	/// @return				에러 코드 (MErrorTable.h 참조)
	virtual int Connect(MCommObject* pCommObj);	// 연결실패시 반드시 Disconnect() 호출해야함
	int ReplyConnect(MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp, MCommObject* pCommObj);
	virtual int OnAccept(MCommObject* pCommObj);
	/// 로그인되었을때
	virtual void OnLocalLogin(MUID CommUID, MUID PlayerUID);
	/// 연결 해제
	virtual void Disconnect(MUID uid);
	virtual int OnDisconnect(const MUID& uid);	// Thread not safe

	virtual void Log(unsigned int nLogLevel, const char* szLog) = 0;

	void LogF(unsigned int Level, const char* Format, ...);
};

#endif
