#include "stdafx.h"
// HelloWin.cpp : Defines the entry point for the application.
//

#include "MyWinBase.h"
#include "NetBase.h"
#include "HelloWin.h"
#include "Calendar.h"
#include "Memo.h"
#include "about.h"
#include <zmouse.h>
#include "Friends.h"
#include "XWheelDll/XWheelDll.h"

#define APP_CLASSNAME	"Hello:)"
#define APP_NAME		"HelloWin!"
#define ID_TRAY_MESSAGE WM_USER+1

// Global Variables:
HWND		g_hWnd=NULL;
HINSTANCE	g_hInst=NULL;
HMENU		g_hMenu=NULL;

BOOL		g_bShowCalendar=false,g_bShowFriends=false;

static UINT s_uTaskbarRestart;

void AddTaskbarIcons()
{
	NOTIFYICONDATA tnd;
	HICON hIcon;

	tnd.cbSize = sizeof(NOTIFYICONDATA);
	tnd.hWnd = g_hWnd;
	tnd.uID = IDI_SMALL;
	tnd.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	tnd.uCallbackMessage = ID_TRAY_MESSAGE;
	hIcon = (HICON)LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_SMALL));
	tnd.hIcon = hIcon;
    strcpy(tnd.szTip, "µüÁö?");

	Shell_NotifyIcon(NIM_ADD,&tnd);

}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_LARGE);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_MENU+1);
	wcex.lpszMenuName	= NULL;//(LPCSTR)IDC_HELLOWIN;
	wcex.lpszClassName	= APP_CLASSNAME;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_LARGE);
	RegisterClassEx(&wcex);

	g_hInst = hInstance; // Store instance handle in our global variable

	g_hWnd = CreateWindowEx(WS_EX_TOOLWINDOW,APP_CLASSNAME, APP_NAME , WS_OVERLAPPED,
	  0, 0,
	  100, 100,
	  NULL, NULL, hInstance, NULL);

	if (!g_hWnd) return FALSE;
	
	AddTaskbarIcons();
	return TRUE;
}

#define _MAX_MENU 100

static int g_nSkin=0,g_nSelectedSkin=0;
static char menubuffer[_MAX_MENU][100]={"Default","",};

void ScanSkinz()
{
#define SKIN_MENU_POSITION	5
#define SKIN_MENU_ID		0x666

	HMENU hMenu_Skin=GetSubMenu(GetSubMenu(g_hMenu,0),SKIN_MENU_POSITION);

	int i,nMenu=GetMenuItemCount(hMenu_Skin);
	for(i=0;i<nMenu;i++)
		RemoveMenu(hMenu_Skin,0,MF_BYPOSITION);

	MENUITEMINFO mii;
	mii.cbSize=sizeof(MENUITEMINFO);
	mii.fMask=MIIM_DATA|MIIM_TYPE|MIIM_ID;
	mii.fType=MFT_STRING;
	mii.fState=MFS_ENABLED;
	mii.wID=SKIN_MENU_ID;

	mii.dwTypeData=menubuffer[0];
	mii.cch=strlen(menubuffer[0]);
	InsertMenuItem(hMenu_Skin,0,TRUE,&mii);

	struct _finddata_t fd_dirs;
	long hFile;

	fd_dirs.attrib=_A_SUBDIR;
	if( (hFile = _findfirst( "skinz\\*.*", &fd_dirs )) == -1L )
	{
	}
	else
	{
		g_nSkin=1;
		do{
			if((strcmp(fd_dirs.name,".")!=0)&&(strcmp(fd_dirs.name,"..")!=0))
			{
				mii.dwTypeData=menubuffer[g_nSkin];
				strcpy(menubuffer[g_nSkin],fd_dirs.name);
				mii.cch=strlen(menubuffer[g_nSkin]);
				mii.wID++;
				InsertMenuItem(hMenu_Skin,GetMenuItemCount(hMenu_Skin),TRUE,&mii);
				g_nSkin++;
			}
		}
		while( _findnext( hFile, &fd_dirs )!=-1 );
	   _findclose( hFile );
	}

	CheckMenuItem(hMenu_Skin, g_nSelectedSkin,MF_BYPOSITION | MF_CHECKED);
}

