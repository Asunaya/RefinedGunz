#include "stdafx.h"
#include <windowsx.h>
#include <time.h>
#include <io.h>
#include <stdio.h>
#include <crtdbg.h>

#include "MyWinBase.h"
#include "Memo.h"
#include "resource.h"

#define MAX_MEMO MAX_MYWIN

#define MEMO_HEADER		666
#define MEMO_VERSION	1

#define MEMO_FILE_NAME	"memos.dat"

int g_nMemo=0;
MemoInfo_t *g_MemoArray[MAX_MEMO];
bool g_bLoading=false;

void GetSerializedData(int nMemo,LPBYTE *data,int *nByte)
{
	MyWinInfo_t *it = MyWin_GetInfo(g_MemoArray[nMemo]->nWin);

	int nLength=GetWindowTextLength(g_MemoArray[nMemo]->hWnd_Edit)+1;
	*nByte=nLength+sizeof(MemoHeader_t);
	*data=new BYTE[*nByte];
	MemoHeader_t *h=(MemoHeader_t*)*data;
	RECT rt;
	MyWin_GetRect(g_MemoArray[nMemo]->nWin,&rt);
	h->x=rt.left;
	h->y=rt.top;
	h->dwFlags=0;
	if(it->bMinizied)
	{
		h->dwFlags|=MF_MINIMIZED;
		h->sx=it->nOriginalSizeX;
		h->sy=it->nOriginalSizeY;
	}
	else
	{
		h->sx=rt.right-rt.left;
		h->sy=rt.bottom-rt.top;
	}
	if(it->bAlwaysOnTop)
		h->dwFlags|=MF_ALWAYSONTOP;
	
	h->color=it->nColor;
	h->nStringLength=nLength;
	GetWindowText(g_MemoArray[nMemo]->hWnd_Edit,(char*)(*data+sizeof(MemoHeader_t)),nLength);
}

void ShiftBackUpFiles()
{
	char filename1[_MAX_PATH],filename2[_MAX_PATH];

	remove(MEMO_FILE_NAME".5");

	for(int i=4;i>=0;i--)
	{
		sprintf(filename1,"%s.%d",MEMO_FILE_NAME,i);
		sprintf(filename2,"%s.%d",MEMO_FILE_NAME,i+1);
		rename(filename1,filename2);
	}
}

void CheckBackup()
{
    struct _finddata_t c_file;
    long hFile;

    hFile = _findfirst( MEMO_FILE_NAME".0", &c_file );
   _findclose( hFile );

	time_t current;
	time(&current);
	double elapsed_time = difftime(current,c_file.time_write);

	// 백업파일이 없거나.. 하루가 지났다면..
	if(hFile==-1L || elapsed_time>=60*60*24) 
	{
		ShiftBackUpFiles();
		int result=rename(MEMO_FILE_NAME,MEMO_FILE_NAME".0");
		_ASSERT(result==0);
	}
}

void SaveAllMemo()
{
	if(!g_bLoading)
	{
		CheckBackup();

		FILE *file;
		file=fopen(MEMO_FILE_NAME,"wb+");
		if(file==NULL) return;

		int i;
		int		nByte;
		LPBYTE	data=NULL;
		
		MemoDatHeader_t header={MEMO_HEADER,MEMO_VERSION};
		fwrite(&header,sizeof(MemoDatHeader_t),1,file);
		fwrite(&g_nMemo,sizeof(int),1,file);
		for(i=0;i<g_nMemo;i++)
		{
			GetSerializedData(i,&data,&nByte);
			fwrite(data,nByte,1,file);
			delete data;
		}
		fclose(file);
	}
//*/
}

void LoadAllMemo()
{
	g_bLoading=true; 
	FILE *file;
	file=fopen(MEMO_FILE_NAME,"rb");
	if(file==NULL) {
		g_bLoading=false;
		return;
	}

	MemoDatHeader_t header;
	fread(&header,sizeof(MemoDatHeader_t),1,file);
	if(header.Header!=MEMO_HEADER) {
		g_bLoading=false;
		return;
	}

	switch(header.Version)
	{
	case 1:
		{
			int i,nMemo;
			fread(&nMemo,sizeof(int),1,file);

			MemoHeader_t Header;
			for(i=0;i<nMemo;i++)
			{
				int nRead;
				nRead=fread(&Header,sizeof(MemoHeader_t),1,file);
				_ASSERT(nRead);
				char *buf=new char[Header.nStringLength+1];
				nRead=fread(buf,Header.nStringLength,1,file);
				_ASSERT(nRead);
				NewMemo(0,&Header,buf);
				delete buf;
			}
		}break;
	}
	fclose(file);
	g_bLoading=false;
}

void Memo_MoveResize(int nWin,HWND hWnd_Edit)
{
	RECT rt;
	MyWin_GetClientRect(nWin,&rt);
	if(hWnd_Edit)
	{
		MoveWindow(hWnd_Edit,rt.left,rt.top+2,rt.right-rt.left,rt.bottom-rt.top-2,true);
		SaveAllMemo();
	}
}

