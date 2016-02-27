// ChildFrm.cpp : CChildFrame 클래스의 구현
//
#include "stdafx.h"
#include "MatchAgent.h"

#include "ChildFrm.h"
#include "OutputView.h"
#include "MatchAgentDoc.h"
#include "MatchAgentView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
END_MESSAGE_MAP()


// CChildFrame 생성/소멸

CChildFrame::CChildFrame()
{
	// TODO: 여기에 멤버 초기화 코드를 추가합니다.
}

CChildFrame::~CChildFrame()
{
}


BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서 Window 클래스 또는 스타일을 수정합니다.
	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}


// CChildFrame 진단

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG


// CChildFrame 메시지 처리기

void CChildFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	CMDIChildWnd::OnUpdateFrameTitle(bAddToTitle);
	CView* pView = GetActiveView();
	if(pView!=NULL){
		if(pView->IsKindOf(RUNTIME_CLASS(COutputView))) SetWindowText("Output");
		else if(pView->IsKindOf(RUNTIME_CLASS(CMatchAgentView))) SetWindowText("MatchAgentView");
	}
}
