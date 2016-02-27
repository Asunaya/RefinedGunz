#include "StdAfx.h"
#include "client.h"
#include "torrent.h"

Client::Client(void)
{
	m_Socket = NULL;
	m_Parent = NULL;
	m_State = CLIENTSTATE_UNKNOWN;
	m_Blocks = NULL;
	m_Complete = false;
	m_BlocksComplete = 0;
	m_Dead = false;
	m_KeepAlive = Settings::GetDWord("ClientKeepAlive") * 60;
	m_Timeout = Settings::GetDWord("ClientTimeout") * 60;
	m_PieceTimeout = 0;
	m_ClientSpeed = 0;

	m_BpsIn = 0;
	m_BpsOut = 0;

	m_BytesIn = 0;
	m_BytesOut = 0;

	m_BytesInDelta = 0;
	m_BytesOutDelta = 0;

	m_Interested = false;
	m_Choking = true;

	m_ClientInterested = false;
	m_ClientChoking = true;

	m_RemoteConnection = false;
}

Client::~Client(void)
{
	DWORD i;

	m_cClientList->DeleteItem(m_hItem);

	if(m_Parent)
	{
		while(m_RequestQueue.GetSize())
		{
			RemoveRequest();
		}
	}

	if(m_Socket) MFree(m_Socket);

	if(m_Blocks)
	{
		for(i = 0; i < m_Parent->GetBlockCount(); i++)
		{
			if(m_Blocks[i] == BLOCKSTATE_COMPLETE)
			{
				m_Parent->DecrementRarity(i);
			}
		}
		MFree(m_Blocks);
	}

//	m_Progress.DestroyWindow();
//	m_Bars.DestroyWindow();

	m_Parent->UnQueueClient(this);
}

void Client::Create(const char * Address, WORD Port, BYTE * PeerId)
{
	CString Temp;
	memcpy(m_PeerId, PeerId, 20);

	Temp.Format("%s:%d", Address, Port);
	m_cClientList->SetItemText(m_hItem, Temp, 0);

	Init();

	m_Socket = MAlloc(DataSocket, 1);

	m_Socket->SetCallbacks(0, _OnClose, _OnConnect, _OnReceive, _OnSend, (DWORD_PTR)this);
	m_Socket->Create();
	m_Socket->Connect(Address, Port);
}

void Client::Create(DataSocket * s)
{
	CString Address;
	WORD Port;
	CString Temp;

	m_RemoteConnection = true;

	m_Socket = s;
	m_Socket->SetCallbacks(0, _OnClose, _OnConnect, _OnReceive, _OnSend, (DWORD_PTR)this);

	m_Socket->GetPeerName(Address, Port);

	Temp.Format("*%s:%d", Address, Port);
	m_cClientList->SetItemText(m_hItem, Temp, 0);

	DEBUG_PROTOCOL("Client %s:%d Routed to '%s'\n", Address, Port, m_Parent->GetTorrentName());

	Init();

	OnConnect(0);
}

void Client::Init(void)
{
	m_Blocks = MAlloc(BYTE, m_Parent->GetBlockCount());
	memset(m_Blocks, BLOCKSTATE_INCOMPLETE, m_Parent->GetBlockCount());

	m_cClientList->SetItemText(m_hItem, "Connecting...", 1);
	m_cClientList->SetItemText(m_hItem, "-", 2);
	m_cClientList->SetItemText(m_hItem, "-", 3);
	m_cClientList->SetItemText(m_hItem, "-", 4);
	m_cClientList->SetItemText(m_hItem, "-", 5);
}

void Client::SetParent(Torrent * Parent)
{
	m_Parent = Parent;
}

void Client::Kill(void)
{
	if(m_Dead) return;
	if(m_Socket) m_Socket->Close();
	m_Dead = true;
}

bool Client::IsDead(void)
{
	return m_Dead;
}

bool Client::IsRemoteConnection(void)
{
	return m_RemoteConnection;
}

