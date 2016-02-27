#ifndef MIDLRESOURCE_H
#define MIDLRESOURCE_H

#include "MZFileSystem.h"
#include "MXml.h"
#include "MTypes.h"
#include "MWidget.h"
#include "MFrame.h"

#include <string>
#include <list>
#include <map>
#include <algorithm>
using namespace std;

class MFrame;
class MLabel;
class MButton;
class MEdit;
class MListBox;
class MPicture;
class MScrollBar;
class MSlider;
class MGroup;
class MComboBox;
class MToolTip;
class MPopupMenu;
class MAniBitmap;
class MAnimation;
class MBmButton;
class MMenuItem;
class MBmLabel;
class MTextArea;
class MTabCtrl;
class MPanel;
class MButtonGroup;

// Look&Feel
class MBLabelLook;
class MBButtonLook;
class MBGroupLook;
class MBFrameLook;
class MBEditLook;
class MBListBoxLook;
class MBScrollBarLook;
class MBArrowLook;
class MBThumbLook;
class MBSliderLook;
class MBGroupLook;
class MBitmap;
class MHotKey;
//class MActionKey;
class MBTextAreaLook;
class MBSliderThumbLook;
class MBTabCtrlLook;


#define IDL_ROOT	"XML"

#define IDL_BOUNDS		"BOUNDS"
#define IDL_TEXT		"TEXT"
#define IDL_TEXTCOLOR	"TEXTCOLOR"

#define IDL_LABEL		"LABEL"
#define IDL_FRAME		"FRAME"
#define IDL_BUTTON		"BUTTON"

#define IDL_ATTR_ITEM	"item"
#define IDL_ATTR_PARENT	"parent"

class MWidgetMMap : public multimap<string, MWidget*>
{
public:
	virtual ~MWidgetMMap(void){ Clear(); }

	void Clear() {
		while(empty()==false)
		{
			delete (*begin()).second;
			erase(begin());
		}
	}
};

class MWidgetList : public list<MWidget*> { };

/// Maiet Interface Definition Language Resource
class MIDLResource
{
private:

protected:
	MWidget*						m_pParent;		///< 추가될 최상위 위젯

	map<string, MBLabelLook*>		m_LabelLookMap;
	map<string, MBButtonLook*>		m_ButtonLookMap;
	map<string, MBGroupLook*>		m_GroupLookMap;
	map<string, MBFrameLook*>		m_FrameLookMap;
	map<string, MBEditLook*>		m_EditLookMap;
	map<string, MBListBoxLook*>		m_ListBoxLookMap;
	map<string, MBScrollBarLook*>	m_ScrollBarLookMap;
	map<string, MBArrowLook*>		m_ArrowLookMap;
	map<string, MBThumbLook*>		m_ThumbLookMap;
	map<string, MBSliderLook*>		m_SliderLookMap;
	map<string, MAniBitmap*>		m_AniBitmapMap;
	map<string, MBTextAreaLook*>	m_TextAreaLookMap;
	map<string, MBTabCtrlLook*>		m_TabCtrlLookMap;

	MWidgetMMap						m_WidgetMap;

	map<string, MButtonGroup*>		m_ButtonGroupMap;

	// 기본요소
	MPOINT			GetPoint(MXmlElement& element);
	MRECT			GetRect(MXmlElement& element);
	MSIZE			GetSize(MXmlElement& element);
	MCOLOR			GetColor(MXmlElement& element);
	MBitmap*		GetBitmap(MXmlElement& element);
	MBitmap*		GetBitmapAlias(MXmlElement& element);	// 부분bitmap을 특정 이름으로 alias
	MAnchors		GetAnchors(MXmlElement& element);
	MAlignmentMode	GetAlignmentMode(MXmlElement& element);

