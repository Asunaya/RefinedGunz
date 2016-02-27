#include <windows.h>
#include <mmsystem.h>

#include "MDebug.h"
#include "RealSpace2.h"

#include "RMtrl.h"

#include "RMesh.h"
#include "RMeshList.h"

#include "RBspObject.h"
#include "RMaterialList.h"

RMODEPARAMS mparams={ 640,480,false,RPIXELFORMAT_565 };

LPDIRECT3DDEVICE8 g_dev = NULL;

RMeshList	g_mesh_list;
int			g_id;

RBspObject g_bsp;

BOOL InitScene(HWND hWnd)
{
	InitLog(MLOGSTYLE_DEBUGSTRING|MLOGSTYLE_FILE);
	
	RInitDisplay(hWnd,&mparams);
	
	RSetCamera(rvector(-100.f,0.f,50.f),rvector(0.f,0.f,0.f),rvector(0.f,1.f,0.f));
	RSetProjection(1.5,5,1000);

	g_dev = RGetDevice();

	g_id = g_mesh_list.Add("1111.elu");

	if(g_id == -1) return false;

	g_mesh_list.GetFast(g_id)->m_mtrl_list_ex.Restore(g_dev);//tex 생성 임시 mtrl : 통합관리시 제거
//	g_mesh_list.GetFast(g_id)->ReadEluAni("1111.elu.ani");

	RMaterialList test;
	test.Open("c:/3dsmax3_1/meshes/test.rml");
	g_bsp.Open("c:/3dsmax3_1/meshes/test.rbs",&test);

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
	
	return TRUE;
}

BOOL CloseScene()
{
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

	static float rotatez=0.f,rotatex=pi/2.f;

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

	
	{

		struct LVERTEX {
			float x, y, z;		// world position
			DWORD color;
		} ;

	RGetDevice()->SetTexture(0,NULL);
    RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );

#define SCALE	100
		LVERTEX lines[6]={{0,0,0,0xff0000},{SCALE,0,0,0xff0000},
						{0,0,0,0xff00},{0,SCALE,0,0xff00},
						{0,0,0,0xff},{0,0,SCALE,0xff}};

		RGetDevice()->SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE );
		RGetDevice()->DrawPrimitiveUP(D3DPT_LINELIST,3,lines,sizeof(LVERTEX));
	}
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

	g_mesh_list.Frame();
//	g_mesh_list.Render();

	g_bsp.Draw();

	RFlip();
	return TRUE;
}

////////////////////////// followings are Nothings...

HWND g_hWnd;
BOOL g_bActive;
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
    wc.lpszClassName = "RealSpace2";
	if(!RegisterClass(&wc)) return FALSE;

    g_hWnd = CreateWindow( "RealSpace2", "rs2test",
            WS_VISIBLE | WS_POPUP | WS_CAPTION, CW_USEDEFAULT, CW_USEDEFAULT, 
			640, 480, NULL, NULL, this_inst , NULL );
	ShowWindow(g_hWnd,SW_SHOW);
	ShowCursor(TRUE);

    if (!g_hWnd) return FALSE;

    return TRUE;
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
					g_mesh_list.GetFast(g_id)->ReadEluAni("1111.elu.ani");
					break;

				case VK_F2:
					g_mesh_list.GetFast(g_id)->ReadEluAni("2222.elu.ani");
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
		}
		break;

    }
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
