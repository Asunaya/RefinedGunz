#include "stdafx.h"

#ifdef _HSHIELD
#include "HShield/HShield.h"
#endif

#ifdef _XTRAP
#include "XTrap/XTrap.h"
#pragma comment ( lib, "XTrap/Multy C Type/XTrapIC.lib")
#endif

#include "ZPrerequisites.h"
#include "ZConfiguration.h"
#include "ZGameClient.h"
#include <windows.h>
#include <wingdi.h>
#include <mmsystem.h>
#include <shlwapi.h>
#include <shellapi.h>

#include "dxerr9.h"

#include "main.h"
#include "resource.h"
#include "VersionNo.h"

#include "Mint4R2.h"
#include "ZApplication.h"
#include "MDebug.h"
#include "ZMessages.h"
#include "MMatchNotify.h"
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

#ifdef USING_VERTEX_SHADER
#include "RShaderMgr.h"
#endif

//#include "mempool.h"
#include "RLenzFlare.h"
#include "ZLocale.h"
#include "MSysInfo.h"

#include "MTraceMemory.h"
#include "ZInput.h"
#include "Mint4Gunz.h"

#ifdef _DEBUG
RMODEPARAMS	g_ModeParams={640,480,false,D3DFMT_R5G6B5};
//RMODEPARAMS	g_ModeParams={1024,768,false,RPIXELFORMAT_565};
#else
RMODEPARAMS	g_ModeParams={800,600,true,D3DFMT_R5G6B5};
#endif

#ifndef _DEBUG
#define SUPPORT_EXCEPTIONHANDLING
#endif


RRESULT RenderScene(void *pParam);

#define RD_STRING_LENGTH 512
char cstrReleaseDate[512];// = "ReleaseDate : 12/22/2003";

ZApplication	g_App;
MDrawContextR2* g_pDC = NULL;
MFontR2*		g_pDefFont = NULL;
ZDirectInput	g_DInput;
ZInput*			g_pInput = NULL;
Mint4Gunz		g_Mint;


HRESULT GetDirectXVersionViaDxDiag( DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor, TCHAR* pcDirectXVersionLetter );



void _ZChangeGameState(int nIndex)
{
	GunzState state = GunzState(nIndex);

	if (ZApplication::GetGameInterface())
	{
		ZApplication::GetGameInterface()->SetState(state);
	}
}

//list<HANDLE>	g_FontMemHandles;

