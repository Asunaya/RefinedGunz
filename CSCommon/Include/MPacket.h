#ifndef _MPACKET_H
#define _MPACKET_H

#include "MCommand.h"
#include "MCommandManager.h"

#define MAX_PACKET_SIZE			16384		// 최대 패킷 크기는 16k

#define MSG_COMMAND	1000

#define MSGID_REPLYCONNECT	10
#define MSGID_RAWCOMMAND	100		// 암호화 안된 커맨드
#define MSGID_COMMAND		101		// 암호화된 커맨드
//#define MSGID_HSHIELDCOMMAND 102	// 핵실드 암호화된 커맨드




class MPacketCrypter;

#pragma pack(1)

struct MPacketHeader
{
	unsigned short nMsg;
	unsigned short nSize;
	unsigned short nCheckSum;

	MPacketHeader() { nMsg=MSG_COMMAND; nSize=0; nCheckSum=0; }
	int CalcPacketSize(MPacketCrypter* pCrypter);		// 만약 암호화되어있을 경우 패킷의 크기는 이 함수로만 알 수 있다.
														// nSize 변수에는 암호화된 값이 들어있을 수 있음
};


struct MReplyConnectMsg : public MPacketHeader
{
	unsigned int	nHostHigh;
	unsigned int	nHostLow;
	unsigned int	nAllocHigh;
	unsigned int	nAllocLow;
	unsigned int	nTimeStamp;
};

struct MCommandMsg : public MPacketHeader
{
	char	Buffer[1];
};


#pragma pack()

// Tiny CheckSum for MCommandMsg
inline unsigned short MBuildCheckSum(MPacketHeader* pPacket, int nPacketSize)
{
	int nStartOffset = sizeof(MPacketHeader);
	BYTE* pBulk = (BYTE*)pPacket;
	//int nPacketSize = min(65535, pPacket->nSize);
	nPacketSize = min(65535, nPacketSize);

	unsigned long nCheckSum = 0;
	for (int i=nStartOffset; i<nPacketSize; i++) {
		nCheckSum += pBulk[i];
	}
	nCheckSum -= (pBulk[0]+pBulk[1]+pBulk[2]+pBulk[3]);
	unsigned short nShortCheckSum = HIWORD(nCheckSum) + LOWORD(nCheckSum);
	return nShortCheckSum;
}


#endif