void Client::Update(void)
{
	if(m_State == CLIENTSTATE_ACTIVE)
	{
		m_Socket->UpdateBps();

		m_BpsIn = m_Socket->GetBpsIn();
		m_BpsOut = m_Socket->GetBpsOut();

		m_AvgBpsIn.Add(m_BpsIn);
		m_AvgBpsOut.Add(m_BpsOut);

		m_cClientList->SetItemText(m_hItem, Util::FormatBps(m_AvgBpsIn.GetAverage()), 2);
		m_cClientList->SetItemText(m_hItem, Util::FormatBps(m_AvgBpsOut.GetAverage()), 3);

		m_cClientList->SetItemText(m_hItem, Util::FormatBytes(m_BytesIn), 4);
		m_cClientList->SetItemText(m_hItem, Util::FormatBytes(m_BytesOut), 5);

		if(m_KeepAlive == 0)
		{
			m_KeepAlive = Settings::GetDWord("ClientKeepAlive") * 60;
			Send_KeepAlive();
		}
		else
		{
			m_KeepAlive--;
		}
		
		if(!m_ClientChoking)
		{
			m_cClientList->SetItemText(m_hItem, "Downloading", 1);
		}
		else
		{
			if(m_Choking)
			{
				m_cClientList->SetItemText(m_hItem, "Idle", 1);
			}
			else
			{
				m_cClientList->SetItemText(m_hItem, "Uploading", 1);
			}
		}


		// Download Timeouts
		if(m_PieceTimeout < GetTickCount() && m_PieceTimeout != 0)
		{
			DEBUG_PROTOCOL("%21s - Timeout\n", GetIp());

			m_PieceTimeout = 0;
			
			// Todo: Not kill timeouts? Instead ensure they're choked and cancel all requested pieces?
			Kill();

			while(m_RequestQueue.GetSize())
			{
				RemoveRequest();
			}
		}
	}

	m_BytesInDelta = 0;
	m_BytesOutDelta = 0;
}

CString Client::GetIp(void)
{
	CString Ip;
	CString Result;
	WORD Port;

	if(m_Socket)
	{
		m_Socket->GetPeerName(Ip, Port);
	}

	Result.Format("%s:%d", Ip, Port);

	return Result;
}

void Client::SetClientList(ListCtrl * lc)
{
	m_cClientList = lc;
	m_hItem = m_cClientList->AddItem();
}

void Client::CheckInterest(void)
{
	DWORD i;

	if(m_Parent->IsComplete())
	{
		Send_NotInterested();

		if(m_BlocksComplete == m_Parent->GetBlocksComplete())
		{
			Kill();
		}

		return;
	}

	for(i = 0; i < m_Parent->GetBlockCount(); i++)
	{
		if(m_Parent->GetBlockState(i) != BLOCKSTATE_COMPLETE && m_Blocks[i] == BLOCKSTATE_COMPLETE)
		{
			Send_Interested();
			return;
		}
	}

	Send_NotInterested();
}

bool Client::Match(const char * Pattern)
{
	DWORD i;

	for(i = 0; i < (DWORD)strlen(Pattern); i++)
	{
		switch(Pattern[i])
		{
			// Numeric
		case '*':
			if(m_PeerId[i] < '0' || m_PeerId[i] > '9')
			{
				return false;
			}
			break;

			// Alpha
		case '^':
			if(m_PeerId[i] < 'A' || (m_PeerId[i] > 'Z' && m_PeerId[i] < 'a') || m_PeerId[i] > 'z')
			{
				return false;
			}
			break;

			// Alpha Numeric
		case '$':
			if(m_PeerId[i] < '0' || (m_PeerId[i] > '9' && m_PeerId[i] < 'A') || (m_PeerId[i] > 'Z' && m_PeerId[i] < 'a') || m_PeerId[i] > 'z')
			{
				return false;
			}
			break;

		default:
			if(Pattern[i] != m_PeerId[i])
			{
				return false;
			}
		}
	}

	return true;
}

void Client::RequestBlock(void)
{
	DWORD Block;
	DWORD i;
	DWORD Len;

	for(i = 0; i < m_Parent->GetIncompleteBlockCount(); i++)
	{
		Block = m_Parent->GetIncompleteBlock(i);

		if(m_Blocks[Block] == BLOCKSTATE_COMPLETE) break;
		Block = (DWORD)-1;
	}

	if(Block == (DWORD)-1) return;

	m_Parent->SetIncompleteBlockDownloading(Block);

	m_Parent->SetBlockState(Block, BLOCKSTATE_DOWNLOADING);

	// 128K Piece Blocks
	for(i = 0; i < m_Parent->GetBlockSize(Block); i += (128 * 1024))
	{
		Len = 128 * 1024;
		if(Len > m_Parent->GetBlockSize(Block) - i)
		{
			Len = m_Parent->GetBlockSize(Block) - i;
		}
		Send_Request(Block, i, Len);
	}

	for(i = 0; i < m_Parent->GetBlockPieceCount(Block); i++)
	{
		if(m_Parent->GetPieceState(Block, i) != BLOCKSTATE_COMPLETE) m_Parent->SetPieceState(Block, i, BLOCKSTATE_DOWNLOADING);
	}
}

