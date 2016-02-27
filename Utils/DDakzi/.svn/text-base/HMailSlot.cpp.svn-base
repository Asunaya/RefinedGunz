#include "stdafx.h"
#include "HMailSlot.h"


bool HMailSlot::CreateSlot(char* szDomain, char* szSlotName)
{
	strcpy(m_szDomain, szDomain);
	strcpy(m_szSlotName, szSlotName);
	wsprintf(m_szMailSlotPath, "\\\\%s\\mailslot\\%s", m_szDomain, m_szSlotName);

	m_hSlot = CreateMailslot(m_szMailSlotPath, 
				0,								// No max msg size
				MAILSLOT_WAIT_FOREVER,			// no timeout for read
				(LPSECURITY_ATTRIBUTES)NULL);
	if (m_hSlot == INVALID_HANDLE_VALUE) {
		return false;
	} else {
		return true;
	}
}

void HMailSlot::DestroySlot()
{
	if (m_hSlot != NULL)
		CloseHandle(m_hSlot);
}

DWORD HMailSlot::PeekSlot()
{
	DWORD dwMsgCount		= 0;	// number of messsage
	DWORD dwMsgSize			= 0;	// sizeof next message

	if ( GetMailslotInfo(m_hSlot, (LPDWORD)NULL, 
			&dwMsgSize, &dwMsgCount, (LPDWORD)NULL) == FALSE )
		return 0;
	
	if (dwMsgCount == 0)
		return 0;

	return dwMsgSize;
}

bool HMailSlot::ReadSlot(char* pszOutBuffer, DWORD* pdwBufferSize)
{
	LPSTR pszBuffer;
	DWORD dwMsgCount		= 0;	// number of messsage
	DWORD dwMsgSize			= 0;	// sizeof next message
	DWORD dwMsgReceivedSize	= 0;

	if ( GetMailslotInfo(m_hSlot, (LPDWORD)NULL, 
			&dwMsgSize, &dwMsgCount, (LPDWORD)NULL) == FALSE )
		return false;

	if (dwMsgCount == 0)
		return false;

	pszBuffer = (LPSTR)GlobalAlloc(GPTR, dwMsgSize);
	if ( ReadFile(m_hSlot, pszBuffer, 
				dwMsgSize, &dwMsgReceivedSize, (LPOVERLAPPED)NULL) == FALSE ) {
		GlobalFree((HGLOBAL)pszBuffer);
		return false;
	}

	if (*pdwBufferSize < dwMsgReceivedSize) {
		memcpy(pszOutBuffer, pszBuffer, *pdwBufferSize);
		*pdwBufferSize = dwMsgReceivedSize;
	} else {
		memcpy(pszOutBuffer, pszBuffer, dwMsgReceivedSize);
		*pdwBufferSize = dwMsgReceivedSize;
	}

	GlobalFree((HGLOBAL)pszBuffer);

	return true;
}

bool HMailSlot::WriteSlot(char* szMailSlotPath, char* szInBuffer, DWORD dwInBufferSize)
{
	HANDLE hFile;
	DWORD dwWritten;
	hFile = CreateFile(szMailSlotPath, GENERIC_WRITE, FILE_SHARE_READ, 
					(LPSECURITY_ATTRIBUTES)NULL,
					OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	if (WriteFile(hFile, szInBuffer, dwInBufferSize, 
				&dwWritten, (LPOVERLAPPED)NULL) == FALSE)
		return false;

	if (CloseHandle(hFile) == FALSE)
		return false;

	return true;
}