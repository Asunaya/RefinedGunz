#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <crtdbg.h>
#include "resource.h"
#include "HelloWin.h"
#include "MyWinBase.h"
#include "NetBase.h"
#include "Friends.h"

HWND g_hWnd_Friends=NULL,g_hWnd_Tree=NULL;
int	g_nWin_Friends=NULL;
int			g_nFriends_PosX=-1,g_nFriends_PosY=-1,g_nFriends_Col;
int			g_nFriends_SizeX=-1,g_nFriends_SizeY=-1;

Friends_Info_t *GetFriendsInfo()
{
	static Friends_Info_t it;
	if(g_bShowFriends)
	{
		RECT rt;
		MyWin_GetRect(g_nWin_Friends,&rt);
		it.xPos=rt.left;
		it.yPos=rt.top;
		it.xSize=rt.right-rt.left;
		it.ySize=rt.bottom-rt.top;
	}
	else
	{
		it.xPos=g_nFriends_PosX;it.yPos=g_nFriends_PosY;
		it.xSize=g_nFriends_SizeX;it.ySize=g_nFriends_SizeY;
	}
	it.nColor=0;
	it.bShow=g_bShowFriends;
	return &it;
}

void Friends_Close()
{
	g_bShowFriends=false;
}

void Friends_MoveResize()
{
#define FRIEND_WINDOW_MARGIN_X	4
#define FRIEND_WINDOW_MARGIN_TOP	4
#define FRIEND_WINDOW_MARGIN_BOTTOM 20
	if(g_hWnd_Tree)
	{
		RECT rt;
		MyWin_GetClientRect(g_nWin_Friends,&rt);
		MoveWindow(g_hWnd_Tree,
			rt.left+FRIEND_WINDOW_MARGIN_X,
			rt.top+FRIEND_WINDOW_MARGIN_TOP,
			rt.right-rt.left-FRIEND_WINDOW_MARGIN_X*2,
			rt.bottom-rt.top-FRIEND_WINDOW_MARGIN_TOP-FRIEND_WINDOW_MARGIN_BOTTOM,false);
	}
}

void Friends_ColorChange(int nColor)
{
/*	TreeView_SetTextColor(g_hWnd_Tree,*MyWin_GetColors(nColor));
	TreeView_SetBkColor(g_hWnd_Tree,*(MyWin_GetColors(nColor)+1));
*/	TreeView_SetTextColor(g_hWnd_Tree,RGB(0,0,0));
	TreeView_SetBkColor(g_hWnd_Tree,RGB(255,255,255));
}

void Friends_Paint(HDC hdc)
{
	HBRUSH br=g_hBrush_Memo[MyWin_GetInfo(g_nWin_Friends)->nColor];

	RECT ort,rt;
	MyWin_GetClientRect(g_nWin_Friends,&ort);
	
	// top margin
	rt.left=ort.left;rt.right=ort.right;
	rt.top=ort.top;rt.bottom=ort.top+FRIEND_WINDOW_MARGIN_TOP;
	FillRect(hdc,&rt,br);

	// bottom
	rt.left=ort.left;rt.right=ort.right;
	rt.top=ort.bottom-FRIEND_WINDOW_MARGIN_TOP-FRIEND_WINDOW_MARGIN_BOTTOM;
	rt.bottom=ort.bottom;
	FillRect(hdc,&rt,br);

	// left
	rt.left=ort.left;rt.right=ort.left+FRIEND_WINDOW_MARGIN_X;
	rt.top=ort.top+FRIEND_WINDOW_MARGIN_TOP;
	rt.bottom=ort.bottom-FRIEND_WINDOW_MARGIN_TOP-FRIEND_WINDOW_MARGIN_BOTTOM;
	FillRect(hdc,&rt,br);

	// right
	rt.left=ort.right-FRIEND_WINDOW_MARGIN_X;rt.right=ort.right;
	rt.top=ort.top+FRIEND_WINDOW_MARGIN_TOP;
	rt.bottom=ort.bottom-FRIEND_WINDOW_MARGIN_TOP-FRIEND_WINDOW_MARGIN_BOTTOM;
	FillRect(hdc,&rt,br);
}

void Friends_SkinChange()
{
	SetWindowFont(g_hWnd_Tree, g_hFont, true);
	Friends_MoveResize();
}

int Friends_Listner(MYWINDOWMESSAGE nMessage,void* Param,void *pUserData)
{
	switch(nMessage)
	{
	case MYWM_PAINT			: Friends_Paint((HDC)Param);break;
	case MYWM_CLOSE			: Friends_Close();break;
	case MYWM_MOVERESIZE	: Friends_MoveResize();break;
	case MYWM_COLORCHANGE	: Friends_ColorChange((int)Param);break;
	case MYWM_SKINCHANGE	: Friends_SkinChange();break;

/*	case MYWM_PAINT		: Friends_Paint(g_nWin_Friends,(HDC)Param);break;
	case MYWM_MOVERESIZE: Memo_MoveResize(t->nWin,t->hWnd_Edit);break;
*/	}
	return 1;
}

