#include "stdafx.h"
#include "CMLexicalAnalyzer.h"
#include <string.h>
#include <stdlib.h>

CMLexicalAnalyzer::CMLexicalAnalyzer(void)
{
	//m_szOriginal[0] = 0;
}

CMLexicalAnalyzer::~CMLexicalAnalyzer(void)
{
	Destroy();
}

bool StrTok(char *pToken, const char *pStr, char *pSep)
{
	static char szTemp[256] = "";

	if(pStr!=NULL){
		strcpy(szTemp, pStr);
	}

	int nLen = strlen(szTemp);
	if(nLen==0) return false;

	int nSepLen = strlen(pSep);
	for(int i=0; i<nLen; i++){
		for(int j=0; j<nSepLen; j++){
			if(szTemp[i]==pSep[j]){
				if(i==0){
					pToken[0] = szTemp[i];
					pToken[1] = 0;
					strcpy(szTemp, szTemp+1);
				}
				else{
					memcpy(pToken, szTemp, i);
					pToken[i] = 0;
					strcpy(szTemp, szTemp+i);
				}
				return true;
			}
		}
	}

	//memcpy(pToken, szTemp, i);
	strcpy(pToken, szTemp);
	szTemp[0] = 0;
	return true;
}

bool CMLexicalAnalyzer::Create(const char *pStr)
{
	//strcpy(m_szOriginal, pStr);
	char seps[] = " ,\t'";	// ','는 임시로 빼 놓는다. ''로 스트링을 구분하기 위해
	char szToken[256];
	if(StrTok(szToken, pStr, seps)==false) return true;
	while(1){
		if(szToken[0]==' '){
		}
		else if(szToken[0]==','){
		}
		else if(szToken[0]=='\t'){
		}
		else if(szToken[0]=='\''){
			if(StrTok(szToken, NULL, "'")==false) return true;
			char *pAddToken = new char[strlen(szToken)+1];
			strcpy(pAddToken, szToken);
			m_Tokens.Add(pAddToken);
			if(StrTok(szToken, NULL, "'")==false) return true;
		}
		else{
			char *pAddToken = new char[strlen(szToken)+1];
			strcpy(pAddToken, szToken);
			m_Tokens.Add(pAddToken);
		}

		if(StrTok(szToken, NULL, seps)==false) return true;
	}
	/*
	char *pToken = strtok(pStr, seps);
	char szTemp[256];
	while(pToken!=NULL){
		if(pToken[0]=='\''){
			strcpy(szTemp, pToken);
			pToken = strtok(NULL, "'");
			strcpy(szTemp+strlen(szTemp), pToken);
			szTemp[strlen(szTemp)+1] = 0;
			szTemp[strlen(szTemp)] = '\'';
			pToken = szTemp;
		}
		char *pAddToken = new char[strlen(pToken)+1];
		strcpy(pAddToken, pToken);
		m_Tokens.Add(pAddToken);

		pToken = strtok(NULL, seps);
	}
	*/

	return true;
}

void CMLexicalAnalyzer::Destroy(void)
{
	for(int i=0; i<m_Tokens.GetCount(); i++){
		char *pToken = m_Tokens.Get(i);
		delete[] pToken;
	}
	m_Tokens.DeleteAll();
}

char *CMLexicalAnalyzer::GetByStr(int i)
{
	return m_Tokens.Get(i);
}

int CMLexicalAnalyzer::GetByInt(int i)
{
	return atoi(GetByStr(i));
}

long CMLexicalAnalyzer::GetByLong(int i)
{
	return atol(GetByStr(i));
}

float CMLexicalAnalyzer::GetByFloat(int i)
{
	return (float)atof(GetByStr(i));
}

int CMLexicalAnalyzer::GetCount(void)
{
	return m_Tokens.GetCount();
}

bool CMLexicalAnalyzer::IsNumber(int i)
{
	char *pStr = GetByStr(i);
	int len = strlen(pStr);
	for(int j=0; j<len; j++){
		if(!(pStr[j]>='0' && pStr[j]<'9') || pStr[j]=='.')
			return false;
	}

	return true;
}
/*
char *CMLexicalAnalyzer::GetOrgStr(void)
{
	return m_pOriginal;
}
*/