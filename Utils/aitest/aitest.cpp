#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>

#include <windows.h>
#include <mmsystem.h>

#include "MDebug.h"
#include "RealSpace2.h"
#include "RBspObject.h"
#include "MZFileSystem.h"
#include "RFont.h"
#include "RNavigationNode.h"
#include "RNavigationMesh.h"

RMODEPARAMS mparams={ 1024,768,false,D3DFMT_R5G6B5 };

LPDIRECT3DDEVICE9 g_dev = NULL;

RBspObject *g_pBsp=NULL;
RNavigationMesh* g_pNMesh = g_pBsp->GetNavigationMesh();

char mapname[256];

bool g_bShowCollision=false;
bool g_bCollision=false;
bool g_bCylinder=false;
bool g_bShowSolid=true;
bool g_bShowMap=false;
bool g_bShowCurrentNodePolygon=false;
bool g_bDrawSolidPolygon=false;
bool g_bWireFrame=true;
bool g_bShowNavigation=false;
bool g_bShowPath = true;

bool g_bLineOfSight = false;

rvector g_CameraFrom;
rvector g_CameraAt;

rvector g_cylinderpos;

//rvector g_StartPos = rvector(-1016.108,-1432.061,0);
//rvector g_StartPos = rvector(0,0,0);
//rvector g_GoalPos = rvector(100,0,0);

rvector g_StartPos = rvector(-1023.7f, 1336.2f, 220.0f);
rvector g_GoalPos = rvector(-911,890,0);

float	g_fRadius = 70.0f;

#define CYLINDER_RADIUS 35


class BFont
{
private:
	ID3DXFont*		m_pDXFont;
public:
	BFont(int nHeight, int nWidth, const char* szFaceName)
	{
		m_pDXFont = NULL;
		int nWeight = FW_NORMAL;
		BOOL italic = FALSE;
		D3DXCreateFont( RGetDevice(), nHeight, nWidth, nWeight, 1, italic, DEFAULT_CHARSET, 
							OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
							LPCSTR(szFaceName), &m_pDXFont );
	}
	virtual ~BFont()
	{
		m_pDXFont->Release();
	}
	void DrawText(const char* szText, int x, int y, unsigned long int nColor)
	{
		RECT rc;
		SetRect(&rc, x,y,0,0);        
		m_pDXFont->DrawText( NULL, szText, -1, &rc, DT_NOCLIP, nColor);
	}
};
BFont* g_pFont=NULL;

void SearchPath();

void InitPath()
{
	if (g_pBsp == NULL) return;
	SearchPath();

}


RNavigationNode* g_pStartNode = NULL;
RNavigationNode* g_pGoalNode = NULL;

void SearchPath()
{
	if (g_pBsp == NULL) return;
	g_pNMesh->BuildNavigationPath(g_StartPos, g_GoalPos);


	g_pStartNode = g_pNMesh->FindClosestNode(g_StartPos);
	g_pGoalNode = g_pNMesh->FindClosestNode(g_GoalPos);

	g_bLineOfSight = g_pNMesh->LineOfSightTest(g_pStartNode, g_StartPos, g_pGoalNode, g_GoalPos);
}



void RenderPath()
{
	if (g_pBsp==NULL) return;

	g_pBsp->DrawNavi_Polygon();

	rvector v1 = g_StartPos, v2 = g_StartPos;

	int nCnt = 0;
	for (list<rvector>::iterator itor = g_pNMesh->GetWaypointList().begin(); 
		itor != g_pNMesh->GetWaypointList().end(); ++itor)
	{
		list<rvector>::iterator itorNext = itor;
		itorNext++;
		if (itorNext == g_pNMesh->GetWaypointList().end()) break;

		rvector v1 = (*itor);
		rvector v2 = (*itorNext);

		if (nCnt == 0)
		{
//			RDrawLine(g_StartPos, v1, 0xFF00FF00);
		}

		RDrawLine(v1, v2, 0xFF00FF00);

		nCnt++;
	}

	RDrawCircle(g_StartPos, g_fRadius, 50);
	RDrawCircle(g_GoalPos, g_fRadius, 50);
}

void DrawTri(rvector& v1, rvector& v2, rvector& v3, unsigned long color)
{
	RDrawLine(v1, v2, color);
	RDrawLine(v2, v3, color);
	RDrawLine(v3, v1, color);
}