#define INIFILENAME	"./DDakzi.ini"

void ReadConfiguration()
{
#define DEFAULT_SKIN_NAME	"Mac OS X"
	ScanSkinz();
	char buf[_MAX_PATH];
	GetPrivateProfileString("Globals","Skin",DEFAULT_SKIN_NAME,buf,sizeof(buf),INIFILENAME);
	for(int i=0;i<g_nSkin;i++)
		if(stricmp(menubuffer[i],buf)==0)
			g_nSelectedSkin=i;

	int tx,ty;

	// read calendar information
	GetPrivateProfileString("Calendar","Show","Default",buf,sizeof(buf),INIFILENAME);
	g_bShowCalendar=(stricmp(buf,"yes")==0);
	GetPrivateProfileString("Calendar","Position","Default",buf,sizeof(buf),INIFILENAME);
	if(sscanf(buf,"%d %d",&tx,&ty)==2)
	{	
		g_nCalendar_PosX=tx;
		g_nCalendar_PosY=ty;
	}
	GetPrivateProfileString("Calendar","nColor","Default",buf,sizeof(buf),INIFILENAME);
	sscanf(buf,"%d",&g_nCalendar_Col);

	// read friends information
	GetPrivateProfileString("Friends","Show","Default",buf,sizeof(buf),INIFILENAME);
	g_bShowFriends=(stricmp(buf,"yes")==0);
	GetPrivateProfileString("Friends","Position","Default",buf,sizeof(buf),INIFILENAME);
	if(sscanf(buf,"%d %d",&tx,&ty)==2)
	{	
		g_nFriends_PosX=tx;
		g_nFriends_PosY=ty;
	}
	GetPrivateProfileString("Friends","Size","Default",buf,sizeof(buf),INIFILENAME);
	if(sscanf(buf,"%d %d",&tx,&ty)==2)
	{	
		g_nFriends_SizeX=tx;
		g_nFriends_SizeY=ty;
	}
	GetPrivateProfileString("Friends","nColor","Default",buf,sizeof(buf),INIFILENAME);
	sscanf(buf,"%d",&g_nFriends_Col);
}

void WriteConfiguration()
{
	// global
	WritePrivateProfileString("Globals","Skin",menubuffer[g_nSelectedSkin],INIFILENAME);

	char buf[_MAX_PATH];
	
	// write Calendar information
	{
		Calendar_Info_t *it=GetCalendarInfo();

		bool bShow=(it->dwFlags & CF_SHOW) !=0 ;
		WritePrivateProfileString("Calendar","Show",bShow?"yes":"no",INIFILENAME);
		bool bMinimized=(it->dwFlags & CF_MINIMIZED) !=0 ;
		WritePrivateProfileString("Calendar","Minimized",bMinimized?"yes":"no",INIFILENAME);
		bool bAlwaysOnTop=(it->dwFlags & CF_ALWAYSONTOP) !=0 ;
		WritePrivateProfileString("Calendar","AlwaysOnTop",bAlwaysOnTop?"yes":"no",INIFILENAME);

		sprintf(buf,"%d %d",it->xPos,it->yPos);
		WritePrivateProfileString("Calendar","Position",buf,INIFILENAME);
		sprintf(buf,"%d",it->nColor);
		WritePrivateProfileString("Calendar","Color",buf,INIFILENAME);
	}


	// write friends information
	{
	Friends_Info_t *it=GetFriendsInfo();
	WritePrivateProfileString("Friends","Show",g_bShowFriends?"yes":"no",INIFILENAME);
	sprintf(buf,"%d %d",it->xPos,it->yPos);
	WritePrivateProfileString("Friends","Position",buf,INIFILENAME);
	sprintf(buf,"%d %d",it->xSize,it->ySize);
	WritePrivateProfileString("Friends","Size",buf,INIFILENAME);
	sprintf(buf,"%d",it->nColor);
	WritePrivateProfileString("Friends","Color",buf,INIFILENAME);
	}

}

