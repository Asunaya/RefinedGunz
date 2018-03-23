/*
	CommonLog.h
	-----------

	Programming by Joongpil Cho
	All copyright (c) 1999, MAIET entertainment
*/
#pragma once

#include <stdio.h>
#include "MFile.h"

// Base Class of General Log System
class CMLog {
public :
	virtual void Init() = 0;
	virtual void Shutdown() = 0;
	virtual void Print( const char *pFormat ) = 0;
};

/* CMNullLog */
class CMNullLog : public CMLog {
public :
	virtual void Init(){}
	virtual void Shutdown(){}
	virtual void Print( const char *pFormat ){}
};

/* CMFileLog */
class CMFileLog : public CMLog {
	char *m_pFileName;
public :
	CMFileLog( char *szFileName = NULL ){
		if( szFileName ){
			m_pFileName = _strdup(szFileName);
		} else {
			m_pFileName = _strdup("mlog.txt");
		}
	}

	virtual ~CMFileLog(){
		Shutdown();
	}

	virtual void Init(){
		MFile::Delete(m_pFileName);
	}

	virtual void Shutdown(){
		if(m_pFileName){
			free(m_pFileName);
			m_pFileName = NULL;
		}
	}

	virtual void Print( const char *string ){
		FILE *pFile = fopen( m_pFileName, "a" );

		if( pFile == nullptr ){
			pFile = fopen( m_pFileName, "w" );
			fclose( pFile );
			pFile = fopen( m_pFileName, "a" );

			if( pFile == nullptr )
				return;
		}

		fputs(string,pFile);
		fclose(pFile);
	}
};

class CMDebugStringLog : public CMLog {
public :
	virtual void Init(){
		//DO NOTHING
	}
	virtual void Shutdown(){
		//DO NOTHING
	}
	virtual void Print( const char *pFormat ){
	}
};

void SetLogHandler( CMLog *pLog );
void Log( const char *pFormat, ... );