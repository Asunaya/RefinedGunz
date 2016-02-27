/*
	LibPak.h
	--------

	Memory mapped file operations service library
	This library based on Jang-ho's memory mapped file library.
	(7/16/1996)
	
	Programming by Chojoongpil
	Update. 6/3/1997

	All copyright (c) 1997, MAIET entertainment software
*/
#include <windows.h>

#include "CMList.h"

#ifdef _DEBUG
	#include <crtdbg.h>
#endif

#ifndef __LIBPAK_HEADER__
#define __LIBPAK_HEADER__

/* --------------------------------------------------------
		MemoryMappedFile
   -------------------------------------------------------- */

class MemoryMappedFile {
private:
	HANDLE	m_fileHandle;	// file handler
	HANDLE	m_mapHandle;	// mapped File handler

	DWORD	m_nFileSize;	// file size of memory mapped file
	LPBYTE	m_lpPointer;	// memory pointer of file head

	BOOL	m_bOpened;
	DWORD	m_nPosition;	// current memory position
public:
	BOOL IsOpen(){
		return m_bOpened; 
	}

	MemoryMappedFile(){
		m_lpPointer = NULL;
		m_nPosition = 0;
		m_nFileSize = 0;

		m_fileHandle = NULL;
		m_mapHandle = NULL;
		m_bOpened = FALSE;
	}

	~MemoryMappedFile(){
		Close();			// close all handles if avail.
	}

	// open memory mapped file.
	//	Modified by Leejangho ( 98-01-10 5:30:04 오전 )
	//		LPBYTE Open( char *lpszFileName )
	LPBYTE Open( const char *lpszFileName , BOOL bReadOnly=TRUE );
	//	Modified by ...

	// close memory mapped file.
	void Close(void);
	
	// get size of memory mapped file.
	DWORD GetFileSize(){ return m_nFileSize; }
	
	// get current file pointer
	LPBYTE GetFilePointer(){ return (m_lpPointer + m_nPosition); }
	LPBYTE GetStartFilePointer(){ return m_lpPointer; }
	
	// set current file pointer
	BOOL SetFilePointer(DWORD lDistanceToMove, DWORD dwMoveMethod = FILE_CURRENT){
		if( dwMoveMethod == FILE_BEGIN ){
			if( lDistanceToMove < m_nFileSize ){
				m_nPosition = lDistanceToMove;
				return TRUE;
			}
		} else if( dwMoveMethod == FILE_END ){
			if( m_nFileSize - lDistanceToMove > 0 ){
				m_nPosition = (m_nFileSize - 1) - lDistanceToMove;
				return TRUE;
			}
		} else {	// FILE_CURRENT
			if( m_nPosition + lDistanceToMove < m_nFileSize ){
				m_nPosition += lDistanceToMove;
				return TRUE;
			}			
		}
		return FALSE;
	}

	// read a byte from memory mapped file
	BYTE ReadByte(){ 
		m_nPosition ++;
		return *(m_lpPointer+m_nPosition); 
	}

	// read data from memory mapped file
	// return value : actual reading data length
	DWORD ReadFile(LPVOID lpBuffer, DWORD dwSize){
		DWORD dwReadSize;

#ifdef _DEBUG
		_ASSERT(lpBuffer != NULL);
#endif
		if( m_nPosition + dwSize >= m_nFileSize ){
			dwReadSize = m_nFileSize - 1 - m_nPosition;
		} else dwReadSize = dwSize;

		CopyMemory(lpBuffer, m_lpPointer+m_nPosition, dwReadSize);
		m_nPosition += dwReadSize;

		return dwReadSize;
	}
};

#define ALIASLENGTH		80

/* ---------------------------------
	Data Structures
	---------------

	PAK

	+-------------+
	|    Header   |
	+-------------+
	|   DATAINFO  |
	+-------------+
	|   DATAINFO  | DATAINFO는 Header에서
	+-------------+ 명시한 갯수 만큼이 놓여진다.
	|    DATA     |
	+-------------+
	|    DATA     |
	+-------------+
	
   --------------------------------- */
