#ifndef MSCROLLBAR_H
#define MSCROLLBAR_H

#include "GlobalTypes.h"
#include "MWidget.h"
#include "MButton.h"

#include "MLookNFeel.h"

#define MSCROLLBAR_DEFAULT_WIDTH	22
#define MSCROLLBAR_DEFAULT_HEIGHT	100
#define MSCROLLBAR_BUTTON_WIDTH		22
#define MSCROLLBAR_BUTTON_HEIGHT	22
#define MSCROLLBAR_THUMB_WIDTH		22
#define MSCROLLBAR_THUMB_HEIGHT		22

enum MScrollBarTypes{
	MSBT_VERTICAL = 0,
	MSBT_HORIZONTAL = 1
};


class MThumb;
class MScrollBar;
class MArrow;

class MThumbLook{
protected:
public:
	virtual void OnDraw(MThumb* pThumb, MDrawContext* pDC);
	virtual MRECT GetClientRect(MThumb* pThumb, const MRECT& r);
};

class MArrowLook{
protected:
	virtual void OnDrawUpArrow(MDrawContext* pDC, MRECT& r, bool bPressed);
	virtual void OnDrawDownArrow(MDrawContext* pDC, MRECT& r, bool bPressed);
	virtual void OnDrawLeftArrow(MDrawContext* pDC, MRECT& r, bool bPressed);
	virtual void OnDrawRightArrow(MDrawContext* pDC, MRECT& r, bool bPressed);
public:
	virtual void OnDraw(MArrow* pArrow, MDrawContext* pDC);
	virtual MRECT GetClientRect(MArrow* pArrow, MRECT& r);
	virtual MSIZE GetDefaultSize(MArrow* pThumb);
};

class MScrollBarLook{
protected:
public:
	virtual void OnDraw(MScrollBar* pScrollBar, MDrawContext* pDC);
	virtual MRECT GetClientRect(MScrollBar* pScrollBar, const MRECT& r);
};


class MThumb : public MWidget{
	DECLARE_LOOK(MThumbLook)
	DECLARE_LOOK_CLIENT()
protected:
	//virtual void OnDraw(MDrawContext* pDC);
public:
	int		m_nDirection;	// 0: Horizontal, 1: Vertical
public:
	MThumb(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
};

class MArrow : public MButton{
	DECLARE_LOOK(MArrowLook)
//	DECLARE_LOOK_CLIENT(m_pLook)
public:
	int		m_nDirection;	// 0: Up, 1: Down, 2: Left, 3: Right
protected:
	//virtual void OnDraw(MDrawContext* pDC);
public:
	MArrow(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);

	MSIZE GetDefaultSize(void);
};



class MScrollBar : public MWidget{
protected:

	int			m_nMinValue;
	int			m_nMaxValue;
	int			m_nValue;

	u64			m_nPrevThumbRefresh;
	int			m_nThumbRefreshDelay;

	bool		m_bThumbMove;
	int			m_nThumbPos;
	int			m_nThumbDownPos;

	MScrollBarTypes	m_nScrollBarType;

	DECLARE_LOOK(MScrollBarLook)
	DECLARE_LOOK_CLIENT()

protected:
	virtual void OnRun(void);
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);
	virtual void OnSize(int w, int h);

	int GetThumbMoveRange(void);
	int GetMoveRange(void);
	virtual int GetThumbSize(void);
	void RecalcThumbPos(void);
	void RecalcThumbBounds(void);
	//virtual void OnDraw(MDrawContext* pDC);

	void Initialize(MScrollBarTypes t);

public:
	MArrow*		m_pUp;
	MArrow*		m_pDown;
	MThumb*		m_pThumb;

	MScrollBar(const char* szName, MWidget* pParent=NULL, MListener* pListener=NULL, MScrollBarTypes t=MSBT_VERTICAL);
	MScrollBar(MWidget* pParent=NULL, MListener* pListener=NULL, MScrollBarTypes t=MSBT_VERTICAL);
	virtual ~MScrollBar(void);

	void ChangeCustomArrowLook(MArrowLook *pArrowLook);
	void ChangeCustomThumbLook(MThumbLook *pThumbLook);

	void SetMinMax(int nMin, int nMax);
	int GetMin(void){ return m_nMinValue; }
	int GetMax(void){ return m_nMaxValue; }

	void SetValue(int nValue);
	int GetValue(void);

	MScrollBarTypes GetType(void);
	void SetType(MScrollBarTypes t);

	int GetDefaultBreadth(void);	// 기본 너비

	//virtual void OnCommand(MWidget* pWindow, char* szMessage);
#define MINT_SCROLLBAR	"ScrollBar"
	virtual const char* GetClassName(void){ return MINT_SCROLLBAR; }
};


#define MLIST_VALUE_CHANGED		"value changed"		// value가 바뀔때 떨어지는 메세지

#endif
