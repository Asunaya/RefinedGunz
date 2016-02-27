#ifndef MBMLABEL_H
#define MBMLABEL_H

#include "MWidget.h"
#include "MDrawContext.h"
#include "MLookNFeel.h"

/// 비트맵으로 그린 레이블
/// 비트맵에 0~9 , A ~ Z , a ~ z순으로 그려져 있어야 한다.
class MBmLabel : public MWidget{
protected:
	MBitmap*		m_pLabelBitmap;
	MSIZE			m_CharSize;

	virtual void OnDraw(MDrawContext* pDC);
public:
	MBmLabel(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);

	void SetLabelBitmap(MBitmap* pLabelBitmap);
	void SetCharSize(MSIZE &size);
#define MINT_BMLABEL	"BmLabel"
	virtual const char* GetClassName(void){ return MINT_BMLABEL; }
};

#endif