#include "stdafx.h"

#include "resource.h"
#include "MyWinBase.h"
#include "memo.h"		// for new button T_T

const char szMyWindowClass[]="dWindow";

#define MYWIN_BUTTON_COUNT 4
#define MYWIN_BUTTON_MENU	0
#define MYWIN_BUTTON_EXIT	1
#define MYWIN_BUTTON_NEW	2
#define MYWIN_BUTTON_COLOR	3

#define MYWIN_BUTTON_STATECOUNT	3

#define CAPTION_FILL_METHOD_SOLID	0
#define CAPTION_FILL_METHOD_STRETCH 1
#define CAPTION_FILL_METHOD_TILE	2

RECT	g_ButtonPositions[MYWIN_BUTTON_COUNT];
HFONT	g_hFont=NULL,g_hFont_Bold=NULL;
HPEN	g_hPen_Light=NULL,g_hPen_Shadow=NULL,g_hPen_White=NULL,g_hPen_Black=NULL;
HBRUSH	g_hBrush_Memo[4]={NULL,};
HBITMAP	g_hBitmap_Buttons[MYWIN_BUTTON_COUNT][MYWIN_BUTTON_STATECOUNT]={NULL,},
		g_hBitmap_Resize_Left=NULL,g_hBitmap_Resize_Left_Mask=NULL,
		g_hBitmap_Resize_Right=NULL,g_hBitmap_Resize_Right_Mask=NULL,
		g_hBitmap_Caption=NULL,g_hBitmap_Caption_Left=NULL,g_hBitmap_Caption_Right=NULL,
		g_hBitmap_Minimized=NULL;

COLORREF g_Colors[4][2]={0,RGB(222,215,181),0,RGB(222,222,222),0,RGB(214,190,181),0,RGB(166,166,181)},
		 g_Color_Caption=RGB(255,255,255);

char skinpath[_MAX_PATH],skininipath[_MAX_PATH];

int	g_nCaptionFillMethod=0,g_nDesktopX,g_nDesktopY,g_nMyWin_CaptionSize,g_nMyWin_ClientMargin,
	g_nMyWin_MinimumSize_X,g_nMyWin_MinimumSize_Y,
	g_nMyWin=0,
	g_nPressedButton=-1,g_nHoverButton=-1;

BOOL g_bMouseOnTheButton=0;

MyWinInfo_t *g_MyWinInfoArray[MAX_MYWIN];
int			g_Handles[MAX_MYWIN];

HBITMAP LoadBitmap(const char *name,bool bLoadFromFile)
{
	return (HBITMAP)LoadImage(g_hInst,name,IMAGE_BITMAP,
					0,0,LR_CREATEDIBSECTION| LR_DEFAULTSIZE | 
					(bLoadFromFile ? LR_LOADFROMFILE : NULL )
					);
}

int GetBitmapWidth(HBITMAP hbitmap)
{
	if(!hbitmap) return 0;
	BITMAP     bm;
	GetObject(hbitmap, sizeof(BITMAP), (LPSTR)&bm);
	return bm.bmWidth;
}

int GetBitmapHeight(HBITMAP hbitmap)
{
	if(!hbitmap) return 0;
	BITMAP     bm;
	GetObject(hbitmap, sizeof(BITMAP), (LPSTR)&bm);
	return bm.bmHeight;
}

char *GetStringFromIni(const char *name)
{
	static char buf[256],returnbuf[256];
	strcpy(buf,skinpath);
	GetPrivateProfileString("Bitmaps",name,"\0",returnbuf,sizeof(returnbuf),skininipath);
	strcat(buf,returnbuf);
	return buf;
}

void GetIntFromIni(const char *name,int *c)
{
	int cc;
	char buf[_MAX_PATH];

	GetPrivateProfileString("General",name,"\0",buf,sizeof(buf),skininipath);
	int nRead=sscanf(buf,"%d",&cc);
	if(nRead==1)
	{
		*c=cc;
	}
}

void GetColorFromIni(const char *name,COLORREF *c)
{
	int cc[6]={0,};
	char buf[_MAX_PATH];

	GetPrivateProfileString("Colors",name,"\0",buf,sizeof(buf),skininipath);
	int nRead=sscanf(buf,"%d %d %d %d %d %d",cc,cc+1,cc+2,cc+3,cc+4,cc+5);
	if(nRead/3)
	{
		*c=RGB(cc[0],cc[1],cc[2]);
		if(nRead/3>1)
			*(c+1)=RGB(cc[3],cc[4],cc[5]);
	}
}

int GetPositionFromIni(const char *name,RECT *c)
{
	int cc[4]={0,};
	char buf[_MAX_PATH];

	GetPrivateProfileString("General",name,"\0",buf,sizeof(buf),skininipath);
	int nRead=sscanf(buf,"%d %d %d %d",cc,cc+1,cc+2,cc+3);
	if(nRead/2)
	{
		c->left=cc[0];
		c->top=cc[1];
		if(nRead/2>1)
		{
			c->right=cc[2];
			c->bottom=cc[3];
			return 2;
		}
		else return 1;
	}
	return 0;
}