RRESULT OnCreate(void *pParam)
{
	g_App.PreCheckArguments();

	RCreateLenzFlare("System/LenzFlare.xml");
	RGetLenzFlare()->Initialize();

	mlog("main : RGetLenzFlare()->Initialize() \n");

	RBspObject::CreateShadeMap("sfx/water_splash.bmp");
	//D3DCAPS9 caps;
	//RGetDevice()->GetDeviceCaps( &caps );
	//if( caps.VertexShaderVersion < D3DVS_VERSION(1, 1) )
	//{
	//	RGetShaderMgr()->mbUsingShader				= false;
	//	RGetShaderMgr()->shader_enabled				= false;
	//	mlog("main : VideoCard Dosen't support Vertex Shader...\n");
	//}
	//else
	//{
	//	mlog("main : VideoCard support Vertex Shader...\n");
	//}

//	sprintf( cstrReleaseDate, "Release Date : %s", __DATE__ );
	sprintf( cstrReleaseDate, "");				// 삭제.
	g_DInput.Create(g_hWnd, FALSE, FALSE);
	g_pInput = new ZInput(&g_DInput);
	/*
	for(int i=0; i<ZApplication::GetFileSystem()->GetFileCount(); i++){
		const char* szFileName = ZApplication::GetFileSystem()->GetFileName(i);
		size_t nStrLen = strlen(szFileName);
		if(nStrLen>3){
			if(stricmp(szFileName+nStrLen-3, "ttf")==0){
				int nFileLenth = ZApplication::GetFileSystem()->GetFileLength(i);
				char* pFileData = new char[nFileLenth];
				ZApplication::GetFileSystem()->ReadFile(szFileName, pFileData, nFileLenth);
				int nInstalled = 0;
				HANDLE hFontMem = AddFontMemResourceEx(pFileData, 1, 0, &nInstalled);
				g_FontMemHandles.insert(g_FontMemHandles.end(), hFontMem);
				delete[] pFileData;
			}
		}
	}
	*/
	RSetGammaRamp(Z_VIDEO_GAMMA_VALUE);
	RSetRenderFlags(RRENDER_CLEAR_BACKBUFFER);

	ZGetInitialLoading()->Initialize(  1, 0, 0, RGetScreenWidth(), RGetScreenHeight(), 0, 0, 1024, 768 );

	mlog("main : ZGetInitialLoading()->Initialize() \n");

	struct _finddata_t c_file;
	intptr_t hFile;
	char szFileName[256];
#define FONT_DIR	"Font/"
#define FONT_EXT	"ttf"
	if( (hFile = _findfirst(FONT_DIR"*."FONT_EXT, &c_file )) != -1L ){
		do{
			strcpy(szFileName, FONT_DIR);
			strcat(szFileName, c_file.name);
			AddFontResource(szFileName);
		}while( _findnext( hFile, &c_file ) == 0 );
		_findclose(hFile);
	}

	g_pDefFont = new MFontR2;

	if( !g_pDefFont->Create("Default", Z_LOCALE_DEFAULT_FONT, 9, 1.0f) )
//	if( !g_pDefFont->Create("Default", RGetDevice(), "FONTb11b", 9, 1.0f, true, false) )
//	if( !g_pDefFont->Create("Default", RGetDevice(), "FONTb11b", 14, 1.0f, true, false) )
	{
		mlog("Fail to Create defualt font : MFontR2 / main.cpp.. onCreate\n" );
		g_pDefFont->Destroy();
		SAFE_DELETE( g_pDefFont );
		g_pDefFont	= NULL;
	}
	//pDefFont->Create("Default", RGetDevice(), "FONTb11b", 10, 1.0f, true, false);
	//pDefFont->Create("Default", RGetDevice(), "FONTb11b", 16, 1.0f, true, false, -1, 4);
	//pDefFont->Create("Default", RGetDevice(), "-2002", 10, 1.0f, false, false, -1, 1);
	//pDefFont->Create("Default", RGetDevice(), "HY수평선L", 12, 1.0f, false, false, -1, 2);

	//MLoadDesignerMode();
	// 기본 800x600 디자인으로 생성하고, 나중에 Resize를 화면 크기로 해준다.

	g_pDC = new MDrawContextR2(RGetDevice());

#ifndef _FASTDEBUG
	if( ZGetInitialLoading()->IsUseEnable() )
	{
		if( ZGetLocale()->IsTeenMode() )
		{
			ZGetInitialLoading()->AddBitmap( 0, "Interface/Default/LOADING/loading_teen.jpg" );
		}
		else
		{
			ZGetInitialLoading()->AddBitmap( 0, "Interface/Default/LOADING/loading_adult.jpg" );
		}
		ZGetInitialLoading()->AddBitmapBar( "Interface/Default/LOADING/loading.bmp" );
		ZGetInitialLoading()->SetText( g_pDefFont, 10, 30, cstrReleaseDate );

		ZGetInitialLoading()->SetPercentage( 0.0f );
		ZGetInitialLoading()->Draw( MODE_FADEIN, 0 , true );
	}
#endif

//	ZGetInitialLoading()->SetPercentage( 10.0f );
//	ZGetInitialLoading()->Draw( MODE_DEFAULT, 0 , true );

	g_Mint.Initialize(800, 600, g_pDC, g_pDefFont);
	Mint::GetInstance()->SetHWND(RealSpace2::g_hWnd);

	mlog("main : g_Mint.Initialize() \n");

//	ZGetConfiguration()->LoadHotKey(FILENAME_CONFIG);

	ZLoadingProgress appLoading("application");
	if(!g_App.OnCreate(&appLoading))
	{
		ZGetInitialLoading()->Release();
		return R_ERROR_LOADING;
	}

//	ZGetInitialLoading()->SetPercentage( 50.0f );
//	ZGetInitialLoading()->Draw( MODE_DEFAULT, 0, true );
	
	mlog("main : g_App.OnCreate() \n");

	ZGetSoundEngine()->SetEffectVolume(Z_AUDIO_EFFECT_VOLUME);
	ZGetSoundEngine()->SetMusicVolume(Z_AUDIO_BGM_VOLUME);
	ZGetSoundEngine()->SetEffectMute(Z_AUDIO_EFFECT_MUTE);
	ZGetSoundEngine()->SetMusicMute(Z_AUDIO_BGM_MUTE);

	g_Mint.SetWorkspaceSize(g_ModeParams.nWidth, g_ModeParams.nHeight);
	g_Mint.GetMainFrame()->SetSize(g_ModeParams.nWidth, g_ModeParams.nHeight);
	ZGetOptionInterface()->Resize(g_ModeParams.nWidth, g_ModeParams.nHeight);

//	ZGetInitialLoading()->SetPercentage( 80.f );
//	ZGetInitialLoading()->Draw( MODE_DEFAULT, 0, true );
    
	// Default Key
	for(int i=0; i<ZACTION_COUNT; i++){
//		g_Mint.RegisterActionKey(i, ZGetConfiguration()->GetKeyboard()->ActionKeys[i].nScanCode);
		ZACTIONKEYDESCRIPTION& keyDesc = ZGetConfiguration()->GetKeyboard()->ActionKeys[i];
		g_pInput->RegisterActionKey(i, keyDesc.nVirtualKey);
		if(keyDesc.nVirtualKeyAlt!=-1)
			g_pInput->RegisterActionKey(i, keyDesc.nVirtualKeyAlt);
	}

	g_App.SetInitialState();

//	ParseParameter(g_szCmdLine);

	ZGetFlashBangEffect()->SetDrawCopyScreen(true);

	static const char *szDone = "Done.";
	ZGetInitialLoading()->SetLoadingStr(szDone);
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

	RGetShaderMgr()->Release();

//	g_App.OnDestroy();

	mlog("main : g_App.OnDestroy()\n");

	ZGetConfiguration()->Destroy();

	mlog("main : ZGetConfiguration()->Destroy()\n");

	delete g_pDC;

	struct _finddata_t c_file;
	intptr_t hFile;
	char szFileName[256];
#define FONT_DIR	"Font/"
#define FONT_EXT	"ttf"
	if( (hFile = _findfirst(FONT_DIR"*."FONT_EXT, &c_file )) != -1L ){
		do{
			strcpy(szFileName, FONT_DIR);
			strcat(szFileName, c_file.name);
			RemoveFontResource(szFileName);
		}while( _findnext( hFile, &c_file ) == 0 );
		_findclose(hFile);
	}

	MFontManager::Destroy();
	MBitmapManager::Destroy();
	MBitmapManager::DestroyAniBitmap();

	mlog("main : MBitmapManager::DestroyAniBitmap()\n");

	/*
	for(list<HANDLE>::iterator i=g_FontMemHandles.begin(); i!=g_FontMemHandles.end(); i++){
		RemoveFontMemResourceEx(*i);
	}
	*/

	//ReleaseMemPool(RealSoundEffectPlay);
	//UninitMemPool(RealSoundEffectPlay);

	//ReleaseMemPool(RealSoundEffect);
	//UninitMemPool(RealSoundEffect);

	//ReleaseMemPool(RealSoundEffectFx);
	//UninitMemPool(RealSoundEffectFx);

	//mlog("main : UninitMemPool(RealSoundEffectFx)\n");

	// 메모리풀 헤제
	ZBasicInfoItem::Release(); // 할당되어 있는 메모리 해제
//	ZHPInfoItem::Release();

	ZGetStencilLight()->Destroy();
	LightSource::Release();

//	ZStencilLight::GetInstance()->Destroy();

	RBspObject::DestroyShadeMap();
	RDestroyLenzFlare();
	RAnimationFileMgr::GetInstance()->Destroy();
	ZStringResManager::FreeInstance();

	mlog("main : OnDestroy() done\n");

	return R_OK;
}

