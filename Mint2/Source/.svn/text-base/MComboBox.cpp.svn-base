#include "stdafx.h"
#include "MComboBox.h"
#include "Mint.h"

MComboListBox::MComboListBox(MWidget* pParent, MListener* pListener)
: MListBox(pParent, pListener)
{
	m_bClipByParent = false;
	LOOK_IN_CONSTRUCTOR()
}

bool MComboListBox::OnEvent(MEvent* pEvent, MListener* pListener)
{
	if(MListBox::OnEvent(pEvent, pListener)==true) return true;
	
	switch(pEvent->nMessage){
	case MWM_LBUTTONDOWN:
	case MWM_RBUTTONDOWN:
	case MWM_LBUTTONDBLCLK:
	case MWM_RBUTTONDBLCLK:
		Show(false);
		return false;	// 다른 Widget에게 넘겨준다.
	}

	return false;
}

IMPLEMENT_LOOK(MComboListBox, MListBoxLook)


#define DEFAULT_DROP_HEIGHT	200

MComboBox::MComboBox(const char* szName, MWidget* pParent, MListener* pListener)
: MButton(szName, pParent, pListener)
{
	MButton::SetListener(this);	// Button 메세지는 한번 걸러서 보내준다.

	m_pListBox = new MComboListBox(pParent, this);
	m_pListBox->Show(false);

	m_nDropHeight = DEFAULT_DROP_HEIGHT;

	if(szName!=NULL && szName[0]!=0) Add(szName);

	m_bHighlight = false; // 콤보박스에서는 하이라이트(&)를 지원할 필요가 없다.

	m_pComboBoxListener = pListener;

	m_AlignmentMode = MAM_HCENTER|MAM_VCENTER;

	m_nComboType = 0;
	m_nNextComboBoxTypeSize = 0;

	m_bAutoDrop = true;
	m_bDropUnder = true;
}

MComboBox::~MComboBox(void)
{
	delete m_pListBox;
}

void MComboBox::SetDropSize(int nHeight)
{
	m_nDropHeight = nHeight;
}

void MComboBox::Add(const char* szItem)
{
	m_pListBox->Add(szItem);
}

void MComboBox::Add(MListItem* pItem)
{
	m_pListBox->Add(pItem);
}

const char* MComboBox::GetString(int i)
{
	return m_pListBox->GetString(i);
}

MListItem* MComboBox::Get(int i)
{
	return m_pListBox->Get(i);
}

void MComboBox::Remove(int i)
{
	m_pListBox->Remove(i);
}

void MComboBox::RemoveAll(void)
{
	m_pListBox->RemoveAll();
}

int MComboBox::GetCount(void)
{
	return m_pListBox->GetCount();
}

int MComboBox::GetSelIndex(void)
{
	return m_pListBox->GetSelIndex();
}

bool MComboBox::SetSelIndex(int i)
{
	bool bReturn = m_pListBox->SetSelIndex(i);
	if(bReturn==true) SetText(GetSelItemString());
	return bReturn;
}

const char* MComboBox::GetSelItemString(void)
{
	return m_pListBox->GetSelItemString();
}

MListItem* MComboBox::GetSelItem(void)
{
	return m_pListBox->GetSelItem();
}

void MComboBox::SetNextSel()
{
	int cnt = GetCount();
	int sel = GetSelIndex();

	if(cnt > 1) {
		if(sel+1==cnt) {
			SetSelIndex(0);
		}
		else {
			SetSelIndex(sel+1);
		}

		if(m_pComboBoxListener!=NULL){
			m_pComboBoxListener->OnCommand(this, MCMBBOX_CHANGED);
		}
	}
}

void MComboBox::SetPrevSel()
{
	int cnt = GetCount();
	int sel = GetSelIndex();

	if(cnt > 1) {
		if(sel==0) {
			SetSelIndex(cnt-1);
		} else {
			SetSelIndex( sel-1 );
		}

		if(m_pComboBoxListener!=NULL){
			m_pComboBoxListener->OnCommand(this, MCMBBOX_CHANGED);
		}
	}
}

bool MComboBox::OnCommand(MWidget* pWindow, const char* szMessage)
{
	MRECT r = GetRect();

 	if(pWindow==this && strcmp(szMessage, MBTN_CLK_MSG)==0) {

		if( GetComboDropped() ) 
		{
			m_pListBox->Show(false);
			SetComboDropped(false);
			ReleaseFocus();
			return true;
		}
		
		bool bSCheck = false;

 		if(m_nComboType == 0) { // 2부분으로 나누어처리

			if( GetNextComboBoxTypeSize() > m_ClickPos.x ) {
				bSCheck = true;
			}
		}

		// DropHeight가 넉넉하면 자동으로 크기를 줄인다.

		if(bSCheck) {

			int cnt = GetCount();
			int sel = GetSelIndex();

			if(cnt > 1) {
				if(sel+1==cnt) {
					SetSelIndex(0);
				}
				else {
					SetSelIndex(sel+1);
				}

				if(m_pComboBoxListener!=NULL){
					m_pComboBoxListener->OnCommand(this, MCMBBOX_CHANGED);
				}

			}
		}
		else {
		
			int nFrameHeight = m_pListBox->GetRect().h - m_pListBox->GetClientRect().h;
 			int nDropHeight	 = min(m_nDropHeight,m_pListBox->GetItemHeight()*m_pListBox->GetCount()+nFrameHeight);

			bool bDropUnder = true;

			MPOINT sp = MClientToScreen(this,MPOINT(r.x,r.y+r.h));

			if( ( GetRect().y + GetRect().h + nDropHeight + 10) > MGetWorkspaceHeight())
				bDropUnder = false;

			if(m_bAutoDrop)
			{
				if(bDropUnder==true)	m_pListBox->SetBounds(MRECT(r.x, r.y+r.h, r.w, nDropHeight));
				else					m_pListBox->SetBounds(MRECT(r.x, r.y-nDropHeight, r.w, nDropHeight));
			}
			else
			{
				if(m_bDropUnder)	m_pListBox->SetBounds(MRECT(r.x, r.y+r.h, r.w, nDropHeight));
				else				m_pListBox->SetBounds(MRECT(r.x, r.y-nDropHeight, r.w, nDropHeight));
			}

			m_pListBox->Show(true, true);
			SetComboDropped( true );
		}

		return true;
	}
	else if(pWindow==m_pListBox) {

		if(IsMsg(szMessage, MLB_ITEM_SEL)==true || IsMsg(szMessage, MLB_ITEM_SEL2)==true || IsMsg(szMessage, MLB_ITEM_DBLCLK)==true){
			m_pListBox->Show(false);
			SetComboDropped( false );
			SetText(m_pListBox->GetSelItemString());
			if(m_pComboBoxListener!=NULL){
				m_pComboBoxListener->OnCommand(this, MCMBBOX_CHANGED);
			}
			return true;
		}
	}
	return false;
}

void MComboBox::Sort(void)
{
	m_pListBox->Sort();
}

void MComboBox::SetListener(MListener* pListener)
{
	m_pComboBoxListener = pListener;
}
MListener* MComboBox::GetListener(void)
{
	return MWidget::GetListener();
}

void MComboBox::OnReleaseFocus()
{
	if( !m_pListBox->IsFocus() && GetComboDropped() )
	{
		SetComboDropped(false);
		m_pListBox->Show(false);
	}
}

void MComboBox::CloseComboBoxList()
{
	m_pListBox->Show(false);
	SetComboDropped( false );
}