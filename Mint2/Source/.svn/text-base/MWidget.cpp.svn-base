#include "stdafx.h"
#include "MResource.h"
#include "MWidget.h"
#include "MToolTip.h"
#include "Mint.h"
#include "MButton.h"
#include "MEdit.h"


inline bool MIsSameWidgetClass(MWidget* pWidget, const char* szClassName)
{
	if (strcmp(pWidget->GetClassName(), szClassName) == 0) return true;
	return false;
}

inline bool MIsSameWidgetClass(MWidget* pWidgetA, MWidget* pWidgetB)
{
	if (strcmp(pWidgetA->GetClassName(), pWidgetB->GetClassName()) == 0) return true;
	return false;
}

MWidget* MWidget::m_pCapturedWidget = NULL;
MWidget* MWidget::m_pFocusedWidget = NULL;

static int g_nWidgetCount=0;

void MWidget::MakeLocalEvent(MEvent* pLoalEvent, const MEvent* pEvent)
{
	memcpy(pLoalEvent, pEvent, sizeof(MEvent));

	MRECT sr = GetScreenRect();
	pLoalEvent->Pos.x -= sr.x;
	pLoalEvent->Pos.y -= sr.y;
}

bool MWidget::EventResize(MEvent* pEvent)
{
	MEvent LocalEvent;
	MakeLocalEvent(&LocalEvent, pEvent);

	// Widget Resize
#define RESIZER_SIZE	4
	if(m_bResizable==true){
		MPOINT sp = pEvent->Pos;
		MWidget* pParent = GetParent();
		if(pParent==NULL) return false;

		if(pEvent->nMessage==MWM_LBUTTONDOWN){
			m_nResizeSide = 0;
			if(LocalEvent.Pos.y>=-RESIZER_SIZE && LocalEvent.Pos.y<=RESIZER_SIZE){
				if(LocalEvent.Pos.x>=-RESIZER_SIZE && LocalEvent.Pos.x<=m_Rect.w+RESIZER_SIZE)
					m_nResizeSide |= 1;
			}
			if(LocalEvent.Pos.x>=m_Rect.w-RESIZER_SIZE && LocalEvent.Pos.x<=m_Rect.w+RESIZER_SIZE){
				if(LocalEvent.Pos.y>=-RESIZER_SIZE && LocalEvent.Pos.y<=m_Rect.h+RESIZER_SIZE)
					m_nResizeSide |= 2;
			}
			if(LocalEvent.Pos.y>=m_Rect.h-RESIZER_SIZE && LocalEvent.Pos.y<=m_Rect.h+RESIZER_SIZE){
				if(LocalEvent.Pos.x>=-RESIZER_SIZE && LocalEvent.Pos.x<=m_Rect.w+RESIZER_SIZE)
					m_nResizeSide |= 4;
			}
			if(LocalEvent.Pos.x>=-RESIZER_SIZE && LocalEvent.Pos.x<=RESIZER_SIZE){
				if(LocalEvent.Pos.y>=-RESIZER_SIZE && LocalEvent.Pos.y<=m_Rect.h+RESIZER_SIZE)
					m_nResizeSide |= 8;
			}
			if(m_nResizeSide!=0){
				SetCapture();
				//
				return true;
			}
		}
		if(m_nResizeSide!=0 && pEvent->nMessage==MWM_LBUTTONUP){
			ReleaseCapture();
			m_nResizeSide = 0;
			return true;
		}
		if(pEvent->nMessage==MWM_MOUSEMOVE){
			if(m_nResizeSide!=0){
				MPOINT pp = MScreenToClient(pParent, sp);
				MPOINT tp = MScreenToClient(this, sp);
				MRECT r = m_Rect;
				if(m_nResizeSide&1){
					r.h += (r.y - pp.y);
					r.y = pp.y;
				}
				if(m_nResizeSide&2){
					r.w = tp.x;
				}
				if(m_nResizeSide&4){
					r.h = tp.y;
				}
				if(m_nResizeSide&8){
					r.w += (r.x - pp.x);
					r.x = pp.x;
				}
				SetPosition(r.x, r.y);
// 임시로 위젯 최소 크기를 지정, 나중에 멤버로 빠져줄 필요가 있다.
				if(r.w<m_nMinWidth) r.w = m_nMinWidth;
				if(r.h<m_nMinHeight) r.h = m_nMinHeight;
				SetSize(r.w, r.h);
				return true;
			}
		}
	}

	return false;
}


void MWidget::InsertChild(MWidget* pWidget)
{
	m_Children.InsertBefore(pWidget);
	pWidget->m_pParent = this;
}

void MWidget::AddChild(MWidget* pWidget)
{
	m_Children.Add(pWidget);
	pWidget->m_pParent = this;
}

void MWidget::RemoveChild(MWidget* pWidget)
{
	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pCurWnd = m_Children.Get(i);
		if(pCurWnd==pWidget){
			pWidget->m_pParent = NULL;
			m_Children.Delete(i);
			return;
		}
	}
}

void MWidget::AddExclusive(MWidget* pWidget)
{
	m_Exclusive.Add(pWidget);
}

bool MWidget::RemoveExclusive(MWidget* pWidget)
{
	for(int i=0; i<m_Exclusive.GetCount(); i++){
		MWidget* pThis = m_Exclusive.Get(i);
		if(pThis==pWidget){
			m_Exclusive.Delete(i);
			return true;
		}
	}
	return false;
}