int Memo_Close(void *pUserData)
{
	int nMemo=(int)pUserData;
	MemoInfo_t *mi=g_MemoArray[nMemo];
	if(Edit_GetTextLength(mi->hWnd_Edit))
	{
		int yesno=MessageBox(g_MemoArray[nMemo]->hWnd,"Really ?","Hello",MB_YESNO);
		if(yesno==IDNO) return 0;
	}
	return 1;
}

void Memo_Destroy(void *pUserData)
{
	int nMemo=(int)pUserData;
	delete g_MemoArray[nMemo];
	g_nMemo--;
	if(g_nMemo>nMemo)
	{
		g_MemoArray[nMemo]=g_MemoArray[g_nMemo];
		MyWin_SetUserData(g_MemoArray[nMemo]->nWin,(void*)nMemo);
	}
	SaveAllMemo();
}

void Memo_Paint(int nWin,HDC hdc)
{
	RECT rt;
	MyWin_GetClientRect(nWin,&rt);
	rt.bottom=rt.top+2;
	FillRect(hdc,&rt,g_hBrush_Memo[MyWin_GetInfo(nWin)->nColor]);
}

void Memo_SkinChange(MemoInfo_t *t)
{
	SetWindowFont(t->hWnd_Edit, g_hFont, true);
	Memo_MoveResize(t->nWin,t->hWnd_Edit);
}

void Memo_Deactivated(MemoInfo_t *t)
{
	if(Edit_GetModify(t->hWnd_Edit))
	{
		SaveAllMemo();
		Edit_SetModify(t->hWnd_Edit,false);
	}
}

int Memo_Listner(MYWINDOWMESSAGE nMessage,void* Param,void *pUserData)
{
	MemoInfo_t *t=g_MemoArray[(int)pUserData];
	if(!t) return 0;
	switch(nMessage)
	{
	case MYWM_PAINT			: Memo_Paint(t->nWin,(HDC)Param);break;
	case MYWM_MOVERESIZE	: Memo_MoveResize(t->nWin,t->hWnd_Edit);break;
	case MYWM_CLOSE			: return Memo_Close(pUserData);break;
	case MYWM_DESTROY		: Memo_Destroy(pUserData);break;
	case MYWM_SKINCHANGE	: Memo_SkinChange(t);break;
	case MYWM_DEACTIVATED	: Memo_Deactivated(t);break;
	case MYWM_FOCUS			: ((int)Param==1)? SetFocus(t->hWnd_Edit): SetFocus(t->hWnd);break;
	}
	return 1;
}

BOOL NewMemo(int nColor,MemoHeader_t *Header,const char *szMemo)
{

#define DEFAULT_MEMO_SIZE_X	200
#define DEFAULT_MEMO_SIZE_Y	100

	// decide new memo's position
	int x,y;
	MyWin_GetNewMemoPosition(DEFAULT_MEMO_SIZE_X,DEFAULT_MEMO_SIZE_Y,&x,&y);

	// now create new memo
	int hMyWin=MyWin_CreateWindow(x,y,DEFAULT_MEMO_SIZE_X,DEFAULT_MEMO_SIZE_Y
		,Header?Header->color:nColor,
		MYWS_RESIZE | MYWS_SYSTEMBUTTONS,Memo_Listner,NULL);
	if(hMyWin==-1) return false;


	MemoInfo_t *t=new MemoInfo_t;
	g_MemoArray[g_nMemo]=t;
	t->nWin=hMyWin;
	t->hWnd=MyWin_GetHWND(hMyWin);

	t->hWnd_Edit=CreateWindow("Edit","",
		ES_MULTILINE|ES_AUTOVSCROLL|WS_VISIBLE|WS_CHILD,0,0,0,0,
		t->hWnd,NULL,g_hInst,NULL);

	Memo_MoveResize(hMyWin,t->hWnd_Edit);
	SetWindowFont(t->hWnd_Edit, g_hFont, true);
	MyWin_SetUserData(hMyWin,(void*)g_nMemo);

	if(Header) 
	{
		MoveWindow(t->hWnd,Header->x,Header->y,Header->sx,Header->sy,false);
		if((Header->dwFlags & MF_MINIMIZED) != 0)
			MyWin_Minimize(hMyWin,true);
		if((Header->dwFlags & MF_ALWAYSONTOP) != 0)
			MyWin_AlwaysOnTop(hMyWin,true);
	}

	if(szMemo) SetWindowText(t->hWnd_Edit,szMemo);	
	Edit_SetModify(t->hWnd_Edit,false);
	MyWin_ShowWindow(hMyWin,TRUE);

	g_nMemo++;

	return TRUE;
}

void Init_Memo()
{
	LoadAllMemo();
}

void ShutDown_Memo()
{
	SaveAllMemo();
	g_bLoading=true; 
	while(g_nMemo)
	{
		DestroyWindow(g_MemoArray[0]->hWnd);
	}
	g_bLoading=false; 
}
