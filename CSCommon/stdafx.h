// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef _STDAFX_H
#define _STDAFX_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "targetver.h"

#define POINTER_64 __ptr64

#ifdef MFC
#include <afxdb.h>
#include <afxtempl.h>
#include <afxdtctl.h>
#endif

#include <Winsock2.h>
#include <mswsock.h>
#include <crtdbg.h>
#include <windows.h>
#include <stdlib.h>

#include <string.h>
#include <map>
#include <list>
#include <vector>
#include <algorithm>

#define _QUEST

#define _QUEST_ITEM
#define _MONSTER_BIBLE

#include "MLocaleDefine.h"
#include "MDebug.h"
#include "MXml.h"

#include "MUID.h"
#include "MSharedCommandTable.h"
#include "MCommand.h"
#include "MCommandParameter.h"
#include "MCommandCommunicator.h"
#include "MErrorTable.h"
#include "MObject.h"

#include "SafeString.h"
#include "GlobalTypes.h"

#include <cassert>
#define ASSERT assert

#endif