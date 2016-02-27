#include <windows.h>
#include <mmsystem.h>

#include "MUtil.h"

#include "MDebug.h"
#include "RealSpace2.h"

#include "RMtrl.h"

#include "RMesh.h"
#include "RMeshList.h"

#include "RBspObject.h"
#include "RMaterialList.h"

#include "MAStar.h"

#include "MFreeMapModel.h"

#include "RPathFinder.h"

#include "fileinfo.h"

_USING_NAMESPACE_REALSPACE2

char g_filename[_MAX_PATH]="test.rbs";

LPDIRECT3DDEVICE8 g_dev = NULL;

RMeshList	g_mesh_list;
int			g_id;

RMaterialList g_ml;
RBspObject g_bsp;

bool g_bShowPath=false;

RPathFinder	g_PathFinder;
RVECTORLIST g_PathList;

RRESULT InitScene(void *pParam)
{
	RSetCamera(rvector(-10.f,0.f,150.f),rvector(0.f,0.f,0.f),rvector(0.f,1.f,0.f));
	RSetProjection(1.5,5,10000);

	g_dev = RGetDevice();

	/*
	g_id = g_mesh_list.Add("teapot.elu");

	if(g_id == -1) return false;
	*/

	/*
	RMesh* mesh=g_mesh_list.GetFast(g_id);

	g_mesh_list.GetFast(g_id)->m_mtrl_list_ex.Restore(g_dev);//tex 생성 임시 mtrl : 통합관리시 제거
//	g_mesh_list.GetFast(g_id)->ReadEluAni("1111.elu.ani");
*/

	g_bsp.Open(g_filename);

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
	
	return R_OK;
}

RRESULT CloseScene(void *pParam)
{
	g_PathFinder.Destroy();

//	RCloseDisplay();
	return R_OK;
}

#define IsKeyDown(key) ((GetAsyncKeyState(key) & 0x8000)!=0)
#pragma comment(lib,"winmm.lib")

void Update()
{
	static DWORD thistime,lasttime=timeGetTime(),elapsed;

	thistime = timeGetTime();
	elapsed = (thistime - lasttime)*(IsKeyDown(VK_SHIFT)?5:1);
	lasttime = thistime;

	static float rotatez=0.f,rotatex=9*pi/10.f;

	float fRotateStep=elapsed*0.001f;
	float fMoveStep=elapsed*1.1f;

	if(RIsActive())
	{
		if(IsKeyDown(VK_LEFT)) rotatez-=fRotateStep;
		if(IsKeyDown(VK_RIGHT)) rotatez+=fRotateStep;
		if(IsKeyDown(VK_UP)) rotatex-=fRotateStep;
		if(IsKeyDown(VK_DOWN)) rotatex+=fRotateStep;
	}

	rvector pos=RCameraPosition,dir=rvector(cosf(rotatez)*sinf(rotatex),sinf(rotatez)*sinf(rotatex),cosf(rotatex));
	D3DXVec3Normalize(&dir,&dir);
	rvector right;
	D3DXVec3Cross(&right,&dir,&rvector(0,0,1));
	D3DXVec3Normalize(&right,&right);

	if(RIsActive())
	{
		if(IsKeyDown('W')) pos+=fMoveStep*dir;
		if(IsKeyDown('S')) pos-=fMoveStep*dir;
		if(IsKeyDown('A')) pos+=fMoveStep*right;
		if(IsKeyDown('D')) pos-=fMoveStep*right;
		if(IsKeyDown(VK_SPACE)) pos+=fMoveStep*rvector(0,0,1);
	}

//	gClipAngle +=2.1f;
//	gClipAngle=fmodf(gClipAngle,180);
	
	rvector at=pos+dir;
	//at.z=0;
	RSetCamera(pos,at,rvector(0,0,1));
//	mlog("%3.3f %3.3f\n",rotatex,rotatez);

	
}


