/*
* (C) Copyright 2005. Ahn Lab, Inc.
*
* Any part of this source code can not be copied with
* any method without prior written permission from
* the author or authorized person.
*
*/

/*
* File Name : HsCryptLib.h
* Author: Nam Sung il(oncea@ahnlab.com)
* Revision History
* Date         Name                    Description
* 2005-07-27   Nam, Sung-Il(siNam)     Created
*
*/
#ifndef _HSCRYPTLIB_H
#define _HSCRYPTLIB_H

#if defined(__cplusplus)
extern "C"
{
#endif

#define HSCRYPTLIB_INITKEY_SIZE		16
#define HSCRYPTLIB_KEY_SIZE			550

#pragma pack(8)
// 키구조체.
typedef struct _HSCRYPT_KEYINFO
{
	BYTE  byInitKey[HSCRYPTLIB_INITKEY_SIZE];
	BYTE  AesEncKey[HSCRYPTLIB_KEY_SIZE];
	BYTE  AesDecKey[HSCRYPTLIB_KEY_SIZE];
} HSCRYPT_KEYINFO, *PHSCRYPT_KEYINFO;
#pragma pack()

// ERROR CODE DEFINE
#define ERROR_HSCRYPTLIB_BASECODE						0x0001B000
#define ERROR_HSCRYPTLIB_UNKNOWN						ERROR_HSCRYPTLIB_BASECODE + 0x0
#define ERROR_HSCRYPTLIB_EXCEPTION						ERROR_HSCRYPTLIB_BASECODE + 0x1
#define ERROR_HSCRYPTLIB_INITCRYPT_INVALIDPARAM			ERROR_HSCRYPTLIB_BASECODE + 0x2
#define ERROR_HSCRYPTLIB_GETENCMSG_INVALIDPARAM			ERROR_HSCRYPTLIB_BASECODE + 0x3
#define ERROR_HSCRYPTLIB_GETDECMSG_INVALIDPARAM			ERROR_HSCRYPTLIB_BASECODE + 0x4

#define ERROR_HSCRYPTLIB_FREAD_INVALIDPARAM				ERROR_HSCRYPTLIB_BASECODE + 0x5
#define ERROR_HSCRYPTLIB_FREAD_DECRYPT_RANGE			ERROR_HSCRYPTLIB_BASECODE + 0x6
#define ERROR_HSCRYPTLIB_FREAD_DECRYPT_FREAD			ERROR_HSCRYPTLIB_BASECODE + 0x7
#define ERROR_HSCRYPTLIB_FREAD_DECRYPT_GETDECMSG		ERROR_HSCRYPTLIB_BASECODE + 0x8
#define ERROR_HSCRYPTLIB_FREAD_GETFILELEN				ERROR_HSCRYPTLIB_BASECODE + 0x9
#define ERROR_HSCRYPTLIB_FREAD_GETPOSITION				ERROR_HSCRYPTLIB_BASECODE + 0xA
#define ERROR_HSCRYPTLIB_FREAD_SIZEZERO					ERROR_HSCRYPTLIB_BASECODE + 0xB
#define ERROR_HSCRYPTLIB_FREAD_FSEEK					ERROR_HSCRYPTLIB_BASECODE + 0xC

#define ERROR_HSCRYPTLIB_FILEFENC_INVALIDPARAM			ERROR_HSCRYPTLIB_BASECODE + 0x10
#define ERROR_HSCRYPTLIB_FILEFENC_FOPEN					ERROR_HSCRYPTLIB_BASECODE + 0x11
#define ERROR_HSCRYPTLIB_FILEFENC_GETFILELEN			ERROR_HSCRYPTLIB_BASECODE + 0x12	
#define ERROR_HSCRYPTLIB_FILEFENC_SIZEZERO				ERROR_HSCRYPTLIB_BASECODE + 0x13
#define ERROR_HSCRYPTLIB_FILEFENC_INITCRYPT				ERROR_HSCRYPTLIB_BASECODE + 0x14
#define ERROR_HSCRYPTLIB_FILEFENC_COPYFILE				ERROR_HSCRYPTLIB_BASECODE + 0x15
#define ERROR_HSCRYPTLIB_FILEFENC_ENCFILE				ERROR_HSCRYPTLIB_BASECODE + 0x16
#define ERROR_HSCRYPTLIB_FILEFENC_FWRITE				ERROR_HSCRYPTLIB_BASECODE + 0x17
#define ERROR_HSCRYPTLIB_FILEFENC_ENCMSG				ERROR_HSCRYPTLIB_BASECODE + 0x18
	
#define ERROR_HSCRYPTLIB_ENCFFILE_INVALIDPARAM			ERROR_HSCRYPTLIB_BASECODE + 0x20
#define ERROR_HSCRYPTLIB_ENCFFILE_FREAD					ERROR_HSCRYPTLIB_BASECODE + 0x21
#define ERROR_HSCRYPTLIB_ENCFFILE_GETENCMSG				ERROR_HSCRYPTLIB_BASECODE + 0x22
	
#define ERROR_HSCRYPTLIB_FILEFDECBLK_INVALIDPARAM		ERROR_HSCRYPTLIB_BASECODE + 0x30
#define ERROR_HSCRYPTLIB_FILEFDECBLK_FOPEN				ERROR_HSCRYPTLIB_BASECODE + 0x31
#define ERROR_HSCRYPTLIB_FILEFDECBLK_GETFILELEN			ERROR_HSCRYPTLIB_BASECODE + 0x32
#define ERROR_HSCRYPTLIB_FILEFDECBLK_FILESEEK			ERROR_HSCRYPTLIB_BASECODE + 0x33
#define ERROR_HSCRYPTLIB_FILEFDECBLK_INITCRYPT			ERROR_HSCRYPTLIB_BASECODE + 0x34
#define ERROR_HSCRYPTLIB_FILEFDECBLK_FREAD				ERROR_HSCRYPTLIB_BASECODE + 0x35
#define ERROR_HSCRYPTLIB_FILEFDECBLK_FWRITE				ERROR_HSCRYPTLIB_BASECODE + 0x36
	
#define ERROR_HSCRYPTLIB_ISFILEENC_INVALIDPARAM			ERROR_HSCRYPTLIB_BASECODE + 0x40
#define ERROR_HSCRYPTLIB_ISFILEENC_FSEEK				ERROR_HSCRYPTLIB_BASECODE + 0x41
#define ERROR_HSCRYPTLIB_ISFILEENC_FREAD				ERROR_HSCRYPTLIB_BASECODE + 0x42
#define ERROR_HSCRYPTLIB_ISFILEENC_GETDECMSG			ERROR_HSCRYPTLIB_BASECODE + 0x43
	
	
// FUNCTION DEFINE
/*************************************************************************************
	함수명 : _HsCrypt_InitCrypt
	목  적 : 암/복호화를 위한 초기화 기능 수행
	인자값 : PHSCRYPT_KEYINFO pHsKeyInfo
	반환값 : ERROR_SUCCESS, ERROR_HSCRYPTLIB_INITCRYPT_INVALIDPARAM
	날  짜 : 2005. 7. 27
	작업자 : 남성일
*************************************************************************************/
DWORD __stdcall _HsCrypt_InitCrypt ( IN OUT PHSCRYPT_KEYINFO pHsKeyInfo  );

/*************************************************************************************
	함수명 : _AntiCpCnt_GetEncMsg
	목  적 : 입력된 버퍼를 암호화하여 버퍼로 출력한다.
	인자값 : pbyInput, nInLength, pbyOutput, pAesEncKey, pbyOutput, nOutLength
	반환값 : ERROR_HSCRYPTLIB_GETENCMSG_INVALIDPARAM, ERROR_SUCCESS
	날  짜 : 2005. 7. 21
	작업자 : 남성일
*************************************************************************************/
DWORD __stdcall _HsCrypt_GetEncMsg ( IN PBYTE pbyInput,				// [in] 암호화할 버퍼			
									 IN UINT nInLength,				// [in] 암호화할 사이즈
									 IN PBYTE pAesEncKey,			// [in] 암호화키
									 OUT PBYTE pbyOutput,			// [out] 암호화된 버퍼
									 IN UINT nOutLength);			// [in] 암호화된 사이즈 

/*************************************************************************************
	함수명 : _HsCrypt_GetDecMsg
	목  적 : 입력된 버퍼를 복호화하여 버퍼로 출력한다.
	인자값 : pbyInput, nInLength, pAesDecKey, pbyOutput, nOutLength
	반환값 : ERROR_SUCCESS, ERROR_HSCRYPTLIB_GETDECMSG_INVALIDPARAM
	날  짜 : 2005. 7. 21
	작업자 : 남성일
*************************************************************************************/
DWORD __stdcall _HsCrypt_GetDecMsg ( IN PBYTE pbyInput,					// [in] 복호화할 버퍼			
									 IN UINT nInLength,					// [in] 복호화할 사이즈
									 IN PBYTE pAesDecKey,				// [in] 복호화 키
									 OUT PBYTE pbyOutput,				// [out] 복호화된 버퍼
									 IN UINT nOutLength );				// [in] 복호화된 사이즈

/*************************************************************************************
	함수명 : _HsCrypt_FRead
	목  적 : 파일구조체 포인터를 이용하여 원하는 블럭만 복호화후 버퍼로 출력
	인자값 : lpOutBuffer, dwDecryptSize, pInputStream, pAesDecKey
	반환값 : ERROR_SUCCESS, ERROR_HSCRYPTLIB_FREAD_INVALIDPARAM,
	         ERROR_HSCRYPTLIB_FREAD_DECRYPT_RANGE, ERROR_HSCRYPTLIB_FREAD_DECRYPT_FREAD, 
			 ERROR_HSCRYPTLIB_FREAD_DECRYPT_GETDECMSG, ERROR_HSCRYPTLIB_EXCEPTION
	날  짜 : 2005. 7. 27
	작업자 : 남성일
*************************************************************************************/
DWORD __stdcall _HsCrypt_FRead ( OUT LPVOID lpOutBuffer,			// [out] 복호화된 버퍼
								 IN DWORD dwDecryptSize,			// [in] 복호화(읽을) 사이즈
								 IN FILE *pInputStream,				// [in] 읽을 파일포인터.
								 IN PBYTE pAesDecKey,				// [in] 복호화 키 
								 OUT PDWORD pdwReadLen);			// [in] 복호화된 사이즈


/*************************************************************************************
	함수명 : _HsCrypt_FileFEnc
	목  적 : 파일 암호화
	인자값 : lpszInputFile, lpszOutputFile
	반환값 : ERROR_SUCCESS, ERROR_HSCRYPTLIB_FILEFENC_INVALIDPARAM,
	         ERROR_HSCRYPTLIB_FILEFENC_FOPEN, ERROR_HSCRYPTLIB_FILEFENC_GETFILELEN, 
			 ERROR_HSCRYPTLIB_FILEFENC_SIZEZERO, ERROR_HSCRYPTLIB_FILEFENC_INITCRYPT, 
			 ERROR_HSCRYPTLIB_FILEFENC_COPYFILE, ERROR_HSCRYPTLIB_FILEFENC_ENCFILE,
			 ERROR_HSCRYPTLIB_FILEFENC_FWRITE, ERROR_HSCRYPTLIB_FILEFENC_ENCMSG,
	날  짜 : 2005. 7. 21
	작업자 : 남성일
*************************************************************************************/
DWORD __stdcall _HsCrypt_FileFEnc ( IN LPCTSTR lpszInputFile,				// [in] 암호화할 파일경로
									IN PBYTE pbyInitKey,					// [in] 암복호화 키 
								    OUT LPTSTR lpszOutputFile );			// [out] 원본을 백업한 파일경로 

#if defined(__cplusplus)
}
#endif //(__cplusplus)

#endif //_HSCRYPTLIB_H