#ifndef MPOPUPMENU_H
#define MPOPUPMENU_H

#include "MWidget.h"
#include "MDrawContext.h"
#include "MLookNFeel.h"

class MPopupMenu;
class MMenuItem;

// MPopupMenu의 Draw 코드가 있는 클래스, 이 클래스를 상속받아서 커스텀룩을 가질 수 있다.
class MPopupMenuLook {
public: 
	MCOLOR		m_SelectedPlaneColor;
	MCOLOR		m_SelectedTextColor;
	MCOLOR		m_UnfocusedSelectedPlaneColor;

protected:
	virtual void OnFrameDraw(MPopupMenu* pPopupMenu, MDrawContext* pDC);
public:
	MPopupMenuLook();
	virtual void OnDraw(MPopupMenu* pPopupMenu, MDrawContext* pDC);
	virtual MRECT GetClientRect(MPopupMenu* pPopupMenu, MRECT& r);
};

class MMenuItem : public MWidget{
private:
	bool			m_bSelected;

protected:
	virtual void OnDraw(MDrawContext* pDC);
	virtual void OnDrawMenuItem(MDrawContext* pDC, bool bSelected);
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);
public:
	MMenuItem(const char* szName=NULL);
	virtual ~MMenuItem(void);

	bool IsSelected(void);

	int GetWidth(void);
	int GetHeight(void);

	virtual MPopupMenu* CreateSubMenu(void);
	MPopupMenu* GetSubMenu(void);

	void Select(bool bSelect);

#define MINT_MENUITEM	"MenuItem"
	virtual const char* GetClassName(void){ return MINT_MENUITEM; }
};

enum MPopupMenuTypes{
	MPMT_VERTICAL = 0,
	MPMT_HORIZONTAL = 1
};

class MPopupMenu : public MWidget{
protected:
	MPopupMenuTypes m_nPopupMenuType;
	int				m_nSelectedMenu;

protected:
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);

	virtual bool OnCommand(MWidget* pWindow, const char* szMessage);

public:
	MPopupMenu(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL, MPopupMenuTypes t=MPMT_VERTICAL);
	virtual ~MPopupMenu(void);

	MPopupMenuTypes GetPopupMenuType() { return m_nPopupMenuType; }
	virtual MMenuItem* AddMenuItem(const char* szMenuName);
	void AddMenuItem(MMenuItem* pMenuItem);
	void RemoveMenuItem(MMenuItem* pMenuItem);
	void RemoveAllMenuItem(void);

	virtual void Show(bool bVisible=true);
	virtual void Show(int x, int y, bool bVisible=true);

	/*
	void SetScreenBounds(MRECT& r);
	void SetSize(int w, int h);
	void SetScreenPosition(int x, int y);
	*/

	void SetType(MPopupMenuTypes t);
	MPopupMenuTypes GetType(void);

	void Select(int idx);
	void Select(MMenuItem* pMenuItem);

#define MINT_POPUPMENU	"PopupMenu"
	virtual const char* GetClassName(void){ return MINT_POPUPMENU; }

	DECLARE_LOOK(MPopupMenuLook)
	DECLARE_LOOK_CLIENT()
};

#endif
