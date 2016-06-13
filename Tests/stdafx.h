#pragma once

#include "targetver.h"

#define POINTER_64 __ptr64

// Windows 헤더 파일입니다.
#include <afxdb.h>
#include <afxtempl.h>
#include <afxdtctl.h>

#define WIN32_LEAN_AND_MEAN		// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#include <windows.h>

#include <winsock2.h>
#include <mswsock.h>

#include <crtdbg.h>

#include <mmsystem.h>
#include <shlwapi.h>
#include <shellapi.h>

// C의 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>
#include <cstddef>
#include <comutil.h>
#include <stdio.h>