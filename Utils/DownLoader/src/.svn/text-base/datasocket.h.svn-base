#pragma once

#include "list.h"

#define SDW(d)	(((d & 0xFF000000) >> 24) | ((d & 0x00FF0000) >> 8) | ((d & 0x000000FF) << 24) | ((d & 0x0000FF00) << 8))
#define CDW(d)	(*(DWORD *)&(d))

#define SW(w)	(((w & 0xFF00) >> 8) | ((w & 0x00FF) << 8))
#define CW(w)	(*(WORD *)&(w))

// Send Interval in Milliseconds
#define DATASOCKET_SEND_INTERVAL 250

/**		비동기 소켓 클래스
*
*		@author soyokaze
*		@date   2005-11-02
*/

class DataSocket
{
	typedef void (*DATASOCKET_CALLBACK)(DataSocket * ds, DWORD_PTR Custom, int ErrorCode);
public:
	DataSocket();
	virtual ~DataSocket();

	// Interface

	bool Create(WORD Port = 0, int Type = SOCK_STREAM);
	void Close(void);
	void Listen(void);
	void Accept(DataSocket & s);
	bool Connect(const char * Address, WORD Port);

	void GetPeerName(CString & Address, WORD & Port);

	void SetCallbacks(DATASOCKET_CALLBACK cbAccept, DATASOCKET_CALLBACK cbClose, DATASOCKET_CALLBACK cbConnect, DATASOCKET_CALLBACK cbReceive, DATASOCKET_CALLBACK cbSend, DWORD_PTR Custom);

	void SendString(const char * Str);
	void Send(const void * Buf, DWORD BufLen);

	BYTE * GetData(void);
	DWORD GetDataLen(void);
	void Discard(DWORD Bytes);
	DWORD GetSendDataLen(void);

	DWORD GetBpsIn(void);
	DWORD GetBpsOut(void);
	void UpdateBps(void);

	void SetMaxOut(DWORD Bps);
	DWORD GetMaxOut(void);

	// Mfc Overrides
	virtual void OnAccept(int nErrorCode);
	virtual void OnClose(int nErrorCode);
	virtual void OnConnect(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);

protected:
	static void InitWindow(void);
	static void KillWindow(void);
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	static DWORD m_SocketCount;
	static HWND m_SocketWnd;
	static List<DataSocket *> m_Sockets;

	SOCKET m_hSocket;

	DATASOCKET_CALLBACK m_Accept;
	DATASOCKET_CALLBACK m_Close;
	DATASOCKET_CALLBACK m_Connect;
	DATASOCKET_CALLBACK m_Receive;
	DATASOCKET_CALLBACK m_Send;
	DWORD_PTR m_Custom;

	BYTE * m_RecvBuf;
	DWORD m_RecvLen;
	DWORD m_RecvMax;
	DWORD m_RecvBytes;
	DWORD m_BpsIn;

	BYTE * m_SendBuf;
	DWORD m_SendLen;
	DWORD m_SendMax;
	DWORD m_SendBytes;
	DWORD m_BpsOut;
	DWORD m_MaxBpsOut;
};
