// WorldEditView.cpp : implementation of the CWorldEditView class
//

#include "stdafx.h"
#include "WorldEdit.h"

#include "WorldEditDoc.h"
#include "WorldEditView.h"
#include "MainFrm.h"

#include "RealSpace2.h"
#include "RBspObject.h"
#include "MDebug.h"

#include "RMeshMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

_USING_NAMESPACE_REALSPACE2

/////////////////////////////////////////////////////////////////////////////
// CWorldEditView

IMPLEMENT_DYNCREATE(CWorldEditView, CView)

BEGIN_MESSAGE_MAP(CWorldEditView, CView)
	//{{AFX_MSG_MAP(CWorldEditView)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_RUNTIMEBOUNDINGBOX, OnRuntimeboundingbox)
	ON_COMMAND(ID_WIREFRAME, OnWireframe)
	ON_UPDATE_COMMAND_UI(ID_BOUNDINGBOX, OnUpdateBoundingbox)
	ON_COMMAND(ID_BOUNDINGBOX, OnBoundingbox)
	ON_UPDATE_COMMAND_UI(ID_WIREFRAME, OnUpdateWireframe)
	ON_COMMAND(ID_OCCLUSION, OnOcclusion)
	ON_UPDATE_COMMAND_UI(ID_OCCLUSION, OnUpdateOcclusion)
	ON_COMMAND(ID_SHOWLIGHTMAP, OnShowlightmap)
	ON_UPDATE_COMMAND_UI(ID_SHOWLIGHTMAP, OnUpdateShowlightmap)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorldEditView construction/destruction

CWorldEditView::CWorldEditView()
{
	// TODO: add construction code here

	m_bWireframe=false;
	m_bShowLightmap=false;
}

CWorldEditView::~CWorldEditView()
{
	RCloseDisplay();
}

BOOL CWorldEditView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

#define DEFAULTSIZE	100.f

/////////////////////////////////////////////////////////////////////////////
// CWorldEditView drawing


rvector		g_LastPickPos;

#include "ProgressDialog.h"
#include ".\worldeditview.h"

