#include "stdafx.h"

#include "ZChat.h"
#include "MChattingFilter.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "ZPost.h"
#include "ZCombatChat.h"
#include "ZCombatInterface.h"
#include "ZIDLResource.h"
#include "MListBox.h"
#include "MLex.h"
#include "MTextArea.h"
#include "ZMyInfo.h"

#include "ZChat_CmdID.h"

#define ZCHAT_CHAT_DELAY				1000		
#define ZCHAT_CHAT_ABUSE_COOLTIME		(1000 * 60)		// 1분
#define ZCHAT_CLEAR_DELAY				(1000 * 10)		// 반복 인식 제거 시간..10초


void ZChatOutput(const char* szMsg, ZChat::ZCHAT_MSG_TYPE msgtype, ZChat::ZCHAT_LOC loc,MCOLOR _color)
{
	ZGetGameInterface()->GetChat()->Output(szMsg, msgtype, loc , _color);
}

void ZChatOutput(MCOLOR color, const char* szMsg, ZChat::ZCHAT_LOC loc)
{
	ZGetGameInterface()->GetChat()->Output(color, szMsg, loc);
}


ZChat::ZChat()
{
	m_nLastInputTime = 0;
	m_nSameMsgCount = 0;
	m_nLastInputMsg[0] = 0;
	m_nLastAbuseTime = 0;
	m_nAbuseCounter = 0;
	m_szWhisperLastSender[0] = 0;

	InitCmds();
}

ZChat::~ZChat()
{

}

bool ZChat::CheckRepeatInput(char* szMsg)
{
#ifndef _PUBLISH
	return true;
#endif

	if (!stricmp(m_nLastInputMsg, szMsg)) m_nSameMsgCount++;
	else m_nSameMsgCount = 0;

	DWORD this_time = timeGetTime();

	if(this_time-m_nLastInputTime > ZCHAT_CLEAR_DELAY) {//10초가 지나면 같은 메시지라고 인정안함..
		m_nSameMsgCount = 0;
	}

	m_nLastInputTime = this_time;

	strcpy(m_nLastInputMsg, szMsg);

	if (m_nSameMsgCount >= 2)
	{
		Output( ZErrStr(MERR_CANNOT_INPUT_SAME_CHAT_MSG), 
			CMT_SYSTEM);
		return false;
	}

	return true;
}

