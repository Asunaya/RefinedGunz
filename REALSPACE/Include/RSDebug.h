#ifndef _RSDEBUG_H
#define _RSDEBUG_H

#include "rutils.h"

#define RS_LOGSTYLE_FILE 0x0001
#define RS_LOGSTYLE_CONSOLE 0x0002
#define RS_LOGSTYLE_DEBUGSTRING 0x0004

void initlog(int logmethodflags=RS_LOGSTYLE_CONSOLE|RS_LOGSTYLE_DEBUGSTRING);
void __cdecl rslog(const char *pFormat,...);
void __cdecl Msg( LPSTR fmt, ... );
void logMatrix(rmatrix &m);
void logVector(rvector &v);
char *GetLogHistory(int i);
int	GetLogHistoryCount();

#endif