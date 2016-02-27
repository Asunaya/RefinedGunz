#ifndef MPANEL_H
#define MPANEL_H

#include "MWidget.h"
#include "MDrawContext.h"
#include "MLookNFeel.h"

class MPanel;

/// Panel의 Draw 코드가 있는 클래스, 이 클래스를 상속받아서 커스텀룩을 가질 수 있다.
class MPanelLook{
protected:
	virtual void OnFrameDraw(MPanel* pPanel, MDrawContext* pDC);
public:
	
	virtual void OnDraw(MPanel* pLabel, MDrawContext* pDC);
	virtual MRECT GetClientRect(MPanel* pLabel, MRECT& r);
};

/// Panel의 주위에 그리는 선 타입
enum MBorderStyle {
	MBS_NONE = 0,		///< 선 없음
	MBS_SINGLE,			///< single-line 선
};

/// Panel
class MPanel : public MWidget{
protected:
	DECLARE_LOOK(MPanelLook)
	DECLARE_LOOK_CLIENT()
protected:
	MBorderStyle	m_nBorderStyle;
	MCOLOR			m_BorderColor;
	MCOLOR			m_BackgroundColor;
public:
	MPanel(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);

	/// 바탕색 컬러 지정
	void SetBackgroundColor(MCOLOR color);
	/// 바탕색 컬러 얻기
	MCOLOR GetBackgroundColor(void);

	/// 외곽선 컬러 지정
	void SetBorderColor(MCOLOR color);
	/// 외곽선 컬러 얻기
	MCOLOR GetBorderColor(void);
	/// 외곽선 스타일 지정
	void SetBorderStyle(MBorderStyle style);
	/// 외곽선 스타일 얻기
	MBorderStyle GetBorderStyle();


#define MINT_PANEL	"Panel"
	virtual const char* GetClassName(void){ return MINT_PANEL; }
};


#endif