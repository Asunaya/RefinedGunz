#ifndef MCOMMANDMANAGER_H
#define MCOMMANDMANAGER_H

#include "MCommand.h"
#include "MCommandParameter.h"
#include <map>
#include <string>
using namespace std;

//typedef vector<MCommandDesc*>	MCommandDescVector;	///< Command Description Vector(ID및 인덱스로 검색을 자주 하므로, 정렬된 벡터를 이용한다.)
typedef map<int, MCommandDesc*>	MCommandDescMap;	///< Command Description Map
typedef list<MCommand*>			MCommandList;		///< Command List
typedef map<string, string>		MCommandAliasMap;		///< Alias


/// 커맨드 매니져
/// - 커맨드 디스크립션 관리
/// - 커맨드 큐 관리
class MCommandManager{
protected:
	//MCommandDescVector	m_CommandDescs;			///< Command Description List
	MCommandDescMap		m_CommandDescs;
	MCommandList		m_CommandQueue;			///< Posted Command List
	MCommandAliasMap	m_CommandAlias;			///< Aliases
protected:
	void InitializeCommandDesc(void);
	void InitializeCommandMemPool();
	void FinalizeCommandMemPool();
public:
	MCommandManager(void);
	virtual ~MCommandManager(void);

	void Initialize(void);

	int GetCommandDescCount(void) const;
	int GetCommandQueueCount(void) const;
	MCommandDesc* GetCommandDesc(int i);
	MCommandDesc* GetCommandDescByID(int nID);
	void AssignDescs(MCommandManager* pTarCM);

	/// Command Description 추가
	void AddCommandDesc(MCommandDesc* pCD);

	/// 커맨드 Posting
	bool Post(MCommand* pNew);

	/// 가장 먼저 추가된 커맨드를 읽어들임
	MCommand* GetCommand(void);
	/// 가장 먼저 추가된 커맨드를 CommandManager에서 지우지 않고 읽어들임
	MCommand* PeekCommand(void);

	/// Command Description의 문법 설명 스트링 얻어내기
	void GetSyntax(char* szSyntax, const MCommandDesc* pCD);
	/// 메세지를 파싱해서 커맨드로 만들기
	/// @param pCmd				[out] 만들어질 커맨드
	/// @param szErrMsg			[out] 에러 메세지
	/// @param nErrMsgMaxLength	[out] 에러 메세지 크기
	/// @param szMsg			[in]  파싱할 원본 메세지
	bool ParseMessage(MCommand* pCmd, char* szErrMsg, int nErrMsgMaxLength, const char* szMsg);

	/// Alias를 추가한다.
	/// @param szName			[in] Alias 이름
	/// @param szText			[in] Alias 내용
	void AddAlias(string szName, string szText);
};

// Post Command Macro For Convenience
#define POSTCMD0(_ID)									{ MCommand* pC=NEWCMD(_ID); ASPostCommand(pC); }
#define POSTCMD1(_ID, _P0)								{ MCommand* pC=NEWCMD(_ID); pC->AP(_P0); ASPostCommand(pC); }
#define POSTCMD2(_ID, _P0, _P1)							{ MCommand* pC=NEWCMD(_ID); pC->AP(_P0); pC->AP(_P1); ASPostCommand(pC); }
#define POSTCMD3(_ID, _P0, _P1, _P2)					{ MCommand* pC=NEWCMD(_ID); pC->AP(_P0); pC->AP(_P1); pC->AP(_P2); ASPostCommand(pC); }
#define POSTCMD4(_ID, _P0, _P1, _P2, _P3)				{ MCommand* pC=NEWCMD(_ID); pC->AP(_P0); pC->AP(_P1); pC->AP(_P2); pC->AP(_P3); ASPostCommand(pC); }
#define POSTCMD5(_ID, _P0, _P1, _P2, _P3, _P4)			{ MCommand* pC=NEWCMD(_ID); pC->AP(_P0); pC->AP(_P1); pC->AP(_P2); pC->AP(_P3); pC->AP(_P4); ASPostCommand(pC); }
#define POSTCMD6(_ID, _P0, _P1, _P2, _P3, _P4, _P5)		{ MCommand* pC=NEWCMD(_ID); pC->AP(_P0); pC->AP(_P1); pC->AP(_P2); pC->AP(_P3); pC->AP(_P4); pC->AP(_P5); ASPostCommand(pC); }

#define ENPOSTCMD0(_ID)									{ MCommand* pC=NEWCMD(_ID); ASEnPostCommand(pC); }
#define ENPOSTCMD1(_ID, _P0)							{ MCommand* pC=NEWCMD(_ID); pC->AP(_P0); ASEnPostCommand(pC); }
#define ENPOSTCMD2(_ID, _P0, _P1)						{ MCommand* pC=NEWCMD(_ID); pC->AP(_P0); pC->AP(_P1); ASEnPostCommand(pC); }
#define ENPOSTCMD3(_ID, _P0, _P1, _P2)					{ MCommand* pC=NEWCMD(_ID); pC->AP(_P0); pC->AP(_P1); pC->AP(_P2); ASEnPostCommand(pC); }
#define ENPOSTCMD4(_ID, _P0, _P1, _P2, _P3)				{ MCommand* pC=NEWCMD(_ID); pC->AP(_P0); pC->AP(_P1); pC->AP(_P2); pC->AP(_P3); ASPostCommand(pC); }
#define ENPOSTCMD5(_ID, _P0, _P1, _P2, _P3, _P4)		{ MCommand* pC=NEWCMD(_ID); pC->AP(_P0); pC->AP(_P1); pC->AP(_P2); pC->AP(_P3); pC->AP(_P4); ASEnPostCommand(pC); }
#define ENPOSTCMD6(_ID, _P0, _P1, _P2, _P3, _P4, _P5)	{ MCommand* pC=NEWCMD(_ID); pC->AP(_P0); pC->AP(_P1); pC->AP(_P2); pC->AP(_P3); pC->AP(_P4); pC->AP(_P5); ASEnPostCommand(pC); }

// Command Description Add Macro For Convenience
#define BEGIN_CMD_DESC(_pCommandManager)	{ MCommandDesc* pCD4m; MCommandManager* pCM4m = _pCommandManager;
#define END_CMD_DESC()						}
#define C(_a, _b, _c, _d)					{ pCD4m = new MCommandDesc(_a, _b, _c, _d); pCM4m->AddCommandDesc(pCD4m); }
#define P(_a, _b)							{ pCD4m->AddParamDesc(new MCommandParameterDesc(_a, _b)); }
#define CA(_a, _b)							{ pCM4m->AddAlias(_a, _b); }
#define P_MINMAX(_a, _b, _min, _max)		{ MCommandParameterDesc* pNewDesc = new MCommandParameterDesc(_a, _b);  \
	                                          pNewDesc->AddCondition(new MCommandParamConditionMinMax(_min, _max)); \
											  pCD4m->AddParamDesc(pNewDesc); }


#endif