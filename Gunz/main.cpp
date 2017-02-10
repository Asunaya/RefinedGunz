#include "stdafx.h"

#include <windows.h>
#include <wingdi.h>
#include <mmsystem.h>
#include <shlwapi.h>
#include <shellapi.h>

#pragma warning(push)
#pragma warning(disable:4091)
#include <ShlObj.h>
#pragma warning(pop)

#include "ZPrerequisites.h"
#include "ZConfiguration.h"
#include "ZGameClient.h"

#include "dxerr.h"

#include "main.h"
#include "resource.h"
#include "VersionNo.h"

#include "Mint4R2.h"
#include "ZApplication.h"
#include "MDebug.h"
#include "ZMessages.h"
#include "RealSpace2.h"
#include "Mint.h"
#include "ZGameInterface.h"
#include "RFrameWork.h"
#include "ZButton.h"
#include "ZDirectInput.h"
#include "ZActionDef.h"
#include "MRegistry.h"
#include "ZInitialLoading.h"
#include "MDebug.h"
#include "MCrashDump.h"
#include "ZEffectFlashBang.h"
#include "ZMsgBox.h"
#include "ZSecurity.h"
#include "ZStencilLight.h"
#include "ZReplay.h"
#include "ZUtil.h"
#include "ZOptionInterface.h"
#include "MMatchNotify.h"
#include "RS2.h"
#include "RBspObject.h"

#ifdef USING_VERTEX_SHADER
#include "RShaderMgr.h"
#endif

#include "RLenzFlare.h"
#include "MSysInfo.h"

#include "MTraceMemory.h"
#include "ZInput.h"
#include "Mint4Gunz.h"

#include "RGMain.h"
#include "RGGlobal.h"

RMODEPARAMS	g_ModeParams = { 800,600,FullscreenType::Fullscreen,D3DFMT_R5G6B5 };
static HANDLE Mutex;
static DWORD g_dwMainThreadID;

#ifndef _DEBUG
#define SUPPORT_EXCEPTIONHANDLING
#endif

RRESULT RenderScene(void *pParam);

#define RD_STRING_LENGTH 512
char cstrReleaseDate[512];

ZApplication	g_App;
MDrawContextR2* g_pDC = NULL;
MFontR2*		g_pDefFont = NULL;
ZDirectInput	g_DInput;
ZInput*			g_pInput = NULL;
Mint4Gunz		g_Mint;

HRESULT GetDirectXVersionViaDxDiag( DWORD* pdwDirectXVersionMajor,
	DWORD* pdwDirectXVersionMinor, TCHAR* pcDirectXVersionLetter );

void _ZChangeGameState(int nIndex)
{
	GunzState state = GunzState(nIndex);

	if (ZApplication::GetGameInterface())
	{
		ZApplication::GetGameInterface()->SetState(state);
	}
}

