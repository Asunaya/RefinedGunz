#include "stdafx.h"
#include <windows.h>
#include "MDebug.h"
#include "RealSpace2.h"
#include "RParticleSystem.h"
#include "RFont.h"
#include "RMeshUtil.h"
#include "RS2.h"
#include "RS2Vulkan.h"
#include "RBspObject.h"

#pragma comment(lib,"winmm.lib")

#ifndef _PUBLISH
#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);
#else
#define __BP(i,n) ;
#define __EP(i) ;
#endif

#define RTOOLTIP_GAP 700
static DWORD g_last_mouse_move_time;
static bool g_tool_tip;

bool IsToolTipEnable() {
	return g_tool_tip;
}

_NAMESPACE_REALSPACE2_BEGIN

extern HWND g_hWnd;

static bool g_bActive;

static RECT g_rcWindowBounds;
static WNDPROC g_WinProc;
static RFFUNCTION g_pFunctions[RF_ENDOFRFUNCTIONTYPE];

#ifdef _USE_GDIPLUS
#include "unknwn.h"
#include "gdiplus.h"

static Gdiplus::GdiplusStartupInput g_gdiplusStartupInput;
static ULONG_PTR g_gdiplusToken;
#endif

void RSetFunction(RFUNCTIONTYPE ft, RFFUNCTION pfunc) {
	g_pFunctions[ft] = pfunc;
}

bool RIsActive() {
	return GetActiveWindow() == g_hWnd;
}

RRESULT RFrame_Error()
{
	if (!g_pFunctions[RF_ERROR])
		return R_OK;
	return g_pFunctions[RF_ERROR](nullptr);
}

void RFrame_Create()
{
#ifdef _USE_GDIPLUS
	Gdiplus::GdiplusStartup(&g_gdiplusToken, &g_gdiplusStartupInput, NULL);
#endif
	GetWindowRect(g_hWnd, &g_rcWindowBounds);
}

void RFrame_Restore()
{
	RParticleSystem::Restore();
	if(g_pFunctions[RF_RESTORE])
		g_pFunctions[RF_RESTORE](NULL);
}

void RFrame_Destroy()
{
	if(g_pFunctions[RF_DESTROY])
		g_pFunctions[RF_DESTROY](NULL);
	RCloseDisplay();

#ifdef _USE_GDIPLUS
	Gdiplus::GdiplusShutdown(g_gdiplusToken);
#endif
}

void RFrame_Invalidate()
{
	RParticleSystem::Invalidate();
	if(g_pFunctions[RF_INVALIDATE])
		g_pFunctions[RF_INVALIDATE](NULL);
}

void RFrame_Update()
{
	if (g_pFunctions[RF_UPDATE])
		g_pFunctions[RF_UPDATE](NULL);
}

void RFrame_RenderD3D9()
{
	RRESULT isOK = RIsReadyToRender();
	if (isOK == R_NOTREADY)
	{
		return;
	}
	else if (isOK == R_RESTORED)
	{
		RMODEPARAMS ModeParams = { RGetScreenWidth(),RGetScreenHeight(),RGetFullscreenMode(),RGetPixelFormat() };
		RResetDevice(&ModeParams);
	}

	if (GetGlobalTimeMS() > g_last_mouse_move_time + RTOOLTIP_GAP)
		g_tool_tip = true;

	if (g_pFunctions[RF_RENDER])
		g_pFunctions[RF_RENDER](nullptr);

	RGetDevice()->SetStreamSource(0, nullptr, 0, 0);
	RGetDevice()->SetIndices(0);
	RGetDevice()->SetTexture(0, nullptr);
	RGetDevice()->SetTexture(1, nullptr);

	__BP(5007, "RFlip");
	RFlip();
	__EP(5007);
}

void RFrame_RenderVulkan()
{
	//GetRS2().DrawStatic<RS2Vulkan>();
	static RealSpace2::RBspObject bsp;
	static bool b;
	if (!b)
	{
		bsp.Open("Maps/Mansion/Mansion.rs");
		b = true;
	}

	bsp.Draw();
}