RRESULT OnUpdate(void* pParam)
{
	__BP(100, "main::OnUpdate");

	g_pInput->Update();

	g_App.OnUpdate();

#ifndef _DEBUG
#ifdef _XTRAP
	// XTrap 주기적인 체크
#define XTRAP_INTERVAL 5000
	static DWORD xTrapLastTime = timeGetTime();
	DWORD currentTime=timeGetTime();
	if(xTrapLastTime+XTRAP_INTERVAL<currentTime)
	{
		xTrapLastTime=currentTime;

			char szMsgBuf[500] = {
#ifdef LOCALE_KOREA
				"비정상적인 행위가 감지되었습니다. 게임을 종료합니다.\n"
#else
				"An abnormal behavior is detected. Terminating game.\n"
#endif
			};

			///////////////////////////////////////////////////////////////////
			// 디폴트 탐지 메시지 루틴

			if (g_bApiMal			== TRUE ||
				g_bMemoryMdl		== TRUE ||
				g_bAutoMousMdl		== TRUE ||
				g_bAutoKeybMdl		== TRUE ||
				g_bMalMdl			== TRUE ||
				g_bSpeedMdl			== TRUE ||
				g_bFileMdl			== TRUE ||
				g_bApiHookMdl		== TRUE ||
				g_bDebugModMdl		== TRUE ||
				g_bMemoryCrack		== TRUE ||
				g_bFileCrack		== TRUE ||
				g_bApiHookCrack		== TRUE)
			{
				mlog("xtrap error : ");
				if (g_bApiMal			== TRUE) mlog("ApiMal");
				if (g_bMemoryMdl		== TRUE) mlog("MemoryMdl");
				if (g_bAutoMousMdl		== TRUE) mlog("AutoMousMdl");
				if (g_bAutoKeybMdl		== TRUE) mlog("AutoKeybMdl");
				if (g_bMalMdl			== TRUE) mlog("MalMdl");
				if (g_bSpeedMdl			== TRUE) mlog("SpeedMdl");
				if (g_bFileMdl			== TRUE) mlog("FileMdl");
				if (g_bApiHookMdl		== TRUE) mlog("ApiHookMdl");
				if (g_bDebugModMdl		== TRUE) mlog("DebugModMdl");
				if (g_bMemoryCrack		== TRUE) mlog("MemoryCrack");
				if (g_bFileCrack		== TRUE) mlog("FileCrack");
				if (g_bApiHookCrack		== TRUE) mlog("ApiHookCrack");
				mlog("\n");


				if (ZGetGameClient()) ZGetGameClient()->Disconnect();
//				AfxMessageBox(szMsgBuf); 
				mlog(szMsgBuf);
	            PostQuitMessage(0);
				//
				// CloseSocket and Exit Game Client
				//
			}

			/* Version 0xA5001069 */
			if (g_bOsMdl			== TRUE)
			{
				if (ZGetGameClient()) ZGetGameClient()->Disconnect();
#ifdef LOCALE_KOREA
//				AfxMessageBox("파일 속성에서 호환성 모드가 설정되었습니다. 설정을 해제하십시오."); 
				mlog("파일 속성에서 호환성 모드가 설정되었습니다. 설정을 해제하십시오.\n"); 
#else
//				AfxMessageBox("The compatibility mode is activated in the file property. Please deactivated it."); 
				mlog("The compatibility mode is activated in the file property. Please deactivated it.\n"); 
#endif 
				PostQuitMessage(0);
				//
				// CloseSocket and Exit Game Clinet
				//
			}

			if (g_bPatchMdl			== TRUE)
			{
				if (ZGetGameClient()) ZGetGameClient()->Disconnect();
#ifdef LOCALE_KOREA
//				AfxMessageBox("패치 서버 접속에 문제가 있습니다. 네트워크 상황을 확인하십시오."); 
				mlog("패치 서버 접속에 문제가 있습니다. 네트워크 상황을 확인하십시오.\n"); 
#else
//				AfxMessageBox("There is a trouble connecting to the patch server. Please check the network condition."); 
				mlog("There is a trouble connecting to the patch server. Please check the network condition.\n"); 
#endif 
				PostQuitMessage(0);
				//
				// CloseSocket and Exit Game Clinet
				//
			}

			if (g_bStartXTrap		== FALSE)
			{
				if (ZGetGameClient()) ZGetGameClient()->Disconnect();
#ifdef LOCALE_KOREA
//				AfxMessageBox("게임 보안모듈을 실행할수 없습니다."); 
				mlog("게임 보안모듈을 실행할수 없습니다.\n"); 
#else
//				AfxMessageBox("Can not run the game security module."); 
				mlog("Can not run the game security module.\n"); 
#endif
				PostQuitMessage(0);

				//
				// CloseSocket and Exit Game Clinet
				//
			}
	}
#endif // of xtrap
#endif
	__EP(100);

	return R_OK;
}

RRESULT OnRender(void *pParam)
{
	__BP(101, "main::OnRender");
	if( !RIsActive() && RIsFullScreen() )
	{
		__EP(101);
		return R_NOTREADY;
	}

	g_App.OnDraw();


#ifdef _SMOOTHLOOP
	Sleep(10);
#endif

#ifndef _PUBLISH

	if(g_pDefFont) {
		static char __buffer[256];

		float fMs = 1000.f/g_fFPS;
		float fScore = 100-(fMs-(1000.f/60.f))*2;

		sprintf(__buffer, "FPS : %3.3f %.3f점 (%.3f ms)",g_fFPS,fScore,fMs);
		g_pDefFont->m_Font.DrawText( MGetWorkspaceWidth()-200,0,__buffer );
//		OutputDebugString(__buffer);
	}

#endif

	__EP(101);

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
			D3DADAPTER_IDENTIFIER9 *ai=RGetAdapterID();
			char szLog[512];
			ZTransMsg( szLog, MSG_DONOTSUPPORT_GPCARD, 1, ai->Description);

			int ret=MessageBox(NULL, szLog, ZMsg( MSG_WARNING), MB_YESNO);
			if(ret!=IDYES)
				return R_UNKNOWN;
		}
		break;
	case RERROR_CANNOT_CREATE_D3D:
		{
			ShowCursor(TRUE);

			char szLog[512];
			sprintf(szLog, ZMsg( MSG_DIRECTX_NOT_INSTALL));

			int ret=MessageBox(NULL, szLog, ZMsg( MSG_WARNING), MB_YESNO);
			if(ret==IDYES)
			{
				ShellExecute(g_hWnd, "open", ZMsg( MSG_DIRECTX_DOWNLOAD_URL), NULL, NULL, SW_SHOWNORMAL); 
			}
		}
		break;

	};

	return R_OK;
}

void SetModeParams()
{
#ifdef _PUBLISH
	g_ModeParams.bFullScreen = true;
#else
	#ifdef _DEBUG
		g_ModeParams.bFullScreen = false;
	#else
		g_ModeParams.bFullScreen = ZGetConfiguration()->GetVideo()->bFullScreen;
	#endif
#endif
	
	g_ModeParams.nWidth = ZGetConfiguration()->GetVideo()->nWidth;
	g_ModeParams.nHeight = ZGetConfiguration()->GetVideo()->nHeight;
	ZGetConfiguration()->GetVideo()->nColorBits == 32 ? 
	g_ModeParams.PixelFormat = D3DFMT_X8R8G8B8 : g_ModeParams.PixelFormat = D3DFMT_R5G6B5 ;

}

// 느려도 관계없다~~ -.-

int FindStringPos(char* str,char* word)
{
	if(!str || str[0]==0)	return -1;
	if(!word || word[0]==0)	return -1;

	int str_len = (int)strlen(str);
	int word_len = (int)strlen(word);

	char c;
	bool bCheck = false;

	for(int i=0;i<str_len;i++) {
		c = str[i];
		if(c == word[0]) {

			bCheck = true;

			for(int j=1;j<word_len;j++) {
				if(str[i+j]!=word[j]) {
					bCheck = false;
					break;
				}
			}

			if(bCheck) {
				return i;
			}
		}
	}
	return -1;
}