void InitFriends()
{
	InitNetBase("");
	if((g_nFriends_PosX==-1)&&(g_nFriends_PosY==-1))
	{
		RECT rt;
		MyWin_GetScreenRect(&rt);
		g_nFriends_SizeX=150;
		g_nFriends_SizeY=200;
		g_nFriends_PosX=rt.right-(g_nFriends_SizeX+MYWIN_BORDER_SIZE*2);
		g_nFriends_PosY=0;
	}
		
	if(g_bShowFriends)	
		CreateFriends();
}

void FormatFriends()
{
	int i,j;
	for(i=0;i<GetGroupCount();i++)
	{
		TV_INSERTSTRUCT tvi;

		tvi.item.mask=TVIF_TEXT;
		tvi.item.cchTextMax=30;
		tvi.item.pszText=(char*)GetGroupName(i);

		tvi.hParent=TVI_ROOT;
		tvi.hInsertAfter=TVI_LAST;

		HTREEITEM hRootItem=TreeView_InsertItem(g_hWnd_Tree,&tvi);
		for(j=0;j<GetFriendsCount(i);j++)
		{
			TV_INSERTSTRUCT tvi;

			tvi.item.mask=TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_IMAGE ;
			tvi.item.cchTextMax=30;
			tvi.item.pszText=(char*)GetFriendsName(i,j);
			tvi.item.iImage=-1;
			tvi.item.iSelectedImage=-1;
			tvi.hParent=hRootItem;
			tvi.hInsertAfter=TVI_LAST;

			HTREEITEM hRootItem=TreeView_InsertItem(g_hWnd_Tree,&tvi);
		}
	}
}

/*
#define CX_BITMAP 16
#define CY_BITMAP 16
#define NUM_BITMAPS 3

BOOL InitTreeView(HWND hwndTV) 
{ 
	int g_nOpen,g_nClosed,g_nDocument;

    HIMAGELIST himl;  // handle to image list 
    HBITMAP hbmp;     // handle to bitmap 

    // Create the image list. 
    if ((himl = ImageList_Create(CX_BITMAP, CY_BITMAP, 
        FALSE, NUM_BITMAPS, 0)) == NULL) 
        return FALSE; 

    // Add the open file, closed file, and document bitmaps. 
    hbmp = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_OPEN_FILE));
    g_nOpen = ImageList_Add(himl, hbmp, (HBITMAP) NULL); 
    DeleteObject(hbmp); 

    hbmp = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CLOSED_FILE)); 
    g_nClosed = ImageList_Add(himl, hbmp, (HBITMAP) NULL); 
    DeleteObject(hbmp); 

    hbmp = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_DOCUMENT)); 
    g_nDocument = ImageList_Add(himl, hbmp, (HBITMAP) NULL); 
    DeleteObject(hbmp); 

    // Fail if not all of the images were added. 
    if (ImageList_GetImageCount(himl) < 3) 
        return FALSE; 

    // Associate the image list with the tree view control. 
    TreeView_SetImageList(hwndTV, himl, TVSIL_NORMAL ); 
	TreeView_SetIndent(hwndTV,0);	
    return TRUE; 
} 


WNDPROC Old_Friends_Proc;

LRESULT CALLBACK Friends_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
	case WM_NOTIFY:
		{
			int idCtrl = (int) wParam; 
			LPNMTREEVIEW pnmtv = (LPNMTREEVIEW) lParam ;
			if(pnmtv->hdr.code==TVN_ITEMEXPANDED )
			{
				pnmtv->itemNew.iImage=!pnmtv->itemNew.iImage;
				TreeView_SetItem(g_hWnd_Tree,&pnmtv->itemNew);

				_RPT0(_CRT_WARN,"notify !");
			}
		}
		break;
		default:				return Old_Friends_Proc(hWnd, message, wParam, lParam);
	}
	return 0;
}
*/

BOOL CreateFriends()
{
	InitCommonControls();
	g_bShowFriends=true;
	g_nWin_Friends=MyWin_CreateWindow(g_nFriends_PosX,g_nFriends_PosY,
		g_nFriends_SizeX,
		g_nFriends_SizeY,
		0,MYWS_SYSTEMBUTTONS | MYWS_SHOWCAPTION | MYWS_RESIZE,Friends_Listner,NULL);
	g_hWnd_Friends=MyWin_GetHWND(g_nWin_Friends);
//	Old_Friends_Proc=SubclassWindow(g_hWnd_Friends,Friends_WndProc);

	g_hWnd_Tree=CreateWindowEx(NULL,WC_TREEVIEW,"",
		TVS_LINESATROOT | TVS_HASLINES | TVS_HASBUTTONS |
		WS_VISIBLE|WS_CHILD|WS_DLGFRAME   
		,0,0,0,0,
		g_hWnd_Friends,NULL,g_hInst,NULL);
//	InitTreeView(g_hWnd_Tree);
	Friends_MoveResize();
	SetWindowFont(g_hWnd_Tree, g_hFont, true);
	FormatFriends();
	Friends_ColorChange(0);
	MyWin_ShowWindow(g_nWin_Friends,TRUE);

	return TRUE;
}

BOOL DestroyFriends()
{
	g_bShowFriends=false;
	DestroyWindow(g_hWnd_Friends);
	return TRUE;
}