#pragma pack(1)
typedef struct _tagPakHeader {
	char			szID[4];	// PAK
	long			nVerMajor;	// Major version is 1
	long			nVerMinor;	// Minor version is 0
	unsigned long	nCount;		// 포함되어진 package 화일 갯수
} PAKFILEHEADER, *LPPAKFILEHEADER;

typedef struct {
	char			pszAlias[ALIASLENGTH];
	unsigned long	ulOffset;
	unsigned long	ulSize;
} PAKDATAINFO, *LPPAKDATAINFO;
#pragma pack()

/* --------------------------------------------------------
		PakData
   -------------------------------------------------------- */
class Package;

class PakData {
private:
	LPBYTE m_lpPointer;		// start pointer of extract data
	DWORD m_nPosition;		// current position

	unsigned long m_ulSize;	// block size

	// set start pointer (m_lpPointer) and sizes
	void ResetPakData(LPBYTE p, unsigned long ulSize){
#ifdef _DEBUG
		_ASSERT(p!=NULL);
		_ASSERT(ulSize!=0);
#endif
		m_lpPointer = p;
		m_ulSize = ulSize;
		m_nPosition = 0;
	}

public:
	// Constructor
	PakData(LPBYTE p, unsigned long ulSize){
		ResetPakData(p, ulSize);
	}

	unsigned long GetFileSize(){ return m_ulSize; }
	LPBYTE GetStartFilePointer(){ return m_lpPointer; }

	// get current file pointer
	LPBYTE GetFilePointer(){ return (m_lpPointer + m_nPosition); }
	
	// set current file pointer
	BOOL SetFilePointer(DWORD lDistanceToMove, DWORD dwMoveMethod = FILE_CURRENT){
		if( dwMoveMethod == FILE_BEGIN ){
			if( lDistanceToMove < m_ulSize ){
				m_nPosition = lDistanceToMove;
				return TRUE;
			}
		} else if( dwMoveMethod == FILE_END ){
			if( m_ulSize - lDistanceToMove > 0 ){
				m_nPosition = (m_ulSize - 1) - lDistanceToMove;
				return TRUE;
			}
		} else {	// FILE_CURRENT
			if( m_nPosition + lDistanceToMove < m_ulSize ){
				m_nPosition += lDistanceToMove;
				return TRUE;
			}			
		}
		return FALSE;
	}

	// read data from pak data
	// return value : actual reading data length
	DWORD ReadFile(LPVOID lpBuffer, DWORD dwSize){
		DWORD dwReadSize;

#ifdef _DEBUG
		_ASSERT(lpBuffer != NULL);
#endif
		if( m_nPosition + dwSize > m_ulSize ){
			dwReadSize = m_ulSize - 1 - m_nPosition;
		} else dwReadSize = dwSize;

		CopyMemory(lpBuffer, m_lpPointer+m_nPosition, dwReadSize);
		m_nPosition += dwReadSize;

		return dwReadSize;
	}
	
	friend Package;
};

typedef PakData PAKDATA, *LPPAKDATA;

/* --------------------------------------------------------
		Package
   -------------------------------------------------------- */

class Package {
private:
	unsigned long m_nFiles;		// Package내에 포함된 파일의 갯수
	MemoryMappedFile m_mmf;		// Memory mapped file object

public:
	CMLinkedList<PAKDATAINFO>	m_FileList;

	Package(){
		m_nFiles = 0;
	}
	~Package(){
		Close();
	}

	BOOL Open( char *lpszFilename );
	void Close();

	// get PAK data object from package file
	LPPAKDATA GetPakData(unsigned long ulIndex);
	LPPAKDATA GetPakData(char* Name);
};


#endif // __LIBPAK_HEADER__