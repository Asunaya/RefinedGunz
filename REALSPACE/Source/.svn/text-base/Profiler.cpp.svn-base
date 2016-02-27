#include <stdio.h>
#include "CMError.h"
#include "CMErrorDef.h"
#include "Profiler.h"
#include "Libpak.h"

BOOL Profiler::Load(char* szFileName)
{
	FILE* fp = NULL;
	char strBuf[BUF_SIZE+1];
	m_ProfileList.DeleteAll();
	
	strcpy(m_strFileName, szFileName);
	fp = fopen(m_strFileName, "rt");
	if (fp == NULL) {
//		SetError(CMERR_CANT_OPEN_FILE);
		return FALSE;
	}

	for (;;) {
		if (!ReadLine(fp, strBuf)) break;
		if (!*strBuf) break;

		if(strBuf[0]==';')					continue;
		if(strBuf[0]=='/'&&strBuf[1]=='/')	continue;

		Parse(strBuf);
		memset(strBuf, 0, BUF_SIZE+1);
	}
	fclose(fp);
	m_IdxFinded = -1;
	return TRUE;
}
/*
	char temp[BUF_SIZE+1];

	do {
		if (!fgets(temp, BUF_SIZE, fl)) return FALSE;
		if (*temp) {
			temp[strlen(temp)] = '\0';
			if (temp[strlen(temp)-1] == '\n')
				temp[strlen(temp)-1] = '\0';
		}
	} while (!feof(fl) && (!*temp));

	strcpy(buf, temp);
	return TRUE;
*/
int g_sp;

bool sReadLine(char* str,char* data)
{
	int c = g_sp;

	int len = strlen(str);

	if(c >= len) return false;

	while( str[c] ) {
		if( str[c]	== 0x0d)
			if( str[c+1]== 0x0a)
				break;
		c++;

		if(c == 0)  break;
		if(c > len) break; 
	}

	int size = c - g_sp;

	if(size == 0) {
		data[0] = ';';
		data[1] = 0;
		g_sp += (size + 2);
		return true;
	}

	memcpy(data,(str+g_sp),size);

	g_sp += (size + 2);
	data[size] = 0;

	return true;
}
// ÀÛ¼ºÁß~
BOOL Profiler::LoadPak(char* szPakFileName,char* szFileName)
{
	Package tPak;

	tPak.Open(szPakFileName);

	PakData *pPakData;
	pPakData = tPak.GetPakData( szFileName );

	if(!pPakData) return FALSE;

	int Len = pPakData->GetFileSize();	

	char* pData = new char [Len+1];

	if( !pData ) return FALSE;

	pPakData->ReadFile( pData, Len );

	pData[Len] = 0;
//	FILE* fp = NULL;
	char strBuf[BUF_SIZE+1];
	m_ProfileList.DeleteAll();
	
	strcpy(m_strFileName, szFileName);

//	fp = fopen(m_strFileName, "rt");
//	if (fp == NULL) return FALSE;

	g_sp = 0;

	for (;;) {
		if (!sReadLine(pData, strBuf)) break;
		if (!*strBuf) break;

		if(strBuf[0]==';')					continue;
		if(strBuf[0]=='/'&&strBuf[1]=='/')	continue;

		Parse(strBuf);
		memset(strBuf, 0, BUF_SIZE+1);
	}

//	fclose(fp);

	m_IdxFinded = -1;

	delete pPakData;	
	delete [] pData;

	return TRUE;
}

BOOL Profiler::Save()
{
	FILE* fp = NULL;
	fp = fopen(m_strFileName, "wt");
	if (fp == NULL) {
//		SetError(CMERR_CANT_SAVE_FILE);
		return FALSE;
	}

	for (int i=0; i<m_ProfileList.GetCount(); i++) {
		PROFILENODE* pPROFILENODE = m_ProfileList.Get(i);
		if ((pPROFILENODE->pName==NULL) || (*pPROFILENODE->pName==' ') || (!*pPROFILENODE->pName)) continue;
		if ((pPROFILENODE->pValue==NULL) || (*pPROFILENODE->pValue==' ') || (!*pPROFILENODE->pValue)) continue;
		fprintf(fp, "%s=%s\n", pPROFILENODE->pName, pPROFILENODE->pValue);
	}

	fclose(fp);
	return TRUE;
}

