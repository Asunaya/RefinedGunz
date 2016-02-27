#include <windows.h>
#include <mmsystem.h>

#include "RFont.h"

#include "MDebug.h"
#include "RealSpace2.h"

#include "RMtrl.h"

#include "RMesh.h"
#include "RMeshList.h"

#include "RBspObject.h"
#include "RMaterialList.h"

#include "MWidget.h"
#include "Mint4R2.h"
#include "MFileDialog.h"

#include <tchar.h>
#include "Mint.h"

#include "RBaseTexture.h"

#include <io.h>

RMODEPARAMS mparams={ 640,480,false,RPIXELFORMAT_565 };

LPDIRECT3DDEVICE8 g_dev = NULL;

int			g_id;
HWND g_hWnd;
BOOL g_bActive;


RFontTexture ft;	// Font
RFont g_Font;
RBaseTexture	g_TestTexture;

BOOL InitScene(HWND hWnd)
{
	InitLog(MLOGSTYLE_DEBUGSTRING|MLOGSTYLE_FILE);
	
	RInitDisplay(hWnd,&mparams);
	
	RSetCamera(rvector(0.f,100.f,-200.f),rvector(0.f,0.f,0.f),rvector(0.f,1.f,0.f));
	RSetProjection(1.5,5,1000);

	g_dev = RGetDevice();

	if(g_id == -1) return false;


	RMaterialList test;

    D3DMATERIAL8 mtrl;
    ZeroMemory( &mtrl, sizeof(D3DMATERIAL8) );

    mtrl.Diffuse.r = 1.0f;
    mtrl.Diffuse.g = 1.0f;
    mtrl.Diffuse.b = 1.0f;
    mtrl.Diffuse.a = 1.0f;

	mtrl.Ambient.r = 1.0f;
	mtrl.Ambient.g = 1.0f;
	mtrl.Ambient.b = 1.0f;
	mtrl.Ambient.a = 1.0f;

    g_dev->SetMaterial( &mtrl );
	g_dev->SetRenderState( D3DRS_AMBIENT, 0x00000000 );

    D3DLIGHT8 light;

    ZeroMemory( &light, sizeof(D3DLIGHT8) );

	light.Type       = D3DLIGHT_POINT;
	
	light.Diffuse.r  = 1.0f;
	light.Diffuse.g  = 1.0f;
	light.Diffuse.b  = 1.0f;

	light.Ambient.r  = 0.5f;
	light.Ambient.g  = 0.5f;
	light.Ambient.b  = 0.5f;

	light.Position.x = 10;
	light.Position.y = 10;
	light.Position.z = 10;

	light.Attenuation1 = 0.01f;

	light.Range       = 1000.0f;

	g_dev->SetLight( 0, &light );
	g_dev->LightEnable( 0, TRUE );

	light.Type       = D3DLIGHT_DIRECTIONAL;

	D3DXVECTOR3 v = D3DXVECTOR3(0.f, 0.f,1.f );
	D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction, &v );

	g_dev->SetLight( 1, &light );
	g_dev->LightEnable( 1, TRUE );

	g_dev->SetRenderState( D3DRS_LIGHTING, TRUE );

	/*
    HFONT hFont    = CreateFont( 20, 0, 0, 0, FW_REGULAR, FALSE,
                          FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                          VARIABLE_PITCH, "굴림");
	ft.Create(g_dev, hFont, "Hello Realspace2! MINT에서 Realspace2를 지원합니다. ★");
	
	//g_Font.Create(g_dev, "송성훈 가나다", 16, false, false, 100);
	g_Font.Create(g_dev, "Tahoma", 50, false, false, 10);
	*/

	MFontR2* pDefFont = new MFontR2;
	pDefFont->Create("Default", g_dev, "Tahoma", 14);

	MLoadDesignerMode();
	MInitialize(640, 480, pDefFont);

    struct _finddata_t c_file;
    long hFile;

	if( (hFile = _findfirst("*.bmp", &c_file )) != -1L ){
		do{
			MBitmapR2* pBitmap = new MBitmapR2;
			if(pBitmap->Create(c_file.name, g_dev, c_file.name)==true)
				MBitmapManager::Add(pBitmap);
			else
				delete pBitmap;
		}while( _findnext( hFile, &c_file ) == 0 );
		_findclose(hFile);
	}

	MCreateSample();

	//MSetDesignerMode(true);

	return TRUE;
}

BOOL CloseScene()
{
	//ft.Destroy();

	//g_Font.Destroy();

	MFinalize();

	RCloseDisplay();
	return TRUE;
}

#define IsKeyDown(key) ((GetAsyncKeyState(key) & 0x8000)!=0)
#pragma comment(lib,"winmm.lib")

void Update()
{
	static DWORD thistime,lasttime=timeGetTime(),elapsed;

	thistime = timeGetTime();
	elapsed = (thistime - lasttime)*(IsKeyDown(VK_SHIFT)?5:1);
	lasttime = thistime;

	static float rotatez=0.7f,rotatex=2.5f;

	float fRotateStep=elapsed*0.001f;
	float fMoveStep=elapsed*0.1f;


	if(IsKeyDown(VK_LEFT)) rotatez-=fRotateStep;
	if(IsKeyDown(VK_RIGHT)) rotatez+=fRotateStep;
	if(IsKeyDown(VK_UP)) rotatex-=fRotateStep;
	if(IsKeyDown(VK_DOWN)) rotatex+=fRotateStep;

	rvector pos=RGetCameraPosition(),dir=rvector(cosf(rotatez)*sinf(rotatex),sinf(rotatez)*sinf(rotatex),cosf(rotatex));
	D3DXVec3Normalize(&dir,&dir);
	rvector right;
	D3DXVec3Cross(&right,&dir,&rvector(0,0,1));
	D3DXVec3Normalize(&right,&right);

	if(IsKeyDown('W')) pos+=fMoveStep*dir;
	if(IsKeyDown('S')) pos-=fMoveStep*dir;
	if(IsKeyDown('A')) pos+=fMoveStep*right;
	if(IsKeyDown('D')) pos-=fMoveStep*right;
	if(IsKeyDown(VK_SPACE)) pos+=fMoveStep*rvector(0,0,1);

//	gClipAngle +=2.1f;
//	gClipAngle=fmodf(gClipAngle,180);
	
	rvector at=pos+dir;
	//at.z=0;
	RSetCamera(pos,at,rvector(0,0,1));
//	mlog("%3.3f %3.3f\n",rotatex,rotatez);

}