void InitResources(const char *szSkinDir)
{
	int i,j;

	// set default values
	static COLORREF Colors[4][2]={0,RGB(199,228,252),0,RGB(236,224,229),0,RGB(229,252,252),0,RGB(248,242,255)};
	static RECT Positions[MYWIN_BUTTON_COUNT]={ { 2,2,},{-16,2,},{-30,2,},{-44,2,} };

	memcpy(g_Colors,Colors,sizeof(Colors));
	memcpy(g_ButtonPositions,Positions,sizeof(Positions));
	g_nCaptionFillMethod=CAPTION_FILL_METHOD_STRETCH;
	g_Color_Caption=RGB(0,0,0);
	for(i=0;i<MYWIN_BUTTON_COUNT;i++)
	{
		for(j=0;j<MYWIN_BUTTON_STATECOUNT;j++)
			g_hBitmap_Buttons[i][j]=NULL;
	}
	g_nMyWin_ClientMargin=2;
	g_nMyWin_MinimumSize_X=100;
	g_nMyWin_MinimumSize_Y=50;

	// if default skin
	if(strcmp(szSkinDir,"Default")==0)
	{
		g_hBitmap_Buttons[MYWIN_BUTTON_MENU][0]=LoadBitmap(MAKEINTRESOURCE(IDDS_menu),false);
		g_hBitmap_Buttons[MYWIN_BUTTON_MENU][1]=LoadBitmap(MAKEINTRESOURCE(IDDS_menu_pressed),false);
		g_hBitmap_Buttons[MYWIN_BUTTON_MENU][2]=LoadBitmap(MAKEINTRESOURCE(IDDS_menu_hover),false);
		g_hBitmap_Buttons[MYWIN_BUTTON_EXIT][0]=LoadBitmap(MAKEINTRESOURCE(IDDS_exit),false);
		g_hBitmap_Buttons[MYWIN_BUTTON_EXIT][1]=LoadBitmap(MAKEINTRESOURCE(IDDS_exit_pressed),false);
		g_hBitmap_Buttons[MYWIN_BUTTON_EXIT][2]=LoadBitmap(MAKEINTRESOURCE(IDDS_exit_hover),false);
		g_hBitmap_Buttons[MYWIN_BUTTON_COLOR][0]=LoadBitmap(MAKEINTRESOURCE(IDDS_color),false);
		g_hBitmap_Buttons[MYWIN_BUTTON_COLOR][1]=LoadBitmap(MAKEINTRESOURCE(IDDS_color_pressed),false);
		g_hBitmap_Buttons[MYWIN_BUTTON_COLOR][2]=LoadBitmap(MAKEINTRESOURCE(IDDS_color_hover),false);
		g_hBitmap_Buttons[MYWIN_BUTTON_NEW][0]=LoadBitmap(MAKEINTRESOURCE(IDDS_new),false);
		g_hBitmap_Buttons[MYWIN_BUTTON_NEW][1]=LoadBitmap(MAKEINTRESOURCE(IDDS_new_pressed),false);
		g_hBitmap_Buttons[MYWIN_BUTTON_NEW][2]=LoadBitmap(MAKEINTRESOURCE(IDDS_new_hover),false);

		g_hBitmap_Resize_Left=LoadBitmap(MAKEINTRESOURCE(IDDS_resize_left),false);
		g_hBitmap_Resize_Left_Mask=LoadBitmap(MAKEINTRESOURCE(IDDS_resize_left_mask),false);
		g_hBitmap_Resize_Right=LoadBitmap(MAKEINTRESOURCE(IDDS_resize_right),false);
		g_hBitmap_Resize_Right_Mask=LoadBitmap(MAKEINTRESOURCE(IDDS_resize_right_mask),false);
		g_hBitmap_Caption=LoadBitmap(MAKEINTRESOURCE(IDDS_caption),false);
		g_hBitmap_Caption_Left=LoadBitmap(MAKEINTRESOURCE(IDDS_caption_left),false);
		g_hBitmap_Caption_Right=LoadBitmap(MAKEINTRESOURCE(IDDS_caption_right),false);
		g_hBitmap_Minimized=LoadBitmap(MAKEINTRESOURCE(IDDS_minimized),false);

	}
	else
	{
		strcpy(skinpath,"skinz\\");strcat(skinpath,szSkinDir);
		if(skinpath[strlen(skinpath)-1]!='\\') strcat(skinpath,"\\");
		strcpy(skininipath,skinpath);strcat(skininipath,"skin.ini");

		g_hBitmap_Buttons[MYWIN_BUTTON_MENU][0]=LoadBitmap(GetStringFromIni("icon_menu"));
		g_hBitmap_Buttons[MYWIN_BUTTON_MENU][1]=LoadBitmap(GetStringFromIni("icon_menu_pressed"));
		g_hBitmap_Buttons[MYWIN_BUTTON_MENU][2]=LoadBitmap(GetStringFromIni("icon_menu_hover"));
		g_hBitmap_Buttons[MYWIN_BUTTON_EXIT][0]=LoadBitmap(GetStringFromIni("icon_exit"));
		g_hBitmap_Buttons[MYWIN_BUTTON_EXIT][1]=LoadBitmap(GetStringFromIni("icon_exit_pressed"));
		g_hBitmap_Buttons[MYWIN_BUTTON_EXIT][2]=LoadBitmap(GetStringFromIni("icon_exit_hover"));
		g_hBitmap_Buttons[MYWIN_BUTTON_COLOR][0]=LoadBitmap(GetStringFromIni("icon_color"));
		g_hBitmap_Buttons[MYWIN_BUTTON_COLOR][1]=LoadBitmap(GetStringFromIni("icon_color_pressed"));
		g_hBitmap_Buttons[MYWIN_BUTTON_COLOR][2]=LoadBitmap(GetStringFromIni("icon_color_hover"));
		g_hBitmap_Buttons[MYWIN_BUTTON_NEW][0]=LoadBitmap(GetStringFromIni("icon_new"));
		g_hBitmap_Buttons[MYWIN_BUTTON_NEW][1]=LoadBitmap(GetStringFromIni("icon_new_pressed"));
		g_hBitmap_Buttons[MYWIN_BUTTON_NEW][2]=LoadBitmap(GetStringFromIni("icon_new_hover"));

		g_hBitmap_Resize_Left=LoadBitmap(GetStringFromIni("resize_left"));
		g_hBitmap_Resize_Left_Mask=LoadBitmap(GetStringFromIni("resize_left_mask"));
		g_hBitmap_Resize_Right=LoadBitmap(GetStringFromIni("resize_right"));
		g_hBitmap_Resize_Right_Mask=LoadBitmap(GetStringFromIni("resize_right_mask"));
		g_hBitmap_Caption=LoadBitmap(GetStringFromIni("caption"));
		g_hBitmap_Caption_Left=LoadBitmap(GetStringFromIni("caption_left"));
		g_hBitmap_Caption_Right=LoadBitmap(GetStringFromIni("caption_right"));
		g_hBitmap_Minimized=LoadBitmap(GetStringFromIni("minimized"));

		char buf[256];
		GetPrivateProfileString("General","CaptionFillMethod","\0",buf,sizeof(buf),skininipath);
		strlwr(buf);
		if(stricmp(buf,"tile")==0) g_nCaptionFillMethod=CAPTION_FILL_METHOD_TILE;
		else if(stricmp(buf,"stretch")==0) g_nCaptionFillMethod=CAPTION_FILL_METHOD_STRETCH;
			else g_nCaptionFillMethod=CAPTION_FILL_METHOD_SOLID;

		GetColorFromIni("Color1",g_Colors[0]);
		GetColorFromIni("Color2",g_Colors[1]);
		GetColorFromIni("Color3",g_Colors[2]);
		GetColorFromIni("Color4",g_Colors[3]);
		GetColorFromIni("Caption",&g_Color_Caption);
		
		GetPositionFromIni("icon_menu_position",&g_ButtonPositions[MYWIN_BUTTON_MENU]);
		GetPositionFromIni("icon_exit_position",&g_ButtonPositions[MYWIN_BUTTON_EXIT]);
		GetPositionFromIni("icon_color_position",&g_ButtonPositions[MYWIN_BUTTON_COLOR]);
		GetPositionFromIni("icon_new_position",&g_ButtonPositions[MYWIN_BUTTON_NEW]);

		RECT minimumsize;
		if(GetPositionFromIni("minimumsize",&minimumsize)==1)
		{
			g_nMyWin_MinimumSize_X=minimumsize.left;
			g_nMyWin_MinimumSize_Y=minimumsize.top;
		}

		GetIntFromIni("margin",&g_nMyWin_ClientMargin);
	}

	g_nMyWin_CaptionSize=GetBitmapHeight(g_hBitmap_Caption);

	// recalculate button's positions
	for(i=0;i<MYWIN_BUTTON_COUNT;i++)
	{
		g_ButtonPositions[i].right=g_ButtonPositions[i].left+GetBitmapWidth(g_hBitmap_Buttons[i][0]);
		g_ButtonPositions[i].bottom=g_ButtonPositions[i].top+GetBitmapHeight(g_hBitmap_Buttons[i][0]);
	}

	LOGFONT logfont;
	SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(logfont),&logfont,NULL);
	g_hFont=CreateFontIndirect(&logfont);
	logfont.lfWeight=FW_BOLD;
	g_hFont_Bold=CreateFontIndirect(&logfont);

	g_hPen_White=CreatePen(PS_SOLID,1,RGB(255,255,255));
	g_hPen_Light=CreatePen(PS_SOLID,1,RGB(192,192,192));
	g_hPen_Shadow=CreatePen(PS_SOLID,1,RGB(128,128,128));
	g_hPen_Black=CreatePen(PS_SOLID,1,RGB(0,0,0));

	for(i=0;i<4;i++)
		g_hBrush_Memo[i]=CreateSolidBrush(g_Colors[i][1]);

	if(g_nMyWin)
	{
		for(i=0;i<g_nMyWin;i++)
		{
			MyWinInfo_t *it=g_MyWinInfoArray[g_Handles[i]];
			if(it->Listner)
				it->Listner(MYWM_SKINCHANGE,NULL,it->pUserData);
			InvalidateRect(it->hWnd,NULL,false);
		}
	}
}