void Profiler::Parse(char* strBuf)
{
	char* pName;
	char* pValue;
	char Line[1024];

	strcpy(Line,strBuf);

	int len = strlen(strBuf);

	pName = strtok(strBuf,"= \t;");
	pValue = strtok(NULL, "= \t;");
	
	if ((pName==NULL) || (*pName==' ') || (!*pName)) return;
	if ((pValue==NULL) || (*pValue==' ') || (!*pValue)) return;

	int arg_start = (pValue - pName);
	int arg_len = len - arg_start;

	PROFILENODE* pPROFILENODE = new PROFILENODE;
	pPROFILENODE->pName = new char[strlen(pName)+1];
	pPROFILENODE->pValue = new char[strlen(pValue)+1];

	pPROFILENODE->pArg = new char[arg_len+1];
		
	strcpy(pPROFILENODE->pName, pName);
	strcpy(pPROFILENODE->pValue, pValue);
	memcpy(pPROFILENODE->pArg,(Line+arg_start), arg_len);
	pPROFILENODE->pArg[arg_len] = 0;

	m_ProfileList.Add(pPROFILENODE);
}

BOOL Profiler::Write(const char* pName, const char* pValue)
{
	if ((pName==NULL) || (*pName==' ') || (!*pName)) return FALSE;
	if ((pValue==NULL) || (*pValue==' ') || (!*pValue)) return FALSE;

	PROFILENODE* pPROFILENODE = NULL;
	BOOL bNameFinded = FALSE;
	for (int i=0; i<m_ProfileList.GetCount(); i++) {
		PROFILENODE* pTmpPROFILENODE = m_ProfileList.Get(i);
		if ((pTmpPROFILENODE->pName==NULL)) continue;
		if (!strcmp(pName, pTmpPROFILENODE->pName)) {
			pPROFILENODE = pTmpPROFILENODE;
			bNameFinded = TRUE;
			break;
		}
	}

	if ((pPROFILENODE == NULL) || (bNameFinded == FALSE)) {
		pPROFILENODE = new PROFILENODE;
		pPROFILENODE->pName = new char[strlen(pName)+1];
		pPROFILENODE->pValue = new char[strlen(pValue)+1];
	}
	
	if (bNameFinded == TRUE) {
		int nNewLen = strlen(pValue) + 1;
		char* pszNewBuffer = new char[nNewLen];
		delete [] pPROFILENODE->pValue;
		pPROFILENODE->pValue = pszNewBuffer;
	}

	if (bNameFinded == FALSE)
		strcpy(pPROFILENODE->pName, pName);
	strcpy(pPROFILENODE->pValue, pValue);

	if (bNameFinded == FALSE)
		m_ProfileList.Add(pPROFILENODE);

	Save();
	m_IdxFinded = -1;
	
	return TRUE;	
}

BOOL Profiler::Read(const char* pName, char* pBuf, int nBufSize)
{
	for (int i=0; i<m_ProfileList.GetCount(); i++) {
		PROFILENODE* pPROFILENODE = m_ProfileList.Get(i);
		if ((pPROFILENODE->pName==NULL) || (*pPROFILENODE->pName==' ') || (!*pPROFILENODE->pName)) continue;
		if ((pPROFILENODE->pValue==NULL) || (*pPROFILENODE->pValue==' ') || (!*pPROFILENODE->pValue)) continue;
		if (!stricmp(pName, pPROFILENODE->pName)) {
			strncpy(pBuf, pPROFILENODE->pValue, nBufSize);
			return TRUE;
		}
	}
	return FALSE;	
}

BOOL Profiler::ReadArg(const char* pName, char* pBuf, int nBufSize)
{
	for (int i=0; i<m_ProfileList.GetCount(); i++) {
		PROFILENODE* pPROFILENODE = m_ProfileList.Get(i);
		if ((pPROFILENODE->pName==NULL) || (*pPROFILENODE->pName==' ') || (!*pPROFILENODE->pName)) continue;
		if ((pPROFILENODE->pValue==NULL) || (*pPROFILENODE->pValue==' ') || (!*pPROFILENODE->pValue)) continue;
		if (!stricmp(pName, pPROFILENODE->pName)) {
			strncpy(pBuf, pPROFILENODE->pArg, nBufSize);
//			memcpy(pBuf,pPROFILENODE->pArg,nBufSize);
			return TRUE;
		}
	}
	return FALSE;	
}