	// Widget
	MWidget*		GetParentWidget(MXmlElement& element);
	MFrame*			GetFrame(MXmlElement& element);
	MLabel*			GetLabel(MXmlElement& element);
	MButton*		GetButton(MXmlElement& element);
	MBmButton*		GetBmButton(MXmlElement& element);
	MEdit*			GetEdit(MXmlElement& element);
	MListBox*		GetListBox(MXmlElement& element);
	MPicture*		GetPicture(MXmlElement& element);
	MScrollBar*		GetScrollBar(MXmlElement& element);
	MSlider*		GetSlider(MXmlElement& element);
	MGroup*			GetGroup(MXmlElement& element);
	MComboBox*		GetComboBox(MXmlElement& element);
	MPopupMenu*		GetPopupMenu(MXmlElement& element);
	MAniBitmap*		GetAniBitmap(MXmlElement& element);
	MAnimation*		GetAnimation(MXmlElement& element);
	MCursor*		GetCursor(MXmlElement& element);
	MBmLabel*		GetBmLabel(MXmlElement& element);
	MFont*			GetFont(MXmlElement& element);
	MHotKey*		GetHotKey(MXmlElement& element);
//	MActionKey*		GetActionKey(MXmlElement& element);
	MTextArea*		GetTextArea(MXmlElement& element);
	MTabCtrl*		GetTabCtrl(MXmlElement& element);
	MPanel*			GetPanel(MXmlElement& element);

	// Look&Feel
	MBLabelLook*	GetLabelLook(MXmlElement& element);
	MBButtonLook*	GetButtonLook(MXmlElement& element);
	MBGroupLook*	GetGroupLook(MXmlElement& element);
	MBFrameLook*	GetFrameLook(MXmlElement& element);
	MBEditLook*		GetEditLook(MXmlElement& element);
	MBListBoxLook*	GetListBoxLook(MXmlElement& element, int nType);	///< nType: 0=ListBox, 1=ComboListBox
	MBScrollBarLook* GetScrollBarLook(MXmlElement& element);
	MBArrowLook*	GetArrowLook(MXmlElement& element);
	MBThumbLook*	GetThumbLook(MXmlElement& element);
	MBSliderLook*	GetSliderLook(MXmlElement& element);
	MBTextAreaLook*	GetTextAreaLook(MXmlElement& element);
	MBSliderThumbLook* GetSliderThumbLook(MXmlElement& element);
	MBTabCtrlLook*	GetTabCtrlLook(MXmlElement& element);

	// Helper
	MPopupMenu* GetSubMenu(MMenuItem* pParentMenuItem, MXmlElement& element);
	MMenuItem* GetMenuItem(MPopupMenu* pPopupMenu, MXmlElement& element);
	void GetRebounds(MXmlElement& element);

	void GetFrameBtn(MFrameBtn* pFrameBtn, MBFrameLook* pFrameLook, MXmlElement& element);
	void GetBmButtonBitmaps(MBitmap** ppBitmaps, MXmlElement& element);
	void GetBitmaps(MBitmap** ppBitmaps, MXmlElement& element, const int nBitmapCount);
	void InsertWidget(MXmlElement& element, MWidget* pWidget);
	void InsertWidget( const char* pItemName, MWidget* pWidget );
	bool GetCommonWidgetProperty(MWidget* pWidget, MXmlElement& element, const char* szTagName);
	void ClearLooks();

	// 저장 관련 - 나중에 필요할때 만들자.
	void SetLabel(MXmlElement& element, MLabel* pLabel);
	void SetPoint(MXmlElement& element, MPOINT* pPoint, const char* szTagName);
	void SetRect(MXmlElement& element, MRECT* pRect, const char* szTagName);
	void SetSize(MXmlElement& element, MSIZE* pSize, const char* szTagName);
	void SetColor(MXmlElement& element, MCOLOR* pColor, const char* szTagName);

	virtual MFrame*	CreateFrame(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=
NULL);
	virtual MFont* CreateFont(char* szAliasName, char* szFontName, int nHeight, 
		bool bBold = false, bool bItalic = false, int nOutlineStyle = 1, bool bAntialiasing = false, DWORD nColorArg1=0, DWORD nColorArg2=0);

	virtual void Parse(MXmlElement& element);
	virtual void TransText(const char* szSrc, char* szOut, int maxlen);		// 국제화를 위한 함수
public:
	MIDLResource();
	virtual ~MIDLResource();
	bool LoadFromFile(const char* szFileName, MWidget* pParent=NULL,MZFileSystem *pfs=NULL);
	bool SaveToFile(const char* szFileName);
	virtual void Clear();
	MWidgetMMap* GetWidgetMap() { return &m_WidgetMap; }
	MWidget* FindWidget(string szItem);
	void FindWidgets(MWidgetList& widgetList, string szItem);

	MBFrameLook* FindFrameLook(string szItem);

	
};

#endif