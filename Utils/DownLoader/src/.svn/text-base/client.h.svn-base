#pragma once

#include "datasocket.h"
#include "listctrl.h"
#include "accrue.h"

#define CLIENTSTATE_UNKNOWN		0
#define CLIENTSTATE_AUTH		1
#define CLIENTSTATE_ACTIVE		2

#define MAX_PIECE_REQUEST_COUNT	5

#define CLIENT_IMG_IDLE			0
#define CLIENT_IMG_SEND			1
#define CLIENT_IMG_RECV			2
#define CLIENT_IMG_SENDRECV		3

class Torrent;

/**		클라이언트들과 연결하며 목록을 관리하며 이들과 통신한다.
*		주고 받을 데이터 블럭의 목록도 관리한다.
*
*		@author soyokaze
*		@date   2005-11-02
*/

class Client
{
	struct Request
	{
		DWORD m_Block;
		DWORD m_Offset;
		DWORD m_Length;
	};
public:
	Client(void);
	~Client(void);

	// Interface
	void Create(const char * Address, WORD Port, BYTE * PeerId);
	void Create(DataSocket * s);

	bool IsDead(void);
	bool IsRemoteConnection(void);
	void Update(void);
	CString GetIp(void);
	bool IsClientInterested(void);
	bool IsChoked(void);	// Is Torrent Choking Client
	void Kill(void);
	DWORD GetBytesInDelta(void);
	DWORD GetBytesOutDelta(void);
	DWORD GetBpsIn(void);
	DWORD GetBpsOut(void);
	DWORD GetAvgBpsIn(void);
	DWORD GetAvgBpsOut(void);
	bool IsSeed(void);
	bool IsLeecher(void);
	void SetMaxUpload(DWORD Rate);
	float GetAverage(void);

	// Prep
	void SetParent(Torrent * Parent);
	void SetClientList(ListCtrl * tlc);

	// Socket Callbacks
	static void _OnClose(DataSocket * s, DWORD_PTR Custom, int ErrorCode);
	static void _OnConnect(DataSocket * s, DWORD_PTR Custom, int ErrorCode);
	static void _OnReceive(DataSocket * s, DWORD_PTR Custom, int ErrorCode);
	static void _OnSend(DataSocket * s, DWORD_PTR Custom, int ErrorCode);

	void OnClose(int ErrorCode);
	void OnConnect(int ErrorCode);
	void OnReceive(int ErrorCode);
	void OnSend(int ErrorCode);

	// Send
	void Send_Handshake(void);
	void Send_BitField(void);
	void Send_KeepAlive(void);
	void Send_Interested(void);
	void Send_NotInterested(void);
	void Send_Request(DWORD Block, DWORD Offset, DWORD Length);
	void Send_Have(DWORD Block);
	void Send_UnChoke(void);
	void Send_Choke(void);
	void Send_Piece(void);
	void Send_Cancel(DWORD Block, DWORD Offset, DWORD Length);

	// Recv
	void Recv_Choke(void);
	void Recv_UnChoke(void);
	void Recv_Interested(void);
	void Recv_NotInterested(void);
	void Recv_Have(void);
	void Recv_BitField(void);
	void Recv_Request(void);
	void Recv_Piece(void);
	void Recv_Cancel(void);

protected:
	void Init(void);

	bool Match(const char * Pattern);
	void CheckInterest(void);
	void RequestBlock(void);
	void RequestPiece(void);
	void RemoveRequest(DWORD r = 0);

	DWORD m_State;
	BYTE m_PeerId[20];
	BYTE * m_Blocks;
	bool m_Complete;
	DWORD m_BlocksComplete;
	DWORD m_KeepAlive;
	DWORD m_PieceTimeout;
	DWORD m_Timeout;

	DWORD m_BpsIn;
	DWORD m_BpsOut;

	QWORD m_BytesIn;
	QWORD m_BytesOut;

	DWORD m_BytesInDelta;
	DWORD m_BytesOutDelta;

	// Our State
	bool m_Interested;	// Whether I'm Interested in this clients data
	bool m_Choking;		// Whether I'm choking this client or not

	// Thier State
	bool m_ClientChoking;		// Client is Choking me
	bool m_ClientInterested;	// Client is interested in my data
	QWORD m_ClientSpeed;
	clock_t m_ClientStarted;	// For calculating ClientSpeed

	bool m_Dead;

	ListCtrl * m_cClientList;
	ListItem m_hItem;

	DataSocket * m_Socket;
	Torrent * m_Parent;

	List<Request> m_RequestQueue;
	List<Request> m_SendQueue;

	Accrue<DWORD> m_AvgBpsIn;
	Accrue<DWORD> m_AvgBpsOut;

	bool m_RemoteConnection;	// Tells whether the connection was initiated from a remote host
};
