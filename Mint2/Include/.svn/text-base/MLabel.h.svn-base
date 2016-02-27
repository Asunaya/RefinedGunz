#ifndef MLABEL_H
#define MLABEL_H

#include "MWidget.h"
#include "MDrawContext.h"
#include "MLookNFeel.h"

class MLabel;

/// Label의 Draw 코드가 있는 클래스, 이 클래스를 상속받아서 커스텀룩을 가질 수 있다.
class MLabelLook{
public:
	virtual void OnDraw(MLabel* pLabel, MDrawContext* pDC);
	virtual MRECT GetClientRect(MLabel* pLabel, MRECT& r);
};

/// Label
class MLabel : public MWidget{
protected:
	MCOLOR			m_TextColor;
	MAlignmentMode	m_AlignmentMode;

	DECLARE_LOOK(MLabelLook)
	DECLARE_LOOK_CLIENT()

protected:

public:
	MLabel(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);

	/// 텍스트 컬러 지정
	void SetTextColor(MCOLOR color);
	/// 텍스트 컬러 얻기
	MCOLOR GetTextColor(void);

	/// 정렬 얻기
	MAlignmentMode GetAlignment(void);
	/// 정렬 지정
	MAlignmentMode SetAlignment(MAlignmentMode am);


#define MINT_LABEL	"Label"
	virtual const char* GetClassName(void){ return MINT_LABEL; }
};


#endif