#include <windows.h>
#include <mmsystem.h>

#include "MDebug.h"
#include "RealSpace2.h"
#include "RBspObject.h"
#include "MZFileSystem.h"

RMODEPARAMS mparams={ 800,600,false,D3DFMT_R5G6B5 };

LPDIRECT3DDEVICE9 g_dev = NULL;

RBspObject *g_pBsp=NULL;
//MZFileSystem g_filesystem;

char mapname[256];

bool g_bShowCollision=false;
bool g_bCollision=true;
bool g_bCylinder=true;
bool g_bShowSolid=true;
bool g_bShowMap=true;
bool g_bShowCurrentNodePolygon=false;
bool g_bDrawSolidPolygon=false;
bool g_bWireFrame=false;
bool g_bShowNavigation=false;
bool g_bShowPickPoly = false;

rvector g_cylinderpos;

#define CYLINDER_RADIUS 35

void InitNavigationMesh()
{
	if (g_pBsp==NULL) return;

//	g_pBsp->GetNavigationMesh()->MakeNodes();
}


void OpenFile(const char *szFileName) 
{
	SAFE_DELETE(g_pBsp);
	g_pBsp = new RBspObject;
	if(!g_pBsp->Open(szFileName))
		SAFE_DELETE(g_pBsp);

	InitNavigationMesh();
}

BOOL InitScene(HWND hWnd)
{
//	g_filesystem.Create(".");

	InitLog(MLOGSTYLE_DEBUGSTRING|MLOGSTYLE_FILE);
	
	RInitDisplay(hWnd,&mparams);
	RAdjustWindow(&mparams);
	ShowWindow(::g_hWnd,SW_SHOW);
	ShowCursor(TRUE);
	
	RSetCamera(rvector(-100.f,0.f,50.f),rvector(0.f,0.f,0.f),rvector(0.f,1.f,0.f));
	RSetProjection(1.5,5,10000);

	g_cylinderpos=rvector(-155.17810 , 170.00000 , 190.00000);

	g_dev = RGetDevice();

//	RSetFileSystem(&g_filesystem);
	OpenFile(mapname);

	g_pBsp->test_MakePortals();

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
	if(!g_pBsp) return;

	/*
	bool btest=g_pBsp->CheckWall(RCameraPosition,RCameraDirection*10.f,0,0);
	if(btest)
	{
		btest=g_pBsp->CheckWall(RCameraPosition,RCameraDirection*10.f,0,0);
	}
	*/


	static DWORD thistime,lasttime=timeGetTime(),elapsed;

	thistime = timeGetTime();
	elapsed = (thistime - lasttime)*(IsKeyDown(VK_SHIFT)?5:1);
	lasttime = thistime;

	if(GetActiveWindow()!=g_hWnd) return;

	static float rotatez=0.f,rotatex=pi/2.f;

	float fRotateStep=elapsed*0.001f;
	float fMoveStep=elapsed*0.5f;

	if(IsKeyDown(VK_LEFT)) rotatez-=fRotateStep;
	if(IsKeyDown(VK_RIGHT)) rotatez+=fRotateStep;
	if(IsKeyDown(VK_UP)) rotatex-=fRotateStep;
	if(IsKeyDown(VK_DOWN)) rotatex+=fRotateStep;

	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(g_hWnd,&pt);
	int iDeltaX, iDeltaY;
	iDeltaX = pt.x-RGetScreenWidth()/2;
	iDeltaY = pt.y-RGetScreenHeight()/2;

	rotatez+=iDeltaX*0.01f;
	rotatex+=iDeltaY*0.01f;

	{
		POINT pt={RGetScreenWidth()/2,RGetScreenHeight()/2};
		ClientToScreen(g_hWnd,&pt);
		SetCursorPos(pt.x,pt.y);
	}

	/*
	static DWORD lastprinttime=timeGetTime();
	if(timeGetTime()-lastprinttime>200)
	{
		lastprinttime=timeGetTime();
		mlog("%3.3f %3.3f \n",rotatez,rotatex);
	}
	*/

	rotatex=max(0.01f,min(pi-0.01f,rotatex));

	rvector posdiff=rvector(0,0,0),dir=rvector(cosf(rotatez)*sinf(rotatex),sinf(rotatez)*sinf(rotatex),cosf(rotatex));
	D3DXVec3Normalize(&dir,&dir);
	rvector right;
	D3DXVec3Cross(&right,&dir,&rvector(0,0,1));
	D3DXVec3Normalize(&right,&right);

	if(IsKeyDown('W')) posdiff+=fMoveStep*dir;
	if(IsKeyDown('S')) posdiff-=fMoveStep*dir;
	if(IsKeyDown('A')) posdiff+=fMoveStep*right;
	if(IsKeyDown('D')) posdiff-=fMoveStep*right;
	if(IsKeyDown(VK_SPACE)) posdiff+=fMoveStep*rvector(0,0,1);

//	gClipAngle +=2.1f;
//	gClipAngle=fmodf(gClipAngle,180);
	
	//at.z=0;


	rvector targetpos=g_cylinderpos+posdiff;
	if(g_bCollision && ( posdiff.x!=0 || posdiff.y!=0 || posdiff.z!=0) )
	{
		bool bCol;
		if(g_bCylinder)
			bCol=g_pBsp->CheckWall(g_cylinderpos,targetpos,CYLINDER_RADIUS ,60,RCW_CYLINDER);
		else
			bCol=g_pBsp->CheckWall(g_cylinderpos,targetpos,CYLINDER_RADIUS ,0, RCW_SPHERE);

		//	_ASSERT(!bCol);
	}

	
	g_cylinderpos=targetpos;
	
//	RSetCamera(g_cylinderpos,g_cylinderpos+dir,rvector(0,0,1));	// center camera
	RSetCamera(g_cylinderpos-dir*200.f,g_cylinderpos,rvector(0,0,1));

//	mlog("%3.3f %3.3f\n",rotatex,rotatez);

	
}