RRESULT RenderScene(void *pParam)
{
	Update();

	RRESULT isOK=RIsReadyToRender();
	if(isOK==R_NOTREADY)
		return R_NOTREADY;
	else
	if(isOK==R_RESTORED)
	{
	} // restore device dependent objects

	g_mesh_list.Frame();
	g_mesh_list.Render();

	RGetDevice()->SetRenderState(D3DRS_CULLMODE  ,D3DCULL_CW);
	g_bsp.Draw();

	RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
	RGetDevice()->SetTexture(0,NULL);
	RGetDevice()->SetTexture(1,NULL);
	RGetDevice()->SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE );

	if(g_bShowPath)
		g_bsp.DrawPathNode();

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

		RGetDevice()->DrawPrimitiveUP(D3DPT_LINELIST,3,lines,sizeof(LVERTEX));
	}



	{
		POINT p;
		GetCursorPos(&p);

		RSBspNode *pNode;
		int nIndex;
		rvector PickPos;
		if(g_bsp.Pick(p.x,p.y,&pNode,&nIndex,&PickPos))
		{
			pNode->DrawWireFrame(nIndex,0xffffff);
//			HRESULT hr=RGetDevice()->DrawPrimitiveUP(D3DPT_LINESTRIP,3,pNode->pVertices+nIndex*3,sizeof(BSPVERTEX));
//			mlog("!\n");

			
			RPathNode *pPathNode=g_bsp.GetPathNode(pNode,nIndex);

			if(pPathNode)
			{
				if(pPathNode->m_Neighborhoods.size())
				{
					for(RPATHLIST::iterator i=pPathNode->m_Neighborhoods.begin();i!=pPathNode->m_Neighborhoods.end();i++)
					{
						(*g_bsp.GetPathList())[(*i)->nIndex]->DrawWireFrame(0xff00);
					}
				}
				pPathNode->DrawWireFrame(0xff0000);
			}
		}
	}



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
	if(g_PathList.size()>0){
		VERTEX3DFORLINE* pl = new VERTEX3DFORLINE[g_PathList.size()];
		i=0;
		for(RVECTORLIST::iterator it=g_PathList.begin(); it!=g_PathList.end(); it++){
			pl[i].p.x = (*it)->x;
			pl[i].p.y = (*it)->y;
			pl[i].p.z = (*it)->z*1.01;
			pl[i].color = 0xFF0000FF;
			i++;
		}
		RGetDevice()->SetVertexShader(D3DFVF_VERTEX3DFORLINE);
		RGetDevice()->DrawPrimitiveUP(D3DPT_LINESTRIP, g_PathList.size()-1, pl, sizeof(VERTEX3DFORLINE));
		//RGetDevice()->DrawPrimitiveUP(D3DPT_POINTLIST, g_PathList.size(), pl, sizeof(VERTEX3DFORLINE));
		delete[] pl;
	}

	/*
	// Render Portal
	for(int ip=0; ip<pPolygonMapModel->m_PortalList.GetCount(); ip++){
		MPMPortal* pPortal = pPolygonMapModel->m_PortalList.Get(ip);
		RGetDevice()->SetVertexShader(D3DFVF_VERTEX3DFORLINE);
		VERTEX3DFORLINE pl[2];
		pl[0].p.x = pPortal->m_pPos[0]->GetX();
		pl[0].p.y = pPortal->m_pPos[0]->GetY();
		pl[0].p.z = pPortal->m_pPos[0]->GetZ()*1.05f;
		pl[0].color = 0xFF00FF00;
		pl[1].p.x = pPortal->m_pPos[1]->GetX();
		pl[1].p.y = pPortal->m_pPos[1]->GetY();
		pl[1].p.z = pPortal->m_pPos[1]->GetZ()*1.05f;
		pl[1].color = 0xFF00FF00;
		RGetDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 1, pl, sizeof(VERTEX3DFORLINE));
	}
	*/

	/*
	// Visible
	for(int ip=0; ip<pPolygonMapModel->m_PointList.GetCount(); ip++){
		MPMPoint* pPoint = pPolygonMapModel->m_PointList.Get(ip);
		RGetDevice()->SetVertexShader(D3DFVF_VERTEX3DFORLINE);
		VERTEX3DFORLINE pl[2];
		pl[0].p.x = pPoint->GetX();
		pl[0].p.y = pPoint->GetY();
		pl[0].p.z = pPoint->GetZ()*1.07f;
		pl[0].color = 0xFF555555;
		for(int j=0; j<pPoint->GetVisiblePointCount(); j++){
			MPMPoint* pPoint2 = pPoint->GetVisiblePoint(j);
			pl[1].p.x = pPoint2->GetX();
			pl[1].p.y = pPoint2->GetY();
			pl[1].p.z = pPoint2->GetZ()*1.07f;
			pl[1].color = 0xFF555555;
			RGetDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 1, pl, sizeof(VERTEX3DFORLINE));
		}
	}
	*/

	RGetDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);

	return R_OK;
}

bool g_bStart = false;
//int g_nStartX, g_nEndX;

long FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
    // Handle messages
    switch (message)
    {
		case WM_LBUTTONDOWN:
		{
			DWORD fwKeys = wParam;        // key flags 
			int xPos = LOWORD(lParam);  // horizontal position of cursor 
			int yPos = HIWORD(lParam);  // vertical position of cursor 

			g_bStart = true;
			//g_nStartX = xPos;
			//g_nEndX = yPos;
			g_PathFinder.SetStartPos(xPos, yPos);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			DWORD fwKeys = wParam;        // key flags 
			int xPos = LOWORD(lParam);  // horizontal position of cursor 
			int yPos = HIWORD(lParam);  // vertical position of cursor 

			g_PathFinder.SetEndPos(xPos, yPos);
			if(g_bStart==true){
				//g_PathFinder.Enlarge(50);
				//g_PathFinder.FindPath(&g_PathList, g_nStartX, g_nEndX, xPos, yPos);
				g_PathFinder.FindPath(&g_PathList);
			}
			break;			
		}
        case WM_KEYDOWN:
        {
            switch (wParam)
            {
				case VK_F1:
					g_bShowPath=!g_bShowPath;
					break;
			}
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

	RMODEPARAMS g_ModeParams={ 1024,768,false,RPIXELFORMAT_565 };
	return RMain("rs2test",this_inst,prev_inst,cmdline,cmdshow, &g_ModeParams, WndProc, 0);
}
