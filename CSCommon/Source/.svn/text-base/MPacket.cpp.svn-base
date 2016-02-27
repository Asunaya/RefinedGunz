#include "stdafx.h"
#include "MPacket.h"
//#include "Msg.h"


// CSCommon에는 #define을 사용하지 마세요.


//#ifdef _HSHIELD
//	#include "MPacketHShieldCrypter.h"
//#else
	#include "MPacketCrypter.h"
//#endif

int MPacketHeader::CalcPacketSize(MPacketCrypter* pCrypter)
{
	unsigned short nPacketSize = 0;

	if (nMsg == MSGID_COMMAND)
	{
//#ifdef _HSHIELD
//		MPacketHShieldCrypter::Decrypt((PBYTE)&(nSize), sizeof(unsigned short), (PBYTE)&nPacketSize);
//#else
		if (pCrypter)
		{
			pCrypter->Decrypt((char*)(&nSize), sizeof(unsigned short), (char*)&nPacketSize, sizeof(unsigned short));
		}
//#endif
	}
	else
	{
		nPacketSize = nSize;
	}


	return (int)nPacketSize;
}