void ClearResources()
{
	int i;

	RELEASE(g_hFont);
	RELEASE(g_hFont_Bold);
	RELEASE(g_hPen_White);
	RELEASE(g_hPen_Light);
	RELEASE(g_hPen_Shadow);
	RELEASE(g_hPen_Black);

	for(i=0;i<4;i++)
		RELEASE(g_hBrush_Memo[i]);
	for(i=0;i<MYWIN_BUTTON_COUNT;i++)
	{
		RELEASE(g_hBitmap_Buttons[i][0]);
		RELEASE(g_hBitmap_Buttons[i][1]);
	}
	
	RELEASE(g_hBitmap_Resize_Left);
	RELEASE(g_hBitmap_Resize_Left_Mask);
	RELEASE(g_hBitmap_Resize_Right);
	RELEASE(g_hBitmap_Resize_Right_Mask);
	RELEASE(g_hBitmap_Caption);
	RELEASE(g_hBitmap_Caption_Left);
	RELEASE(g_hBitmap_Caption_Right);
	RELEASE(g_hBitmap_Minimized);
}

void ResizeMinimizedMyWin()
{
	int i;
	for(i=0;i<g_nMyWin;i++)
	{
		MyWinInfo_t *it=g_MyWinInfoArray[g_Handles[i]];
		if(it->bMinizied)
			MoveWindow(it->hWnd,it->rt.left,it->rt.top,GetBitmapWidth(g_hBitmap_Minimized),GetBitmapHeight(g_hBitmap_Minimized),true);

	}
}

void DrawLine(HDC hdc,int x1,int y1,int x2,int y2,HPEN pen)
{
	HPEN hOldPen;
	hOldPen=(HPEN)SelectObject(hdc,pen);
	MoveToEx(hdc,x1,y1,NULL);
	LineTo(hdc,x2,y2);
	SelectObject(hdc,hOldPen);
}

void DrawBoxFrame(HDC hdc,RECT *rt,HPEN light,HPEN dark)
{
	DrawLine(hdc,rt->right-1,rt->top,rt->right-1,rt->bottom-1,dark);
	DrawLine(hdc,rt->left,rt->bottom-1,rt->right,rt->bottom-1,dark);

	DrawLine(hdc,rt->left,rt->top,rt->right,rt->top,light);
	DrawLine(hdc,rt->left,rt->top,rt->left,rt->bottom-1,light);
}

void MyMaskBlt(HDC hdcDest,int dx,int dy,int cx,int cy,HDC hdcSrc,int sx,int sy,HBITMAP mask,int maskx,int masky)
{
	COLORREF bkcolor=SetBkColor(hdcDest, RGB(255, 255, 255));      // 1s --> 0xFFFFFF
	COLORREF txcolor=SetTextColor(hdcDest, RGB(0, 0, 0));          // 0s --> 0x000000

	// Do the real work.
	HDC hdcMask=CreateCompatibleDC(hdcDest);
	HBITMAP hBitmapPrev = (HBITMAP)SelectObject(hdcMask,mask);

	BitBlt(hdcDest, dx,dy,cx,cy,hdcSrc	,sx,sy,			SRCINVERT);
	BitBlt(hdcDest, dx,dy,cx,cy,hdcMask	,maskx,masky,	SRCAND);
	BitBlt(hdcDest, dx,dy,cx,cy,hdcSrc	,sx,sy,			SRCINVERT);

	SetBkColor(hdcDest, bkcolor);
	SetTextColor(hdcDest, txcolor);

	SelectObject(hdcMask,hBitmapPrev);

	DeleteDC(hdcMask);
}