void Client::RequestPiece(void)
{
	DWORD i, j, k, l;
	DWORD Block;
	bool Valid;
	DWORD * Randomize;

	if(!m_Interested) return;

	if(m_Parent->GetIncompleteBlockCount() == 0 || (m_Parent->GetCompleteBlockCount() == 0 && m_Parent->GetDownloadingBlockCount() != 0))
	{
		if(m_Parent->GetDownloadingBlockCount() == 0) return;

		for(i = 0; i < m_Parent->GetDownloadingBlockCount(); i++)
		{
			Block = m_Parent->GetDownloadingBlock(i);
			if(m_Blocks[Block] == BLOCKSTATE_COMPLETE)
			{
				Randomize = MAlloc(DWORD, m_Parent->GetBlockPieceCount(Block));

				for(j = 0; j < m_Parent->GetBlockPieceCount(Block); j++)
				{
					Randomize[j] = j;
				}

				for(j = 0; j < m_Parent->GetBlockPieceCount(Block); j++)
				{
					l = rand() % m_Parent->GetBlockPieceCount(Block);
					k = Randomize[j];
					Randomize[j] = Randomize[l];
					Randomize[l] = k;
				}

				for(j = 0; j < m_Parent->GetBlockPieceCount(Block); j++)
				{
					// See if block is already downloading this piece
					Valid = true;
					for(k = 0; k < m_RequestQueue.GetSize(); k++)
					{
						if(m_RequestQueue[k].m_Block == Block)
						{
							if(m_RequestQueue[k].m_Offset / PIECESIZE == Randomize[j])
							{
								Valid = false;
								break;
							}
						}
					}

					if(Valid)
					{
						if(m_Parent->GetPieceState(Block, Randomize[j]) != BLOCKSTATE_COMPLETE)
						{
							if(m_Parent->GetPieceState(Block, Randomize[j]) != BLOCKSTATE_COMPLETE) m_Parent->SetPieceState(Block, Randomize[j], BLOCKSTATE_DOWNLOADING);
							Send_Request(Block, Randomize[j] * PIECESIZE, m_Parent->GetPieceSize(Block, Randomize[j]));
							
							MFree(Randomize);
							return;
						}
					}
				}

				MFree(Randomize);
			}
		}

		if(m_Parent->GetCompleteBlockCount() != 0) return;
	}else{
		for(i = 0; i < m_Parent->GetDownloadingBlockCount(); i++)
		{
			Block = m_Parent->GetDownloadingBlock(i);
			if(m_Blocks[Block] == BLOCKSTATE_COMPLETE)
			{
				for(j = 0; j < m_Parent->GetBlockPieceCount(Block); j++)
				{
					if(m_Parent->GetPieceState(Block, j) == BLOCKSTATE_INCOMPLETE)
					{
						if(m_Parent->GetPieceState(Block, j) != BLOCKSTATE_COMPLETE) m_Parent->SetPieceState(Block, j, BLOCKSTATE_DOWNLOADING);
						Send_Request(Block, j * PIECESIZE, m_Parent->GetPieceSize(Block, j));
						return;
					}
				}
			}
		}
	}

	// Get the Next Block to Download
	for(i = 0; i < m_Parent->GetIncompleteBlockCount(); i++)
	{
		Block = m_Parent->GetIncompleteBlock(i);

		if(m_Blocks[Block] == BLOCKSTATE_COMPLETE) break;
		Block = (DWORD)-1;
	}

	if(Block == (DWORD)-1) return;

	m_Parent->SetIncompleteBlockDownloading(Block);

	RequestPiece();
}

