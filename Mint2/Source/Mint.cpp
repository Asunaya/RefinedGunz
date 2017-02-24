#include "stdafx.h"
#include "Mint.h"
#include "MWidget.h"
#include "TChar.h"
#include "MFileDialog.h"
#include "MPopupMenu.h"
#include "MListBox.h"
#include "MComboBox.h"
#include "MGroup.h"
#include "MSlider.h"
#include "MHyperText.h"
#include "MTextArea.h"
#include "MResource.h"
#include "MDragObject.h"
#include "MPicture.h"
#include "MAnimation.h"
#include "MHotKey.h"
#include "MBmButton.h"
#include "MTabCtrl.h"
#include "MPanel.h"
#include "GlobalTypes.h"

class MMainFrame : public MWidget{
protected:
	virtual void OnDraw(MDrawContext* pDC){
		// Draw Nothing
	}
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		return false;
	}
public:
	MMainFrame(const char* szName = nullptr, MWidget* pParent = nullptr, MListener* pListener = nullptr)
		: MWidget(szName, pParent, pListener){
	}
	virtual ~MMainFrame() = default;
};


Mint* Mint::m_pInstance;

void Mint::DrawCandidateList(MDrawContext* pDC, MPOINT& p)
{
	if(GetCandidateCount()>0){
		MCOLOR c = pDC->GetColor();

		MRECT r(p.x, p.y, GetCandidateListWidth(), GetCandidateListHeight());

		pDC->SetColor(MCOLOR(0xFF050505));
		pDC->FillRectangle(r);

		pDC->SetColor(MCOLOR(0xFF505050));
		pDC->Rectangle( r.x+1, r.y+1, r.w,   r.h);

		pDC->SetColor(MCOLOR(0xFFB0B0B0));
		pDC->Rectangle(r);

		MFont* pFont = pDC->GetFont();
		pDC->SetFont( MFontManager::Get( "Default"));

		int nStart = GetCandidatePageStart();

		char temp[MIMECOMPOSITIONSTRING_LENGTH+4];
		for(int i=nStart; i<(int)min(GetCandidateCount(), nStart+GetCandidatePageSize()); i++)
		{
			const char* szCandidate = GetCandidate(i);
			if(i==GetCandidateSelection())
				pDC->SetColor(MCOLOR(DEFCOLOR_NORMAL));
			else
				pDC->SetColor(MCOLOR(0xFF909090));
			int nIndexInPage = i-nStart;
			sprintf_safe(temp, "%d: %s", nIndexInPage+1, szCandidate);
			pDC->Text(p.x+4, p.y + nIndexInPage*pDC->GetFont()->GetHeight() + 4, temp);
		}

		sprintf_safe(temp, "(%d/%d)", GetCandidateSelection()+1, GetCandidateCount());
		pDC->SetColor(MCOLOR(DEFCOLOR_NORMAL));
		pDC->Text(p.x + 4, p.y + GetCandidatePageSize()*pDC->GetFont()->GetHeight() + 4, temp);

		pDC->SetColor(c);
		pDC->SetFont( MFontManager::Get( pFont->m_szName));
	}
}


Mint::Mint()
{
	_ASSERT(m_pInstance==NULL);	// Singleton!!!
	m_pInstance = this;
	m_pMainFrame = NULL;
	m_pDC = NULL;
	m_hImc = NULL;

	m_szDragObjectString[0] = 0;
	m_szDragObjectItemString[0] = 0;
	m_pDragObjectBitmap = NULL;
	m_bVisibleDragObject = false;
	m_pDropableObject = NULL;
	m_pDragSourceObject = NULL;

	m_nWorkspaceWidth = 640;
	m_nWorkspaceHeight = 480;

	m_fnGlobalEventCallBack = NULL;

	m_pCandidateList = NULL;
	m_nCandidateListSize = 0;

	m_nCompositionAttributeSize = 0;
	memset(m_nCompositionAttributes, 0, sizeof(BYTE)*(MIMECOMPOSITIONSTRING_LENGTH));

	m_nCompositionCaretPosition = 0;

	m_bEnableIME = false;
}

Mint::~Mint()
{
	m_fnGlobalEventCallBack = NULL;
	m_pInstance = NULL;
}