void DrawFrame(HDC hdc,RECT *rt,MyWinInfo_t *info)
{
	HDC MemDC=CreateCompatibleDC(hdc);
    
	RECT rCaption;
	BITMAP bm;

	if(info->bMinizied)
	{
		if(g_hBitmap_Minimized)
		{
			HBITMAP hBitmapPrev = (HBITMAP)SelectObject(MemDC,g_hBitmap_Minimized);
			BitBlt(hdc,0,0,GetBitmapWidth(g_hBitmap_Minimized),GetBitmapHeight(g_hBitmap_Minimized),MemDC,0,0,SRCCOPY);
			SelectObject(MemDC,hBitmapPrev);
		}
	}
	else
	{
	// draw maginal region
		RECT mrt;
		mrt.left=rt->left+MYWIN_BORDER_SIZE;
		mrt.top=rt->top+g_nMyWin_CaptionSize;
		mrt.right=rt->right-MYWIN_BORDER_SIZE;
		mrt.bottom=mrt.top+g_nMyWin_ClientMargin+1;
		FillRect(hdc,&mrt,g_hBrush_Memo[info->nColor]);
		mrt.bottom=rt->bottom-MYWIN_BORDER_SIZE-g_nMyWin_ClientMargin;
		mrt.right=mrt.left+g_nMyWin_ClientMargin;
		FillRect(hdc,&mrt,g_hBrush_Memo[info->nColor]);
		mrt.left=rt->right-MYWIN_BORDER_SIZE-g_nMyWin_ClientMargin;
		mrt.right=mrt.left+g_nMyWin_ClientMargin;
		FillRect(hdc,&mrt,g_hBrush_Memo[info->nColor]);



	// draw resize border
		if(info->bResizable)
		{
			rCaption.top=rt->bottom-MYWIN_BORDER_SIZE-6-g_nMyWin_ClientMargin;rCaption.bottom=rt->bottom-MYWIN_BORDER_SIZE;
			rCaption.left=rt->left;rCaption.right=rt->right;
			FillRect(hdc,&rCaption,g_hBrush_Memo[info->nColor]);

			HBITMAP hBitmapPrev = (HBITMAP)SelectObject(MemDC,g_hBitmap_Resize_Left);
			MyMaskBlt(hdc,MYWIN_BORDER_SIZE,rt->bottom-MYWIN_BORDER_SIZE-6,6,6,
				MemDC,0,0,g_hBitmap_Resize_Left_Mask,0,0);
			SelectObject(MemDC,g_hBitmap_Resize_Right);
			MyMaskBlt(hdc,rt->right-MYWIN_BORDER_SIZE-6,rt->bottom-MYWIN_BORDER_SIZE-6,6,6,
						MemDC,0,0,g_hBitmap_Resize_Right_Mask,0,0);
			SelectObject(MemDC,hBitmapPrev);
		}
		else
		{
			rCaption.top=rt->bottom-MYWIN_BORDER_SIZE-g_nMyWin_ClientMargin;rCaption.bottom=rt->bottom-MYWIN_BORDER_SIZE;
			rCaption.left=rt->left;rCaption.right=rt->right;
			FillRect(hdc,&rCaption,g_hBrush_Memo[info->nColor]);
		}


	// draw frames
		RECT r;

		r.top=rt->top+g_nMyWin_CaptionSize-1;r.left=rt->left;
		r.bottom=rt->bottom;r.right=rt->right;
		DrawBoxFrame(hdc,&r,g_hPen_Shadow,g_hPen_Black);

		r.top++;r.left++;r.bottom--;r.right--;
		DrawBoxFrame(hdc,&r,g_hPen_White,g_hPen_Shadow);


	// draw caption background
		int i;
		GetObject(g_hBitmap_Caption,sizeof(BITMAP),&bm);
		int nWidthToCaption=rt->right-rt->left-GetBitmapWidth(g_hBitmap_Caption_Left)-GetBitmapWidth(g_hBitmap_Caption_Right);

		HBITMAP hBitmapPrev = (HBITMAP)SelectObject(MemDC,g_hBitmap_Caption);
		switch(g_nCaptionFillMethod)
		{
		case CAPTION_FILL_METHOD_TILE:
			{
				for(i=0;i<nWidthToCaption/bm.bmWidth;i++)
				{
					BitBlt(hdc,i*bm.bmWidth+GetBitmapWidth(g_hBitmap_Caption_Left),0
						,bm.bmWidth,GetBitmapHeight(g_hBitmap_Caption),MemDC,0,0,SRCCOPY);
				}
				BitBlt(hdc,(nWidthToCaption/bm.bmWidth)*bm.bmWidth+GetBitmapWidth(g_hBitmap_Caption_Left),0,
					nWidthToCaption%bm.bmWidth,GetBitmapHeight(g_hBitmap_Caption),MemDC,0,0,SRCCOPY);
			}break;
		case CAPTION_FILL_METHOD_STRETCH:
			{
				SetStretchBltMode(hdc,COLORONCOLOR);
				StretchBlt(hdc,GetBitmapWidth(g_hBitmap_Caption_Left),0,
							nWidthToCaption,
							GetBitmapHeight(g_hBitmap_Caption),
					MemDC,0,0,bm.bmWidth,bm.bmHeight,SRCCOPY);

			}break;
		};

		if(g_hBitmap_Caption_Left && g_hBitmap_Caption_Right)
		{
			SelectObject(MemDC,g_hBitmap_Caption_Left);
			BitBlt(hdc,0,0,GetBitmapWidth(g_hBitmap_Caption_Left),GetBitmapHeight(g_hBitmap_Caption_Left),
				MemDC,0,0,SRCCOPY);

			SelectObject(MemDC,g_hBitmap_Caption_Right);
			BitBlt(hdc,rt->right-GetBitmapWidth(g_hBitmap_Caption_Right),0,
				GetBitmapWidth(g_hBitmap_Caption_Right),GetBitmapHeight(g_hBitmap_Caption_Right),
				MemDC,0,0,SRCCOPY);
		}


	// draw caption icons
		for(i=0;i<MYWIN_BUTTON_COUNT;i++)
		{
			int state=0;
			POINT point;
			RECT *prt=GetRelativeRect(&g_ButtonPositions[i],rt);
			GetCursorPos(&point);
			point.x-=info->rt.left;point.y-=info->rt.top;
			if(isOnTheRect(point.x,point.y,&g_ButtonPositions[i],&info->rt))
				state=2;
			if((g_nPressedButton==i)&&(g_bMouseOnTheButton)) state=1;

			if(i==MYWIN_BUTTON_MENU && info->bAlwaysOnTop) state=1;

			if(g_hBitmap_Buttons[i][state]==0) state=0;
			HBITMAP hIcon=g_hBitmap_Buttons[i][state];
			if(hIcon)
			{
				SelectObject(MemDC,hIcon);
				BitBlt(hdc,prt->left,prt->top,GetBitmapWidth(hIcon),GetBitmapHeight(hIcon),MemDC,0,0,SRCCOPY);
			}
		}
		SelectObject(MemDC,hBitmapPrev);

	}	// end of not minimized;

	DeleteDC(MemDC);
}

void GetDesktopEnvironment()
{
	RECT rt;
	HWND hWnd;
	hWnd=FindWindow("Shell_TrayWnd","");
	GetWindowRect(hWnd,&rt);
	g_nDesktopY=rt.top;
	hWnd=GetDesktopWindow();
	GetWindowRect(hWnd,&rt);
	g_nDesktopX=rt.right;
}

LRESULT CALLBACK MyWin_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void Init_MyWin(const char *szSkinDir)
{
	GetDesktopEnvironment();

	int i;
	for(i=0;i<MAX_MYWIN;i++)
		g_Handles[i]=i;

	InitResources(szSkinDir);

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)MyWin_WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= g_hInst;
	wcex.hIcon			= LoadIcon(g_hInst, (LPCTSTR)IDI_LARGE);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;//g_hBrush_Memo;
	wcex.lpszMenuName	= NULL;//(LPCSTR)IDC_HELLOWIN;
	wcex.lpszClassName	= szMyWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_LARGE);

	RegisterClassEx(&wcex);
}

void ShutDown_MyWin()
{
	while(g_nMyWin)
		MyWin_DestroyWindow(g_Handles[0]);
	ClearResources();
}

int MyWin_CreateWindow(int x,int y,int Width,int Height,int nColor,DWORD dwMyWindowStyle,
					   PFN_Listner Listner,void* pUserData)
{
	int handle=g_Handles[g_nMyWin];

	MyWinInfo_t *it=new MyWinInfo_t;
	g_MyWinInfoArray[handle]=it;
	it->bResizable=dwMyWindowStyle		&MYWS_RESIZE;
	it->bDrawCaption=dwMyWindowStyle	&MYWS_SHOWCAPTION;
	it->pUserData=pUserData;
	it->nColor=nColor;
	it->nHandlePos=g_nMyWin;
	it->Listner=Listner;
	it->nMinimumSizeX=g_nMyWin_MinimumSize_X;
	it->nMinimumSizeY=g_nMyWin_MinimumSize_Y;
	it->bMinizied=false;
	it->bAlwaysOnTop=false;

	HWND hWnd = CreateWindowEx(WS_EX_TOOLWINDOW,szMyWindowClass, "" , 
	   WS_POPUP, x, y, Width, Height, NULL, NULL, g_hInst, NULL);

	if(!hWnd)
	{
		DWORD result=GetLastError();
		return -1;
	}

	it->hWnd=hWnd;
	GetWindowRect(hWnd,&it->rt);

	g_nMyWin++;
	return handle;
}

void MyWin_DestroyWindow(int nWin)
{
	if(g_MyWinInfoArray[nWin])
		DestroyWindow(g_MyWinInfoArray[nWin]->hWnd);
}

void MyWin_ShowWindow(int nWin,BOOL bShow)
{
	ShowWindow(g_MyWinInfoArray[nWin]->hWnd,bShow?SW_SHOW:SW_HIDE);
	UpdateWindow(g_MyWinInfoArray[nWin]->hWnd);
}

void MyWin_SetMinimumWindowSize(int nWin,int x,int y)
{
	MyWinInfo_t *info=g_MyWinInfoArray[nWin];
	info->nMinimumSizeX=x;
	info->nMinimumSizeY=y;
}

