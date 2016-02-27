// realspace.cpp maintained by dubble.

#include "RealSpace.h"
//#include "RSGlobal_Dll.h"
#include "RSVisualObject.h"
#include "RSConsoleRenderer.h"
#include "RSEnvironmentObject.h"
#include "RSDebug.h"
#include "FileInfo.h"
#include "RSMMX.h"
#include "RSPointLight.h"
#include "RSDecal.h"
#include "RSImage.h"
#include "RSMaterialManager.h"
#include "Profiler.h"

#include "MProfiler.h"

#define REALSPACE_DEFAULT_FOV (pi/6.0f)
#define REALSPACE_DEFAULT_NEARZCLIP_DISTANCE 0.01f
#define REALSPACE_DEFAULT_FARZCLIP_DISTANCE 60.0f
#define REALSPACE_DEFAULT_FOGSTART	30.0f
#define REALSPACE_DEFAULT_FOGEND	60.0f

rmatrix44 RSView;
rmatrix44 RSProjection,RSViewProjection,RSViewProjectionViewport,RSViewportMatrix;
rvector	RSCameraPosition,RSCameraOrientation;
rplane	RSViewFrustrum[6];
RSVIEWPORT RSViewport;
DWORD	tps,RSLightMapTexture,RSShadowTexture;
RSMaterial RSDefaultMaterial;

int		RSFrameCount=0;
float	RSFrameRate=0.f;

RealSpace::RealSpace()
{
	RSDllName[0]=0;RSDllAdvanceString[0]=0;
	Map=NULL;
	lasttime=timeGetTime();
	bShowConsole=false;ShadowStyle=RS_SHADOWSTYLE_NONE;
	bUseFog=false;
	LastFrameCount=0;RSFrameCount=0;LastFrameTime=timeGetTime();
	RSShadowTexture=NULL;RSLightMapTexture=NULL;
	RSLightColor=rvector(1.0f,1.0f,1.0f);
	m_bMMX = false;

	m_pConsoleRenderer=NULL;
	m_pEnvironmentObject=NULL;
	m_pDefaultRML=NULL;
	
#define EYE_DISTANCE	3
#define FOCUS_DISTANCE	10
#define ROTATE_DELTA 0.01f

	m_fEyeDistance=EYE_DISTANCE;
	m_fFocusDistance=FOCUS_DISTANCE;
	m_fRotateAngle=ROTATE_DELTA;

	m_nDrawOption	= 3;
	m_nDbgRenderLevel	= 5;
	m_nEnvLevel		= 2;

	m_nRenderedObjectCount = 0;
}

RealSpace::~RealSpace()
{
	if(m_pConsoleRenderer)
	{
		delete m_pConsoleRenderer;
		m_pConsoleRenderer=NULL;
	}
}

void RealSpace::Destroy()
{
	Base3DList.DeleteAll();
	ObjList.DeleteAll();
	CloseDefaultMaterial();
	int nRemainTextures=RSGetTextureCount();
	_ASSERT(!nRemainTextures);
	RSCloseDisplay();
//	RSDestroyDll();
}

bool RealSpace::Create(HWND hWnd, bool bFullScreen, int nWidth, int nHeight)
{
	Profiler profiler;

#define RSDEFAULTMATERIALNAME	"default"
	if(RSDefaultMaterial.Name==NULL)
	{
		RSDefaultMaterial.Name=new char[strlen(RSDEFAULTMATERIALNAME)+1];
		strcpy(RSDefaultMaterial.Name,RSDEFAULTMATERIALNAME);
	}

	m_bMMX = InitMMXFunction();
	if(m_bMMX){
		rslog("MMX support processor detected.\n");
	} else {
		rslog("MMX support processor not detected.\n");
	}
	RUtils_Initilize();
	if(g_bSSE)
		rslog("SSE support processor detected.\n");
	
	rslog("reading realspace.ini.\n");

	// read configuration
	char buffer[256];
	m_bDualMon=false;

	if(profiler.Load("realspace.ini"))
	{
//		if(profiler.Read("width",buffer,sizeof(buffer))) nWidth=atoi(buffer);
//		if(profiler.Read("height",buffer,sizeof(buffer))) nHeight=atoi(buffer);
		if(profiler.Read("fullscreen",buffer,sizeof(buffer))) bFullScreen=(atoi(buffer)==1);
		if(profiler.Read("picmip",buffer,sizeof(buffer))) g_nPicmip=atoi(buffer);
		if(profiler.Read("dualmon",buffer,sizeof(buffer))) m_bDualMon=(atoi(buffer)==1);
		if(profiler.Read("dualeyedist",buffer,sizeof(buffer))) m_fEyeDistance=(float)atof(buffer);
		if(profiler.Read("dualfocusdist",buffer,sizeof(buffer))) m_fFocusDistance=(float)atof(buffer);
		if(profiler.Read("rotateangle",buffer,sizeof(buffer))) m_fRotateAngle=(float)atof(buffer);
	}

#ifdef _DEBUG
		bFullScreen=0;		// 디버그모드는 일단 창모드 640x480 으로
		nWidth=640;
		nHeight=480;
#endif
	rslog("initializing\n");

	// init display
	if(!RSInitDisplay(hWnd,nWidth,nHeight,bFullScreen,m_bDualMon)) return false;	

	rslog("RealSpace Initialized ok.\n");

	m_fFOV_horiz=REALSPACE_DEFAULT_FOV;
	m_NearZClipDistance=REALSPACE_DEFAULT_NEARZCLIP_DISTANCE;
	m_FarZClipDistance=REALSPACE_DEFAULT_FARZCLIP_DISTANCE;

	RSViewport.x1=0;RSViewport.y1=0;
	RSViewport.x2=RSGetScreenWidth();RSViewport.y2=RSGetScreenHeight();
	SetViewport(&RSViewport);

	SetDirectionalLight(rvector(1,1,-1),rvector(1,1,1));
	SetFogRange(REALSPACE_DEFAULT_FOGSTART,REALSPACE_DEFAULT_FOGEND);

	m_pConsoleRenderer=new RSConsoleRenderer;
	m_pConsoleRenderer->Create("renglish.rfn");

	return TRUE;
}

