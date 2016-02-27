#ifndef NETMANAGER_H
#define NETMANAGER_H

#include "HMailSlot.h"
#include "MailSlotMsg.h"


#define IDT_REFRESH_NETLOCAL	100
#define MSEC_REFRESH_NETLOCAL	5000
#define IDT_RECEIVE				101
#define MSEC_RECEIVE			100


class NetManager {
	char m_szMyName[256];

	HMailSlot	m_MailSlot;

	UINT m_hReceiveTimer;
	static VOID CALLBACK TimerProcReceive(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);

	UINT m_hRefreshNetLocal;
	static VOID CALLBACK TimerProcRefreshNetLocal(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);

public:
	bool Create();
	bool Destroy();

	void SetMyName(char* szName) { strcpy(m_szMyName, szName); }
	char* GetMyName() { return m_szMyName; }

	char* GetMailSlotPath(char* szAddr);
	char* GetSenderName(PRMSG_GENERIC pMsg, DWORD dwMsgSize);
	char* GetReceiverName(PRMSG_GENERIC pMsg, DWORD dwMsgSize);

	void MSSendMsgKnock(char* szDestAddr);

	void Receive();
	void MsgParse(PRMSG_GENERIC pMsg, DWORD dwMsgSize);
	void MSGHandler_KNOCK(PRMSG_GENERIC pMsg, DWORD dwMsgSize);
};


#endif

