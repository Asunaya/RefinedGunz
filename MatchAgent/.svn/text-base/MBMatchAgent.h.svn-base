#ifndef MBMATCHAGENT_H
#define MBMATCHAGENT_H

#include "MMatchAgent.h"

class COutputView;
class CCommandLogView;

class MBMatchAgent : public MMatchAgent {
public:
	COutputView*		m_pView;
	CCommandLogView*	m_pCmdLogView;

protected:
	/// Create()호출시에 불리는 함수
	virtual bool OnCreate(void);
	/// Destroy()호출시에 불리는 함수
	virtual void OnDestroy(void);
	/// 커맨드를 처리하기 전에
	virtual void OnPrepareCommand(MCommand* pCommand);
	/// 사용자 커맨드 처리
	virtual bool OnCommand(MCommand* pCommand);
public:
	MBMatchAgent(COutputView* pView=NULL);
	virtual void Log(unsigned int nLogLevel, const char* szLog);
};


#endif
