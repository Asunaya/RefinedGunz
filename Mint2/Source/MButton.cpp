#include "stdafx.h"
#include "MButton.h"
#include "MColorTable.h"
#include "MTooltip.h"
#include "MMsgBox.h"
#include "Mint.h"

#define MBUTTON_DEFAULT_ALIGNMENT_MODE	(MAM_HCENTER|MAM_VCENTER)

/*
MButtonLook MButton::m_DefaultLook;
MButtonLook* MButton::m_pLook = &MButton::m_DefaultLook;
*/

IMPLEMENT_LOOK(MButton, MButtonLook)

/*
void MButton::OnDraw(MDrawContext* pDC)
{
	m_pLook->OnDraw(this, pDC);
}
*/

void MButton::OnMouseIn(void)
{
}

void MButton::OnMouseOut(void)
{
}

void MButton::OnButtonDown(void)
{
}

void MButton::OnButtonUp(void)
{
}

void MButton::OnButtonClick(void)
{
	if(GetListener()) GetListener()->OnCommand(this, MBTN_CLK_MSG);
}

bool MButton::OnEvent(MEvent* pEvent, MListener* pListener)
{
	MRECT r = GetInitialClientRect();

	m_ClickPos = pEvent->Pos;

	switch(pEvent->nMessage){
	case MWM_LBUTTONDOWN:
		if(r.InPoint(pEvent->Pos)==true){
			m_bLButtonDown = true;

			m_LDragStartClickPos = pEvent->Pos;
			m_LDragVariationX = 0;
			m_LDragVariationY = 0;

			OnButtonDown();
			if(pListener!=NULL) pListener->OnCommand(this, MBTN_DN_MSG);
			SetCapture();
			return true;
		}
		break;

	case MWM_LBUTTONUP:
		if(m_bLButtonDown==true){
			m_bLButtonDown = false;

			m_LDragVariationX = 0;
			m_LDragVariationY = 0;

			OnButtonUp();
			ReleaseCapture();
			if(r.InPoint(pEvent->Pos)==true){
				if(GetType()==MBT_PUSH || GetType()==MBT_PUSH2 ) 
				{
					if(m_pButtonGroup)
						SetCheck(true);
					else
						SetCheck(!m_bChecked);
				}

				if(m_pMsgBox!=NULL) m_pMsgBox->Show(true, true);
				else if(pListener!=NULL) {
					pListener->OnCommand(this, MBTN_UP_MSG);
					OnButtonClick();
				}
				return true;
			}
		}
		break;

	case MWM_LBUTTONDBLCLK:
		if(r.InPoint(pEvent->Pos)==true){
			if(m_pMsgBox!=NULL) m_pMsgBox->Show(true, true);
			else OnButtonClick();
			return true;
		}
		break;

	case MWM_RBUTTONDBLCLK:
		if(r.InPoint(pEvent->Pos)==true){
			if(m_pMsgBox!=NULL) m_pMsgBox->Show(true, true);
			else if(pListener!=NULL) pListener->OnCommand(this, MBTN_RCLK_MSG);
			return true;
		}
		break;

	case MWM_RBUTTONDOWN:
		if(r.InPoint(pEvent->Pos)==true){
			m_bRButtonDown = true;
			if(pListener!=NULL) pListener->OnCommand(this, MBTN_RDN_MSG);
			SetCapture();
			return true;
		}
		break;

	case MWM_RBUTTONUP:
		if(m_bRButtonDown==true){
			m_bRButtonDown = false;
			ReleaseCapture();
			if(r.InPoint(pEvent->Pos)==true && pListener!=NULL){
				if(GetType()==MBT_PUSH && GetType() == MBT_PUSH2 ) m_bChecked = !m_bChecked;
				pListener->OnCommand(this, MBTN_RUP_MSG);
				pListener->OnCommand(this, MBTN_RCLK_MSG);
			}
			return true;
		}
		break;
	case MWM_MOUSEMOVE:
		if(r.InPoint(pEvent->Pos)==true){
			if(m_bMouseOver==false){
				m_bMouseOver = true;
				OnMouseIn();
				if(pListener!=NULL) pListener->OnCommand(this, MBTN_IN_MSG);
			}

			if(m_bLButtonDown) {//drag
				m_LDragVariationX = m_LDragStartClickPos.x - pEvent->Pos.x;
				m_LDragVariationY = m_LDragStartClickPos.y - pEvent->Pos.y;
			}
		}
		else{
			if(m_bMouseOver==true){
				m_bMouseOver = false;
				OnMouseOut();
				if(pListener!=NULL) pListener->OnCommand(this, MBTN_OUT_MSG);
			}
		}
		return false;
	case MWM_KEYDOWN:
		if(m_bEnableEnter==true && IsFocus()==true){
			if(pEvent->nKey==VK_RETURN){
				if(m_pMsgBox!=NULL) m_pMsgBox->Show(true, true);
				else OnButtonClick();
				return true;
			}
		}
		break;
	}
	return false;
}

