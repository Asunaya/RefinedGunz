/*
	CfgFile.cpp

	Programming by Joongpil Cho
	All copyright (c) reserved by MAIET entertainment
*/
#include "stdafx.h"
#include "CfgFile.h"

enum TOKENTYPE {
	TINVALID	= -1,
	TINTEGER	= 0,	// 절대 에디트 하지 말것!!!
	TREAL		= 1,	// 절대 에디트 하지 말것!!!
	TSTRING		= 2,	// 절대 에디트 하지 말것!!!
	TSYMBOL		= 3,
	TEQUAL		= 4,	// =
	TUNARY		= 5,	// -
	TSECTION	= 6,	// !
	TEOF		= 127
};

struct _reserved_word {
	TOKENTYPE	val;
	char*		szText;
} rwords[] = {
	{ TINVALID, NULL }
};
/*
static char *fold( char *s )
{
	int i;
	for(i=0; s[i]!='\0'; i++)
		if(isupper(s[i])) s[i] = s[i] + ('a'-'A');
    
	return(s);
}
*/

char CfgFile::Input()
{
	if(m_pbptr > -1){
		return(m_pbbuf[m_pbptr--]);
	} else {
		if(m_ibuf[m_iptr] == '\0') ReadLine();
		if(m_ibuf[m_iptr]!='\0'){
			return(m_ibuf[m_iptr++]);
		} else {
			return 0;
		}
	}
	return 0;
}

void CfgFile::ReadLine()
{
    char c;

    m_iptr = 0;
    for(c=fgetc(m_FilePointer); c!='\n' && c!=EOF; c=fgetc(m_FilePointer)) m_ibuf[m_iptr++] = c;
    
	m_ibuf[m_iptr++] = c;
    m_ibuf[m_iptr++] = '\0';
    m_iptr = 0;
	m_nLineNum ++;
}

void CfgFile::Unput(char c)
{
	m_pbbuf[++m_pbptr] = c;
}

CFGTOKEN CfgFile::Scan()
{
	CFGTOKEN ret;	
    int i;
	char c;	

	ret.id = TINVALID;
	
loop:
	c = Input();
	if(c=='\t' || c==' '){
		goto loop;
	} else if(c=='\n' || c==';'){
		ReadLine();
		goto loop;
	} else if(isdigit(c) || c=='.'){
		m_nTextLen = 0;
		
		if( GetDigit(c) > 0 ) ret.id = TREAL; else ret.id = TINTEGER;
		
		if(m_nTextLen == 1 && m_szText[0]=='.') goto reterr; 
		if(m_nTextLen == 0) goto reterr;

		Gather('\0');

		if( ret.id == TINTEGER ){
			ret.v.nval = atoi(m_szText);
		} else if( ret.id == TREAL ){
			ret.v.fval = (float)atof(m_szText);
		}
		return ret;
reterr:
		ret.id = TINVALID;
		return ret;
	}

	// SYMBOL

    else if( isalpha(c) ){
		m_nTextLen=0;
		Gather(c);
		for( c = Input(); isalpha(c) || isdigit(c); c=Input() ) Gather(c);
		Unput(c);
		Gather('\0');

		for( i=0; rwords[i].val != TINVALID; i++ ){
			if( strcmp(m_szText, rwords[i].szText) == 0 ){
				break;
			}
		}
		
		if( rwords[i].val != TINVALID ){
			ret.id = rwords[i].val;
			return ret;
		} else {
			ret.id = TSYMBOL;
			ret.v.szval = strdup(m_szText);
			return ret;
		}
	}

	// STRING CONSTANT
    else if(c == '\"'){
		m_nTextLen = 0;
		for( c = Input(); ; c=Input() ){
			if(c == '\"'){
				if((c=Input()) == '\"'){					
					Gather('\\');
					Gather('\"');
				} else {
					break;
				}
			} else if(c == '\\'){
				c=Input();
				Gather(c);
				if(c == '\n') ReadLine();
			} else if(c == '\n'){
				ret.id = TINVALID;
				ReadLine();
				return ret;
			} else {
				Gather(c);
			}
		}
		Unput(c);
		Gather('\0');

		ret.id = TSTRING;
		ret.v.szval = strdup(m_szText);
		return ret;
    } else if(c == '='){
		ret.id = TEQUAL;
	} else if(c == '!'){
		ret.id = TSECTION;
	} else if(c == '-'){
		ret.id = TUNARY;
	} else if(c == EOF){
		ret.id = TEOF;
	}

	return ret;
}