Mint* Mint::GetInstance()
{
	_ASSERT(m_pInstance!=NULL);
	return m_pInstance;
}

bool Mint::Initialize(int nWorkspaceWidth, int nWorkspaceHeight, MDrawContext* pDC, MFont* pDefaultFont)
{
	_ASSERT(m_pMainFrame==NULL);

	m_pDC = pDC;

	MFontManager::SetDefaultFont(pDefaultFont);

	m_pMainFrame = new MMainFrame("Mint");
	m_pMainFrame->SetBounds(0, 0, nWorkspaceWidth, nWorkspaceHeight);

	SetWorkspaceSize(nWorkspaceWidth, nWorkspaceHeight);

	return true;
}

void Mint::Finalize()
{
	if(m_pMainFrame!=NULL){
		delete m_pMainFrame;
		m_pMainFrame = NULL;
	}

	MFontManager::Destroy();

	m_pDC = NULL;
}

bool Mint::ProcessEvent(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	if(!m_pMainFrame) return false;

	MEvent e;
	int nResult = e.TranslateEvent(hwnd, message, wparam, lparam);
	if(nResult&EVENT_MINT_TRANSLATED){
		if(m_pDragSourceObject!=NULL){
#define DRAm_VISIBLE_LENGTH	2
			if(e.nMessage==MWM_MOUSEMOVE){
				MPOINT p = e.Pos;
				int px = m_GrabPoint.x - p.x;
				if ( px < 0)  px *= -1;
				int py = m_GrabPoint.y - p.y;
				if ( py < 0)  py *= -1;

				if( m_bVisibleDragObject==false &&
				    ((px > DRAm_VISIBLE_LENGTH) || (py > DRAm_VISIBLE_LENGTH)))
                    m_bVisibleDragObject = true;
				MWidget* pFind = FindWidget(p);
				if(pFind!=NULL && pFind->IsDropable(m_pDragSourceObject)==true)
					m_pDropableObject = pFind;
				else
					m_pDropableObject = NULL;
			}
			if(e.nMessage==MWM_LBUTTONUP){
				MPOINT p = e.Pos;
				MWidget* pFind = FindWidgetDropAble(p);
				if(pFind!=NULL && pFind->IsDropable(m_pDragSourceObject)==true)
					pFind->Drop(m_pDragSourceObject, m_pDragObjectBitmap, m_szDragObjectString, m_szDragObjectItemString);
				m_pDragSourceObject = NULL;
				m_pMainFrame->ReleaseCapture();
				return true;
			}
		}

		if (m_fnGlobalEventCallBack) {
			if (m_fnGlobalEventCallBack(&e) == true) return true;
		}

		if(m_pMainFrame->Event(&e)==true) return true;
		if(m_pMainFrame->EventAccelerator(&e)==true) return true;
		if(m_pMainFrame->EventDefaultKey(&e)==true) return true;

	}
	if(nResult&EVENT_PROCESSED) return true;

	return false;
}

void Mint::Run(void)
{
	if(!m_pMainFrame) return;

	m_pMainFrame->Run();
}

void Mint::Draw(void)
{
	if(!m_pMainFrame) return;

	MDrawContext* pDC = m_pDC;
	if(pDC==NULL) return;

	pDC->SetOpacity(0xFF);
	m_pMainFrame->Draw(pDC);

	MRECT sr = m_pMainFrame->GetScreenRect();
	pDC->SetOrigin(MPOINT(sr.x, sr.y));
	pDC->SetClipRect(sr);

	DrawCandidateList(pDC, m_CandidateListPos);

	MWidget* pDragSourceObject = GetDragObject();
	if(pDragSourceObject!=NULL && m_bVisibleDragObject==true){
		if(m_pDropableObject!=NULL){
			MRECT r = m_pDropableObject->GetScreenRect();
			pDC->SetColor(0, 0, 0, 32);
			pDC->FillRectangle(r);
		}

		pDC->SetBitmap(m_pDragObjectBitmap);
		MPOINT p = MEvent::GetMousePos();
		p.x-=(m_pDragObjectBitmap!=NULL?m_pDragObjectBitmap->GetWidth()/2:0);
		p.y-=(m_pDragObjectBitmap!=NULL?m_pDragObjectBitmap->GetHeight()/2:0);
		if (m_pDragObjectBitmap)
		{
			pDC->SetOpacity(128);
			pDC->Draw(p.x, p.y, m_pDragObjectBitmap->GetWidth(), m_pDragObjectBitmap->GetHeight());
			pDC->SetOpacity(0xFF);
		}
		p.x+=((m_pDragObjectBitmap!=NULL?m_pDragObjectBitmap->GetWidth():0) + 2);
		p.y+=((m_pDragObjectBitmap!=NULL?m_pDragObjectBitmap->GetHeight()/2:0)-pDC->GetFont()->GetHeight()/2);
		pDC->SetColor(0, 0, 0, 128);
		pDC->Text(p, m_szDragObjectString);
		pDC->SetColor(255, 255, 255, 128);
		p.x--;
		p.y--;
		pDC->Text(p, m_szDragObjectString);
	}
	
}

