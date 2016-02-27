/******************************************************************
   
   Log.cpp

     Corperation : MAIET entertainment
     Programmer  : Lim Dong Hwan
	 Date        : 22.June.2005

*******************************************************************/


#include "Log.h"
#include "stdafx.h"
#include <sys/timeb.h>


// Define variables
FILE*				m_pFile;						// Pointer of the file
char				m_szFileName[ 128];				// File name
bool				m_bOpenLogFile;					// Flag of open file
LOG_TYPE			m_nFileType;					// File type of Log
CRITICAL_SECTION	m_csLog;						// Critical section



// OpenLog
bool OpenLog( const char* pszFileName, const char* pszTitle, LOG_TYPE nFileType)
{
	// Check already open file
	if ( m_bOpenLogFile)
		return false;


#ifdef LOG_ENABLE

	// Check output log
	if ( nFileType == LOGTYPE_OUTPUT)
	{
		m_bOpenLogFile = true;

		return true;
	}

	// Set file name
	strcpy( m_szFileName, pszFileName);
	if ( nFileType == LOGTYPE_TEXT)
		strcat( m_szFileName, ".txt");
	else if ( nFileType == LOGTYPE_HTML)
		strcat( m_szFileName, ".html");
	else if ( nFileType == LOGTYPE_EXEL)
		strcat( m_szFileName, ".exl");
	else
		return false;

	m_nFileType = nFileType;


	// Open file
	m_pFile = fopen( _T( m_szFileName), "w");
	if ( m_pFile == NULL)
		return false;


	// Text file initialize
	if ( m_nFileType == LOGTYPE_TEXT)
	{
		fprintf( m_pFile, "[   Time   ]   [                     Log Message                     ]\n");
	}

	// HTML file initialize
	else if ( m_nFileType == LOGTYPE_HTML)
	{
		fprintf( m_pFile, "<HTML>\n");
		fprintf( m_pFile, "<title>%s</title>\n", pszTitle);
		fprintf( m_pFile, "<STYLE TYPE=\"text/css\">\n");
		fprintf( m_pFile, "<!--\n");
		fprintf( m_pFile, "\t.tit { font-size:15px; font-family:%s; color:black; font-weight:bold}\n", LOG_DEFFONT);
		fprintf( m_pFile, "\t.hig { font-size:12px; font-family:%s; color:blue}\n", LOG_DEFFONT);
		fprintf( m_pFile, "\t.nor { font-size:12px; font-family:%s; color:black}\n", LOG_DEFFONT);
		fprintf( m_pFile, "\t.war { font-size:12px; font-family:%s; color:orange; font-weight:bold}\n", LOG_DEFFONT);
		fprintf( m_pFile, "\t.err { font-size:12px; font-family:%s; color:red; font-weight:bold}\n", LOG_DEFFONT);
		fprintf( m_pFile, "-->\n");
		fprintf( m_pFile, "</STYLE>\n");
		fprintf( m_pFile, "<BODY BGCOLOR=\"white\">\n");
		fprintf( m_pFile, "<P CLASS=\"nor\" ALIGN=\"right\">Press F5 to refresh</P>\n");
		fprintf( m_pFile, "<TABLE WIDTH=100%% BORDER=1 CELLPADDING=2 CELLSPACING=0>\n");
		fprintf( m_pFile, "\t<TR>\n");
		fprintf( m_pFile, "\t\t<TD WIDTH=\"90\" BGCOLOR=#E0E0E0><P CLASS=\"tit\" ALIGN=\"center\">Time</P></TD>\n");
		fprintf( m_pFile, "\t\t<TD BGCOLOR=#E0E0E0><P CLASS=\"tit\" ALIGN=\"center\">Log Message</P></TD>\n");
		fprintf( m_pFile, "\t</TR>\n");
		fprintf( m_pFile, "</TABLE>\n</BODY>\n</HTML>");


		// Open HTML file
		if ( LOG_SHOWFILE)
		{
			char szOpenHtml[ 128];
			sprintf( szOpenHtml, "explorer %s", m_szFileName);
			WinExec( szOpenHtml, SW_SHOW);
		}
	}

	// EXEL file initialize
	else if ( m_nFileType == LOGTYPE_EXEL)
	{
	}

	// Create critical section
	InitializeCriticalSection( &m_csLog);


#endif

	// Set variables
	m_bOpenLogFile = true;


	// Get UNIX time and date, and write
	__time64_t ltime;
	_time64( &ltime);
	char szMsg[ 125];
	sprintf( szMsg, "Log start : %s ", _ctime64( &ltime));
	szMsg[ (int)strlen( szMsg) - 2] = 0;
	PutLog( szMsg, LOG_NORMAL);


	char szDir[ 512];
	GetCurrentDirectory( sizeof( szDir), szDir);
	sprintf( szMsg, "Current Directory : %s", szDir);
	PutLog( szMsg);


	return true;
}


