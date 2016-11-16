#pragma once

#ifdef _DEBUG
#pragma comment(lib,"cmld.lib")
#pragma comment(lib,"mint2d.lib")
#pragma comment(lib,"RealSoundd.lib")
#pragma comment(lib,"RealSpace2d.lib")
#pragma comment(lib,"CSCommonD.lib")
#pragma comment(lib,"SafeUDPd.lib")
#pragma comment(lib, "dxerr9.lib")

#else
#pragma comment(lib,"cml.lib")
#pragma comment(lib,"mint2.lib")
#pragma comment(lib,"RealSound.lib")
#pragma comment(lib,"RealSpace2.lib")
#pragma comment(lib,"CSCommon.lib")
#pragma comment(lib,"SafeUDP.lib")
#pragma comment(lib, "dxerr9.lib")

#endif

#include "Config.h"

#ifdef VOICECHAT
#pragma comment(lib, "opus.lib")
#pragma comment(lib, "celt.lib")
#pragma comment(lib, "silk_common.lib")
#pragma comment(lib, "silk_fixed.lib")
#pragma comment(lib, "silk_float.lib")
#pragma comment(lib, "portaudio.lib")
#endif

#pragma comment(lib,"winmm.lib")

#include "BulletCollisionLibs.h"