MWidget* Mint::GetMainFrame(void)
{
	return m_pMainFrame;
}

MDrawContext* Mint::GetDrawContext(void)
{
	return m_pDC;
}

#ifdef WIN32
HWND m_hWnd = NULL;
void Mint::SetHWND(HWND hWnd)
{
	m_hWnd = hWnd;

	HWND imehwnd=ImmGetDefaultIMEWnd(Mint::GetInstance()->GetHWND());

	LRESULT lr=SendMessage(imehwnd,WM_IME_CONTROL ,IMC_CLOSESTATUSWINDOW,0);
	_ASSERT(lr==0);

	HIMC	hImc;
	hImc = ImmGetContext(Mint::GetInstance()->GetHWND());
	if (hImc)
	{
		// Get current IME status
		DWORD dwIMEConvMode, dwSentMode;
		ImmGetConversionStatus(hImc, &dwIMEConvMode, &dwSentMode);
		ImmSetConversionStatus(hImc,IME_CMODE_ALPHANUMERIC, dwSentMode);

		Mint::EnableIME(false);
	}
}
HWND Mint::GetHWND(void)
{
	return m_hWnd;
}
#endif

void Mint::EnableIME(bool bEnable)
{
	_ASSERT(GetHWND());
	if ( (bEnable == true) && MEvent::GetIMESupport()) {
		if (m_hImc) {
			ImmAssociateContext(GetHWND(), m_hImc);
			m_hImc = NULL;
			::SetFocus(GetHWND());
		}
		m_bEnableIME = true;
	} else {
		m_hImc = ImmGetContext(GetHWND());
		if (m_hImc) {
			ImmAssociateContext(GetHWND(), NULL);
			ImmReleaseContext(GetHWND(), m_hImc);
			::SetFocus(GetHWND());
		}
		m_bEnableIME = false;
	}

	HIMC hImc = ImmGetContext(GetInstance()->GetHWND());
	if(hImc!=NULL){
		ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
		ImmReleaseContext(GetInstance()->GetHWND(), hImc);
	}
}

bool Mint::IsEnableIME(void)
{
	return m_bEnableIME;
}


int Mint::RegisterHotKey(unsigned long int nModifier, unsigned long int nVirtKey)
{
#ifdef WIN32
	_ASSERT(m_hWnd!=NULL);	// Should call SetHWND() before this function

	char szAtomName[64] = {0,};
	if(nModifier==MMODIFIER_ALT) strcat_safe(szAtomName, "Alt");
	if(nModifier==MMODIFIER_CTRL) strcat_safe(szAtomName, "Ctrl");
	if(nModifier==MMODIFIER_SHIFT) strcat_safe(szAtomName, "Shift");
	char szKey[16] = {0, };
	sprintf_safe(szKey, "%d", nVirtKey);
	strcat_safe(szAtomName, szKey);

	int nID = GlobalAddAtom(szAtomName);
	if(nID==0) return 0;

	if(::RegisterHotKey(m_hWnd, nID, nModifier, nVirtKey)==TRUE)
		return nID;

	return 0;
#else
	// Not Implemented
	_ASSERT(FALSE);
	return 0;
#endif
}
void Mint::UnregisterHotKey(int nID)
{
#ifdef WIN32
	_ASSERT(m_hWnd!=NULL);	// Should call SetHWND() before this function
	::UnregisterHotKey(m_hWnd, nID);
	GlobalDeleteAtom(nID);
#else
	// Not Implemented
	_ASSERT(FALSE);
	return 0;
#endif
}