void MyWin_GetMinimumWindowSize(int nWin,int *x,int *y)
{
	MyWinInfo_t *info=g_MyWinInfoArray[nWin];
	*x=info->nMinimumSizeX;
	*y=info->nMinimumSizeY;
}

void MyWin_GetScreenRect(RECT *rt)
{
	rt->left=0;
	rt->top=0;
	rt->right=g_nDesktopX;
	rt->bottom=g_nDesktopY;
}

void MyWin_GetClientRect(int nWin,RECT *rt)
{
	MyWinInfo_t *info=g_MyWinInfoArray[nWin];
	if(info->bMinizied)
	{
		rt->top=0;
		rt->left=0;
		rt->right=0;
		rt->bottom=0;
	}
	else
	{
		RECT *r=&info->rt;

		rt->left=MYWIN_BORDER_SIZE+g_nMyWin_ClientMargin;
		rt->top	=g_nMyWin_CaptionSize+MYWIN_BORDER_SIZE-1+g_nMyWin_ClientMargin;

		rt->right=(r->right-r->left)-MYWIN_BORDER_SIZE-g_nMyWin_ClientMargin;
		rt->bottom=(r->bottom-r->top)-MYWIN_BORDER_SIZE-g_nMyWin_ClientMargin;
		if(info->bResizable)
			rt->bottom-=MYWIN_RESIZEBORDER_SIZE;
	}
}

void MyWin_GetRect(int nWin,RECT *rt)
{
	RECT *r=&g_MyWinInfoArray[nWin]->rt;
	memcpy(rt,r,sizeof(RECT));
}

int MyWin_GetMyWinCount()
{
	return g_nMyWin;
}

HWND MyWin_GetHWND(int nWin)
{
	return g_MyWinInfoArray[nWin]->hWnd;
}

COLORREF *MyWin_GetColors(int nColor)
{
	return g_Colors[nColor];
}

MyWinInfo_t *MyWin_GetInfo(int nWin)
{
	return g_MyWinInfoArray[nWin];
}

void MyWin_SetUserData(int nWin,void *pUserData)
{
	g_MyWinInfoArray[nWin]->pUserData=pUserData;
}

void *MyWin_GetUserData(int nWin)
{
	return g_MyWinInfoArray[nWin]->pUserData;
}

LRESULT MyWin_OnCreate(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//	_RPT0(_CRT_WARN,"Created \n");
	SetWindowLong(hWnd,GWL_USERDATA,(LONG)(g_Handles[g_nMyWin]));
	return 0;
}

#define DISTANCE(x1,y1) (abs(xy.x-(x1))+abs(xy.y-(y1)))

RECT *GetRelativeRect(const RECT *rt,const RECT *winrt)
{
	static RECT r;
	r.left= (rt->left<0)? rt->left+(winrt->right-winrt->left) : rt->left;
	r.right= (rt->right<0)? rt->right+(winrt->right-winrt->left) : rt->right;
	r.top= (rt->top<0)? rt->top+(winrt->bottom-winrt->top) : rt->top;
	r.bottom= ( rt->bottom<0)? rt->bottom + (winrt->bottom-winrt->top) : rt->bottom;
	return &r;
}

bool isOnTheRect(int x,int y,const RECT *rt,const RECT *winrt=NULL)
{
	RECT *r=GetRelativeRect(rt,winrt);
	if((x>=r->left)&&(x<r->right)&&(y>=r->top)&&(y<r->bottom))
		return true;
	return false;
}

LRESULT MyWin_OnNCHitTest(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	POINT xy;
	xy.x= LOWORD(lParam);  // horizontal position of cursor 
	xy.y = HIWORD(lParam);  // vertical position of cursor 
//	ClientToScreen(hWnd,&xy);

	int nMemo=GetWindowLong(hWnd,GWL_USERDATA);
	MyWinInfo_t *it=g_MyWinInfoArray[nMemo];
	if(it->bMinizied) return HTCAPTION;
	RECT *prt=&it->rt;

//	if(!isOnTheRect(xy.x,xy.y,prt)) return HTNOWHERE;

	ScreenToClient(hWnd,&xy);
	for(int i=0;i<MYWIN_BUTTON_COUNT;i++)
	{
		if(isOnTheRect(xy.x,xy.y,&g_ButtonPositions[i],prt))
		{
			return HTCLIENT;
		}
	}

	if(xy.y<g_nMyWin_CaptionSize)
	{
		return HTCAPTION;
	}
	
	ClientToScreen(hWnd,&xy);
	if(it->bResizable)
	{
		if( DISTANCE(prt->left,prt->bottom)<20 ) return HTBOTTOMLEFT;
		if( DISTANCE(prt->right,prt->bottom)<20 ) return HTBOTTOMRIGHT;
	}
	return HTCLIENT;
}

bool isOnTheRect(int xPos,int yPos,int nButton,MyWinInfo_t *it)
{
	if(nButton<MYWIN_BUTTON_COUNT)
		return isOnTheRect(xPos,yPos,&g_ButtonPositions[nButton],&it->rt);
	else
		return isOnTheRect(xPos,yPos,&it->buttons.at(nButton-MYWIN_BUTTON_COUNT)->rect,&it->rt);
}

void InvalidateButton(MyWinInfo_t *it,int nButton)
{
	if(nButton==-1) return;
	if(nButton<MYWIN_BUTTON_COUNT)
		InvalidateRect(it->hWnd,GetRelativeRect(&g_ButtonPositions[nButton],&it->rt),false);
	else
		InvalidateRect(it->hWnd,GetRelativeRect(&it->buttons.at(nButton-MYWIN_BUTTON_COUNT)->rect,&it->rt),false);
}

void MyWin_AlwaysOnTop(MyWinInfo_t *it,bool bAlwaysOnTop)
{
	it->bAlwaysOnTop=bAlwaysOnTop;

	RECT rt;
	GetWindowRect(it->hWnd,&rt);
	SetWindowPos(it->hWnd,it->bAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
	rt.left,rt.top,rt.right-rt.left,rt.bottom-rt.top,NULL);
}

void MyWin_AlwaysOnTop(int nWin,bool bAlwaysOnTop)
{
	MyWinInfo_t *it=MyWin_GetInfo(nWin);
	MyWin_AlwaysOnTop(it,bAlwaysOnTop);
}

LRESULT CALLBACK MyWin_OnMouseMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MyWinInfo_t *it=g_MyWinInfoArray[GetWindowLong(hWnd,GWL_USERDATA)];
	int xPos = LOWORD(lParam);
	int yPos = HIWORD(lParam);
	RECT *prt=&it->rt;


	if(g_nPressedButton==-1)
	{
		int nHover=-1,i;
		for(i=0;i<MYWIN_BUTTON_COUNT+it->buttons.size();i++)
		{
			if(isOnTheRect(xPos,yPos,i,it))
			{
				SetCapture(hWnd);
				nHover=i;
				break;
			}
		}

		if(g_nHoverButton!=nHover)
		{
			InvalidateButton(it,g_nHoverButton);
			if(nHover==-1)
				ReleaseCapture();
			else
				InvalidateButton(it,nHover);
			g_nHoverButton=nHover;
		}

		return 0;
	}

	BOOL bPrevious=g_bMouseOnTheButton;
	g_bMouseOnTheButton=isOnTheRect(xPos,yPos,g_nPressedButton,it);
	if(bPrevious!=g_bMouseOnTheButton)
		InvalidateButton(it,g_nPressedButton);
	return 0;
}