void CWorldEditView::OnDraw(CDC* pDC)
{
	CWorldEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here

	/*
	rvector targetpos=rvector(size.x*(float)pt.x/(float)SCROLLBARSIZE+pbb->vmin.x,
	size.y*(float)pt.y/(float)SCROLLBARSIZE+pbb->vmin.y,0);

	float AngleZ=m_CameraAngleZ*2*pi-pi/2;
	float AngleX=m_CameraAngleX*pi/2;
	rvector sourcepos=2*size.z*rvector(cosf(AngleZ)*sinf(AngleX),sinf(AngleZ)*sinf(AngleX),cosf(AngleX))+targetpos;
	*/

	rmatrix id;
	D3DXMatrixIdentity(&id);
	RGetDevice()->SetTransform( D3DTS_WORLD , &id);

	if(!g_bProgress && pDoc->m_pBspObject)
	{
		if(m_bWireframe) {
			RGetDevice()->SetRenderState( D3DRS_FILLMODE , D3DFILL_WIREFRAME );
			RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
		}
		else {
			RGetDevice()->SetRenderState( D3DRS_FILLMODE ,  D3DFILL_SOLID );
		}

		pDoc->m_pBspObject->SetWireframeMode(m_bWireframe);
		pDoc->m_pBspObject->SetShowLightmapMode(m_bShowLightmap);
		
		if(m_EditMode!=EDITMODE_PATH)
			pDoc->m_pBspObject->Draw();
		if(m_bDrawBoundingBox)
			pDoc->m_pBspObject->DrawBoundingBox();
		if(m_bDrawOcclusion)
			pDoc->m_pBspObject->DrawOcclusions();

		if(m_EditMode==EDITMODE_PATH)
		{
			RBspObject *pBsp=pDoc->m_pBspObject;
//			pBsp->DrawPathNode();

//			pBsp->DrawCollision();
			pBsp->Draw();

			RGetDevice()->SetTexture(0,NULL);
			RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
			RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );

			RBSPPICKINFO *ppi=&GetDocument()->m_LastPicked;
			if(GetDocument()->m_bLastPicked)
			{
				ppi->pNode->DrawWireFrame(ppi->nIndex,0xffffffff);
				ppi->pNode->DrawBoundingBox(0xffff0000);

				POINT p;
				GetCursorPos(&p);
				ScreenToClient(&p);

				rvector pos,dir,to;
				RGetScreenLine(p.x,p.y,&pos,&dir);
				to = pos+dir;

				rvector worldpos;
				D3DXPlaneIntersectLine(&worldpos,&ppi->pInfo->plane,&pos,&to);

				rvector normal;
				pBsp->GetNormal(ppi->pInfo->nConvexPolygon,worldpos,&normal);
				RDrawLine(worldpos,worldpos+normal*100,0xffff0000);


				/*
				RPathNode *pPathNode=pBsp->GetPathNode(bpi.pNode,bpi.nIndex);
				if(pPathNode)
				{
					if(pPathNode->m_Neighborhoods.size())
					{
						for(RPATHLIST::iterator i=pPathNode->m_Neighborhoods.begin();i!=pPathNode->m_Neighborhoods.end();i++)
						{
							(*pDoc->m_pBspObject->GetPathList())[(*i)->nIndex]->DrawWireFrame(0xff00);
						}
					}
					pPathNode->DrawWireFrame(0xff0000);
				}
				*/

				RGetDevice()->SetRenderState( D3DRS_ZENABLE , FALSE );

				int nS=ppi->pInfo->nConvexPolygon;
				pBsp->DrawNormal(nS,100);

			}
		}
		//*/

	}

	{

#define GRIDCOUNT	10
#define GRIDSPACE	(DEFAULTSIZE/GRIDCOUNT)

		RGetDevice()->SetTexture(0,NULL);
		RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
		RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );
		for(int i=0;i<GRIDCOUNT+1;i++)
		{
			RDrawLine(rvector(i*GRIDSPACE,0.f,0),rvector(i*GRIDSPACE,DEFAULTSIZE,0),0xffffff);
			RDrawLine(rvector(0.f,i*GRIDSPACE,0),rvector(DEFAULTSIZE,i*GRIDSPACE,0),0xffffff);
		}

	}

	RDrawLine(rvector(0,0,0),rvector(GRIDSPACE*.5f,0,0),0xff0000);
	RDrawLine(rvector(0,0,0),rvector(0,GRIDSPACE*.5f,0),0x00ff00);
	RDrawLine(rvector(0,0,0),rvector(0,0,GRIDSPACE*.5f),0x0000ff);

	/*
	{
		rmatrix mat;
		D3DXMatrixTranslation(&mat,g_LastPickPos.x,g_LastPickPos.y,g_LastPickPos.z);
		g_mesh_list.RenderFast(g_nID,&mat);
	}
	*/

	RFlip();

	Sleep(0);
}

/////////////////////////////////////////////////////////////////////////////
// CWorldEditView diagnostics

#ifdef _DEBUG
void CWorldEditView::AssertValid() const
{
	CView::AssertValid();
}

void CWorldEditView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWorldEditDoc* CWorldEditView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWorldEditDoc)));
	return (CWorldEditDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWorldEditView message handlers

BOOL CWorldEditView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	BOOL ret=CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

	ModifyStyleEx( 0, WS_EX_ACCEPTFILES, 0);

	if(ret)
	{
		RMODEPARAMS mparams={ 1024,768,false,D3DFMT_R5G6B5 };
		if(!RInitDisplay(m_hWnd,&mparams))
		{
			AfxMessageBox("Cannot Initialize 3D Engine.");
			return false;
		}
		RSetRenderFlags(RRENDER_CLEAR_BACKBUFFER);
	}

//	g_nID=g_mesh_list.Add("teapot.elu");

	return ret;
}

BOOL CWorldEditView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	
//	return CView::OnEraseBkgnd(pDC);
	return true;
}

void CWorldEditView::Resize(CSize size)
{
	RMODEPARAMS mparams={ size.cx,size.cy,false,D3DFMT_R5G6B5 };
	RBspObject *pbsp=GetDocument()->m_pBspObject;
	if(pbsp) pbsp->OnInvalidate();
	RResetDevice(&mparams);
	if(pbsp) pbsp->OnRestore();
}

void CWorldEditView::GetWorldCoordinate(rvector *ret,CPoint pt)
{
	*ret=RGetIntersection(pt.x,pt.y,rplane(0,0,1,0));

	/*
	rvector scrpoint=rvector((float)pt.x,(float)pt.y,0.1f);	
	rmatrix inv;	
	float det;
	D3DXMatrixInverse(&inv,&det,&RViewProjectionViewport);

	rvector worldpoint;
	D3DXVec3TransformCoord(&worldpoint,&scrpoint,&inv);

	rplane plane=rplane(0,0,1,0);
	D3DXPlaneIntersectLine(ret,&plane,&worldpoint,&RCameraPosition);
	*/
}

