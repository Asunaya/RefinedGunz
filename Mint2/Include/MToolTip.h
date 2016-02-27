#ifndef MTOOLTIP_H
#define MTOOLTIP_H

#include "MWidget.h"

class MToolTip : public MWidget{
protected:
	bool	m_bUseParentName;

public:
	// szName이 NULL이면 Parent의 m_szName을 표시한다.
	MToolTip(const char* szName, MWidget* pParent);
	virtual ~MToolTip(void);

	// ToolTip의 String의 크기에 따라 Widget크기 자동 조절
	virtual void SetBounds(void);

	// 텍스트 입력, SetBounds()실행된다.
	void SetText(const char* szText);

	bool IsUseParentName(void);

	virtual void OnDraw(MDrawContext* pDC);

#define MINT_TOOLTIP	"ToolTip"
	virtual const char* GetClassName(void){ return MINT_TOOLTIP; }
};

#endif