BOOL RenderScene()
{
	LPDIRECT3DDEVICE9 pd3dDevice=RGetDevice();

	Update();

	RRESULT isOK=RIsReadyToRender();
	if(isOK==R_NOTREADY)
		return false;
	else
	if(isOK==R_RESTORED)
	{
	} // restore device dependent objects

	if(!g_pBsp) {
		return true;
	}

	if(g_bWireFrame) {
		RGetDevice()->SetRenderState( D3DRS_FILLMODE , D3DFILL_WIREFRAME );
		RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
	}
	else {
		RGetDevice()->SetRenderState( D3DRS_FILLMODE ,  D3DFILL_SOLID );
	}
	g_pBsp->SetWireframeMode(g_bWireFrame);
	RSetWBuffer(true);

	if(g_bShowMap)
	{
		if(g_bDrawSolidPolygon)
		{
			g_pBsp->Draw();
			g_pBsp->DrawSolid();
		}else
		{
			if(g_bShowCollision)
			{
				g_pBsp->DrawCollision_Polygon();
			}
			else
			{
				g_pBsp->Draw();
			}
		}

		if (g_bShowNavigation)
		{
			g_pBsp->DrawNavi_Polygon();
			g_pBsp->DrawNavi_Links();
		}

	}

	if(g_bShowSolid)
	{
		g_pBsp->DrawSolidNode();
	}

	RGetDevice()->SetRenderState( D3DRS_LIGHTING , FALSE );
	if(g_bCollision)
	{
		rmatrix id;
		D3DXMatrixIdentity(&id);
		RGetDevice()->SetTransform( D3DTS_WORLD , &id);

		RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
		RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
		RGetDevice()->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
		RGetDevice()->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

		if(g_bCylinder)
			RDrawCylinder(g_cylinderpos,CYLINDER_RADIUS ,60,10);
		else
			RDrawSphere(g_cylinderpos,CYLINDER_RADIUS ,20);

		RDrawAxis(g_cylinderpos,20);
	}
	/*

	rvector pOut;
	g_pBsp->CheckWall_Corn(&pOut,g_cylinderpos,g_cylinderpos+rvector(200,0,0),30);
	RDrawCorn(pOut+rvector(-200,0,0),pOut,30,10);
	*/

	if(g_bShowCurrentNodePolygon)
	{
		g_pBsp->DrawColNodePolygon(g_cylinderpos);
	}

//	g_pBsp->GetColRoot()->DrawPlaneVertices(rplane(0.689,0.611,-0.390,2136.985));

	if(g_bShowPickPoly) {
		RBSPPICKINFO bpi;

		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(g_hWnd,&pt);

		rvector pos,dir;
		RGetScreenLine(pt.x,pt.y,&pos,&dir);
		bool bPicked=g_pBsp->PickOcTree(pos,dir,&bpi);
		if(bPicked)
		{
			rplane *pPlane=&bpi.pInfo->plane;

			BSPVERTEX *v=bpi.pNode->pInfo[bpi.nIndex].pVertices;

			/*
			sprintf(buffer,"( %3.1f %3.1f %3.1f ) ( %3.1f %3.1f %3.1f ) ( %3.1f %3.1f %3.1f ) ",
				v->x,v->y,v->z,
				(v+1)->x,(v+1)->y,(v+1)->z,
				(v+2)->x,(v+2)->y,(v+2)->z);

			pDC->SetColor(MCOLOR(0xFFffffff));
			OUTTEXT();
			NEXTLINE();
				*/

			RGetDevice()->SetTexture(0,NULL);
			RGetDevice()->SetTexture(1,NULL);
			RGetDevice()->SetRenderState(D3DRS_ZENABLE, false );
			RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );

			for(int i=0;i<bpi.pInfo->nVertices;i++)
			{
				RDrawLine(*v[i].Coord(),*v[(i+1)%bpi.pInfo->nVertices].Coord(),0xffffff00);
			}
		}
	}

	RFlip();

	/*
	static DWORD lastprinttime=timeGetTime();
	if(timeGetTime()-lastprinttime>200)
	{
		lastprinttime=timeGetTime();
		mlog("current pos ( %3.3f %3.3f %3.3f )\n",g_cylinderpos.x,g_cylinderpos.y,g_cylinderpos.z);
	}
	*/

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

	::g_hWnd = CreateWindowEx( WS_EX_ACCEPTFILES,"RealSpace2", "collision",
            WS_POPUP | WS_CAPTION, CW_USEDEFAULT, CW_USEDEFAULT, 
			640, 480, NULL, NULL, this_inst , NULL );

	if (!::g_hWnd) return FALSE;

    return TRUE;
}

