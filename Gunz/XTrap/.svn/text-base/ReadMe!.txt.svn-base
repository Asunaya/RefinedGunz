////////////////////////////////////////////////////////////////////

Copyright (c) 2005 WiseLogic Corporation

Author:
 WiseLogic Oct, 12, 2005

////////////////////////////////////////////////////////////////////



////////////////////////////
XTrap 연동 절차
////////////////////////////

1.	XTrapIC.Lib 파일을 프로젝트에 링크

2.	XTrap.h 파일과, XTrap.Cpp 파일을 프로젝트에 포함

3.	클라이언트의 최초 실행부분에 XTrapStart부분 구현 및, 헤더파일 포함.
	Start 구현에 대한 부분은 XTrap.Cpp 파일의 _XTRAPSTART을 참조.

4.	클라이언트의 종료 부분에 XTrapStop부분 구현.
	Stop 구현에 대한 부분은 XTrap.Cpp 파일의 _XTRAPSTOP을 참조.

5.	게임 엔진내에, g_ 계열의 XTrap 제공 글로벌 변수에 대한 비교구문 구현.
	비교구문 구현에 대한 부분은 XTrap.Cpp 파일의 XTrapMessageThreadProc(...)을 참조.
	단, XTrapMessageThreadProc(...) 은, 비교루틴에 대한 예 일뿐이므로, 변수 비교루틴을 참조하여, 엔진에서 직접 구현해야함.



////////////////////////////
XTrap 함수 설명
////////////////////////////

/* Optional */ void SetXTrapMgrInfo	(char *pMgrIp);
	자체적인 매니저서버를 사용할 시, 서버 IP를 셋팅하는 함수

/* Optional */ void SetXTrapPatchInfo	(char *pPatchUrl);
	XTrap 패치를 사용할 시, 패치 URL을 셋팅하는 함수

void SetXTrapStartInfo(	char *pGameName,		// 게임명		ex) neoGame
			char *pGameProcessName,		// 게임프로세스명	ex) neoGame.exe
			char *pGamePath,		// 게임실행경로		ex) c:\program files\neoGame
			DWORD ApiVersion,		// XTrapApi Version	ex) 0xA5001018
			DWORD VendorCode,		// 게임 벤더코드
			DWORD KeyboardType,		// 키보드 타입정의
			DWORD PatchType,		// 패치타입 정의
			DWORD ModuleType);		// 모듈타입 정의
	XTrap의 시작에 관련된 정보를 셋팅하는 함수

void XTrapStart		();
	XTrap을 시작하는 함수

void XTrapKeepAlive		();
	XTrap 실행상태 모니터링을 시작하는 함수

void SetOptGameInfo(	char *pUserName,		// 유저 ID
			char *pGameServerName,		// 서버명
			char *pCharacterName,		// 아바타명
			char *pClassName,		// 직업명
			long UserLevel);		// 레벨
	Login 직후, 유저 정보를 셋팅하는 함수

void XTrapStop		();
	XTrap을 종료하는 함수



////////////////////////////
XTrap 변수 설명
////////////////////////////


g_bApiMal		XTrap의 작동및, 이에 관여하는 API에 대한 이상

g_bMemoryMdl		XTrap의 작동및, 이에 관여하는 Memory에 대한 이상

g_bAutoMousMdl		오토마우스 검출

g_bAutoKeybMdl		오토키보드 검출

g_bMalMdl		악성코드 검출

g_bSpeedMdl		스피드핵 검출

g_bFileMdl		XTrap 파일 변조 및, 접근 이상 검출

g_bApiHookMdl		API Hook 검출

g_bDebugModMdl		디버그모드 검출

g_bMemoryCrack		메모리 크랙 검출

g_bFileCrack		파일 크랙 검출

g_bApiHookCrack		API Hook 에 의한 크랙 검출

g_bOsMdl		호환성모드 검출

g_bPatchMdl		XTrap Patch에 대한 행위 검출

g_bStartXTrap		XTrap의 시작상태 검출(시작된 경우 True)



////////////////////////////
자체 매니저서버 운용
////////////////////////////

자체적인 매니저서버 운용을 원하는경우,
당사에서 제공하는 메니져서버 프로그램을 사용하여, 서버시스템 구축 가능.
SetXTrapMgrInfo(...) 함수를 이용하여, 해당 서버 IP입력 후,
로그 정보는 당사에서 제공하는 관리기를 사용여 확인 가능함.



////////////////////////////
주의사항
////////////////////////////

1.	XTrapStart 부분은 클라이언트 구동시 Initial entry point 에 구현되어야 함.

2.	조이스틱의 사용이 필요한 경우,
	에물레이터를 통한 사용은 지원되지 않으며, 게임내에 직접 구현되어야 함. 필요한 경우 당사에서 조이스틱 모듈 제공.
	이때, XTrapStart() 호출 이전에, 조이스틱의 초기화가 선행 되어야 함.

3.	신용카드등 결제 시스템 이용시 타사의 보안 모듈이 실행되는 경우,
	이에 대한 인터페이스를 별도 제공하므로, 확인이 필요함.

4.	벤더코드는 특정 지역의 특정 게임, 특정 퍼블리셔에 한함.
	따라서, 타 지역이나, 타 퍼블리셔와 함께 진행할 경우 새로운 벤더코드 부여가 필요.