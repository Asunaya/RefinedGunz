// MatchAgentDoc.h : CMatchAgentDoc 클래스의 인터페이스
//
#pragma once


#include "MCommandManager.h"
#include "MMonitor.h"
#include "MMaster.h"

class COutputView;
class CMatchServerDoc;
class MCommandProcessor;
class MBMatchAgent;
class CCommandLogView;

#define TIME_COLOR	RGB(128, 128, 128)


class CMatchAgentDoc : public CDocument
{
protected: // serialization에서만 만들어집니다.
	CMatchAgentDoc();
	DECLARE_DYNCREATE(CMatchAgentDoc)

protected:
	MBMatchAgent*	m_pMatchAgent;

protected:
	COutputView*		m_pMatchAgentView;
	CCommandLogView*	m_pCmdLogView;
	CFrameWnd*			m_pFrmWndCmdLogView;


// 특성
public:

// 작업
public:

// 재정의
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

	COutputView* GetOutputView(void);

	void PostCommand(const char* szCommand);
	void Run(void);

protected:
	void ProcessLocalCommand(MCommand* pCommand);

// 구현
public:
	virtual ~CMatchAgentDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 메시지 맵 함수를 생성했습니다.
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
};


