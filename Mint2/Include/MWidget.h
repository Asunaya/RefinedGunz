//
// MINT ( MAIET In-house wiNdows sysTem )
//
////////////////////////////////////////////////////////////////////////////////
#ifndef MWIDGET_H
#define MWIDGET_H

#include "CMPtrList.h"
#include "MDrawContext.h"
#include "MTypes.h"
#include "MEvent.h"



class MWidget;

/// 위젯들의 메세지를 받을 수 있는 Listener(Pure Virtual Class)
class MListener{
public:
	/// 메세지를 받기위한 Virtual Function
	/// @param	pWidget		메세지를 보내는 위젯 포인터
	/// @param	szMessage	메세지(스트링)
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) = 0;
};


class MToolTip;
class MResourceMap;

/// 위젯의 Z 순서
enum MZOrder{
	MZ_TOP = 0,		///< 맨 앞으로
	MZ_BOTTOM,		///< 맨 뒤로
};

struct MAnchors
{
	bool m_bLeft;
	bool m_bRight;
	bool m_bTop;
	bool m_bBottom;
	MAnchors(void)	{ m_bLeft = true; m_bRight = false; m_bTop = true; m_bBottom = false; }
	MAnchors(bool bLeft, bool bRight, bool bTop, bool bBottom)
					{ m_bLeft = bLeft; m_bRight = bRight; m_bTop = bTop; m_bBottom = bBottom; }
};

#define MWIDGET_NAME_LENGTH		256			///< 위젯 이름 최대 길이

/// 기본 위젯 클래스
class MWidget : public MListener{
private:
	bool				m_bEnable;			///< Enable
	bool				m_bFocusEnable;		///< Focus Enable

	MListener*			m_pListener;		///< Listener
	MToolTip*			m_pToolTip;			///< 기본 툴팁

	int					m_nAndPos;
	int					m_nAccelerator;		///< Accelerator Key

protected:
	bool				m_bVisible;			///< Visible Flag

	CMPtrList<MWidget>	m_Children;			///< Children Widgets

	MWidget*			m_pParent;			///< Parent Widget
	CMPtrList<MWidget>	m_Exclusive;		///< Exclusive Child Widget Stack ( for Modal Support )

	MCursor*			m_pCursor;			///< Default Cursor of this Widget
	MFont*				m_pFont;			///< Default Font of this Widget

	static MWidget*		m_pCapturedWidget;	///< SetCapture(), ReleaseCapture()
	static MWidget*		m_pFocusedWidget;	///< SetFocus()

	bool				m_bZOrderChangable;	///< Z Order Change by L-Button Click
	bool				m_bResizable;		///< Resizable?

	int					m_nResizeSide;		///< 0: N/A   1: Up  2: Right  4: Down  8: Left(BitFlag)

	unsigned char		m_nOpacity;			///< 불투명도
//	bool				m_bVisibleChildren;	///< Children Widget의 Visibility

	bool				m_bClipByParent;	///< Parent Widget에 의해 Clip되는가?

	MAlignmentMode		m_BoundsAlignment;	///< m_Rect에 대한 Alignment

public:
	char				m_szName[MWIDGET_NAME_LENGTH];	///< Name
	char				m_szIDLName[MWIDGET_NAME_LENGTH];	///< Name

	MRECT				m_Rect;				///< Rect in Parent Widget
	MRECT				m_IDLRect;			///< IDL 에서 읽었을때의 초기 rect

	MAnchors			m_Anchors;			///< Parent Widget에 비례해서 위치하는가?
	int					m_nMinWidth, m_nMinHeight;	///< 위젯의 최소 크기

	bool				m_bisListBox;
	int					m_nDebugType;
	bool				m_bEventAcceleratorCall;///< 키입력이벤트로 불렸는지 감지
protected:
	// Only for Designer Mode
	bool				m_bEnableDesignerMode;	///< Designer Mode 활성화 플래그
	int					m_nDMDragWidget;		///< 위젯 Move 또는 Resize 플래그
	MPOINT				m_DMDragPoint;			///< 위젯 Move 또는 Resize 포지션
	bool				m_bModifiedByDesigner;	///< Designer에 의해 위치,크기가 수정된 경우
	bool				m_bAddedByDesigner;		///< Designer에 의해 추가된 경우
	int					m_nID;					///< 위젯의 ID

private:
	/// Local Coordinate를 이용한 Local Event로 변환
	void MakeLocalEvent(MEvent* pLoalEvent, const MEvent* pEvent);
	/// Event에서 Resize부분 처리
	bool EventResize(MEvent* pEvent);
protected:
	/// Child Widget 맨 앞에 추가
	void InsertChild(MWidget* pWidget);
	/// Child Widget 추가
	void AddChild(MWidget* pWidget);
	/// Child Widget 삭제
	void RemoveChild(MWidget* pWidget);
	// Accelerator를 szText의 &뒤에 나오는 문자로 한다.
	//void UseAcceleractorAndCharacter(char* szText);

