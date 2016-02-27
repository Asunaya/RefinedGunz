#pragma once

#include "MWindowFinder.h"


//// °¢Á¾ º¸¾È ¹× ÇØÅ·¹æÁöÄÚµå ////

class MDataChecker;

__forceinline bool ZCheckHackProcess();
unsigned long ZGetMZFileChecksum(const char* pszFileName);
bool ZCheckFileHack();
void ZSetupDataChecker_Global(MDataChecker* pDataChecker);
void ZSetupDataChecker_Game(MDataChecker* pDataChecker);


// ------ inline


__forceinline bool ZCheckHackProcess()
{
	bool bHack = false;

#ifdef _PUBLISH

/*	MHackWindowFinder HackFinder;
//	HackFinder.EncodeTest("SpeederXP");
//	HackFinder.EncodeTest("ArtMoney");
//	HackFinder.EncodeTest("TSearch");
//	HackFinder.EncodeTest("GameHack");
//	HackFinder.EncodeTest("Memory Doctor");
	HackFinder.AddHackList("‡¤±±°±¦Œ?", true);
	HackFinder.AddHackList("•¦ ™»º±­", true);
	HackFinder.AddHackList("‡±µ¦·¼", true);
	HackFinder.AddHackList("“µ¹±œµ·¿", true);
	HackFinder.AddHackList("™±¹»¦­?»· »?", true);

	if (HackFinder.Find())
		bHack = true;
*/

	// 0x4b ·Î xor ÈÄ ¿ª¼øÀ¸·Î
	char *cheatProgramNames[] = { 
		// ÇØÅ·Åøµé
			"\x9\x1b\x13\x39\x2e\x2f\x2e\x2e\x3b\x18", // SpeederXP
			"\x8\x32\x2e\x25\x24\x6\x3f\x39\xa", // ArtMoney
			"\x7\x23\x28\x39\x2a\x2e\x18\x1f", // TSearch
			"\x8\x20\x28\x2a\x3\x2e\x26\x2a\xc", // GameHack
			"\xd\x39\x24\x3f\x28\x24\xf\x6b\x32\x39\x24\x26\x2e\x6", // Memory Doctor
			"\x3\xe\x1b\x1c", // WPE
			"\xc\x2e\x25\x22\x2c\x25\xe\x6b\x3f\x2a\x2e\x23\x8", // Cheat Engine
			"\x7\xc\x9\xf\x12\x7\x7\x4", // OLLYDBG
			"\xe\x24\x22\x2f\x3e\x3f\x18\x6b\x39\x2e\x25\x22\x2a\x39\x1f", // Trainer Studio
			"\x10\x25\x24\x22\x3f\x2a\x2e\x39\x8\x6b\x39\x2e\x25\x22\x2a\x39\x1f", // Trainer Creation
			"\xd\x39\x2e\x20\x2a\x6\x6b\x39\x2e\x25\x22\x2a\x39\x1f", // Trainer Maker
		// À¯ÀúµéÀÌ ¸¸µç Åø
			"\xb\x2f\x22\x0\x2e\x23\x1f\x32\x27\x27\x22\x9", // BillyTheKid
			"\x11\x39\x2e\x25\x22\x2a\x39\x1f\x32\x39\x24\x26\x2e\x6\x26\x39\x2d\x1f", // TfrmMemoryTrainer		
	};

#define DECODESTRING(_dest,_source) \
	{ for(int __i=0; __i<_source[0];__i++) { \
	_dest[__i]=((unsigned char)_source[_source[0]-__i]) ^ 0x4b; } \
	_dest[_source[0]]=0; }
	

	MWindowFinder WindowFinder;

	char szTemp[256];

	for(int i=0;i<sizeof(cheatProgramNames)/sizeof(char*);i++) {
		DECODESTRING(szTemp,cheatProgramNames[i]);
		if(WindowFinder.Find(szTemp))
			return true;
	}

	/*
	char *kernelStrings[] = {
	"\xc\x27\x27\x2f\x65\x79\x78\x27\x2e\x25\x39\x2e\x20", // kernel32.dll
	"\x11\x3f\x25\x2e\x38\x2e\x39\x1b\x39\x2e\x2c\x2c\x3e\x29\x2e\xf\x38\x2" // IsDebuggerPresent
	};

	typedef BOOL * (__stdcall *ISDEBUGGERPRESENTTYPE)();
	static ISDEBUGGERPRESENTTYPE isDebuggerPresent = NULL;
	if(	NULL == isDebuggerPresent ) {
		DECODESTRING(szTemp,kernelStrings[0]);
		HMODULE hKernel32 = LoadLibrary( szTemp );
		if (!hKernel32) return true;

		DECODESTRING(szTemp,kernelStrings[1]);
		isDebuggerPresent = (ISDEBUGGERPRESENTTYPE) GetProcAddress(hKernel32, szTemp);
		FreeLibrary(hKernel32);
	}
	if (isDebuggerPresent && isDebuggerPresent())
		bHack = true;
	*/
	if (IsDebuggerPresent())
		bHack = true;

#endif

	return bHack;
}