int RealSpace::CreateTexture(int sx,int sy,char *memory,char *alphamap,bool bForceNotMipmap)
{
	return RSCreateTexture(sx,sy,memory,alphamap,bForceNotMipmap,"Directly Created");
}

int RealSpace::CreateTexture(const char *RMLName,const char *TextureName,bool bForceNotMipmap)
{
	char mask_name[256],texname[256];
	RSMaterialList *ml=new RSMaterialList;
	if(ml->Open(RMLName))
	{
		int iTex,iMask;

		strcpy(mask_name,TextureName);
		strcat(mask_name,"_mask");
		
		iTex=ml->GetTextureIndex(TextureName);
		iMask=ml->GetTextureIndex(mask_name);
		
		RSTexture *tex=NULL,*mask=NULL;
		if(iTex!=-1)
		{
			char *maskdata=NULL;
			tex=ml->GetTexture(iTex);
			ml->Lock(iTex);
			if(iMask!=-1)
			{
				mask=ml->GetTexture(iMask);
				ml->Lock(iMask);
				maskdata=mask->Data;
			}
//			maskdata=NULL;

#ifdef _DEBUG
			sprintf(texname,"%s,%s",RMLName,TextureName);
#endif
			int ret=RSCreateTexture(tex->x,tex->y,tex->Data,maskdata,bForceNotMipmap,texname);
			ml->Unlock(iTex);
			if(iMask!=-1)
				ml->Unlock(iMask);
			delete ml;
			return ret;
		}
	}
	delete ml;
	return NULL;
}

rvector RealSpace::GetCameraPosition()
{
	return RSCameraPosition;
}

rvector RealSpace::GetCameraOrientation()
{
	return RSCameraOrientation;
}

void RealSpace::SetCamera(rvector from, rvector at, rvector up)
{
	RSCameraOrientation=Normalize(at-from);
	RSCameraPosition=from;
	RSView=ViewMatrix44(from,at,up, 0);
	RSViewProjection=RSView * RSProjection;
	RSViewProjectionViewport=RSViewProjection * RSViewportMatrix;
	
	float fovh2=m_fFOV_horiz/2.0f,fovv2=m_fFOV_vert/2.0f;
	float ch=(float)cos(fovh2),sh=(float)sin(fovh2);
	float cv=(float)cos(fovv2),sv=(float)sin(fovv2);

	ComputeViewFrustrum(RSViewFrustrum+0, &RSView, RSCameraPosition, -ch,  0, sh);
	ComputeViewFrustrum(RSViewFrustrum+1, &RSView, RSCameraPosition, ch,  0, sh);
	ComputeViewFrustrum(RSViewFrustrum+2, &RSView, RSCameraPosition, 0,  cv, sv);
	ComputeViewFrustrum(RSViewFrustrum+3, &RSView, RSCameraPosition, 0, -cv, sv);
#define NEAR_DISTANCE	0.1f
	ComputeZPlane(RSViewFrustrum+4,RSCameraOrientation, RSCameraPosition, NEAR_DISTANCE,1);
	ComputeZPlane(RSViewFrustrum+5,RSCameraOrientation, RSCameraPosition, m_FarZClipDistance,-1);
}

void RealSpace::SetDirectionalLight(rvector &dir,rvector &color)
{
	RSSetDirectionalLight(dir,color);
}