	/// Exclusive Stack에 추가한다.
	void AddExclusive(MWidget* pWidget);
	/// Exclusive Stack에서 제거한다.
	bool RemoveExclusive(MWidget* pWidget);

	/// Runtime Loop
	virtual void OnRun(void);
	/// Draw Handler
	virtual void OnDraw(MDrawContext* pDC);
	/*
	/// Draw Handler
	/// @deprecated	3D Rendering Pass를 따로 처리하기 위한 코드로 더이상 사용하지 않는다.
	virtual void OnDraw3D(MDrawContext3D* pDC);
	*/
	/// Event Handler. Event가 처리되면 return true, 아니면 return false.
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);

	/// 핫키를 눌렀을때 불려지는 이벤트 핸들러
	//virtual bool OnHotKey(int a, bool bShift, bool bCtrl, bool bAlt){ return false; }
	virtual bool OnHotKey(int nID){ return false; }

	/// Show()호출에 의한 Event Handler
	virtual bool OnShow(void);
	virtual void OnHide(void);

	/// 포커스 핸들러
	virtual void OnSetFocus(void){}
	virtual void OnReleaseFocus(void){}

	/// SetSize(), SetBounds()호출에 의한 Event Handler
	virtual void OnSize(int w, int h);

	/// Show(), Hide()호출에 의한 Event Handler
	void OnShow(bool bVisible);

	/// Tab Key
	virtual bool OnTab(bool bForward=true);

	/// Drop&Drop에 의해 놓여지는 핸들러
	virtual bool OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);

	/// Listener Event Handler ( 지정한 메세지를 수행했으면 return true를 한다. )
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){ return false; }

	/// Anchor 설정에 따른 Children Resize
	/// @param w	new width
	/// @param h	new height
	void ResizeChildrenByAnchor(int w, int h);

	/// Bounds Alignment에 따른 위치값
	void GetBoundsAlignmentPosition(MPOINT* p, MAlignmentMode am, int w=-1, int h=-1);

public:
	/// Workspace가 변경될때 호출되는 함수
	//virtual void OnReSize();