bool FindCrashFunc(char* pName)
{
//	Function Name
//	File Name 
	if(!pName) return false;

	FILE *fp;
	fp = fopen( "mlog.txt", "r" );
	if(fp==NULL)  return false;

	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	char* pBuffer = new char [size];

	fread(pBuffer,1,size,fp);

	fclose(fp);

	// 우리 쏘스에서 찾는다.
	int posSource = FindStringPos(pBuffer,"ublish");
	if(posSource==-1) return false;

	int posA = FindStringPos(pBuffer+posSource,"Function Name");
//	int posB = FindStringPos(pBuffer,"File Name");	
	// filename 이 없는 경우도 있어서 이렇게 바꿨다
	int posB = posA + FindStringPos(pBuffer+posSource+posA,"\n");

	if(posA==-1) return false;
	if(posB==-1) return false;

	posA += 16;

//	int memsize = posB-posA-6;
	int memsize = posB-posA;
	memcpy(pName,&pBuffer[posA+posSource],memsize);

	pName[memsize] = 0;

	delete [] pBuffer;

	for(int i=0;i<memsize;i++) {
		if(pName[i]==':') {
			pName[i] = '-';
		}
	}

	return true;
}

void HandleExceptionLog()
{
	#define ERROR_REPORT_FOLDER	"ReportError"

	extern char* logfilename;	// Instance on MDebug.cpp

	// ERROR_REPORT_FOLDER 존재하는지 검사하고, 없으면 생성
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFile(ERROR_REPORT_FOLDER, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		if (!CreateDirectory("ReportError", NULL)) {
			MessageBox(g_hWnd, "ReportError 폴더를 생성할 수 없습니다.", APPLICATION_NAME , MB_ICONERROR|MB_OK);
			return;
		}
	} else 	{
		FindClose(hFind);
	}


	// mlog.txt 를 ERROR_REPORT_FOLDER 로 복사

	//acesaga_0928_911_moanus_rslog.txt
	//USAGE_EX) BAReport app=acesaga;addr=moon.maiet.net;port=21;id=ftp;passwd=ftp@;gid=10;user=moanus;localfile=rslog.txt;remotefile=remote_rslog.txt;
/*
	if(ZGetCharacterManager()) {
		ZGetCharacterManager()->OutputDebugString_CharacterState();
	}
*/	
	ZGameClient* pClient = (ZGameClient*)ZGameClient::GetInstance();

	char* pszCharName = NULL;
	MUID uidChar;
	MMatchObjCache* pObj;
	char szPlayer[128];

	if( pClient ) {

		uidChar = pClient->GetPlayerUID();
		pObj = pClient->FindObjCache(uidChar);
		if (pObj)
			pszCharName = pObj->GetName();

		wsprintf(szPlayer, "%s(%d%d)", pszCharName?pszCharName:"Unknown", uidChar.High, uidChar.Low);
	}
	else { 
		wsprintf(szPlayer, "Unknown(-1.-1)");
	}


//	if (pClient) {

		time_t currtime;
		time(&currtime);
		struct tm* pTM = localtime(&currtime);

		char cFuncName[1024];

		if(FindCrashFunc(cFuncName)==false) {
			strcpy(cFuncName,"Unknown Error");
		}

		char szFileName[_MAX_DIR], szDumpFileName[_MAX_DIR];
		wsprintf(szFileName, "%s_%s_%.2d%.2d_%.2d%.2d_%s_%s", cFuncName,
				APPLICATION_NAME, pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, szPlayer, "mlog.txt");
		wsprintf(szDumpFileName, "%s.dmp", szFileName);

		char szFullFileName[_MAX_DIR], szDumpFullFileName[_MAX_DIR];
		wsprintf(szFullFileName, "%s/%s", ERROR_REPORT_FOLDER, szFileName);
		wsprintf(szDumpFullFileName, "%s/%s", ERROR_REPORT_FOLDER, szDumpFileName);

		if (CopyFile("mlog.txt", szFullFileName, TRUE))
		{
			CopyFile("Gunz.dmp", szDumpFullFileName, TRUE);

			// BAReport 실행
			char szCmd[4048];
			char szRemoteFileName[_MAX_DIR], szRemoteDumpFileName[_MAX_DIR];
			wsprintf(szRemoteFileName, "%s/%s/%s", ZGetConfiguration()->GetBAReportDir(), "gunzlog", szFileName);
			wsprintf(szRemoteDumpFileName, "%s/%s/%s", ZGetConfiguration()->GetBAReportDir(), "gunzlog", szDumpFileName);

			wsprintf(szCmd, "BAReport app=%s;addr=%s;port=21;id=ftp;passwd=ftp@;user=%s;localfile=%s,%s;remotefile=%s,%s", 
				APPLICATION_NAME, ZGetConfiguration()->GetBAReportAddr(), szPlayer, szFullFileName, szDumpFullFileName, szRemoteFileName, szRemoteDumpFileName);

			WinExec(szCmd, SW_SHOW);

			FILE *file = fopen("bareportpara.txt","w+");
			fprintf(file,szCmd);
			fclose(file);
		}	
//	}
}

long FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_SYSCHAR:
			if(ZIsLaunchDevelop() && wParam==VK_RETURN)
			{
#ifndef _PUBLISH
				RFrame_ToggleFullScreen();
#endif
				return 0;
			}
			break;

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

			/*
		case  WM_LBUTTONDOWN:
			SetCapture(hWnd);
			return TRUE;
		case WM_LBUTTONUP:
			ReleaseCapture();
			return TRUE;
			*/
		case WM_KEYDOWN:
			{
				bool b = false;
			}
	}

	if(Mint::GetInstance()->ProcessEvent(hWnd, message, wParam, lParam)==true)
		return 0;

	// thread safe하기위해 넣음
	if (message == WM_CHANGE_GAMESTATE)
	{
		_ZChangeGameState(wParam);
	}


	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
class mtrl {
public:

};

class node {
public:
	int		m_nIndex[5];
};


class _map{
public:
	mtrl* GetMtrl(node* node,int index) { return GetMtrl(node->m_nIndex[index]); }
	mtrl* GetMtrl(int id) { return m_pIndex[id]; }

	mtrl*	m_pIndex[5];
};

class game {
public:
	_map m_map;	
};

game _game;
game* g_game;
*/

void ClearTrashFiles()
{
}