BOOL Profiler::Read(const char* pName, char** ppValue)
{
	for (int i=0; i<m_ProfileList.GetCount(); i++) {
		PROFILENODE* pPROFILENODE = m_ProfileList.Get(i);
		if ((pPROFILENODE->pName==NULL) || (*pPROFILENODE->pName==' ') || (!*pPROFILENODE->pName)) continue;
		if ((pPROFILENODE->pValue==NULL) || (*pPROFILENODE->pValue==' ') || (!*pPROFILENODE->pValue)) continue;
		if (!stricmp(pName, pPROFILENODE->pName)) {
			*ppValue = pPROFILENODE->pValue;
			return TRUE;
		}
	}
	return FALSE;	
}

BOOL Profiler::Delete(const char* pName)
{
	for (int i=0; i<m_ProfileList.GetCount(); i++) {
		PROFILENODE* pPROFILENODE = m_ProfileList.Get(i);
		if ((pPROFILENODE->pName==NULL) || (*pPROFILENODE->pName==' ') || (!*pPROFILENODE->pName)) continue;
		if ((pPROFILENODE->pValue==NULL) || (*pPROFILENODE->pValue==' ') || (!*pPROFILENODE->pValue)) continue;
		if (!strcmp(pName, pPROFILENODE->pName)) {
			m_ProfileList.Delete(i);
			Save();
			m_IdxFinded = -1;
			return TRUE;
		}
	}
	return FALSE;
}

int Profiler::Find(const char* pName, const char* pValue)
{
	for (int i=0; i<m_ProfileList.GetCount(); i++) {
		PROFILENODE* pPROFILENODE = m_ProfileList.Get(i);
		if ((pPROFILENODE->pName==NULL) || (*pPROFILENODE->pName==' ') || (!*pPROFILENODE->pName)) continue;
		if ((pPROFILENODE->pValue==NULL) || (*pPROFILENODE->pValue==' ') || (!*pPROFILENODE->pValue)) continue;
			if (pName != NULL) {
				if (!strcmp(pName, pPROFILENODE->pName)) {
					if (pValue == NULL) {
						m_IdxFinded = i;
						return i;
					} else {
						if (!strcmp(pValue, pPROFILENODE->pValue)) {
							m_IdxFinded = i;
							return i;
						}
					}
				}
			} else {
				if (pValue != NULL)
					if (!strcmp(pValue, pPROFILENODE->pValue)) {
						m_IdxFinded = i;
						return i;
					}
			}
	}
	return -1;
}

int Profiler::FindNext(const char* pName, const char* pValue)
{
	if ((m_IdxFinded == -1) || (m_IdxFinded >= m_ProfileList.GetCount()))
		return -1;
	for (int i=m_IdxFinded+1; i<m_ProfileList.GetCount(); i++) {
		PROFILENODE* pPROFILENODE = m_ProfileList.Get(i);
		if ((pPROFILENODE->pName==NULL) || (*pPROFILENODE->pName==' ') || (!*pPROFILENODE->pName)) continue;
		if ((pPROFILENODE->pValue==NULL) || (*pPROFILENODE->pValue==' ') || (!*pPROFILENODE->pValue)) continue;
			if (pName != NULL) {
				if (!strcmp(pName, pPROFILENODE->pName)) {
					if (pValue == NULL) {
						m_IdxFinded = i;
						return i;
					} else {
						if (!strcmp(pValue, pPROFILENODE->pValue)) {
							m_IdxFinded = i;
							return i;
						}
					}
				}
			} else {
				if (pValue != NULL)
					if (!strcmp(pValue, pPROFILENODE->pValue)) {
						m_IdxFinded = i;
						return i;
					}
			}
	}
	return -1;
}

BOOL ReadLine(FILE* fl, char *buf)
{
	char temp[BUF_SIZE+1];

	do {
		if (!fgets(temp, BUF_SIZE, fl)) return FALSE;
		if (*temp) {
			temp[strlen(temp)] = '\0';
			if (temp[strlen(temp)-1] == '\n')
				temp[strlen(temp)-1] = '\0';
		}
	} while (!feof(fl) && (!*temp));

	strcpy(buf, temp);
	return TRUE;
}