RRESULT OnCreate(void *pParam)
{
	if (GetRS2().UsingVulkan())
	{
		g_DInput.Create(g_hWnd, FALSE, FALSE);
		g_pInput = new ZInput(&g_DInput);
		RCameraUp = { 0, 0, -1 };
		return R_OK;
	}

	g_App.PreCheckArguments();

	g_RGMain->OnCreateDevice();

	RCreateLenzFlare("System/LenzFlare.xml");
	RGetLenzFlare()->Initialize();

	mlog("main : RGetLenzFlare()->Initialize() \n");

	RBspObject::CreateShadeMap("sfx/water_splash.bmp");
	
	sprintf_safe( cstrReleaseDate, "");
	g_DInput.Create(g_hWnd, FALSE, FALSE);
	g_pInput = new ZInput(&g_DInput);
	RSetGammaRamp(Z_VIDEO_GAMMA_VALUE);
	RSetRenderFlags(RRENDER_CLEAR_BACKBUFFER);

	ZGetInitialLoading()->Initialize(  1, 0, 0, RGetScreenWidth(), RGetScreenHeight(), 0, 0, 1024, 768 );

	mlog("main : ZGetInitialLoading()->Initialize() \n");

	struct _finddata_t c_file;
	intptr_t hFile;
	char szFileName[256];
#define FONT_DIR	"Font/"
#define FONT_EXT	"ttf"
	if( (hFile = _findfirst(FONT_DIR "*." FONT_EXT, &c_file )) != -1L ){
		do{
			strcpy_safe(szFileName, FONT_DIR);
			strcat_safe(szFileName, c_file.name);
			AddFontResource(szFileName);
		}while( _findnext( hFile, &c_file ) == 0 );
		_findclose(hFile);
	}

	g_pDefFont = new MFontR2;

	if( !g_pDefFont->Create("Default", Z_LOCALE_DEFAULT_FONT, 9, 1.0f) )
	{
		mlog("Fail to Create defualt font : MFontR2 / main.cpp.. onCreate\n" );
		g_pDefFont->Destroy();
		SAFE_DELETE( g_pDefFont );
		g_pDefFont	= NULL;
	}

	g_pDC = new MDrawContextR2(RGetDevice());

#ifndef _FASTDEBUG
	if( ZGetInitialLoading()->IsUseEnable() )
	{
		ZGetInitialLoading()->AddBitmap( 0, "Interface/Default/LOADING/loading_adult.jpg" );
		ZGetInitialLoading()->AddBitmapBar( "Interface/Default/LOADING/loading.bmp" );
		ZGetInitialLoading()->SetText( g_pDefFont, 10, 30, cstrReleaseDate );

		ZGetInitialLoading()->SetPercentage( 0.0f );
		ZGetInitialLoading()->Draw( MODE_FADEIN, 0 , true );
	}
#endif

	g_Mint.Initialize(800, 600, g_pDC, g_pDefFont);
	Mint::GetInstance()->SetHWND(RealSpace2::g_hWnd);

	mlog("main : g_Mint.Initialize() \n");

	ZLoadingProgress appLoading("application");
	if(!g_App.OnCreate(&appLoading))
	{
		ZGetInitialLoading()->Release();
		return R_ERROR_LOADING;
	}
	
	mlog("main : g_App.OnCreate() \n");

	ZGetSoundEngine()->SetEffectVolume(Z_AUDIO_EFFECT_VOLUME);
	ZGetSoundEngine()->SetMusicVolume(Z_AUDIO_BGM_VOLUME);
	ZGetSoundEngine()->SetEffectMute(Z_AUDIO_EFFECT_MUTE);
	ZGetSoundEngine()->SetMusicMute(Z_AUDIO_BGM_MUTE);

	g_Mint.SetWorkspaceSize(g_ModeParams.nWidth, g_ModeParams.nHeight);
	g_Mint.GetMainFrame()->SetSize(g_ModeParams.nWidth, g_ModeParams.nHeight);
	ZGetOptionInterface()->Resize(g_ModeParams.nWidth, g_ModeParams.nHeight);
    
	// Default Key
	for(int i=0; i<ZACTION_COUNT; i++){
		ZACTIONKEYDESCRIPTION& keyDesc = ZGetConfiguration()->GetKeyboard()->ActionKeys[i];
		g_pInput->RegisterActionKey(i, keyDesc.nVirtualKey);
		if(keyDesc.nVirtualKeyAlt!=-1)
			g_pInput->RegisterActionKey(i, keyDesc.nVirtualKeyAlt);
	}

	g_App.SetInitialState();

	ZGetFlashBangEffect()->SetDrawCopyScreen(true);

	ZGetInitialLoading()->SetLoadingStr("Done.");
	if( ZGetInitialLoading()->IsUseEnable() )
	{
#ifndef _FASTDEBUG
		ZGetInitialLoading()->SetPercentage( 100.f );
		ZGetInitialLoading()->Draw( MODE_FADEOUT, 0 ,true  );
#endif
		ZGetInitialLoading()->Release();
	}

	mlog("main : OnCreate() done\n");

	SetFocus(g_hWnd);

	return R_OK;
}