bool MButton::OnShow(void)
{
	m_bMouseOver = false;
	m_bLButtonDown = false;
	if(GetToolTip()!=NULL) GetToolTip()->Show(false);
	return true;
}

void MButton::OnHide(void)
{
	if(GetToolTip()!=NULL) GetToolTip()->Show(false);
	if(m_pMsgBox) m_pMsgBox->Show(false);
}

MButton::MButton(const char* szName, MWidget* pParent, MListener* pListener)
: MWidget(szName, pParent, pListener), m_TextColor(0, 255, 0)//, m_TextOffset(0, 0)
{
	m_bMouseOver = false;
	m_bLButtonDown = false;
	m_bShowText = true;

	m_AlignmentMode = MBUTTON_DEFAULT_ALIGNMENT_MODE;
	m_Type = MBT_NORMAL;
	m_bChecked = false;

	m_bEnableEnter = true;
	m_bHighlight = true;

	m_pIcon = NULL;

	m_nKeyAssigned = MBKA_NONE;

	SetFocusEnable(true);

	m_pMsgBox = NULL;

	m_ClickPos = MPOINT(0,0);

	m_LDragStartClickPos = MPOINT(0,0);
	m_LDragVariationX = 0;
	m_LDragVariationY = 0;

	m_bComboDropped = false;
	m_bStretch = false;

	m_pButtonGroup = NULL;
	m_nIndexInGroup = 0;

	LOOK_IN_CONSTRUCTOR()
}

MButton::~MButton(void)
{
	if(m_pMsgBox!=NULL) delete m_pMsgBox;
}


/*
void MButton::SetTextOffset(MPOINT p)
{
	m_TextOffset = p;
}
*/
void MButton::SetTextColor(MCOLOR color)
{
	m_TextColor = color;
}

MCOLOR MButton::GetTextColor(void)
{
	return m_TextColor;
}

void MButton::ShowText(bool bShow)
{
	m_bShowText = bShow;
}

bool MButton::DefaultCommand(void)
{
	MListener* pListener = GetListener();
	if(m_pMsgBox!=NULL) m_pMsgBox->Show(true, true);
	else OnButtonClick(); 
	return true;
}

MAlignmentMode MButton::GetAlignment(void)
{
	return m_AlignmentMode;
}

MAlignmentMode MButton::SetAlignment(MAlignmentMode am)
{
	MAlignmentMode temp = m_AlignmentMode;
	m_AlignmentMode = am;
	return temp;
}

void MButton::SetType(MButtonType t)
{
	m_Type = t;
}

MButtonType MButton::GetType(void)
{
	return m_Type;
}

void MButton::SetCheck(bool bCheck)
{
	if(bCheck && m_pButtonGroup)
	{
		m_pButtonGroup->m_pPrevious->SetCheck(false);
		m_pButtonGroup->m_pPrevious=this;
	}

	m_bChecked = bCheck;
}

bool MButton::GetCheck(void)
{
	return m_bChecked;
}

bool MButton::IsButtonDown(void)
{
	if(m_bLButtonDown==true && m_bMouseOver==true) return true;
	return false;
}

bool MButton::IsMouseOver(void)
{
	return m_bMouseOver;
}

void MButton::SetConfirmMessageBox(const char* szMessage)
{
	if(szMessage==NULL || szMessage[0]==0){
		if(m_pMsgBox!=NULL) delete m_pMsgBox;
		m_pMsgBox = NULL;
		return;
	}

	
	//if(m_pMsgBox==NULL) m_pMsgBox = new MMsgBox(szMessage, GetParent(), MT_OKCANCEL, this);
	if(m_pMsgBox==NULL) {
		m_pMsgBox = (MMsgBox*)Mint::GetInstance()->NewWidget(MINT_MSGBOX, "", Mint::GetInstance()->GetMainFrame(), this );
		m_pMsgBox->SetType(MT_OKCANCEL);
//		m_pMsgBox->SetTitle(szMessage);		// title 은 look의 default title로 대체
	}
	
	m_pMsgBox->SetText(szMessage);
}