void Client::RemoveRequest(DWORD r)
{
	DWORD i;

	ASSERT(r < m_RequestQueue.GetSize());

	for(i = m_RequestQueue[r].m_Offset; i < m_RequestQueue[r].m_Offset + m_RequestQueue[r].m_Length; i += PIECESIZE)
	{
		DEBUG_PROTOCOL("Cancel Piece (%d, %d)\n", m_RequestQueue[r].m_Block, i);
		if(m_Parent->GetPieceState(m_RequestQueue[r].m_Block, i / PIECESIZE) != BLOCKSTATE_COMPLETE) m_Parent->SetPieceState(m_RequestQueue[r].m_Block, i / PIECESIZE, BLOCKSTATE_INCOMPLETE);
	}

	Send_Cancel(m_RequestQueue[r].m_Block, m_RequestQueue[r].m_Offset, m_RequestQueue[r].m_Length);

	m_RequestQueue.Delete(r);
}

bool Client::IsClientInterested(void)
{
	return m_ClientInterested;
}

bool Client::IsChoked(void)
{
	return m_Choking;
}

void Client::_OnClose(DataSocket * s, DWORD_PTR Custom, int ErrorCode)
{
	((Client *)Custom)->OnClose(ErrorCode);
}

void Client::OnClose(int ErrorCode)
{
	DEBUG_PROTOCOL("%21s - Client Disconnected\n", GetIp());
	Kill();
}

void Client::_OnConnect(DataSocket * s, DWORD_PTR Custom, int ErrorCode)
{
	((Client *)Custom)->OnConnect(ErrorCode);
}

void Client::OnConnect(int ErrorCode)
{
	if(ErrorCode)
	{
		Kill();
		return;
	}

	if(m_RemoteConnection)
	{
		m_cClientList->SetItemText(m_hItem, "*" + GetIp(), 0);
		m_Parent->Log("Connection from %s accepted.\n", GetIp());
	}
	else
	{
		m_cClientList->SetItemText(m_hItem, GetIp(), 0);
		m_Parent->Log("%s connected.\n", GetIp());
	}

	m_cClientList->SetItemText(m_hItem, "Connected", 1);

	m_State = CLIENTSTATE_AUTH;

	Send_Handshake();
}

void Client::_OnReceive(DataSocket * s, DWORD_PTR Custom, int ErrorCode)
{
	((Client *)Custom)->OnReceive(ErrorCode);
}

void Client::OnReceive(int ErrorCode)
{
	BYTE * Data;
	DWORD Len;

	if(ErrorCode)
	{
		DEBUG_PROTOCOL("%21s - Client::OnReceive - Error (%d) %s\n", GetIp(), ErrorCode, Util::StrError(ErrorCode));
		return;
	}

	Data = m_Socket->GetData();
	Len = m_Socket->GetDataLen();

	if(m_State == CLIENTSTATE_AUTH)
	{
		if(Len >= 68)
		{
			if(
				Data[0] == 19 &&
				memcmp(&Data[1], "BitTorrent protocol", 19) == 0 &&
				memcmp(&Data[28], m_Parent->GetTorrentId(), 20) == 0 &&
				/*(memcmp(&Data[48], m_PeerId, 20) == 0 || m_RemoteConnection) &&*/	// This code is now Useless thanks to the new Compact Tracker protocol
				memcmp(&Data[48], m_Parent->GetPeerId(), 20) != 0	// Prevent loopback connections
				)
			{
				/*if(m_RemoteConnection) */
				memcpy(m_PeerId, &Data[48], 20);

				// Todo: Debunk Duplicate connections?

				DEBUG_PROTOCOL("%21s - Client Authenticated\n", GetIp());
				m_State = CLIENTSTATE_ACTIVE;
				m_ClientStarted = clock();
				m_Socket->Discard(68);
				
				Send_BitField();

				OnReceive(ErrorCode);
			}
			else
			{
				DEBUG_PROTOCOL("%21s - Client Failed Authentication\n", GetIp());

				Kill();
				return;
			}
		}
	}else{
		if(Len < 4) return;
		if(Len < SDW(CDW(Data[0])) + 4) return;

		if(SDW(CDW(Data[0])) != 0)
		{
			switch(Data[4])
			{
			case 0x00:	Recv_Choke();			break;
			case 0x01:	Recv_UnChoke();			break;
			case 0x02:	Recv_Interested();		break;
			case 0x03:	Recv_NotInterested();	break;
			case 0x04:	Recv_Have();			break;
			case 0x05:	Recv_BitField();		break;
			case 0x06:	Recv_Request();			break;
			case 0x07:	Recv_Piece();			break;
			case 0x08:	Recv_Cancel();			break;
			}
		}

		m_Socket->Discard(SDW(CDW(Data[0])) + 4);

		OnReceive(ErrorCode);
	}
}

