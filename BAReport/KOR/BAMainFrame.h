#pragma once


// CBAMainFrame 프레임입니다.

class CBAMainFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CBAMainFrame)
public:
	CBAMainFrame();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CBAMainFrame();

protected:
	DECLARE_MESSAGE_MAP()
};


