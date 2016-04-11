#ifndef MCOMMAND_H
#define MCOMMAND_H

#include "MUID.h"

#include <vector>
#include <list>
#include <set>
#include <deque>

using namespace std;

#include "MCommandParameter.h"
#include "mempool.h"

class MCommandManager;

// Command Description Flag
#define MCDT_NOTINITIALIZED		0	///< 초기화가 안된 상태
#define MCDT_MACHINE2MACHINE	1	///< 머신에서 머신으로 전달되는 커맨드
#define MCDT_LOCAL				2	///< 로컬로 전달되는 커맨드
#define MCDT_TICKSYNC			4	///< 틱싱크가 필요한 커맨드, 다음틱에 실행된다.
#define MCDT_TICKASYNC			8	///< 틱싱크가 필요 없는 커맨드, 곧바로 실행된다.
#define MCDT_USER				16	///< 유저 레벨 커맨드
#define MCDT_ADMIN				32	///< 운영자 레벨 커맨드
#define MCDT_PEER2PEER			64	///< Peer에서 Peer로 전달되는 커맨드, MACHINE2MACHINE와는 따로 구분한다.

#define MCCT_NON_ENCRYPTED		128 ///< 암호화하지 않는 커맨드
#define MCCT_HSHIELD_ENCRYPTED	256 ///< 핵실드 암호화하는 커맨드(아직 미구현) MCCT_NON_ENCRYPTED와 공존할 수 없다.

#define MAX_COMMAND_PARAMS		255	///< 커맨드는 최고 255개까지 파라메타를 가질 수 있다.

/// 커맨드의 속성을 정의
/// - MCommandDesc::m_nID 와 같은 ID를 가진 MCommand는 MCommandDesc::m_ParamDescs에 정의된 파라미터를 가지게 된다.
class MCommandDesc{
protected:
	int			m_nID;					///< Command ID
	char		m_szName[256];			///< Name for Parsing
	char		m_szDescription[256];	///< Description of Command
	int			m_nFlag;				///< Command Description Flag

	vector<MCommandParameterDesc*>	m_ParamDescs;	///< Parameters
public:
	/// @param nID				커맨드 ID
	/// @param szName			커맨드 이름
	/// @param szDescription	커맨드 설명
	/// @param nFlag			커맨드 플래그, MCDT_NOTINITIALIZED
	MCommandDesc(int nID, const char* szName, const char* szDescription, int nFlag);
	virtual ~MCommandDesc(void);

	/// MCommandParameterDesc 추가
	void AddParamDesc(MCommandParameterDesc* pParamDesc);

	/// 플래그 검사
	bool IsFlag(int nFlag) const;
	/// 아이디 얻기
	int GetID(void) const { return m_nID; }
	/// 이름 얻기
	const char* GetName(void) const { return m_szName; }
	/// 설명 얻기
	const char* GetDescription(void) const { return m_szDescription; }
	/// Parameter Description 얻기
	MCommandParameterDesc* GetParameterDesc(int i) const {
		if(i<0 || i>=(int)m_ParamDescs.size()) return NULL;
		return m_ParamDescs[i];
	}
	/// Parameter Description 갯수 얻기
	int GetParameterDescCount(void) const {
		return (int)m_ParamDescs.size();
	}
	MCommandParameterType GetParameterType(int i) const
	{
		if(i<0 || i>=(int)m_ParamDescs.size()) return MPT_END;
		return m_ParamDescs[i]->GetType();
	}
	/// 자기복제 함수 - 버드 테스트에서만 사용한다.
	MCommandDesc* Clone();
};



/// 머신과 머신 혹은 로컬에 전달되는 커멘드
class MCommand : public CMemPool<MCommand> 
{
public:
	MUID						m_Sender;				///< 메세지를 보내는 머신(혹은 오브젝트) UID
	MUID						m_Receiver;				///< 메세지를 받는 머신(혹은 오브젝트) UID
	const MCommandDesc*			m_pCommandDesc;			///< 해당 커맨드의 Description
	vector<MCommandParameter*>	m_Params;				///< 파라미터
	unsigned char				m_nSerialNumber;		///< 커맨드의 무결성을 체크하기 위한 일련번호
	void ClearParam(int i);

protected:
	/// 초기화
	void Reset(void);
	/// 파라미터 초기화
	void ClearParam(void);

public:
	MCommand(void);
	MCommand(const MCommandDesc* pCommandDesc, MUID Receiver, MUID Sender);
	MCommand::MCommand(int nID, MUID Sender, MUID Receiver, MCommandManager* pCommandManager);
	virtual ~MCommand(void);

