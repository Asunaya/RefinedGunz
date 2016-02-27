#pragma once


// CAnnounce 대화 상자입니다.

class CAnnounce : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CAnnounce)

public:
	CAnnounce(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CAnnounce();
// 재정의
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// 대화 상자 데이터입니다.
	enum { IDD = IDD_PATCHINTERFACE_DIALOG, IDH = IDR_HTML_ANNOUNCE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
