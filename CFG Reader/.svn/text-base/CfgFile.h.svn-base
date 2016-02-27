/*
	CfgFile.h
	---------

	Programming by Joongpil Cho
	All copyright (c) reserved by MAIET entertainment
*/
#include <stdio.h>
#include <stdlib.h>
#include <CMList.h>

#ifndef __CFGFILE_HEADER__
#define __CFGFILE_HEADER__


#define __MAX_STRING__	256


#define CFG_NOERROR		0	// No Error
#define CFG_UNEXPECTED	1	// Unexpected Token
#define CFG_INVALID		2	// Invalid Token
#define CFG_OPENFAIL	3	// Open Failure


#define CFG_INTEGERTYPE	0
#define CFG_REALTYPE	1
#define CFG_STRINGTYPE	2


typedef union {
	char*	szval;
	int		nval;
	float	fval;
} CFG32V;

/*
	Token Definition
	32 bit system only
*/
typedef struct _tagCfgToken {
	int		id;
	CFG32V	v;
} CFGTOKEN, *PCFGTOKEN, **PPCFGTOKEN;

class CFGVALUE {
public :
	CFGTOKEN	tValue;
	char*		szKey;

	CFGVALUE(char *Key){
		szKey = strdup(Key);
	}
	
	~CFGVALUE(){
		if( tValue.id == 2 && tValue.v.szval != NULL ){
			free(tValue.v.szval);
			tValue.v.szval = NULL;
		}

		if(szKey){ free(szKey); szKey = NULL; }
	}

	void SetInteger(int a){
		tValue.id = CFG_INTEGERTYPE;		// TINTEGER
		tValue.v.nval = a;
	}

	void SetReal(float a){
		tValue.id = CFG_REALTYPE;			// TREAL
		tValue.v.fval = a;
	}

	void SetString(char *szVal){
		tValue.id = CFG_STRINGTYPE;			// TSTRING
		tValue.v.szval = strdup(szVal);
	}

	int GetInteger(){
		switch(tValue.id){
		case CFG_INTEGERTYPE:	//INTEGER
			return tValue.v.nval;
		case CFG_REALTYPE:		// REAL
			return (int)tValue.v.fval;
		case CFG_STRINGTYPE:	// STRING
			return atoi(tValue.v.szval);
		}
		return 0;
	}

	float GetReal(){
		switch(tValue.id){
		case CFG_INTEGERTYPE :	//INTEGER
			return (float)(tValue.v.nval);
		case CFG_REALTYPE :		//REAL
			return tValue.v.fval;
		case CFG_STRINGTYPE :	//STRING
			return (float)atof(tValue.v.szval);
		}
		return 0;
	}

	BOOL GetString(char *szBuffer){
		if(tValue.id == CFG_STRINGTYPE){	//자동 타입 캐스팅을 하지 않는다.
			strcpy(szBuffer, tValue.v.szval);
			return TRUE;
		}
		return FALSE;		
	}

	// 0 : Integer, 1 : Real, 2 : String
	int GetType(){ return tValue.id; }
};
typedef CFGVALUE *PCFGVALUE;

class CFGSECTION {
public:
	char	*szKey;	// SECTION NAME
	CMLinkedList<CFGVALUE>	aValList;

	CFGSECTION( char *Key ){
		szKey = strdup(Key);
	}

	virtual ~CFGSECTION(){
		aValList.DeleteAll();
		if(szKey){ free(szKey); szKey = NULL; }
	}
	
	PCFGVALUE GetValue( char *szName ){
		PCFGVALUE pValue = NULL;
		int i, cnt;

		if(szName == NULL) return NULL;

		cnt = aValList.GetCount();
		for( i = 0; i < cnt; i ++ ){
			pValue = aValList.Get(i);
			if(pValue!=NULL && (strcmp(pValue->szKey, szName)==0)) return pValue;
		}

		return NULL;
	}
	PCFGVALUE GetValue(int nIndex){
		return aValList.Get(nIndex);
	}

	int GetValueCount(){ return aValList.GetCount(); }
};
typedef CFGSECTION *PCFGSECTION;

class CfgFile {
private:
	/* Scanner에 관련된 변수들 */
	char	m_szText[__MAX_STRING__];	// same with yytext
	int		m_nTextLen;					// same with yyleng
	char	m_pbbuf[5];					// unput buffer
	char	m_ibuf[__MAX_STRING__];		// input buffer
	int		m_iptr;
	int		m_pbptr;

	FILE	*m_FilePointer;

	int		m_nLineNum;
	int		m_nErrorLine;

	CMLinkedList<CFGSECTION>	aSectionList;
	CFGTOKEN Scan();

	char Input();
	void Unput(char c);
	void ReadLine();

	inline void Gather(char c){
		m_szText[m_nTextLen++] = c;
	}

	inline int GetDigit(char c){
		int ret =0;		
		for(;isdigit(c)||c=='.';c=Input()){
			if(c=='.') ret++;
			if(ret > 1) break;
			Gather(c);
		}
		Unput(c);

		return ret;
	}

	void FreeToken(PCFGTOKEN token);

public :
	CfgFile(){ m_nErrorLine = -1; m_nLineNum = 0; m_FilePointer = NULL; }
	virtual ~CfgFile(){ Close(); }

	/* CFG 파일을 연다. */
	int Open(char *szFileName);

	BOOL OpenBinary(char *szFileName);
	void SaveBinary(char *szFileName);

	void Close(){ aSectionList.DeleteAll(); m_nErrorLine = -1; }

	/* Open중에 생긴 에러가 몇번째 줄에서 일어난 것인가를 판별한다. */
	int GetErrorLine(){ return m_nErrorLine; }
	int GetTotalLine(){ return m_nLineNum; }

	/* 섹션 이름으로 객체를 얻어낸다. */
	PCFGSECTION	GetSection(char *szName = NULL){
		PCFGSECTION pRet = NULL;
		int i, cnt;

		cnt = aSectionList.GetCount();
		for( i = 0; i < cnt; i ++ ){
			pRet = aSectionList.Get(i);
			if(pRet!=NULL){
				if(szName == NULL){
					if(strcmp(pRet->szKey, "@GLOBAL")==0) return pRet;
				} else {
					if(strcmp(pRet->szKey, szName)==0) return pRet;
				}
			}
		}
		return NULL;
	}
	
	PCFGSECTION GetSection(int nIndex){
		return aSectionList.Get(nIndex);
	}

	/* 현재 읽혀진 파일내의 섹션 갯수를 센다. */
	int GetSectionCount(){ return aSectionList.GetCount(); }
};



#endif // __CFGFILE_HEADER__
