/*
	CommonLog.h
	-----------

	Programming by Joongpil Cho
	All copyright (c) 1999, MAIET entertainment
*/
#include <stdio.h>

#ifndef __COMMON_HEADER__
#define __COMMON_HEADER__


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
			m_pFileName = strdup(szFileName);
		} else {
			m_pFileName = strdup("mlog.txt");
		}
	}

	virtual ~CMFileLog(){
		Shutdown();
	}

	virtual void Init(){
		DeleteFile(m_pFileName);
	}

	virtual void Shutdown(){
		if(m_pFileName){
			free(m_pFileName);
			m_pFileName = NULL;
		}
	}

	virtual void Print( const char *string ){
		FILE *pFile;

		pFile = fopen( m_pFileName, "a" );
		if( !pFile ){
			pFile = fopen( m_pFileName, "w" );
			fclose( pFile );
			pFile = fopen( m_pFileName, "a" );
		}
		fprintf(pFile,string);
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
		OutputDebugString(pFormat);
	}
};



void SetLogHandler( CMLog *pLog );
void Log( const char *pFormat, ... );



#endif // __COMMON_HEADER__