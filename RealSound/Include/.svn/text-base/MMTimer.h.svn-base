/*
	----------------
	Multimedia Timer
	----------------

	MMTimer.h

	"Multimedia Timer" handling class header file

	Programming by Chojoongpil
*/
#include <windows.h>
#include <mmsystem.h>

#pragma comment( lib, "winmm.lib" )

typedef BOOL (*MMTIMERCALLBACK)(DWORD);

// 하나의 클래스는 한개의 타이머를 갖는다.
// 여러개의 셋팅을 지원하지 않으므로 주의할 것.
class MMTimer
{
public:
    MMTimer();
    ~MMTimer();
	
	// Multimedia Timer한개를 생성한다.
    BOOL Create(UINT nPeriod, UINT nRes, DWORD dwUser,  MMTIMERCALLBACK pfnCallback);
	void Destroy();

protected:
    static void CALLBACK TimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

	// 현재 타이머가 분기할 콜백 함수
    MMTIMERCALLBACK m_pfnCallback;

    DWORD m_dwUser;
    UINT m_nPeriod;
    UINT m_nRes;
    UINT m_nIDTimer;
};