bool CheckFileList()
{
	MZFileSystem *pfs=ZApplication::GetFileSystem();
	MZFile mzf;
	if(!mzf.Open("system/filelist.xml",pfs))
		return false;

	char *buffer;
	buffer=new char[mzf.GetLength()+1];
	mzf.Read(buffer,mzf.GetLength());
	buffer[mzf.GetLength()]=0;

	MXmlDocument aXml;
	aXml.Create();
	if(!aXml.LoadFromMemory(buffer))
	{
		delete buffer;
		return false;
	}

	delete buffer;

	int iCount, i;
	MXmlElement		aParent, aChild;
	aParent = aXml.GetDocumentElement();
	iCount = aParent.GetChildNodeCount();

	char szTagName[256];
	for (i = 0; i < iCount; i++)
	{
		aChild = aParent.GetChildNode(i);
		aChild.GetTagName(szTagName);
		if(stricmp(szTagName,"FILE")==0)
		{
			char szContents[256],szCrc32[256];
			aChild.GetAttribute(szContents,"NAME");
			aChild.GetAttribute(szCrc32,"CRC32");

			if(stricmp(szContents,"config.xml")!=0)
			{
				unsigned int crc32_list = pfs->GetCRC32(szContents);
				unsigned int crc32_current;
				sscanf(szCrc32,"%x",&crc32_current);

				if(crc32_current!=crc32_list)
				{
					mlog("crc error , file %s ( current = %x, original = %x ).\n",szContents,crc32_current,crc32_list);
					// 모든 파일을 검사는 한다
					//	return false; 
				}
			}
		}
	}

	return true;
}

enum RBASE_FONT{
	RBASE_FONT_GULIM = 0,
	RBASE_FONT_BATANG = 1,

	RBASE_FONT_END
};

static int g_base_font[RBASE_FONT_END];
static char g_UserDefineFont[256];

bool _GetFileFontName(char* pUserDefineFont)
{
	if(pUserDefineFont==NULL) return false;

	FILE* fp = fopen("_Font", "rt");
	if (fp) {
		fgets(pUserDefineFont,256, fp);
		fclose(fp);
		return true;
	}
	return false;
}


bool CheckFont()
{
	char FontPath[MAX_PATH];
	char FontNames[MAX_PATH+100];

	::GetWindowsDirectory(FontPath, MAX_PATH);

	strcpy(FontNames,FontPath);
	strcat(FontNames, "\\Fonts\\gulim.ttc");

	if (_access(FontNames,0) != -1)	{ g_base_font[RBASE_FONT_GULIM] = 1; }
	else							{ g_base_font[RBASE_FONT_GULIM] = 0; }

	strcpy(FontNames,FontPath);
	strcat(FontNames, "\\Fonts\\batang.ttc");

	if (_access(FontNames,0) != -1)	{ g_base_font[RBASE_FONT_BATANG] = 1; }
	else							{ g_base_font[RBASE_FONT_BATANG] = 0; }

	//	strcpy(FontNames,FontPath);
	//	strcat(FontNames, "\\Fonts\\System.ttc");
	//	if (_access(FontNames,0) != -1)	{ g_font[RBASE_FONT_BATANG] = 1; }
	//	else							{ g_font[RBASE_FONT_BATANG] = 0; }

	if(g_base_font[RBASE_FONT_GULIM]==0 && g_base_font[RBASE_FONT_BATANG]==0) {//둘다없으면..

		if( _access("_Font",0) != -1) { // 이미 기록되어 있다면..
			_GetFileFontName( g_UserDefineFont );
		}
		else {

			int hr = IDOK;

			//hr = ::MessageBox(NULL,"귀하의 컴퓨터에는 건즈가 사용하는 (굴림,돋움) 폰트가 없는 것 같습니다.\n 다른 폰트를 선택 하시겠습니까?","알림",MB_OKCANCEL);
			//hr = ::MessageBox(NULL,"귀하의 컴퓨터에는 건즈가 사용하는 (굴림,돋움) 폰트가 없는 것 같습니다.\n 계속 진행 하시겠습니까?","알림",MB_OKCANCEL);

			if(hr==IDOK) {
				/*			
				CFontDialog dlg;
				if(dlg.DoModal()==IDOK) {
				CString facename = dlg.GetFaceName();
				lstrcpy((LPSTR)g_UserDefineFont,(LPSTR)facename.operator const char*());

				hr = ::MessageBox(NULL,"선택하신 폰트를 저장 하시겠습니까?","알림",MB_OKCANCEL);

				if(hr==IDOK)
				_SetFileFontName(g_UserDefineFont);
				}
				*/
				return true;
			}
			else {
				return false;
			}
		}
	}
	return true;
}

#include "shlobj.h"

void CheckFileAssociation()
{
#define GUNZ_REPLAY_CLASS_NAME	"GunzReplay"

	// 체크해봐서 등록이 안되어있으면 등록한다. 사용자에게 물어볼수도 있겠다.
	char szValue[256];
	if(!MRegistry::Read(HKEY_CLASSES_ROOT,"."GUNZ_REC_FILE_EXT,NULL,szValue))
	{
		MRegistry::Write(HKEY_CLASSES_ROOT,"."GUNZ_REC_FILE_EXT,NULL,GUNZ_REPLAY_CLASS_NAME);

		char szModuleFileName[_MAX_PATH] = {0,};
		GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);

		char szCommand[_MAX_PATH];
		sprintf(szCommand,"\"%s\" \"%%1\"",szModuleFileName);

		MRegistry::Write(HKEY_CLASSES_ROOT,GUNZ_REPLAY_CLASS_NAME"\\shell\\open\\command",NULL,szCommand);

		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, NULL, NULL);
	}
}

// XTRAP CRC 다르면 강제패치하는 문제로 SKIP 뒷구멍추가
bool CheckXTrapPatchSkip()
{
	char szBuf[256]="";
	FILE* fp = fopen("test.txt", "rt");
	if (fp) {
		fgets(szBuf, 256, fp);
		mlog("test.txt : \n");
		mlog(szBuf);
		mlog("\n");

		fclose(fp);
		if (stricmp(szBuf, "XTRAP_PATCH_SKIP")==0)
			return true;
		else
			return false;
	} else {
		mlog("test.txt not found \n");
		return false;
	}
}

void UpgradeMrsFile()
{
	char temp_path[ 1024];
	sprintf( temp_path,"*");

	FFileList file_list;
	GetFindFileListWin(temp_path,".mrs",file_list);
	file_list.UpgradeMrs();
}

HANDLE Mutex = 0;

#ifdef _HSHIELD
int __stdcall AhnHS_Callback(long lCode, long lParamSize, void* pParam);
#endif

DWORD g_dwMainThreadID;

int PASCAL WinMain(HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow)
{
	g_dwMainThreadID = GetCurrentThreadId();
	
#ifdef _MTRACEMEMORY
	MInitTraceMemory();
#endif

//	_CrtSetBreakAlloc(25483);

	// Current Directory를 맞춘다.
	char szModuleFileName[_MAX_DIR] = {0,};
	GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);
	PathRemoveFileSpec(szModuleFileName);
	SetCurrentDirectory(szModuleFileName);

#ifdef _PUBLISH
	// 중복 실행 금지
	Mutex = CreateMutex(NULL, TRUE, "Gunz");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		exit(-1);
		return 0;
	}
#endif

#ifdef _HSHIELD

	TCHAR szFullFileName[_MAX_DIR];
	GetCurrentDirectory( sizeof( szFullFileName), szFullFileName);
	strcat( szFullFileName, "\\HShield\\EhSvc.dll");

	int nRet = 0;