// Close
bool CloseLog( void)
{
	// Check already close file
	if ( !m_bOpenLogFile)
		return false;


#ifdef LOG_ENABLE

	bool bRetVal = false;

	// Close output log
	if ( m_nFileType == LOGTYPE_OUTPUT)
		bRetVal = true;

	// Close file
	else if ( (m_nFileType == LOGTYPE_TEXT) || (m_nFileType == LOGTYPE_HTML) || (m_nFileType == LOGTYPE_EXEL))
	{
		// Delete critical section
		DeleteCriticalSection( &m_csLog);


		// Close file
		if ( fclose( m_pFile) == 0)
			bRetVal = true;


		// Open text file
		if ( (m_nFileType == LOGTYPE_TEXT) && LOG_SHOWFILE)
		{
			char szOpenText[ 128];
			sprintf( szOpenText, "notepad %s", m_szFileName);
			WinExec( szOpenText, SW_SHOW);
		}
	}


	// Set variables
	m_bOpenLogFile = false;

	return bRetVal;

#else

	// Set variables
	m_bOpenLogFile = false;

	return true;

#endif
}


// PutLog
bool PutLog( const char* pszLog)
{
	return PutLog( pszLog, LOG_NORMAL);
}

bool PutLogH( const char* pszLog)
{
	return PutLog( pszLog, LOG_HIGHLIGHT);
}

bool PutLogW( const char* pszLog)
{
	return PutLog( pszLog, LOG_WARNNING);
}

bool PutLogE( const char* pszLog)
{
	return PutLog( pszLog, LOG_ERROR);
}

bool PutLog( const char* pszLog, LOG_MSG nLogType)
{
	// Check already close file
	if ( !m_bOpenLogFile)
		return false;


#ifdef LOG_ENABLE

	// Enter critical section
	EnterCriticalSection( &m_csLog);


	// Text
	if ( m_nFileType == LOGTYPE_OUTPUT)
	{
		OutputDebugString( pszLog);
		OutputDebugString( "\n");

		return true;
	}


	// Get time
	char szBuff[128];
	_strtime( szBuff);
	struct __timeb64 tstruct;
	_ftime64( &tstruct);
	char szTime[128];
	sprintf( szTime, "%s:%03d", szBuff, tstruct.millitm);


	// Text file
	if ( m_nFileType == LOGTYPE_TEXT)
		fprintf( m_pFile, "%s : %s\n", szTime, pszLog);

	// HTML file
	else if ( m_nFileType == LOGTYPE_HTML)
	{
		// Get font type
		char szFont[ 25];
		strcpy( szFont, "\"nor\"");
		if ( nLogType == LOG_HIGHLIGHT)
			strcpy( szFont, "\"hig\"");
		else if ( nLogType == LOG_WARNNING)
			strcpy( szFont, "\"war\"");
		else if ( nLogType == LOG_ERROR)
			strcpy( szFont, "\"err\"");


		// Write log message
		fseek( m_pFile, -((int)strlen( "</TABLE>\n</BODY>\n</HTML>") + 2), SEEK_CUR);
		fprintf( m_pFile, "\t<TR><TD><P CLASS=%s>%s</P></TD> <TD><P CLASS=%s>%s</P></TD></TR>\n", szFont, szTime, szFont, pszLog);
		fprintf( m_pFile, "</TABLE>\n</BODY>\n</HTML>");
	}
    
	// EXEL file
	else if ( m_nFileType == LOGTYPE_EXEL)
	{
	}


	// Leave critical section
	LeaveCriticalSection( &m_csLog);

#endif

	return true;
}