void Mint::SetGlobalEvent(MGLOBALEVENTCALLBACK pGlobalEventCallback)
{
	m_fnGlobalEventCallBack = pGlobalEventCallback;
}

MWidget* Mint::SetDragObject(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	MWidget* pPrev = m_pDragSourceObject;
	m_pDragSourceObject = pSender;
	if(szString==NULL) m_szDragObjectString[0] = 0;
	else strcpy_safe(m_szDragObjectString, szString);
	if(szItemString==NULL) m_szDragObjectItemString[0] = 0;
	else strcpy_safe(m_szDragObjectItemString, szItemString);
	m_pDragObjectBitmap = pBitmap;
	m_GrabPoint = MEvent::GetMousePos();
	m_bVisibleDragObject = false;
	return pPrev;
}
MWidget* Mint::GetDragObject(void)
{
	return m_pDragSourceObject;
}

/*
#define MINT_WIDGET	"GenericWidget"
#define MINT_LABEL			"Label"
#define MINT_BUTTON			"Button"
#define MINT_EDIT			"Edit"
#define MINT_LISTBOX		"ListBox"
#define MINT_COMBOBOX		"ComboBox"
#define MINT_FILEBOX		"FileBox"
#define MINT_FRAME			"Frame"
#define MINT_GROUP			"Group"
#define MINT_POPUPMENU			"Menu"
#define MINT_SCROLLBAR		"ScrollBar"
#define MINT_SLIDER			"Slider"
#define MINT_TEXTAREA		"TextArea"
#define MINT_MSGBOX		"MessageBox"
*/

/*
char* m_szWidgetClasses[] = {
	MINT_WIDGET,
	MINT_LABEL,
	MINT_BUTTON,
	MINT_EDIT,
	MINT_LISTBOX,
	MINT_COMBOBOX,
	MINT_FILEBOX,
	MINT_FRAME,
	MINT_GROUP,
	MINT_POPUPMENU,
	MINT_SCROLLBAR,
	MINT_SLIDER,
	MINT_TEXTAREA,
	MINT_MSGBOX,
};
*/

MWidget* Mint::NewWidget(const char* szClass, const char* szName, MWidget* pParent, MListener* pListener)
{
	if(strcmp(szClass, MINT_WIDGET)==0) return new MWidget(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_LABEL)==0) return new MLabel(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_BUTTON)==0) return new MButton(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_BMBUTTON)==0) return new MBmButton(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_EDIT)==0) return new MEdit(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_LISTBOX)==0) return new MListBox(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_COMBOBOX)==0) return new MComboBox(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_FILEBOX)==0) return new MFileBox(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_FRAME)==0) return new MFrame(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_GROUP)==0) return new MGroup(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_POPUPMENU)==0) return new MPopupMenu(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_SCROLLBAR)==0) return new MScrollBar(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_SLIDER)==0) return new MSlider(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_TEXTAREA)==0) return new MTextArea(256, szName, pParent, pListener);
	else if(strcmp(szClass, MINT_TABCTRL)==0) return new MTabCtrl(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_MSGBOX)==0) return new MMsgBox(szName, pParent, pListener, MT_OK);
	else if(strcmp(szClass, MINT_PICTURE)==0) return new MPicture(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_ANIMATION)==0) return new MAnimation(szName, NULL, pParent);
	else if(strcmp(szClass, MINT_MENUITEM)==0) return new MMenuItem(szName);
	else if(strcmp(szClass, MINT_HOTKEY)==0) return new MHotKey(szName, pParent, pListener);
//	else if(strcmp(szClass, MINT_ACTIONKEY)==0) return new MActionKey(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_PANEL)==0) return new MPanel(szName, pParent, pListener);

	else _ASSERT(false);

	return new MWidget(szName, pParent, pListener);
}

MWidget* Mint::FindWidgetDropAble(MPOINT& p)
{
	return m_pMainFrame->FindDropAble(p);
}