BOOL RenderScene()
{
	Update();

	RRESULT isOK=RIsReadyToRender();
	if(isOK==R_NOTREADY)
		return false;
	else
	if(isOK==R_RESTORED)
	{
	} // restore device dependent objects

	/*
	// Draw Everything
	//ft.DrawText(10, 10, 0xFFFFFFFF);
	//ft.DrawText(15, 15, 0xFFFFFFFF);
	int nRawSize = sizeof(szStar)/sizeof(char*);
	static int nRaw = 0;
	for(int i=0; i<20; i++){
		//g_Font.DrawText(10, 10+i*(g_Font.GetHeight()+2), "Hello Realspace2! asdasdwdwd MINT에서 Realspace2를 지원합니다. ★", 0x77FFFFFF);
		g_Font.DrawText(10, 10+i*(g_Font.GetHeight()+2), szStar[(nRaw+i)%nRawSize], 0xFFFFFFFF, 1.0f);
	}
	nRaw++;
	nRaw%=nRawSize;
	*/
	//g_Font.DrawText(10, 10, "Hello Realspace2! 안녕하세요~", 0xffffffff, 1.0f);

	MRun();
	MDrawContextR2 dc(g_dev);
	MDraw(&dc);

	RFlip();
	return TRUE;
}

////////////////////////// followings are Nothings...

long FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * Register window class for the application, and do any other
 * application initialization
 */
static BOOL FirstInstance(HINSTANCE this_inst)
{
    WNDCLASS    wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(DWORD);
    wc.hInstance = this_inst;
    wc.hIcon = NULL;//LoadIcon(this_inst, "ViewerIcon");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = _T("RealSpace2");
	if(!RegisterClass(&wc)) return FALSE;

    g_hWnd = CreateWindow( _T("RealSpace2"), _T("MINT for Realspace2"),
            WS_VISIBLE | WS_POPUP , CW_USEDEFAULT, CW_USEDEFAULT, 
			640, 480, NULL, NULL, this_inst , NULL );
	ShowWindow(g_hWnd,SW_SHOW);
	ShowCursor(TRUE);

    if (!g_hWnd) return FALSE;

    return TRUE;
}

void OnLostAndResetDevice(void)
{
	for(int i=0; i<MBitmapManager::GetCount(); i++){
		MBitmapR2* pBitmap = (MBitmapR2*)MBitmapManager::Get(i);
		pBitmap->OnLostDevice();
		//pBitmap->OnResetDevice();
	}
}

long FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
    // Handle messages
    switch (message)
    {
        case WM_KEYDOWN:
        {
            switch (wParam)
            {
				case VK_F1:
					break;

				case VK_F2:
					MSetDesignerMode(!MIsDesignerMode());
					break;

                case VK_ESCAPE:	
					PostMessage(g_hWnd,WM_CLOSE,0,0);
					break;
			}
        }
        break;

		case WM_SYSCHAR:
			if(wParam==VK_RETURN)
			{
				mparams.bFullScreen=!mparams.bFullScreen;
				RResetDevice(&mparams);
				LONG WndStyle = GetWindowLong(hWnd, GWL_STYLE);
				if(mparams.bFullScreen==true){
					WndStyle&=~WS_CAPTION;
					WndStyle&=~WS_BORDER;
				}
				else{
					WndStyle|=WS_CAPTION;
					WndStyle|=WS_BORDER;
				}
				SetWindowLong(hWnd, GWL_STYLE, WndStyle);

				OnLostAndResetDevice();
			}return 0;
		case WM_SYSCOMMAND:
			{
				switch (wParam)
				{
					// Trap ALT so it doesn't pause the app
					case SC_KEYMENU :
					{
						return 0;
					}
					break;
				}
			}       
        
        case WM_ACTIVATEAPP:
        {
            // Determine whether app is being activated or not
            g_bActive = (BOOL)wParam ? TRUE : FALSE;
        }
        break;

		case WM_CLOSE:
		{
            PostQuitMessage(0);
			break;
		}

    }

	MProcessEvent(hWnd, message, wParam, lParam);

    return DefWindowProc(hWnd, message, wParam, lParam);
}

int PASCAL WinMain
    (HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow)
{
	if(!FirstInstance(this_inst)) return 1;
	if(!InitScene(g_hWnd)) return 1;

	// message loop
    // Now we're ready to recieve and process Windows messages.
    BOOL bGotMsg;
    MSG  msg;
    PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE );

    while( WM_QUIT != msg.message  )
    {
        // Use PeekMessage() if the app is active, so we can use idle time to
        // render the scene. Else, use GetMessage() to avoid eating CPU time.
        bGotMsg = PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE );
        if( bGotMsg )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
		RenderScene();
    }

	CloseScene();
    return (INT)msg.wParam;
}
