#ifndef _MCRYPT_H
#define _MCRYPT_H

#include "seed.h"

/// SEED를 사용하는 암호화
class MSeed
{
private:
	SEED_ALG_INFO	m_AlgInfo;	
	BYTE m_Key[SEED_USER_KEY_LEN];
	BYTE m_IV[SEED_BLOCK_LEN];
public:
	MSeed();
	virtual ~MSeed();
	bool InitKey(BYTE Key[SEED_USER_KEY_LEN], BYTE IV[SEED_BLOCK_LEN]);
	bool Encrypt(BYTE* in, DWORD nInLen, BYTE* out, DWORD* pnOutLen);
	bool Decrypt(BYTE* in, DWORD nInLen, BYTE* out, DWORD* pnOutLen);
	SEED_ALG_INFO* GetAlgInfo() { return &m_AlgInfo; }

	static bool Encrypt(BYTE* in, DWORD nInLen, BYTE* out, DWORD* pnOutLen, SEED_ALG_INFO* pAlgInfo);
	static bool Decrypt(BYTE* in, DWORD nInLen, BYTE* out, DWORD* pnOutLen, SEED_ALG_INFO* pAlgInfo);
};





#endif