bool MButton::OnCommand(MWidget* pWidget, const char* szMessage)
{
	if(pWidget==m_pMsgBox){
		m_pMsgBox->Show(false);
		MListener* pListener = GetListener();
		if(strcmp(szMessage, MMSGBOX_OK)==0) if(pListener!=NULL) pListener->OnCommand(this, MBTN_CLK_MSG);
		return true;
	}
	return false;
}

void MButton::SetButtonGroup(MButtonGroup *pGroup)
{
	m_pButtonGroup = pGroup;
	m_nIndexInGroup = pGroup->m_nCount;
	pGroup->m_nCount++;
	
	if(GetCheck()==true)
		pGroup->m_pPrevious = this;
}

void MButtonLook::OnDrawText(MButton* pButton, MRECT& r, MDrawContext* pDC)
{
	if( !pButton->IsVisible() ) return;
	if(pButton->m_pIcon!=NULL){
		pDC->SetBitmap(pButton->m_pIcon);
		pDC->Draw(r.x, r.y+(r.h-pButton->m_pIcon->GetHeight())/2);
		r.x+=pButton->m_pIcon->GetWidth();
		r.w-=pButton->m_pIcon->GetWidth();
	}

	pDC->SetColor(MCOLOR(DEFCOLOR_MBUTTON_TEXT));
	if(pButton->m_bHighlight==true){
		MCOLOR PrevHCol = pDC->SetHighlightColor(MCOLOR(DEFCOLOR_PEN_HIGHLIGHT));
		pDC->TextWithHighlight(r, pButton->m_szName, pButton->GetAlignment());
		pDC->SetHighlightColor(PrevHCol);
	}
	else{
		pDC->Text(r, pButton->m_szName, pButton->GetAlignment());
	}
}

void MButtonLook::OnDownDraw(MButton* pButton, MDrawContext* pDC)
{
	MRECT r = pButton->GetInitialClientRect();
	pDC->SetColor(MCOLOR(DEFCOLOR_MBUTTON_DARKPLANE));
	pDC->FillRectangle(r);

	OnDrawText(pButton, pButton->GetClientRect(), pDC);
}

void MButtonLook::OnUpDraw(MButton* pButton, MDrawContext* pDC)
{
	MRECT r = pButton->GetInitialClientRect();
	pDC->SetColor(MCOLOR(DEFCOLOR_MBUTTON_PLANE));
	pDC->FillRectangle(r);

	OnDrawText(pButton, pButton->GetClientRect(), pDC);
}

void MButtonLook::OnOverDraw(MButton* pButton, MDrawContext* pDC)
{
	if(pButton->GetType()==MBT_PUSH){
		//if(pButton->GetCheck()==true) OnDownDraw(pButton, pDC);
		//else OnUpDraw(pButton, pDC);
		OnCheckBoxDraw(pButton, pDC, pButton->GetCheck());
	}
	else{
		MRECT r = pButton->GetInitialClientRect();
		pDC->SetColor(MCOLOR(DEFCOLOR_MBUTTON_PLANE));
		pDC->FillRectangle(r);

		OnDrawText(pButton, pButton->GetClientRect(), pDC);
	}
}

void MButtonLook::OnDisableDraw(MButton* pButton, MDrawContext* pDC)
{
	MRECT r = pButton->GetInitialClientRect();
	pDC->SetColor(MCOLOR(DEFCOLOR_MBUTTON_PLANE));
	pDC->FillRectangle(r);

	OnDrawText(pButton, pButton->GetClientRect(), pDC);
}

void MButtonLook::OnDraw(MButton* pButton, MDrawContext* pDC)
{
 	if(pButton->GetType()==MBT_PUSH) OnCheckBoxDraw(pButton, pDC, pButton->GetCheck());
	else if(pButton->IsEnable()==false) OnDisableDraw(pButton, pDC);
	else if(pButton->IsButtonDown()==true) OnDownDraw(pButton, pDC);
	else if(pButton->IsMouseOver()==true) OnOverDraw(pButton, pDC);
	else{
		//if(pButton->GetType()==MBT_PUSH){
		//	//if(pButton->GetCheck()) OnDownDraw(pButton, pDC);
		//	//else OnUpDraw(pButton, pDC);
		//	OnCheckBoxDraw(pButton, pDC, pButton->GetCheck());
 	//	}
		//else
			OnUpDraw(pButton, pDC);
	}
}

MRECT MButtonLook::GetClientRect(MButton* pButton, MRECT& r)
{
	return r;
}

////////////////////////////////////
// MButtonGroup

MButtonGroup::MButtonGroup()
{
	m_nCount = 0;
	m_pPrevious = NULL;
}

MButtonGroup::~MButtonGroup()
{
}