RRESULT OnDestroy(void *pParam)
{
	mlog("main : OnDestroy()\n");

	g_App.OnDestroy();

	SAFE_DELETE(g_pDefFont);

	g_Mint.Finalize();

	mlog("main : g_Mint.Finalize()\n");

	SAFE_DELETE(g_pInput);
	g_DInput.Destroy();

	mlog("main : g_DInput.Destroy()\n");

	mlog("main : g_App.OnDestroy()\n");

	ZGetConfiguration()->Destroy();

	mlog("main : ZGetConfiguration()->Destroy()\n");

	delete g_pDC;

	struct _finddata_t c_file;
	intptr_t hFile;
	char szFileName[256];
#define FONT_DIR	"Font/"
#define FONT_EXT	"ttf"
	if( (hFile = _findfirst(FONT_DIR" *." FONT_EXT, &c_file )) != -1L ){
		do{
			strcpy_safe(szFileName, FONT_DIR);
			strcat_safe(szFileName, c_file.name);
			RemoveFontResource(szFileName);
		}while( _findnext( hFile, &c_file ) == 0 );
		_findclose(hFile);
	}

	if (GetRS2().UsingD3D9())
	{
		MFontManager::Destroy();
		MBitmapManager::Destroy();
		MBitmapManager::DestroyAniBitmap();

		mlog("main : MBitmapManager::DestroyAniBitmap()\n");

		ZBasicInfoItem::Release();

		ZGetStencilLight()->Destroy();
		LightSource::Release();

		RBspObject::DestroyShadeMap();
		RDestroyLenzFlare();
		RAnimationFileMgr::GetInstance()->Destroy();
		ZStringResManager::FreeInstance();
	}

	g_RGMain.reset();

	mlog("main : OnDestroy() done\n");

	return R_OK;
}

template <typename T, int (ZConfiguration::*Getter)() const>
struct FPSLimiter
{
	T Action;
	int CurFPS{};
	int LastFPS{};
	u64 LastSecondTime{};

	FPSLimiter(T&& Action) : Action{ std::move(Action) } {}

	auto Tick()
	{
		auto TPS = QPF();
		auto CurTime = QPC();

		if (CurTime - LastSecondTime > TPS) {
			LastSecondTime = QPC();
			LastFPS = CurFPS;
			CurFPS = 0;
		}

		auto FPSLimit = (ZGetConfiguration()->*Getter)();
		if (FPSLimit <= 0)
		{
			++CurFPS;
			return true;
		}

		double fActual = double(CurFPS) / (FPSLimit - 1);
		double fGoal = double((CurTime - LastSecondTime) % TPS) / TPS;
		int nSleep = int((fActual - fGoal) * 1000);
		auto ret = Action(nSleep);
		if (ret)
			++CurFPS;
		return ret;
	}
};

template <int (ZConfiguration::*Getter)() const, typename T>
constexpr auto MakeFPSLimiter(T&& Action)
{
	return FPSLimiter<T, Getter>(std::move(Action));
}

auto VisualFPSLimiter = MakeFPSLimiter<&ZConfiguration::GetVisualFPSLimit>(
	[&](auto nSleep) {
	return nSleep <= 0;
});
auto LogicalFPSLimiter = MakeFPSLimiter<&ZConfiguration::GetLogicalFPSLimit>(
	[&](auto nSleep) {
	if (nSleep <= 0)
		return true;

	if (nSleep > 250)
		MLog("Large sleep %d!\n", nSleep);
	else
		Sleep(nSleep);

	return true;
});

RRESULT OnUpdate(void* pParam)
{
	auto prof = MBeginProfile("main::OnUpdate");

	g_pInput->Update();
	g_App.OnUpdate();

	LogicalFPSLimiter.Tick();

	return R_OK;
}

