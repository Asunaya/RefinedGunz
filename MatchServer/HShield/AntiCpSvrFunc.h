/*
* (C) Copyright 2005. AhnLab, Inc.
*
* Any part of this source code can not be copied with
* any method without prior written permission from
* the author or authorized person.
*
* File Name : AntiCpSvrFunc.h
* Author: Nam Sung il(oncea@ahnlab.com)
*
* Revision History
* Date			Name					Description
* 2005-06-07	Nam, Sung-Il(siNam)		Created
* 2005-10-28	Park, Heean(hapark)		Add the Memory Checking
* 2005-11-15	Park, Heean(hapark)		Solve the Server rebooting issue
*
*/

#ifndef _ANTICPSVRFUNC_H
#define _ANTICPSVRFUNC_H

#define SIZEOF_REQMSG			160				// Request Message를 담을 버퍼의 크기
#define SIZEOF_REQINFO			72				// Request Info를 담을 버퍼의 크기
#define SIZEOF_ACKMSG			56				// Ack Message를 담을 버퍼의 크기
#define SIZEOF_GUIDREQMSG		20				// GUID Request Message를 담을 버퍼의 크기
#define SIZEOF_GUIDREQINFO		20				// GUID Request Info를 담을 버퍼의 크기
#define SIZEOF_GUIDACKMSG		20				// GUID Ack Message를 담을 버퍼의 크기

#define ALLOWED_ALL_SESSION				0xFFFFFFFF
#define ALLOWED_ONLY_LATEST_SESSION		0x00000001

#define ANTICPSVR_CHECK_GAME_MEMORY			0x1		// Game에서 보호하는 Memory에 대한 Request만 만든다.
#define ANTICPSVR_CHECK_HACKSHIELD_FILE		0x2		// HackShield File에 대한 Request만 만든다.
#define ANTICPSVR_CHECK_GAME_FILE			0x4		// Game File에 대한 Request만 만든다.
#define ANTICPSVR_CHECK_ALL	( ANTICPSVR_CHECK_GAME_MEMORY | ANTICPSVR_CHECK_HACKSHIELD_FILE | ANTICPSVR_CHECK_GAME_FILE )


// 에러코드 정의
#define ERROR_ANTICPSVR_BASECODE									0x0001C000
#define ERROR_ANTICPSVR_INIT_INVALIDPARAM							ERROR_ANTICPSVR_BASECODE + 0x1
#define ERROR_ANTICPSVR_INIT_INSERTCRCDATATOLIST_FAIL				ERROR_ANTICPSVR_BASECODE + 0x2

#define ERROR_ANTICPSVR_MAKEREQMSG_INVALIDPARAM						ERROR_ANTICPSVR_BASECODE + 0x10
#define ERROR_ANTICPSVR_MAKEREQMSG_MAKESKEY_FAIL					ERROR_ANTICPSVR_BASECODE + 0x11
#define ERROR_ANTICPSVR_MAKEREQMSG_INITCRYPT_FAIL					ERROR_ANTICPSVR_BASECODE + 0x12
#define ERROR_ANTICPSVR_MAKEREQMSG_ENCRYPT_FAIL						ERROR_ANTICPSVR_BASECODE + 0x13
#define ERROR_ANTICPSVR_MAKEREQMSG_GETRNDHASHINFO_FAIL				ERROR_ANTICPSVR_BASECODE + 0x14

