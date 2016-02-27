#if !defined(AFX_HELLOWIN_H__F38868E9_F6D5_4B0D_B7CC_F34D02818FBD__INCLUDED_)
#define AFX_HELLOWIN_H__F38868E9_F6D5_4B0D_B7CC_F34D02818FBD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include "resource.h"

extern HWND			g_hWnd;
extern HINSTANCE	g_hInst;
extern BOOL			g_bShowCalendar,g_bShowFriends;

// Foward declarations of functions included in this code module:
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void WriteConfiguration();

#endif // !defined(AFX_HELLOWIN_H__F38868E9_F6D5_4B0D_B7CC_F34D02818FBD__INCLUDED_)