MWidget* Mint::FindWidget(MPOINT& p)
{
	return m_pMainFrame->Find(p);
}

MWidget* Mint::FindWidget(int x, int y)
{
	return m_pMainFrame->Find(x, y);
}


int Mint::GetWorkspaceWidth(void)
{
	return m_nWorkspaceWidth;
}

int Mint::GetWorkspaceHeight(void)
{
	return m_nWorkspaceHeight;
}

void Mint::SetWorkspaceSize(int w, int h)
{
	m_nWorkspaceWidth = w;
	m_nWorkspaceHeight = h;
}

const char* Mint::GetDefaultFontName(void) const
{
	static char* szFontName[] = {
		"Tahoma",		// Default
		"Gulim",		// �ѱ�
		"MS PGothic",	// �Ϻ�
		"MingLiU",		// ��ü
		"NSimSun"		// ��ü
	};

	int nFont = 0;
	if(GetPrimaryLanguageIdentifier()==LANG_KOREAN) nFont = 1;
	else if(GetPrimaryLanguageIdentifier()==LANG_JAPANESE) nFont = 2;
	else if(GetPrimaryLanguageIdentifier()==LANG_CHINESE){
		if(GetSubLanguageIdentifier()==SUBLANG_CHINESE_TRADITIONAL) nFont = 3;
		else nFont = 4;
	}

	return szFontName[nFont];
}

int Mint::GetPrimaryLanguageIdentifier(void) const
{
	/*
	Language Identifiers
	+-------------------------+-------------------------+
	|      SubLanguage ID     |   Primary Language ID   |
	+-------------------------+-------------------------+
	15                    10  9                         0   bit

	ms-help://MS.VSCC.2003/MS.MSDNQTR.2003FEB.1042/intl/nls_8xo3.htm
	ms-help://MS.VSCC.2003/MS.MSDNQTR.2003FEB.1042/intl/nls_238z.htm
	*/
	HKL hKeyboardLayout = GetKeyboardLayout(0);
	WORD nLanguageIdentifier = LOWORD(hKeyboardLayout);
	return PRIMARYLANGID(nLanguageIdentifier);
}

int Mint::GetSubLanguageIdentifier(void) const
{
	HKL hKeyboardLayout = GetKeyboardLayout(0);
	WORD nLanguageIdentifier = LOWORD(hKeyboardLayout);
	return SUBLANGID(nLanguageIdentifier);
}

const char* Mint::GetLanguageIndicatorString(void) const
{
	static char* szIndicator[] = {
		"?", "EN", "��", "��", "��", "PT"
	};
	switch(GetPrimaryLanguageIdentifier()){
	case LANG_ENGLISH:
		return szIndicator[1];
	case LANG_KOREAN:
		return szIndicator[2];
	case LANG_JAPANESE:
		return szIndicator[3];
	case LANG_CHINESE:
		return szIndicator[4];
	case LANG_PORTUGUESE:
		return szIndicator[5];
	default:
		return szIndicator[0];
	}
}

bool Mint::IsNativeIME(void) const
{
	HIMC hImc = ImmGetContext(GetInstance()->GetHWND());
	if(hImc==NULL) return false;

	bool bNative = false;

	if(GetPrimaryLanguageIdentifier()==LANG_JAPANESE){
		bNative = (ImmGetOpenStatus(hImc)==TRUE);
	}
	else{
		DWORD dwConvMode, dwSentMode;
		ImmGetConversionStatus(hImc, &dwConvMode, &dwSentMode);

		bNative = (dwConvMode&IME_CMODE_NATIVE);
	}

	ImmReleaseContext(GetInstance()->GetHWND(), hImc);

	return bNative;
}

void Mint::OpenCandidateList()
{
	HIMC hImc = ImmGetContext(GetInstance()->GetHWND());
	if(hImc==NULL) return;

	m_nCandidateListSize = ImmGetCandidateList(hImc, 0, NULL, 0);

	if(m_pCandidateList!=NULL) delete[] m_pCandidateList;
	m_pCandidateList = new unsigned char[m_nCandidateListSize];

	if(m_nCandidateListSize>0) { 
		ImmGetCandidateList(hImc, 0, (LPCANDIDATELIST)m_pCandidateList, m_nCandidateListSize);
	}

	ImmReleaseContext(GetInstance()->GetHWND(), hImc);
}

