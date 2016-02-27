/*#include "MConsoleCmd.h"

MConsoleCmd::MConsoleCmd(const char* szName, int nMinArgs = ARGVNoMin, int nMaxArgs=ARGVNoMax,
			const char* szUsage=NULL, const char* szHelp=NULL)
{
	m_fnProc = NULL;

	m_nMinArgs = ARGVNoMin;
	m_nMaxArgs = ARGVNoMax;

	strcpy(m_szName, szName);
	strcpy(m_szUsage, szUsage);
	strcpy(m_szHelp, szHelp);
}

void MConsoleCmd::OnProc(const int argc, char **const argv)
{
	if (m_fnProc != NULL)
	{
		(*m_fnProc)(argc, argv);
	}
}



bool MConsoleCmdManager::MakeArgv(char* szLine, MConsoleArgvInfo* pAI)
{
	int c;
	char* scp;
	char* dcp;
	char* dlim;
	char* arg;

	int nArgcMax = (sizeof(pAI->cargv) / sizeof(char*));

	scp = szLine;
	dcp = pAI->argbuf;
	dlim = dcp + sizeof(pAI->argbuf) - 1;

	for (pAI->cargc = 0; pAI->cargc < nArgcMax; )
	{
		for ( ; ; scp++)
		{
			c = *scp;
			if (isspace(c)) continue;

			if ( (c == '\0') || (c == ';') || (c == '\n') )
			{
				pAI->cargv[pAI->cargc] = NULL;
				return true;
			}
			break;
		}
		arg = dcp;
		pAI->cargv[pAI->cargc] = arg;
		(pAI->cargc)++;

		for ( ; ; )
		{
			c = *scp;
			if ( (c == '\0') || (isspace(c)) || (c == ';') || (c == '\n')) break;
			scp++;
			if (dcp >= dlim) return false;
			*dcp++ = c;
		}

		*dcp++ = '\0';
	}
	
	return false;
}


void MConsoleCmdManager::AddCommand(const char* szName, MConsoleCmdProc* fnProc,
				int nMinArgs=ARGVNoMin, int nMaxArgs=ARGVNoMax,
				const char* szUsage=NULL, const char* szHelp=NULL)
{
	MConsoleCmd* pCmd = new MConsoleCmd(szName, nMinArgs, nMaxArgs, szUsage, szName);
	pCmd->SetProc(fnProc);
	m_CmdMap.insert(MConsoleCmdMap::value_type(szName, pCmd));
}
void MConsoleCmdManager::DoCommand(char* szLine)
{
	MConsoleArgvInfo	ai;
	memset(&ai, 0, sizeof(MConsoleArgvInfo));

	if (MakeArgv(szLine, &ai))
	{
		if (ai.cargc > 0)
		{
			MConsoleCmd* pCmd;
			pCmd = GetCommandByName(ai.cargv[0]);
			if (pCmd != NULL)
			{
				pCmd->OnProc(ai.cargc, ai.cargv);
			}
		}
	}

}
*/