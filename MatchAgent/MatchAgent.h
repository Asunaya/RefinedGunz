// MatchAgent.h : MatchAgent 응용 프로그램에 대한 주 헤더 파일
//
#pragma once

#ifndef __AFXWIN_H__
	#error PCH에서 이 파일을 포함하기 전에 'stdafx.h'를 포함하십시오.
#endif

#include "resource.h"       // 주 기호
#include "MZFileSystem.h"


class CSeException;


// CMatchAgentApp:
// 이 클래스의 구현에 대해서는 MatchAgent.cpp을 참조하십시오.
//
class CMatchAgentApp : public CWinApp
{
private:
	bool				m_bShutdown;

public:
	double				m_dFPS;
	double				GetFPS()	{ return m_dFPS; }

public:
	MZFileSystem		m_ZFS;

public:
	CMultiDocTemplate* m_pDocTemplateOutput;
	CMultiDocTemplate* m_pDocTemplateCmdLogView;

public:
	CMatchAgentApp();
	void Shutdown() { m_bShutdown = true; }
	void HeartBeat();
	double UpdateFPS();

// 재정의
public:
	virtual BOOL InitInstance();

// 구현
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
	virtual int Run();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnCmdDebugpeer();
};

extern CMatchAgentApp theApp;