void Mint::CloseCandidateList()
{
	if(m_pCandidateList!=NULL) delete[] m_pCandidateList;
	m_pCandidateList = NULL;
	m_nCandidateListSize = 0;
}

const char* Mint::GetCandidate(int nIndex) const
{
	if(m_pCandidateList==NULL) return NULL;

	CANDIDATELIST* pCandidateList = (CANDIDATELIST*)m_pCandidateList;

	if(nIndex>=(int)pCandidateList->dwCount) return NULL;

	char* pCandidate = (char*)((BYTE*)pCandidateList+pCandidateList->dwOffset[nIndex]);
	return pCandidate;
}

int Mint::GetCandidateCount() const
{
	if(m_pCandidateList==NULL) return 0;

	CANDIDATELIST* pCandidateList = (CANDIDATELIST*)m_pCandidateList;

	return pCandidateList->dwCount;
}

int Mint::GetCandidateSelection() const
{
	if(m_pCandidateList==NULL) return 0;

	CANDIDATELIST* pCandidateList = (CANDIDATELIST*)m_pCandidateList;

	return pCandidateList->dwSelection;
}

int Mint::GetCandidatePageStart() const
{
	if(m_pCandidateList==NULL) return 0;

	int nStart = GetCandidatePageSize() * (GetCandidateSelection()/GetCandidatePageSize());

	return nStart;
}

int Mint::GetCandidatePageSize(void) const
{
	if(m_pCandidateList==NULL) return 0;

	CANDIDATELIST* pCandidateList = (CANDIDATELIST*)m_pCandidateList;

	return pCandidateList->dwPageSize;
}

void Mint::SetCandidateListPosition(MPOINT& p, int nWidgetHeight)
{
	MPOINT cp = p;

	if((cp.x+GetCandidateListWidth())>=MGetWorkspaceWidth()){
		cp.x = MGetWorkspaceWidth()-GetCandidateListWidth();
	}
	
	if((cp.y+GetCandidateListHeight()+nWidgetHeight+8)>=MGetWorkspaceHeight()){
		cp.y -= GetCandidateListHeight() + 6;
	}
	else{
		cp.y += (nWidgetHeight+6);
	}

	m_CandidateListPos = cp;
}

int Mint::GetCandidateListWidth(void)
{
	int w = 60;
	if(GetCandidateCount()>0){
		const char* szCandidate = GetCandidate(0);
		w = max(w, MFontManager::Get("Default")->GetWidth(szCandidate) + 100);
	}
	return w + 4;
}

int Mint::GetCandidateListHeight(void)
{
	return (MFontManager::Get( "Default")->GetHeight()*(GetCandidatePageSize()+1) + 6);
}

int Mint::DrawCompositionAttribute(MDrawContext* pDC, MPOINT& p, const char* szComposition, int i)
{
	if(i>=(int)strlen(szComposition)) return 0;

	const BYTE* pCompAttr = GetCompositionAttributes();
	DWORD nCompAttrSize = GetCompositionAttributeSize();

	if(i>=(int)nCompAttrSize) return 0;

	MFont* pFont = pDC->GetFont();
	int nFontHeight = pFont->GetHeight();
	MCOLOR c = pDC->GetColor();

	int nCharSize = 1;
	bool bTwoByteChar = IsHangul(szComposition[i]);
	if(bTwoByteChar) nCharSize = 2;
	int nWidth = pFont->GetWidth(&(szComposition[i]), nCharSize);

	if(pCompAttr[i]==ATTR_TARGET_CONVERTED)
		pDC->SetColor(MCOLOR(255, 0, 0, 128));
	else if(pCompAttr[i]==ATTR_TARGET_NOTCONVERTED)
		pDC->SetColor(MCOLOR(0, 196, 0, 128));
	else
		pDC->SetColor(MCOLOR(128, 128, 128, 128));

	pDC->FillRectangle(p.x, p.y, nWidth, nFontHeight);

	pDC->SetColor(c);

	return nWidth;
}

