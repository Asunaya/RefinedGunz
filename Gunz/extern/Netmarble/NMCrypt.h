// NMCrypt.h : header file
//

#if !defined(_NMCRYPT_H_)
#define _NMCRYPT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define CRYPT_SUCCESS			 0
#define	CRYPT_INPUT_FAIL		-1
#define	CRYPT_OUTPUT_FAIL		-2

void EncryptString(const char *psSource, const char *psKey, char *psTarget);
int DecryptString(const char *psSource, const char *psKey, char *psTarget);	

bool SetNMClipData(const char *str, const char *key);
bool GetNMClipData(char *pstr, unsigned int maxlen, const char *key, bool bClearData=true);

//#pragma comment ( lib, "NMCrypt.lib"  )



#endif // !defined(_NMCRYPT_H_)