RRESULT OnInvalidate(void *pParam)
{
	g_pBsp->OnInvalidate();
	return R_OK;
}

RRESULT OnRestore(void *pParam)
{
	g_pBsp->OnRestore();
	return R_OK;
}

long FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
    // Handle messages
    switch (message)
    {
		case WM_DROPFILES : {
			
			HDROP hDrop = (HDROP)wParam;

			char szFileName[256];
			DragQueryFile(hDrop,0,szFileName,sizeof(szFileName));
			OpenFile(szFileName);
		}break;
        case WM_KEYDOWN:
        {
            switch (wParam)
            {
				case VK_HOME: 
					RCameraPosition=rvector(0,0,50);
					break;
				case VK_F1:g_bShowCollision=!g_bShowCollision;break;
				case VK_F2:g_bCollision=!g_bCollision;break;
				case VK_F3:g_bCylinder=!g_bCylinder;break;
				case VK_F4:g_bShowSolid=!g_bShowSolid;break;
				case VK_F5:g_bShowMap=!g_bShowMap;break;
				case VK_F6:g_bShowCurrentNodePolygon=!g_bShowCurrentNodePolygon;break;
				case VK_F7:g_bDrawSolidPolygon=!g_bDrawSolidPolygon;break;
				case VK_F8:RSolidBspNode::m_bTracePath = !RSolidBspNode::m_bTracePath;break;
				case VK_F9:g_bWireFrame = !g_bWireFrame;break;
				case 'P' : g_bShowPickPoly = !g_bShowPickPoly;break;

				case 'T':
					{
						rplane impactplane;
						rvector origin = rvector(-1114.999878, -2349.183594, 238.283569);
						*(DWORD*)&origin.x = 0xc48b5fff;
						*(DWORD*)&origin.y = 0xc513cb05;
						*(DWORD*)&origin.z = 0x43b3244c;

						rvector diff = rvector(-6.620697 ,-65.601311, 0.000000);

						*(DWORD*)&diff.x = 0xc1b89382;
						*(DWORD*)&diff.y = 0xc280c22d;
						*(DWORD*)&diff.z = 0x00000000;

						rvector targetpos = origin + diff;
						g_pBsp->CheckWall(origin,targetpos,35.f,60,RCW_CYLINDER,0,&impactplane);

						diff = targetpos - origin;

					}break;
				case 'N':
					{
						g_bShowNavigation = !g_bShowNavigation;

					} break;
                case VK_ESCAPE:	
					PostMessage(::g_hWnd,WM_CLOSE,0,0);
					break;
			}
        }
        break;

		case WM_SYSCHAR:
			if(wParam==VK_RETURN)
			{
				mparams.bFullScreen=!mparams.bFullScreen;
				OnInvalidate(NULL);
				RResetDevice(&mparams);
				OnRestore(NULL);
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
	strcpy(mapname,cmdline);
	if(!FirstInstance(this_inst)) return 1;
	if(!InitScene(::g_hWnd)) return 1;

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
		else
		{
			RenderScene();
			Sleep(0);
		}
    }
	CloseScene();
  return (INT)msg.wParam;
}