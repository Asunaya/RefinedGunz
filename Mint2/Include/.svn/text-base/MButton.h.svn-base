#ifndef MBUTTON_H
#define MBUTTON_H

#include "MWidget.h"
#include "MLookNFeel.h"

class MMsgBox;
class MButtonGroup;

/// 버튼 타입
enum MButtonType{
	MBT_NORMAL = 0,		///< 일반 버튼
	MBT_PUSH,			///< 푸쉬 버튼
	MBT_PUSH2,
};

/// 디폴트 키 등록
enum MButtonKeyAssigned{
	MBKA_NONE = 0,		///< 아무키도 등록되어 있지 않은 버튼
	MBKA_ENTER,			///< Enter 키 눌리면 실행되는 버튼
	MBKA_ESC,			///< ESC 키 눌리면 실행되는 버튼
};

class MButton;
/// Button의 Draw 코드가 있는 클래스, 이 클래스를 상속받아서 커스텀룩을 가질 수 있다.
class MButtonLook{
protected:
	bool	m_bWireLook;
protected:
	virtual void OnDownDraw(MButton* pButton, MDrawContext* pDC);
	virtual void OnUpDraw(MButton* pButton, MDrawContext* pDC);
	virtual void OnOverDraw(MButton* pButton, MDrawContext* pDC);
	virtual void OnDisableDraw(MButton* pButton, MDrawContext* pDC);
	virtual void OnCheckBoxDraw(MButton* pButton, MDrawContext* pDC, bool bPushed){};
public:
	virtual void OnDrawText(MButton* pButton, MRECT& r, MDrawContext* pDC);
	virtual void OnDraw(MButton* pButton, MDrawContext* pDC);
	virtual MRECT GetClientRect(MButton* pButton, MRECT& r);
	void SetWireLook(bool b) {	m_bWireLook = b;	}
	bool GetWireLook() const{	return m_bWireLook; }
public:
	MButtonLook():m_bWireLook(false){}
};


/// 버튼 클래스
class MButton : public MWidget{
protected:
	bool		m_bMouseOver;
	bool		m_bLButtonDown;
	bool		m_bRButtonDown;
	MCOLOR		m_TextColor;
	bool		m_bShowText;
	MAlignmentMode	m_AlignmentMode;
	MButtonType		m_Type;
	bool		m_bChecked;				///< MBT_PUSH인 경우
	bool		m_bComboDropped;
	bool		m_bStretch;
	
	MButtonGroup	*m_pButtonGroup;	///< 속해있는 그룹
	int				m_nIndexInGroup;	///< 같은 그룹내에서의 index

public:
	bool		m_bEnableEnter;			///< 포커스 있을때 Enter키로 버튼 누르기 활성화 ( Default : true )
	bool		m_bHighlight;			///< Highlight(&) 지원
	MBitmap*	m_pIcon;				///< 아이콘
	MButtonKeyAssigned	m_nKeyAssigned;	///< Key Assigned
	MMsgBox*	m_pMsgBox;				///< 메세지 박스를 통한 확인 메세지

	MPOINT		m_ClickPos;
	MPOINT		m_LDragStartClickPos;
	int			m_LDragVariationX;
	int			m_LDragVariationY;

private:
	//void DrawButton(MDrawContext* pDC);
protected:
	//virtual void OnDraw(MDrawContext* pDC);
	/// 마우스가 버튼 안에 들어올때 발생하는 이벤트 핸들러
	virtual void OnMouseIn(void);
	/// 마우스가 버튼 밖으로 빠져 나갈때 발생하는 이벤트 핸들러
	virtual void OnMouseOut(void);
	/// 버튼이 눌릴때 발생하는 이벤트 핸들러
	virtual void OnButtonDown(void);
	/// 버튼이 띄어질때 있을때 발생하는 이벤트 핸들러
	virtual void OnButtonUp(void);
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);

	/// 클릭되어졌을때
	virtual void OnButtonClick(void);	

	virtual bool OnShow(void);
	virtual void OnHide(void);
public:
	MButton(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MButton(void);
	/// 버튼 텍스트 컬러 지정
	void SetTextColor(MCOLOR color);
	/// 버튼 텍스트 컬러 얻기
	MCOLOR GetTextColor(void);
	/// 버튼에 텍스트 보여주기
	void ShowText(bool bShow=true);
	virtual bool DefaultCommand(void);

	/// 버튼내 텍스트 정렬 얻기
	MAlignmentMode GetAlignment(void);
	/// 버튼내 텍스트 정렬 지정
	MAlignmentMode SetAlignment(MAlignmentMode am);

	/// 버튼 타입 지정
	void SetType(MButtonType t);
	/// 버튼 타입 얻기
	MButtonType GetType(void);

	/// Push 버튼인 경우 체크
	void SetCheck(bool bCheck);
	/// Push 버튼인 경우 체크값 얻기
	bool GetCheck(void);

	/// 버튼이 눌려져 있는가? ( 마우스로 누르는 상태를 얻어낼 수 있다. )
	bool IsButtonDown(void);	// Async Button Down ?
	/// 마우스가 버튼 위에 올려져 있는가?
	bool IsMouseOver(void);

	/// Confirm Message Box
	void SetConfirmMessageBox(const char* szMessage);

	virtual bool OnCommand(MWidget* pWidget, const char* szMessage);

	int GetLDragVariationX() {
		return m_LDragVariationX;
	}

	int GetLDragVariationY() {
		return m_LDragVariationY;
	}

	DECLARE_LOOK(MButtonLook)
	DECLARE_LOOK_CLIENT()

	void SetComboDropped(bool b) {
		m_bComboDropped = b;
	}
	bool GetComboDropped() const{
		return m_bComboDropped;
	}

	void SetStretch( bool b) {
		m_bStretch = b;
	}

	bool GetStretch( ) const {
		return m_bStretch;
	}

	void SetButtonGroup(MButtonGroup *pGroup);

	int GetIndexInGroup() { 
		return m_nIndexInGroup; 
	}

#define MINT_BUTTON	"Button"
	virtual const char* GetClassName(void){ return MINT_BUTTON; }
};

#define MBTN_CLK_MSG		"click"	///< 버튼이 클릭될때 Listener에게 발생되는 메세지
#define MBTN_RCLK_MSG		"rclick"///< R 버튼이 클릭될때 Listener에게 발생되는 메세지
#define MBTN_DN_MSG			"down"	///< 버튼이 눌릴때
#define MBTN_UP_MSG			"up"	///< 버튼이 띄어질때
#define MBTN_RDN_MSG		"rdown"	///< R버튼이 눌릴때
#define MBTN_RUP_MSG		"rup"	///< R버튼이 띄어질때
#define MBTN_IN_MSG			"in"	///< 버튼 위에 커서가 올려 질때
#define MBTN_OUT_MSG		"out"	///< 버튼 위에 커서가 빠질때 올려 질때


// 버튼 그룹을 구현하는데 필요한 클래스
class MButtonGroup {
	friend MButton;
public:
	MButtonGroup();
	~MButtonGroup();

protected:
	int		m_nCount;
	MButton *m_pPrevious;
};

#endif