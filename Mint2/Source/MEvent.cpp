#include "stdafx.h"
#include "MEvent.h"
#include <zmouse.h>
#include "Mint.h"

MPOINT MEvent::LatestPos = MPOINT(0, 0);	// 마지막 위치
bool MEvent::bIMESupport = false;	// Default IME Support Enabled
bool MEvent::bLButton = false;
bool MEvent::bMButton = false;
bool MEvent::bRButton = false;

bool MEvent::GetShiftState(void)
{
	if((GetKeyState(VK_SHIFT)&0x8000)==0x8000) return true;
	return false;
}

bool MEvent::GetCtrlState(void)
{
	if((GetKeyState(VK_CONTROL)&0x8000)==0x8000) return true;
	return false;
}

bool MEvent::GetAltState(void)
{
	if((GetKeyState(VK_MENU)&0x8000)==0x8000) return true;
	return false;
}

bool MEvent::IsKeyDown(int key)
{
	return ((GetAsyncKeyState(key) & 0x8000)!=0);
}

bool MEvent::GetLButtonState()
{
	return bLButton;
}

bool MEvent::GetRButtonState()
{
	return bRButton;
}

bool MEvent::GetMButtonState()
{
	return bMButton;
}

MPOINT MEvent::GetMousePos(void)
{
	POINT p;
	GetCursorPos(&p);
	return MPOINT(p.x, p.y);
}

#include "MDebug.h"
void MEvent::ForceSetIME(DWORD fdwConversion,DWORD fdwSentence)
{
	HWND hWnd = Mint::GetInstance()->GetHWND();
	HIMC hImc = ImmGetContext(hWnd);
	if (hImc)
	{
		ImmSetConversionStatus(hImc,fdwConversion,fdwSentence);
		ImmReleaseContext(hWnd, hImc);
	}
}

