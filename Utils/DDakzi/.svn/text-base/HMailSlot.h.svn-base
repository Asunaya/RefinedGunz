#ifndef HMAILSLOT_H

#define HMAILSLOT_H





#include <windows.h>





#define MAX_DOMAIN_LEN			128

#define MAX_SLOTNAME_LEN		128

#define MAX_MAILSLOTPATH_LEN	256





class HMailSlot {

	HANDLE m_hSlot;

	char m_szDomain[MAX_DOMAIN_LEN];	// : [. | * | ComputerName | DomainName]

	char m_szSlotName[MAX_SLOTNAME_LEN];

	char m_szMailSlotPath[MAX_MAILSLOTPATH_LEN];



public:

	char* GetMailSlotPath() { return m_szMailSlotPath; }



	bool CreateSlot(char* szDomain, char* szSlotName);

	void DestroySlot();

	DWORD PeekSlot();

	bool ReadSlot(char* pszOutBuffer, DWORD* pdwBufferSize);	// No Auto NULL

	bool WriteSlot(char* szMailSlotPath, char* szInBuffer, DWORD dwInBufferSize);		// No Auto NULL

};





#endif