void Client::_OnSend(DataSocket * s, DWORD_PTR Custom, int ErrorCode)
{
	((Client *)Custom)->OnSend(ErrorCode);
}

void Client::OnSend(int ErrorCode)
{
	if(ErrorCode)
	{
		DEBUG_PROTOCOL("%21s - Client::OnSend - Error (%d) %s\n", GetIp(), ErrorCode, Util::StrError(ErrorCode));
		return;
	}

	if(m_SendQueue.GetSize() > 0 && m_Choking == false)
	{
		Send_Piece();
	}
}

//
// Send
//

//13
//42 69 74 54 6F 72 72 65 6E 74 20 70 72 6F 74 6F 63 6F 6C
//00 00 00 00 00 00 00 00
//29 9F AF 4C 34 60 23 50 1E F4 52 85 B6 B4 BC 6D 2E 85 F7 89


void Client::Send_Handshake(void)
{
	DEBUG_PROTOCOL("%21s - Send_Handshake\n", GetIp());

	BYTE Out[68];

	Out[0] = 19;
	memcpy(&Out[1], "BitTorrent protocol", 19);
	memset(&Out[20], 0, 8);
	memcpy(&Out[28], m_Parent->GetTorrentId(), 20);
	memcpy(&Out[48], m_Parent->GetPeerId(), 20);
	
	m_Socket->Send(Out, 68);
}

void Client::Send_BitField(void)
{
	DEBUG_PROTOCOL("%21s - Send_BitField\n", GetIp());

	BYTE * Out;
	DWORD i;
	int ByteNum;
	int BitNum;
	DWORD ByteCount;
	DWORD NumBlocks;

	ByteCount = ((m_Parent->GetBlockCount() - 1) / 8) + 1;
	NumBlocks = m_Parent->GetBlockCount();

	Out = MAlloc(BYTE, 5 + ByteCount);

	ZeroMemory(Out, 5 + ByteCount);

	for(i = 0; i < NumBlocks; i++)
	{
		ByteNum = i / 8;
		BitNum = 7 - (i % 8);

		if(m_Parent->GetBlockState(i) == BLOCKSTATE_COMPLETE)
		{
			Out[5 + ByteNum] |= (1 << BitNum);
		}else{
		}
	}

	CDW(Out[0]) = SDW(ByteCount + 1);
	Out[4] = 0x05;

	m_Socket->Send(Out, 5 + ByteCount);

	MFree(Out);
}

void Client::Send_KeepAlive(void)
{
	BYTE Out[4];

	if(m_State != CLIENTSTATE_ACTIVE) return;

	DEBUG_PROTOCOL("%21s - Send_KeepAlive\n", GetIp());

	CDW(Out[0]) = SDW(0x00000000);

	m_Socket->Send(Out, 4);
}

void Client::Send_Interested(void)
{
	BYTE Out[5];

	if(m_Interested) return;

	m_Interested = true;

	DEBUG_PROTOCOL("%21s - Send_Interested\n", GetIp());

	CDW(Out[0]) = SDW(0x00000001);
	Out[4] = 0x02;

	m_Socket->Send(Out, 5);
}

void Client::Send_NotInterested(void)
{
	BYTE Out[5];

	if(!m_Interested) return;

	m_Interested = false;

	DEBUG_PROTOCOL("%21s - Send_NotInterested\n", GetIp());

	CDW(Out[0]) = SDW(0x00000001);
	Out[4] = 0x03;

	m_Socket->Send(Out, 5);
}

void Client::Send_Request(DWORD Block, DWORD Offset, DWORD Length)
{
	BYTE Out[17];
	Request r;

	if(Block >= m_Parent->GetBlockCount()) { ASSERT(FALSE); return; }
	
	DEBUG_PROTOCOL("%21s - Send_Request (%d, %d, %d)\n", GetIp(), Block, Offset, Length);

//	m_Parent->SetPieceState(Block, Piece, PIECESTATE_DOWNLOADING);
	m_Parent->SetBlockState(Block, BLOCKSTATE_DOWNLOADING);

	if(m_RequestQueue.GetSize() == 0)
	{
		m_PieceTimeout = GetTickCount() + Settings::GetDWord("PieceTimeout") * 1000 * 60;
	}

	r.m_Block = Block;
	r.m_Offset = Offset;
	r.m_Length = Length;

	m_RequestQueue.PushBack(r);

	CDW(Out[0]) = SDW(13);
	Out[4] = 0x06;
	CDW(Out[5]) = SDW(Block);
	CDW(Out[9]) = SDW(Offset);
	CDW(Out[13]) = SDW(Length);

	m_Socket->Send(Out, 17);
}