LRESULT CALLBACK MyWin_OnLButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MyWinInfo_t *it=g_MyWinInfoArray[GetWindowLong(hWnd,GWL_USERDATA)];

	int fwKeys = wParam;      
	int xPos = LOWORD(lParam);
	int yPos = HIWORD(lParam);

	for(int i=0;i<MYWIN_BUTTON_COUNT+it->buttons.size();i++)
	{
		if(isOnTheRect(xPos,yPos,i,it))
		{
			g_nPressedButton=i;g_bMouseOnTheButton=1;
			InvalidateButton(it,i);
			SetCapture(hWnd);
			return 0;
		}
	}

	return 0;
}

LRESULT CALLBACK MyWin_OnLButtonUp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MyWinInfo_t *it=g_MyWinInfoArray[GetWindowLong(hWnd,GWL_USERDATA)];
	
	if(g_bMouseOnTheButton)
	{
		g_bMouseOnTheButton=false;
		switch(g_nPressedButton)
		{
		case MYWIN_BUTTON_EXIT:
			SendMessage(hWnd,WM_CLOSE,0,0);
			InvalidateButton(it,MYWIN_BUTTON_EXIT);
			break;
		case MYWIN_BUTTON_COLOR:
			it->nColor=(it->nColor+1)%4;
			it->Listner(MYWM_COLORCHANGE,(void*)it->nColor,NULL);
			InvalidateRect(hWnd,NULL,false);
			break;
		case MYWIN_BUTTON_NEW:
			NewMemo(it->nColor);
			InvalidateButton(it,MYWIN_BUTTON_NEW);
			break;
		case MYWIN_BUTTON_MENU:
		{
			MyWin_AlwaysOnTop(it,!it->bAlwaysOnTop);
			InvalidateButton(it,MYWIN_BUTTON_MENU);
		}break;
		default:
			_ASSERT(g_nPressedButton>=MYWIN_BUTTON_COUNT);
			it->Listner(MYWM_BUTTONPRESSED,(void*)it->buttons.at(g_nPressedButton-MYWIN_BUTTON_COUNT)->nID,NULL);
			InvalidateRect(hWnd,NULL,false);
			break;
		}
	}
	ReleaseCapture();
	g_nPressedButton=-1;
	return 0;
}

LRESULT MyWin_OnPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MyWinInfo_t *it=g_MyWinInfoArray[GetWindowLong(hWnd,GWL_USERDATA)];

	PAINTSTRUCT ps;
	HDC hdc;
	hdc = BeginPaint(hWnd, &ps);
	RECT rt;
	GetClientRect(hWnd, &rt);
	DrawFrame(hdc,&rt,it);

	if(it->Listner) it->Listner(MYWM_PAINT,(void*)hdc,it->pUserData);

	if(!it->bMinizied)
	{
		HDC MemDC=CreateCompatibleDC(hdc);
		for(int i=0;i<it->buttons.size();i++)
		{
			MyButton_t *btn=it->buttons.at(i);

			// state 0 은 평상시 상태
			int state=0;
			POINT point;
			RECT *prt=GetRelativeRect(&btn->rect,&rt);
			GetCursorPos(&point);
			point.x-=it->rt.left;point.y-=it->rt.top;
			
			// 마우스커서가 위에 있을때
			if(isOnTheRect(point.x,point.y,&btn->rect,&it->rt))
				state=2;

			if((g_nPressedButton==i+MYWIN_BUTTON_COUNT)&&(g_bMouseOnTheButton)) state=1;

			if(btn->hBitmaps[state][0]==0) state=0;
			HBITMAP hIcon=btn->hBitmaps[state][0];
			if(hIcon)
			{
				SelectObject(MemDC,hIcon);
				MyMaskBlt(hdc,prt->left,prt->top,GetBitmapWidth(hIcon),GetBitmapHeight(hIcon),MemDC,0,0,
					btn->hBitmaps[state][1],0,0);
			}
		}
		DeleteDC(MemDC);	
	}
	
	EndPaint(hWnd, &ps);
	return 0;
}

LRESULT MyWin_OnMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MyWinInfo_t *it=g_MyWinInfoArray[GetWindowLong(hWnd,GWL_USERDATA)];
	GetWindowRect(hWnd,&it->rt);
	if(it->hWnd && it->Listner) it->Listner(MYWM_MOVERESIZE,&it->rt,it->pUserData);
	return 0;
}

LRESULT MyWin_OnSize(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MyWinInfo_t *it=g_MyWinInfoArray[GetWindowLong(hWnd,GWL_USERDATA)];
	GetWindowRect(hWnd,&it->rt);
	if(it->hWnd && it->Listner) it->Listner(MYWM_MOVERESIZE,&it->rt,it->pUserData);
	return 0;
}

LRESULT MyWin_OnCtlColorEdit(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int nWin=GetWindowLong(hWnd,GWL_USERDATA);
	MyWinInfo_t *it=g_MyWinInfoArray[nWin];
	if(it)
	{
		SetTextColor((HDC)wParam, g_Colors[it->nColor][0]);
		SetBkColor((HDC)wParam, g_Colors[it->nColor][1]);
		return (long)g_hBrush_Memo[it->nColor];
	}
	return 0;
}

