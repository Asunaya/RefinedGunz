//================================================================================================|
// NMCrypt.h : Netmarble Crypt Library Header
//================================================================================================|

#ifndef   __NM_CRYPT_H__
#define   __NM_CRYPT_H__


#define CRYPT_SUCCESS      0
#define	CRYPT_INPUT_FAIL  -1
#define	CRYPT_OUTPUT_FAIL -2

void EncryptString( const char *psSource, const char *psKey, char *psTarget );
int  DecryptString( const char *psSource, const char *psKey, char *psTarget );

bool SetCryptKey( const char *psStr );
bool GetCryptKey( char *pStr, unsigned int uiMaxLen, bool bClearKey = true );



//#pragma comment ( lib, "NMCrypt.lib"  )




#endif // __NM_CRYPT_H__