void DrawDebugInfo()
{
	char text[256];
	int y = 10;

	sprintf(text, "Start: %.3f %.3f %.3f", g_StartPos.x, g_StartPos.y, g_StartPos.z);
	g_pFont->DrawText(text, 10, y, 0xFFFFFFFF);	y+=12;

	sprintf(text, "Goal: %.3f %.3f %.3f", g_GoalPos.x, g_GoalPos.y, g_GoalPos.z);
	g_pFont->DrawText(text, 10, y, 0xFFFFFFFF);	y+=12;


	if (g_pStartNode)
	{
		rvector v1, v2, v3;
		v1 = g_pStartNode->Vertex(0);
		v2 = g_pStartNode->Vertex(1);
		v3 = g_pStartNode->Vertex(2);
		DrawTri(v1, v2, v3, 0xFF00FFBB);

		sprintf(text, "StartNode: (%.3f %.3f %.3f) (%.3f %.3f %.3f) (%.3f %.3f %.3f)", 
			v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v3.x, v3.y, v3.z);
		g_pFont->DrawText(text, 10, y, 0xFFFFFFFF);	y+=12;
	}
	if (g_pGoalNode)
	{
		rvector v1, v2, v3;
		v1 = g_pGoalNode->Vertex(0);
		v2 = g_pGoalNode->Vertex(1);
		v3 = g_pGoalNode->Vertex(2);
		DrawTri(v1, v2, v3, 0xFF00FF00);

		sprintf(text, "GoalNode: (%.3f %.3f %.3f) (%.3f %.3f %.3f) (%.3f %.3f %.3f)", 
			v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v3.x, v3.y, v3.z);
		g_pFont->DrawText(text, 10, y, 0xFFFFFFFF);	y+=12;

	}

	RDrawLine(rvector(706,-1290,220), rvector(712.417, -1287.500, 220), 0xFFFFFF00);


	if (g_bLineOfSight)
	{
		sprintf(text, "시야 보임");
		g_pFont->DrawText(text, 10, y, 0xFFFFFFFF);	y+=12;
	}
	else
	{
		sprintf(text, "시야 안보임");
		g_pFont->DrawText(text, 10, y, 0xFFFFFFFF);	y+=12;
	}
}

void DrawDebugNode()
{
	unsigned long int nColor = 0xFFFF0000;
	list<RAStarNode*>* pPath = &g_pNMesh->m_AStar.m_ShortestPath;
	for (list<RAStarNode*>::iterator itor = pPath->begin(); itor != pPath->end(); itor++)
	{
		RNavigationNode* pTestNode = (RNavigationNode*)(*itor);
		
		rvector v1, v2, v3;
		v1 = pTestNode->Vertex(0);
		v2 = pTestNode->Vertex(1);
		v3 = pTestNode->Vertex(2);
		DrawTri(v1, v2, v3, nColor);

		if (nColor != 0xFF000000) nColor -= 0x050000;
		
	}

}

void OpenFile(const char *szFileName) 
{
	SAFE_DELETE(g_pBsp);
	g_pBsp = new RBspObject;
	if(!g_pBsp->Open(szFileName))
		SAFE_DELETE(g_pBsp);

	g_pNMesh = g_pBsp->GetNavigationMesh();

	InitPath();
}

BOOL InitScene(HWND hWnd)
{
	InitLog(MLOGSTYLE_DEBUGSTRING|MLOGSTYLE_FILE);
	
	RInitDisplay(hWnd,&mparams);
	
	RSetCamera(rvector(0.f,0.f,50.f),rvector(0.f,0.f,0.f),rvector(0.f,0.f,1.f));
//	RSetProjection(1.5,5,10000);

	rmatrix matProj;
	D3DXMatrixOrthoLH(&matProj, mparams.nWidth * 4, mparams.nHeight * 4, -1, 10000);
	RGetDevice()->SetTransform( D3DTS_PROJECTION, &matProj );


	g_cylinderpos=rvector(-155.17810 , 170.00000 , 190.00000);

	g_dev = RGetDevice();
	if (!g_pFont) g_pFont = new BFont(12, 10, "돋움체");

	OpenFile(mapname);

	g_pBsp->test_MakePortals();

	return TRUE;
}

BOOL CloseScene()
{
	if (g_pFont) SAFE_DELETE(g_pFont);
	RCloseDisplay();
	return TRUE;
}

#define IsKeyDown(key) ((GetAsyncKeyState(key) & 0x8000)!=0)
#pragma comment(lib,"winmm.lib")

