#include "stdafx.h"
#include <stdio.h>
#include <time.h>
#include <crtdbg.h>

#include "HelloWin.h"
#include "MyWinBase.h"
#include "Calendar.h"
#include "resource.h"

#define CALENDAR_WINDOW_CLIENT_SIZE_X 175
#define	CALENDAR_WINDOW_CLIENT_SIZE_Y 145
#define BUTTON_HEIGHT_FROM_BOTTOM 12
#define RATIOOFBUTTONPOSITION	7
#define LEFTBUTTON_CENTER	((CALENDAR_WINDOW_CLIENT_SIZE_X/RATIOOFBUTTONPOSITION)+MYWIN_BORDER_SIZE)
#define RIGHTBUTTON_CENTER	(((RATIOOFBUTTONPOSITION-1)*CALENDAR_WINDOW_CLIENT_SIZE_X/RATIOOFBUTTONPOSITION)+MYWIN_BORDER_SIZE)

#define CALENDAR_BUTTON_LEFT	0
#define CALENDAR_BUTTON_RIGHT	1

HWND g_hWnd_Calendar=NULL;
int	g_nWin_Calendar=NULL;
int	g_nCalendar_PosX=-1,g_nCalendar_PosY=-1,g_nCalendar_Col;

char szCalendarCaptionFormat[]="yyyy년 MMM월";
char szCalendarCaption[256];
char szCalendarWindowClass[]="dCalendar";

HBITMAP g_hBitmap_Calendar_Buttons[2][3][2]={0,};

void Init_Calendar()
{

	if((g_nCalendar_PosX==-1)&&(g_nCalendar_PosY==-1))
	{
		RECT rt;
		MyWin_GetScreenRect(&rt);
		g_nCalendar_PosX=rt.right-(CALENDAR_WINDOW_CLIENT_SIZE_X+MYWIN_BORDER_SIZE*2);
		g_nCalendar_PosY=rt.bottom-(CALENDAR_WINDOW_CLIENT_SIZE_Y+MYWIN_BORDER_SIZE*2+g_nMyWin_CaptionSize);
	}
		
	if(g_bShowCalendar)	
		CreateCalendar();
}

void ShutDown_Calendar()
{
	DestroyWindow(g_hWnd_Calendar);
}

// Calendar globals
char name_days[7][7][4];
int g_nYear,g_nMonth,g_nToday;

void SYSTEMTIME_From_tm(SYSTEMTIME *s,const tm *t)
{
	s->wYear=t->tm_year+1900;
	s->wMonth=t->tm_mon+1;
	s->wDay=t->tm_mday;
	s->wHour=t->tm_hour;
	s->wMinute=t->tm_min;
	s->wSecond=t->tm_sec;
	s->wMilliseconds=0;
	s->wDayOfWeek=t->tm_wday;
}

void FormatCalendar(int nYear,int nMonth,int nDay)
{
	int i,j;

	// get date format;
	{
		SYSTEMTIME lt;
		GetLocalTime(&lt);

		int nMonday=lt.wDay-lt.wDayOfWeek;
		if(nMonday<1) nMonday+=7;
		nMonday=nMonday % 7;

		for(i=0;i<7;i++)
		{
			lt.wDay=nMonday+i;
			GetDateFormat(LOCALE_USER_DEFAULT,NULL,&lt,"ddd",name_days[0][i],4);
		}
	}

	// get yyyy 년 mmm 월 caption
	{
		tm when={ 0, 0, 0, nDay, nMonth-1, nYear-1900};
		time_t targett=mktime(&when);
		when=*localtime(&targett);

		// set window's caption
		SYSTEMTIME localtime;
		SYSTEMTIME_From_tm(&localtime,&when);
		GetDateFormat(LOCALE_USER_DEFAULT,NULL ,&localtime,szCalendarCaptionFormat,
						szCalendarCaption,sizeof(szCalendarCaption));
	}

	tm when={ 0, 0, 0, nDay, nMonth-1, nYear-1900};
	time_t targett=mktime(&when);
	when=*localtime(&targett);

	g_nYear=when.tm_year+1900;
	g_nMonth=when.tm_mon+1;
	g_nToday=when.tm_mday;

	when.tm_mday=((when.tm_mday-when.tm_wday+7 -1) %7 +1 -7);

	for(i=1;i<7;i++)
		for(j=0;j<7;j++)
		{
			time_t ttt=mktime(&when);
			tm *tt2=localtime(&ttt);
			_itoa(tt2->tm_mday,name_days[i][j],10);
			when.tm_mday++;
		}
}