RSObject* RealSpace::CreateObject(FILE *file,RSMaterialManager *pMM)
{
	RSObject *obj=new RSObject;
	if(!obj->Load(file,pMM ? pMM : m_pDefaultRML))
	{
		delete obj;
		return NULL;
	}
	ObjList.Add(obj);
	return obj;
}

RSObject* RealSpace::CreateObject(const char *RSMFileName,RSMaterialManager *pMM)
{
	RSObject *obj=new RSObject;
	if(!obj->Load(RSMFileName,pMM ? pMM : m_pDefaultRML))
	{
		delete obj;
		return NULL;
	}
	ObjList.Add(obj);
	return obj;
}

RSVisualObject* RealSpace::CreateVisualObject(RSObject *rsm,RSBase3D *parent)
{
	if(!rsm) return false;
	RSVisualObject *VO=new RSVisualObject;
	VO->Create(rsm);
	VO->SetShadowState(ShadowStyle);
	if(ShadowStyle==RS_SHADOWSTYLE_TEXTUREPROJECTION)
	{
		if(rsm->GetShadowTextureHandle()!=NULL)
			VO->SetShadowTextureHandle(rsm->GetShadowTextureHandle());
		else
			VO->SetShadowTextureHandle(RSShadowTexture);
	}
	if(parent) parent->AddChild(VO);
	Base3DList.Add(VO);
	return VO;
}

bool RealSpace::DeleteObject(int index)
{
	ObjList.Delete(index);
	return TRUE;
}

bool RealSpace::DeleteObject(RSObject *obj)
{
	for(int i=0;i<ObjList.GetCount();i++)
	{
		if(ObjList.Get(i)==obj)
		{
			DeleteObject(i);
			return TRUE;
		}
	}
	return false;
}

bool RealSpace::Delete3DObject(RSBase3D *vs)
{
	for(int i=0;i<Base3DList.GetCount();i++)
	{
		if(Base3DList.Get(i)==vs)
		{
			Base3DList.Delete(i);
			return true;
		}
	}
	return false;
}

RSPointLight* RealSpace::CreatePointLight(rvector Position,rvector Color,float radius,RSLIGHTSTYLE style)
{
	RSPointLight *PointLight=new RSPointLight(Position,Color,radius,style);
	PointLightList.Add(PointLight);
	return PointLight;
}

bool RealSpace::DeletePointLight(int i)
{
	PointLightList.Delete(i);
	return TRUE;
}

bool RealSpace::DeletePointLight(RSPointLight *l)
{
	for(int i=0;i<PointLightList.GetCount();i++)
	{
		if(PointLightList.Get(i)==l)
		{
			PointLightList.Delete(i);
			return TRUE;
		};
	}
	return false;
}

