#include "stdafx.h"
#include "MCommandCommunicator.h"
#include "MErrorTable.h"
#include <stdarg.h>
#include "MCommandBuilder.h"


MCommObject::MCommObject(MCommandCommunicator* pCommunicator)
{
	m_uid = MUID(0,0);

	m_pDirectConnection = NULL;
	m_dwUserContext = 0;

	m_szIP[0] = NULL;
	m_nPort = 0;

	m_bAllowed = true;

	m_pCommandBuilder = new MCommandBuilder(MUID(0,0), pCommunicator->GetUID(), 
											pCommunicator->GetCommandManager());

}

MCommObject::~MCommObject()
{
	if (m_pCommandBuilder) {
		delete m_pCommandBuilder;
		m_pCommandBuilder = NULL;
	}
}

void MCommandCommunicator::ReceiveCommand(MCommand* pCommand)
{
	pCommand->m_Receiver = m_This;
	m_CommandManager.Post(pCommand);
}

void MCommandCommunicator::OnRegisterCommand(MCommandManager* pCommandManager)
{
}

bool MCommandCommunicator::OnCommand(MCommand* pCommand)
{
	return false;
}

void MCommandCommunicator::OnPrepareRun(void)
{
}

void MCommandCommunicator::OnPrepareCommand(MCommand* pCommand)
{
}

void MCommandCommunicator::OnRun(void)
{
}

void MCommandCommunicator::SetDefaultReceiver(MUID Receiver)
{
	m_DefaultReceiver = Receiver;
}

MCommandCommunicator::MCommandCommunicator(void)
{
	m_This.SetZero();
	m_DefaultReceiver.SetZero();
}

MCommandCommunicator::~MCommandCommunicator(void)
{
	Destroy();
#ifdef _CMD_PROFILE
	m_CommandProfiler.Analysis();
#endif
}

bool MCommandCommunicator::Create(void)
{
#ifdef _CMD_PROFILE
	m_CommandProfiler.Init(&m_CommandManager);
#endif

	OnRegisterCommand(&m_CommandManager);	// Virtual Function은 Constructor에서 호출되지 않는다.
	return true;
}

void MCommandCommunicator::Destroy(void)
{
	while(MCommand* pCmd = GetCommandSafe()) {
		delete pCmd;
	}
}

int MCommandCommunicator::OnConnected(MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp, MCommObject* pCommObj)
{
	m_This = *pAllocUID;
	SetDefaultReceiver(*pTargetUID);

	if (pCommObj) {
		MCommandBuilder* pCmdBuilder = pCommObj->GetCommandBuilder();
		pCmdBuilder->SetUID(*pAllocUID, *pTargetUID);
	}
	
	return MOK;
}

bool MCommandCommunicator::Post(MCommand* pCommand)
{
	return m_CommandManager.Post(pCommand);
}

bool MCommandCommunicator::Post(char* szErrMsg, int nErrMsgCount, const char* szCommand)
{
	MCommand* pCmd = new MCommand;
	if(m_CommandManager.ParseMessage(pCmd, szErrMsg, nErrMsgCount, szCommand)==false){
		delete pCmd;
		return false;
	}

	pCmd->m_Sender = m_This;
	pCmd->m_Receiver = m_DefaultReceiver;

	if(Post(pCmd)==false){
		delete pCmd;
		return false;
	}

	return true;
}

MCommand* MCommandCommunicator::CreateCommand(int nCmdID, const MUID& TargetUID)
{
	return new MCommand(m_CommandManager.GetCommandDescByID(nCmdID), TargetUID, m_This);
}

MCommand* MCommandCommunicator::GetCommandSafe()
{
	return m_CommandManager.GetCommand();
}

