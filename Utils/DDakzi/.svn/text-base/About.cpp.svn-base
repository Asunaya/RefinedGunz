#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include "MyWinBase.h"
#include "resource.h"

int g_hMyWin_About=NULL;
HWND g_hWnd_AboutDlg=NULL;

WNDPROC oldproc;
HBITMAP g_hBitmap_Logo,g_hBitmap_Logo_Mask;

void ResizeAboutDialog(bool bPreservePosition)
{
	RECT rt;

	int px,py;
	if(bPreservePosition)
	{
		MyWin_GetRect(g_hMyWin_About,&rt);
		px=rt.left;py=rt.top;
	}
	
	HWND hWnd=MyWin_GetHWND(g_hMyWin_About);

	// get dialog size
	GetWindowRect(g_hWnd_AboutDlg,&rt);
	int sx=rt.right-rt.left,sy=rt.bottom-rt.top;

	// get parent size
	MoveWindow(hWnd,0,0,sx,sy,true);
	MyWin_GetClientRect(g_hMyWin_About,&rt);
	rt.top-=g_nMyWin_ClientMargin;
	rt.left-=g_nMyWin_ClientMargin;
	rt.bottom+=g_nMyWin_ClientMargin;
	rt.right+=g_nMyWin_ClientMargin;
	int ox=rt.left,oy=rt.top;
	int psx=sx+sx-(rt.right-rt.left),psy=sy+sy-(rt.bottom-rt.top);

	// move parent window
	MyWin_GetScreenRect(&rt);
	if(bPreservePosition)
		MoveWindow(hWnd,px,py,psx,psy,true);
	else	
		MoveWindow(hWnd,rt.right-psx,rt.bottom-psy,psx,psy,true);

	// move dialog
	MoveWindow(g_hWnd_AboutDlg,MYWIN_BORDER_SIZE,MYWIN_BORDER_SIZE+g_nMyWin_CaptionSize,sx,sy,true);
}

int About_Listner(MYWINDOWMESSAGE nMessage,void* Param,void *pUserData)
{
	switch(nMessage)
	{
		case MYWM_SKINCHANGE: ResizeAboutDialog(true);break;
	}
	return 1;
}

LRESULT CALLBACK LogoProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc;
				hdc = BeginPaint(hWnd, &ps);
				HDC MemDC=CreateCompatibleDC(hdc);
				SelectObject(MemDC,g_hBitmap_Logo);
				MyMaskBlt(hdc,0,0,GetBitmapWidth(g_hBitmap_Logo),GetBitmapHeight(g_hBitmap_Logo),
					MemDC,0,0,g_hBitmap_Logo_Mask,0,0);
				DeleteDC(MemDC);
				EndPaint(hWnd,&ps);
				return 0;
			}break;
		default:
			return oldproc(hWnd,message,wParam,lParam);
	}
}

// Mesage handler for about dialog.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			{
				g_hWnd_AboutDlg=hDlg;
				ResizeAboutDialog(false);
				MyWin_ShowWindow(g_hMyWin_About,TRUE);

				char buf[256];
				sprintf(buf,"Build at %s",__DATE__);
				SetWindowText(GetDlgItem(hDlg,IDC_STATIC_BUILDDATE),buf);
				
				oldproc=(WNDPROC)SetWindowLong(GetDlgItem(hDlg,IDC_STATIC_LOGO),GWL_WNDPROC,(long)LogoProc);
				g_hBitmap_Logo=LoadBitmap(MAKEINTRESOURCE(IDB_BITMAP_LOGO),false);
				g_hBitmap_Logo_Mask=LoadBitmap(MAKEINTRESOURCE(IDB_BITMAP_LOGO_MASK),false);
				return TRUE;
			}
		case WM_CTLCOLORBTN		:
		case WM_CTLCOLORSTATIC	:
		case WM_CTLCOLORDLG		: 
			{
				MyWinInfo_t *it=MyWin_GetInfo(g_hMyWin_About);
				HDC hdc= (HDC) wParam;
				SetBkColor(hdc,g_Colors[it->nColor][1]);
				return (long)g_hBrush_Memo[it->nColor];
			}break;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				MyWin_DestroyWindow(g_hMyWin_About);
				return TRUE;
			}
			break;
		case WM_DESTROY:
			{
				RELEASE(g_hBitmap_Logo);
				RELEASE(g_hBitmap_Logo_Mask);
				g_hMyWin_About=NULL;
				g_hWnd_AboutDlg=NULL;
				return TRUE;
			}

	}
    return FALSE;
}

void CreateAbout()
{
	g_hMyWin_About=MyWin_CreateWindow(0,0,0,0,0,NULL,About_Listner,NULL);
	HWND hWnd=MyWin_GetHWND(g_hMyWin_About);
	
	RECT rt,wrt;
	MyWin_GetRect(g_hMyWin_About,&wrt);
	MyWin_GetClientRect(g_hMyWin_About,&rt);
	rt.top+=wrt.top;
	rt.bottom+=wrt.top;
	rt.left+=wrt.left;
	rt.right+=wrt.left;
	g_hWnd_AboutDlg=CreateDialog(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
	ShowWindow(g_hWnd_AboutDlg,SW_SHOW);
	UpdateWindow(g_hWnd_AboutDlg);
}

