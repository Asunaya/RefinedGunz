#pragma once


#include <windows.h>
#include "shlwapi.h"


class MWindowFinder {
protected:
	char	m_szSearchText[_MAX_DIR];
	bool	m_bResult;
	HWND	m_hWnd;
public:
	MWindowFinder()		{ m_szSearchText[0]=NULL;	m_bResult=false;	m_hWnd=NULL; }
	bool GetResult()	{ return m_bResult; }
	HWND GetHWND()		{ return m_hWnd; }

	static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {
		char szWinText[_MAX_DIR] = "";
		GetWindowText(hWnd, szWinText, _MAX_DIR);

		if (strlen(szWinText) <= 0)
			return TRUE;
		
		MWindowFinder* pFinder = (MWindowFinder*)lParam;
		if (StrStrI(szWinText, pFinder->m_szSearchText) != 0) {
			pFinder->m_bResult = true;
			pFinder->m_hWnd = hWnd;
			return FALSE;
		}

		return TRUE;
	}
	static BOOL CALLBACK EnumWindowsClassNameProc(HWND hWnd, LPARAM lParam) {
	
		char szClassName[_MAX_DIR] = "";
		GetClassName(hWnd, szClassName, _MAX_DIR);
		if (strlen(szClassName) <= 0)
			return TRUE;
		
		MWindowFinder* pFinder = (MWindowFinder*)lParam;
		if (StrStrI(szClassName, pFinder->m_szSearchText) != 0) {
			pFinder->m_bResult = true;
			pFinder->m_hWnd = hWnd;
			return FALSE;
		}

		return TRUE;
	}

	bool Find(const char* pszName) {
		m_bResult = false;
		m_hWnd = NULL;
		strcpy(m_szSearchText, pszName);

		EnumWindows(EnumWindowsProc, (LPARAM)this);

		return GetResult();
	}
	bool FindExact(const char* pszName) {
		HWND hWnd = ::FindWindow(NULL, pszName);
		if (hWnd) {
			m_bResult = true;
			m_hWnd = hWnd;
		} else {
			m_bResult = false;
			m_hWnd = NULL;
		}
		return m_bResult;
	}

	/// 윈도우 클래스 이름으로 찾기
	bool FindClassName(const char* pszName) {
		m_bResult = false;
		m_hWnd = NULL;
		strcpy(m_szSearchText, pszName);

		EnumWindows(EnumWindowsClassNameProc, (LPARAM)this);

		return GetResult();
	}

};