#include "LogMatrix.h"

RRESULT OnRender(void *pParam)
{
	auto mainOnRender = MBeginProfile("main::OnRender");
	if (!RIsActive() && RIsFullscreen())
		return R_NOTREADY;

	if (ZGetConfiguration()->GetVisualFPSLimit() != 0 && !VisualFPSLimiter.Tick())
		return R_NOFLIP;

	g_App.OnDraw();

	if(g_pDefFont) {
		char buf[512];
		size_t y_offset{};
		auto PrintText = [&](const char* Format, ...)
		{
			va_list va;
			va_start(va, Format);
			vsprintf_safe(buf, Format, va);
			va_end(va);
			g_pDefFont->m_Font.DrawText(MGetWorkspaceWidth() - 200, y_offset, buf);
			y_offset += 20;
		};

		if (ZGetConfiguration()->GetVisualFPSLimit() != 0)
		{
			PrintText("Visual FPS: %d", VisualFPSLimiter.LastFPS);
			PrintText("Logical FPS: %d", LogicalFPSLimiter.LastFPS);
		}
		else
		{
			PrintText("FPS: %d", LogicalFPSLimiter.LastFPS);
		}

#ifdef _DEBUG
		if (ZGetGame() && ZGetGame()->m_pMyCharacter)
		{
			v3 pos = ZGetGame()->m_pMyCharacter->GetPosition();
			PrintText("Pos: %d, %d, %d", int(pos.x), int(pos.y), int(pos.z));
			auto&& dir = ZGetGame()->m_pMyCharacter->GetDirection();
			PrintText("Dir: %f, %f, %f", dir.x, dir.y, dir.z);

			auto* vmesh = ZGetGame()->m_pMyCharacter->m_pVMesh;
			if (ZGetGame()->m_pMyCharacter->m_pVMesh)
			{
				pos = vmesh->GetHeadPosition();
				PrintText("Head pos: %d, %d, %d", int(pos.x), int(pos.y), int(pos.z));

				auto lower = ZGetGame()->m_pMyCharacter->GetStateLower();
				auto upper = ZGetGame()->m_pMyCharacter->GetStateUpper();

				PrintText("Lower ani: %s\n", g_AnimationInfoTableLower[lower].Name);
				PrintText("Upper ani: %s\n", g_AnimationInfoTableUpper[upper].Name);
				PrintText("Draw calls: %d\n", g_nCall);
				PrintText("Polygons: %d\n", g_nPoly);
			}
		}
#endif
	}

	g_RGMain->OnRender();

	MEndProfile(mainOnRender);

	return R_OK;
}

RRESULT OnInvalidate(void *pParam)
{
	MBitmapR2::m_dwStateBlock=NULL;

	g_App.OnInvalidate();
	
	return R_OK;
}

RRESULT OnRestore(void *pParam)
{
	for(int i=0; i<MBitmapManager::GetCount(); i++){
		MBitmapR2* pBitmap = (MBitmapR2*)MBitmapManager::Get(i);
		pBitmap->OnLostDevice();
	}

	g_App.OnRestore();

	return R_OK;
}

// Toggle the process priority boost when the window is activated or deactivated
RRESULT OnActivate(void *pParam)
{
	if (ZGetGameInterface() && ZGetGameClient() && Z_ETC_BOOST)
		ZGetGameClient()->PriorityBoost(true);
	return R_OK;
}

RRESULT OnDeActivate(void *pParam)
{
	if (ZGetGameInterface() && ZGetGameClient())
		ZGetGameClient()->PriorityBoost(false);
	return R_OK;
}