bool RealSpace::RenderScene(int nDevice)
{
	if(m_nDbgRenderLevel==0) return true;

	RSSelectDevice(nDevice);
	RSSetDirectionalLight(RSLightDirection,RSLightColor);
	RSSetCullState(RS_CULLSTYLE_NONE);

	int i;

	_BP("RealSpace::RenderScene() PointLight BB Test");

	RSPointLightList PointLightListInThisScene;

	if( (m_nDrawOption > 1 ) && ( m_nDbgRenderLevel > 4) ) {
		for(i=0;i<PointLightList.GetCount();i++) {
			RSPointLight *pl=PointLightList.Get(i);
			if(pl->BoundingBoxTest(RSViewFrustrum))
				PointLightListInThisScene.Add(pl);
		}
	}

	_EP("RealSpace::RenderScene() PointLight BB Test");

	RSSetTextureWrapState(TRUE);
	RSSetAlphaState(RS_ALPHASTYLE_NONE);
	RSSetZBufferState(TRUE,TRUE);
	if(bUseFog) RSSetFogState(true,m_FogColor);

// Draw Environment :: sky box ... etc
//	if(bUseFog) RSSetFogState(TRUE,m_FogColor);
	if(m_nDbgRenderLevel > 4)
		if(m_pEnvironmentObject){
			m_pEnvironmentObject->SetDrawLevel(m_nEnvLevel);
			m_pEnvironmentObject->Draw();
		}

	RSSetFogRange(m_FogStartZ,m_FogEndZ);

// Draw Preprocessing lights of Map ..
	_BP("Preprocessing lights Map");

	if( ( m_nDrawOption > 1 ) && ( m_nDbgRenderLevel > 4 ) ) {
		if(Map)	{
			for(i=0;i<PointLightListInThisScene.GetCount();i++)	{
				RSPointLight *pl=PointLightListInThisScene.Get(i);
				if(pl->GetStyle()==RS_LIGHTSTYLE_VERTEX) {
					Map->ProcessVertexLight(pl);
				}
			}
		}
	}

	_EP("Preprocessing lights Map");

// Draw Map
	_BP("Draw Map");
	if(bUseFog) RSSetFogState(TRUE,m_FogColor);
	RSSetZBufferState(TRUE,TRUE);

	if(m_nDbgRenderLevel > 3){
		if(Map) 
			Map->Draw();
	}
	else {
		if(Map) 
			Map->DrawLine();
	}

	_EP("Draw Map");

// Draw Shadows...
	_BP("Draw Shadows & Light Map");
	g_pd3dDevice->SetVertexShader(RSLFVF);
	RSSetTextureWrapState(TRUE);
	RSSetFogState(false,0);
	RSSetAlphaState(RS_ALPHASTYLE_NONE);
	RSSetZBufferState(TRUE,false);
	RSSetTexture(NULL);

	static RSBase3D *bo;

	if(( m_nDrawOption > 0 ) && (m_nDbgRenderLevel > 3)) {
		if(Map)	{
			for(i=0;i<Base3DList.GetCount();i++) {
				bo = Base3DList.Get(i);
				if(bo->GetVisible() && bo->BoundingBoxTest_Shadow(RSViewFrustrum,Map))	{
					bo->DrawShadow(Map);
				}
			}
		}
	}

// Draw LightMaps of Map ..
	if(bUseFog) RSSetFogState(TRUE,m_FogColor);
	RSSetZBufferState(TRUE,TRUE);
	RSSetTextureWrapState(TRUE);
	RSSetAlphaState(RS_ALPHASTYLE_ADD);
	RSSetTexture(RSLightMapTexture);

	if( (m_nDrawOption > 1) && (m_nDbgRenderLevel > 2)) {
		if(Map) {
			for(i=0;i<PointLightListInThisScene.GetCount();i++) {
				RSPointLight *pl=PointLightListInThisScene.Get(i);
				if(pl->GetStyle()==RS_LIGHTSTYLE_LIGHTMAP)
					Map->DrawLightMap(pl);
			}
		}
	}

	_EP("Draw Shadows & Light Map");

	
// Draw Visual Objects. ..
	_BP("RealSpace::RenderScene() Object BBTest & Draw");

	RSSetFogState(false,0);

	if(m_nDbgRenderLevel == 3) 
		g_pd3dDevice->SetRenderState( D3DRS_FILLMODE ,  D3DFILL_WIREFRAME );

	m_nRenderedObjectCount = 0;

	for(i=0;i<Base3DList.GetCount();i++)
	{
		bo=Base3DList.Get(i);
		bo->MoveAnimation((float)elapsedtime);

		if(bo->GetVisible() && bo->BoundingBoxTest(RSViewFrustrum))
		{
			if(Map && bUseFog)
			{
				rmatrix44 mat=MatrixMult(bo->GetMatrix(),RSView);
				float centerz=mat._43/mat._44;
				bo->SetFogColor(Map->GetFogValue(centerz));
			}
			else
				bo->SetFogColor(0xff000000);
//			bo->TransformAndLight(&PointLightListInThisScene);

			if((m_nDbgRenderLevel > 3)) {
				bo->SetupLight(&PointLightListInThisScene);
				bo->Draw();
			}
			else if(m_nDbgRenderLevel > 2) {
				bo->Draw();
			}
			else
				((RSVisualObject*)bo)->DrawBoundingBox();

			bo->m_bIsInScreen = true;
			if(((RSVisualObject*)bo)->m_isRender)
				m_nRenderedObjectCount++;
		}
		else
			bo->m_bIsInScreen = false;
	}

	if(m_nDbgRenderLevel == 3)
		g_pd3dDevice->SetRenderState( D3DRS_FILLMODE ,  D3DFILL_SOLID );
	
	_EP("RealSpace::RenderScene() Object BBTest & Draw");

	RSSetTexture(0);
	RSSetTextureWrapState(false);
	RSSetAlphaState(RS_ALPHASTYLE_NONE);
	RSSetZBufferState(TRUE,TRUE);
	RSSetCullState(RS_CULLSTYLE_NONE);
	
	PointLightListInThisScene.DeleteRecordAll();
	g_pd3dDevice->SetVertexShader(RSTLFVF);

	if( (m_nDrawOption > 0 ) && ( m_nDbgRenderLevel > 3 ) ) {
		g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, false );		//*/
		for(i=1;i<MAXIMUM_LIGHT+1;i++)
		{
			g_pd3dDevice->LightEnable( i, false );
		}
	}

	return TRUE;
}

