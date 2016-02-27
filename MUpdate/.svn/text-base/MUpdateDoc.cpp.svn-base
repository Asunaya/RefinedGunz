// MUpdateDoc.cpp : CMUpdateDoc 클래스의 구현
//

#include "stdafx.h"
#include "MUpdate.h"

#include "MUpdateDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMUpdateDoc

IMPLEMENT_DYNCREATE(CMUpdateDoc, CDocument)

BEGIN_MESSAGE_MAP(CMUpdateDoc, CDocument)
END_MESSAGE_MAP()


// CMUpdateDoc 생성/소멸

CMUpdateDoc::CMUpdateDoc()
{
	// TODO: 여기에 일회성 생성 코드를 추가합니다.

}

CMUpdateDoc::~CMUpdateDoc()
{
}

BOOL CMUpdateDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 여기에 다시 초기화 코드를 추가합니다.
	// SDI 문서는 이 문서를 다시 사용합니다.

	return TRUE;
}




// CMUpdateDoc serialization

void CMUpdateDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 여기에 저장 코드를 추가합니다.
	}
	else
	{
		// TODO: 여기에 로딩 코드를 추가합니다.
	}
}


// CMUpdateDoc 진단

#ifdef _DEBUG
void CMUpdateDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMUpdateDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CMUpdateDoc 명령