void Calendar_Paint(int nWin,HDC hdc)
{

#define Y_FIRST_LINE 18
#define Y_SECOND_LINE 120

	RECT rt;
	MyWin_GetClientRect(nWin,&rt);
	FillRect(hdc,&rt,g_hBrush_Memo[MyWin_GetInfo(nWin)->nColor]);
	MyWin_GetRect(nWin,&rt);
	rt.right-=rt.left+MYWIN_BORDER_SIZE;
	rt.bottom-=rt.top+MYWIN_BORDER_SIZE;
	rt.left=MYWIN_BORDER_SIZE;
	rt.top=MYWIN_BORDER_SIZE+g_nMyWin_CaptionSize;
	
	SelectObject(hdc, g_hFont_Bold); 
	SetBkMode(hdc,TRANSPARENT);
	SetTextColor(hdc,g_Color_Caption);

	SIZE size;
	GetTextExtentPoint32(hdc,szCalendarCaption,strlen(szCalendarCaption),&size);
 
	if(MyWin_GetInfo(g_nWin_Calendar)->bMinizied)
		TextOut(hdc,1,1,szCalendarCaption,strlen(szCalendarCaption));
	else
		TextOut(hdc,(rt.right+rt.left-size.cx)/2,
			(CALENDAR_WINDOW_CLIENT_SIZE_Y+Y_SECOND_LINE-size.cy)/2+1+rt.top,
			szCalendarCaption,strlen(szCalendarCaption));

	SelectObject(hdc, g_hFont); 
	SetTextColor(hdc,RGB(0,0,0));

	int sizex=rt.right-rt.left;
	int nStepX=sizex/7;

#define GAP_MINIMUM 2
#define GAP_LINE	5
#define GAPY 16


#define COLOR_NOTMAINTEXT	RGB(128,128,128)
#define COLOR_MAINTEXT		RGB(0,0,0)
#define COLOR_HOLIDAY		RGB(255,0,0)
#define COLOR_TODAY			RGB(128,0,255)

	BOOL inMain=FALSE;

	int i,j;
	for(i=0;i<7;i++)
	{
		for(j=0;j<7;j++)
		{
			if((name_days[i][j][0]=='1')&&(name_days[i][j][1]==0))
				inMain=!inMain;
			
			SetTextColor(hdc,inMain?COLOR_MAINTEXT:COLOR_NOTMAINTEXT);
			if(i==0) SetTextColor(hdc,COLOR_MAINTEXT);
			if(j==0) SetTextColor(hdc,COLOR_HOLIDAY);

			if(inMain&&
				(atoi(name_days[i][j])==g_nToday)) SetTextColor(hdc,COLOR_TODAY);

			SIZE textsize;
			GetTextExtentPoint32(hdc,name_days[i][j],strlen(name_days[i][j]),&textsize);
			TextOut(hdc,
					rt.left+j*nStepX+(nStepX-textsize.cx)/2,
					rt.top+	i*GAPY+GAP_MINIMUM+GAP_LINE*(i>0),
					name_days[i][j],strlen(name_days[i][j]));
		}
	}

	DrawLine(hdc,rt.left,Y_FIRST_LINE+rt.top,rt.right,Y_FIRST_LINE+rt.top,g_hPen_Light);
	DrawLine(hdc,rt.left,Y_SECOND_LINE+rt.top,rt.right,Y_SECOND_LINE+rt.top,g_hPen_Light);
	DrawLine(hdc,rt.left,Y_FIRST_LINE+1+rt.top,rt.right,Y_FIRST_LINE+1+rt.top,g_hPen_Shadow);
	DrawLine(hdc,rt.left,Y_SECOND_LINE+1+rt.top,rt.right,Y_SECOND_LINE+1+rt.top,g_hPen_Shadow);
}

Calendar_Info_t *GetCalendarInfo()
{
	static Calendar_Info_t it;
	if(g_bShowCalendar)
	{
		RECT rt;
		MyWin_GetRect(g_nWin_Calendar,&rt);
		it.xPos=rt.left;
		it.yPos=rt.top;
	}
	else
	{
		it.xPos=g_nCalendar_PosX;
		it.yPos=g_nCalendar_PosY;
	}
	it.nColor=0;
	it.dwFlags=0;
	if(g_bShowCalendar)
		it.dwFlags|=CF_SHOW;

	return &it;
}

