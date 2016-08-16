#ifndef _MMATCHDEBUG_H
#define _MMATCHDEBUG_H

// cscommon에서 디버그에 관한 사항 정리

//#define _DEBUG_PUBLISH				// 디버그모드로 퍼블리쉬 - 테스트하기 위함


#if defined(_DEBUG_PUBLISH) && defined(_DEBUG)
	#ifdef _ASSERT
	#undef _ASSERT
	#endif
	#define _ASSERT(X)		((void)0)

	#ifdef OutputDebugString
	#undef OutputDebugString
	#endif
	#define OutputDebugString(X)		((void)0)
#endif

// 컴파일러옵션 /GS옵션을 사용할때 버퍼 오버런이 났을 경우 뜨는 메시지 박스를 가로챈다.
//void SetSecurityErrorHandler(_secerr_handler_func func);
//void __cdecl ReportBufferOverrun(int code, void * unused);


#endif