void RFrame_Render()
{
	if (!RIsActive() && RIsFullscreen()) return;

	if (GetRS2().UsingD3D9())
		return RFrame_RenderD3D9();

	return RFrame_RenderVulkan();
}

LRESULT FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Handle messages
	switch (message)
	{
	case WM_SYSCOMMAND:
	{
		switch (wParam)
		{
			// Trap ALT so it doesn't pause the app
		case SC_PREVWINDOW:
		case SC_NEXTWINDOW:
		case SC_KEYMENU:
		{
			return 0;
		}
		break;
		}
	}
	break;

	case WM_ACTIVATEAPP:
	{
		if (wParam == TRUE) {
			if (g_pFunctions[RF_ACTIVATE])
				g_pFunctions[RF_ACTIVATE](NULL);
			g_bActive = true;
		}
		else {
			if (g_pFunctions[RF_DEACTIVATE])
				g_pFunctions[RF_DEACTIVATE](NULL);

			if (RIsFullscreen()) {
				ShowWindow(hWnd, SW_MINIMIZE);
				UpdateWindow(hWnd);
			}
			g_bActive = false;
		}
	}
	break;

	case WM_MOUSEMOVE:
	{
		g_last_mouse_move_time = GetGlobalTimeMS();
		g_tool_tip = false;
	}
	break;

	case WM_CLOSE:
	{
		mlog("Received WM_CLOSE, exiting\n");
		RFrame_Destroy();
		PostQuitMessage(0);
	}
	break;
	}
	return g_WinProc(hWnd, message, wParam, lParam);
}

static bool RegisterWindowClass(HINSTANCE this_inst, WORD IconResID, WNDPROC WindowProc)
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(DWORD);
	wc.hInstance = this_inst;
	wc.hIcon = LoadIcon(this_inst, MAKEINTRESOURCE(IconResID));
	wc.hCursor = 0;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "RealSpace2";
	return RegisterClass(&wc) != 0;
}

static bool InitGraphicsAPI(const RMODEPARAMS* ModeParams, HINSTANCE inst, HWND hWnd, GraphicsAPI API)
{
	RFrame_Create();

	ShowWindow(g_hWnd, SW_SHOW);
	if (!RInitDisplay(g_hWnd, inst, ModeParams, API))
	{
		mlog("Fatal error: Failed to initialize display\n");
		return false;
	}

	if (g_pFunctions[RF_CREATE])
	{
		if (g_pFunctions[RF_CREATE](NULL) != R_OK)
		{
			RFrame_Destroy();
			return false;
		}
	}

	return true;
}

static int RenderLoop()
{
	MSG msg;

	do
	{
		auto GotMsg = PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE);

		if (GotMsg)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			__BP(5006, "RMain::Run");

			RFrame_Update();
			RFrame_Render();

			__EP(5006);

			MCheckProfileCount();
		}

		if (!g_bActive)
			Sleep(10);
	} while (WM_QUIT != msg.message);

	return static_cast<int>(msg.wParam);
}

int RMain(const char *AppName, HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline,
	int cmdshow, RMODEPARAMS *ModeParams, WNDPROC winproc, WORD IconResID,
	GraphicsAPI API)
{
	g_WinProc = winproc ? winproc : DefWindowProc;

	// Make a window
	if (!RegisterWindowClass(this_inst, IconResID, WndProc))
		return -1;

	auto Style = GetWindowStyle(*ModeParams);
	g_hWnd = CreateWindow("RealSpace2", AppName, Style, CW_USEDEFAULT, CW_USEDEFAULT,
		ModeParams->nWidth, ModeParams->nHeight, NULL, NULL, this_inst, NULL);

	RAdjustWindow(ModeParams);

	while (ShowCursor(FALSE) > 0)
		Sleep(10);

	if (!InitGraphicsAPI(ModeParams, this_inst, g_hWnd, API))
		return -1;

	return RenderLoop();
}

_NAMESPACE_REALSPACE2_END