	/// MCommandDesc으로 ID 지정
	void SetID(const MCommandDesc* pCommandDesc);
	/// ID 지정
	void MCommand::SetID(int nID, MCommandManager* pCommandManager);
	/// ID 얻기
	int GetID(void) const { return m_pCommandDesc->GetID(); }
	/// 설명 얻기
	const char* GetDescription(void){ return m_pCommandDesc->GetDescription(); }

	/// 파라미터 추가
	bool AddParameter(MCommandParameter* pParam);
	/// 파라미터 갯수 얻기
	int GetParameterCount(void) const;
	/// 파라미터 얻기
	MCommandParameter* GetParameter(int i) const;

	/// 인덱스로 파라미터를 얻는다. 파라미터 타입을 정확하게 명시하지 않으면 false를 리턴한다.
	/// @brief 파라미터 얻기
	/// @param pValue	[out] 파라미터 값
	/// @param i		[in] 파라미터 인덱스
	/// @param t		[in] 파라미터 타입, 정확한 타입을 명시해줘야 한다.
	bool GetParameter(void* pValue, int i, MCommandParameterType t, int nBufferSize=-1) const;

	MUID GetSenderUID(void){ return m_Sender; }
	void SetSenderUID(MUID &uid) { m_Sender = uid; }
	MUID GetReceiverUID(void){ return m_Receiver; }

	bool IsLocalCommand(void){ return (m_Sender==m_Receiver); }

	/// 같은 내용을 가진 커맨드 복제
	MCommand* Clone(void) const;

	/// Description에 맞게끔 설정되었는가?
	bool CheckRule(void);	

	/// 커맨드 메모리 블럭 데이터로 얻어내기
	/// @param pData	[out] 커맨드 데이터 블럭
	/// @param nSize	[in] 커맨드 데이터 블럭 최대 사이즈
	/// @return			블럭 사이즈
	int GetData(char* pData, int nSize);
	/// 커맨드 메모리 블럭 데이터로부터 저장
	/// @param pData	[in] 커맨드 데이터 블럭
	/// @param pPM		[in] 커맨드 매니져(MCommandDesc를 enum할 수 있다.)
	/// @return			성공 여부
	bool SetData(char* pData, MCommandManager* pCM, unsigned short nDataLen=USHRT_MAX, bool ReadSerial = true);

	int GetSize();
};


class MCommandSNChecker
{
private:
	int				m_nCapacity;
	deque<int>		m_SNQueue;
	set<int>		m_SNSet;
public:
	MCommandSNChecker();
	~MCommandSNChecker();
	void InitCapacity(int nCapacity);
	bool CheckValidate(int nSerialNumber);
};



// 라인수 줄이기 위한 매크로
#define NEWCMD(_ID)		(new MCommand(_ID))
#define MKCMD(_C, _ID)									{ _C = NEWCMD(_ID); }
#define MKCMD1(_C, _ID, _P0)							{ _C = NEWCMD(_ID); _C->AddParameter(new _P0); }
#define MKCMD2(_C, _ID, _P0, _P1)						{ _C = NEWCMD(_ID); _C->AddParameter(new _P0); _C->AddParameter(new _P1); }
#define MKCMD3(_C, _ID, _P0, _P1, _P2)					{ _C = NEWCMD(_ID); _C->AddParameter(new _P0); _C->AddParameter(new _P1); _C->AddParameter(new _P2); }
#define MKCMD4(_C, _ID, _P0, _P1, _P2, _P3)				{ _C = NEWCMD(_ID); _C->AddParameter(new _P0); _C->AddParameter(new _P1); _C->AddParameter(new _P2); _C->AddParameter(new _P3); }
#define MKCMD5(_C, _ID, _P0, _P1, _P2, _P3, _P4)		{ _C = NEWCMD(_ID); _C->AddParameter(new _P0); _C->AddParameter(new _P1); _C->AddParameter(new _P2); _C->AddParameter(new _P3); _C->AddParameter(new _P4); }
#define MKCMD6(_C, _ID, _P0, _P1, _P2, _P3, _P4, _P5)	{ _C = NEWCMD(_ID); _C->AddParameter(new _P0); _C->AddParameter(new _P1); _C->AddParameter(new _P2); _C->AddParameter(new _P3); _C->AddParameter(new _P4); _C->AddParameter(new _P5); }


// Short Name
typedef MCommand				MCmd;
typedef MCommandDesc			MCmdDesc;

#endif