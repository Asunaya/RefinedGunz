#ifndef __MEMO_H
#define __MEMO_H

#include <windows.h>

struct MemoInfo_t
{
	int		nWin;
	HWND	hWnd,hWnd_Edit;

	MemoInfo_t() { hWnd=NULL;hWnd_Edit=NULL; }
};

struct MemoDatHeader_t
{
	int Header,Version;
};

struct MemoHeader_t {
	int x,y,sx,sy,color;
	DWORD dwFlags;
	int nStringLength;
};

#define MF_MINIMIZED	1
#define MF_ALWAYSONTOP	2

void Init_Memo();
void ShutDown_Memo();

BOOL NewMemo(int nColor=0,MemoHeader_t *Header=NULL,const char *szMemo=NULL);

#endif