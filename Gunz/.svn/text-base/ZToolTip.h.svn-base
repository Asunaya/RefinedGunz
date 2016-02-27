#ifndef ZTOOLTIP_H
#define ZTOOLTIP_H

#include "MWidget.h"
#include "MToolTip.h"

#define MAX_TOOLTIP_LINE_STRING 40

class ZToolTip : public MToolTip
{
public:
	ZToolTip(const char* szName, MWidget* pParent);
	~ZToolTip();
	virtual void OnDraw(MDrawContext* pDC);
	virtual void SetBounds(void);

	//	MTextArea* m_pTextArea;
	MBitmap* m_pBitmap1;
	MBitmap* m_pBitmap2;
};

#endif//ZTOOLTIP_H