void CWorldEditView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	/*
	// test code
	{
		RBspObject *pbsp=GetDocument()->m_pBspObject;
		RPathNode *pNode;
		rvector ColPos;

		if(pbsp && pbsp->PickPathNode(point.x,point.y,&pNode,&ColPos))
		{
			_RPT1(_CRT_WARN,"PATHNODE GROUP ID : %d\n",pNode->m_nGroupID);
		}
	}
	*/

	CView::OnLButtonDown(nFlags, point);

	m_bLastShiftState=(GetKeyState(VK_SHIFT)&0x8000)!=0;
	m_bLastAltState=(GetKeyState(VK_MENU)&0x8000)!=0;
	m_LastMatrix=RViewProjectionViewport;
	m_LastCameraPosition=RCameraPosition;
	m_LastCursorPosition=point;

	if(m_bLastShiftState)	// shift 키가 눌려있으면 camera를 이동한다..
	{
		// 눌린지점의 world coordinate 를 구한다.
		GetWorldCoordinate(&m_LastWorldPosition,point);
	}

	if(m_bLastAltState)		// alt 를 누른상태에서는 화면 중앙을 중심으로 회전..
	{
		// 화면 중앙의 점을 기준으로 회전
		GetWorldCoordinate(&m_LastWorldPosition,CPoint(RGetScreenWidth()/2,(float)RGetScreenHeight()/2));
	}

	if(m_EditMode==EDITMODE_PATH)
	{
		POINT p = point;
//		ScreenToClient(&p);


		rvector pos,dir;
		RGetScreenLine(p.x,p.y,&pos,&dir);
		GetDocument()->m_bLastPicked=
			GetDocument()->m_pBspObject->PickOcTree(pos,dir,&GetDocument()->m_LastPicked,RM_FLAG_ADDITIVE | RM_FLAG_HIDE);
	}

}

void CWorldEditView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CView::OnLButtonUp(nFlags, point);


	{

		// pick test
		RBSPPICKINFO bpi;
		if(GetDocument()->m_pBspObject)
		{
			rvector pos,dir;
			RGetScreenLine(point.x,point.y,&pos,&dir);
			if(GetDocument()->m_pBspObject->Pick(pos,dir,&bpi))
			{
				g_LastPickPos=bpi.PickPos;
//				Invalidate();

				/*
				m_pSelectedNode=pNode;
				m_nSelectedIndex=nIndex;

				rvector *a,*b,*c;
				*a=pNode->pVertices[nIndex*3].Coord();
				*b=pNode->pVertices[nIndex*3+1].Coord();
				*c=pNode->pVertices[nIndex*3+2].Coord();

				float dist0=GetDistance(position,*a,*b);
				float dist1=GetDistance(position,*b,)
*/


			}
		}
	}

}

void CWorldEditView::OnResetCamera()
{
	// TODO: Add your control notification handler code here

	CWorldEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	rboundingbox *pbb,defaultbb;
	defaultbb.vmin=rvector(0.f,0.f,0.f);
	defaultbb.vmax=rvector(DEFAULTSIZE,DEFAULTSIZE,DEFAULTSIZE);
	rvector size;

	if(!pDoc->m_pBspObject)
		pbb=&defaultbb;
	else
		pbb=&pDoc->m_pBspObject->GetRootNode()->bbTree;

	size=pbb->vmax-pbb->vmin;

	rvector targetpos=.5f*(pbb->vmax+pbb->vmin);
	targetpos.z=0;
	rvector sourcepos=targetpos+rvector(0,100,100);
	RSetCamera(sourcepos,targetpos,rvector(0,0,1));
	RSetProjection(1.f/3.f*pi,100,55000);

	Invalidate();

}