LRESULT CALLBACK Main_OnDestroy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WriteConfiguration();
	ShutDown_Memo();
	ShutDown_Calendar();
	DestroyFriends();
	ShutDown_MyWin();

	NOTIFYICONDATA tnd;

	tnd.cbSize = sizeof(NOTIFYICONDATA);
	tnd.hWnd = g_hWnd;
	tnd.uID = IDI_SMALL;
	Shell_NotifyIcon(NIM_DELETE,&tnd);
	DestroyMenu(g_hMenu);
	PostQuitMessage(0);
	return 0;
}

void OnUpdate()
{
	WinExec("WiseUpdt.exe",SW_SHOW);
}

LRESULT CALLBACK Main_OnTrayMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (lParam)
	{
	case WM_MBUTTONDOWN:	NewMemo();break;
	case WM_LBUTTONDOWN:	MyWin_ShowAll();break;
//	case WM_RBUTTONDOWN:	SetCapture(hWnd);break;
	case WM_RBUTTONUP:
		{
			ReleaseCapture();
			ScanSkinz();
			CheckMenuItem(GetSubMenu(g_hMenu, 0),ID_MENUITEM_SHOW_CALENDAR,
				g_bShowCalendar?MF_CHECKED:MF_UNCHECKED);
			CheckMenuItem(GetSubMenu(g_hMenu, 0),ID_MENUITEM_SHOW_FRIENDS,
				g_bShowFriends?MF_CHECKED:MF_UNCHECKED);

			POINT c;
			GetCursorPos(&c);
			SetForegroundWindow(g_hWnd);
			int sel=TrackPopupMenuEx(GetSubMenu(g_hMenu, 0),TPM_RETURNCMD | TPM_HORIZONTAL,
				c.x,c.y,g_hWnd,NULL);
			PostMessage(g_hWnd, WM_NULL, 0, 0);
			if((sel>=SKIN_MENU_ID)&&(sel<SKIN_MENU_ID+g_nSkin))
			{
				ClearResources();
				g_nSelectedSkin=sel-SKIN_MENU_ID;
				InitResources(menubuffer[g_nSelectedSkin]);
				ResizeMinimizedMyWin();
				WriteConfiguration();
			}
			else
			{
				switch(sel)
				{
				case ID_MENUITEM_SHOW_CALENDAR:
				{
					if(!g_bShowCalendar)
						CreateCalendar();
					else
						DestroyCalendar();
				}
				break;
				case ID_MENUITEM_SHOW_FRIENDS:
				{
					if(!g_bShowFriends)
						CreateFriends();
					else
						DestroyFriends();
				}
				break;
				
				case ID_MENUITEM_NEW_MEMO:		NewMemo();break;
				case ID_MENUITEM_ABOUT:			CreateAbout();break;
				case ID_MENUITEM_EXIT:			DestroyWindow(g_hWnd);break;
				case ID_MENUITEM_SHOWHIDEALL:	MyWin_ShowAll();break;
				case ID_MENUITEM_UPDATE:		OnUpdate();break;
				}
			}
			break;
		}
	}
	return 0;
}

LRESULT CALLBACK Main_OnCreate(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
		MY_HANDLE_MSG(WM_CREATE				,Main_OnCreate);
		MY_HANDLE_MSG(ID_TRAY_MESSAGE		,Main_OnTrayMessage);
		MY_HANDLE_MSG(WM_DESTROY			,Main_OnDestroy);
		default:
			if(message == s_uTaskbarRestart)
			{
				AddTaskbarIcons();
				break;
			}
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}


int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	HINSTANCE mMouseDllHinst = LoadLibrary("xwheel.dll") ;

	MSG msg;

	if(FindWindow(APP_CLASSNAME, APP_NAME)!=NULL){
		MessageBox(NULL, "Already Running!", "Hello ?", MB_ICONINFORMATION);
		return 0;
	}

	g_hMenu=LoadMenu(hInstance,MAKEINTRESOURCE(IDR_MENU1));
	if (!InitInstance (hInstance, nCmdShow)) return FALSE;

	ReadConfiguration();
	Init_MyWin(menubuffer[g_nSelectedSkin]);

	Init_Memo();
	Init_Calendar();
	InitFriends();

	StartMouseHook();

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	EndMouseHook();
	FreeLibrary(mMouseDllHinst);
	return msg.wParam;
}

