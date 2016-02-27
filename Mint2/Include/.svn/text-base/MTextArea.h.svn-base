#ifndef MTEXTAREA_H
#define MTEXTAREA_H

#pragma warning(disable:4786)

#include <list>
#include <string>
#include "MWidget.h"
#include "MLookNFeel.h"
#include "MScrollBar.h"

using namespace std;

class MTextArea;
class MScrollBar;

#define MTEXTAREA_DEFAULT_TEXT_COLOR		MCOLOR(224,224,224)

class MTextAreaLook{
public:
	virtual void OnDraw(MTextArea* pTextArea, MDrawContext* pDC);
	virtual MRECT GetClientRect(MTextArea* pTextArea, MRECT& r);

private:
	virtual void OnFrameDraw(MTextArea* pTextArea, MDrawContext* pDC);
	virtual void OnTextDraw(MTextArea* pTextArea, MDrawContext* pDC);
	virtual void OnTextDraw_WordWrap(MTextArea* pTextArea, MDrawContext* pDC);
};

struct MLineItem {
	MLineItem(MCOLOR _color,string &_text) {
		color=_color;
		text=_text;
	}
	MLineItem(string &_text) {
		color=MTEXTAREA_DEFAULT_TEXT_COLOR;
		text=_text;
	}
	MCOLOR color;
	string text;
};

typedef list<MLineItem> MLINELIST;
typedef list<MLineItem>::iterator MLINELISTITERATOR;


class MTextArea : public MWidget{
	friend MTextAreaLook;
protected:
	bool		m_bScrollBarEnable;
	int			m_nIndentation;		// 행이바뀔시 들여쓰는 픽셀수
	bool		m_bWordWrap;
	bool		m_bColorSupport;
	MPOINT		m_TextOffset;		// 이게 뭘까요 ?  쓰는데가 없어보이는데...
	bool		m_bMouseOver;
	MCOLOR		m_TextColor;
	int			m_nMaxLen;
	char		m_szIMECompositionString[MIMECOMPOSITIONSTRING_LENGTH];
	bool		m_bEditable;
//	char*		m_pBuffer;			//Text Buffer
	int			m_nStartLine;
	int			m_nStartLineSkipLine;
	
	int			m_nCurrentSize;
	bool		m_bVerticalMoving;
	int			m_nVerticalMoveAxis;

	int			m_nLineHeight;
	
	MPOINT		m_CaretPos;		// 전체에서 캐럿 위치. y번째 라인 x번째 글자
//	MPOINT		m_LineCaretPos;	// 한라인 안에서의 캐럿 위치
	bool		m_bCaretFirst;	// 한라인 안에서 줄바뀜에 캐럿이 있을때 앞인지 뒤인지 플래그

	MLINELIST			m_Lines;
	MLINELISTITERATOR	m_CurrentLine;

	MScrollBar*		m_pScrollBar;

	// Look & Feel
	DECLARE_LOOK(MTextAreaLook)
	DECLARE_LOOK_CLIENT()

protected:
	virtual void OnSize(int w, int h);
	virtual bool OnCommand(MWidget* pWindow, const char* szMessage);
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);
	virtual void OnSetFocus(void);
	virtual void OnReleaseFocus(void);
	
	// 키 입력이 들어왔을 때 키값에 따라 처리할 수 있다.
	// true를 리턴하면 InputFilter에서 처리됐음을 의미한다.
	// false를 리턴하면 Edit에 스트링을 추가함을 의미한다.
	virtual bool InputFilterKey(int nKey,bool bCtrl);	// MWM_KEYDOWN
	virtual bool InputFilterChar(int nKey);	// MWM_CHAR

	bool OnLButtonDown(MPOINT pos);
	void OnScrollBarChanged(int nPos);

	bool MoveLeft(bool bBackspace=false);
	void MoveRight();
	void MoveDown();
	void MoveUp();
	void DeleteCurrent();
	void ScrollDown();
	void ScrollUp();
	void MoveFirst();
	void MoveLast();
	void OnHome();
	void OnEnd();

// 안에서 필요한 함수들
	
	// nPos번째 글자가 어느 위치에 그려져야 하는지 pOut으로 리턴해준다
	// x는 픽셀수 , y 는 라인수,bFirst는 캐럿일경우 앞/뒤플래그
	bool GetCaretPosition(MPOINT *pOut,int nSize,const char* szText,int nPos,bool bFirst);

	// MDrawTextMultiline으로 그릴때 nLine 행에 nX픽셀 위치에 몇번째 글자가 찍힐지 알려주는 펑션
	int GetCharPosition(const char* szText,int nX,int nLine);

	// 현재 캐럿의 위치가 한 라인안에서 다음줄로 wrap때문에 넘어간경우
	// 그줄 마지막이나 다음줄 첫번째가 사실상 같은 위치이다. 이런경우인지 알아본다
	bool IsDoubleCaretPos();	

	void UpdateScrollBar(bool bAdjustStart=false);

	MLINELISTITERATOR GetIterator(int nLine);

public:
	MTextArea(int nMaxLen = 120, const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MTextArea();

#define MINT_TEXTAREA	"TextArea"
	virtual const char* GetClassName(void){ return MINT_TEXTAREA; }

	MPOINT GetCaretPos() { return m_CaretPos; }
	int	GetStartLine() { return m_nStartLine; }

	bool IsScrollBarVisible() { return m_pScrollBar->IsVisible();	}
	int GetScrollBarWidth() { return m_pScrollBar->GetRect().w;	}

	int GetClientWidth();

	int GetLength() { return (int)(m_nCurrentSize+m_Lines.size()); }
	int GetLineCount() { return (int)m_Lines.size(); }

	bool GetText(char *pBuffer,int nBufferSize);
	const char* GetTextLine(int nLine);

	void SetMaxLen(int nMaxLen);
	int	GetMaxLen() { return m_nMaxLen; }

	const char* GetCompositionString(void);

	void SetEditable(bool editable){ m_bEditable = editable; }
	bool GetEditable() { return m_bEditable; }

	void SetScrollBarEnable(bool bEnable) { m_bScrollBarEnable = bEnable; }
	bool GetScrollBarEnable() { return m_bScrollBarEnable; }
	
	void SetTextOffset(MPOINT p);

	void SetIndentation(int nIndentation) { m_nIndentation = nIndentation; }
	
	void SetTextColor(MCOLOR color);
	MCOLOR GetTextColor(void);

	// 모든 텍스트 내용을 지운다.
	void Clear();

	void SetText(const char *szText);

	void AddText(const char *szText,MCOLOR color);
	void AddText(const char *szText);
	
	// 첫줄을 지운다
	void DeleteFirstLine();

	int GetLineHeight( void);
	void SetLineHeight( int nHeight);
};

#define MTEXTAREA_ENTER_VALUE		"entered"
#define MTEXTAREA_ESC_VALUE			"esc"

#endif