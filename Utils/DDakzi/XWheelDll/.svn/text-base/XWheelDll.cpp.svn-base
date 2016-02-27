#define _WIN32_WINNT 0x0500
#include <Windows.h>
#include <Windowsx.h>
#include "XWheelDll.h"

#pragma data_seg(".hdata")
HHOOK hMouseHook = NULL;
int				iInstanceCount		= 0;

#pragma data_seg()

bool			bHooked = false;
HINSTANCE		hInst	= 0;

void XWHEELDLL_API StartMouseHook()
{
	if(!bHooked) {
		hMouseHook = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)MouseProc, hInst, 0);
		bHooked = true;
		MessageBeep(MB_OK);
	}
}

void XWHEELDLL_API EndMouseHook()
{
	if (bHooked)
	{
		MessageBeep(MB_ICONQUESTION);
		UnhookWindowsHookEx(hMouseHook);
		hMouseHook = NULL;
		bHooked = false;
	}
}

LRESULT XWHEELDLL_API CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode != HC_ACTION) 
	{
		return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
	}

	if (nCode == HC_ACTION)
	{
		if (wParam == WM_MOUSEWHEEL) {
			//	MOUSEHOOKSTRUCTEX 는 윈2000 이상에서만 가능하다고 한다
			MOUSEHOOKSTRUCTEX *pmhs = (MOUSEHOOKSTRUCTEX*)lParam;
			short zDelta = GET_WHEEL_DELTA_WPARAM(pmhs->mouseData);
			HWND hWnd = WindowFromPoint(pmhs->pt);
			if(zDelta!=0)
				PostMessage(hWnd, WM_MOUSEWHEEL, MAKELONG(0,short(zDelta)) ,MAKELONG(pmhs->pt.x,pmhs->pt.y));
//			MessageBeep(MB_OK);
			return 1;

			/*
			MOUSEHOOKSTRUCT *pmhs = (MOUSEHOOKSTRUCT*)lParam;
			
			HWND hWnd = WindowFromPoint(pmhs->pt);
			pmhs->hwnd = hWnd;
			pmhs->wHitTestCode = HTCLIENT;
			MessageBeep(MB_OK);
			*/

		}
	}
	return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  // handle to the DLL module
  DWORD fdwReason,     // reason for calling function
  LPVOID lpvReserved   // reserved
)
{
	UNREFERENCED_PARAMETER(lpvReserved);
	
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			hInst		= hinstDLL;
			++iInstanceCount;
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			--iInstanceCount;
			break;
	}

	return TRUE;
}