MWidget* MWidget::GetLatestExclusive(void)
{
	if(m_Exclusive.GetCount()>0) return m_Exclusive.Get(m_Exclusive.GetCount()-1);
	return NULL;
}

/*
void MWidget::UseAcceleractorAndCharacter(char* szText)
{
	if(szText==NULL){
		m_nAccelerator = -1;
		return;
	}
	char szAnd[2] = {GetAndChar(szText), 0};
	_strupr(szAnd);
	m_nAccelerator = szAnd[0];
	if(m_nAccelerator==0) m_nAccelerator = -1;
}
*/

MWidget::MWidget(const char* szName, MWidget* pParent, MListener* pListener)
{
	g_nWidgetCount++;
	m_nID = g_nWidgetCount;

	if(szName==NULL) m_szName[0] = NULL;
	else{
		if(strlen(szName)<MWIDGET_NAME_LENGTH){
			strcpy(m_szName, szName);
		}
		else{
			// MWIDGET_NAME_LENGTH보다 큰 문자열은 잘라 버림.
			memcpy(m_szName, szName, MWIDGET_NAME_LENGTH-4);
			m_szName[MWIDGET_NAME_LENGTH-4] = '.';
			m_szName[MWIDGET_NAME_LENGTH-3] = '.';
			m_szName[MWIDGET_NAME_LENGTH-2] = '.';
			m_szName[MWIDGET_NAME_LENGTH-1] = 0;
		}
	}

	// Default Region
	m_Rect.x = 0;
	m_Rect.y = 0;
	m_Rect.w = 100;
	m_Rect.h = 100;

	m_pParent = pParent;
	if(m_pParent!=NULL) m_pParent->AddChild(this);

	m_pListener = pListener;

	m_pCursor = NULL;		// NULL Cursor is Default Cursor
	m_pFont = NULL;			// NULL Font is Default Font

	m_bVisible = true;
	m_bEnable = true;
	m_bFocusEnable = false;	// Default Focus Disabled

	m_pToolTip = NULL;

	//EnableAccelerator(true);
	//SetLabelAccelerator();
	SetAccelerator(0);

	//Show(true, false);

	m_bZOrderChangable = false;
	m_bResizable = false;	// 디폴트는 리사이즈 불가능
	m_nResizeSide = 0;

	m_bClipByParent = true;

//	if (m_pParent!=NULL) m_nOpacity = m_pParent->GetOpacity();
//	else 
		m_nOpacity = 255;

//	m_bVisibleChildren = true;

	m_bEnableDesignerMode = true;
	m_nDMDragWidget = 0;
	m_bModifiedByDesigner = false;
	m_bAddedByDesigner = false;
	//m_nID = -1;

	m_BoundsAlignment = MAM_NOTALIGN;

	/*
	m_bAnchorLeft = true;
	m_bAnchorRight = false;
	m_bAnchorTop = true;
	m_bAnchorBottom = false;
	*/

	m_nMinWidth = 10;
	m_nMinHeight = 10;

	m_bisListBox = false;

	m_nDebugType = 0;
	m_bEventAcceleratorCall = false;

	m_IDLRect = MRECT(-1,-1,-1,-1);

}

MWidget::~MWidget(void)
{
	ReleaseExclusive();

	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pWidget = m_Children.Get(i);
		pWidget->m_pParent = NULL;	// 부모가 먼저 죽을 경우 Children's Parent Link 삭제
	}

	/*
	// 기존 코드가 명시적으로 Child를 각각 가지고 있을때 문제가 발생한다.
	// 가지고 있는 Children도 모두 삭제
	while(m_Children.GetCount()>0){
		MWidget* pWidget = m_Children.Get(0);
		delete pWidget;
		//RemoveChild(pWidget);
	}
	*/

	if(m_pParent!=NULL) m_pParent->RemoveChild(this);
	if(MWidget::m_pCapturedWidget==this) MWidget::m_pCapturedWidget = NULL;
	if(MWidget::m_pFocusedWidget==this) MWidget::m_pFocusedWidget = NULL;
	DetachToolTip();
}

void MWidget::OnRun(void)
{
}

void MWidget::OnDraw(MDrawContext* pDC)
{
	// Draw Nothing
	pDC->SetColor(MCOLOR(196, 196, 196));
	pDC->FillRectangle(GetInitialClientRect());
}

/*
void MWidget::OnDraw3D(MDrawContext3D* pDC)
{
}
*/

bool MWidget::OnEvent(MEvent* pEvent, MListener* pListener)
{
	return false;
}

void MWidget::Run(void)
{
	OnRun();

	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pCurWnd = m_Children.Get(i);
		pCurWnd->Run();
	}
}
#include "MDebug.h"

