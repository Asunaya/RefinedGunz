#include "RSGlobal_Dll.h"

PFNInitDisplay	RSInitDisplay=NULL;
PFNCloseDisplay RSCloseDisplay=NULL;
PFNFlip		RSFlip=NULL;
PFNDrawFace	RSDrawFace=NULL;
PFNDrawFaceArray	RSDrawFaceArray=NULL;
PFNDrawLine	RSDrawLine=NULL;
PFNDrawTriangle RSDrawTriangle=NULL;
PFNCreateTexture RSCreateTexture=NULL;
PFNDeleteTexture RSDeleteTexture=NULL;
PFNSetTexture	RSSetTexture=NULL;
PFNLock		RSLock=NULL,RSLockZ=NULL;
PFNUnlock	RSUnlock=NULL,RSUnlockZ=NULL;
PFNSetRenderRect	RSSetRenderRect=NULL;
PFNGetDriverInformation	RSGetDriverInformation=NULL;
PFNSetAlphaState	RSSetAlphaState=NULL;
PFNSetZBufferState	RSSetZBufferState=NULL;
PFNSetChromaKeyState RSSetChromaKeyState=NULL;
PFNSetAdvanceSetting RSSetAdvanceSetting=NULL;
PFNSetFogState RSSetFogState = NULL;
PFNSetTextureWrapState RSSetTextureWrapState = NULL;
PFNDrawStrip		RSDrawStrip = NULL, RSDrawLineStrip = NULL;
PFNSetCullState		RSSetCullState = NULL;

HINSTANCE __g_rshLibrary=NULL;

HINSTANCE RSGetDllHandle()
{
	return __g_rshLibrary;
}

bool RSInitDll(const char *DllName,const char *AdvanceString)
{
	__g_rshLibrary = LoadLibrary(DllName);
	if( !__g_rshLibrary ) return false;

	RSInitDisplay=(PFNInitDisplay)GetProcAddress(__g_rshLibrary,"_InitDisplay@8");
	RSCloseDisplay=(PFNCloseDisplay)GetProcAddress(__g_rshLibrary,"_CloseDisplay@0");
	RSFlip=(PFNFlip)GetProcAddress(__g_rshLibrary,"_Flip@0");
	RSDrawFace=(PFNDrawFace)GetProcAddress(__g_rshLibrary,"_DrawFace@4");
	RSDrawFaceArray=(PFNDrawFaceArray)GetProcAddress(__g_rshLibrary,"_DrawFaceArray@8");
	RSDrawLine=(PFNDrawLine)GetProcAddress(__g_rshLibrary,"_DrawLine@8");
	RSDrawTriangle=(PFNDrawTriangle)GetProcAddress(__g_rshLibrary,"_DrawTriangle@12");
	RSCreateTexture=(PFNCreateTexture)GetProcAddress(__g_rshLibrary,"_CreateTexture@20");
	RSDeleteTexture=(PFNDeleteTexture)GetProcAddress(__g_rshLibrary,"_DeleteTexture@4");
	RSSetTexture=(PFNSetTexture)GetProcAddress(__g_rshLibrary,"_SetTexture@4");
	RSLock=(PFNLock)GetProcAddress(__g_rshLibrary,"_Lock@4");
	RSUnlock=(PFNUnlock)GetProcAddress(__g_rshLibrary,"_Unlock@0");
	RSSetRenderRect=(PFNSetRenderRect)GetProcAddress(__g_rshLibrary,"_SetRenderRect@16");
	RSGetDriverInformation=(PFNGetDriverInformation)GetProcAddress(__g_rshLibrary,"_GetDriverInformation@0");

	RSSetAlphaState=(PFNSetAlphaState)GetProcAddress(__g_rshLibrary,"_SetAlphaState@4");
	RSSetZBufferState=(PFNSetZBufferState)GetProcAddress(__g_rshLibrary,"_SetZBufferState@8");
	RSSetChromaKeyState=(PFNSetChromaKeyState)GetProcAddress(__g_rshLibrary,"_SetChromaKeyState@4");
	RSLockZ=(PFNLock)GetProcAddress(__g_rshLibrary,"_LockZ@4");
	RSUnlockZ=(PFNUnlock)GetProcAddress(__g_rshLibrary,"_UnlockZ@0");
	RSSetFogState=(PFNSetFogState)GetProcAddress(__g_rshLibrary, "_SetFogState@8");
	RSSetTextureWrapState=(PFNSetTextureWrapState)GetProcAddress(__g_rshLibrary,"_SetTextureWrapState@4");
	RSDrawStrip=(PFNDrawStrip)GetProcAddress(__g_rshLibrary,"_DrawStrip@4");
	RSDrawLineStrip=(PFNDrawStrip)GetProcAddress(__g_rshLibrary,"_DrawLineStrip@4");
	RSSetCullState=(PFNSetCullState)GetProcAddress(__g_rshLibrary,"_SetCullState@4");

	RSSetAdvanceSetting=(PFNSetAdvanceSetting)GetProcAddress(__g_rshLibrary,"_SetAdvanceSetting@4");

	if(!RSInitDisplay || !RSCloseDisplay || !RSFlip || !RSDrawFace || !RSDrawLine ||
		!RSDrawTriangle || !RSCreateTexture ||
		!RSDeleteTexture || !RSSetTexture || !RSLock || !RSUnlock || !RSSetRenderRect || 
		!RSGetDriverInformation || !RSSetAlphaState || !RSSetChromaKeyState)
		return false;

	if(RSSetAdvanceSetting) RSSetAdvanceSetting(AdvanceString);
	return true;
}

bool RSDestroyDll()
{
	if(__g_rshLibrary)
	{
		FreeLibrary(__g_rshLibrary);
		__g_rshLibrary=NULL;
	}
	return true;
}