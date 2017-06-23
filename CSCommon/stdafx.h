#pragma once

#include "targetver.h"

#define POINTER_64 __ptr64

#include <WinSock2.h>
#include "MWindows.h"

#ifdef MFC
#include <afxdb.h>
#include <afxtempl.h>
#include <afxdtctl.h>
#endif

#include <stdlib.h>

#include <string.h>
#include <map>
#include <list>
#include <vector>
#include <algorithm>

#define _QUEST

#define _QUEST_ITEM
#define _MONSTER_BIBLE

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