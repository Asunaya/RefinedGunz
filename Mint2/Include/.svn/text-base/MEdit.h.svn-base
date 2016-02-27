#ifndef MEDIT_H
#define MEDIT_H

#pragma warning(disable:4786)

#include "MWidget.h"
#include <list>
#include "MLookNFeel.h"

using namespace std;

class MEdit;

/// Edit의 Draw 코드가 있는 클래스, 이 클래스를 상속받아서 커스텀룩을 가질 수 있다.
class MEditLook{
protected:
	bool		m_bCustomLook;
public:
	virtual void OnFrameDraw(MEdit* pEdit, MDrawContext* pDC);
	virtual void OnTextDraw(MEdit* pEdit, MDrawContext* pDC, bool bShowLanguageTab);

	virtual void OnDraw(MEdit* pEdit, MDrawContext* pDC, bool bShowLanguageTab = true);
	virtual MRECT GetClientRect(MEdit* pEdit, MRECT& r);

	void SetCustomLook(bool b) {
		m_bCustomLook = b;
	}
	bool GetCustomLook() const{
		return m_bCustomLook;
	}
};


/// Edit
class MEdit : public MWidget{
protected:
//	MPOINT		m_TextOffset;
	bool		m_bMouseOver;
	MCOLOR		m_TextColor;
	int			m_nMaxLength;
	char*		m_pBuffer;
	char		m_szIMECompositionString[MIMECOMPOSITIONSTRING_LENGTH];
	bool		m_bPassword;
	int			m_nCaretPos;	
	int			m_nStartPos;
	MWidget*	m_pTabHandler;

public:
	int			m_nSelectionRange;	//< [m_nCaretPos, m_nCaretPos+m_nSelectionRange]
protected:
	list<char*>				m_History;			///< 문자열 히스토리
	list<char*>::iterator	m_nCurrentHistory;	///< 문자열 히스토리중 현재 문자열

	// Look & Feel
	DECLARE_LOOK(MEditLook)
	DECLARE_LOOK_CLIENT()

public:
	bool		m_bSupportHistory;

protected:
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);
	
	/// 키 입력이 들어왔을 때 키값에 따라 처리할 수 있다.
	/// @return	true를 리턴하면 InputFilter에서 처리됐음을 의미한다. \n
	///			false를 리턴하면 Edit에 스트링을 추가함을 의미한다.
	virtual bool InputFilterKey(int nKey);	// MWM_KEYDOWN
	virtual bool InputFilterChar(int nKey);	// MWM_CHAR

	virtual void OnSetFocus(void);
	virtual void OnReleaseFocus(void);

	void Initialize(int nMaxLength, const char* szName);
public:
	MEdit(int nMaxLength, const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	MEdit(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL );
	virtual ~MEdit();

	virtual void OnHide(void);

	//void SetTextOffset(MPOINT p);
	//void SetTextColor(MCOLOR color);
	//MCOLOR GetTextColor(void);

	virtual void SetText(const char* szText);
	virtual const char* GetText(void);
	void AddText(const char* szText);

	int MoveCaretHome(void);
	int MoveCaretEnd(void);
	int MoveCaretPrev(void);
	int MoveCaretNext(void);

	void SetMaxLength(int nMaxLength);
	int GetMaxLength(void);
	const char* GetCompositionString(void);
	int GetCarretPos(void);
	int GetStartPos(void) { return m_nStartPos; }
	bool SetStartPos(int nStartPos);
	int GetPosByScreen(int x);

	void SetPasswordField(bool bPassword);
	bool IsPasswordField(void);

	bool GetClipboard(char* szText, int nSize);
	bool SetClipboard(const char* szText);

	void AddHistory(const char* szText);

	MWidget* GetTabHandler()				{ return m_pTabHandler; }
	void SetTabHandler(MWidget* pWidget)	{ m_pTabHandler = pWidget; }

#define MINT_EDIT	"Edit"
	virtual const char* GetClassName(void){ return MINT_EDIT; }
};

#define MEDIT_KEYDOWN_MSG		"keydown"	///< 키를 입력했을때
#define MEDIT_CHAR_MSG			"char"		///< 키를 입력했을때
#define MEDIT_ENTER_VALUE		"entered"	///< Enter키를 입력했을때
#define MEDIT_ESC_VALUE			"esc"		///< ESC키를 입력했을때



#define MEDIT_BLINK_TIME		400		///< 커서가 깜밖이는 딜레이, 단위는 mili-second
#endif