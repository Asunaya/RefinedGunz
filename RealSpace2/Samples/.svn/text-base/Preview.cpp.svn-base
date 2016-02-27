#include "MClient.h"
#include <windows.h>
#include <mmsystem.h>

#include "MUtil.h"

#include "MDebug.h"
#include "RealSpace2.h"

#include "RMtrl.h"

#include "RMeshMgr.h"
#include "RAnimationMgr.h"

#include "RBspObject.h"
#include "RMaterialList.h"

#include "MAStar.h"

#include "MFreeMapModel.h"

#include "RPathFinder.h"

#include "fileinfo.h"

#include "Mint4R2.h"
#include "Mint.h"
#include "MConsoleFrame.h"
#include "ConsoleCmds.h"

#pragma comment(lib,"winmm.lib")

_USING_NAMESPACE_REALSPACE2

RMODEPARAMS mparams={ 640,480,false,RPIXELFORMAT_565 };
char g_filename[_MAX_PATH]="test.rbs";

LPDIRECT3DDEVICE8 g_dev = NULL;

#define LOADFILE1		"c1_a2.elu"
#define LOADFILEANI1	"c1_a1.elu.ani"
#define LOADFILEANI2	"c1_a2.elu.ani"

RMeshList		g_mesh_list;
int				g_id;
RAnimationList	g_ani_list;

RMaterialList g_ml;
RBspObject g_bsp;

bool g_bShowPathNode=false;
bool g_bShowPath=false;
bool g_bShowCurrentNode=false;

HWND g_hWnd;
BOOL g_bActive;

RPathFinder	g_PathFinder;
//RVECTORLIST g_PathList;

MClient			g_Client;
MConsoleFrame*	g_pConsole = NULL;

struct SmoothCamera {
	SmoothCamera() : fAngleX(8*pi/10.f) , fAngleZ(0.f) { }

	rvector m_Target;
	rvector m_CurrentPosition;
	rvector m_CurrentDirection;

	void Update(float fTime);
	void SetTarget(rvector &target) { m_Target=target; }

	float fAngleX,fAngleZ;
};

struct Character {
	Character() : m_Position(0,0,0),m_Target(0,0,0),m_bMoving(false) { }

	rvector m_Position;
	rvector m_Target;

	bool m_bMoving;

	RVECTORLIST::iterator m_PathProgress;
	RVECTORLIST m_PathList;

	void Update(float fTime);
};

Character g_Character;
SmoothCamera g_Camera;



void SmoothCamera::Update(float FTime)
{
	rvector CharacterPos=g_Character.m_Position;
	rvector diff=CharacterPos-m_Target;
	float fMagnitude=Magnitude(diff);
	Normalize(diff);
	//m_Target+=.01f*fMagnitude*diff;
	m_Target+=.1f*fMagnitude*diff;

#define CAMERA_DISTANCE	800.f

	rvector up(0,0,1);
	rvector dir=rvector(cosf(fAngleZ)*sinf(fAngleX),sinf(fAngleZ)*sinf(fAngleX),cosf(fAngleX));
	D3DXVec3Normalize(&dir,&dir);
	rvector right;
	D3DXVec3Cross(&right,&dir,&up);
	D3DXVec3Normalize(&right,&right);
	m_CurrentPosition=m_Target-dir*CAMERA_DISTANCE;

	RSetCamera(m_CurrentPosition,m_Target,up);
}

void Character::Update(float fTime)
{
	if(m_bMoving)
	{
		rvector Diff=m_Target-m_Position;
		if(Magnitude(Diff)<5.f)
		{
			m_PathProgress++;
			if(m_PathProgress!=m_PathList.end())
			{
				m_Target=**m_PathProgress;
			}else
				m_bMoving=false;

		}else
		{
			Normalize(Diff);
			m_Position+=5.5f*Diff;
		}
	}
}

void InitClient()
{
	// Console 생성
	MFontR2* pDefFont = new MFontR2;
	pDefFont->Create("Default", g_dev, "Tahoma", 14);

	MLoadDesignerMode();
	MInitialize(640, 480, pDefFont);

	g_pConsole = new MConsoleFrame("Console", MGetMainFrame(), MGetMainFrame());
	g_pConsole->SetByResourceMap();
	g_pConsole->Show(false);

	g_Client.Create();

	// 소켓 이벤트 연결
	g_Client.GetClientSocket()->SetSocketErrorCallback(SocketErrorEvent);
	g_Client.GetClientSocket()->SetConnectCallback(SocketConnectEvent);
	g_Client.GetClientSocket()->SetDisconnectCallback(SocketDisconnectEvent);
	g_Client.GetClientSocket()->SetRecvCallback(SocketRecvEvent);

	InitConsoleCmds();
}


