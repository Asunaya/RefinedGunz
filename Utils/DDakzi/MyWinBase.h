#ifndef __MYWINBASE_H
#define __MYWINBASE_H

#include <windows.h>
#include <vector>

using namespace std;

#define MAX_MYWIN	100

#define MYWIN_BORDER_SIZE	2
#define MYWIN_RESIZEBORDER_SIZE	6

extern HFONT		g_hFont,g_hFont_Bold;
extern HPEN			g_hPen_Light,g_hPen_Shadow;
extern HBITMAP		g_hBitmap_Exit;
extern HBRUSH		g_hBrush_Memo[4];
extern HINSTANCE	g_hInst;
extern COLORREF		g_Color_Caption,g_Colors[4][2];
extern int			g_nMyWin_CaptionSize,g_nMyWin_ClientMargin;

#define RELEASE(x) if(x) { DeleteObject(x);x=NULL; }
#define MY_HANDLE_MSG(msg,fn)	case msg:	return fn(hWnd,message,wParam,lParam);

// my window messages
typedef enum MYWINDOWMESSAGE
{
	MYWM_CREATE,
	MYWM_CLOSE,
	MYWM_DESTROY,
	MYWM_MOVERESIZE,
	MYWM_PAINT,
	MYWM_SKINCHANGE,
	MYWM_COLORCHANGE,
	MYWM_BUTTONPRESSED,
	MYWM_DEACTIVATED,
	MYWM_FOCUS,
};

// my window styles
#define MYWS_RESIZE			0x00000001
#define MYWS_SYSTEMBUTTONS	0x00000002
#define MYWS_SHOWCAPTION	0x00000004

typedef int (*PFN_Listner) (MYWINDOWMESSAGE,void*,void*);	// message parameter pUserdata

struct MyButton_t
{
	HBITMAP hBitmaps[3][2];
	RECT	rect;
	int		nID;
};

struct MyWinInfo_t
{
	RECT	rt;
	HWND	hWnd;
	BOOL	bResizable;
	BOOL	bDrawCaption;
	int		nColor;
	void	*pUserData;
	int		nHandlePos;
	BOOL	bMinizied;
	BOOL	bAlwaysOnTop;
	int		nOriginalSizeX,nOriginalSizeY;
	int		nMinimumSizeX,nMinimumSizeY;
	PFN_Listner	Listner;
	vector<MyButton_t*> buttons;

	MyWinInfo_t() { pUserData=NULL;Listner=NULL;hWnd=NULL;nHandlePos=0; }
};

// general functions
void	Init_MyWin(const char *szSkinDir);
void	ShutDown_MyWin();

void	InitResources(const char *szSkinDir);
void	ClearResources();
void	ResizeMinimizedMyWin();
int		GetBitmapWidth(HBITMAP hbitmap);
int		GetBitmapHeight(HBITMAP hbitmap);
void	MyWin_ShowHideAll();
void	MyWin_ShowAll();

// helper functions
HBITMAP LoadBitmap(const char *name,bool bLoadFromFile=true);
void	MyMaskBlt(HDC hdcDest,int dx,int dy,int cx,int cy,HDC hdcSrc,int sx,int sy,HBITMAP mask,int maskx,int masky);

// MyWin functions
int				MyWin_CreateWindow(int x,int y,int Width,int Height,int nColor,DWORD dwMyWindowStyle,
						PFN_Listner Listner,void* pUserData);
void			MyWin_SetMinimumWindowSize(int nWin,int x,int y);
void			MyWin_GetMinimumWindowSize(int nWin,int *x,int *y);
void			MyWin_DestroyWindow(int nWin);
void			MyWin_ShowWindow(int nWin,BOOL bShow);
void			MyWin_GetScreenRect(RECT *rt);
void			MyWin_GetClientRect(int nWin,RECT *rt);
void			MyWin_GetRect(int nWin,RECT *rt);
HWND			MyWin_GetHWND(int nWin);
MyWinInfo_t*	MyWin_GetInfo(int nWin);
COLORREF*		MyWin_GetColors(int nColor);
void			MyWin_SetUserData(int nWin,void *pUserData);
void*			MyWin_GetUserData(int nWin);
void			MyWin_GetNewMemoPosition(int sizex,int sizey,int *x,int *y);
void			MyWin_Minimize(int nWin,bool bMinimize);
void			MyWin_AlwaysOnTop(int nWin,bool bAlwaysOnTop);
void			MyWin_RegisterButton(int nWin,MyButton_t *buttoninfo);

// private
RECT *GetRelativeRect(const RECT *rt,const RECT *winrt);
bool isOnTheRect(int x,int y,const RECT *rt,const RECT *winrt);
void DrawLine(HDC hdc,int x1,int y1,int x2,int y2,HPEN pen);

#endif