void Client::Send_Have(DWORD Block)
{
	BYTE Out[9];

	if(m_State != CLIENTSTATE_ACTIVE) return;

	DEBUG_PROTOCOL("%21s - Send_Have (%d)\n", GetIp(), Block);

	CDW(Out[0]) = SDW(0x00000005);
	Out[4] = 0x04;
	CDW(Out[5]) = SDW(Block);

	m_Socket->Send(Out, 9);

	CheckInterest();
}

void Client::Send_UnChoke(void)
{
	BYTE Out[5];

	if(!m_Choking) return;

	m_Choking = false;

	DEBUG_PROTOCOL("%21s - Send_UnChoke\n", GetIp());

	CDW(Out[0]) = SDW(0x00000001);
	Out[4] = 0x01;

	m_Socket->Send(Out, 5);

	if(m_ClientInterested) m_Parent->QueueClient(this);
}

void Client::Send_Choke(void)
{
	BYTE Out[5];

	if(m_Choking) return;

	m_Choking = true;

	DEBUG_PROTOCOL("%21s - Send_Choke\n", GetIp());

	CDW(Out[0]) = SDW(0x00000001);
	Out[4] = 0x00;

	m_Socket->Send(Out, 5);

	m_SendQueue.DeleteAll();

	m_Parent->UnQueueClient(this);
}

void Client::Send_Piece(void)
{
	Request r;
	BYTE * Out;

	r = m_SendQueue[0];
	m_SendQueue.Delete(0);

	// Remove Block from BarMap
//	m_Bars.SetBlock(r.m_Block, BARSTATE_INCOMPLETE);
	// Reset Bars incase there was another piece requested from the removed block
//	for(i = 0; i < m_SendQueue.GetSize(); i++)
//	{
//		m_Bars.SetBlock(m_SendQueue[i].m_Block, BARSTATE_REQUESTED);
//	}

	DEBUG_PROTOCOL("%21s - Send_Piece (%d, %d, %d)\n", GetIp(), r.m_Block, r.m_Offset, r.m_Length);

	Out = MAlloc(BYTE, r.m_Length + 13);

	CDW(Out[0]) = SDW(r.m_Length + 9);
	Out[4] = 0x07;
	CDW(Out[5]) = SDW(r.m_Block);
	CDW(Out[9]) = SDW(r.m_Offset);

	m_Parent->ReadData(&Out[13], r.m_Block, r.m_Offset, r.m_Length);

	m_Socket->Send(Out, r.m_Length + 13);

	m_BytesOut += r.m_Length;
	m_BytesOutDelta += r.m_Length;

	MFree(Out);
}

void Client::Send_Cancel(DWORD Block, DWORD Offset, DWORD Length)
{
	BYTE Out[17];

	DEBUG_PROTOCOL("%21s - Send_Cancel (%d, %d, %d)\n", GetIp(), Block, Offset, Length);

	CDW(Out[0]) = SDW(0x0000000D);
	Out[4] = 0x08;
	CDW(Out[5]) = SDW(Block);
	CDW(Out[9]) = SDW(Offset);
	CDW(Out[13]) = SDW(Length);

	m_Socket->Send(&Out, 17);
}

//
// Recv
//

void Client::Recv_Choke(void)
{
	DEBUG_PROTOCOL("%21s - Recv_Choke\n", GetIp());

	m_ClientChoking = true;

	while(m_RequestQueue.GetSize())
	{
		RemoveRequest();
	}

	m_PieceTimeout = 0;
}

void Client::Recv_UnChoke(void)
{
	DWORD i;

	DEBUG_PROTOCOL("%21s - Recv_UnChoke\n", GetIp());

	if(m_ClientChoking == false) return;
	m_ClientChoking = false;

	if(m_Parent->IsPaused()) return;

	m_Timeout = Settings::GetDWord("ClientTimeout") * 60;

	for(i = 0; i < MAX_PIECE_REQUEST_COUNT; i++)
	{
		RequestPiece();
	}
}

