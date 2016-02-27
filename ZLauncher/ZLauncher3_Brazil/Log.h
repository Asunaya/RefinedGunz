/******************************************************************
   
   Log.h

     Corperation : MAIET entertainment
     Programmer  : Lim Dong Hwan
	 Date        : 22.June.2005

*******************************************************************/


#ifndef _LOG
#define _LOG


// Defines
//#ifdef _DEBUG
#define LOG_ENABLE							// 만약 로그 출력을 원하지 않을 경우 주석처리 할것.
//#endif

#define LOG_DEFFONT			"Arial"			// 기본 폰트
#define LOG_SHOWFILE		false			// 이 값이 true면 : Text 파일은 종료할 때 파일을 화면에 띄운다.
											//                  HTML 파일은 시작할 때 페이지를 화면에 띄운다.

enum LOG_TYPE								// 로그 출력 방식
{
	LOGTYPE_OUTPUT,								// 출력 창
	LOGTYPE_TEXT,								// 텍스트 파일
	LOGTYPE_HTML,								// HTML 파일
	LOGTYPE_EXEL,								// 엑셀 파일
};

enum LOG_MSG								// 로그 메시지 종류
{
	LOG_NORMAL,									// 일반
	LOG_HIGHLIGHT,								// 돋움
	LOG_WARNNING,								// 경고
	LOG_ERROR,									// 에러
};


// Functions
bool OpenLog( const char* pszFileName, const char* pszTitle, LOG_TYPE nFileType);
bool CloseLog( void);
bool PutLog( const char* pszLog);
bool PutLogH( const char* pszLog);
bool PutLogW( const char* pszLog);
bool PutLogE( const char* pszLog);
bool PutLog( const char* pszLog, LOG_MSG nMsgType);

#endif