void MWidget::Draw(MDrawContext* pDC)
{
	if(m_bVisible==false) return;

//	MBeginProfile(100+m_nID,m_szIDLName);

	// Opacity 설정
	unsigned char nLastOpacity;
	nLastOpacity = pDC->GetOpacity();

	MRECT sr = GetScreenRect();
	pDC->SetOrigin(MPOINT(sr.x, sr.y));

	// 그리기 전 초기화
	if(m_pFont!=NULL) pDC->SetFont(m_pFont);
	else pDC->SetFont(MFontManager::Get(NULL));	// 폰트가 NULL인 경우 Default Font로 세팅한다.

	pDC->SetOpacity((unsigned char)(nLastOpacity * (float)(m_nOpacity / 255.0f)));
	if(!IsEnable())
		pDC->SetOpacity((unsigned char)(pDC->GetOpacity()*0.70));		// Disable 상태일때 너무 어두워서 좀 밝게 했음(동환)

	bool bIntersect = true;
	MRECT rectScreen(0, 0, MGetWorkspaceWidth()-1, MGetWorkspaceHeight()-1);
	MRECT PrevClipRect;
	if(GetParent()!=NULL) {
		MRECT parentClipRect = MClientToScreen(GetParent(), GetParent()->GetClientRect());
		bIntersect = rectScreen.Intersect(&PrevClipRect,parentClipRect);
	}else
		PrevClipRect = rectScreen;

	//if(GetParent()!=NULL) PrevClipRect = GetParent()->GetScreenRect();
	MRECT CurrClipRect = GetScreenRect();
	MRECT IntersectClipRect;

	if(m_bClipByParent==true){
		// 부모 위젯과 공통되는 영역이 있을 경우에만 그린다.
		if(PrevClipRect.Intersect(&IntersectClipRect, CurrClipRect)==true){
			MRECT test = IntersectClipRect;
			if(IntersectClipRect.w>0 && IntersectClipRect.h>0) {
				pDC->SetClipRect(IntersectClipRect);
				// 그리기
				OnDraw(pDC);
			}
		}
	}
	else{
		pDC->SetClipRect(CurrClipRect);
		// 그리기
		OnDraw(pDC);
	}

	// Child Widget Drawing
//	if(m_bVisibleChildren==true){
	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pCurWnd = m_Children.Get(i);
		if(pCurWnd==GetLatestExclusive()) continue;
		if(pCurWnd != NULL ) pCurWnd->Draw(pDC);
	}
	if(GetLatestExclusive()!=NULL) GetLatestExclusive()->Draw(pDC);	// Exclusive Widget을 마지막에 그린다.
//	}

	// Opacity 재설정
	pDC->SetOpacity(nLastOpacity);

//	MEndProfile(100+m_nID);
}

/*
void MWidget::Draw3D(MDrawContext3D* pDC)
{
	if(m_bVisible==true) OnDraw3D(pDC);
	else return;

	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pCurWnd = m_Children.Get(i);
		pCurWnd->Draw3D(pDC);
	}
}
*/

void MWidget::Redraw(void)
{
	Mint::GetInstance()->Update();
}