void Client::Recv_Interested(void)
{
	DEBUG_PROTOCOL("%21s - Recv_Interested\n", GetIp());

	m_ClientInterested = true;

	m_Parent->QueueClient(this);
}

void Client::Recv_NotInterested(void)
{
	DEBUG_PROTOCOL("%21s - Recv_NotInterested\n", GetIp());

	m_ClientInterested = false;

	Send_Choke();
	m_Parent->UnQueueClient(this);
}

void Client::Recv_Have(void)
{
	BYTE * Data = m_Socket->GetData();
	DWORD Block;
	clock_t t;
	
	Block = SDW(CDW(Data[5]));

	DEBUG_PROTOCOL("%21s - Recv_Have (%d)\n", GetIp(), Block);

	if(Block >= m_Parent->GetBlockCount())
	{
		// Bogus Client
		Kill();
		return;
	}

	m_ClientSpeed += m_Parent->GetBlockSize(Block);
	t = (clock() - m_ClientStarted) / 1000;

	if(m_Blocks[Block] != BLOCKSTATE_COMPLETE)
	{
		m_Blocks[Block] = BLOCKSTATE_COMPLETE;
		m_Parent->IncrementRarity(Block);

		m_BlocksComplete++;
		CheckInterest();
	}
	else
	{
		// Bogus Client
		Kill();
		return;
	}

}

void Client::Recv_BitField(void)
{
	DEBUG_PROTOCOL("%21s - Recv_BitField\n", GetIp());

	BYTE * Data = m_Socket->GetData();
	DWORD Count;
	DWORD i;

	Count = 0;
	for(i = 0; i < m_Parent->GetBlockCount(); i++)
	{
		if(Data[5 + (i / 8)] & (1 << (7 - (i % 8))))
		{
			m_Blocks[i] = BLOCKSTATE_COMPLETE;
			m_Parent->IncrementRarity(i);
			Count++;
		}
	}

	m_BlocksComplete = Count;


	if(Count == m_Parent->GetBlockCount())
	{
		m_Complete = true;
	}

	CheckInterest();
}

void Client::Recv_Request(void)
{
	BYTE * Data = m_Socket->GetData();
	DWORD Block;
	DWORD Offset;
	DWORD Length;
	Request r;

	Block = SDW(CDW(Data[5]));
	Offset = SDW(CDW(Data[9]));
	Length = SDW(CDW(Data[13]));

	if(m_Choking) return;

	DEBUG_PROTOCOL("%21s - Recv_Request (%d, %d, %d)\n", GetIp(), Block, Offset, Length);

	if(Block >= m_Parent->GetBlockCount()) return;	// Bogus Block
	if(Offset >= m_Parent->GetBlockSize(Block)) return;	// Bogus Offset

	r.m_Block = Block;
	r.m_Offset = Offset;
	r.m_Length = Length;

	m_SendQueue.PushBack(r);

//	m_Bars.SetBlock(Block, BARSTATE_REQUESTED);

//	if(m_SendQueue.GetSize() == 1)
	if(m_Socket->GetSendDataLen() == 0)
	{
		OnSend(0);
	}
}