bool ZChat::Input(char* szMsg)
{
	GunzState state = ZApplication::GetGameInterface()->GetState();

#ifdef _PUBLISH
	if ((timeGetTime() - m_nLastInputTime) < ZCHAT_CHAT_DELAY)
	{
		ZGetSoundEngine()->PlaySound("if_error");
		return false;
	}
#endif
	
	// 커맨드 명령어 처리 //////////////////////
	bool bMsgIsCmd = false;
	if (szMsg[0] == '/')
	{
		if (strlen(szMsg) >= 2)
		{
			if (((szMsg[1] > 0) && (isspace(szMsg[1]))) == false)
			{
				ZChatCmdFlag nCurrFlag = CCF_NONE;

				switch (state)
				{
					case GUNZ_LOBBY: nCurrFlag = CCF_LOBBY; break;
					case GUNZ_STAGE: nCurrFlag = CCF_STAGE; break;
					case GUNZ_GAME: nCurrFlag = CCF_GAME; break;
				}

				int nCmdInputFlag = ZChatCmdManager::CIF_NORMAL;

				// 관리자인지 판별
				if ((ZGetMyInfo()->GetUGradeID() == MMUG_ADMIN) || 
					(ZGetMyInfo()->GetUGradeID() == MMUG_DEVELOPER) ||
					(ZGetMyInfo()->GetUGradeID() == MMUG_EVENTMASTER))
				{
					nCmdInputFlag |= ZChatCmdManager::CIF_ADMIN;
				}
				// 테스터인지 판별 - test서버이고 launchdevelop모드일 경우에는 테스터
				if ((ZIsLaunchDevelop()) && 
					((ZGetGameClient()->GetServerMode() == MSM_TEST) || (!ZGetGameClient()->IsConnected())) )
				{
					nCmdInputFlag |= ZChatCmdManager::CIF_TESTER;
				}

				bool bRepeatEnabled = m_CmdManager.IsRepeatEnabled(&szMsg[1]);
				if (!bRepeatEnabled)
				{
					if (!CheckRepeatInput(szMsg)) return false;
				}

				if (m_CmdManager.DoCommand(&szMsg[1], nCurrFlag, ZChatCmdManager::CmdInputFlag(nCmdInputFlag)))
				{
					return true;
				}
				else
				{
					bMsgIsCmd = false;

//					return true;			// 채팅창에 표시 안되게 한다.
				}
			}
		}
	}

	
	if (!bMsgIsCmd)
	{
		if (!CheckRepeatInput(szMsg)) return false;
	}



	if (ZGetMyInfo()->GetUGradeID() == MMUG_CHAT_LIMITED)
	{
		ZChatOutput( ZMsg(MSG_CANNOT_CHAT) );
		return false;
	}

	// 욕방지 //////////////////////////////////
	if (!CheckChatFilter(szMsg)) return false;

	// 편의 커맨드 명령어 //////////////////////
	bool bTeamChat = false;
	if (szMsg[0] == '!') {	// Team Chat
		bTeamChat = true;
	} else if (szMsg[0] == '@') {	// ChatRoom
		ZPostChatRoomChat(&szMsg[1]);
		return true;
	} else if (szMsg[0] == '#') {	// Clan Chat
		ZPostClanMsg(ZGetGameClient()->GetPlayerUID(), &szMsg[1]);
		return true;
	}

	switch (state)
	{
	case GUNZ_GAME:
		{
			ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
			int nTeam = MMT_ALL;
			if (pCombatInterface->IsTeamChat() || bTeamChat)
				nTeam = ZApplication::GetGame()->m_pMyCharacter->GetTeamID();
			ZPostPeerChat(szMsg, nTeam);
		}
		break;
	case GUNZ_LOBBY:
		{
			ZPostChannelChat(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetChannelUID(), szMsg);
		}
		break;
	case GUNZ_STAGE:
		{
			ZPostStageChat(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), szMsg);
		}
		break;
	}

	return true;
}

