// MUpdateView.cpp : CMUpdateView 클래스의 구현
//

#include "stdafx.h"
#include "MUpdate.h"

#include "MUpdateDoc.h"
#include "MUpdateView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMUpdateView

IMPLEMENT_DYNCREATE(CMUpdateView, CHtmlView)

BEGIN_MESSAGE_MAP(CMUpdateView, CHtmlView)
END_MESSAGE_MAP()

// CMUpdateView 생성/소멸

CMUpdateView::CMUpdateView()
{
	// TODO: 여기에 생성 코드를 추가합니다.

}

CMUpdateView::~CMUpdateView()
{
}

BOOL CMUpdateView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	// Window 클래스 또는 스타일을 수정합니다.

	return CHtmlView::PreCreateWindow(cs);
}

void CMUpdateView::OnInitialUpdate()
{
	CHtmlView::OnInitialUpdate();
	Navigate2(_T("http://www.gunz.co.kr/update"),NULL,NULL);
}


// CMUpdateView 진단

#ifdef _DEBUG
void CMUpdateView::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CMUpdateView::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}

CMUpdateDoc* CMUpdateView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMUpdateDoc)));
	return (CMUpdateDoc*)m_pDocument;
}
#endif //_DEBUG


// CMUpdateView 메시지 처리기
