/*
	MMTimer.cpp
	-----------
	
	"Multimedia Timer" handling class implementation file
	
	Programming by Chojoongpil
	MAIET entertainment software
*/
#include "stdafx.h"
#include "MMTimer.h"
#include "crtdbg.h"

////////////////////////////////////////
// Constructor & Destructor

MMTimer::MMTimer()
{	
    m_nIDTimer = NULL;
}

MMTimer::~MMTimer()
{
	Destroy();	// 생성된 타이머가 있다면 삭제 한다.
}

////////////////////////////////////////
// Member Function Implementation

void MMTimer::Destroy()
{
    if (m_nIDTimer){
        timeKillEvent (m_nIDTimer);
    }
}

// 콜백함수를 위해 포인터를 두었다.
BOOL MMTimer::Create (UINT nPeriod, UINT nRes, DWORD dwUser, MMTIMERCALLBACK pfnCallback)
{
    BOOL bRtn = TRUE;
    
    _ASSERT(pfnCallback);
    _ASSERT(nPeriod > 10);
    _ASSERT(nPeriod >= nRes);

    m_nPeriod = nPeriod;
    m_nRes = nRes;
    m_dwUser = dwUser;
    m_pfnCallback = pfnCallback;

    if ((m_nIDTimer = timeSetEvent (m_nPeriod, m_nRes, TimeProc, (DWORD) this, TIME_PERIODIC)) == NULL){
        bRtn = FALSE;
    }

    return (bRtn);
}

// Multimedia 타이머 메시지가 호출될때마다 이 콜백 함수가 불리운다.
void CALLBACK MMTimer::TimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{	    
	// 콜백함수를 위해 포인터를 두었다.
	// Callback함수가 static이므로 현재 오브젝트의 pointer를 얻는 방식으로 처리된다.
    MMTimer * ptimer = (MMTimer *) dwUser;
	
	// 진짜 Callback함수가 호출된다.
	// 함수는 MMTIMERCALLBACK의 형을 가졌다.
    (ptimer->m_pfnCallback) (ptimer->m_dwUser);
}
