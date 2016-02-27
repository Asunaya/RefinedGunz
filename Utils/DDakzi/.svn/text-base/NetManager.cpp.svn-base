#include "NetManager.h"
#include <windowsx.h>
#include "NetBase.h"

bool NetManager::Create()
{
	char szComputerName[MAX_COMPUTERNAME_LENGTH+1];
	DWORD dwComputerNameLen = MAX_COMPUTERNAME_LENGTH+1;
	GetComputerName(szComputerName, &dwComputerNameLen);
	SetMyName(szComputerName);

	m_MailSlot.CreateSlot(".", "hellobuddy");

	m_hRefreshNetLocal = SetTimer(GetActiveWindow(), IDT_REFRESH_NETLOCAL, MSEC_REFRESH_NETLOCAL, TimerProcRefreshNetLocal);
	m_hReceiveTimer = SetTimer(GetActiveWindow(), IDT_RECEIVE, MSEC_RECEIVE, TimerProcReceive);
	return true;
}

bool NetManager::Destroy()
{
	KillTimer(GetActiveWindow(), m_hRefreshNetLocal); 
	m_hRefreshNetLocal = 0;
	KillTimer(GetActiveWindow(), m_hReceiveTimer); 
	m_hReceiveTimer = 0;

	m_MailSlot.DestroySlot();
	return true;
}

char* NetManager::GetMailSlotPath(char* szAddr)
{
	static char szMailSlotPath[256];
	wsprintf(szMailSlotPath, "\\\\%s\\mailslot\\%s", szAddr, "hellobuddy");
	return szMailSlotPath;
}

char* NetManager::GetSenderName(PRMSG_GENERIC pMsg, DWORD dwMsgSize)
{
	DWORD dwBufferSize = dwMsgSize - sizeof(pMsg->dwType);
	for (DWORD i=1; i<dwMsgSize; i++) {
		if (*(pMsg->pBuffer + i) == NULL)
			return pMsg->pBuffer;
	}
	return NULL;
}

char* NetManager::GetReceiverName(PRMSG_GENERIC pMsg, DWORD dwMsgSize)
{
	DWORD dwBufferSize = dwMsgSize - sizeof(pMsg->dwType);

	bool bSenderFound = false;
	for (DWORD i=1; i<dwMsgSize; i++) {
		if (*(pMsg->pBuffer + i) == NULL) {
			bSenderFound = true;
			break;
		}
	}
	if (bSenderFound == false) 
		return NULL;
	
	for (DWORD j=i+1; j<dwMsgSize; j++) {
		if (*(pMsg->pBuffer + j) == NULL)
			return (pMsg->pBuffer + i + 1);
	}

	return NULL;
}

VOID CALLBACK NetManager::TimerProcRefreshNetLocal(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	NetManager* pManager = HBGetNetManager();
	pManager->MSSendMsgKnock("*");
}

void NetManager::MSSendMsgKnock(char* szDestAddr)
{
	int nMyAddrLen = strlen(m_szMyName);
	int nDestAddrLen = strlen(szDestAddr);
	int nOffset = 0;
	DWORD dwMsgSize = sizeof(RMSG_KNOCK) + nMyAddrLen+1 + nDestAddrLen+1;

	PRMSG_KNOCK pMsg = (PRMSG_KNOCK)GlobalAllocPtr(GHND, dwMsgSize);
	pMsg->dwType = MSMSG_KNOCK;
	memcpy(pMsg->pBuffer, m_szMyName, nMyAddrLen);
	nOffset = nMyAddrLen;
	*(pMsg->pBuffer+nOffset) = NULL;
	nOffset++;
	memcpy(pMsg->pBuffer+nOffset, szDestAddr, nDestAddrLen);
	nOffset += nDestAddrLen;
	*(pMsg->pBuffer+nOffset) = NULL;

	char* szMailSlotPath = GetMailSlotPath(szDestAddr);
	m_MailSlot.WriteSlot(szMailSlotPath, (char*)pMsg, dwMsgSize);
}

VOID CALLBACK NetManager::TimerProcReceive(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	NetManager* pManager = HBGetNetManager();
	pManager->Receive();
}

void NetManager::Receive()
{
	DWORD dwSize = m_MailSlot.PeekSlot();
	if (dwSize == 0) 
		return;

	PRMSG_GENERIC pMsgBuffer = (PRMSG_GENERIC)GlobalAllocPtr(GHND, dwSize);
	DWORD dwReadSize = dwSize;
	m_MailSlot.ReadSlot((char*)pMsgBuffer, &dwReadSize);

	MsgParse(pMsgBuffer, dwReadSize);
}

void NetManager::MsgParse(PRMSG_GENERIC pMsg, DWORD dwMsgSize)
{
/*	char szBuf[256];
	wsprintf(szBuf, "MSG Received : %d Bytes\n", dwMsgSize);
	OutputDebugString(szBuf);	*/

	switch (pMsg->dwType) {
	case MSMSG_KNOCK:
		MSGHandler_KNOCK(pMsg, dwMsgSize);
		break;

	default:
		break;
	};
}

void NetManager::MSGHandler_KNOCK(PRMSG_GENERIC pMsg, DWORD dwMsgSize)
{
	char* pszSender = GetSenderName(pMsg, dwMsgSize);
	char* pszReceiver = GetReceiverName(pMsg, dwMsgSize);
	if ((pszSender==NULL) || (pszReceiver==NULL))
		return;

	if ( !((*pszReceiver == '.') || (*pszReceiver == '*') || 
		  (stricmp(pszReceiver, m_szMyName) == 0)) )
		return;

	char szBuf[256];
	wsprintf(szBuf, "SENDER:'%s', RECEIVER:'%s'\n", pszSender, pszReceiver);
	OutputDebugString(szBuf);
}