public:
	/// @param szName	위젯 이름
	/// @param pParent	부모 위젯
	/// @param szName	메세지를 전달받을 리스너, 보통은 부모 위젯이 해당된다.
	MWidget(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MWidget(void);

	/// Loop가 돌때마다 Widget System에서 처리할 일이 있을때 실행한다.
	void Run(void);
	/// 이 Widget을 비롯한 Child Widget을 그린다.
	void Draw(MDrawContext* pDC);
	/// 현재 다시 그린다.
	void Redraw(void);
	/// 이벤트를 처리할 핸들러 호출
	bool Event(MEvent* pEvent);
	/// 키보드 Accelerator 이벤트만 처리
	bool EventAccelerator(MEvent* pEvent);
	/// 디폴트 키(Enter, ESC) 핸들러 호출
	bool EventDefaultKey(MEvent* pEvent);

	/// 위젯을 보여준다.
	/// @param bVisible	Visible Flag
	/// @param bModal	Modal Dialog Flag
	virtual void Show(bool bVisible=true, bool bModal=false);
	/// 위젯을 감춘다.
	void Hide(void){ Show(false); }
	/// 위젯을 활성화 시킨다.
	/// @param bEnable	Enable Flag
	void Enable(bool bEnable=true);
	/// 위젯이 보이는가?
	bool IsVisible(void);
	/// 위젯이 활성화 되었는가?
	bool IsEnable(void);

	void SetResizable(bool bEnable);
	bool IsResizable(void);

	/// 리스너 지정
	/// @param pListener	지정할 리스너
	virtual void SetListener(MListener* pListener);
	/// 현재 리스너 얻기
	/// @return 현재 리스너
	virtual MListener* GetListener(void);


	/// 위젯 ID 얻기
	int GetID(void);
	/// 위제 ID 지정하기
	void SetID(int nID);

	/// 위젯 텍스트(Name) 지정하기
	virtual void SetText(const char* szText);
	/// 위젯 텍스트(Name) 얻기
	virtual const char* GetText(void);

	/// 현재 위젯에 모든 마우스 메세지가 가도록 캡쳐
	void SetCapture(void);
	/// 현재 캡쳐된 위젯을 해제
	void ReleaseCapture(void);

	void SetFocusEnable(bool bEnable);
	bool IsFocusEnable(void);

	/// 위젯에 포커스를 위치시킨다. 포커스된 위젯에 키보드 메세지가 우선적으로 들어간다.
	void SetFocus(void);
	/// 위젯에 포커스를 해제시킨다.
	void ReleaseFocus(void);
	/// 위젯에 포커스를 가지고 있는가?
	bool IsFocus(void);

	/// 부모 위젯을 얻어낸다.
	MWidget* GetParent(void);
	/// 자식 위젯 갯수를 얻어낸다.
	int GetChildCount(void);
	/// 자식 위젯을 얻어낸다.
	/// @param i	자식 위젯 인덱스
	MWidget* GetChild(int i);
	/// 자식 위젯의 인덱스를 얻어낸다.
	/// @param	pWidget	자식 위젯
	/// @return	자식 위젯의 인덱스
	int GetChildIndex(MWidget* pWidget);

	/// Exculsive 모드(Modal)로 전환한다. Exclusive 모드로 설정되어 있는 위젯이 모든 메세지를 독점한다.
	void SetExclusive(void);
	/// Exclusive 모드를 해제하나.
	void ReleaseExclusive(void);
	/// Exclusive Stack의 가장 마지막 위젯을 얻어낸다.
	MWidget* GetLatestExclusive(void);
	/// Child Widget이 Exclusive인지 테스트
	bool IsExclusive(MWidget* pWidget);

	/// 위젯에 커서를 설정한다.
	/// @param pCursor	위젯에 설정할 커서
	/// @return			이전에 설정되어 있던 커서
	MCursor* SetCursor(MCursor* pCursor);
	/// 현재 커서를 얻어낸다.
	MCursor* GetCursor(void);

	/// 위젯에 폰트를 설정한다.
	MFont* SetFont(MFont* pFont);
	/// 현재 폰트를 얻어낸다.
	MFont* GetFont(void);

	/// Size 지정하기
	/// @param w, h		가로 세로 크기
	void SetSize(int w, int h);
	/// Size 지정하기
	/// @param s		사이즈
	void SetSize(MSIZE& s);

	/// Parent Widget의 Local 좌표계에서 Poition 지정하기
	/// @param x, y		위치
	void SetPosition(int x, int y);
	/// Parent Widget의 Local 좌표계에서 Poition 지정하기
	/// @param p		위치
	void SetPosition(const MPOINT& p);
	/// Parent Widget의 Local 좌표계에서 Initial Rect 지정하기
	void SetInitialBounds(MRECT& r);
	/// Parent Widget의 Local 좌표계에서 Rect 지정하기
	/// @param r		Rectangle
	void SetBounds(const MRECT& r);
	/// Parent Widget의 Local 좌표계에서 Rect 지정하기
	/// @param x,y,w,h		Rectangle
	void SetBounds(int x, int y, int w, int h);
	/// Parent Widget의 Local 좌표계에서 Position 얻어내기
	MPOINT GetPosition(void);
	/// Parent Widget의 Local 좌표계에서 Rect 얻어내기
	MRECT GetRect(void);
	/// idl에서 읽은 rect를 얻는다
	MRECT GetIDLRect(void);
	/// 위치에대한 얼라인먼트 지정
	void SetBoundsAlignment(MAlignmentMode am, int w, int h);
	/// 위치에대한 얼라인먼트 얻기
	MAlignmentMode GetBoundsAlignment(void);

	void SetOpacity(unsigned char nOpacity);
	unsigned char GetOpacity();

	/// Screen 좌표계에서 Rect 얻어내기
	/// @deprecated		전역 좌표계는 혼란을 가져오므로 로컬 좌표계 함수들로 대체, 전역 좌표를 알고 싶으면 MClientToScreen(),MScreenToClient()를 이용
	MRECT GetScreenRect(void) const;

	/// 툴팁 스트링 지정
	void AttachToolTip(const char* szToolTipString=NULL);
	/// 툴팁 오브젝트 지정
	void AttachToolTip(MToolTip* pToolTip);
	/// 툴팁을 없앰
	void DetachToolTip(void);
	/// 현재 툴팁 오브젝트 얻기
	MToolTip* GetToolTip(void);

	/// Accelerator 지정
	void SetAccelerator(int a);
	/// Label에서 &다음에 나오는 문자로 Accelerator 지정
	void SetLabelAccelerator(void);
	/// 현재 Accelerator 얻기
	char GetLabelAccelerator(void);
	/// 현재 ToolTip의 Accelerator 얻기
	char GetToolTipAccelerator(void);

	/// Widget의 커스터마이즈된 Client Rect
	virtual MRECT GetClientRect(void);
	/// Widget의 원래 Client Rect(0, 0, width, height)
	MRECT GetInitialClientRect(void);

	/// Child Widget 순서를 바꿔 그리는 순서를 조정한다.
	void SetZOrder(MZOrder z);
	/// 자손 Widget중 맨처음 발견된 Exclusive Widget 리턴
	MWidget* FindExclusiveDescendant(void);

	/// 위치로 위젯 검색
	MWidget* Find(int x, int y) { return Find(MPOINT(x, y)); }
	/// 위치로 위젯 검색
	MWidget* Find(const MPOINT& p);
	MWidget* FindDropAble(MPOINT& p);

	/// 해당 ID의 Drag Object가 Drop가능한가?
	virtual bool IsDropable(MWidget* pSender){ return false; }
	bool Drop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);

	void SetVisible(bool b) { m_bVisible = b; }
	bool GetVisible() { return m_bVisible; }

	template<size_t size>
	void GetHierarchicalName(char(&szName)[size]) {
		GetHierarchicalName(szName, size);
	}
	/// 계층 구조를 포함한 이름 얻어내기 ( 예: Parent/Child )
	void GetHierarchicalName(char* szName, int maxlen);
	/// 계층 구조를 포함한 이름으로 위젯 찾기
	MWidget* FindWidgetByHierarchicalName(const char* szName);

	/// 현재 위젯에서 Accelerator등에 의해 수행될 수 있는 Default Command를 지정 ( 오버라이드한 경우 return true를 한다. )
	virtual bool DefaultCommand(void){ return false; }

	/// 메세지가 일치하는가?
	static bool IsMsg(const char* szMsg1, const char* szMsg2);

	/// Query Anything
	virtual void* Query(const char* szQuery);

	void SetClipByParent(bool b) {
		m_bClipByParent = b;
	}