void MCommandCommunicator::Run(void)
{
	OnPrepareRun();

	while(1){
		MCommand* pCommand = GetCommandSafe();

		if(pCommand==NULL) break;

		OnPrepareCommand(pCommand);

		/*if (pCommand->GetID() == MC_MATCH_P2P_COMMAND)
			MLog("Run MC_MATCH_P2P_COMMAND %d %d\n", pCommand->m_pCommandDesc->IsFlag(MCDT_PEER2PEER), pCommand->m_pCommandDesc->IsFlag(MCDT_LOCAL) == true ||
				(m_This.IsValid() && pCommand->m_Receiver == m_This));*/

		if ((pCommand->m_pCommandDesc->IsFlag(MCDT_PEER2PEER)==true))
		{
			if (pCommand->m_Sender != m_This)
			{
				#ifdef _CMD_PROFILE
					m_CommandProfiler.OnRecv(pCommand);
					m_CommandProfiler.OnCommandBegin(pCommand, timeGetTime());
				#endif
				OnCommand(pCommand);

				#ifdef _CMD_PROFILE
					m_CommandProfiler.OnCommandEnd(pCommand, timeGetTime());
				#endif
			}
			else
			{
				#ifdef _CMD_PROFILE
					m_CommandProfiler.OnSend(pCommand);
				#endif

				SendCommand(pCommand);

				#ifdef _CMD_PROFILE
					m_CommandProfiler.OnCommandBegin(pCommand, timeGetTime());
				#endif

				OnCommand(pCommand);

				#ifdef _CMD_PROFILE
					m_CommandProfiler.OnCommandEnd(pCommand, timeGetTime());
				#endif
			}
		}
		else if (pCommand->m_pCommandDesc->IsFlag(MCDT_LOCAL)==true || 
			    (m_This.IsValid() && pCommand->m_Receiver==m_This))
		{
			#ifdef _CMD_PROFILE
				m_CommandProfiler.OnRecv(pCommand);
				m_CommandProfiler.OnCommandBegin(pCommand, timeGetTime());
			#endif

			OnCommand(pCommand);	// Local Command면 로컬에서 처리

			#ifdef _CMD_PROFILE
				m_CommandProfiler.OnCommandEnd(pCommand, timeGetTime());
			#endif
		}
		else 
		{
			#ifdef _CMD_PROFILE
				m_CommandProfiler.OnSend(pCommand);
			#endif

			SendCommand(pCommand);	// 그외에는 설정된 Receiver로 전송
		}

		delete pCommand;
		pCommand = NULL;
	}

	OnRun();
}

void MCommandCommunicator::LOG(unsigned int nLogLevel, const char *pFormat,...)
{
	if (nLogLevel != LOG_DEBUG)
	{
		va_list args;
		static char temp[1024];

		va_start(args, pFormat);
		vsprintf_safe(temp, pFormat, args);
		Log(nLogLevel, temp);
		va_end(args);
	}
	else
	{
#if defined(_DEBUG) && (!defined(_DEBUG_PUBLISH))
		va_list args;
		static char temp[1024];

		va_start(args, pFormat);
		vsprintf_safe(temp, pFormat, args);
		Log(nLogLevel, temp);
		va_end(args);
#endif
	}
}

MCommand* MCommandCommunicator::BlobToCommand(const void* Data, size_t Size)
{
	auto Command = new MCommand();

	Command->SetData((char*)Data, &m_CommandManager, Size);

	return Command;
}

MCommand* MCommandCommunicator::BlobToCommand(MCmdParamBlob* Blob)
{
	return BlobToCommand(Blob->GetPointer(), Blob->GetPayloadSize());
}

MCmdParamBlob* CommandToBlob(MCommand& Command)
{
	size_t BlobSize = Command.GetSize();

	auto Param = new MCmdParamBlob();

	Param->m_Value = new char[BlobSize];

	Command.GetData(static_cast<char*>(Param->m_Value), BlobSize);
}


int CalcPacketSize(MCommand* pCmd)
{
	return (sizeof(MPacketHeader) + pCmd->GetSize());
}