RRESULT InitScene(void *pParam)
{

	RSetCamera(rvector(-10.f,0.f,150.f),rvector(0.f,0.f,0.f),rvector(0.f,1.f,0.f));
	RSetProjection(1.5,5,10000);

	g_dev = RGetDevice();

	g_id = g_mesh_list.Add(LOADFILE1);

	if(g_id==-1) return R_ERROR_LOADING;

//	g_mesh_list.GetFast(g_id)->ReadEluAni("c5_a1.elu.ani");

	g_ani_list.Add("idle" ,LOADFILEANI1,1);
	g_ani_list.Add("run"  ,LOADFILEANI2,2);

	RAnimation* pAni = g_ani_list.GetAnimation("idle");

	g_mesh_list.GetFast(g_id)->SetAnimation(pAni);

	g_mesh_list.GetFast(g_id)->m_mtrl_list_ex.Restore(g_dev);//tex 생성 임시 mtrl : 통합관리시 제거

	if(g_id == -1) return R_ERROR_LOADING;

	/*
	RMesh* mesh=g_mesh_list.GetFast(g_id);

	g_mesh_list.GetFast(g_id)->m_mtrl_list_ex.Restore(g_dev);//tex 생성 임시 mtrl : 통합관리시 제거
//	g_mesh_list.GetFast(g_id)->ReadEluAni("1111.elu.ani");
*/

	if(!strlen(g_filename))
		return R_ERROR_LOADING;


	char rm2name[_MAX_PATH];
	ReplaceExtension(rm2name,g_filename,"rm2");

	if(!g_ml.Open(rm2name))
	{
		MLog("error while loading %s \n",rm2name);
		return R_ERROR_LOADING;
	}

	if(!g_bsp.Open(g_filename,&g_ml))
	{
		MLog("error while loading %s \n",g_filename);
		return R_ERROR_LOADING;
	}

	g_bsp.OpenLightmap();
	g_bsp.CreateVertexBuffer();

	g_PathFinder.Create(&g_bsp);

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


	InitClient();

	return R_OK;
}

RRESULT CloseScene(void *pParam)
{
	g_PathFinder.Destroy();

	g_Client.Destroy();
	delete g_pConsole; g_pConsole = NULL;

	MFinalize();

//	RCloseDisplay();
	
	return R_OK;
}

#define IsKeyDown(key) ((GetAsyncKeyState(key) & 0x8000)!=0)

void Update()
{
	static DWORD thistime,lasttime=timeGetTime(),elapsed;

	thistime = timeGetTime();
	elapsed = (thistime - lasttime)*(IsKeyDown(VK_SHIFT)?5:1);
	lasttime = thistime;

	g_Character.Update(0.f);
	g_Camera.Update(0.f);

	static float rotatez=0.f,rotatex=9*pi/10.f;

	float fRotateStep=elapsed*0.001f;
	float fMoveStep=elapsed*1.1f;

	if(g_bActive)
	{
		if(IsKeyDown(VK_LEFT))	g_Camera.fAngleZ-=fRotateStep;
		if(IsKeyDown(VK_RIGHT)) g_Camera.fAngleZ+=fRotateStep;
		if(IsKeyDown(VK_UP))	g_Camera.fAngleX-=fRotateStep;
		if(IsKeyDown(VK_DOWN))	g_Camera.fAngleX+=fRotateStep;
	}
}

