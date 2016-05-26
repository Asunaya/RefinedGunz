
/******************************************************************************
// XTrap.cpp Version 1005
// By WiseLogic
//****************************************************************************/

#include "stdafx.h"
#include "XTrap.h"

///////////////////////////////////////////////////////////////////////////
// XTrap Start
///////////////////////////////////////////////////////////////////////////
#ifdef _XTRAPSTART
{
	char	sGameName[MAX_PATH];					/* example "XGame"     */
	char	sGamePath[MAX_PATH];					/* example "C:\\xgame" */
	char	sGameProcessName[_MAX_PATH];			/* example "xgame.exe" */

	DWORD	ApiVersion		= 0xA5001004;
	
	DWORD	VendorCode		= 0x00000000;				
	DWORD	KeyboardType	= 0x00000000;
	DWORD	PatchType		= 0x00000000;
	DWORD	ModuleType		= 0x00000000;

	sprintf_safe(sGameName, "%s",	"XGAME");			/* 게임 식별이름						*/
	sprintf_safe(sGameProcessName,	"Xgame.exe");		/* 게임 프로세서명을 입력				*/
	sprintf_safe(sGamePath, "%s",	GPath);				/* 게임 프로세서가 실행된 경로명 입력	*/	

	//
	// KnightOnline(Korea)		URL				"http://wizweb.nefficient.co.kr/wizweb/wizgate3/xtrap/Knight"
	//							PatchType		0x00000010
	//							VendorCode		0x01006010 
	//							KeyboardType	0x00000000
	//							ModuleType		0x0FFFFFFF
	//							VerifyHash		"02bfd34ce6563c57f6513274d5e8ff1f"  // 주의 knightonline.exe 에서만 동작

	/*Option SetXTrapMgrInfo(char *pMgrIp)		*/
	/*Option SetXTrapPatchInfo(char *pPatchUrl)	*/
	SetXTrapStartInfo	(sGameName, sGameProcessName, sGamePath, 
						 ApiVersion, VendorCode, KeyboardType, PatchType, ModuleType)

	SetXTrapStartInfo	(sGameName, sGameProcessName, sGamePath, ApiVersion, VendorCode);
	XTrapStart			();
	XTrapKeepAlive		();

	/*Test	XTrapMessage(); */
}
#endif

///////////////////////////////////////////////////////////////////////////
// XTrap Stop
///////////////////////////////////////////////////////////////////////////
#ifdef _XTRAPSTOP
{
	XTrapStop();
}
#endif

///////////////////////////////////////////////////////////////////////////
// Example XTrapMessageThreadProc
///////////////////////////////////////////////////////////////////////////
/*
DWORD WINAPI XTrapMessageThreadProc(LPVOID lpParameter)
{
	DWORD dwReturn;

	char szMsgBuf[500] = {
		"비정상적인 행위가 감지되었습니다. 게임을 종료합니다. "
	};


	do
	{
		dwReturn = WaitForSingleObject(g_XTrapMsgEvent, 5000);

		if		(dwReturn == WAIT_FAILED)			break;
		else if (dwReturn == WAIT_ABANDONED)		break;
		else if (dwReturn == WAIT_OBJECT_0)			break;

		else if (dwReturn == WAIT_TIMEOUT)
		{
			///////////////////////////////////////////////////////////////////
			// 디폴트 탐지 메시지 루틴

			if (g_bApiMal			== TRUE ||
				g_bMemoryMdl		== TRUE ||
				g_bAutoMousMdl		== TRUE ||
				g_bAutoKeybMdl		== TRUE ||
				g_bMalMdl			== TRUE ||
				g_bSpeedMdl			== TRUE ||
				g_bFileMdl			== TRUE ||
				g_bApiHookMdl		== TRUE ||
				g_bDebugModMdl		== TRUE ||
				g_bMemoryCrack		== TRUE ||
				g_bFileCrack		== TRUE ||
				g_bApiHookCrack		== TRUE)
			{
				AfxMessageBox(szMsgBuf); 
				//
				// CloseSocket and Exit Game Client
				//
			}

			if (g_bOsMdl			== TRUE)
			{
				AfxMessageBox("파일 속성에서 호환성 모드가 설정되었습니다. 설정을 해제하십시오."); 
				//
				// CloseSocket and Exit Game Clinet
				//
			}

			if (g_bPatchMdl			== TRUE)
			{
				AfxMessageBox("패치 서버 접속에 문제가 있습니다. 네트워크 상황을 확인하십시오."); 
				//
				// CloseSocket and Exit Game Clinet
				//
			}

			if (g_bStartXTrap		== FALSE)
			{
				AfxMessageBox("게임 보안모듈을 실행할수 없습니다."); 
				//
				// CloseSocket and Exit Game Clinet
				//
			}

			continue;
		}

	} while(TRUE);

	CloseHandle(g_XTrapMsgEvent);
	g_XTrapMsgEvent = NULL;

	return TRUE;
}
*/