void CWorldEditView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CView::OnMouseMove(nFlags, point);
 	
	if( (MK_LBUTTON & nFlags) != 0 )
	{
		if(m_bLastShiftState)
		{

			// 이동한 지점의 월드좌표를 구한다.
			rvector scrpoint=rvector((float)point.x,(float)point.y,0.1f);

			rmatrix inv;	
			float det;
			D3DXMatrixInverse(&inv,&det,&m_LastMatrix);

			rvector worldpoint;
			D3DXVec3TransformCoord(&worldpoint,&scrpoint,&inv);

			rplane plane=rplane(0,0,1,0);

			rvector intpointdst;
			D3DXPlaneIntersectLine(&intpointdst,&plane,&worldpoint,&m_LastCameraPosition);	

			// 이동을 시작했던 월드좌표가 구한 월드좌표로 이동하려면 카메라가 어떻게 움직여야 하는지 구한다.
			rmatrix cameratm=RView;

			D3DXMatrixTranslation(&cameratm,-m_LastCameraPosition.x,-m_LastCameraPosition.y,-m_LastCameraPosition.z);
			cameratm=cameratm*m_LastMatrix;

			D3DXMatrixInverse(&inv,&det,&cameratm);

			rvector screenpoint;
			D3DXVec3TransformCoord(&screenpoint,&intpointdst,&m_LastMatrix);
			D3DXVec3TransformCoord(&screenpoint,&screenpoint,&inv);

			// 카메라를 움직인다.
			RCameraPosition=m_LastCameraPosition+m_LastCameraPosition-(screenpoint-m_LastWorldPosition);
			RUpdateCamera();
		}

		if(m_bLastAltState)
		{
            CPoint Diff=point-m_LastCursorPosition;


			// 회전의 중심에 대해 카메라의 좌표를 상대좌표로 만들고..
			rvector relpos=m_LastCameraPosition-m_LastWorldPosition;
			float length=D3DXVec3Length(&relpos);
			Normalize(relpos);

			// 극좌표계로 바꾼다음..
			float anglex,anglez;
			anglex=acos(relpos.z);
			anglez=asin(relpos.x/sin(anglex));
			if(relpos.y<0)
				anglez=pi-anglez;

			// 회전해서..
			anglex+=-0.01f*Diff.y;
			anglex=min(max(anglex,0.001f),pi-0.001f);
			anglez+=-0.01f*Diff.x;

			relpos=length*rvector(sin(anglez)*sin(anglex),cos(anglez)*sin(anglex),cos(anglex));

			// 카메라를 회전시킨다..
			rvector newcamerapos=m_LastWorldPosition+relpos;
			RSetCamera(newcamerapos,m_LastWorldPosition,rvector(0,0,1));
            
		}

//		Invalidate();
	}
}

#define CAMERA_WHEEL_STEP	100.f

BOOL CWorldEditView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default

	GetWorldCoordinate(&m_LastWorldPosition,CPoint(RGetScreenWidth()/2,(float)RGetScreenHeight()/2));

	rvector dir=m_LastWorldPosition-RCameraPosition;
	Normalize(dir);

	RCameraPosition+=dir*CAMERA_WHEEL_STEP*zDelta/(float)WHEEL_DELTA;
	RUpdateCamera();
//	Invalidate();

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}
void CWorldEditView::OnRuntimeboundingbox()
{
	RBspObject *pbsp=GetDocument()->m_pBspObject;
	if(!pbsp) return;

	pbsp->OptimizeBoundingBox();
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CWorldEditView::OnWireframe()
{
	m_bWireframe=!m_bWireframe;
}

void CWorldEditView::OnUpdateWireframe(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bWireframe);
}

void CWorldEditView::OnBoundingbox()
{
	m_bDrawBoundingBox=!m_bDrawBoundingBox;
}

void CWorldEditView::OnUpdateBoundingbox(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bDrawBoundingBox);
}

void CWorldEditView::OnOcclusion()
{
	m_bDrawOcclusion=!m_bDrawOcclusion;
}

void CWorldEditView::OnUpdateOcclusion(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bDrawOcclusion);
}

void CWorldEditView::OnShowlightmap()
{
	m_bShowLightmap=!m_bShowLightmap;
}

void CWorldEditView::OnUpdateShowlightmap(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowLightmap);
}

void CWorldEditView::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

//	CView::OnDropFiles(hDropInfo);

	// .rs 혹은 .rs.xml .rs.lm 등등을 drop 해도 열린다.

	char szFileName[_MAX_PATH];
	DragQueryFile(hDropInfo,0,szFileName,sizeof(szFileName));

	int nLen;
	while( (nLen=strlen(szFileName))>0)
	{
		if(strnicmp(szFileName+nLen-3,".rs",3)==0)
		{
			AfxGetApp()->OpenDocumentFile(szFileName);
			return;
		}

		char *lastdot=strrchr(szFileName,'.');
		if(!lastdot) return;

		*lastdot=0;
	}
}
