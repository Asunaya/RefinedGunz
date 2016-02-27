#ifndef _RSGLOBAL_DLL_H
#define _RSGLOBAL_DLL_H

#include "windows.h"
#include "rutils.h"
#include "rsutils.h"

typedef BOOL (WINAPI *PFNInitDisplay) (HWND,void*);
typedef BOOL (WINAPI *PFNCloseDisplay) ();
typedef BOOL (WINAPI *PFNFlip) ();
typedef void (WINAPI *PFNDrawFace) (rface*);
typedef void (WINAPI *PFNDrawFaceArray) (rface*,int);
typedef void (WINAPI *PFNDrawLine) (rvertex *,rvertex *);
typedef void (WINAPI *PFNDrawTriangle) (rvertex *,rvertex *,rvertex *);
typedef int  (WINAPI *PFNCreateTexture) (int,int,char*,char*,bool);
typedef BOOL (WINAPI *PFNDeleteTexture) (int);
typedef BOOL (WINAPI *PFNSetTexture) (int);
typedef char* (WINAPI *PFNLock) (int*);
typedef void (WINAPI *PFNUnlock) ();
typedef char* (WINAPI *PFNLockZ) (int*);
typedef void (WINAPI *PFNUnlockZ) ();
typedef void (WINAPI *PFNSetRenderRect) (int,int,int,int);
typedef void (WINAPI *PFNSetAlphaState) (RSALPHASTYLE);
typedef void (WINAPI *PFNSetChromaKeyState) (BOOL);
typedef void (WINAPI *PFNSetZBufferState) (BOOL,BOOL);
typedef void (WINAPI *PFNSetAdvanceSetting) (const char*);
typedef void (WINAPI *PFNSetFogState) (BOOL,DWORD);
typedef void (WINAPI *PFNSetTextureWrapState) (BOOL);
typedef void (WINAPI *PFNDrawStrip) (rstrip*);
typedef void (WINAPI *PFNSetCullState) (RSCULLSTYPE);

typedef RSDRIVERINFORMATION * (WINAPI *PFNGetDriverInformation) ();

extern PFNInitDisplay	RSInitDisplay;
extern PFNCloseDisplay	RSCloseDisplay;
extern PFNFlip			RSFlip;
extern PFNDrawFace		RSDrawFace;
extern PFNDrawFaceArray	RSDrawFaceArray;
extern PFNDrawLine		RSDrawLine;
extern PFNDrawTriangle	RSDrawTriangle;
extern PFNCreateTexture RSCreateTexture;
extern PFNDeleteTexture RSDeleteTexture;
extern PFNSetTexture	RSSetTexture;
extern PFNLock			RSLock,RSLockZ;
extern PFNUnlock		RSUnlock,RSUnlockZ;
extern PFNSetRenderRect	RSSetRenderRect;
extern PFNGetDriverInformation RSGetDriverInformation;
extern PFNSetAlphaState	RSSetAlphaState;
extern PFNSetZBufferState	RSSetZBufferState;
extern PFNSetChromaKeyState RSSetChromaKeyState;
extern PFNSetAdvanceSetting RSSetAdvanceSetting;
extern PFNSetFogState	RSSetFogState;
extern PFNSetTextureWrapState RSSetTextureWrapState;
extern PFNDrawStrip		RSDrawStrip,RSDrawLineStrip;
extern PFNSetCullState	RSSetCullState;

bool RSInitDll(const char *DllName,const char *AdvanceString);
bool RSDestroyDll();
HINSTANCE RSGetDllHandle();

#endif