bool MWidget::Event(MEvent* pEvent)
{
	if(m_bVisible==false) return false;	// 보이는 윈도우만 이벤트를 처리할 수 있다.

	// Look에서 정의한 Client영역이 아닌 실제 Client 영역
	//MRECT r = GetClientRect();
	MRECT r = GetRect();
	r.x = r.y = 0;

	if(pEvent->nMessage==MWM_LBUTTONDOWN)
		int k=0;

	MEvent LocalEvent;
	MakeLocalEvent(&LocalEvent, pEvent);

	if(m_nDebugType==2){
		if(pEvent->nMessage==MWM_LBUTTONDOWN )
			int k=0;
	}
	// Disable상태에서도 툴팁은 보이게 해준다.
	//if(pEvent->nMessage==MWM_MOUSEMOVE && m_Rect.InPoint(pEvent->Pos)==true && (MWidget::m_pCapturedWidget==NULL || MWidget::m_pCapturedWidget==this)){
	if(r.InPoint(LocalEvent.Pos)==true && (MWidget::m_pCapturedWidget==NULL || MWidget::m_pCapturedWidget==this) && IsVisible()==true){
		if(m_pCursor!=NULL) MCursorSystem::Set(m_pCursor);
		else MCursorSystem::Set(MCURSOR_ARROW);		// 아무것도 지정되어 있지 않으면 Default Cursor로...

		if(m_pToolTip!=NULL){
			// 툴팁 보여주기 위해 ZOrder를 바꾸면 안되지만, 해결책이 없으므로 우선 이렇게 해둔다.
			SetZOrder(MZ_TOP);	// 이 윈도우를 차일드 윈도우 맨 앞에 보이게 해서.. 툴팁이 가리지 않게..
			m_pToolTip->Show(true);
		}
	}
	else{
		if(m_pToolTip!=NULL){
			m_pToolTip->Show(false);
		}
	}

	if(m_bEnable==false) return false;

//	if(m_bVisibleChildren==true && GetLatestExclusive()!=NULL){
	if(GetLatestExclusive()!=NULL){
		if(GetLatestExclusive()->Event(pEvent)==true) return true;
		if(GetLatestExclusive()!=NULL) return false;	// 이벤트 핸들러에 의해 Exclusive가 풀리는 경우가 있으면 다음으로 넘어간다.
	}

	if(pEvent->nMessage==MWM_LBUTTONDOWN)
	{
		if ( r.InPoint(LocalEvent.Pos)==true)
		{
			SetFocus();
//			OnSetFocus();
 			if((m_bFocusEnable)&&(m_bZOrderChangable==true)) SetZOrder(MZ_TOP);	// 해당 위젯을 맨 위로 올려준다.
		}
		else	// ReleaseFocus
		{
//			ReleaseFocus();
//			OnReleaseFocus();
//			ReleaseCapture();
		}
	}

//	if(m_bVisibleChildren==true && GetLatestExclusive()==NULL){
	if(GetLatestExclusive()==NULL){
		for(int i=m_Children.GetCount()-1; i>=0; i--){	// 마지막에 추가된 윈도우부터
			MWidget* pCurWnd = m_Children.Get(i);
			if(pCurWnd->Event(pEvent)==true) {
				return true;
			}
		}
	}

	// 위젯 리사이즈
	// 클라이언트 영역을 벗어나서 제어해야하므로, Event()에서 처리한다.
	if(EventResize(pEvent)==true) return true;

	//if(pEvent->nMessage==MWM_RESIZE) OnReSize();
	
	// HotKey
	/*
	if(pEvent->nMessage==MWM_KEYDOWN){
	//if(OnHotKey(pEvent->nKey, pEvent->GetShiftState(), pEvent->GetCtrlState(), pEvent->GetAltState())==true) return true;
	if(OnHotKey(pEvent->nKey, pEvent->GetShiftState(), pEvent->bCtrl, pEvent->bAlt)==true) return true;
	}
	*/
	if(pEvent->nMessage==MWM_HOTKEY){
		//if(OnHotKey(pEvent->nKey, pEvent->GetShiftState(), pEvent->GetCtrlState(), pEvent->GetAltState())==true) return true;
		if(OnHotKey(pEvent->nKey)==true) return true;
	}

	// Focused Event
	if(pEvent->nMessage==MWM_CHAR || pEvent->nMessage==MWM_KEYDOWN || pEvent->nMessage==MWM_IMECOMPOSE ||
		pEvent->nMessage==MWM_ACTIONKEYDOWN || pEvent->nMessage==MWM_ACTIONKEYUP || pEvent->nMessage==MWM_ACTIONPRESSED || pEvent->nMessage==MWM_ACTIONRELEASED){
		if (IsFocus()) {	// KEY Event는 Focus 없는 위젯에는 의미없다
			//먼저 자식에게로 전해져서 처리되면 ok 처리한 자식이 없으면 부모에게까지 간다
			if(OnEvent(&LocalEvent, GetListener())==true) 
				return true;

			// Tab Key
			if(pEvent->nMessage==MWM_KEYDOWN && pEvent->nKey==VK_TAB){
				if(OnTab(!pEvent->GetShiftState())==true) 
					return true;
			}

			/*
			// Focused Widget에게 우선권이 있으며...
			if(MWidget::m_pFocusedWidget==this){	// MWM_CHAR 메세지인 경우 현재 포커스된 윈도우에게만 메세지가 보내진다.
				if(OnEvent(&LocalEvent, GetListener())==true) 
					return true;
			}
			*/
			// Accelerator는 EventAccelerator쪽으로 빠짐
		}
	}
	else{
		if((MWidget::m_pCapturedWidget==NULL || MWidget::m_pCapturedWidget==this))	// Capture가 걸릴 경우 Capture된 윈도우에만 메세지 보내기
			if(OnEvent(&LocalEvent, GetListener())==true) 
				return true;
	}

	if(pEvent->nMessage==MWM_LBUTTONUP) ReleaseCapture();

	return false;
}

bool MWidget::EventAccelerator(MEvent* pEvent)
{
	if(m_bVisible==false) return false;	// 보이는 윈도우만 이벤트를 처리할 수 있다.
	if(m_bEnable==false) return false;

	if(!(pEvent->nMessage==MWM_KEYDOWN || pEvent->nMessage==MWM_SYSKEYDOWN)) return false;

	// KEYDOWN일때 Edit에 Focus가 있을 경우 Accelerator는 무시된다. (SYSKEYDOWN만 유효하게 된다)
	if(pEvent->nMessage==MWM_KEYDOWN)
		if(MWidget::m_pFocusedWidget!=NULL)
			if(strcmp(MWidget::m_pFocusedWidget->GetClassName(), MINT_EDIT)==0) return false;

	if(GetLatestExclusive()!=NULL){
		if(GetLatestExclusive()->EventAccelerator(pEvent)==true) return true;
		if(GetLatestExclusive()!=NULL) return false;	// 이벤트 핸들러에 의해 Exclusive가 풀리는 경우가 있으면 다음으로 넘어간다.
	}

	if(GetLatestExclusive()==NULL){
		for(int i=m_Children.GetCount()-1; i>=0; i--){	// 마지막에 추가된 윈도우부터
			MWidget* pCurWnd = m_Children.Get(i);
			if(pCurWnd->EventAccelerator(pEvent)==true) return true;
		}
	}

	// Accelerator
	char szKey[2] = {(char)pEvent->nKey, 0};
//	_strupr(szKey);
	if(m_nAccelerator==szKey[0]) {

		m_bEventAcceleratorCall = true;

		if(DefaultCommand()==true) {
			m_bEventAcceleratorCall = false;
			return true;
		}
		m_bEventAcceleratorCall = false;
	}

	return false;
}