#define MINT_WIDGET	"Widget"
	/// 클래스 이름 얻기
	virtual const char* GetClassName(void){ return MINT_WIDGET; }
};


// Help Functions
/// &가 있는 위치를 알려준다. szText가 NULL이면 m_szName에서 &의 위치를 찾는다.
int GetAndPos(const char* szText);
/// & 뒤에 붙어있는 문자를 얻어낸다.
char GetAndChar(const char* szText);
/// &문자 제거
int RemoveAnd(char* szText);

template<size_t size>
int RemoveAnd(char(&szRemovedText)[size], const char* szText) {
	return RemoveAnd(szRemovedText, size, szText);
}
/// &문자 제거
/// @param szRemovedText	[out]	제거된 문자열
/// @param szText			[in]	소스 문자열
int RemoveAnd(char* szRemovedText, int maxlen, const char* szText);

template<size_t size>
int RemoveAnd(char(&szRemovedFrontText)[size], char* cUnderLineChar, char* szRemovedBackText, const char* szText) {
	return RemoveAnd(szRemovedFrontText, size, cUnderLineChar, szRemovedBackText, szText);
}
/// &문자 제거
/// @param szRemovedFrontText	[out]	&앞쪽 문자열
/// @param cUnderLineChar		[out]	&로 인해 밑줄이 붙을 문자열
/// @param szRemovedBackText	[out]	&뒤쪽 문자열
/// @param szText				[in]	소스 문자열
int RemoveAnd(char* szRemovedFrontText, int maxlen, char* cUnderLineChar, char* szRemovedBackText, const char* szText);

/// 로컬 위치를 전역 위치로 변환
MPOINT MClientToScreen(const MWidget* pWidget, const MPOINT& p);
/// 전역 위치를 로컬 위치로 변환
MPOINT MScreenToClient(const MWidget* pWidget, const MPOINT& p);
/// 로컬 영역을 전역 영역으로 변환
MRECT MClientToScreen(const MWidget* pWidget, const MRECT& p);
/// 전역 영역을 로컬 영역으로 변환
MRECT MScreenToClient(const MWidget* pWidget, const MRECT& p);

/// 현재 커서 위치 얻기
MPOINT GetCursorPosition(void);

/// 같은 위젯 클래스인지 검사
inline bool MIsSameWidgetClass(MWidget* pWidget, const char* szClassName);
/// 같은 위젯 클래스인지 검사
inline bool MIsSameWidgetClass(MWidget* pWidgetA, MWidget* pWidgetB);


#endif
