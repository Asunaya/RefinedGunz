// MUpdateView.h : iCMUpdateView 클래스의 인터페이스
//


#pragma once


class CMUpdateView : public CHtmlView
{
protected: // serialization에서만 만들어집니다.
	CMUpdateView();
	DECLARE_DYNCREATE(CMUpdateView)

// 특성
public:
	CMUpdateDoc* GetDocument() const;

// 작업
public:

// 재정의
	public:
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // 생성 후 처음 호출되었습니다.

// 구현
public:
	virtual ~CMUpdateView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 메시지 맵 함수를 생성했습니다.
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // MUpdateView.cpp의 디버그 버전
inline CMUpdateDoc* CMUpdateView::GetDocument() const
   { return reinterpret_cast<CMUpdateDoc*>(m_pDocument); }
#endif