void Calendar_Close()
{
	g_bShowCalendar=false;
}

void Calendar_Destroy()
{
	WriteConfiguration();
	RELEASE(g_hBitmap_Calendar_Buttons[0][0][0]);
	RELEASE(g_hBitmap_Calendar_Buttons[0][0][1]);
	RELEASE(g_hBitmap_Calendar_Buttons[0][1][0]);
	RELEASE(g_hBitmap_Calendar_Buttons[0][1][1]);
	RELEASE(g_hBitmap_Calendar_Buttons[0][2][0]);
	RELEASE(g_hBitmap_Calendar_Buttons[0][2][1]);
	RELEASE(g_hBitmap_Calendar_Buttons[1][0][0]);
	RELEASE(g_hBitmap_Calendar_Buttons[1][0][1]);
	RELEASE(g_hBitmap_Calendar_Buttons[1][1][0]);
	RELEASE(g_hBitmap_Calendar_Buttons[1][1][1]);
	RELEASE(g_hBitmap_Calendar_Buttons[1][2][0]);
	RELEASE(g_hBitmap_Calendar_Buttons[1][2][1]);
}

void Calendar_MoveResize()
{
	RECT rt;
	MyWin_GetRect(g_nWin_Calendar,&rt);
	g_nCalendar_PosX=rt.left;
	g_nCalendar_PosY=rt.top;
	_RPT0(_CRT_WARN,"Config saved\n");
	WriteConfiguration();
}

void Calendar_SkinChange()
{
	RECT rt;
	MyWin_GetClientRect(g_nWin_Calendar,&rt);
	if(g_hWnd_Calendar)
		MoveWindow(g_hWnd_Calendar,g_nCalendar_PosX,g_nCalendar_PosY,
		CALENDAR_WINDOW_CLIENT_SIZE_X+MYWIN_BORDER_SIZE*2,
		CALENDAR_WINDOW_CLIENT_SIZE_Y+MYWIN_BORDER_SIZE*2+g_nMyWin_CaptionSize,true);
}

void Calendar_ButtonPressed(int nButton)
{
	_RPT1(_CRT_WARN,"%d button pressed.\n",nButton);
	switch(nButton)
	{
	case CALENDAR_BUTTON_LEFT :
		g_nMonth--;
		FormatCalendar(g_nYear,g_nMonth,g_nToday);
		break;
	case CALENDAR_BUTTON_RIGHT :
		g_nMonth++;
		FormatCalendar(g_nYear,g_nMonth,g_nToday);
		break;

	}
}

int Calendar_Listner(MYWINDOWMESSAGE nMessage,void* Param,void *pUserData)
{
	switch(nMessage)
	{
	case MYWM_PAINT			: Calendar_Paint(g_nWin_Calendar,(HDC)Param);break;
	case MYWM_CLOSE			: Calendar_Close();break;
	case MYWM_DESTROY		: Calendar_Destroy();break;
	case MYWM_MOVERESIZE	: Calendar_MoveResize();break;
	case MYWM_SKINCHANGE	: Calendar_SkinChange();break;
	case MYWM_BUTTONPRESSED	: Calendar_ButtonPressed((int)Param);break;
	}
	return 1;
}