bool MWidget::EventDefaultKey(MEvent* pEvent)
{
	if(m_bVisible==false) return false;	// 보이는 윈도우만 이벤트를 처리할 수 있다.
	if(m_bEnable==false) return false;

	if(!(pEvent->nMessage==MWM_KEYDOWN))return false;

	if(GetLatestExclusive()!=NULL){
		if(GetLatestExclusive()->EventDefaultKey(pEvent)==true) return true;
		if(GetLatestExclusive()!=NULL) return false;	// 이벤트 핸들러에 의해 Exclusive가 풀리는 경우가 있으면 다음으로 넘어간다.
	}

	if(GetLatestExclusive()==NULL){
		for(int i=m_Children.GetCount()-1; i>=0; i--){	// 마지막에 추가된 윈도우부터
			MWidget* pCurWnd = m_Children.Get(i);
			if(pCurWnd->EventDefaultKey(pEvent)==true) return true;
		}
	}

	if(strcmp(GetClassName(), MINT_BUTTON)!=0) return false;

	MButton* pButton = (MButton*)this;
	if((pButton->m_nKeyAssigned==MBKA_ENTER && pEvent->nKey==VK_RETURN) ||
		(pButton->m_nKeyAssigned==MBKA_ESC && pEvent->nKey==VK_ESCAPE))
		if(DefaultCommand()==true) return true;

	return false;
}

bool MWidget::OnShow(void)
{
	return true;
}

void MWidget::OnHide(void)
{
}

void MWidget::OnSize(int w, int h)
{
}

/*
void MWidget::OnReSize()
{
}
*/

void MWidget::OnShow(bool bVisible)
{
	for(int i=m_Children.GetCount()-1; i>=0; i--){	// 마지막에 추가된 윈도우부터
		MWidget* pCurWnd = m_Children.Get(i);
		if(pCurWnd->m_bVisible==true) pCurWnd->OnShow(bVisible);
	}

	if(bVisible==true) OnShow();
	else OnHide();

	if(bVisible==false && MWidget::m_pFocusedWidget==this) ReleaseFocus();
}

bool MWidget::OnTab(bool bForward)
{
	MWidget* pParent = GetParent();
	if(pParent==NULL) return false;

	if(m_pParent->GetLatestExclusive()==this) return false;

	int nThisIndex = pParent->GetChildIndex(this);
	if(nThisIndex<0) return false;

	for(int i=0; i<pParent->GetChildCount(); i++){
		int nIndex = 0;
		if(bForward==true) nIndex = (nThisIndex+i+1) % pParent->GetChildCount();
		else nIndex = (nThisIndex+pParent->GetChildCount()-1-i) % pParent->GetChildCount();

		MWidget* pSibling = pParent->GetChild( nIndex );
		if(pSibling->IsFocusEnable()==true && pSibling!=this && pSibling->IsVisible()==true && pSibling->IsEnable()==true){
			pSibling->SetFocus();
			return true;
		}
	}

	return false;
}

bool MWidget::OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	return false;
}

void MWidget::ResizeChildrenByAnchor(int w, int h)
{
	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pChild = m_Children.Get(i);
		if(pChild->GetBoundsAlignment()!=0){
			pChild->SetBoundsAlignment(pChild->GetBoundsAlignment(), w, h);
			continue;	// Bounds Alignment에 우선순위가 있다.
		}

		MRECT r = pChild->m_Rect;
		if(pChild->m_Anchors.m_bLeft==true && pChild->m_Anchors.m_bRight==true){
			r.w += (w-m_Rect.w);
		}
		else if(pChild->m_Anchors.m_bRight==true){
			r.x += (w-m_Rect.w);
		}
		if(pChild->m_Anchors.m_bTop==true && pChild->m_Anchors.m_bBottom==true){
			r.h += (h-m_Rect.h);
		}
		else if(pChild->m_Anchors.m_bBottom==true){
			r.y += (h-m_Rect.h);
		}
		pChild->SetBounds(r);
	}
}

void MWidget::Show(bool bVisible, bool bModal)
{
	if(m_bVisible==bVisible){
		if(bModal==true){
			if(m_pParent!=NULL && m_pParent->GetLatestExclusive()==this)
				return;
		}
		else return;
	}

	m_bVisible = bVisible;

	if(bVisible==true && bModal==true) SetExclusive();
	else if(bVisible==false) {
		ReleaseExclusive();
		if(MWidget::m_pFocusedWidget==this) ReleaseFocus();
	}

	OnShow(bVisible);
}

void MWidget::Enable(bool bEnable)
{
	m_bEnable = bEnable;
}

bool MWidget::IsVisible(void)
{
	return m_bVisible;
}

bool MWidget::IsEnable(void)
{
	return m_bEnable;
}

void MWidget::SetResizable(bool bEnable)
{
	m_bResizable = bEnable;
}

bool MWidget::IsResizable(void)
{
	return m_bResizable;
}

void MWidget::SetListener(MListener* pListener)
{
	m_pListener = pListener;
}

MListener* MWidget::GetListener(void)
{
	return m_pListener;
}

int MWidget::GetID(void)
{
	return m_nID;
}

void MWidget::SetID(int nID)
{
	m_nID = nID;
}