bool RealSpace::RenderScene()
{
	if(!IsReadyToRender())
		return false;
	
	if(m_bDualMon)
	{
		rvector CameraPos=RSCameraPosition,CameraDir=RSCameraOrientation;
		rvector CameraTarget=CameraPos+CameraDir*m_fFocusDistance;

		rvector eye=m_fEyeDistance/2*Normalize(CrossProduct(CameraDir,rvector(0,0,1)));
		rvector up=CrossProduct(eye,CameraDir);
		
		SetCamera(CameraPos-eye,CameraTarget,up);
		SetProjection(m_fFOV_horiz,m_NearZClipDistance,m_FarZClipDistance,false,false);
		RenderScene(1);
		// right screen

		RSFrameCount++;

		SetCamera(CameraPos+eye,CameraTarget,up);
		SetProjection(m_fFOV_horiz,m_NearZClipDistance,m_FarZClipDistance,false,false);
		RenderScene(0);
		// left screen

		SetCamera(CameraPos,CameraPos+CameraDir,rvector(0,0,1));
	}
	else
		RenderScene(0);
	return true;
}

bool RealSpace::SetTargetMap(ASSMap *pTargetMap)
{
	Map=pTargetMap;
	if(Map==NULL) return false;
	Map->SetFogRange(m_FogStartZ,m_FogEndZ);
	FloorVertices[0]=rvertex(0,0,0);
	FloorVertices[1]=rvertex((float)Map->GetMapSizeX(),0,0);
	FloorVertices[2]=rvertex((float)Map->GetMapSizeX(),(float)Map->GetMapSizeY(),0);
	FloorVertices[3]=rvertex(0,(float)Map->GetMapSizeY(),0);
	pFloorVertices[0]=&FloorVertices[0];
	pFloorVertices[1]=&FloorVertices[1];
	pFloorVertices[2]=&FloorVertices[2];
	pFloorVertices[3]=&FloorVertices[3];
	return true;
}

char* RealSpace::Lock(int *lPitch)
{
	return RSLock(lPitch);
}

void RealSpace::Unlock()
{
	RSUnlock();
}

void RealSpace::Flip()
{
	thistime=timeGetTime();
	elapsedtime=thistime-lasttime;
	lasttime=thistime;

	if(m_pDefaultRML)
		m_pDefaultRML->MoveAnimation(elapsedtime);
	RSFrameCount++;
	if(thistime-LastFrameTime>2000)
	{
		RSFrameRate=float(RSFrameCount-LastFrameCount)*1000.f/float(thistime-LastFrameTime);
		LastFrameTime=thistime;
		LastFrameCount=RSFrameCount;
	}

	if(bShowConsole)
	{
		int i,Height,lPitch;
		char* screen=Lock(&lPitch);
		if(screen)
		{
			Height=m_pConsoleRenderer->GetHeight();
			for(i=0;i<GetLogHistoryCount();i++)
				m_pConsoleRenderer->TextOut((unsigned char*)screen,lPitch,0,i*Height,0xffffff,GetLogHistory(i));
			Unlock();
		}
	}

	tps=0;
	RSFlip();
}

void RealSpace::LogCurrentStatus()
{
	rslog("Current State : FPS %3.1f , %d Triangles\n",RSFrameRate,tps);
}

void RealSpace::SetViewport(RSVIEWPORT *a)
{
	RSViewport.x1=a->x1;RSViewport.y1=a->y1;
	RSViewport.x2=a->x2;RSViewport.y2=a->y2;
	RSSetRenderRect(a->x1,a->y1,a->x2,a->y2);
	RSwx=(float)(a->x2-a->x1)/2;RSwy=(float)(a->y2-a->y1)/2;
	RScx=(float)RSwx+a->x1;RScy=(float)RSwy+a->y1;
	RSViewportMatrix=IdentityMatrix44();
	RSViewportMatrix._11=RSwx;
	RSViewportMatrix._22=-RSwy;
	RSViewportMatrix._41=RScx;
	RSViewportMatrix._42=RScy;
	SetProjection(m_fFOV_horiz,m_NearZClipDistance,m_FarZClipDistance);
}

void RealSpace::SetProjection(float fov,float fNearZ,float fFarZ,bool bFlipHoriz,bool bFlipVert)
{
	m_fFOV_horiz=fov;
	m_NearZClipDistance=fNearZ;
	m_FarZClipDistance=fFarZ;
	m_fFOV_vert=(float)atan(tan(m_fFOV_horiz/2.0f)*(RSwy/RSwx))*2.0f;
	RSProjection=ProjectionMatrix(fNearZ,fFarZ,m_fFOV_horiz,m_fFOV_vert,bFlipHoriz,bFlipVert);
	RSViewProjection=RSView * RSProjection;
	RSViewProjectionViewport=RSViewProjection * RSViewportMatrix;
}

