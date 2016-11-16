#ifndef MCOMMANDCOMMUNICATOR_H
#define MCOMMANDCOMMUNICATOR_H

#include "MCommandManager.h"
#include "MUID.h"
#include "MTCPSocket.h"
#include "MPacketCrypter.h"

#include "SafeString.h"
#include "GlobalTypes.h"

//#define _CMD_PROFILE

#ifdef _CMD_PROFILE
#include "MCommandProfiler.h"
#endif


class MCommandCommunicator;
class MCommandBuilder;


/// 커뮤니케이터와의 접속을 위한 객체. MCommandCommunicator::Connect()의 파라미터로 들어간다.
class MCommObject {
protected:
	MUID					m_uid;

	MCommandBuilder*		m_pCommandBuilder;
	MPacketCrypter			m_PacketCrypter;

	MCommandCommunicator*	m_pDirectConnection;
	DWORD					m_dwUserContext;

	char					m_szIP[128];
	int						m_nPort;
	DWORD					m_dwIP;
	bool					m_bAllowed;

public:
	MCommObject(MCommandCommunicator* pCommunicator);
	virtual ~MCommObject();

	MUID GetUID()			{ return m_uid; }
	void SetUID(MUID uid)	{ m_uid = uid; }

	MCommandBuilder*	GetCommandBuilder()				{ return m_pCommandBuilder; }
	MPacketCrypter*		GetCrypter()					{ return &m_PacketCrypter; }

	MCommandCommunicator* GetDirectConnection()			{ return m_pDirectConnection; }
	void SetDirectConnection(MCommandCommunicator* pDC)	{ m_pDirectConnection = pDC; }
	DWORD GetUserContext() const						{ return m_dwUserContext; }
	void SetUserContext(DWORD dwContext)				{ m_dwUserContext = dwContext; }

	char* GetIPString()		{ return m_szIP; }
	DWORD GetIP() const		{ return m_dwIP; }
	int GetPort() const		{ return m_nPort; }
	void SetAddress(const char* pszIP, int nPort) {
		strcpy_safe(m_szIP, pszIP);
#pragma warning(push)
#pragma warning(disable: 4996)
		m_dwIP = inet_addr(m_szIP);
#pragma warning(pop)
		m_nPort = nPort;
	}
	void SetAllowed(bool bAllowed)	{ m_bAllowed = bAllowed; }
	bool IsAllowed() const			{ return m_bAllowed; }
};


class MPacketInfo {
public:
	MCommObject*		m_pCommObj;
	MPacketHeader*		m_pPacket;

	MPacketInfo(MCommObject* pCommObj, MPacketHeader* pPacket) { m_pCommObj = pCommObj, m_pPacket = pPacket; }
};
typedef list<MPacketInfo*>			MPacketInfoList;
typedef MPacketInfoList::iterator	MPacketInfoListItor;


/// 커맨드 기반의 통신을 목적으로한 커뮤니케이터
class MCommandCommunicator{
protected:
	MCommandManager	m_CommandManager;		///< 커맨드 매니저

	MUID			m_This;					///< 자기 커뮤니케이터 UID
	MUID			m_DefaultReceiver;		///< 커맨드를 파싱할때 기본이 되는 타겟 커뮤니케이터 UID

protected:
	/// Low-Level Command Transfer Function. 나중에 모아두었다가 블럭 전송등이 가능하게 해줄 수 있다.
	virtual void SendCommand(MCommand* pCommand)=0;
	/// Low-Level Command Transfer Function. 나중에 모아두었다가 블럭 전송등이 가능하게 해줄 수 있다.
	virtual void ReceiveCommand(MCommand* pCommand);

	/// 초기 커맨드 등록할때 불리는 함수
	virtual void OnRegisterCommand(MCommandManager* pCommandManager);
	/// 커뮤니케이터가 커맨드를 처리하기 위해 불리는 함수
	virtual bool OnCommand(MCommand* pCommand);
	/// 커뮤니케이터 루프 전 준비
	virtual void OnPrepareRun(void);
	/// 커맨드를 처리하기 전에
	virtual void OnPrepareCommand(MCommand* pCommand);
	/// 커뮤니케이터 루프
	virtual void OnRun(void);

	/// Post()되는 기본 리시버 커뮤니케이터 설정
	void SetDefaultReceiver(MUID Receiver);
public:
	MCommandCommunicator(void);
	virtual ~MCommandCommunicator(void);

	/// 초기화
	bool Create(void);
	/// 해제
	void Destroy(void);

	/// 다른 커뮤티케이터로 연결 설정
	/// @param	pAllocUID	자기 Communicator가 배정받은 UID
	/// @return				에러 코드 (MErrorTable.h 참조)
	virtual int Connect(MCommObject* pCommObj)=0;
	/// 커넥션이 이루어진 경우
	virtual int OnConnected(MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp, MCommObject* pCommObj);
	/// 연결 해제
	virtual void Disconnect(MUID uid)=0;

	/// 커맨드 입력
	virtual bool Post(MCommand* pCommand);
	/// 스트링으로 커맨드 입력
	virtual bool Post(char* szErrMsg, int nErrMsgCount, const char* szCommand);

	virtual MCommand* GetCommandSafe();

	/// 실행 ( 커맨드 전송및 처리 )
	void Run(void);

	/// 커맨드 매니져 얻기
	MCommandManager* GetCommandManager(void){
		return &m_CommandManager;
	}
	MCommand* CreateCommand(int nCmdID, const MUID& TargetUID);

	enum _LogLevel	{ LOG_DEBUG = 1, LOG_FILE = 2, LOG_PROG = 4, LOG_ALL = 7,  };
	/// 디폴트 로그 출력 ( 로그 서버로 리다이렉션할 수 있게 해준다. )
	virtual void Log(unsigned int nLogLevel, const char* szLog){}
	void LOG(unsigned int nLogLevel, const char *pFormat,...);

	/// 자신의 UID를 얻어낸다.
	MUID GetUID(void){ return m_This; }
	MCommand* MakeCmdFromSaneTunnelingBlob(const MUID& Sender, const MUID& Receiver, const void* pBlob, size_t Size);

#ifdef _CMD_PROFILE
	MCommandProfiler		m_CommandProfiler;
#endif

	MCommand* BlobToCommand(const void* Data, size_t Size);
	MCommand* BlobToCommand(MCmdParamBlob * Blob);
};

MCmdParamBlob* CommandToBlob(MCommand& Command);
int CalcPacketSize(MCommand* pCmd);

bool MakeSaneTunnelingCommandBlob(MCommand* pWrappingCmd, MCommand* pSrcCmd);

#endif
