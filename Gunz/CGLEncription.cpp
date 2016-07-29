/**********************************************************************
  GunZ Launcher Enctiption

   file : CGLEncription.cpp
   desc : 건즈 실행에 필요한 시리얼 키를 암호화 한다.
***********************************************************************/



#include "stdafx.h"
#include "CGLEncription.h"
#include "MCrypt.h"
#include <Mmsystem.h>
#include <math.h>

#define DATA_MSG_INTERNATIONAL		"I_hate_hacker"
#define DATA_MSG_LAUNCHDEV			"I_love_MAIET"
#define DATA_MSG_TEST				"Find_Me"

/*
// CreateSerialKey
bool CGLEncription::CreateSerialKey()
{
	// Create random seed
	unsigned char szSerialKey[ MAX_BUF];
	srand( (unsigned)time( NULL));
	for ( int i = 0;  i < MAX_BUF;  i++)
		szSerialKey[ i] = (unsigned char)( rand() % 256);
    

	// Get limited time
	DWORD dwLimitedTime = GetGlobalTimeMS() + 60000;
	char szTime[ 20];
	memset( szTime, 0, sizeof(szTime));
	sprintf_safe( szTime, "%X", dwLimitedTime);


	// Get Disk serial number
	char szDiskLabel[ 128];
	char szFileSysName[ 128];
	DWORD dwDiskSerial;
	DWORD dwMaxNameLength = 0; 
	DWORD dwFileSystemFlags = 0;
	char szDirectory[ 256];
	GetCurrentDirectory( sizeof( szDirectory), szDirectory);
	szDirectory[ 3] = 0;
	GetVolumeInformation( szDirectory, szDiskLabel, sizeof(szDiskLabel)-1, &dwDiskSerial,
														&dwMaxNameLength,
														&dwFileSystemFlags,
														szFileSysName,
														sizeof( szFileSysName) - 1);
	char szDiskSerial[ 20];
	memset( szDiskSerial, 0, sizeof(szDiskSerial));
	sprintf_safe( szDiskSerial, "%X", dwDiskSerial);


	// Set Data
	char szData[ 20];
	memset( szData, 0, sizeof(szData));
	strcpy_safe( szData, DATA_MSG_LAUNCHDEV);


	// Mixing data
	int nPos = szSerialKey[ MAX_BUF - 1] % 10;
	for ( i = 0;  i < 20;  i++)
	{
		int nAbsPos = (int)((i / 10) * 10) + nPos;

		szSerialKey[      nAbsPos] = (unsigned char)szTime[ nAbsPos];
		szSerialKey[ 20 + nAbsPos] = (unsigned char)szDiskSerial[ nAbsPos];
		szSerialKey[ 40 + nAbsPos] = (unsigned char)szData[ nAbsPos];

		nPos = ( nPos + 7) % 10;
	}


	// Get CRC
	szSerialKey[ MAX_BUF - 2] = 0;
	for ( i = 0;  i < 60;  i++)
		szSerialKey[ MAX_BUF - 2] += szSerialKey[ i];
	

	// Encrypt
	MSeed cSeed;
	unsigned char byKey[ SEED_USER_KEY_LEN];
	memset( byKey, 0, sizeof( byKey));
	unsigned char byIV[ SEED_BLOCK_LEN];
	memset( byIV, 0, sizeof( byIV));
	for ( int i = 0;  i < 20;  i++)
	{
		if ( i < SEED_USER_KEY_LEN)
			byKey[ i] = szSerialKey[ 200 + i];
		if ( i < SEED_BLOCK_LEN)
	        byIV[ i]  = szSerialKey[ 220 + i];
	}
	if ( !cSeed.InitKey( byKey, byIV))
		return false;

	DWORD dwLength;
	unsigned char szCriptSerialKey[ MAX_BUF];
	if ( !cSeed.Encrypt( szSerialKey, 60, szCriptSerialKey, &dwLength))
		return false;

	memcpy( szSerialKey, szCriptSerialKey, dwLength);
	szSerialKey[ MAX_BUF - 3] = (unsigned char)dwLength;


	// Write to clip board
	bool bRetVal = false;
	if ( ::OpenClipboard( NULL))
	{
		HGLOBAL hMem;
		hMem = GlobalAlloc( GMEM_DDESHARE, MAX_BUF + 1);
		char* buffer = (char*)GlobalLock( hMem);
		memcpy( buffer, szSerialKey, MAX_BUF);
		GlobalUnlock( hMem);

		bRetVal = ( ::SetClipboardData( CF_GUNZLAUNCHER, hMem) == NULL) ? false : true;

		::CloseClipboard();
	}


	return bRetVal;
}
*/

