#include "stdafx.h"
#include "MatchAgent.h"
#include "MBMatchAgent.h"
#include "MatchAgentDoc.h"
#include "OutputView.h"
#include <atltime.h>
#include "MErrorTable.h"
#include "CommandLogView.h"
#include "MSharedCommandTable.h"
#include "MDebug.h"


bool MBMatchAgent::OnCreate(void)
{
	CMatchAgentApp* pApp = (CMatchAgentApp*)AfxGetApp();
	return true;
}

void MBMatchAgent::OnDestroy(void)
{
}

void MBMatchAgent::OnPrepareCommand(MCommand* pCommand)
{
#ifndef _DEBUG
	return;		// _DEBUG 버전아니면 리턴
#endif

	// 커맨드 로그 남기기
	if(m_pCmdLogView==NULL) return;

	if(pCommand->GetID() == MC_AGENT_TUNNELING_TCP) return;
	if(pCommand->GetID() == MC_AGENT_TUNNELING_UDP) return;

	CCommandLogView::CCommandType t=CCommandLogView::CCT_LOCAL;
	if(pCommand->m_pCommandDesc->IsFlag(MCDT_LOCAL)==true) t = CCommandLogView::CCT_LOCAL;
	else if(pCommand->m_Sender==m_This) t = CCommandLogView::CCT_SEND;
	else if(pCommand->m_Receiver==m_This) t = CCommandLogView::CCT_RECEIVE;
//	else _ASSERT(FALSE);
	m_pCmdLogView->AddCommand(GetGlobalClockCount(), t, pCommand);
}

bool MBMatchAgent::OnCommand(MCommand* pCommand)
{
	return MMatchAgent::OnCommand(pCommand);
}

MBMatchAgent::MBMatchAgent(COutputView* pView)
{
	m_pView = pView;
	m_pCmdLogView = NULL;
}

void MBMatchAgent::Log(unsigned int nLogLevel, const char* szLog)
{
	CTime theTime = CTime::GetCurrentTime();
	CString strTime = theTime.Format( "[%c] " );

//#ifdef _DEBUG	// mlog 자체가 _DEBUG 에서 출력
//	if (nLogLevel || LOG_DEBUG)
//	{
//		OutputDebugString((LPCTSTR)strTime);
//		OutputDebugString(szLog);
//		OutputDebugString("\n");
//	}
//#endif

	if (nLogLevel || LOG_FILE)
	{
		char szTemp[1024];
		strcpy(szTemp, strTime);
		strcat(szTemp, szLog);
		mlog(szTemp);

		#ifdef _DEBUG
		OutputDebugString("\n");
		#endif
	}

	if (nLogLevel || LOG_PROG)
	{
		if(m_pView==NULL) return;
		m_pView->AddString(strTime, TIME_COLOR, false);
		m_pView->AddString(szLog, RGB(0,0,0));
	}
}