void MWidget::SetText(const char* szText)
{
	if(szText==NULL) return;
	_ASSERT(strlen(szText)<MWIDGET_NAME_LENGTH);
	if (strlen(szText) >= MWIDGET_NAME_LENGTH) return;

	strcpy(m_szName, szText);
	if(m_pToolTip!=NULL) if(m_pToolTip->IsUseParentName()==true) m_pToolTip->SetBounds();

	SetLabelAccelerator();
	//if(m_nAcceleratorType==1) UseAcceleratorAndCharacter(szText);
}

const char* MWidget::GetText(void)
{
	return m_szName;
}

void MWidget::SetCapture(void)
{
	MWidget::m_pCapturedWidget = this;
}

void MWidget::ReleaseCapture(void)
{
	MWidget::m_pCapturedWidget = NULL;
}

void MWidget::SetFocusEnable(bool bEnable)
{
	m_bFocusEnable = bEnable;
}

bool MWidget::IsFocusEnable(void)
{
	return m_bFocusEnable;
}

void MWidget::SetFocus(void)
{
	if(m_bFocusEnable==false) return;
	/*
	if(m_pParent!=NULL){
		if(m_pParent->m_pExclusive!=NULL && m_pParent->m_pExclusive!=MWidget::m_pFocusedWidget) return;
	}
	*/
	// 자식중에 Exclusive 위젯이 있는경우,
	// 현재 Exclusive 위젯과 그 Children에게만 Focus가 갈 수 있도록 한다.
	MWidget* pExDes = FindExclusiveDescendant();
	if(pExDes!=NULL) return;
	/*
	if(MWidget::m_pFocusedWidget!=NULL){
		if(MWidget::m_pFocusedWidget->m_pParent!=NULL){
			if(MWidget::m_pFocusedWidget->m_pParent->m_pExclusive==MWidget::m_pFocusedWidget){
				return;
			}
		}
	}
	*/

	if(MWidget::m_pFocusedWidget==this) return;

	if(MWidget::m_pFocusedWidget!=NULL) MWidget::m_pFocusedWidget->OnReleaseFocus();

	MWidget::m_pFocusedWidget = this;
	OnSetFocus();
}

void MWidget::ReleaseFocus(void)
{
	if(MWidget::m_pFocusedWidget==this) OnReleaseFocus();
	MWidget::m_pFocusedWidget = NULL;
}

bool MWidget::IsFocus(void)
{
	if(MWidget::m_pFocusedWidget==this) return true;
	return false;
}

MWidget* MWidget::GetParent(void)
{
	return m_pParent;
}

int MWidget::GetChildCount(void)
{
	return m_Children.GetCount();
}

MWidget* MWidget::GetChild(int i)
{
	return m_Children.Get(i);
}

int MWidget::GetChildIndex(MWidget* pWidget)
{
	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pChild = m_Children.Get(i);
		if(pChild==pWidget) return i;
	}

	return -1;
}

void MWidget::SetExclusive(void)
{
	if(m_pParent!=NULL){
		m_pParent->AddExclusive(this);
		SetFocus();	// Exclusive로 보여질때 포커싱.
	}
}

void MWidget::ReleaseExclusive(void)
{
	if(m_pParent!=NULL)
		m_pParent->RemoveExclusive(this);
}

MCursor* MWidget::SetCursor(MCursor* pCursor)
{
	MCursor* pTemp = m_pCursor;
	m_pCursor = pCursor;
	return pTemp;
}

MCursor* MWidget::GetCursor(void)
{
	return m_pCursor;
}

MFont* MWidget::SetFont(MFont* pFont)
{
	MFont* pTemp = m_pFont;
	m_pFont = pFont;
	return pTemp;
}

MFont* MWidget::GetFont(void)
{
	if(m_pFont==NULL) return MFontManager::Get(NULL);
	else return m_pFont;
}

void MWidget::SetSize(int w, int h)
{
	if(w<0) w = 1;
	if(h<0) h = 1;

	// TODO: 100,100 으로 지정하면 default size 가 되어 onsize가 불리지 않음.
	if(w==m_Rect.w && h==m_Rect.h) return;

	ResizeChildrenByAnchor(w, h);

	m_Rect.w = w;
	m_Rect.h = h;

	OnSize(w, h);
}

void MWidget::SetSize(MSIZE& s)
{
	SetSize(s.w, s.h);
}


void MWidget::SetPosition(int x, int y)
{
	MPOINT p;

	//if(m_BoundsAlignment!=MAM_NOTALIGN) return;	// 얼라인 모드 설정된 위젯은 위치 재설정을 허용하지 않는다.
	//m_BoundsAlignment = MAM_NOTALIGN;	// 위치를 바꾸면 얼라인이 없어진다.

	m_Rect.x = x;
	m_Rect.y = y;

}

void MWidget::SetPosition(MPOINT& p)
{
	SetPosition(p.x, p.y);
}

void MWidget::SetBounds(MRECT& r)
{
	SetBounds(r.x, r.y, r.w, r.h);
}

void MWidget::SetBounds(int x, int y, int w, int h)
{
	SetPosition(x, y);
	//m_Rect.x = x;
	//m_Rect.y = y;

	if(w<0) w = 1;
	if(h<0) h = 1;

	SetSize(w, h);
	/*
	bool bResized = false;
	if(w!=m_Rect.w || h!=m_Rect.h){
		m_Rect.w = w;
		m_Rect.h = h;
		bResized = true;
	}
	*/

	//if(bResized==true) OnSize(w, h);
}