int MEvent::TranslateEvent(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	//bShift = GetShiftState();
	bCtrl = GetCtrlState();
	//bAlt = GetAltState();

	switch(message){
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_MOUSEMOVE:
		Pos.x = LOWORD(lparam);
		Pos.y = HIWORD(lparam);

		LatestPos = Pos;
		break;
	case WM_MOUSEWHEEL:	// 휠은 절대좌표로 날아옴
		{
			POINT pos;
			pos.x = LOWORD(lparam);
			pos.y = HIWORD(lparam);
			ScreenToClient(Mint::GetInstance()->GetHWND(),&pos);

			Pos.x = pos.x;
			Pos.y = pos.y;

			LatestPos = Pos;
		}
		break;
	}

	switch(message){
	case WM_LBUTTONDOWN:
		nMessage = MWM_LBUTTONDOWN;
		bLButton=true;
		return EVENT_MINT_TRANSLATED;
	case WM_LBUTTONUP:
		bLButton=false;
		nMessage = MWM_LBUTTONUP;
		return EVENT_MINT_TRANSLATED;
	case WM_RBUTTONDOWN:
		bRButton=true;
		nMessage = MWM_RBUTTONDOWN;
		return EVENT_MINT_TRANSLATED;
	case WM_RBUTTONUP:
		bRButton=false;
		nMessage = MWM_RBUTTONUP;
		return EVENT_MINT_TRANSLATED;
	case WM_MBUTTONDOWN:
		bMButton=true;
		nMessage = MWM_MBUTTONDOWN;
		return EVENT_MINT_TRANSLATED;
	case WM_MBUTTONUP:
		bMButton=false;
		nMessage = MWM_MBUTTONUP;
		return EVENT_MINT_TRANSLATED;
	case WM_LBUTTONDBLCLK:
		nMessage = MWM_LBUTTONDBLCLK;
		bLButton=true;
		return EVENT_MINT_TRANSLATED;
	case WM_RBUTTONDBLCLK:
		nMessage = MWM_RBUTTONDBLCLK;
		bRButton=true;
		return EVENT_MINT_TRANSLATED;
	case WM_MBUTTONDBLCLK:
		nMessage = MWM_MBUTTONDBLCLK;
		bMButton=true;
		return EVENT_MINT_TRANSLATED;
	case WM_MOUSEMOVE:
		nMessage = MWM_MOUSEMOVE;
		return EVENT_MINT_TRANSLATED;
	case WM_MOUSEWHEEL:
		nMessage = MWM_MOUSEWHEEL;
		nDelta = (short)HIWORD(wparam);
		return EVENT_MINT_TRANSLATED;
		/*
	case WM_RESIZE_EVENT:
		nMessage = MWM_RESIZE;
		return EVENT_MINT_TRANSLATED;

	case WM_MOVIE_NOTIFY:
		nMessage = MWM_MOVIE_NOTIFY;
		return EVENT_MINT_TRANSLATED;
		*/

	case WM_KEYDOWN:
		nMessage = MWM_KEYDOWN;
		nKey = wparam;
		return EVENT_MINT_TRANSLATED;

		/*
		switch(wparam){
		case VK_DELETE:
			nMessage = MWM_SCHAR;
			nKey = wparam;
			return EVENT_MINT_TRANSLATED;
		case VK_LEFT:
		case VK_RIGHT:
		case VK_HOME:
		case VK_END:
			nMessage = MWM_CHAR;
			nKey = wparam;
			return EVENT_MINT_TRANSLATED;
		default:
			return EVENT_NOT_PROCESSED;
		}
		*/
		return EVENT_MINT_TRANSLATED;
	case WM_KEYUP:
		nMessage = MWM_KEYUP;
		nKey = wparam;
		return EVENT_MINT_TRANSLATED;
	case WM_SYSCHAR:
		nMessage = MWM_SYSCHAR;
		nKey = wparam;
		bAlt = true;
		return EVENT_MINT_TRANSLATED;
	case WM_SYSKEYDOWN:
		nMessage = MWM_SYSKEYDOWN;
		nKey = wparam;
		return EVENT_MINT_TRANSLATED;
	case WM_SYSKEYUP:
		nMessage = MWM_SYSKEYUP;
		nKey = wparam;
		return EVENT_MINT_TRANSLATED;
	case WM_CHAR:
		nMessage = MWM_CHAR;
		nKey = wparam;
		return EVENT_MINT_TRANSLATED;
	case WM_HOTKEY:
		nMessage = MWM_HOTKEY;
		nKey = wparam;
		return EVENT_MINT_TRANSLATED;

	// IME 관련
	case WM_INPUTLANGCHANGE:
		// 원래는 여기에 Input Language 이벤트 변화를 받아야 하지만, Mint에서는 폴링 방식으로
		// GetKeyboardLayout()을 이용하여 필요한 때 얻어서 사용한다.
		// Mint::IsNativeIME() 참고
		// 2005-08-18, 이장호
		return (EVENT_PROCESSED|EVENT_MINT_TRANSLATED);
	case WM_IME_STARTCOMPOSITION:
		if(bIMESupport==true){
			//nMessage = MWM_IMECOMPOSE;
			szIMECompositionString[0] = NULL;
			szIMECompositionResultString[0] = NULL;
//			OutputDebugString("WM_IME_STARTCOMPOSITION");
			return (EVENT_PROCESSED|EVENT_MINT_TRANSLATED);
		}
		return EVENT_PROCESSED;	// 처리가 되므로
	case WM_IME_COMPOSITION:
		if(bIMESupport==true){
//			OutputDebugString("WM_IME_COMPOSITION\n");
			nMessage = MWM_IMECOMPOSE;

			Mint* pMint = Mint::GetInstance();

			HIMC hIMC = ImmGetContext(hwnd);
			if (hIMC){
				if(lparam&GCS_RESULTSTR){
					LONG i = ImmGetCompositionString(hIMC, GCS_RESULTSTR, szIMECompositionResultString, sizeof(szIMECompositionResultString));
					szIMECompositionResultString[i] = NULL;
					//				OutputDebugString("GCS_RESULTSTR\n");
					pMint->m_nCompositionAttributeSize = 0;
					memset(pMint->m_nCompositionAttributes, 0, sizeof(BYTE)*(MIMECOMPOSITIONSTRING_LENGTH));
					//memset(pMint->m_dwCompositionClauses, 0, sizeof(DWORD)*(MIMECOMPOSITIONSTRING_LENGTH));
				}
				else{
					szIMECompositionResultString[0] = NULL;
				}
				if(lparam&GCS_COMPSTR){
					LONG i = ImmGetCompositionString(hIMC, GCS_COMPSTR, szIMECompositionString, sizeof(szIMECompositionString));
					szIMECompositionString[i] = NULL;
					//				OutputDebugString("GCS_COMPSTR\n");

				}
				else{
					szIMECompositionString[0] = NULL;
				}

				/*
				// 절 정보 얻기(일어)
				if(lparam & GCS_COMPCLAUSE)
					ImmGetCompositionString(hIMC, GCS_COMPCLAUSE, pMint->m_dwCompositionClauses, sizeof(pMint->m_dwCompositionClauses));
				*/

				// 속성 얻기(각 절이 어떤 값을 가지는가)
				if(lparam & GCS_COMPATTR)
					pMint->m_nCompositionAttributeSize = ImmGetCompositionString(hIMC, GCS_COMPATTR, pMint->m_nCompositionAttributes, sizeof(pMint->m_nCompositionAttributes));

				if(lparam & GCS_CURSORPOS)
					pMint->m_nCompositionCaretPosition = ImmGetCompositionString(hIMC, GCS_CURSORPOS, NULL, 0);
			}
			
//			OutputDebugString("Comp - ");
//			OutputDebugString(szIMECompositionString);
//			OutputDebugString("\n");
//			OutputDebugString("Resl - ");
//			OutputDebugString(szIMECompositionResultString);
//			OutputDebugString("\n");
			return (EVENT_PROCESSED|EVENT_MINT_TRANSLATED);
		}
		return EVENT_PROCESSED;	// 처리가 되므로
	case WM_IME_ENDCOMPOSITION:
		if(bIMESupport==true){
			//nMessage = MWM_IMECOMPOSE;
			szIMECompositionString[0] = NULL;
			szIMECompositionResultString[0] = NULL;
//			OutputDebugString("WM_IME_ENDCOMPOSITION");
			Mint* pMint = Mint::GetInstance();
			pMint->m_nCompositionCaretPosition = 0;
			return (EVENT_PROCESSED|EVENT_MINT_TRANSLATED);
		}
		return EVENT_PROCESSED;	// 처리가 되므로
	case WM_IME_NOTIFY:
		{
			Mint* pMint = Mint::GetInstance();
			if(bIMESupport==true && pMint->IsEnableIME()==true){	// IsEnableIME() 체크는 창 밖에서 Candidate List가 안나오게 하기 위해
				/*
				if(wparam==IMN_SETCONVERSIONMODE){
					nMessage = MWM_IMECONVERSION;

					if(Mint::GetInstance()->m_dwIMEConvMode==IME_CMODE_ALPHANUMERIC)
						Mint::GetInstance()->m_dwIMEConvMode=IME_CMODE_NATIVE;
					else
						Mint::GetInstance()->m_dwIMEConvMode=IME_CMODE_ALPHANUMERIC;
					return EVENT_MINT_TRANSLATED;
				} else if (wparam == IMN_OPENCANDIDATE) {
					OutputDebugString("IMEDEBUG: IMN_OPENCANDIDATE \n");
				} else if (wparam == IMN_CLOSECANDIDATE) {
					OutputDebugString("IMEDEBUG: IMN_CLOSECANDIDATE \n");
				} else if (wparam == IMN_CHANGECANDIDATE) {
					OutputDebugString("IMEDEBUG: IMN_CHANGECANDIDATE \n");
				} else if (wparam == IMN_SETCANDIDATEPOS) {
					OutputDebugString("IMEDEBUG: IMN_SETCANDIDATEPOS \n");
				}
				*/
				if(wparam==IMN_OPENCANDIDATE || wparam==IMN_CHANGECANDIDATE){
					// IMN_CHANGECANDIDATE 또한 Open과 마찬가지로 CandidateList를 얻어야 함
					pMint->OpenCandidateList();
				}
				else if(wparam==IMN_CLOSECANDIDATE){
					pMint->CloseCandidateList();
				}
				return (EVENT_PROCESSED|EVENT_MINT_TRANSLATED);
			}
		}
		return EVENT_PROCESSED;
	case WM_IME_SETCONTEXT:
		// 일본어에서 F5에 입력창 뜨는 명령어 차단
		return EVENT_PROCESSED;
	default:
		return EVENT_NOT_PROCESSED;
	}

	return EVENT_NOT_PROCESSED;
}

