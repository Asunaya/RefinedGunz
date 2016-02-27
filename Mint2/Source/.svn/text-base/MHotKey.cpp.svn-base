#include "stdafx.h"
#include "MHotKey.h"
#include "Mint.h"

IMPLEMENT_LOOK(MHotKey, MEditLook)

void GetKeyName(char* szKeyName, int nKeyNameSize, UINT nVirtKey, bool bExtended)
{
	if(nVirtKey==VK_SHIFT || nVirtKey==VK_CONTROL || nVirtKey==VK_MENU){
		szKeyName[0] = 0;
		return;
	}

	LONG lScan = MapVirtualKey(nVirtKey, 0) << 16;

	// if it's an extended key, add the extended flag
	if (bExtended)
		lScan |= 0x01000000L;
	//lScan &= ~VK_SHIFT;
	//lScan &= ~VK_CONTROL;

	GetKeyNameText(lScan, szKeyName, nKeyNameSize);
}

/*
static const TCHAR szPlus[] = _T(" + ");
CString CHotKeyCtrl::GetHotKeyName() const
{
	ASSERT(::IsWindow(m_hWnd));

	CString strKeyName;
	WORD wCode;
	WORD wModifiers;

	GetHotKey(wCode, wModifiers);
	if (wCode != 0 || wModifiers != 0)
	{
		if (wModifiers & HOTKEYF_CONTROL)
		{
			strKeyName += GetKeyName(VK_CONTROL, FALSE);
			strKeyName += szPlus;
		}

		if (wModifiers & HOTKEYF_SHIFT)
		{
			strKeyName += GetKeyName(VK_SHIFT, FALSE);
			strKeyName += szPlus;
		}

		if (wModifiers & HOTKEYF_ALT)
		{
			strKeyName += GetKeyName(VK_MENU, FALSE);
			strKeyName += szPlus;
		}

		strKeyName += GetKeyName(wCode, wModifiers & HOTKEYF_EXT);
	}

	return strKeyName;
}
*/


void MHotKey::OnRun(void)
{
	if(IsFocus()==false) return;

	char szHotKeyName[128] = "";
	GetHotKeyName(szHotKeyName);
	SetText(szHotKeyName);
}

bool MHotKey::OnEvent(MEvent* pEvent, MListener* pListener)
{
	switch(pEvent->nMessage){
	case MWM_KEYDOWN:
		m_nKey = pEvent->nKey;
		if(pEvent->nKey==VK_SHIFT || pEvent->nKey==VK_CONTROL || pEvent->nKey==VK_MENU) m_nKey = -1;
		m_bCtrl = MEvent::GetCtrlState();
		m_bAlt = MEvent::GetAltState();
		m_bShift = MEvent::GetShiftState();
		return true;
	case MWM_KEYUP:
		if(pEvent->nKey==VK_SHIFT && m_nKey==-1) m_bShift = false;
		if(pEvent->nKey==VK_CONTROL && m_nKey==-1) m_bCtrl = false;
		if(pEvent->nKey==VK_MENU && m_nKey==-1) m_bAlt = false;
		return true;
	case MWM_CHAR:
		return true;
	}
	return false;
}

MHotKey::MHotKey(const char* szName, MWidget* pParent, MListener* pListener)
: MEdit(szName, pParent, pListener)
{
	LOOK_IN_CONSTRUCTOR();

	m_bCtrl = false;
	m_bAlt = false;
	m_bShift = false;
	m_nKey = -1;
}

void MHotKey::GetHotKeyName(char* szHotKeyName)
{
	if(m_bCtrl==true){
		strcat(szHotKeyName, "Ctrl");
	}
	if(m_bAlt==true){
		if(szHotKeyName[0]!=0) strcat(szHotKeyName, "+");
		strcat(szHotKeyName, "Alt");
	}
	if(m_bShift==true){
		if(szHotKeyName[0]!=0) strcat(szHotKeyName, "+");
		strcat(szHotKeyName, "Shift");
	}

	if(m_nKey>0){
		char szKey[128];
		GetKeyName(szKey, 128, m_nKey, false);
		if(szKey[0]!=0){
			if(szHotKeyName[0]!=0) strcat(szHotKeyName, "+");
			strcat(szHotKeyName, szKey);
		}
	}
}

void MHotKey::GetHotKey(unsigned int* pKey, bool* pCtrl, bool* pAlt, bool* pShift)
{
	if(pKey!=NULL) *pKey = m_nKey;
	if(pCtrl!=NULL) *pCtrl = m_bCtrl;
	if(pAlt!=NULL) *pAlt = m_bAlt;
	if(pShift!=NULL) *pShift = m_bShift;
}

int MHotKey::RegisterHotKey(void)
{
	return Mint::GetInstance()->RegisterHotKey((m_bCtrl?MMODIFIER_CTRL:0)|(m_bAlt?MMODIFIER_ALT:0)|(m_bShift?MMODIFIER_SHIFT:0), m_nKey);
}

void MHotKey::UnregisterHotKey(int nID)
{
	Mint::GetInstance()->UnregisterHotKey(nID);
}