void Mint::DrawCompositionAttributes(MDrawContext* pDC, MPOINT& p, const char* szComposition)
{
	if(strlen(szComposition)==0) return;

	int nWidth = 0;

	for(int i=0; szComposition[i]!=NULL; i++){
		bool bTwoByteChar = IsHangul(szComposition[i]);

		nWidth += DrawCompositionAttribute(pDC, MPOINT(p.x+nWidth, p.y), szComposition, i);

		if(bTwoByteChar) i++;
	}
}

void Mint::DrawIndicator(MDrawContext* pDC, MRECT& r)
{
	MFont* pFont = pDC->GetFont();
	int nFontHeight = pFont->GetHeight();

	const char* szLanguageIndicator = GetLanguageIndicatorString();

	int nIdicatorWidth = pFont->GetWidth(szLanguageIndicator);
	MCOLOR c = pDC->GetColor();

	MRECT fr(r.x+r.w-nIdicatorWidth-4, r.y+2, nIdicatorWidth+3, r.h-4);

	if(IsNativeIME()==true){
		pDC->SetColor(MCOLOR(130,130,130));
		pDC->FillRectangle(fr);
		pDC->SetColor(MCOLOR(0, 0, 0));
		pDC->Text(r.x+r.w-nIdicatorWidth-2, r.y + (r.h-nFontHeight)/2, szLanguageIndicator);
	}

	pDC->SetColor(c);
}



// Sample Widgets


TCHAR* szStar[] = {
	_T("������ �������� �ϴÿ���"),
	_T("������ ���� �� �ֽ��ϴ�."),
	_T("���� �ƹ� ������ ����"),
	_T("���� ���� ������ �� ���� ���մϴ�. "),
	_T("���� �ӿ� �ϳ� �� �������� ����  "),
	_T("���� �� ����� ����"),
	_T("���� ��ħ�� ���� ����̿�, "),
	_T("���Ϲ��� ���� ����̿�,"),
	_T("���� ���� û���� ������ ���� ����Դϴ�. "),
	_T("�� �ϳ��� �߾��"),
	_T("�� �ϳ��� �����"),
	_T("�� �ϳ���  �����԰�"),
	_T("�� �ϳ��� �����"),
	_T("�� �ϳ��� �ÿ�"),
	_T("�� �� ���� ��Ӵ�, ��Ӵ�"),
	_T("��Ӵ�, ���� �� �ϳ��� �Ƹ� �ٿ� �� �� ���� �ҷ� ���ϴ�. ���б� �� å���� "),
	_T("�����ߴ� ���̵��� �̸���, ��, ��, ��, �̷� �̱� �ҳ���� �̸���,"),
	_T("���� �Ʊ� ��Ӵ� �� �����ֵ��� �̸���, ������ �̿� ������� �̸���,"),
	_T("��ѱ�, ������, �䳢, ���,  ���, �����ý� ��, ���̳� ������ ����,"),
	_T("�̷� ������ �̸��� �ҷ� ���ϴ�."),
	_T("�̳׵��� �ʹ��� �ָ� �ֽ��ϴ�."),
	_T("���� �ƽ����� �� ����,"),
	_T("��Ӵ�,"),
	_T("�׸��� ����� �ָ� �ϰ����� ��ʴϴ�."),
	_T("���� �������� �׸���"),
	_T("�� ���� ������ ���� ��� ����"),
	_T("�� �̸��ڸ� �� ����,"),
	_T("������ ���� ���������ϴ�."),
	_T("����, ���� ���� ��� ������"),
	_T("�β����� �̸��� �����ϴ� ����Դϴ�."),
	_T("�׷��� �ܿ��� ������ ���� ������ ���� ����,"),
	_T("���� ���� �Ķ� �ܵ� �Ǿ����"),
	_T("�� �̸��� ���� ��� ������,"),
	_T("�ڶ�ó�� Ǯ�� ������ �ſܴ�. "),
};