void CfgFile::FreeToken(PCFGTOKEN token)
{
	if( token->id == TSYMBOL || token->id == TSTRING ){
		if( token->v.szval ){
			free(token->v.szval);
			token->v.szval = NULL;
		}
	}
}

int CfgFile::Open(char *filename)
{
	CFGTOKEN	token;
	PCFGSECTION	pCurrentSection	= NULL;	
	PCFGVALUE	pValue = NULL;
	int			ret				= CFG_NOERROR;

	m_nErrorLine = -1; m_nLineNum = 0;

	m_FilePointer = fopen(filename, "r");
	if(m_FilePointer == NULL) {
		ret = CFG_OPENFAIL;	
		goto return_label;
	}
	m_iptr = -1; m_pbptr = -1;
	ReadLine();

	while(1){		
		token = Scan();

		if( token.id == TEOF ){
			goto return_label;
		}
		if( token.id == TINVALID ){
			ret = CFG_INVALID;
			goto return_label;
		}

		switch(token.id){
		case TINTEGER :
		case TREAL :
		case TSTRING :
			if( pCurrentSection == NULL ){	// 만일 NULL이라면...
				pCurrentSection = new CFGSECTION("@GLOBAL");
				aSectionList.Add( pCurrentSection );
			}
			pValue = new CFGVALUE("<unnamed>");

			switch( token.id )
			{
			case TINTEGER:	pValue->SetInteger(token.v.nval); break;
			case TREAL:		pValue->SetReal(token.v.fval); break;
			case TSTRING:	pValue->SetString(token.v.szval); break;
			}

			pCurrentSection->aValList.Add(pValue);
			FreeToken(&token);
			break;
		case TSYMBOL :
			{
				pValue = new CFGVALUE(token.v.szval);
				FreeToken(&token);

				token = Scan();
				if(token.id != TEQUAL){
					ret = CFG_UNEXPECTED;
					delete pValue;
					goto return_label;
				}

				//INSTALL NEW VALUE
				token = Scan();
				if(token.id != TINTEGER && token.id != TREAL && token.id != TSTRING){
					ret = CFG_UNEXPECTED;
					delete pValue;
					goto return_label;
				}

				if( pCurrentSection == NULL ){	// 만일 NULL이라면...
					pCurrentSection = new CFGSECTION("@GLOBAL");
					aSectionList.Add( pCurrentSection );
				}
				
				//새로운 값을 추가해넣는다. 
				pCurrentSection->aValList.Add(pValue);
				switch( token.id )
				{
				case TINTEGER:	pValue->SetInteger(token.v.nval); break;
				case TREAL:		pValue->SetReal(token.v.fval); break;
				case TSTRING:	pValue->SetString(token.v.szval); break;
				}

				FreeToken(&token);
			}
			break;
		case TSECTION :
			{
				token = Scan();
				if( token.id != TSYMBOL ){
					ret = CFG_UNEXPECTED;
					goto return_label;
				}

				// MAKE NEW SECTION & ADD NEW SECTION TO SECTION LIST
				pCurrentSection = new CFGSECTION(token.v.szval);
				aSectionList.Add(pCurrentSection);
				FreeToken(&token);
			}
			break;
		default :
			ret = CFG_UNEXPECTED;
			goto return_label;
		}
		
		FreeToken(&token);
	}

return_label:
	if( ret != CFG_NOERROR ) m_nErrorLine = m_nLineNum;

	FreeToken(&token);

	if(m_FilePointer){
		fclose(m_FilePointer);
		m_FilePointer = NULL;
	}

	return ret;
}

