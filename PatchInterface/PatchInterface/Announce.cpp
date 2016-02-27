// Announce.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "PatchInterface.h"
#include "Announce.h"


// CAnnounce 대화 상자입니다.

IMPLEMENT_DYNCREATE(CAnnounce, CDHtmlDialog)

CAnnounce::CAnnounce(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CAnnounce::IDD, CAnnounce::IDH, pParent)
{
}

CAnnounce::~CAnnounce()
{
}

void CAnnounce::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BOOL CAnnounce::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

BEGIN_MESSAGE_MAP(CAnnounce, CDHtmlDialog)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CAnnounce)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()



// CAnnounce 메시지 처리기입니다.

HRESULT CAnnounce::OnButtonOK(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

HRESULT CAnnounce::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}