// atodw
DWORD atodw( const char* szValue)
{
	int nCount = 0;
	DWORD dwValue = 0;

	while ( *(szValue + nCount) != 0)
	{
		dwValue *= 16;

		if ( ( *(szValue + nCount) >= '0') && ( *(szValue + nCount) <= '9'))
			dwValue += *(szValue + nCount) - '0';
		else
			dwValue += *(szValue + nCount) - 'A' + 10;

		nCount++;
	}

	return dwValue;
}


// GetEncription
int CGLEncription::Decription( void)
{
	unsigned char szSerialKey[ MAX_BUF];
	memset( szSerialKey, 0, MAX_BUF);


	// Read from clipboard
	if ( ! ::OpenClipboard( NULL))
	{
		mlog( "Serialkey error : 201\n");
		return false;
	}

	if ( !IsClipboardFormatAvailable( CF_GUNZLAUNCHER))
	{
		mlog( "Serialkey error : 202\n");
		return false;
	}

	memcpy( szSerialKey, (unsigned char*)GetClipboardData( CF_GUNZLAUNCHER), MAX_BUF);

	::CloseClipboard();


	// Variables;
	char szTime[ 20];
	memset( szTime, 0, sizeof( szTime));

	char szDiskSerial[ 20];
	memset( szDiskSerial, 0, sizeof( szDiskSerial));

	char szData[ 20];
	memset( szData, 0, sizeof( szData));


	// Decrypt
	MSeed cSeed;
	unsigned char byKey[ SEED_USER_KEY_LEN];
	memset( byKey, 0, sizeof( byKey));
	unsigned char byIV[ SEED_BLOCK_LEN];
	memset( byIV, 0, sizeof( byIV));
	for ( int i = 0;  i < 20;  i++)
	{
		if ( i < SEED_USER_KEY_LEN)
			byKey[ i] = szSerialKey[ 200 + i];
		if ( i < SEED_BLOCK_LEN)
	        byIV[ i]  = szSerialKey[ 220 + i];
	}
	if ( !cSeed.InitKey( byKey, byIV))
	{
		mlog( "Serialkey error : 203\n");
		return false;
	}

	DWORD dwLength;
	unsigned char szCryptSerialKey[ MAX_BUF];
	if ( !cSeed.Decrypt( szSerialKey, szSerialKey[ MAX_BUF - 3], szCryptSerialKey, &dwLength))
	{
		mlog( "Serialkey error : 204\n");
		return false;
	}

	memcpy( szSerialKey, szCryptSerialKey, dwLength);



	// Check CRC
	unsigned char byCRC = 0;
	for ( int i = 0;  i < 60;  i++)
		byCRC += szSerialKey[ i];

	if ( byCRC != szSerialKey[ MAX_BUF - 2])
	{
		mlog( "Serialkey error : 205\n");
		return false;
	}


	// Decoding
	int nPos = szSerialKey[ 60] % 10;
	for ( int i = 0;  i < 20;  i++)
	{
		int nAbsPos = (int)((i / 10) * 10) + nPos;

		szTime[ nAbsPos]        = (char)szSerialKey[      nAbsPos];
		szDiskSerial[ nAbsPos]  = (char)szSerialKey[ 20 + nAbsPos];
		szData[ nAbsPos]        = (char)szSerialKey[ 40 + nAbsPos];

		nPos = ( nPos + 3) % 10;
	}


	// Check time
	DWORD dwTime = atodw( szTime);
	DWORD dwCurrTime = GetGlobalTimeMS();
	if ( (DWORD)atodw( szTime) < GetGlobalTimeMS())
	{
		mlog( "Serialkey error : 206\n");
		return false;
	}


	// Check serial number
	char szDiskLabel[ 128];
	char szFileSysName[ 128];
	DWORD dwDiskSerial;
	DWORD dwMaxNameLength = 0; 
	DWORD dwFileSystemFlags = 0;
	char szDirectory[ 256];
	GetCurrentDirectory( sizeof( szDirectory), szDirectory);
	szDirectory[ 3] = 0;
	GetVolumeInformation( szDirectory, szDiskLabel, sizeof(szDiskLabel)-1, &dwDiskSerial,
														&dwMaxNameLength,
														&dwFileSystemFlags,
														szFileSysName,
														sizeof( szFileSysName) - 1);

	if ( (DWORD)atodw( szDiskSerial) != dwDiskSerial)
	{
		mlog( "Serialkey error : 207\n");
		return false;
	}


	// Check Data
	if ( strcmp( szData, DATA_MSG_INTERNATIONAL) == 0)
		return GLE_LAUNCH_INTERNATIONAL;
	else if ( strcmp( szData, DATA_MSG_LAUNCHDEV) == 0)
		return GLE_LAUNCH_DEVELOP;
	else if ( strcmp( szData, DATA_MSG_TEST) == 0)
		return GLE_LAUNCH_TEST;


	mlog( "Serialkey error : 208\n");
	return false;
}