MPOINT MWidget::GetPosition(void)
{
	return MPOINT(m_Rect.x, m_Rect.y);
}

MRECT MWidget::GetRect(void)
{
	return m_Rect;
}

MRECT MWidget::GetIDLRect(void)
{
	return m_IDLRect;
}

void MWidget::GetBoundsAlignmentPosition(MPOINT* p, MAlignmentMode am, int w, int h)
{
	p->x = m_Rect.x;
	p->y = m_Rect.y;

	MSIZE s(w, h);
	if(w==-1){
		s.w = MGetWorkspaceWidth();
		if(GetParent()!=NULL) s.w = GetParent()->m_Rect.w;
	}
	if(h==-1){
		s.h = MGetWorkspaceHeight();
		if(GetParent()!=NULL) s.h = GetParent()->m_Rect.h;
	}

	if(m_BoundsAlignment&MAM_LEFT) p->x = 0;
	else if(m_BoundsAlignment&MAM_RIGHT) p->x = s.w-m_Rect.w;
	else if(m_BoundsAlignment&MAM_HCENTER) p->x = (s.w-m_Rect.w)/2;

	if(m_BoundsAlignment&MAM_LEFT) p->y = 0;
	else if(m_BoundsAlignment&MAM_BOTTOM) p->y = s.h-m_Rect.h;
	else if(m_BoundsAlignment&MAM_VCENTER) p->y = (s.h-m_Rect.h)/2;
}

void MWidget::SetBoundsAlignment(MAlignmentMode am, int w, int h)
{
	m_BoundsAlignment = am;

	MPOINT p;
	GetBoundsAlignmentPosition(&p, m_BoundsAlignment, w, h);

	// 포지션 정렬에 의해 재설정
	m_Rect.x = p.x;
	m_Rect.y = p.y;
}

MAlignmentMode MWidget::GetBoundsAlignment(void)
{
	return m_BoundsAlignment;
}


MRECT MWidget::GetScreenRect(void)
{
	if(m_pParent!=NULL){
		MRECT sr = m_pParent->GetScreenRect();
		MRECT r = m_Rect;
		r.Offset(sr.x, sr.y);
		return r;
	}

	return m_Rect;
}

void MWidget::AttachToolTip(const char* szToolTipString)
{
	DetachToolTip();
	m_pToolTip = new MToolTip(szToolTipString, this);
}

void MWidget::AttachToolTip(MToolTip* pToolTip)
{
	DetachToolTip();
	m_pToolTip = pToolTip;
}

void MWidget::DetachToolTip(void)
{
	if(m_pToolTip!=NULL) delete m_pToolTip;
	m_pToolTip = NULL;
}

MToolTip* MWidget::GetToolTip(void)
{
	return m_pToolTip;
}

void MWidget::SetAccelerator(int a)
{
	//m_nAcceleratorType = 0;	// use custom accelerator type
	if(a==0) m_nAccelerator = -1;
	else m_nAccelerator = a;
}

void MWidget::SetLabelAccelerator(void)
{
	SetAccelerator(GetLabelAccelerator());
}

char MWidget::GetLabelAccelerator(void)
{
	char szAnd[2] = {GetAndChar(m_szName), 0};
	_strupr(szAnd);
	if(szAnd[0]==0) return -1;
	return szAnd[0];
}

char MWidget::GetToolTipAccelerator(void)
{
	if(m_pToolTip!=NULL){
		char szAnd[2] = {GetAndChar(m_pToolTip->m_szName), 0};
		_strupr(szAnd);
		if(szAnd[0]==0) return -1;
		return szAnd[0];
	}
	else{
		return -1;
	}
}
/*
void MWidget::SetLabelAccelerator(void)
{
	m_nAcceleratorType = 1;	// use label's & character
	UseAcceleractorAndCharacter(m_szName);
}

void MWidget::SetToolTipAccelerator(void)
{
	m_nAcceleratorType = 2;	// use tooltip's & character
	if(m_pToolTip!=NULL)
		UseAcceleractorAndCharacter(m_pToolTip->m_szName);
	else
		UseAcceleractorAndCharacter(NULL);
}

void MWidget::EnableAccelerator(bool bEnabled)
{
	m_bAcceleratorEnabled = bEnabled;
}
*/

MRECT MWidget::GetClientRect(void)
{
	return MRECT(0, 0, m_Rect.w, m_Rect.h);
}

MRECT MWidget::GetInitialClientRect(void)
{
	return MWidget::GetClientRect();
}

void MWidget::SetZOrder(MZOrder z)
{
	if(m_pParent==NULL) return;

	MWidget* pParent = m_pParent;
	pParent->RemoveChild(this);

	switch(z){
	case MZ_TOP:
		pParent->AddChild(this);
		break;
	case MZ_BOTTOM:
		pParent->InsertChild(this);
		break;
	}
}

MWidget* MWidget::FindExclusiveDescendant(void)
{
	if(m_Exclusive.GetCount()>0) return m_Exclusive.Get(m_Exclusive.GetCount()-1);	// 마지막에 추가된것

	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pChild = m_Children.Get(i);
		MWidget* pExDes = pChild->FindExclusiveDescendant();
		if(pExDes!=NULL) return pExDes;
	}

	return NULL;
}

