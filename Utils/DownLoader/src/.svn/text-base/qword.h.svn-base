#pragma once

#define QWORD __int64
#define MAKEQWORD(High, Low)	(((QWORD)High << 32) | ((QWORD)Low))
#define HIGHDWORD(QWord)		((DWORD)(QWord >> 32))
#define LOWDWORD(QWord)			((DWORD)(QWord & 0xFFFFFFFF))