#define ERROR_ANTICPSVR_ANALACKMSG_INVALIDPARAM						ERROR_ANTICPSVR_BASECODE + 0x20
#define ERROR_ANTICPSVR_ANALACKMSG_MAKESKEY_FAIL					ERROR_ANTICPSVR_BASECODE + 0x21
#define ERROR_ANTICPSVR_ANALACKMSG_INITCRYPT_FAIL					ERROR_ANTICPSVR_BASECODE + 0x22
#define ERROR_ANTICPSVR_ANALACKMSG_DECRYPT_FAIL  					ERROR_ANTICPSVR_BASECODE + 0x23
#define ERROR_ANTICPSVR_ANALACKMSG_HSHIELD_ERROR 					ERROR_ANTICPSVR_BASECODE + 0x24
#define ERROR_ANTICPSVR_ANALACKMSG_PACKET_ERROR 					ERROR_ANTICPSVR_BASECODE + 0x25
#define ERROR_ANTICPSVR_ANALACKMSG_FILECRC_ERROR					ERROR_ANTICPSVR_BASECODE + 0x26
#define ERROR_ANTICPSVR_ANALACKMSG_MEMORYCRC_ERROR					ERROR_ANTICPSVR_BASECODE + 0x27
#define ERROR_ANTICPSVR_ANALACKMSG_INVLIDSESSION					ERROR_ANTICPSVR_BASECODE + 0x28

#define ERROR_ANTICPSVR_INITCRYPT_INVALIDPARAM						ERROR_ANTICPSVR_BASECODE + 0x30
#define ERROR_ANTICPSVR_MAKESESSIONKEY_INVALIDPARAM					ERROR_ANTICPSVR_BASECODE + 0x31
#define ERROR_ANTICPSVR_GETENCMSG_INVALIDPARAM						ERROR_ANTICPSVR_BASECODE + 0x32
#define ERROR_ANTICPSVR_GETDECMSG_INVALIDPARAM						ERROR_ANTICPSVR_BASECODE + 0x33
#define ERROR_ANTICPSVR_MAKEACKSEQ_INVALIDPARAM						ERROR_ANTICPSVR_BASECODE + 0x34
#define ERROR_ANTICPSVR_GETRNDHASHINFO_INVALIDPARAM					ERROR_ANTICPSVR_BASECODE + 0x35
#define ERROR_ANTICPSVR_GETRNDHASHINFO_INVALIDENTRY					ERROR_ANTICPSVR_BASECODE + 0x36
#define ERROR_ANTICPSVR_GETRNDHASHINFO_EMPTYFUNCTIONLIST_ERROR		ERROR_ANTICPSVR_BASECODE + 0x37
#define ERROR_ANTICPSVR_EXCEPTION_OCCURED							ERROR_ANTICPSVR_BASECODE + 0x38
#define ERROR_ANTICPSVR_GETGUIDFROMCRCFILE_INVALIDPARAM				ERROR_ANTICPSVR_BASECODE + 0x39

#define ERROR_ANTICPSVR_MAKEGUIDREQMSG_INVALIDPARAM					ERROR_ANTICPSVR_BASECODE + 0x40
#define ERROR_ANTICPSVR_MAKEGUIDREQMSG_MAKESKEY_FAIL				ERROR_ANTICPSVR_BASECODE + 0x41
#define ERROR_ANTICPSVR_MAKEGUIDREQMSG_INITCRYPT_FAIL				ERROR_ANTICPSVR_BASECODE + 0x42
#define ERROR_ANTICPSVR_MAKEGUIDREQMSG_ENCRYPT_FAIL					ERROR_ANTICPSVR_BASECODE + 0x43

#define ERROR_ANTICPSVR_ANALGUIDACKMSG_INVALIDPARAM					ERROR_ANTICPSVR_BASECODE + 0x50
#define ERROR_ANTICPSVR_ANALGUIDACKMSG_MAKESKEY_FAIL				ERROR_ANTICPSVR_BASECODE + 0x51
#define ERROR_ANTICPSVR_ANALGUIDACKMSG_INITCRYPT_FAIL				ERROR_ANTICPSVR_BASECODE + 0x52
#define ERROR_ANTICPSVR_ANALGUIDACKMSG_DECRYPT_FAIL  				ERROR_ANTICPSVR_BASECODE + 0x53
#define ERROR_ANTICPSVR_ANALGUIDACKMSG_PACKET_ERROR 				ERROR_ANTICPSVR_BASECODE + 0x54
#define ERROR_ANTICPSVR_ANALGUIDACKMSG_DENIED_NEWSESSION			ERROR_ANTICPSVR_BASECODE + 0x55
#define ERROR_ANTICPSVR_ANALGUIDACKMSG_GETGUIDFROMCRCFILE_ERROR		ERROR_ANTICPSVR_BASECODE + 0x56
#define ERROR_ANTICPSVR_ANALGUIDACKMSG_INSERTCRCDATATOLIST_FAIL		ERROR_ANTICPSVR_BASECODE + 0x57
#define ERROR_ANTICPSVR_ANALGUIDACKMSG_INVALIDGUID					ERROR_ANTICPSVR_BASECODE + 0x58