RSVisualObject *RealSpace::PickObject(int x,int y)
{
	static RSVisualObject *vobj,*ret;
	static FRECT rt;
	float minz=99999999.f;

	ret=NULL;
	for(int i=0;i<Base3DList.GetCount();i++)
	{
		float z,d2z;
		vobj=(RSVisualObject*)Base3DList.Get(i);
		if(vobj->isVisualObject)
		{
			if(vobj->m_bIsInScreen)
			{
				vobj->GetRect(&rt,&d2z);
/*
				if((rt.y1<y)&&(rt.y2>y)&&(rt.x1<x)&&(rt.x2>x)
					&& (((RSVisualObject*)vobj)->isContainThisPoint(x,y,&z)) 
					&& (minz>z) )
*/
				if((rt.y1<y)&&(rt.y2>y)&&(rt.x1<x)&&(rt.x2>x))
					if((((RSVisualObject*)vobj)->isContainThisPoint(x,y,&z))) 
						if( minz>d2z )
				{
					ret=vobj;
					minz=d2z;
				}
			}
		}
	}
	return ret;
}

int RealSpace::PickObjects(int x1,int y1,int x2,int y2,int BufferSize,RSVisualObject** Buffer)
{
#define EFFECTRANGE 0.3f

	int nPickedObject=0;
	static RSVisualObject **buf,*vobj;
	static FRECT rt,rt2;
	buf=Buffer;

	for(int i=0;i<Base3DList.GetCount();i++)
	{
		float z;
		vobj=(RSVisualObject*)Base3DList.Get(i);
		if(vobj->isVisualObject)
		{
			if(vobj->m_bIsInScreen && vobj->GetIsRendered() )
//			if(vobj->m_bIsInScreen )
			{
				vobj->GetRect(&rt2,&z);
				rt.x1=(rt2.x1+rt2.x2)*0.5f-(rt2.x2-rt2.x1)*EFFECTRANGE;
				rt.x2=(rt2.x1+rt2.x2)*0.5f+(rt2.x2-rt2.x1)*EFFECTRANGE;
				rt.y1=(rt2.y1+rt2.y2)*0.5f-(rt2.y2-rt2.y1)*EFFECTRANGE;
				rt.y2=(rt2.y1+rt2.y2)*0.5f+(rt2.y2-rt2.y1)*EFFECTRANGE;

				if( (rt.x1<x2 && rt.x2>x1) && (rt.y1<y2 && rt.y2>y1) &&
					(BufferSize>nPickedObject) ){
					nPickedObject++;
					*buf=vobj;
					buf++;
				}
			}
		}
	}
	return nPickedObject;
}

int RealSpace::PickObjects(int x,int y,int BufferSize,RSVisualObject** Buffer)
{
	int nPickedObject=0;
	static RSVisualObject **buf,*vobj;
	static FRECT rt,rt2;
	buf=Buffer;

	for(int i=0;i<Base3DList.GetCount();i++)
	{
		float z;
		vobj=(RSVisualObject*)Base3DList.Get(i);
		if(vobj->isVisualObject)
		{
			if(vobj->m_bIsInScreen)
			{
				vobj->GetRect(&rt2,&z);
				rt.x1=(rt2.x1+rt2.x2)*0.5f-(rt2.x2-rt2.x1)*EFFECTRANGE;
				rt.x2=(rt2.x1+rt2.x2)*0.5f+(rt2.x2-rt2.x1)*EFFECTRANGE;
				rt.y1=(rt2.y1+rt2.y2)*0.5f-(rt2.y2-rt2.y1)*EFFECTRANGE;
				rt.y2=(rt2.y1+rt2.y2)*0.5f+(rt2.y2-rt2.y1)*EFFECTRANGE;

				if((rt.y1<y)&&(rt.y2>y)&&(rt.x1<x)&&(rt.x2>x)&&
					/*(minz>z)&&*/(BufferSize>nPickedObject)){
					nPickedObject++;
					*buf=vobj;
					buf++;
				}
			}
		}
	}
	return nPickedObject;
}

bool RealSpace::Pick(rvector &from,rvector &to,rvector *ret)
{
	bool bPicked=false;
	float mindist = FLT_MAX;
	
	for(int i=0;i<Base3DList.GetCount();i++)
	{
		RSBase3D *pB3=Base3DList.Get(i);
		if(pB3->isVisualObject)
		{
			RSVisualObject *pVO=(RSVisualObject*)pB3;
			rvector objret;
			if(pVO->Pick(from,to,&objret))
			{
				if(!bPicked)
				{
					bPicked=true;
					*ret=pVO->GetWorldCoordinate(objret);
					mindist=(*ret).GetDistance(from);
				} else
				{

					rvector temp=pVO->GetWorldCoordinate(objret);
					float dist=temp.GetDistance(from);
					if(dist<mindist)
					{
						mindist=dist;
						*ret=temp;
					}
				}
			}
		}
	}

	if(Map)
	{
		rvector mapret;
		if(Map->Pick(from,to,&mapret))
		{
			float dist=mapret.GetDistance(from);
			if(dist<mindist)
			{
				mindist=dist;
				*ret=mapret;
			}
		}
	}

	return bPicked;
}

