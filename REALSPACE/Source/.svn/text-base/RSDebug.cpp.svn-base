#include <stdio.h>
#include <windows.h>
#include <crtdbg.h>

#include "fileinfo.h"
#include "RSdebug.h"
#include "rstypes.h"

#define MAX_LOG_HISTORY 10
static char logfilename[256];
static char szLoghistory[MAX_LOG_HISTORY][256];
static int nLogMethod=0,nHead=0,nTail=0;

void initlog(int logmethodflags)
{
	int i;
	nLogMethod=logmethodflags;
	nHead=0;nTail=0;
	for(i=0;i<MAX_LOG_HISTORY;i++)
		szLoghistory[i][0]=0;
	if(nLogMethod&RS_LOGSTYLE_FILE)
	{
		GetFullPath(logfilename,"rslog.txt");
		FILE *pFile;
		pFile=fopen(logfilename,"w");
		if(pFile!=NULL) fclose(pFile);
	}
}

void __cdecl rslog(const char *pFormat,...)
{
	char temp[4096];

	va_list args;

	va_start(args,pFormat);
	vsprintf(temp,pFormat,args);
	va_end(args);

	strncpy(szLoghistory[nTail],temp,255);

	if(nLogMethod&RS_LOGSTYLE_FILE)
	{
		FILE *pFile;
		pFile = fopen( logfilename, "a" );
		if( !pFile ) pFile=fopen(logfilename,"w");
		if( pFile==NULL ) return;
		fprintf(pFile,temp);
		fclose(pFile);
	}
	if(nLogMethod&RS_LOGSTYLE_DEBUGSTRING)
	{
		OutputDebugString(temp);
	}
	nTail=(nTail+1)%MAX_LOG_HISTORY;
	if((nTail==(nHead+1)%MAX_LOG_HISTORY)&&(szLoghistory[nTail][0]))
		nHead=(nHead+1)%MAX_LOG_HISTORY;
}

void logMatrix(rmatrix &m)
{
	char s[200];
	sprintf(s,"%5.5f %5.5f %5.5f \n",m._11,m._12,m._13);rslog(s);
	sprintf(s,"%5.5f %5.5f %5.5f \n",m._21,m._22,m._23);rslog(s);
	sprintf(s,"%5.5f %5.5f %5.5f \n",m._31,m._32,m._33);rslog(s);
	sprintf(s,"%5.5f %5.5f %5.5f \n",m._41,m._42,m._43);rslog(s);
}

void logMatrix(rmatrix44 &m)
{
	char s[200];
	sprintf(s,"%5.5f %5.5f %5.5f %5.5f\n",m._11,m._12,m._13,m._14);rslog(s);
	sprintf(s,"%5.5f %5.5f %5.5f %5.5f\n",m._21,m._22,m._23,m._24);rslog(s);
	sprintf(s,"%5.5f %5.5f %5.5f %5.5f\n",m._31,m._32,m._33,m._34);rslog(s);
	sprintf(s,"%5.5f %5.5f %5.5f %5.5f\n",m._41,m._42,m._43,m._44);rslog(s);
}

void logVector(rvector &v)
{
	char s[200];
	sprintf(s,"%5.5f %5.5f %5.5f\n",v.x,v.y,v.z);rslog(s);
}

void __cdecl Msg( LPSTR fmt, ... )
{
    char buff[256];

    wvsprintf(buff, fmt, (char *)(&fmt+1));
    lstrcat(buff, "\r\n");
    MessageBox( NULL, buff, "RealSpace Message", MB_OK );
	rslog(buff);rslog("\n");
}

char *GetLogHistory(int i)
{
	_ASSERT((i>=0)&&(i<MAX_LOG_HISTORY));
	return szLoghistory[(i+nHead)%MAX_LOG_HISTORY];
}

int GetLogHistoryCount()
{
	return (nHead<nTail)?(nTail-nHead):MAX_LOG_HISTORY;
}