RRESULT RenderScene(void *pParam)
{
	Update();

	RRESULT isOK=RIsReadyToRender();

	if(isOK==R_NOTREADY)
		return R_NOTREADY;
	else if(isOK==R_RESTORED) {

	} // restore device dependent objects

	RAnimation* pAni;

	if(g_Character.m_bMoving)
		pAni = g_ani_list.GetAnimation("run");
	else 
		pAni = g_ani_list.GetAnimation("idle");

	g_mesh_list.GetFast(g_id)->SetAnimation(pAni);

	g_mesh_list.Frame();

#define SCALE	100.f

	D3DXMATRIX world;

	static rvector dir = rvector(0.f,1.f,0.f);

	static rvector dir_backup;

	dir_backup = dir;

	if(g_Character.m_bMoving) {
		dir = g_Character.m_Target - g_Character.m_Position;
		dir.z = 0.f;
	}

	if(dir.x==0.f&&dir.y==0.f&&dir.z==0.f)
		dir = dir_backup;

	MakeWorldMatrix(&world,g_Character.m_Position,dir,rvector(0,0,1));


	g_mesh_list.RenderFast(0,&world);

	RGetDevice()->SetRenderState(D3DRS_CULLMODE  ,D3DCULL_CW);

	g_bsp.Draw();

	RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
	RGetDevice()->SetTexture(0,NULL);
	RGetDevice()->SetTexture(1,NULL);
	RGetDevice()->SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE );

	if(g_bShowPathNode)
		g_bsp.DrawPathNode();

	{

		struct LVERTEX {
			float x, y, z;		// world position
			DWORD color;
		} ;

	RGetDevice()->SetTexture(0,NULL);
    RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );

#define AXIS_SCALE	100
		LVERTEX lines[6]={{0,0,0,0xff0000},{AXIS_SCALE,0,0,0xff0000},
						{0,0,0,0xff00},{0,AXIS_SCALE,0,0xff00},
						{0,0,0,0xff},{0,0,AXIS_SCALE,0xff}};

		RGetDevice()->DrawPrimitiveUP(D3DPT_LINELIST,3,lines,sizeof(LVERTEX));
	}


	if(g_bShowCurrentNode)
	{
		POINT p;
		GetCursorPos(&p);

		RSBspNode *pNode;
		int nIndex;
		rvector PickPos;

		if(g_bsp.Pick(p.x,p.y,&pNode,&nIndex,&PickPos)) {

			pNode->DrawWireFrame(nIndex,0xffffff);
//			HRESULT hr=RGetDevice()->DrawPrimitiveUP(D3DPT_LINESTRIP,3,pNode->pVertices+nIndex*3,sizeof(BSPVERTEX));
//			mlog("!\n");

			
			RPathNode *pPathNode=g_bsp.GetPathNode(pNode,nIndex);

			if(pPathNode) {
				if(pPathNode->m_Neighborhoods.size()) {
					for(RPATHLIST::iterator i=pPathNode->m_Neighborhoods.begin();i!=pPathNode->m_Neighborhoods.end();i++) {
						(*g_bsp.GetPathList())[(*i)->nIndex]->DrawWireFrame(0xff00);
					}
				}
				pPathNode->DrawWireFrame(0xff0000);
			}
		}
	}


	if(g_bShowPath)
	{

		RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
		//RGetDevice()->SetRenderState(D3DRS_ZENABLE, FALSE);

		if(g_PathFinder.GetStartNode()!=NULL){
			g_PathFinder.GetStartNode()->DrawWireFrame(0xFFFF00);
		}
		if(g_PathFinder.GetEndNode()!=NULL){
			g_PathFinder.GetEndNode()->DrawWireFrame(0x00FFFF);
		}

		MAStar* pPolygonPathFinder = g_PathFinder.GetPolygonPathFinder();
		for(int i=0; i<pPolygonPathFinder->GetShortestPathCount(); i++){
			MRPathNode* pMRPathNode = (MRPathNode*)pPolygonPathFinder->GetShortestPath(i);
			pMRPathNode->GetRPathNode()->DrawWireFrame(0xFF4444);
		}

		struct VERTEX3DFORLINE { D3DXVECTOR3 p; DWORD color; };
		#define D3DFVF_VERTEX3DFORLINE (D3DFVF_XYZ|D3DFVF_DIFFUSE)

		// Render Outline
		MPolygonMapModel* pPolygonMapModel = g_PathFinder.GetPolygonMapModel();
		for(int ip=0; ip<pPolygonMapModel->m_PolygonList.GetCount(); ip++){
			MPMPolygon* pPolygon = pPolygonMapModel->m_PolygonList.Get(ip);
			int nPointCount = pPolygon->m_PointList.GetCount();
			if(pPolygon->m_bEnclosed==true) nPointCount++;
			VERTEX3DFORLINE* pl = new VERTEX3DFORLINE[nPointCount];
			for(int ipp=0; ipp<nPointCount; ipp++){
				MPMPoint* pPoint = pPolygon->m_PointList.Get(ipp%(pPolygon->m_PointList.GetCount()));
				pl[ipp].p.x = pPoint->GetX();
				pl[ipp].p.y = pPoint->GetY();
				pl[ipp].p.z = pPoint->GetZ()*1.02;
				//pl[ipp].color = 0xFF00FFFF;
				pl[ipp].color = 0xFF000000;
			}
			RGetDevice()->SetVertexShader(D3DFVF_VERTEX3DFORLINE);
			RGetDevice()->DrawPrimitiveUP(D3DPT_LINESTRIP, nPointCount-1, pl, sizeof(VERTEX3DFORLINE));
			delete[] pl;
		}

		RGetDevice()->SetRenderState(D3DRS_ZENABLE, FALSE);

		// Render Path
		if(g_Character.m_PathList.size()>0){
			VERTEX3DFORLINE* pl = new VERTEX3DFORLINE[g_Character.m_PathList.size()];
			i=0;
			for(RVECTORLIST::iterator it=g_Character.m_PathList.begin(); it!=g_Character.m_PathList.end(); it++){
				pl[i].p.x = (*it)->x;
				pl[i].p.y = (*it)->y;
				pl[i].p.z = (*it)->z*1.01;
				pl[i].color = 0xFF0000FF;
				i++;
			}
			RGetDevice()->SetVertexShader(D3DFVF_VERTEX3DFORLINE);
			RGetDevice()->DrawPrimitiveUP(D3DPT_LINESTRIP, (int)g_Character.m_PathList.size()-1, pl, sizeof(VERTEX3DFORLINE));
			//RGetDevice()->DrawPrimitiveUP(D3DPT_POINTLIST, g_PathList.size(), pl, sizeof(VERTEX3DFORLINE));
			delete[] pl;
		}

		RGetDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);
	}

	// Draw Console
	MDrawContextR2 dc(g_dev);
	MDraw(&dc);

	//RFlip();
	return R_OK;
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
    wc.lpszClassName = "RealSpace2";
	if(!RegisterClass(&wc)) return FALSE;

    g_hWnd = CreateWindow( "RealSpace2", "rs2test",
            WS_VISIBLE | WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, 
			640, 480, NULL, NULL, this_inst , NULL );
	ShowWindow(g_hWnd,SW_SHOW);
	ShowCursor(TRUE);

    if (!g_hWnd) return FALSE;

    return TRUE;
}

