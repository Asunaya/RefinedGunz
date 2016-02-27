#include "MClient.h"
#include "MGameClient.h"
#include "ConsoleCmds.h"
#include "MCommand.h"
#include "MCommandManager.h"

static MConsoleCmdManager	g_ConsoleCmdManager;
extern MConsoleFrame*		g_pConsole;
extern MGameClient			g_Client;


void OutputToConsole(char* str)
{
	g_pConsole->AddLines(str);
}

// 콘솔에서 입력창에 입력했을때의 이벤트를 받는 함수
void ConsoleInputEvent(const char* szInputStr)
{
	char szBuf[1024];

	if (!g_ConsoleCmdManager.DoCommand(szInputStr, szBuf))
	{
		OutputToConsole(szBuf);
	}
}

// 소켓 에러 이벤트
void SocketErrorEvent(void* pCallbackContext, SOCKET sock, SOCKET_ERRER_EVENT ErrorEvent, int &ErrorCode)
{
	char szText[1024];
	sprintf(szText, "소켓에러입니다. ErrorCode: %d", ErrorCode);
	OutputToConsole(szText);
}

// 소켓 접속 이벤트
bool SocketConnectEvent(void* pCallbackContext, SOCKET sock)
{
	char szText[1024];
	sprintf(szText, "접속되었습니다.");
	OutputToConsole(szText);

	return true;
}

bool SocketDisconnectEvent(void* pCallbackContext, SOCKET sock)
{
	char szText[1024];
	sprintf(szText, "접속이 해제되었습니다.");
	OutputToConsole(szText);

	return true;
}

// 콘솔에 쓰일 명령어 초기화
void InitConsoleCmds()
{
	g_pConsole->SetInputCallback(ConsoleInputEvent);

	g_ConsoleCmdManager.AddCmd
	(
		"?",									// 명령어 이름
		HelpCmd,								// 명령어입력시 실행할 함수 포인터
		"[optional commands]",					// usage
		"shows commands, or detailed help on specified commands" // help
	);

	g_ConsoleCmdManager.AddCmd
	(
		"help", 
		HelpCmd,
		"[optional commands]",
		"shows commands, or detailed help on specified commands"
	);

	g_ConsoleCmdManager.AddCmd
	(
		"clear", 
		ClearCmd,
		"[Clear]",
		"콘솔 화면을 지운다."
	);

	g_ConsoleCmdManager.AddCmd
	(
		"@", 
		ClientCmd,
		"[Client]",
		"Client로 직접 Command를 보낸다."
	);

	g_ConsoleCmdManager.AddCmd
		(
		"connect", 
		ConnectCmd,
		"[connect ip port]",
		"해당 ip의 서버에 접속한다. ip, port를 입력하지 않으면 127.0.0.1 6000로 접속한다."
		);
	g_ConsoleCmdManager.AddCmd
		(
		"con", 
		ConnectCmd,
		"[con]",
		"해당 ip의 서버에 접속한다. ip, port를 입력하지 않으면 127.0.0.1 6000로 접속한다."
		);

	g_ConsoleCmdManager.AddCmd
		(
		"disconnect", 
		DisconnectCmd,
		"[disconnect]",
		"서버와의 연결을 끊는다."
		);
	g_ConsoleCmdManager.AddCmd
		(
		"dis", 
		DisconnectCmd,
		"[dis]",
		"서버와의 연결을 끊는다."
		);

	g_ConsoleCmdManager.AddCmd
		(
		"send",
		SendCmd,
		"[send data]",
		"서버에 Custom Data 전송"
		);

}

void HelpCmd(const int argc, char **const argv)
{
	char szBuf[1024];
	char szBuf2[1024];

	bool bShowAll = false;
	int nColumns = 40;

	if (argc == 2) 
	{
		bShowAll = (stricmp(argv[1], "showall") == 0);
	}

	if ((argc == 1) || (bShowAll))
	{
		OutputToConsole("'help <command>' gives a brief description of <command>\n");

		int nCol = 0;
		memset(szBuf, 0, sizeof(szBuf));

		for (MConsoleCmdItor itor = g_ConsoleCmdManager.GetCmdMap()->begin(); 
			          itor != g_ConsoleCmdManager.GetCmdMap()->end(); ++itor)
		{
			string sName = (*itor).first;

			sprintf(szBuf2, "%16s", sName.c_str());
			strcat(szBuf, szBuf2);
			
			nCol++;
			if (nCol > 2)
			{
				nCol = 0;
				OutputToConsole(szBuf);
				memset(szBuf, 0, sizeof(szBuf));
			}
		}
		if (nCol > 0) OutputToConsole(szBuf);

		return;
	}

	for (int i = 1; i < argc; i++)
	{
		MConsoleCmd* pCmd = g_ConsoleCmdManager.GetCmdByName(argv[i]);
		if (pCmd != NULL)
		{
			sprintf(szBuf, "help: %s\nusage: %s\n", pCmd->GetHelp(), pCmd->GetUsage());
			OutputToConsole(szBuf);
		}
	}
}

void ClearCmd(const int argc, char **const argv)
{
	g_pConsole->ClearLines();
}

void ClientCmd(const int argc, char **const argv)
{
	char szBuf[65535];

	strcpy(szBuf, "");
	for (int i = 1; i < argc; i++)
	{
		strcat(szBuf, argv[i]);
		if (i != argc-1) strcat(szBuf, " ");
	}

	char szErr[1024];
	if (!g_Client.Post(szErr, 1024, szBuf)) OutputToConsole("Post Error");
}


// 접속
void ConnectCmd(const int argc, char **const argv)
{
	if (g_Client.GetClientSocket()->IsActive())
	{
		OutputToConsole("이미 접속되어 있습니다.");
		return;
	}

	if (argc < 3)
	{
		// Test 목적으로 argv가 없으면 127.0.0.1, 6000 port 로 접속한다.
		g_Client.Connect("127.0.0.1", 6000);
	}
	else if (argc == 3)
	{
		g_Client.Connect(argv[1], atoi(argv[2]));
	}
	else
	{
		OutputToConsole("인자가 틀렸습니다.");
	}
	
}

// 접속해제
void DisconnectCmd(const int argc, char **const argv)
{
	if (!g_Client.GetClientSocket()->IsActive())
	{
		OutputToConsole("접속되어 있지 않습니다.");
		return;
	}
	g_Client.GetClientSocket()->Disconnect();
}


void SendCmd(const int argc, char **const argv)
{
	if (argc < 2) 
	{
		OutputToConsole("인자가 부족합니다.");
		return;
	}

	if (!g_Client.GetClientSocket()->IsActive())
	{
		OutputToConsole("접속되어 있지 않습니다.");
		return;
	}

	char szBuf[256];
	strcpy(szBuf, argv[1]);

	g_Client.GetClientSocket()->Send(szBuf, DWORD(strlen(szBuf) + 1));
}