void ZChat::Output(const char* szMsg, ZCHAT_MSG_TYPE msgtype, ZCHAT_LOC loc,MCOLOR _color)
{
	GunzState state = ZApplication::GetGameInterface()->GetState();

	char szOutput[512];

	if (strlen(szMsg) < sizeof(szOutput)-2) strcpy(szOutput, szMsg);
	else {
		_ASSERT(0);	// 채팅 버퍼가 너무 작음
		char temp[32];strncpy(temp,szMsg,30);temp[30]=0;temp[31]=0;
		mlog("warning : chat buffer overflow : %s\n",temp);
		strncpy(szOutput, szMsg, sizeof(szOutput)-2);
		szOutput[sizeof(szOutput)-1]=0;
		szOutput[sizeof(szOutput)-2]=0;
	}

	if (msgtype == CMT_SYSTEM)
	{
		MCOLOR color = MCOLOR(ZCOLOR_CHAT_SYSTEM_GAME);

		if (((loc == CL_CURRENT) && (state==GUNZ_GAME)) || (loc==CL_GAME))
		{
			color = MCOLOR(ZCOLOR_CHAT_SYSTEM_GAME);
		}
		else if (((loc == CL_CURRENT) && (state==GUNZ_LOBBY)) || (loc==CL_LOBBY))
		{
			color = MCOLOR(ZCOLOR_CHAT_SYSTEM_LOBBY);
		}
		else if (((loc == CL_CURRENT) && (state==GUNZ_STAGE)) || (loc==CL_STAGE))
		{
			color = MCOLOR(ZCOLOR_CHAT_SYSTEM_STAGE);
		}

		Output(color, szOutput, loc);
		return;
	}
	else if (msgtype == CMT_BROADCAST)
	{
		MCOLOR color = MCOLOR(ZCOLOR_CHAT_BROADCAST_GAME);

		if (((loc == CL_CURRENT) && (state==GUNZ_GAME)) || (loc==CL_GAME))
		{
			color = MCOLOR(ZCOLOR_CHAT_BROADCAST_GAME);
		}
		else if (((loc == CL_CURRENT) && (state==GUNZ_LOBBY)) || (loc==CL_LOBBY))
		{
			color = MCOLOR(ZCOLOR_CHAT_BROADCAST_LOBBY);
		}
		else if (((loc == CL_CURRENT) && (state==GUNZ_STAGE)) || (loc==CL_STAGE))
		{
			color = MCOLOR(ZCOLOR_CHAT_BROADCAST_STAGE);
		}

		Output(color, szOutput, loc);
		return;
	}
	else if ( msgtype == CMT_NORMAL)
	{
		if (((loc == CL_CURRENT) && (state==GUNZ_GAME)) || (loc==CL_GAME))
		{
			ZApplication::GetGameInterface()->GetCombatInterface()->OutputChatMsg(szOutput);
		}
		else if (((loc == CL_CURRENT) && (state==GUNZ_LOBBY)) || (loc==CL_LOBBY))
		{
			if(_color.GetARGB() == ZCOLOR_CHAT_SYSTEM )// 기본색인 경우 로비의 기본색
				LobbyChatOutput(szOutput);
			else 
				LobbyChatOutput(szOutput,_color);	// 등록한색..
		}
		else if (((loc == CL_CURRENT) && (state==GUNZ_STAGE)) || (loc==CL_STAGE))
		{
			if(_color.GetARGB() == ZCOLOR_CHAT_SYSTEM )// 기본색인 경우 스테이지의 기본색
				StageChatOutput(szOutput);
			else 
				StageChatOutput(szOutput,_color);	// 등록한색..
		}

		m_ReportAbuse.OutputString(szMsg);
	}
}

void ZChat::Output(MCOLOR color, const char* szMsg, ZCHAT_LOC loc)
{
	m_ReportAbuse.OutputString(szMsg);

	GunzState state = ZApplication::GetGameInterface()->GetState();

	char szOutput[512];
	if (strlen(szMsg) < sizeof(szOutput)-2) strcpy(szOutput, szMsg);
	else {
		_ASSERT(0);	// 채팅 버퍼가 너무 작음
		char temp[32];strncpy(temp,szMsg,30);temp[30]=0;temp[31]=0;
		mlog("warning : chat buffer overflow : %s\n",temp);
		strncpy(szOutput, szMsg, sizeof(szOutput)-2);
		szOutput[sizeof(szOutput)-1]=0;
		szOutput[sizeof(szOutput)-2]=0;
	}

	if (((loc == CL_CURRENT) && (state==GUNZ_GAME)) || (loc==CL_GAME))
	{
		ZCombatInterface* pCombat = ZApplication::GetGameInterface()->GetCombatInterface();
		if (pCombat)
			pCombat->OutputChatMsg(color, szOutput);
	}
	else if (((loc == CL_CURRENT) && (state==GUNZ_LOBBY)) || (loc==CL_LOBBY))
	{
		LobbyChatOutput( szOutput , color );
	}
	else if (((loc == CL_CURRENT) && (state==GUNZ_STAGE)) || (loc==CL_STAGE))
	{
		StageChatOutput( szOutput , color );
	}
}

void ZChat::Clear(ZCHAT_LOC loc)
{
	GunzState state = ZApplication::GetGameInterface()->GetState();

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if (((loc == CL_CURRENT) && (state==GUNZ_GAME)) || (loc==CL_GAME))
	{
	}
	else if (((loc == CL_CURRENT) && (state==GUNZ_LOBBY)) || (loc==CL_LOBBY))
	{
		MTextArea* pWidget = (MTextArea*)pResource->FindWidget("ChannelChattingOutput");
		if (pWidget != NULL) pWidget->Clear();
	}
	else if (((loc == CL_CURRENT) && (state==GUNZ_STAGE)) || (loc==CL_STAGE))
	{
		MTextArea* pWidget = (MTextArea*)pResource->FindWidget("StageChattingOutput");
		if (pWidget != NULL) pWidget->Clear();
	}
}