RRESULT OnError(void *pParam)
{
	mlog("RealSpace::OnError(%d) \n", RGetLastError());

	switch (RGetLastError())
	{
	case RERROR_INVALID_DEVICE:
	{
		D3DADAPTER_IDENTIFIER9 *ai = RGetAdapterID();
		char szLog[512];
		ZTransMsg(szLog, MSG_DONOTSUPPORT_GPCARD, 1, ai->Description);

		int ret = MessageBox(NULL, szLog, ZMsg(MSG_WARNING), MB_YESNO);
		if (ret != IDYES)
			return R_UNKNOWN;
	}
	break;
	case RERROR_CANNOT_CREATE_D3D:
	{
		ShowCursor(TRUE);

		auto* szLog = ZMsg(MSG_DIRECTX_NOT_INSTALL);

		// Tell the user that DirectX 9 is not installed, and offer to direct them to the download page.
		int ret = MessageBox(NULL, szLog, ZMsg(MSG_WARNING), MB_YESNO);
		// Open the DirectX 9 download page if the user pressed Yes.
		if (ret == IDYES)
			ShellExecute(g_hWnd, "open", ZMsg(MSG_DIRECTX_DOWNLOAD_URL), NULL, NULL, SW_SHOWNORMAL);
	}
	break;
	}

	return R_OK;
}

// Sets mode parameters to values retrieved from config.xml
static void SetModeParams()
{
	g_ModeParams.FullscreenMode = ZGetConfiguration()->GetVideo()->FullscreenMode;
	g_ModeParams.nWidth = ZGetConfiguration()->GetVideo()->nWidth;
	g_ModeParams.nHeight = ZGetConfiguration()->GetVideo()->nHeight;
}

LONG_PTR FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			if (strlen(Z_LOCALE_HOMEPAGE_TITLE) > 0)
			{
				ShowIExplorer(false, Z_LOCALE_HOMEPAGE_TITLE);
			}
			break;
		case WM_DESTROY:
			if (strlen(Z_LOCALE_HOMEPAGE_TITLE) > 0)
			{
				ShowIExplorer(true, Z_LOCALE_HOMEPAGE_TITLE);
			}
			break;
		case WM_SETCURSOR:
			if(ZApplication::GetGameInterface())
				ZApplication::GetGameInterface()->OnResetCursor();
			return TRUE; // prevent Windows from setting cursor to window class cursor

		case WM_KEYDOWN:
			{
				bool b = false;
			}
	}

	if(Mint::GetInstance()->ProcessEvent(hWnd, message, wParam, lParam)==true)
		return 0;

	if (message == WM_CHANGE_GAMESTATE)
	{
		_ZChangeGameState(wParam);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void CheckFileAssociation()
{
#define GUNZ_REPLAY_CLASS_NAME	"GunzReplay"

	char szValue[256];
	if (!MRegistry::Read(HKEY_CLASSES_ROOT, "." GUNZ_REC_FILE_EXT, NULL, szValue))
	{
		MRegistry::Write(HKEY_CLASSES_ROOT, "." GUNZ_REC_FILE_EXT, NULL, GUNZ_REPLAY_CLASS_NAME);

		char szModuleFileName[_MAX_PATH] = { 0, };
		GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);

		char szCommand[_MAX_PATH];
		sprintf_safe(szCommand, "\"%s\" \"%%1\"", szModuleFileName);

		MRegistry::Write(HKEY_CLASSES_ROOT, GUNZ_REPLAY_CLASS_NAME "\\shell\\open\\command", NULL, szCommand);

		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, NULL, NULL);
	}
}

void UpgradeMrsFile()
{
	char temp_path[ 1024];
	sprintf_safe( temp_path,"*");

	FFileList file_list;
	GetFindFileListWin(temp_path,".mrs",file_list);
	file_list.UpgradeMrs();
}

static LONG __stdcall ExceptionFilter(_EXCEPTION_POINTERS* p)
{
	return CrashExceptionDump(p, "Gunz.dmp");
}

