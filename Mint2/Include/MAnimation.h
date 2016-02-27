#ifndef MANIMATION_H
#define MANIMATION_H

#include "MWidget.h"

enum MAPlayMode{
	MAPM_FORWARDONCE,
	MAPM_FORWARDNBACKWARD,
	MAPM_REPETITION,
};

class MAnimation : public MWidget{
private:
	unsigned int	m_nStartTime;
	MAniBitmap*		m_pAniBitmap;
public:
	int				m_nDelay;
	MAPlayMode		m_nPlayMode;
protected:
	virtual void OnDraw(MDrawContext* pDC);
public:
	MAnimation(const char* szName=NULL, MAniBitmap* pAniBitmap=NULL, MWidget* pParent=NULL);
	void SetAniBitmap(MAniBitmap* pAniBitmap);
	void InitStartTime();
	MBitmap* GetBitmap();

#define MINT_ANIMATION	"Animation"
	virtual const char* GetClassName(void){ return MINT_ANIMATION; }



/* 동환이의 추가 부분 :
    
	아래 추가된 부분은 특정 상황마다 이미지를 수시로 변경하는 경우 이를 애니메이션 위젯으로
	해결하기 위해서 추가한 것이다. 쉽게 말해서 애니메이션을 정지시킨 상태에서 유저가 보여줄
	이미지 번호만 바꿔주는 형식이다.
*/
protected:
	int				m_nCurrFrame;;			// 현재 보여지고 있는 애니메이션 이미지 번호

public:
	bool			m_bRunAnimation;		// 현재 애니메이션이 되고있는지 여부
	bool			GetRunAnimation( void)  { return m_bRunAnimation;}
	void			SetRunAnimation( bool bRun);
	int				GetCurrentFrame( void);
	void			SetCurrentFrame( int nFrame);
};

#endif