#define MAX_CHAT_LINES	100

void ZChat::LobbyChatOutput(const char* szChat,MCOLOR color)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MTextArea* pWidget = (MTextArea*)pResource->FindWidget("ChannelChattingOutput");
	if(!pWidget) return;

	pWidget->AddText(szChat,color);
	while(pWidget->GetLineCount()>MAX_CHAT_LINES)
		pWidget->DeleteFirstLine();

}

void ZChat::StageChatOutput(const char* szChat,MCOLOR color)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MTextArea* pWidget = (MTextArea*)pResource->FindWidget("StageChattingOutput");
	if(!pWidget) return;

	pWidget->AddText(szChat,color);
	while(pWidget->GetLineCount()>MAX_CHAT_LINES)
		pWidget->DeleteFirstLine();

}

void ZChat::Report112(const char* szReason)
{
	m_ReportAbuse.Report(szReason);
}

bool ZChat::CheckChatFilter(const char* szMsg)
{
	if (m_nLastAbuseTime > 0) 
	{
		if ((timeGetTime() - m_nLastAbuseTime) < ZCHAT_CHAT_ABUSE_COOLTIME)
		{
			char szOutput[512];
			sprintf(szOutput, 
				ZErrStr(MERR_CHAT_PENALTY_FOR_ONE_MINUTE) );
			Output(szOutput, CMT_SYSTEM);
			return false;
		}
		else
		{
			m_nLastAbuseTime = 0;
		}
	}

	if ( !MGetChattingFilter()->IsValidChatting(szMsg))
	{
#ifndef _DEBUG
		m_nAbuseCounter++;

		if ( m_nAbuseCounter >= 3)			// 3번 연속 욕하면 1분간 채팅금지
			m_nLastAbuseTime = timeGetTime();
#endif

		char szOutput[512];
		sprintf( szOutput, "%s (%s)", ZErrStr( MERR_CANNOT_ABUSE), MGetChattingFilter()->GetLastFilteredStr());
		Output( szOutput, CMT_SYSTEM);

		return false;
	}
	else
	{
		m_nAbuseCounter = 0;
	}

	return true;
}

bool _InsertString(char* szTarget, const char* szInsert, int nPos, int nMaxTargetLen=-1)
{
	int nTargetLen = (int)strlen(szTarget);
	int nInsertLen = (int)strlen(szInsert);
	if(nPos>nTargetLen) return false;
	if(nMaxTargetLen>0 && nTargetLen+nInsertLen>=nMaxTargetLen) return false;

	char* temp = new char[nTargetLen-nPos+2];
	strcpy(temp, szTarget+nPos);
	strcpy(szTarget+nPos, szInsert);
	strcpy(szTarget+nPos+nInsertLen, temp);
	delete[] temp;

	return true;
}

// 채팅창에서 '/r ' 을 치면 이전에 귓말이 왔던 사람한테 바로 귓말 명령어로 변경
void ZChat::FilterWhisperKey(MWidget* pWidget)
{
	char text[256];
	strcpy(text, pWidget->GetText());

	if ((!stricmp(text, "/r ")) || (!stricmp(text, "/ㄱ ")))
	{
		char msg[128] = "";

		ZChatCmd* pWhisperCmd = GetCmdManager()->GetCommandByID(CCMD_ID_WHISPER);
		if (pWhisperCmd)
		{
			sprintf(msg, "/%s ", pWhisperCmd->GetName());
		}

		//strcpy(msg, "/귓말 ");

		if (m_szWhisperLastSender[0])
		{
			strcat(msg, m_szWhisperLastSender);
			strcat(msg, " ");
		}
		pWidget->SetText(msg);
	}


}
///////////////////////////////////////////////////////////////////////////
