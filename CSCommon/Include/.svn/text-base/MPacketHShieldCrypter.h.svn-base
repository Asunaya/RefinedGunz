#pragma once
#include "../HShield/HsCryptLib.h"
#include "MPacket.h"

class MPacketHShieldCrypter
{
public:
	MPacketHShieldCrypter(void);
	~MPacketHShieldCrypter(void);

	static DWORD Init();
	static DWORD Encrypt(PBYTE pbyInput, UINT nInLength);
	static DWORD Decrypt(PBYTE pbyInput, UINT nInLength);
	static DWORD Decrypt(PBYTE pbyInput, UINT nInLength, PBYTE pbyOutput);

private:
	static HSCRYPT_KEYINFO m_HsKeyInfo;
	static unsigned char m_OutputBuf[MAX_PACKET_SIZE];
};
