#include "stdafx.h"
#include "ZReportAbuse.h"
#include "ZGameInterface.h"
#include "ZGameClient.h"
#include "ZChat.h"
#include "MDrawContext.h"
#include "ZNetRepository.h"

#include "ZApplication.h"
#include "ZLanguageConf.h"
#include "ZConfiguration.h"


#define REPORT_ABUSE_ACTIVATE_TIME		(1000 * 60 * 3)		// 이전 3분만 저장한다.
#define REPORT_ABUSE_COOL_TIME			(1000 * 60)			// 신고하고 1분후 신고할 수 있음
#define REPORT_ABUSE_FILENAME			"119.txt"


ZReportAbuse::ZReportAbuse()
{
	m_szReason[0] = 0;
	m_nReportTime=0;
	m_nChatTop = 0;
	for (int i = 0; i < REPORT_MAX_HISTORY; i++)
	{
		m_ChatHistory[i].timestamp = 0;
	}
}

ZReportAbuse::~ZReportAbuse()
{

}

void ZReportAbuse::Report(const char* szReason)
{
	unsigned long int nNowTime = timeGetTime();
	if ((nNowTime - m_nReportTime) < REPORT_ABUSE_COOL_TIME)
	{
		ZChatOutput( ZMsg(MSG_119_REPORT_WAIT_ONEMINUTE) );
		return;
	}
	
	strcpy(m_szReason, szReason);
	
	SaveFile();
	SendFile();
	m_nReportTime = nNowTime;

	ZChatOutput( ZMsg(MSG_119_REPORT_OK) );
}

void ZReportAbuse::OutputString(const char* szStr)
{
	char *pPureText = MDrawContext::GetPureText(szStr);
	char temp[512];
	strcpy(temp, pPureText);
	free(pPureText);

	m_ChatHistory[m_nChatTop].str = temp;
	m_ChatHistory[m_nChatTop].timestamp = timeGetTime();

	m_nChatTop++;
	if (m_nChatTop >= REPORT_MAX_HISTORY) m_nChatTop = 0;
}

void ZReportAbuse::SendFile()
{
	time_t currtime;
	time(&currtime);
	struct tm* pTM = localtime(&currtime);

	char szPlayer[128];
	wsprintf(szPlayer, "%s", ZGetMyInfo()->GetCharName());

	char szFileName[_MAX_DIR];
	wsprintf(szFileName, "%02d%02d_%02d%02d_%s_%s",
			pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, szPlayer, "119.txt");


	// BAReport 실행
	char szCmd[4048];
	char szRemoteFileName[_MAX_DIR];
	wsprintf(szRemoteFileName, "%s/%s/%s", ZGetConfiguration()->GetBAReportDir(), "gunz119", szFileName);

	wsprintf(szCmd, "app=%s;addr=%s;port=21;id=ftp;passwd=ftp@;user=%s;localfile=%s;remotefile=%s;srcdelete=1;agree=1",
		"gunz", ZGetConfiguration()->GetBAReportAddr(), szPlayer, REPORT_ABUSE_FILENAME, szRemoteFileName);
	
	ShellExecute(g_hWnd, NULL, "BAReport.exe", szCmd, NULL, SW_HIDE);

}


void ZReportAbuse::SaveFile()
{
	unsigned long int nNowTime = timeGetTime();

	FILE* fp = fopen(REPORT_ABUSE_FILENAME, "wt");
	if (fp == NULL) return;

	fprintf(fp, "%s\n", m_szReason);

	for (int i = m_nChatTop; i < REPORT_MAX_HISTORY; i++)
	{
		if ((m_ChatHistory[i].timestamp != 0) && 
			((nNowTime - m_ChatHistory[i].timestamp) < REPORT_ABUSE_ACTIVATE_TIME))
		{
			fprintf(fp, "%s\n", m_ChatHistory[i].str.c_str());
		}
	}
	for (int i = 0; i < m_nChatTop; i++)
	{
		if ((m_ChatHistory[i].timestamp != 0) && 
			((nNowTime - m_ChatHistory[i].timestamp) < REPORT_ABUSE_ACTIVATE_TIME))
		{
			fprintf(fp, "%s\n", m_ChatHistory[i].str.c_str());
		}
	}

	fclose(fp);	
}