void Client::Recv_Piece(void)
{
	BYTE * Data = m_Socket->GetData();
	DWORD Block;
	DWORD Offset;
	DWORD Length;
	bool Dup;
	
	DWORD i, j;

	Block = SDW(CDW(Data[5]));
	Offset = SDW(CDW(Data[9]));
	Length = SDW(CDW(Data[0])) - 9;

	DEBUG_PROTOCOL("%21s - Recv_Piece (%d, %d, %d)\n", GetIp(), Block, Offset, Length);

	if(Block >= m_Parent->GetBlockCount())			{ DEBUG_PROTOCOL("%21s - Bogus Block\n"); return; }		// Bogus Block
	if(Offset >= m_Parent->GetBlockSize(Block))		{ DEBUG_PROTOCOL("%21s - Bogus Offset\n"); return; }		// Bogus Offset

	Dup = false;
	for(i = Offset / PIECESIZE; i < (Offset + Length) / PIECESIZE; i++)
	{
		if(m_Parent->GetPieceState(Block, i) == BLOCKSTATE_COMPLETE)
		{
			m_Parent->AddRedundantData(PIECESIZE);
			Dup = true;
			DEBUG_PROTOCOL("%21s - Duplicate Piece\n", GetIp());
			break;
		}
	}

	m_PieceTimeout = GetTickCount() + Settings::GetDWord("PieceTimeout") * 1000 * 60;

	if(!Dup)
	{
		m_Parent->SetPieceState(Block, Offset / PIECESIZE, BLOCKSTATE_COMPLETE);
		for(i = Offset / PIECESIZE; i < (Offset + Length) / PIECESIZE; i++)
		{
			m_Parent->SetPieceState(Block, i, BLOCKSTATE_COMPLETE);
		}

		for(i = 0; i < m_RequestQueue.GetSize(); i++)
		{
			if(m_RequestQueue[i].m_Block == Block && m_RequestQueue[i].m_Offset == Offset && m_RequestQueue[i].m_Length == Length)
			{
				m_RequestQueue.Delete(i);
				break;
			}
		}

		m_Parent->WriteData(&Data[13], Block, Offset, Length);

		m_BytesIn += Length;
		m_BytesInDelta += Length;

		if(m_Parent->CheckBlock(Block, true))	// Check Piece Count
		{
			if(m_Parent->CheckBlock(Block, false))
			{
				DEBUG_PROTOCOL("Block %d Complete\n", Block);
				m_Parent->Log("Block %d complete.\n", Block);
				m_Parent->SetBlockState(Block, BLOCKSTATE_COMPLETE);
				m_Parent->SetDownloadingBlockComplete(Block);
				m_Parent->SendHave(Block);
//				m_BytesInDelta += Length;
			}
			else
			{
				DEBUG_PROTOCOL("Block %d Failed\n", Block);
				m_Parent->Log("Block %d failed check.\n", Block);
				m_Parent->AddRedundantData(m_Parent->GetBlockSize(Block));
				m_Parent->SetBlockState(Block, BLOCKSTATE_INCOMPLETE);
				for(i = 0; i < m_Parent->GetBlockPieceCount(Block); i++)
				{
					m_Parent->SetPieceState(Block, i, BLOCKSTATE_INCOMPLETE);
				}
				m_Parent->SetDownloadingBlockIncomplete(Block);
			}
		}
	}

	if(Length > PIECESIZE)
	{
		for(i = 0; i < Length; i += PIECESIZE)
		{
			RequestPiece();
		}
	}else{
		if(m_RequestQueue.GetSize() < 5)
		{
			j = MAX_PIECE_REQUEST_COUNT - m_RequestQueue.GetSize();

			for(i = 0; i < j; i++)
			{
				RequestPiece();
			}
		}
	}
}

void Client::Recv_Cancel(void)
{
	BYTE * Data = m_Socket->GetData();
	DWORD Block;
	DWORD Offset;
	DWORD Length;
	DWORD i;

	Block = SDW(CDW(Data[5]));
	Offset = SDW(CDW(Data[9]));
	Length = SDW(CDW(Data[13]));

	DEBUG_PROTOCOL("%21s - Recv_Cancel (%d, %d, %d)\n", GetIp(), Block, Offset, Length);

	for(i = 0; i < m_SendQueue.GetSize(); i++)
	{
		if(m_SendQueue[i].m_Block == Block && m_SendQueue[i].m_Length == Length && m_SendQueue[i].m_Offset == Offset)
		{
			m_SendQueue.Delete(i);
			break;
		}
	}
}

DWORD Client::GetBytesInDelta(void)
{
	return m_BytesInDelta;
}

DWORD Client::GetBytesOutDelta(void)
{
	return m_BytesOutDelta;
}

DWORD Client::GetBpsIn(void)
{
	return m_BpsIn;
}

DWORD Client::GetBpsOut(void)
{
	return m_BpsOut;
}

DWORD Client::GetAvgBpsIn(void)
{
	return m_AvgBpsIn.GetAverage();
}

DWORD Client::GetAvgBpsOut(void)
{
	return m_AvgBpsOut.GetAverage();
}

bool Client::IsSeed(void)
{
	if(m_Complete) return true;
	return false;
}

bool Client::IsLeecher(void)
{
	if(m_State != CLIENTSTATE_ACTIVE) return false;
	if(!m_Complete) return true;
	return false;
}

void Client::SetMaxUpload(DWORD Rate)
{
	m_Socket->SetMaxOut(Rate);
}

float Client::GetAverage(void)
{
	return (float)m_BlocksComplete / (float)m_Parent->GetBlockCount();
}
