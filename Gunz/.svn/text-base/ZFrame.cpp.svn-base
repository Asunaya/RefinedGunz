#include "stdafx.h"

#include <MMSystem.h>

#include "ZFrame.h"
#include "Mint.h"

#define TRANSIENT_TIME	300

void ZFrame::Show(bool bVisible, bool bModal)
{
	DWORD elapsed=(timeGetTime()-m_nShowTime);

	if(m_bNextVisible==m_bVisible && m_bVisible==bVisible && elapsed>TRANSIENT_TIME)
		return;

	// 상태가 변경되는 중간에 다른방향으로 가야할경우
	if(m_bNextVisible!=bVisible){
		if(elapsed<TRANSIENT_TIME)
			m_nShowTime=timeGetTime()-(TRANSIENT_TIME-elapsed);
		else
			m_nShowTime=timeGetTime();
	}
	m_bNextVisible = bVisible;

	MFrame::Show(bVisible,bModal);
	m_bVisible = true;
	
 	Enable(bVisible);

	if(bVisible)
		m_bExclusive=bModal;
}

void ZFrame::OnDraw(MDrawContext* pDC)
{
//	bool bExclusive=false;
	
	float fOpacity = 0;
	if(m_bNextVisible==false){	// Hide
		fOpacity = 1.0f-min(float(timeGetTime()-m_nShowTime)/(float)TRANSIENT_TIME, 1.0f);
		if(fOpacity==0.0f) {
			m_bVisible = false;
			m_bExclusive = false;
		}
		SetOpacity(unsigned char(fOpacity*0xFF));
	}
	else{	// Show
		fOpacity = min(float(timeGetTime()-m_nShowTime)/(float)TRANSIENT_TIME, 1.0f);
		SetOpacity(unsigned char(fOpacity*0xFF));
	}

	// 백그라운를 어둡게 만들고...
 	if( m_bExclusive ){

		MRECT Full(0, 0, MGetWorkspaceWidth()-1, MGetWorkspaceHeight()-1);
		MRECT PrevClip = pDC->GetClipRect();
		pDC->SetClipRect(Full);
		unsigned char oldopacity=pDC->SetOpacity(200*fOpacity);
		pDC->SetColor(0, 0, 0, 255);
		MPOINT PrevOrigin = pDC->GetOrigin();
		pDC->SetOrigin(0, 0);
		pDC->FillRectangle(Full);

		// 복구
		pDC->SetClipRect(PrevClip);
		pDC->SetOrigin(PrevOrigin);
		pDC->SetOpacity(oldopacity);
	}

	// 프레임 그리기
	MFrame::OnDraw(pDC);
}

ZFrame::ZFrame(const char* szName, MWidget* pParent, MListener* pListener)
			: MFrame(szName, pParent, pListener)
{
	m_bCanShade = false;
	m_bNextVisible = false;
	m_nShowTime = timeGetTime()-TRANSIENT_TIME*2;
	SetOpacity(0);
}

ZFrame::~ZFrame(void)
{
}