bool g_bStart = false;
//int g_nStartX, g_nEndX;

long FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Console Event
	MProcessEvent(hWnd, message, wParam, lParam);

	
    // Handle messages
    switch (message)
    {
		case WM_RBUTTONDOWN:
		{
			DWORD fwKeys = wParam;        // key flags 
			int xPos = LOWORD(lParam);  // horizontal position of cursor 
			int yPos = HIWORD(lParam);  // vertical position of cursor 

			g_PathFinder.SetStartPos(g_Character.m_Position+rvector(0,0,50));
			if(g_PathFinder.SetEndPos(xPos, yPos))
			{
				g_PathFinder.Enlarge(100);
				g_PathFinder.FindPath(&g_Character.m_PathList);
				if(g_Character.m_PathList.size())
				{
					g_Character.m_bMoving=true;
					g_Character.m_PathProgress=g_Character.m_PathList.begin();
					g_Character.m_Target=**g_Character.m_PathProgress;
				}
			}
			break;			
		}
        case WM_KEYDOWN:
        {
            switch (wParam)
            {
				case VK_F1: g_bShowPathNode=!g_bShowPathNode; break;
				case VK_F2: g_bShowPath=!g_bShowPath; break;
				case VK_F3: g_bShowCurrentNode=!g_bShowCurrentNode; break;

				case VK_F5: g_pConsole->Show(!g_pConsole->IsVisible());
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
	InitLog(MLOGSTYLE_DEBUGSTRING|MLOGSTYLE_FILE);

	RSetFunction(RF_CREATE	,InitScene);
	RSetFunction(RF_RENDER	,RenderScene);
	RSetFunction(RF_DESTROY ,CloseScene);

	return RMain("Preview",this_inst,prev_inst,cmdline,cmdshow,WndProc);
}