#ifdef _DEBUG
#define AHNHS_CHKOPT_GUNZ		AHNHS_CHKOPT_SPEEDHACK
#else
#define AHNHS_CHKOPT_GUNZ		AHNHS_CHKOPT_ALL
#endif

#ifdef  LOCALE_BRAZIL					/* Brazil */
	nRet = _AhnHS_Initialize(szFullFileName, AhnHS_Callback, 4001, "DA0EF49C0F6D029F", AHNHS_CHKOPT_GUNZ
		| AHNHS_ALLOW_SVCHOST_OPENPROCESS | AHNHS_ALLOW_LSASS_OPENPROCESS | AHNHS_ALLOW_CSRSS_OPENPROCESS | AHNHS_DONOT_TERMINATE_PROCESS
		, AHNHS_SPEEDHACK_SENSING_RATIO_NORMAL);

#elif  LOCALE_INDIA						/* India */
	nRet = _AhnHS_Initialize(szFullFileName, AhnHS_Callback, 4003, "CC4686AE10F63E4A", AHNHS_CHKOPT_GUNZ
		| AHNHS_ALLOW_SVCHOST_OPENPROCESS | AHNHS_ALLOW_LSASS_OPENPROCESS | AHNHS_ALLOW_CSRSS_OPENPROCESS | AHNHS_DONOT_TERMINATE_PROCESS
		, AHNHS_SPEEDHACK_SENSING_RATIO_NORMAL);
#endif

#ifndef _DEBUG
	// 아래 코드는 릴리즈시 _AhnHS_Initialize 옵션이 AHNHS_CHKOPT_ALL로 되어있지 않을 경우 
	// 컴파일 타임에 에러를 일으켜서 알 수 있게 한다.
	void* OptionCheckTool[(AHNHS_CHKOPT_GUNZ == AHNHS_CHKOPT_ALL)?1:0];
#endif

	

	//아래 에러는 개발과정에서만 발생할 수 있으면 
	//이후 결코 발생해서는 안되는 에러이므로 assert처리를 했습니다.
	assert(nRet != HS_ERR_INVALID_PARAM);
	assert(nRet != HS_ERR_INVALID_LICENSE);
	assert(nRet != HS_ERR_ALREADY_INITIALIZED);

	TCHAR szTitle[256] = "Hack Shield Error";

	if (nRet != HS_ERR_OK) 
	{
		//Error 처리 
		switch(nRet)
		{
		case HS_ERR_ANOTHER_SERVICE_RUNNING:
			{
//				MessageBox(NULL, _T("다른 게임이 실행중입니다.\n프로그램을 종료합니다."), szTitle, MB_OK);
				mlog( "다른 게임이 실행중입니다. 프로그램을 종료합니다.\n");
				break;
			}
		case HS_ERR_INVALID_FILES:
			{
//				MessageBox(NULL, _T("잘못된 파일 설치되었습니다.\n프로그램을 재설치하시기 바랍니다."), szTitle, MB_OK);
				mlog( "잘못된 파일 설치되었습니다. 프로그램을 재설치하시기 바랍니다.\n");
				break;
			}
		case HS_ERR_DEBUGGER_DETECT:
			{
//				MessageBox(NULL, _T("컴퓨터에서 디버거 실행이 감지되었습니다.\n디버거의 실행을 중지시킨 뒤에 다시 실행시켜주시기바랍니다."), szTitle, MB_OK);
				mlog( "컴퓨터에서 디버거 실행이 감지되었습니다. 디버거의 실행을 중지시킨 뒤에 다시 실행시켜주시기바랍니다.\n");
				break;
			}
		case HS_ERR_NEED_ADMIN_RIGHTS:
			{
//				MessageBox(NULL, _T("Admin 권한으로 실행되어야 합니다.\n프로그램을 종료합니다."), szTitle, MB_OK);
				mlog( "Admin 권한으로 실행되어야 합니다. 프로그램을 종료합니다.\n");
				break;
			}
		case HS_ERR_COMPATIBILITY_MODE_RUNNING:
			{
//				MessageBox(NULL, _T("호환성 모드로 프로그램이 실행중입니다.\n프로그램을 종료합니다."), szTitle, MB_OK);
				mlog( "호환성 모드로 프로그램이 실행중입니다. 프로그램을 종료합니다.\n");
				break;				
			}
		default:
			{
				TCHAR szMsg[255];
				wsprintf(szMsg, _T("해킹방지 기능에 문제가 발생하였습니다.(Error Code = %x)\n프로그램을 종료합니다."), nRet);
//				MessageBox(NULL, szMsg, szTitle, MB_OK);
				mlog( szMsg);
				break;
			}
		}

		return FALSE;
	}

	//시작 함수 호출 
	nRet = _AhnHS_StartService();

	assert(nRet != HS_ERR_NOT_INITIALIZED);
	assert(nRet != HS_ERR_ALREADY_SERVICE_RUNNING);

	if (nRet != HS_ERR_OK)
	{
//		TCHAR szMsg[255];
		mlog(_T("해킹방지 기능에 문제가 발생하였습니다.(Error Code = %x)\n프로그램을 종료합니다."), nRet);
//		MessageBox(NULL, szMsg, szTitle, MB_OK);

		return FALSE;
	}

	nRet = _AhnHS_SaveFuncAddress(6, _AhnHS_Initialize, _AhnHS_StartService,			// 핵실드 함수
										ZCheckHackProcess, ZGetMZFileChecksum,				// 핵판별 자체함수
										ZSetupDataChecker_Global, ZSetupDataChecker_Game);
	if(nRet != ERROR_SUCCESS)
	{
//		AfxMessageBox(_T("_AhnHS_SaveFuncAddress Failed!"));
		return FALSE;
	}
#endif // _HSHIELD