void Update()
{
	if(!g_pBsp) return;

	static DWORD thistime,lasttime=timeGetTime(),elapsed;

	thistime = timeGetTime();
	elapsed = (thistime - lasttime)*(IsKeyDown(VK_SHIFT)?5:1);
	lasttime = thistime;

	if(GetActiveWindow()!=g_hWnd) return;

	static float rotatez=0.f,rotatex=pi/2.f;

	float fRotateStep=elapsed*0.001f;
	float fMoveStep=elapsed*0.5f;

	float fMoveDiff = 3.0f;

	if(IsKeyDown(VK_CONTROL)) fMoveDiff *= 10.0f;

	if(IsKeyDown(VK_LEFT)) 
	{
		g_GoalPos.y -= fMoveDiff;
		SearchPath();
	}
	if(IsKeyDown(VK_RIGHT))
	{
		g_GoalPos.y += fMoveDiff;
		SearchPath();
	}
	if(IsKeyDown(VK_UP))
	{
		g_GoalPos.x += fMoveDiff;
		SearchPath();
	}
	if(IsKeyDown(VK_DOWN))
	{
		g_GoalPos.x -= fMoveDiff;
		SearchPath();
	}
	if(IsKeyDown('A')) 
	{
		g_StartPos.y -= fMoveDiff;
		SearchPath();
	}
	if(IsKeyDown('D'))
	{
		g_StartPos.y += fMoveDiff;
		SearchPath();
	}
	if(IsKeyDown('W'))
	{
		g_StartPos.x += fMoveDiff;
		SearchPath();
	}
	if(IsKeyDown('S'))
	{
		g_StartPos.x -= fMoveDiff;
		SearchPath();
	}


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

	rotatex=max(0.01f,min(pi-0.01f,rotatex));

	rvector posdiff=rvector(0,0,0),dir=rvector(cosf(rotatez)*sinf(rotatex),sinf(rotatez)*sinf(rotatex),cosf(rotatex));
	D3DXVec3Normalize(&dir,&dir);
	rvector right;
	D3DXVec3Cross(&right,&dir,&rvector(0,0,1));
	D3DXVec3Normalize(&right,&right);

	if(IsKeyDown(VK_SPACE))
	{
		SearchPath();
	}


	rvector targetpos=g_cylinderpos+posdiff;
	
	g_cylinderpos=targetpos;
	
	g_CameraFrom = g_cylinderpos-dir*200.f;
	g_CameraAt = g_cylinderpos;

	g_CameraFrom = rvector(0, 0, 2400);
	g_CameraAt = rvector(0, 0, 0);

	RSetCamera(g_CameraFrom,g_CameraAt,rvector(1,0,0));

}

BOOL RenderScene()
{
	LPDIRECT3DDEVICE9 pd3dDevice=RGetDevice();

	rmatrix id;
	D3DXMatrixIdentity(&id);
	RGetDevice()->SetTransform( D3DTS_WORLD , &id);

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

	if(g_bShowCurrentNodePolygon)
	{
		g_pBsp->DrawColNodePolygon(g_cylinderpos);
	}

	if (g_bShowPath) RenderPath();

	//mlog("from: %.5f %.5f %.5f\n", g_CameraFrom.x, g_CameraFrom.y, g_CameraFrom.z);
	//mlog("at  : %.5f %.5f %.5f\n", g_CameraAt.x, g_CameraAt.y, g_CameraAt.z);

	DrawDebugInfo();
	//DrawDebugNode();
	

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

	::g_hWnd = CreateWindowEx( WS_EX_ACCEPTFILES,"RealSpace2", "collision",
            WS_VISIBLE | WS_POPUP | WS_CAPTION, CW_USEDEFAULT, CW_USEDEFAULT, 
			mparams.nWidth, mparams.nHeight, NULL, NULL, this_inst , NULL );
	ShowWindow(::g_hWnd,SW_SHOW);
	ShowCursor(FALSE);

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
				case VK_F1:
					g_bShowPath = !g_bShowPath;
					break;
				case VK_F2:
					break;
				case VK_F3:

					break;
				case VK_F4:
					g_bShowSolid=!g_bShowSolid;
					break;
				case VK_F5:
					g_bShowMap=!g_bShowMap;
					break;
				case VK_F6:
					g_bShowCurrentNodePolygon=!g_bShowCurrentNodePolygon;
					break;
				case VK_F7:
					break;
				case VK_F8:
					RSolidBspNode::m_bTracePath = !RSolidBspNode::m_bTracePath;
					break;
				case VK_F9:
					g_bWireFrame = !g_bWireFrame;
					break;

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