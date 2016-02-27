#ifndef _MTABCTRL_H
#define _MTABCTRL_H
//#pragma once

#include <list>
using namespace std;

#include "MWidget.h"
#include "MLookNFeel.h"


class MTabCtrl;
class MDrawContext;
class MButton;


/// MTabCtrl의 Draw 코드가 있는 클래스, 이 클래스를 상속받아서 커스텀룩을 가질 수 있다.
class MTabCtrlLook{
public:
	virtual void OnDraw(MTabCtrl* pTabCtrl, MDrawContext* pDC);
	virtual MRECT GetClientRect(MTabCtrl* pTabCtrl, MRECT& r);
};


//// MTabItem
class MTabItem {
public:
	MWidget	*pFrame;
	MButton *pButton;
	MListener *pButtonListener;		// 버튼의 원래 리스너
	MTabItem(MWidget *pFrame,MButton *pButton,MListener *pListener);
};

class MTabList : public list<MTabItem*> {};


//// MTabCtrl
class MTabCtrl : public MWidget {
protected:
	int			m_nCurrentSel;
	MTabList	m_TabList;

public:
	MTabCtrl(const char* szName, MWidget* pParent=NULL, MListener* pListener=NULL);
	MTabCtrl(MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MTabCtrl(void);
		
	void Add(MButton *pButton, MWidget *pFrame);		// pButton 을 누르면 pFrame 이 보여진다
	void RemoveAll();

	int GetCount(void);
	int GetSelIndex(void);
	bool SetSelIndex(int i);

	void UpdateListeners();			// 만약 버튼의 리스너가 바뀐다면 이펑션을 불러주어야 한다.

	virtual bool OnCommand(MWidget* pWidget, const char* szMessage);
public:
	DECLARE_LOOK(MTabCtrlLook)
	DECLARE_LOOK_CLIENT()

	#define MINT_TABCTRL	"TabCtrl"
	virtual const char* GetClassName(void){ return MINT_TABCTRL; }

public:
	
};

#endif