LRESULT MyWin_OnClose(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int nWin=GetWindowLong(hWnd,GWL_USERDATA);
	MyWinInfo_t *it=g_MyWinInfoArray[nWin];
	
	int confirm=(it->Listner)? it->Listner(MYWM_CLOSE,NULL,it->pUserData):1;
	if(confirm){
		/*
		// 테스트 코드 -_-
		RECT r;
		GetWindowRect(hWnd, &r);
		HDC hDC = GetDC(NULL);

		int nWidth = r.right-r.left;
		int nHeight = r.bottom-r.top;

		SetROP2(hDC, R2_NOT);
		//R2_NOT

	#define STEP	100
		for(int i=0; i<STEP; i++){
			float fScale = i / (float)STEP ;
			Rectangle(hDC, (int)(r.left + nWidth*0.5f*fScale), (int)(r.top + nHeight*0.5f*fScale), (int)(r.right - nWidth*0.5f*fScale), (int)(r.bottom - nHeight*0.5f*fScale));
		}
		ReleaseDC(hWnd, hDC);
		*/

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

LRESULT MyWin_OnDestroy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int nWin=GetWindowLong(hWnd,GWL_USERDATA);
	MyWinInfo_t *it=g_MyWinInfoArray[nWin];
	if(it->Listner) it->Listner(MYWM_DESTROY,NULL,it->pUserData);
	g_nMyWin--;
	if(g_nMyWin)
	{
		int recover=g_Handles[g_nMyWin];
		int temp=g_Handles[g_MyWinInfoArray[nWin]->nHandlePos];
		g_Handles[g_MyWinInfoArray[nWin]->nHandlePos]=g_Handles[g_nMyWin];
		g_Handles[g_nMyWin]=temp;
		g_MyWinInfoArray[recover]->nHandlePos=g_MyWinInfoArray[nWin]->nHandlePos;
	}
	delete g_MyWinInfoArray[nWin];
	g_MyWinInfoArray[nWin]=NULL;
	return 0;
}

int GetDistance_H(int x11,int x12,int y1,int x21,int x22,int y2)
{
	int tx1=max(x11,x21),tx2=min(x12,x22);
	if(tx1<=tx2) return abs(y1-y2);
	return abs(y1-y2)+abs(tx1-tx2);
}

int GetDistance_V(int x1,int y11,int y12,int x2,int y21,int y22)
{
	int ty1=max(y11,y21),ty2=min(y12,y22);
	if(ty1<=ty2) return abs(x1-x2);
	return abs(x1-x2)+abs(ty1-ty2);
}

static RECT rtDock;

void FindMinimumDistY(int x1,int x2,int y,int except,int offset,int *mindisty,int *miny)
{
	int i,dist;
	for(i=0;i<g_nMyWin;i++)
	{
		if(i!=except)
		{
			RECT *prt=&g_MyWinInfoArray[g_Handles[i]]->rt;
			dist=GetDistance_H(x1,x2,y,prt->left,prt->right,prt->bottom);
			if(dist<*mindisty){*mindisty=dist;*miny=prt->bottom+offset;}
			dist=GetDistance_H(x1,x2,y,prt->left,prt->right,prt->top);
			if(dist<*mindisty){*mindisty=dist;*miny=prt->top+offset;}
		}
	}
	dist=GetDistance_H(x1,x2,y,0,g_nDesktopX,0);
	if(dist<*mindisty){*mindisty=dist;*miny=offset;}
	dist=GetDistance_H(x1,x2,y,0,g_nDesktopX,g_nDesktopY);
	if(dist<*mindisty){*mindisty=dist;*miny=g_nDesktopY+offset;}
}

void FindMinimumDistX(int x,int y1,int y2,int except,int offset,int *mindistx,int *minx)
{
	int i,dist;
	for(i=0;i<g_nMyWin;i++)
	{
		if(i!=except)
		{
			RECT *prt=&g_MyWinInfoArray[g_Handles[i]]->rt;
			dist=GetDistance_V(x,y1,y2,prt->left,prt->top,prt->bottom);
			if(dist<*mindistx){*mindistx=dist;*minx=prt->left+offset;}
			dist=GetDistance_V(x,y1,y2,prt->right,prt->top,prt->bottom);
			if(dist<*mindistx){*mindistx=dist;*minx=prt->right+offset;}
		}
	}
	dist=GetDistance_V(x,y1,y2,0,0,g_nDesktopY);
	if(dist<*mindistx){*mindistx=dist;*minx=offset;}
	dist=GetDistance_V(x,y1,y2,g_nDesktopX,0,g_nDesktopY);
	if(dist<*mindistx){*mindistx=dist;*minx=g_nDesktopX+offset;}
}

LRESULT MyWin_OnMoving(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//	OutputDebugString("WM_MOVING\n");
	MyWinInfo_t *it=g_MyWinInfoArray[GetWindowLong(hWnd,GWL_USERDATA)];
	GetWindowRect(hWnd,&it->rt);
	
#define MAX_SCREEN 2048
#define DOCKING_DISTANCE 10

	int mindisty=MAX_SCREEN,mindistx=MAX_SCREEN;
	RECT rtOrig,*rt=&rtOrig;
	rtOrig=rtDock;
	POINT xy;GetCursorPos(&xy);
	rtOrig.top+=xy.y;rtOrig.bottom+=xy.y;
	rtOrig.left+=xy.x;rtOrig.right+=xy.x;

	int x=rt->left,y=rt->top,tx=rt->left,ty=rt->top;
	int except=it->nHandlePos;
	FindMinimumDistY(rt->left,rt->right,rt->top,except,0,&mindisty,&y);
	FindMinimumDistY(rt->left,rt->right,rt->bottom,except,rt->top-rt->bottom,&mindisty,&y);
	FindMinimumDistX(rt->left,rt->top,rt->bottom,except,0,&mindistx,&x);
	FindMinimumDistX(rt->right,rt->top,rt->bottom,except,rt->left-rt->right,&mindistx,&x);

	if(mindisty<DOCKING_DISTANCE) ty=y;
	if(mindistx<DOCKING_DISTANCE) tx=x;
	
	rt=(LPRECT)lParam;
	int right=tx+rt->right-rt->left,bottom=ty+rt->bottom-rt->top;
	rt->left=tx;rt->top=ty;
	rt->right=right;rt->bottom=bottom;
	return TRUE;
}

LRESULT MyWin_OnSizing(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//	OutputDebugString("WM_MOVING\n");
	MyWinInfo_t *it=g_MyWinInfoArray[GetWindowLong(hWnd,GWL_USERDATA)];
	GetWindowRect(hWnd,&it->rt);

	int fwSide=wParam;

#define MAX_SCREEN 2048
#define DOCKING_DISTANCE 10

	int mindisty=MAX_SCREEN,mindistx=MAX_SCREEN;
	RECT rtOrig,*rt=&rtOrig;
	rtOrig=rtDock;
	POINT xy;GetCursorPos(&xy);
	rtOrig.top+=xy.y;rtOrig.bottom+=xy.y;
	rtOrig.left+=xy.x;rtOrig.right+=xy.x;
	int x=rt->left,y=rt->top,tx=rt->left,ty=rt->top;
	int except=it->nHandlePos;
	
	rt=(LPRECT)lParam;
	FindMinimumDistY(rt->left,rt->right,rt->bottom,except,0,&mindisty,&y);
	switch(fwSide)
	{
	case WMSZ_BOTTOMLEFT :
		{
			FindMinimumDistX(rt->left,rt->top,rt->bottom,except,0,&mindistx,&x);
			if(mindistx<DOCKING_DISTANCE) rt->left=x;
			rt->left=min(rt->left,rt->right-it->nMinimumSizeX);
		}break;
	case WMSZ_BOTTOMRIGHT:
		{
			FindMinimumDistX(rt->right,rt->top,rt->bottom,except,0,&mindistx,&x);
			if(mindistx<DOCKING_DISTANCE) rt->right=x;
			rt->right=max(rt->right,rt->left+it->nMinimumSizeX);
		}break;
	}
	if(mindisty<DOCKING_DISTANCE) rt->bottom=y;
	rt->bottom=max(rt->bottom,rt->top+it->nMinimumSizeY);
	_RPT4(_CRT_WARN,"Sizing...%d %d %d %d \n",rt->left,rt->top,rt->right,rt->bottom);
	return TRUE;
}

LRESULT CALLBACK MyWin_OnEnterSizeMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	GetDesktopEnvironment();
	POINT xy;
	GetCursorPos(&xy);
	RECT *rt=&g_MyWinInfoArray[GetWindowLong(hWnd,GWL_USERDATA)]->rt;
	rtDock.top=rt->top-xy.y;rtDock.bottom=rt->bottom-xy.y;
	rtDock.left=rt->left-xy.x;rtDock.right=rt->right-xy.x;
	return 0;
}

void MyWin_Minimize(MyWinInfo_t *it,bool bMinimize)
{
	if(bMinimize)
	{
		it->bMinizied=true;
		it->nOriginalSizeX=it->rt.right-it->rt.left;
		it->nOriginalSizeY=it->rt.bottom-it->rt.top;

		MoveWindow(it->hWnd,it->rt.left,it->rt.top,GetBitmapWidth(g_hBitmap_Minimized),GetBitmapHeight(g_hBitmap_Minimized),true);
		_RPT0(_CRT_WARN,"Minimize!\n");
	}
	else
	{
		it->bMinizied=false;
		MoveWindow(it->hWnd,it->rt.left,it->rt.top,it->nOriginalSizeX,it->nOriginalSizeY,true);
		_RPT0(_CRT_WARN,"Restore!\n");
	}

	if(it->Listner!=NULL) it->Listner(MYWM_FOCUS,(void *)((bMinimize==false)?1:0),it->pUserData);
}