BOOL CreateCalendar()
{
	g_hBitmap_Calendar_Buttons[0][0][0]=LoadBitmap(MAKEINTRESOURCE(IDB_arrow_l),false);
	g_hBitmap_Calendar_Buttons[0][0][1]=LoadBitmap(MAKEINTRESOURCE(IDB_arrow_l_mask),false);
	g_hBitmap_Calendar_Buttons[0][1][0]=LoadBitmap(MAKEINTRESOURCE(IDB_arrow_l_pressed),false);
	g_hBitmap_Calendar_Buttons[0][1][1]=LoadBitmap(MAKEINTRESOURCE(IDB_arrow_l_pressed_mask),false);
	g_hBitmap_Calendar_Buttons[0][2][0]=LoadBitmap(MAKEINTRESOURCE(IDB_arrow_l_hover),false);
	g_hBitmap_Calendar_Buttons[0][2][1]=LoadBitmap(MAKEINTRESOURCE(IDB_arrow_l_hover_mask),false);
	g_hBitmap_Calendar_Buttons[1][0][0]=LoadBitmap(MAKEINTRESOURCE(IDB_arrow_r),false);
	g_hBitmap_Calendar_Buttons[1][0][1]=LoadBitmap(MAKEINTRESOURCE(IDB_arrow_r_mask),false);
	g_hBitmap_Calendar_Buttons[1][1][0]=LoadBitmap(MAKEINTRESOURCE(IDB_arrow_r_pressed),false);
	g_hBitmap_Calendar_Buttons[1][1][1]=LoadBitmap(MAKEINTRESOURCE(IDB_arrow_r_pressed_mask),false);
	g_hBitmap_Calendar_Buttons[1][2][0]=LoadBitmap(MAKEINTRESOURCE(IDB_arrow_r_hover),false);
	g_hBitmap_Calendar_Buttons[1][2][1]=LoadBitmap(MAKEINTRESOURCE(IDB_arrow_r_hover_mask),false);
	
	g_bShowCalendar=true;

	{	// get current time and format calendar
        struct tm *newtime;
        time_t long_time;

        time( &long_time );
        newtime = localtime( &long_time );
		FormatCalendar(newtime->tm_year+1900,newtime->tm_mon+1,newtime->tm_mday);
	}

	g_nWin_Calendar=MyWin_CreateWindow(g_nCalendar_PosX,g_nCalendar_PosY,
		CALENDAR_WINDOW_CLIENT_SIZE_X+MYWIN_BORDER_SIZE*2,
		CALENDAR_WINDOW_CLIENT_SIZE_Y+MYWIN_BORDER_SIZE*2+g_nMyWin_CaptionSize,
		0,MYWS_SYSTEMBUTTONS | MYWS_SHOWCAPTION ,Calendar_Listner,NULL);
	g_hWnd_Calendar=MyWin_GetHWND(g_nWin_Calendar);

	MyButton_t btn;
	btn.hBitmaps[0][0]=g_hBitmap_Calendar_Buttons[0][0][0];
	btn.hBitmaps[0][1]=g_hBitmap_Calendar_Buttons[0][0][1];
	btn.hBitmaps[1][0]=g_hBitmap_Calendar_Buttons[0][1][0];
	btn.hBitmaps[1][1]=g_hBitmap_Calendar_Buttons[0][1][1];
	btn.hBitmaps[2][0]=g_hBitmap_Calendar_Buttons[0][2][0];
	btn.hBitmaps[2][1]=g_hBitmap_Calendar_Buttons[0][2][1];
	btn.rect.top=-BUTTON_HEIGHT_FROM_BOTTOM-GetBitmapHeight(btn.hBitmaps[0][0])/2;
	btn.rect.left=LEFTBUTTON_CENTER-GetBitmapWidth(btn.hBitmaps[0][0])/2;
	btn.rect.bottom=btn.rect.top+GetBitmapHeight(btn.hBitmaps[0][0]);
	btn.rect.right=btn.rect.left+GetBitmapWidth(btn.hBitmaps[0][0]);
	btn.nID=CALENDAR_BUTTON_LEFT;
	MyWin_RegisterButton(g_nWin_Calendar,&btn);

	btn.hBitmaps[0][0]=g_hBitmap_Calendar_Buttons[1][0][0];
	btn.hBitmaps[0][1]=g_hBitmap_Calendar_Buttons[1][0][1];
	btn.hBitmaps[1][0]=g_hBitmap_Calendar_Buttons[1][1][0];
	btn.hBitmaps[1][1]=g_hBitmap_Calendar_Buttons[1][1][1];
	btn.hBitmaps[2][0]=g_hBitmap_Calendar_Buttons[1][2][0];
	btn.hBitmaps[2][1]=g_hBitmap_Calendar_Buttons[1][2][1];
	btn.rect.top=-BUTTON_HEIGHT_FROM_BOTTOM-GetBitmapHeight(btn.hBitmaps[0][0])/2;
	btn.rect.left=RIGHTBUTTON_CENTER-GetBitmapWidth(btn.hBitmaps[0][0])/2;
	btn.rect.bottom=btn.rect.top+GetBitmapHeight(btn.hBitmaps[0][0]);
	btn.rect.right=btn.rect.left+GetBitmapWidth(btn.hBitmaps[0][0]);
	btn.nID=CALENDAR_BUTTON_RIGHT;
	MyWin_RegisterButton(g_nWin_Calendar,&btn);

	MyWin_ShowWindow(g_nWin_Calendar,TRUE);
	return TRUE;
}

BOOL DestroyCalendar()
{
	g_bShowCalendar=false;
	WriteConfiguration();
	DestroyWindow(g_hWnd_Calendar);
	return TRUE;
}