#ifdef _XTRAP
	{
	char	sGameName[MAX_PATH];					/* example "neoGame"     */
	char	sGamePath[MAX_PATH];					/* example "C:\\neoGame" */
	char	sGameProcessName[_MAX_PATH];			/* example "neoGame.exe" */
	char	sUpdateURL[512];						/* example "http://netmarblecdn.cachenet.com/cp/Gunz/XTrap" */

/* LOCALE_KOREA */
	DWORD	ApiVersion		= 0xA5001166;
	DWORD	VendorCode		= 0x04009010;			/* 부여된 벤더코드 사용 */
	DWORD	KeyboardType	= 0x00000000;
	DWORD	PatchType		= 0x00000010;			/* 정의된 패치사용 타입 */
	DWORD	ModuleType		= 0x0FFF3FFF;			/* 정의된 모듈사용 타입 */
	sprintf(sGameName, "%s", "gunz_KOR");			/* 게임 식별이름 */
	strcpy( sUpdateURL, "http://netmarblecdn.cachenet.com/cp/Gunz/XTrap");

/* LOCALE_JAPAN */
#ifdef LOCALE_JAPAN
	ApiVersion				= 0xA5001166;
	VendorCode				= 0x04010030;			/* 부여된 벤더코드 사용 */
 	sprintf(sGameName, "%s", "gunz_JPN");			/* 게임 식별이름 */
	strcpy( sUpdateURL, "http://netmarble.nefficient.jp/netmarble/Game/CP/Gunz/XTrap");
#endif

/* LOCALE_US */
#ifdef LOCALE_US
	ApiVersion				= 0xA5001166;
	VendorCode				= 0x040090FF;			/* 부여된 벤더코드 사용 */
	PatchType				= 0;					/* 정의된 패치사용 타입 */
	sprintf(sGameName, "%s", "gunz_INT");			/* 게임 식별이름 */
	strcpy( sUpdateURL, "");
#endif

/* LOCALE_INDIA */
#ifdef LOCALE_INDIA
	ApiVersion				= 0xA5001166;
	VendorCode				= 0x00000014;			/* 부여된 벤더코드 사용 */
	sprintf(sGameName, "%s", "gunz_IND");			/* 게임 식별이름 */
	strcpy( sUpdateURL, "http://patch.gunzonline.co.in/gunzweb/GunzDownload/XTrap");
#endif

/* LOCALE_BRAZIL */
#ifdef LOCALE_BRAZIL
	ApiVersion				= 0xA5001166;
	VendorCode				= 0x04009220;			/* 부여된 벤더코드 사용 */
	sprintf(sGameName, "%s", "gunz_BRZ");			/* 게임 식별이름 */
	strcpy( sUpdateURL, "http://fpatch.theduel.com.br/gunzweb/GunzDownload/XTrap");
#endif


#ifdef _DEBUG
	sprintf(sGameName, "%s", "gunz_TEST");			/* 게임 식별이름 */
#endif


	sprintf(sGameProcessName, "Gunz.exe");			/* 게임 프로세서명을 입력 */
	sprintf(sGamePath, "%s", szModuleFileName);		/* 게임 프로세서가 실행된 경로명 입력 */


	/* Option SetXTrapMgrInfo(char *pMgrIp) */

	// XTRAP CRC 다르면 강제패치하는 문제로 SKIP 뒷구멍추가
	if (CheckXTrapPatchSkip() == true) {
		PatchType = 0;
	}

//	SetXTrapPatchInfo(sUpdateURL);
//	SetXTrapStartInfo	(sGameName, sGameProcessName, sGamePath, 
//						 ApiVersion, VendorCode, KeyboardType, PatchType, ModuleType);

	SetXTrapPatchHttpUrl( sUpdateURL);
	SetXTrapStartInfo( sGameName, ApiVersion, VendorCode, KeyboardType, PatchType, ModuleType);			// sGameProcessName, sGamePath 사용 안함.



#ifdef _DEBUG
	mlog( "XTRAP setting : Game=%s, Process=%s, Path=%s, ApiVer=0x%08X, Vendor=0x%08X, Kbd=0x%08X, Patch=0x%08X, Module=0x%08X\n",
				sGameName, sGameProcessName, sGamePath, ApiVersion, VendorCode, KeyboardType, PatchType, ModuleType);
#endif

	XTrapStart			();
	XTrapKeepAlive		();
	}

#endif

	ClearTrashFiles();

	InitLog(MLOGSTYLE_DEBUGSTRING|MLOGSTYLE_FILE);

	srand( (unsigned)time( NULL ));


	// 로그 시작
	mlog("GUNZ " STRFILEVER " launched. build ("__DATE__" "__TIME__") \n");

	char szDateRun[128]="";
	char szTimeRun[128]="";
	_strdate( szDateRun );
	_strtime( szTimeRun );
	mlog("Log time (%s %s)\n", szDateRun, szTimeRun);

	if (CheckXTrapPatchSkip()) {
		mlog("Skip XTrapPatch\n");
	} else {
		mlog("NON-Skip XTrapPatch\n");
	}


#ifndef _PUBLISH
	mlog("cmdline = %s\n",cmdline);
#endif

#ifndef _LAUNCHER
	UpgradeMrsFile();// mrs1 이라면 mrs2로 업그래이드 한다..
#endif

	MSysInfoLog();

//	if (CheckVideoAdapterSupported() == false)
//		return 0;

	CheckFileAssociation();

	// Initialize MZFileSystem - MUpdate 
	MRegistry::szApplicationName=APPLICATION_NAME;

	g_App.InitFileSystem();
//	mlog("CheckSum: %u \n", ZApplication::GetFileSystem()->GetTotalCRC());

	//if(!InitializeMessage(ZApplication::GetFileSystem())) {
	//	MLog("Check Messages.xml\n");
	//	return 0;
	//}

//	넷마블 버전은 구분해야함... 넷마블 버전은 MZIPREADFLAG_MRS1 도 읽어야함...

#ifdef _PUBLISH
//	#ifndef NETMARBLE_VERSION
		MZFile::SetReadMode( MZIPREADFLAG_MRS2 );
//	#endif
#endif

	// StringRes먼저 로드하고 그다음에 Config을 로드한다.
	ZGetConfiguration()->Load();

	ZStringResManager::MakeInstance();
	if( !ZApplication::GetInstance()->InitLocale() )
	{
		MLog("Locale Init error !!!\n");
		return false;
	}

	// 여기서 메크로 컨버팅... 먼가 구리구리~~ -by SungE.
	if( !ZGetConfiguration()->LateStringConvert() )
	{
		MLog( "main.cpp - Late string convert fale.\n" );
		return false;
	}

	DWORD ver_major = 0;
	DWORD ver_minor = 0;
	TCHAR ver_letter = ' ';

	// 의미없음 ... 외부에서 dll 이 없다고 먼저뜸...

/*_
	bool DXCheck = false;

	if( SUCCEEDED( GetDirectXVersionViaDxDiag( &ver_major, &ver_minor, &ver_letter ) ) ) {
		if(ver_major >= 8)
			DXCheck = true;
	} // 성공 못한 경우 알수없으므로 실패~

	if(DXCheck==false) {
		::MessageBox(NULL,"DirectX 8.0 이상을 설치하고 다시 실행해 주시기 바랍니다.","알림",MB_OK);
	}
*/

