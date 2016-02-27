#include "stdafx.h"
//#define _WIN32_WINNT	0x0400
#include <winsock2.h>
#include "ZSecurity.h"
#include "MDataChecker.h"
//#include "MProcessController.h"
#include "MZFileSystem.h"
#include "MMatchItem.h"
#include "ZCharacter.h"
#include "ZApplication.h"
#include "ZItemDesc.h"
#include "ZModule_HPAP.h"

#include <list>

#define HACKFINDERKEY	0x05D4
class MHackWindowFinder {
protected:
	HWND			m_hWnd;
	bool			m_bResult;
	list<string>	m_HackList;

public:
	MHackWindowFinder()	{ m_bResult=false;	m_hWnd=NULL; }
	bool GetResult()	{ return m_bResult; }
	HWND GetHWND()		{ return m_hWnd; }

	static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {
		char szWinText[_MAX_DIR] = "";
		GetWindowText(hWnd, szWinText, _MAX_DIR);

		// XOR Encoding
		string strWindow = Encode(szWinText);

		if (strlen(strWindow.c_str()) <= 0)
			return TRUE;

		MHackWindowFinder* pFinder = (MHackWindowFinder*)lParam;
		for (list<string>::iterator i=pFinder->m_HackList.begin(); i!=pFinder->m_HackList.end(); i++) {
			string strHack = (*i);
			if (StrStrI(strWindow.c_str(), strHack.c_str()) != 0) {
				pFinder->m_bResult = true;
				pFinder->m_hWnd = hWnd;
				return FALSE;
			}
		}
		return TRUE;
	}
	static string Encode(const char* pszString) {
		// XOR Encoding
		char szBuf[256];
		sprintf(szBuf, pszString);
		for (int i=0; i<256; i++) {
			if (szBuf[i] == NULL) break;
			szBuf[i] = szBuf[i] ^ HACKFINDERKEY;
		}
		return szBuf;
	}
	static void EncodeTest(const char* pszString) {
		char szLog[256];
		string strEncoded = Encode(pszString);
		sprintf(szLog, "ENCODETEST: [%s] -> [%s] \n", pszString, strEncoded.c_str());
		OutputDebugString(szLog);
	}
	void AddHackList(const string& strHackName, bool bEncoded=false) {
		if (bEncoded)
			m_HackList.push_back(strHackName.c_str());	
		else
			m_HackList.push_back(Encode(strHackName.c_str()));
	}
	bool Find() {
		m_bResult = false;
		m_hWnd = NULL;

		EnumWindows(EnumWindowsProc, (LPARAM)this);

		return GetResult();
	}
};

unsigned long ZGetMZFileChecksum(const char* pszFileName) 
{
	MZFileSystem* pFS = ZGetFileSystem();
	MZFile mzf;
	if (mzf.Open(pszFileName, pFS) == false)
		if(!mzf.Open(pszFileName)) 
			return 0;

	char* pBuffer = NULL;
	int nLen = mzf.GetLength();
	pBuffer = new char[mzf.GetLength()+1];
	pBuffer[nLen] = 0;
	mzf.Read(pBuffer, nLen);
	mzf.Close();

	unsigned long nChecksum = 0;
	for (int i=0; i<nLen; i++)
		nChecksum += pBuffer[i];

	delete pBuffer;
	return nChecksum;
}

bool ZCheckFileHack() // 사용하지 않는 펑션
{
	return false;

	unsigned long nChecksum = ZGetMZFileChecksum(FILENAME_ZITEM_DESC);

#ifndef _PUBLISH
	mlog("ZITEM Checksum = %u", nChecksum);
	return false;	// false == NO HACK
#endif

#ifdef _PUBLISH
	if (nChecksum != 6739119)
		return true;
	else
		return false;
#else
	return false;	// No Check File
#endif
}

void ZSetupDataChecker_Global(MDataChecker* pDataChecker)
{
	pDataChecker->Clear();

	//// 아이템능력치 정보
	MMatchItemDescMgr* pItemMgr = MMatchItemDescMgr::GetInstance();
	for (MMatchItemDescMgr::iterator i=pItemMgr->begin(); i!=pItemMgr->end(); i++) {
		MMatchItemDesc* pItem = (*i).second;
		pDataChecker->AddCheck((BYTE*)&pItem->m_nDamage, sizeof(int));
		pDataChecker->AddCheck((BYTE*)&pItem->m_nDelay, sizeof(int));
		pDataChecker->AddCheck((BYTE*)&pItem->m_nMagazine, sizeof(int));
		pDataChecker->AddCheck((BYTE*)&pItem->m_nMaxBullet, sizeof(int));
		pDataChecker->AddCheck((BYTE*)&pItem->m_nReloadTime, sizeof(int));
		pDataChecker->AddCheck((BYTE*)&pItem->m_nHP, sizeof(int));
		pDataChecker->AddCheck((BYTE*)&pItem->m_nAP, sizeof(int));
	}
}

void ZSetupDataChecker_Game(MDataChecker* pDataChecker)
{
	pDataChecker->Clear();

	//// 캐릭터 HP,AP 정보 
	ZModule_HPAP* pCharHPAP= (ZModule_HPAP*)ZApplication::GetGame()->m_pMyCharacter->GetModule(ZMID_HPAP);
	pDataChecker->AddCheck((BYTE*)pCharHPAP->GetHPPointer(), sizeof(float));
	pDataChecker->AddCheck((BYTE*)pCharHPAP->GetAPPointer(), sizeof(float));
	/*
	ZCharacterStatus* pCharStatus = ZApplication::GetGame()->m_pMyCharacter->GetCharacterStatus();
	pDataChecker->AddCheck((BYTE*)pCharStatus->GetHPPointer(), sizeof(float));
	pDataChecker->AddCheck((BYTE*)pCharStatus->GetAPPointer(), sizeof(float));
	*/
	
	//// 캐릭터 장비 아이템 잔탄량 정보
	ZCharacterItem* pCharItem = ZApplication::GetGame()->m_pMyCharacter->GetItems();
	ZItem* pPrimaryItem = pCharItem->GetItem(MMCIP_PRIMARY);
	if (pPrimaryItem) {
		pDataChecker->AddCheck((BYTE*)pPrimaryItem->GetBulletPointer(), sizeof(int));
		pDataChecker->AddCheck((BYTE*)pPrimaryItem->GetAMagazinePointer(), sizeof(int));
	}
	ZItem* pSecondaryItem = pCharItem->GetItem(MMCIP_SECONDARY);
	if (pPrimaryItem) {
		pDataChecker->AddCheck((BYTE*)pSecondaryItem->GetBulletPointer(), sizeof(int));
		pDataChecker->AddCheck((BYTE*)pSecondaryItem->GetAMagazinePointer(), sizeof(int));
	}
	ZItem* pCustom1Item = pCharItem->GetItem(MMCIP_CUSTOM1);
	if (pPrimaryItem) {
		pDataChecker->AddCheck((BYTE*)pCustom1Item->GetBulletPointer(), sizeof(int));
		pDataChecker->AddCheck((BYTE*)pCustom1Item->GetAMagazinePointer(), sizeof(int));
	}
	ZItem* pCustom2Item = pCharItem->GetItem(MMCIP_CUSTOM2);
	if (pPrimaryItem) {
		pDataChecker->AddCheck((BYTE*)pCustom2Item->GetBulletPointer(), sizeof(int));
		pDataChecker->AddCheck((BYTE*)pCustom2Item->GetAMagazinePointer(), sizeof(int));
	}
}