bool RealSpace::OpenDefaultMaterial(const char *RMLName)
{
	m_pDefaultRML=new RSMaterialManager;
	m_pDefaultRML->SetUsePicMip(true);	// Use Texture DetailMap
	return m_pDefaultRML->Open(RMLName);
}

void RealSpace::CloseDefaultMaterial()
{	
	if(m_pDefaultRML)
	{
		m_pDefaultRML->Close();
		delete m_pDefaultRML;
		m_pDefaultRML=NULL;
	}
}

void RealSpace::DrawHLine( LPBYTE pTarget, int nLeft, int nTop, int nLen, int lPitch, int nColor )
{
	int Bpp=RSPixelFormat==RS_PIXELFORMAT_X888 ? 4 : 2;
	WORD color=RGB322RGB16(nColor);

	int w = int(RSViewport.x2 - RSViewport.x1);
	int h = int(RSViewport.y2 - RSViewport.y1);

	if(nLen<0){
		nLeft += nLen;
		nLen = -nLen;
	}
	if(nLen<=0) return;	// nLen는 1이상이어야 한다.
	int nClipedLeft;
	int nClipedTop;
	int nClipedLen;
	if(nLeft<0) nClipedLeft = 0;
	else if(nLeft>=w) return;
	else nClipedLeft = nLeft;
	if(nTop<0) return;
	else if(nTop>=h) return;
	else nClipedTop = nTop;

	int nTargetX = nLeft + nLen - 1;
	if(nTargetX>=w) nClipedLen = nLen - (nTargetX-w+1) - (nClipedLeft-nLeft);
	else nClipedLen = nLen - (nClipedLeft-nLeft);

	switch(Bpp)
	{
	case 2:
		{
			LPWORD pPtr;
			pPtr = (LPWORD)pTarget + ((nClipedTop * lPitch) >> 1) + nClipedLeft;
			for(int i=0; i<nClipedLen; i++){
				*pPtr++ = color;
			}
		}break;
	case 4:
		{
			LPDWORD pPtr;
			pPtr = (LPDWORD) ( pTarget + nClipedTop * lPitch + nClipedLeft * Bpp );
			for(int i=0; i<nClipedLen; i++){
				*pPtr++ = nColor;
			}
		}break;
	}
}

void RealSpace::DrawVLine( LPBYTE pTarget, int nLeft, int nTop, int nLen, int lPitch, int nColor )
{
	int Bpp=RSPixelFormat==RS_PIXELFORMAT_X888 ? 4 : 2;
	WORD color=RGB322RGB16(nColor);

	int w = int(RSViewport.x2 - RSViewport.x1);
	int h = int(RSViewport.y2 - RSViewport.y1);

	if(nLen<0){
		nTop += nLen;
		nLen = -nLen;
	}

	if(nLen<=0) return;	// nLen는 1이상이어야 한다.
	int nClipedLeft;
	int nClipedTop;
	int nClipedLen;
	if(nLeft<0) return;
	else if(nLeft>=w) return;
	else nClipedLeft = nLeft;
	if(nTop<0) nClipedTop = 0;
	else if(nTop>=h) return;
	else nClipedTop = nTop;

	int nTargetY = nTop + nLen - 1;
	if(nTargetY>=h) nClipedLen = nLen - (nTargetY-h+1) - (nClipedTop-nTop);
	else nClipedLen = nLen - (nClipedTop-nTop);

	switch(Bpp)
	{
	case 2:
		{
			LPWORD pPtr;
			pPtr = (LPWORD)pTarget + ((nClipedTop * lPitch) >> 1) + nClipedLeft;
			for(int i=0; i<nClipedLen; i++){
				*pPtr = color;		
				pPtr += (lPitch >> 1);
			}
		}break;
	case 4:
		{
			LPDWORD pPtr;
			pPtr = (LPDWORD) ( pTarget + nClipedTop * lPitch + nClipedLeft*Bpp );
			for(int i=0; i<nClipedLen; i++){
				*pPtr = nColor;		
				pPtr += lPitch >> 2;
			}
		}break;
	}
}