#ifndef UNIX
#ifdef ANTICPSVR_EXPORTS
#define ANTICPSVR_API  __declspec(dllexport)
#else
#define ANTICPSVR_API  __declspec(dllimport)
#endif
#else
#define IN
#define OUT
#define __stdcall
#define ANTICPSVR_API
#endif

#if defined(__cplusplus)
extern "C"
{
#endif


/*!
 * 
 * @remarks 이 함수는 서버가 로딩될때 한번 호출하는 함수로 내부적으로 사용하는 정보들을 초기화한다.
 *
 * @param	lpszHashFilePath : 해쉬정보저장 파일(HackShield.crc)에 대한 전체 경로
 *
 */
ANTICPSVR_API
unsigned long
__stdcall
_AntiCpSvr_Initialize (
	IN const char *lpszHashFilePath
	);


/*!
 * 
 * @remarks 이 함수는 서버가 언로딩될때 한번 호출하는 함수로 내부적으로 사용하던 정보들을 초기화한다.
 *
 */
ANTICPSVR_API
void
__stdcall
_AntiCpSvr_Finalize ();


/*!
 * 
 * @remarks 이 함수는 새로운 클라이언트가 연결될때 클라이언트당 한번 호출되는 함수로 현재 접속한
 *			클라이언트가 사용하는 GUID 버전이 무엇인지 확인하기 위한 요청메시지를 생성하는 함수이다.
 *
 * @param	pbyGuidReqMsg : 클라이언트에게 보낼 암호화된 Guid Request Message
 *							이 데이터를 담을 버퍼의 크기(Byte)는 위에 정의된 SIZEOF_GUIDREQMSG이다.
 * @param	pbyGuidReqInfo : _AntiCpSvr_AnalyzeGuidAckMsg() 에서 검사할때 사용할 원본 Guid Request Info
 *							 이 데이터를 담을 버퍼의 크기(Byte)는 위에 정의된 SIZEOF_GUIDREQINFO이다.
 *
 * @attention	이 함수를 호출하지 않으면 해당 클라이언트의 GUID를 알 수 없어 크랙 유무를 확인할 수 없다.
 *				또한 버퍼 크기는 앞으로 변경될 수도 있으므로 반드시 이 헤더파일에 정의된 것을 사용하여야한다.
 *
 */
ANTICPSVR_API
unsigned long
__stdcall
_AntiCpSvr_MakeGuidReqMsg (
	OUT unsigned char *pbyGuidReqMsg,
	OUT unsigned char *pbyGuidReqInfo
	);


/*!
 * 
 * @remarks 이 함수는 새로운 클라이언트가 연결될때 클라이언트당 한번 호출되는 함수로
 *			_AntiCpSvr_MakeGuidReqMsg()를 통해 생성된 GUID Request Message의 응답메시지를
 *			클라이언트가 암호화하여 보내면 해당 클라이언트의 GUID가 허용되는 버전인지 확인한 후
 *			허용되는 버전일 경우 앞으로 계속 사용하게 될 CRC Info의 주소를 리턴한다.
 *
 * @param	pbyGuidAckMsg : 클라이언트에서 보낸 암호화된 Guid ACK Message
 * @param	pbyGuidReqInfo : _AntiCpSvr_MakeGuidReqMsg() 함수에서 만들어진 원본 Guid Request Info
 * @param	ppCrcInfo : CRC Info를 담고있는 메모리를 가리킬 포인터를 저장할 버퍼
 *
 * @attention	여기서 리턴되는 ppCrcInfo에 담긴 주소는 해당 클라이언트의 Session이 끊길때까지
 *				계속해서 사용되므로 클라이언트별로 저장되고 관리되어야 한다.
 *
 */
ANTICPSVR_API
unsigned long
__stdcall
_AntiCpSvr_AnalyzeGuidAckMsg (
	IN unsigned char *pbyGuidAckMsg,
	IN unsigned char *pbyGuidReqInfo,
	OUT unsigned long **ppCrcInfo
	);


/*!
 * 
 * @remarks 이 함수는 클라이언트의 크랙 유무를 확인하고 싶을때마다 호출되는 함수로
 *			해당 클라이언트 버전에 맞는 CRC 정보를 이용하여 실행파일 및 메모리, 핵쉴드
 *			모듈이 정상적으로 동작하는지 확인하는 Request Message를 생성해 낸다.
 *
 * @param	pCrcInfo : _AntiCpSvr_AnalyzeGuidAckMsg() 함수에서 구해진 CRC Info의 메모리 주소
 * @param	pbyReqMsg : 클라이언트에게 보낼 암호화된 Request Message
 *						이 정보를 담을 버퍼의 크기(Byte)는 위에 정의된 SIZEOF_REQMSG이다.
 * @param	pbyReqInfo : _AntiCpSvr_AnalyzeAckMsg() 에서 검사할때 사용할 원본 Request Info
 *						 이 정보를 담을 버퍼의 크기(Byte)는 위에 정의된 SIZEOF_REQINFO이다.
 * @param	ulOption : 어떤 정보들에 대한 Request Message를 만들지에 대한 Flag, 위에 정의된
 *						ANTICPSVR_CHECK_GAME_MEMORY, ANTICPSVR_CHECK_HACKSHIELD_FILE,
 *						ANTICPSVR_CHECK_GAME_FILE 들의 OR를 이용하여 정의할 수 있다.
 *						단, 안전을 위해 최초 호출시는 ANTICPSVR_CHECK_ALL Option을 이용하여
 *						전체에 대한 안전 유무를 검사하고 그 다음부터는 Performance를 위해
 *						ANTICPSVR_CHECK_GAME_MEMORY Option만 사용하길 권장한다.
 *
 * @attention	버퍼 크기는 앞으로 변경될 수도 있으므로 반드시 이 헤더파일에 정의된 것을 사용하여야한다.
 *
 */
ANTICPSVR_API
unsigned long
__stdcall
_AntiCpSvr_MakeReqMsg (
	IN unsigned long *pCrcInfo,
	OUT unsigned char *pbyReqMsg,
	OUT unsigned char *pbyReqInfo,
	IN unsigned long ulOption
	);


/*!
 * 
 * @remarks 이 함수는 _AntiCpSvr_MakeReqMsg() 함수를 통해 생성된 크랙 유무 Request Message에
 *			대한 응답으로 클라이언트로부터 암호화된 Ack Message가 올 경우 해당 Ack Message를
 *			분석하여 클라이언트가 정상적인지 검사한다.
 *
 * @param	pCrcInfo : _AntiCpSvr_AnalyzeGuidAckMsg() 함수에서 구해진 CRC Info의 메모리 주소
 * @param	pbyAckMsg : 클라이언트에서 보낸 암호화된 ACK Message
 * @param	pbyReqInfo : _AntiCpSvr_MakeReqMsg() 함수에서 만들어진 원본 Request Message 정보
 *
 */
ANTICPSVR_API
unsigned long
__stdcall
_AntiCpSvr_AnalyzeAckMsg (
	IN unsigned long *pCrcInfo,
	IN unsigned char *pbyAckMsg,
	IN unsigned char *pbyReqInfo
	);

#if defined(__cplusplus)
}
#endif

#endif //_ANTICPSVRFUNC_H
