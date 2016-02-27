#ifndef __CALENDAR_H
#define __CALENDAR_H

#include <windows.h>

struct Calendar_Info_t {
	int xPos,yPos;
	int nColor;
	DWORD dwFlags;
};

#define CF_MINIMIZED	1
#define CF_ALWAYSONTOP	2
#define CF_SHOW			3

extern int	g_nCalendar_PosX,g_nCalendar_PosY,g_nCalendar_Col;

void Init_Calendar();
void ShutDown_Calendar();

void FormatCalendar();
BOOL CreateCalendar();
BOOL DestroyCalendar();
Calendar_Info_t *GetCalendarInfo();

#endif