void RealSpace::DrawRect( LPBYTE pTarget, int nLeft, int nTop, int nWidth, int nHeight, int lPitch, int nColor )
{
	DrawHLine(pTarget, nLeft, nTop, nWidth, lPitch, nColor);
	DrawHLine(pTarget, nLeft, nTop+nHeight-1, nWidth, lPitch, nColor);
	DrawVLine(pTarget, nLeft, nTop, nHeight, lPitch, nColor);
	DrawVLine(pTarget, nLeft+nWidth-1, nTop, nHeight, lPitch, nColor);
}

void RealSpace::PutPixel( LPBYTE pTarget, int nX, int nY, int lPitch, int nColor )
{
	if( nX < RSViewport.x1 || nX > RSViewport.x2 || nY < RSViewport.y1 || nY > RSViewport.y2 ) return;
	
	LPWORD pPtr;
	pPtr = (LPWORD)pTarget + ((nY * lPitch) >> 1) + nX;
	*pPtr = RGB322RGB16(nColor);
}

RSDecal* RealSpace::CreateDecal(const char *MaterialName)
{
	RSDecal *decal=new RSDecal;
	decal->SetMaterial(m_pDefaultRML->GetMaterial(m_pDefaultRML->GetIndex(MaterialName)));
	return decal;
}

RSImage* RealSpace::CreateImage(const char *RMLName,const char *name)
{
	RSMaterialList *ml=new RSMaterialList;
	ml->Open(RMLName);
	if(!ml) return NULL;
	RSImage *newimage=new RSImage;
	if(!newimage->Open(ml,name)) {delete newimage;return NULL;}
	delete ml;
	return newimage;
}

/*
void RealSpace::CreateEnvironment(const char *RMLName,const char *uptexture,const char *lefttexture,
								  const char *fronttexture,const char *righttexture,const char *backtexture,
								  const char *downtexture)
{
	int handle[6];
	handle[0]=CreateTexture(RMLName,uptexture);
	handle[1]=CreateTexture(RMLName,lefttexture);
	handle[2]=CreateTexture(RMLName,fronttexture);
	handle[3]=CreateTexture(RMLName,righttexture);
	handle[4]=CreateTexture(RMLName,backtexture);
	handle[5]=CreateTexture(RMLName,downtexture);

	bNeedEnvironment=1;
	m_pEnvironmentObject=new RSEnvironmentObject;
	m_pEnvironmentObject->CreateAsBox(handle);
}
*/

void RealSpace::SetEnvironment(RSEnvironmentObject *pEnvironment)
{
	m_pEnvironmentObject=pEnvironment;
}

void RealSpace::SetFogRange(float sz,float ez)
{
	/*
	float ssz,sez;
	rvector v0,vr;
	v0=rvector(0,0,sz);
	vr=TransformVector(v0,RSProjection);ssz=vr.z;
	v0=rvector(0,0,ez);
	vr=TransformVector(v0,RSProjection);sez=vr.z;
	m_FogStartZ=ssz;m_FogEndZ=sez;
	m_FogDistance=m_FogEndZ-m_FogStartZ;
	*/
	if(Map) Map->SetFogRange(sz,ez);
	RSSetFogRange(sz,ez);
	m_FogStartZ=sz;
	m_FogEndZ=ez;
}

bool RealSpace::GetViewFrustrumClippedFloorVertices(rvertex ***verbuff,int *nverbuff)
{
	if(!Map) return false;
	return GetVerlistClipped(pFloorVertices,4,verbuff,nverbuff);
}

void RealSpace::DrawLine3D(rvertex *v1,rvertex *v2)
{
	LITVERTEX v[2];
	v[0].x=v1->x;v[0].y=v1->y;v[0].z=v1->z;v[0].Diffuse=v1->color;
	v[1].x=v2->x;v[1].y=v2->y;v[1].z=v2->z;v[1].Diffuse=v2->color;
	g_pd3dDevice->SetTransform(D3DTS_WORLD, (_D3DMATRIX*)&IdentityMatrix44());
	g_pd3dDevice->SetVertexShader(RSLFVF);
	g_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST,1,v,sizeof(LITVERTEX));
}

bool RealSpace::IsReadyToRender()
{
	HRESULT hr;
    if( FAILED( hr = g_pd3dDevice->TestCooperativeLevel() ) )
    {
        // If the device was lost, do not render until we get it back
        if( D3DERR_DEVICELOST == hr )
            return false;

        // Check if the device needs to be resized.
        if( D3DERR_DEVICENOTRESET == hr )
			return ResetDevice();
	}
	return true;
}

bool RealSpace::ResetDevice()
{
	if(Map)
		Map->Invalidate();

	int i;

	for(i=0;i<RSGetDeviceCount();i++)
	{
		if(g_pDevices[i])
		{
			RSSelectDevice(i);
			RSResetDevice();
		}
	}
	
	if(Map)
		Map->Restore();
	rslog("All devices Restored. \n");
	return true;
}