#ifdef SUPPORT_EXCEPTIONHANDLING
	char szDumpFileName[256];
	sprintf(szDumpFileName, "Gunz.dmp");
	__try{
#endif

	if (ZApplication::GetInstance()->ParseArguments(cmdline) == false)
	{
		// Korean or Japan Version
		if ((ZGetLocale()->GetCountry() == MC_KOREA) || (ZGetLocale()->GetCountry() == MC_JAPAN))
		{
			mlog("Routed to Website \n");

			ShellExecute(NULL, "open", ZGetConfiguration()->GetLocale()->szHomepageUrl, NULL, NULL, SW_SHOWNORMAL);

			char szMsgWarning[128]="";
			char szMsgCertFail[128]="";
			ZTransMsg(szMsgWarning,MSG_WARNING);
			ZTransMsg(szMsgCertFail,MSG_REROUTE_TO_WEBSITE);
//			MessageBox(g_hWnd, szMsgCertFail, szMsgWarning, MB_OK);

			mlog(szMsgWarning);
			mlog(" : ");
			mlog(szMsgCertFail);

			return 0;
		}
		else
		{
			return 0;
		}
	}

//#ifdef _PUBLISH
	if(!CheckFileList()) {
		// 종료하는것은 일단 보류
		// int ret=MessageBox(NULL, "파일이 손상되었습니다.", "중요!", MB_OK);
		// return 0;
	}
//#endif

	if (ZCheckHackProcess() == true)
	{
//		MessageBox(NULL,
//			ZMsg(MSG_HACKING_DETECTED), ZMsg( MSG_WARNING), MB_OK);
		mlog(ZMsg(MSG_HACKING_DETECTED));
		mlog("\n");
		return 0;
	}

	if(!InitializeNotify(ZApplication::GetFileSystem())) {
		MLog("Check notify.xml\n");
		return 0;
	}

	// font 있는가 검사..

	if(CheckFont()==false) {
		MLog("폰트가 없는 유저가 폰트 선택을 취소\n");
		return 0;
	}

	RSetFunction(RF_CREATE	,	OnCreate);
	RSetFunction(RF_RENDER	,	OnRender);
	RSetFunction(RF_UPDATE	,	OnUpdate);
	RSetFunction(RF_DESTROY ,	OnDestroy);
	RSetFunction(RF_INVALIDATE,	OnInvalidate);
	RSetFunction(RF_RESTORE,	OnRestore);
	RSetFunction(RF_ACTIVATE,	OnActivate);
	RSetFunction(RF_DEACTIVATE,	OnDeActivate);
	RSetFunction(RF_ERROR,		OnError);

	SetModeParams();

//	while(ShowCursor(FALSE)>0);

	int nReturn = RMain(APPLICATION_NAME,this_inst,prev_inst,cmdline,cmdshow,&g_ModeParams,WndProc,IDI_ICON1);

#ifdef _MTRACEMEMORY
	MShutdownTraceMemory();
#endif

#ifdef _HSHIELD
	_AhnHS_StopService();
	_AhnHS_Uninitialize();		
#endif

#ifdef _XTRAP
//	XTrapStop			();				// 새버젼에선 더이상 사용하지 않는다.
#endif
	return nReturn;

//	ShowCursor(TRUE);

#ifdef SUPPORT_EXCEPTIONHANDLING
	}
	//__except(MFilterException(GetExceptionInformation())){
	__except(CrashExceptionDump(GetExceptionInformation(), szDumpFileName, true)){
		HandleExceptionLog();
//		MessageBox(g_hWnd, "예상치 못한 오류가 발생했습니다.", APPLICATION_NAME , MB_ICONERROR|MB_OK);
	}
#endif

#ifdef _PUBLISH
	if (Mutex != 0) CloseHandle(Mutex);
#endif

//	CoUninitialize();

	return 0;
}

#ifdef _HSHIELD
int __stdcall AhnHS_Callback(long lCode, long lParamSize, void* pParam)
{
//	TCHAR szTitle[256];

	switch(lCode)
	{
		//Engine Callback
	case AHNHS_ENGINE_DETECT_GAME_HACK:
		{
			TCHAR szMsg[255];
			wsprintf(szMsg, _T("다음 위치에서 해킹툴이 발견되어 프로그램을 종료시켰습니다.\n%s"), (char*)pParam);
//			MessageBox(NULL, szMsg, szTitle, MB_OK);
			mlog(szMsg);
			PostThreadMessage(g_dwMainThreadID, WM_QUIT, 0, 0);
			break;
		}

		//일부 API가 이미 후킹되어 있는 상태
		//그러나 실제로는 이를 차단하고 있기 때문에 다른 후킹시도 프로그램은 동작하지 않습니다.
		//이 Callback은 단지 경고 내지는 정보제공 차원에서 제공되므로 게임을 종료할 필요가 없습니다.
	case AHNHS_ACTAPC_DETECT_ALREADYHOOKED:
		{
			PACTAPCPARAM_DETECT_HOOKFUNCTION pHookFunction = (PACTAPCPARAM_DETECT_HOOKFUNCTION)pParam;
			TCHAR szMsg[255];
			wsprintf(szMsg, _T("[HACKSHIELD] Already Hooked\n- szFunctionName : %s\n- szModuleName : %s\n"), 
				pHookFunction->szFunctionName, pHookFunction->szModuleName);
			OutputDebugString(szMsg);
			break;
		}

		//Speed 관련
	case AHNHS_ACTAPC_DETECT_SPEEDHACK:
	case AHNHS_ACTAPC_DETECT_SPEEDHACK_APP:
		{
//			MessageBox(NULL, _T("현재 이 PC에서 SpeedHack으로 의심되는 동작이 감지되었습니다."), szTitle, MB_OK);
			mlog("현재 이 PC에서 SpeedHack으로 의심되는 동작이 감지되었습니다.");
			PostThreadMessage(g_dwMainThreadID, WM_QUIT, 0, 0);
			break;
		}

		//디버깅 방지 
	case AHNHS_ACTAPC_DETECT_KDTRACE:	
	case AHNHS_ACTAPC_DETECT_KDTRACE_CHANGED:
		{
			TCHAR szMsg[255];
			wsprintf(szMsg, _T("프로그램에 대하여 디버깅 시도가 발생하였습니다. (Code = %x)\n프로그램을 종료합니다."), lCode);
//			MessageBox(NULL, szMsg, szTitle, MB_OK);
			mlog(szMsg);
			PostThreadMessage(g_dwMainThreadID, WM_QUIT, 0, 0);
			break;
		}

		//그외 해킹 방지 기능 이상 
	case AHNHS_ACTAPC_DETECT_AUTOMOUSE:
	case AHNHS_ACTAPC_DETECT_DRIVERFAILED:
	case AHNHS_ACTAPC_DETECT_HOOKFUNCTION:
	case AHNHS_ACTAPC_DETECT_MESSAGEHOOK:
	case AHNHS_ACTAPC_DETECT_MODULE_CHANGE:
		{
			TCHAR szMsg[255];
			wsprintf(szMsg, _T("해킹 방어 기능에 이상이 발생하였습니다. (Code = %x)\n프로그램을 종료합니다."), lCode);
//			MessageBox(NULL, szMsg, szTitle, MB_OK);
			mlog(szMsg);
			PostThreadMessage(g_dwMainThreadID, WM_QUIT, 0, 0);
			break;
		}
	}
	return 1;
}
#endif