void MyWin_Minimize(int nWin,bool bMinimize)
{
	MyWinInfo_t *it=MyWin_GetInfo(nWin);
	MyWin_Minimize(it,bMinimize);
}

LRESULT CALLBACK MyWin_OnNCLButtonDblClk(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int nHittest = (INT) wParam;    // hit-test value 
	switch(nHittest)
	{
	case HTCAPTION :
		{
			MyWinInfo_t *it=g_MyWinInfoArray[GetWindowLong(hWnd,GWL_USERDATA)];
			MyWin_Minimize(it,!it->bMinizied);
		}
		break;
	}

	return 0;
}

LRESULT CALLBACK MyWin_OnNCLButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int nHittest = (INT) wParam;    // hit-test value 
	switch(nHittest)
	{
	case HTCAPTION :
		{
			MyWinInfo_t *it=g_MyWinInfoArray[GetWindowLong(hWnd,GWL_USERDATA)];
			if(it->Listner!=NULL) it->Listner(MYWM_FOCUS,(void*)1,it->pUserData);
		}
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
/*
LRESULT CALLBACK MyWin_OnCaptureChanged(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	_RPT0(_CRT_WARN,"capture changed\n");
	return 0;
}
*/

LRESULT CALLBACK MyWin_OnActive(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
	MyWinInfo_t *it=g_MyWinInfoArray[GetWindowLong(hWnd,GWL_USERDATA)];
	if((LOWORD(wParam)==WA_INACTIVE)&&it->Listner) 
		it->Listner(MYWM_DEACTIVATED,NULL,it->pUserData);
	return 0;
}

LRESULT CALLBACK MyWin_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
		MY_HANDLE_MSG(WM_NCLBUTTONDBLCLK	,MyWin_OnNCLButtonDblClk);
		MY_HANDLE_MSG(WM_NCLBUTTONDOWN		,MyWin_OnNCLButtonDown);
		MY_HANDLE_MSG(WM_NCHITTEST			,MyWin_OnNCHitTest);
		MY_HANDLE_MSG(WM_LBUTTONDOWN		,MyWin_OnLButtonDown);
		MY_HANDLE_MSG(WM_LBUTTONUP			,MyWin_OnLButtonUp);
		MY_HANDLE_MSG(WM_MOUSEMOVE			,MyWin_OnMouseMove);
//		MY_HANDLE_MSG(WM_CAPTURECHANGED		,MyWin_OnCaptureChanged);
		MY_HANDLE_MSG(WM_MOVING				,MyWin_OnMoving);
		MY_HANDLE_MSG(WM_SIZING				,MyWin_OnSizing);
		MY_HANDLE_MSG(WM_ENTERSIZEMOVE		,MyWin_OnEnterSizeMove);
		MY_HANDLE_MSG(WM_CTLCOLOREDIT		,MyWin_OnCtlColorEdit);
		MY_HANDLE_MSG(WM_CREATE				,MyWin_OnCreate);
		MY_HANDLE_MSG(WM_PAINT				,MyWin_OnPaint);
		MY_HANDLE_MSG(WM_DESTROY			,MyWin_OnDestroy);
		MY_HANDLE_MSG(WM_MOVE				,MyWin_OnMove);
		MY_HANDLE_MSG(WM_SIZE				,MyWin_OnSize);
		MY_HANDLE_MSG(WM_ACTIVATE			,MyWin_OnActive);
		MY_HANDLE_MSG(WM_CLOSE				,MyWin_OnClose);

		default:				return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

bool IsExistWindowOnRect(int left,int top,int right,int bottom)
{
	int i;
	for(i=0;i<g_nMyWin;i++)
	{
		RECT *prt=&g_MyWinInfoArray[g_Handles[i]]->rt;
		if((prt->top==top)&&(prt->left==left)&&(prt->right==right)&&(prt->bottom==bottom))
			return true;
	}
	return false;
}

void MyWin_GetNewMemoPosition(int sizex,int sizey,int *x,int *y)
{
	int i,j,nx,ny;
	
	GetDesktopEnvironment();
	nx=g_nDesktopX/sizex;
	ny=g_nDesktopY/sizey;

	*x=g_nDesktopX-sizex;
	*y=0;

	for(i=0;i<nx;i++)
	{
		for(j=0;j<ny;j++)
		{
			if(!IsExistWindowOnRect(g_nDesktopX-(i+1)*sizex,j*sizey,g_nDesktopX-i*sizex,j*sizey+sizey))
			{
				*x=g_nDesktopX-(i+1)*sizex;
				*y=j*sizey;
				return;
			}
		}
	}
}

void MyWin_RegisterButton(int nWin,MyButton_t *buttoninfo)
{
	MyWinInfo_t *info=g_MyWinInfoArray[nWin];
	MyButton_t *btn=new MyButton_t;
	memcpy(btn,buttoninfo,sizeof(MyButton_t));
	info->buttons.push_back(btn);
}

//#define SUPPORT_TRANSLUCENT

void SetTranslucent(HWND hWnd, int nPercent)
{
#ifdef SUPPORT_TRANSLUCENT
#ifdef _WIN32_WINNT
	if(nPercent<100){
		// Set WS_EX_LAYERED on this window 
		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		// Make this window nPercent% alpha
		SetLayeredWindowAttributes(hWnd, 0, (255 * nPercent) / 100, LWA_ALPHA);
	}
	else{
		long lValue = GetWindowLong(hWnd, GWL_EXSTYLE);
		if((lValue&WS_EX_LAYERED)==WS_EX_LAYERED){
			lValue ^= WS_EX_LAYERED;
			SetWindowLong(hWnd, GWL_EXSTYLE, lValue);
		}
	}
#endif
#endif
}

void MyWin_ShowHideAll()
{
#ifdef SUPPORT_TRANSLUCENT
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx (&osvi);
	bool bWinNT5orLater = (osvi.dwMajorVersion >= 5);
#else
	bool bWinNT5orLater = false;
#endif
 
	static int nShow = 1;
	nShow ++;
	nShow %= ((bWinNT5orLater==true)?3:2);

	for(int i=0;i<g_nMyWin;i++)
	{
		MyWinInfo_t *it=g_MyWinInfoArray[g_Handles[i]];
		if(nShow==0) ShowWindow(it->hWnd,SW_HIDE);
		else if(nShow==1){
			ShowWindow(it->hWnd,SW_SHOW);
			SetTranslucent(it->hWnd,100);
		}
		else{
			ShowWindow(it->hWnd,SW_SHOW);
			SetTranslucent(it->hWnd,70);
		}
		if(nShow>0) SetForegroundWindow(it->hWnd);
	}
}

void MyWin_ShowAll()
{
	for(int i=0;i<g_nMyWin;i++)
	{
		MyWinInfo_t *it=g_MyWinInfoArray[g_Handles[i]];
		ShowWindow(it->hWnd,SW_SHOW);
		SetForegroundWindow(it->hWnd);
	}
}