void MCreateSample()
{
	MWidget* pMainFrame = Mint::GetInstance()->GetMainFrame();
	MWidget* pNew = new MFileDialog("*.*", pMainFrame, pMainFrame);
	pNew->Show(true);

	class MFrameBitmap : public MFrame{
		int			m_nBitmap;
	protected:
		virtual void OnDraw(MDrawContext* pDC){
			MFrame::OnDraw(pDC);

			MRECT r = GetClientRect();

			if(MBitmapManager::GetCount()==0) return;
			m_nBitmap %= MBitmapManager::GetCount();
			pDC->SetBitmap(MBitmapManager::Get(m_nBitmap));
			m_nBitmap++;
			pDC->Draw(r.x, r.y);
		}
	public:
		MFrameBitmap(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL)
			: MFrame(szName, pParent, pListener){
			m_nBitmap = 0;
		}
		virtual ~MFrameBitmap(void){
		}
	};

	MWidget* pNewFrame = new MFrameBitmap("Bitmap Test", pMainFrame, pMainFrame);
	pNewFrame->SetBounds(0, 20, 200, 200);
	pNewFrame->Show(true);

	class MFrameInfo : public MFrame{
		int m_nRenderCount;
		u64 m_nPrevTime;
		int	m_nFPS;
	protected:
		virtual void OnDraw(MDrawContext* pDC){
			MFrame::OnDraw(pDC);

			MRECT r = GetClientRect();
			char temp[256];

			pDC->SetColor(MCOLOR(255,255,255));

			// FPS
			auto nCurrTime = GetGlobalTimeMS();
			if(nCurrTime-m_nPrevTime>1000){
				m_nFPS = m_nRenderCount;
				m_nPrevTime = nCurrTime;
				m_nRenderCount = 0;
			}
			m_nRenderCount++;
			sprintf_safe(temp, "FPS = %d", m_nFPS);
			pDC->Text(r.x, r.y, temp);

			MPOINT p = MEvent::GetMousePos();
			sprintf_safe(temp, "Cursor Pos = %d, %d", p.x, p.y);
			pDC->Text(r.x, r.y+GetFont()->GetHeight(), temp);

		}
	public:
		MFrameInfo(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL)
			: MFrame(szName, pParent, pListener),
			m_nRenderCount(0),
			m_nPrevTime(GetGlobalTimeMS()),
			m_nFPS(0)
		{
		}
		virtual ~MFrameInfo(void){
		}
	};

	MWidget* pNewCursorInfo = new MFrameInfo("����", pMainFrame, pMainFrame);
	pNewCursorInfo->SetBounds(400, 0, 200, 200);
	pNewCursorInfo->Show(true);

	class MFrameStar : public MFrame{
	protected:
		virtual void OnDraw(MDrawContext* pDC){
			MFrame::OnDraw(pDC);

			MRECT r = GetClientRect();
			int nRawSize = sizeof(szStar)/sizeof(char*);
			static int nRaw = 0;
			pDC->SetColor(MCOLOR(128,128,255));
			for(int i=0; i<20; i++){
				pDC->Text(r.x, r.y+i*(GetFont()->GetHeight()+2), szStar[(nRaw+i)%nRawSize]);
			}
			nRaw++;
			nRaw%=nRawSize;
		}
		virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
			return false;
		}
	public:
		MFrameStar(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL)
			: MFrame(szName, pParent, pListener){
		}
		virtual ~MFrameStar(void){
		}
	};

	MWidget* pNewStar = new MFrameStar("�� ��� ��", pMainFrame, pMainFrame);
	pNewStar->SetBounds(30, 250, 500, 200);
	pNewStar->Show(true);

	MPopupMenu* pNewMenu = new MPopupMenu("SampleMenu", pMainFrame, pMainFrame, MPMT_HORIZONTAL);
	MMenuItem* pFile = pNewMenu->AddMenuItem("File");
	MMenuItem* pWidget = pNewMenu->AddMenuItem("Widget");
	MPopupMenu* pFileMenu = pFile->CreateSubMenu();
	MPopupMenu* pWidgetMenu = pWidget->CreateSubMenu();
	pFileMenu->AddMenuItem("Open");
	pFileMenu->AddMenuItem("Exit");
	pWidgetMenu->AddMenuItem("Info");
	pWidgetMenu->AddMenuItem("����¹�");

	pNewMenu->SetBounds(0, 0, MGetWorkspaceWidth(), 16);
	pNewMenu->Show(0, 0, true);
	pNewMenu->Show(true);
}

void MDestroySample()
{
}