bool MWidget::IsExclusive(MWidget* pWidget)
{
	for(int i=0; i<m_Exclusive.GetCount(); i++){
		MWidget* pThis = m_Exclusive.Get(i);
		if(pThis==pWidget) return true;
	}
	return false;
}


MWidget* MWidget::Find(MPOINT& p)
{
	if(IsVisible()==false) return NULL;

	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pChild = m_Children.Get(m_Children.GetCount()-i-1);
		MWidget* pFind = pChild->Find(p);
		if(pFind!=NULL) return pFind;
	}

	if(GetScreenRect().InPoint(p)==true)
		return this;

	return NULL;
}

MWidget* MWidget::FindDropAble(MPOINT& p)
{
	if(IsVisible()==false) return NULL;

	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pChild = m_Children.Get(m_Children.GetCount()-i-1);
		MWidget* pFind = pChild->FindDropAble(p);
		if(pFind!=NULL) return pFind;
	}

	if(GetScreenRect().InPoint(p)==true)
		if(m_bFocusEnable==true)
			return this;

	return NULL;
}


bool MWidget::Drop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	return OnDrop(pSender, pBitmap, szString, szItemString);
}

void MWidget::GetHierarchicalName(char* szName)
{
	if(m_pParent!=NULL){
		m_pParent->GetHierarchicalName(szName);
		strcat(szName, "/");
		strcat(szName, m_szName);
	}
	else{
		strcpy(szName, m_szName);
	}
}

MWidget* MWidget::FindWidgetByHierarchicalName(const char* szName)
{
	char szHierachicalName[2048];
	GetHierarchicalName(szHierachicalName);
	if(strcmp(szName, szHierachicalName)==0){
		return this;
	}

	for(int i=0; i<GetChildCount(); i++){
		MWidget* pFind = GetChild(i)->FindWidgetByHierarchicalName(szName);
		if(pFind!=NULL) return pFind;
	}

	return NULL;
}

bool MWidget::IsMsg(const char* szMsg1, const char* szMsg2)
{
	// 대소문자를 안가린다.
	char szTmp1[256];
	char szTmp2[256];
	strcpy(szTmp1, szMsg1);
	strcpy(szTmp2, szMsg2);
	_strupr(szTmp1);
	_strupr(szTmp2);
	if(strcmp(szTmp1, szTmp2)==0) return true;
	return false;
}

void* MWidget::Query(const char* szQuery)
{
	return NULL;
}

void MWidget::SetOpacity(unsigned char nOpacity)
{
	m_nOpacity = nOpacity;
	/*
	for(int i=0; i<m_Children.GetCount(); i++)
	{
		MWidget* pCurWnd = m_Children.Get(i);
		if(pCurWnd==GetLatestExclusive()) continue;
		pCurWnd->SetOpacity(nOpacity);
	}
	*/
}

unsigned char MWidget::GetOpacity()
{
	return m_nOpacity;
}

int GetAndPos(const char* szText)
{
	if(szText==NULL) return -1;
	int nLen = strlen(szText);
	for(int i=0; i<nLen; i++){
		if(szText[i]=='&') return i;
	}
	return -1;
}

char GetAndChar(const char* szText)
{
	int nLen = GetAndPos(szText);
	if(nLen==-1) return 0;
	return szText[nLen+1];
}

int RemoveAnd(char* szText)
{
	if(szText==NULL) return -1;
	int nPos = GetAndPos(szText);
	if(nPos==-1) return -1;
	int nLen = strlen(szText);
	for(int i=0; i<(nLen-nPos); i++){
		szText[nPos+i] = szText[nPos+i+1];
	}
	return nPos;
}

int RemoveAnd(char* szRemovedText, const char* szText)
{
	if(szText==NULL) return -1;
	strcpy(szRemovedText, szText);
	return RemoveAnd(szRemovedText);
}

int RemoveAnd(char* szRemovedFrontText, char* cUnderLineChar, char* szRemovedBackText, const char* szText)
{
	if(szText==NULL) return -1;
	int nPos = GetAndPos(szText);
	if(nPos==-1) return -1;
	memcpy(szRemovedFrontText, szText, nPos);
	szRemovedFrontText[nPos] = 0;
	*cUnderLineChar = szText[nPos+1];
	strcpy(szRemovedBackText, szText+nPos+2);
	return nPos;
}

MPOINT MClientToScreen(MWidget* pWidget, MPOINT& p)
{
	MRECT r = pWidget->GetScreenRect();
	return MPOINT(p.x+r.x, p.y+r.y);
}

MPOINT MScreenToClient(MWidget* pWidget, MPOINT& p)
{
	MRECT r = pWidget->GetScreenRect();
	return MPOINT(p.x-r.x, p.y-r.y);
}

MRECT MClientToScreen(MWidget* pWidget, MRECT& p)
{
	MRECT r = pWidget->GetScreenRect();
	return MRECT(p.x+r.x, p.y+r.y, p.w, p.h);
}

MRECT MScreenToClient(MWidget* pWidget, MRECT& p)
{
	MRECT r = pWidget->GetScreenRect();
	return MRECT(p.x-r.x, p.y-r.y, p.w, p.h);
}

MPOINT GetCursorPosition(void)
{
	POINT p;
	GetCursorPos(&p);
	MPOINT mp;
	mp.x = p.x;
	mp.y = p.y;
	return mp;
}