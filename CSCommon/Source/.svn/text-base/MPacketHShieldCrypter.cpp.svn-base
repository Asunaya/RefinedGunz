#include "stdafx.h"
#include "MPacketHShieldCrypter.h"

HSCRYPT_KEYINFO MPacketHShieldCrypter::m_HsKeyInfo;
unsigned char MPacketHShieldCrypter::m_OutputBuf[MAX_PACKET_SIZE];

MPacketHShieldCrypter::MPacketHShieldCrypter(void)
{
}

MPacketHShieldCrypter::~MPacketHShieldCrypter(void)
{
}

DWORD MPacketHShieldCrypter::Init()
{
	memcpy(m_HsKeyInfo.byInitKey, "_9+a4%b7&d2$p1q", HSCRYPTLIB_INITKEY_SIZE);
	return _HsCrypt_InitCrypt(&m_HsKeyInfo);
}

DWORD MPacketHShieldCrypter::Encrypt(PBYTE pbyInput, UINT nInLength)
{
	DWORD ret = _HsCrypt_GetEncMsg(pbyInput, nInLength, m_HsKeyInfo.AesEncKey, m_OutputBuf, nInLength);
	CopyMemory(pbyInput, m_OutputBuf, nInLength);

	return ret;
}

DWORD MPacketHShieldCrypter::Decrypt(PBYTE pbyInput, UINT nInLength)
{
	DWORD ret = _HsCrypt_GetDecMsg(pbyInput, nInLength, m_HsKeyInfo.AesDecKey, m_OutputBuf, nInLength);
	CopyMemory(pbyInput, m_OutputBuf, nInLength);

	return ret;
}

DWORD MPacketHShieldCrypter::Decrypt(PBYTE pbyInput, UINT nInLength, PBYTE pbyOutput)
{
	return _HsCrypt_GetDecMsg(pbyInput, nInLength, m_HsKeyInfo.AesDecKey, pbyOutput, nInLength);
}