static void SetRS2Callbacks()
{
	RSetFunction(RF_CREATE, OnCreate);
	RSetFunction(RF_RENDER, OnRender);
	RSetFunction(RF_UPDATE, OnUpdate);
	RSetFunction(RF_DESTROY, OnDestroy);
	RSetFunction(RF_INVALIDATE, OnInvalidate);
	RSetFunction(RF_RESTORE, OnRestore);
	RSetFunction(RF_ACTIVATE, OnActivate);
	RSetFunction(RF_DEACTIVATE, OnDeActivate);
	RSetFunction(RF_ERROR, OnError);
}

int PASCAL WinMain(HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow)
{
	SetUnhandledExceptionFilter(ExceptionFilter);

	_set_invalid_parameter_handler([](const wchar_t* expression,
		const wchar_t* function,
		const wchar_t* file,
		unsigned int line,
		uintptr_t pReserved)
	{
		MLog("Invalid parameter detected in function %s.\n"
			"File: %s, line: %d.\nExpression: %s.\n",
			function, file, line, expression);
		assert(false);
	});

	g_dwMainThreadID = GetCurrentThreadId();
	
#ifdef _MTRACEMEMORY
	MInitTraceMemory();
#endif

	char szModuleFileName[_MAX_DIR]; szModuleFileName[0] = 0;
	GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);
	PathRemoveFileSpec(szModuleFileName);
	SetCurrentDirectory(szModuleFileName);

#ifdef _PUBLISH
	// Create a mutex so we can't run multiple clients
	Mutex = CreateMutex(NULL, TRUE, "RGunz");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(0, "Refined Gunz is already running", "RGunz", 0);
		exit(-1);
		return 0;
	}
#endif

	// Initialize MLog. Can't call it before this point.
	InitLog(MLOGSTYLE_DEBUGSTRING | MLOGSTYLE_FILE);

	// Seed the random number generator rand() from the C standard library
	srand((unsigned int)time(nullptr));

	mlog("Refined Gunz version %d launched. Build date: " __DATE__ " " __TIME__ "\n", RGUNZ_VERSION);

	char szDateRun[128]; szDateRun[0] = 0;
	char szTimeRun[128]; szTimeRun[0] = 0;
	_strdate_s(szDateRun);
	_strtime_s(szTimeRun);
	mlog("Log time (%s %s)\n", szDateRun, szTimeRun);

#ifndef _PUBLISH
	mlog("cmdline = %s\n", cmdline);
#endif

	MSysInfoLog();

	CheckFileAssociation();

	// Initialize MZFileSystem - MUpdate 
	MRegistry::szApplicationName = APPLICATION_NAME;

	g_App.InitFileSystem();

#if defined(_PUBLISH) && defined(ONLY_LOAD_MRS_FILES)
	MZFile::SetReadMode( MZIPREADFLAG_MRS2 );
#endif

	ZGetConfiguration()->Load();

	g_RGMain = std::make_unique<RGMain>();

	ZStringResManager::MakeInstance();
	if( !ZApplication::GetInstance()->InitLocale() )
	{
		MLog("Failed to initialize locale, exiting\n");
		return false;
	}

	if (!ZGetConfiguration()->LateStringConvert())
	{
		MLog("main.cpp - Late string convert fail.\n");
		return false;
	}

	if (!ZApplication::GetInstance()->ParseArguments(cmdline))
		return 0;

	GraphicsAPI GfxAPI = GraphicsAPI::D3D9;

	// TODO: Parse command-line arguments properly
	if (strstr(cmdline, "/vulkan"))
		GfxAPI = GraphicsAPI::Vulkan;

	if(!InitializeNotify(ZApplication::GetFileSystem())) {
		MLog("Failed to load notify.xml\n");
		return 0;
	}

	SetRS2Callbacks();
	SetModeParams();

	int nReturn = RMain(APPLICATION_NAME, this_inst, prev_inst, cmdline, cmdshow,
		&g_ModeParams, WndProc, IDI_ICON1, GfxAPI);

#ifdef _MTRACEMEMORY
	MShutdownTraceMemory();
#endif

	return nReturn;
}