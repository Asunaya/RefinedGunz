#include "stdafx.h"
#include "MCrypt.h"


MSeed::MSeed()
{
}

MSeed::~MSeed()
{

}

bool MSeed::InitKey(BYTE Key[SEED_USER_KEY_LEN], BYTE IV[SEED_BLOCK_LEN])
{
	memcpy(m_Key, Key, sizeof(m_Key));
	memcpy(m_IV, IV, sizeof(m_IV));
	
	SEED_SetAlgInfo(AI_CFB, AI_PKCS_PADDING, IV, &m_AlgInfo);

	RET_VAL	ret = SEED_KeySchedule(Key, SEED_USER_KEY_LEN, &m_AlgInfo);
	if( ret!=CTR_SUCCESS ) return false;

	return true;
}

bool MSeed::Encrypt(BYTE* in, DWORD nInLen, BYTE* out, DWORD* pnOutLen, SEED_ALG_INFO* pAlgInfo)
{
	RET_VAL	ret;

	SEED_EncInit(pAlgInfo);

	DWORD DstLen = nInLen;
	ret = SEED_EncUpdate(pAlgInfo, in, nInLen, out, &DstLen);
	if( ret!=CTR_SUCCESS ) return false;

	*pnOutLen = DstLen;
	ret = SEED_EncFinal(pAlgInfo, (out + DstLen), &DstLen);
	if( ret!=CTR_SUCCESS ) return false;

	*pnOutLen += DstLen;

	return true;
}

bool MSeed::Decrypt(BYTE* in, DWORD nInLen, BYTE* out, DWORD* pnOutLen, SEED_ALG_INFO* pAlgInfo)
{
	RET_VAL	ret;
	SEED_DecInit(pAlgInfo);

	DWORD DstLen = nInLen;
	ret = SEED_DecUpdate(pAlgInfo, in, nInLen, out, &DstLen);
	if( ret!=CTR_SUCCESS ) return false;

	*pnOutLen = DstLen;
	ret = SEED_DecFinal(pAlgInfo, out + *pnOutLen, &DstLen);
	if( ret!=CTR_SUCCESS ) return false;

	*pnOutLen += DstLen;

	return true;
}

bool MSeed::Encrypt(BYTE* in, DWORD nInLen, BYTE* out, DWORD* pnOutLen)
{
	return MSeed::Encrypt(in, nInLen, out, pnOutLen, &m_AlgInfo);
}

bool MSeed::Decrypt(BYTE* in, DWORD nInLen, BYTE* out, DWORD* pnOutLen)
{
	return MSeed::Decrypt(in, nInLen, out, pnOutLen, &m_AlgInfo);
}




