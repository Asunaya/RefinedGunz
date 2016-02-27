// RealSpace.h: interface for the RealSpace class.

#ifndef __REALSPACE_H
#define __REALSPACE_H

#include "RsUtils.h"
#include "RSPointLight.h"
#include "RSObject.h"
#include "RSBase3D.h"
#include "RSD3D.h"

#pragma comment( lib, "winmm.lib" )

#ifndef _MAX_PATH
	#define _MAX_PATH 255
#endif

extern rmatrix44	RSView;
extern rmatrix44	RSProjection,RSViewProjection,RSViewportMatrix,RSViewProjectionViewport;
extern rvector		RSCameraPosition,RSCameraOrientation;
extern rplane		RSViewFrustrum[6];
extern RSVIEWPORT RSViewport;
extern int RSFrameCount;
extern float RSFrameRate;
extern DWORD tps,RSLightMapTexture,RSShadowTexture;
extern RSMaterial	RSDefaultMaterial;

class RealSpace  
{
public:
	RealSpace();
	virtual ~RealSpace();

// 전체적인 제어..
	bool Create(HWND hWnd, bool bFullScreen=false, int nWidth=640, int nHeight=480);
	void Destroy();

	void SetProjection(float fov,float fNearZ,float fFarZ,bool bFlipHoriz=false,bool bFlipVert=false);

	void SetViewport(RSVIEWPORT*);
	RSVIEWPORT *GetViewport() { return &RSViewport; }
	static int GetViewportWidth(){ return (RSViewport.x2 - RSViewport.x1); }
	static int GetViewportHeight(){ return (RSViewport.y2 - RSViewport.y1); }

// skybox skysphere(?) 등등은 environmentobject로..
	void SetEnvironment(RSEnvironmentObject *pEnvironment);
/*
	void CreateEnvironment(const char *RMLName,
								const char *uptexture,const char *lefttexture,
								  const char *fronttexture,const char *righttexture,
								  const char *backtexture,const char *downtexture);
*/
// Texture, Material 관련
	int CreateTexture(int sx,int sy,char *memory,char *alphamap=NULL,bool bForceNotMipmap=false);
	int CreateTexture(const char *RMLName,const char *TextureName,bool bForceNotMipmap=false);
	void DeleteTexture(int Handle) { RSDeleteTexture(Handle); }
	bool OpenDefaultMaterial(const char *RMLName);
	void CloseDefaultMaterial();
	RSMaterialManager *GetDefaultMaterial() { return m_pDefaultRML; }

// Object 관련
	RSObject* CreateObject(FILE *file,RSMaterialManager *pMM=NULL);
	RSObject* CreateObject(const char *RSMFileName,RSMaterialManager *pMM=NULL);
	RSVisualObject* CreateVisualObject(RSObject* rsm,RSBase3D *parent=NULL);
	RSObject* GetObject(int i){return ObjList.Get(i);}
	RSBase3D* Get3DObject(int i){return Base3DList.Get(i);}

	bool DeleteObject(int index);
	bool DeleteObject(RSObject *);
	bool Delete3DObject(RSBase3D *vs);

// Create helpers ... 그러나 렌더링이나 delete는 하지 않음
	RSDecal* CreateDecal(const char *MaterialName);						// obsolete :: use  "new RSDecal"
	RSImage* CreateImage(const char *RMLName,const char *name);

// 빛 관련
	void SetDirectionalLight(rvector &dir,rvector &color);
	RSPointLight *CreatePointLight(rvector Position,rvector Color,float radius,RSLIGHTSTYLE style=RS_LIGHTSTYLE_VERTEX);
	bool DeletePointLight(RSPointLight*);
	bool DeletePointLight(int);

// Camera 관련
	rvector GetCameraPosition();
	rvector GetCameraOrientation();
	void SetCamera(rvector from, rvector at, rvector up);
	float GetFarZClipDistance() { return m_FarZClipDistance; }
	float GetFOV_Horiz() { return m_fFOV_horiz; }
	float GetFOV_Vert() { return m_fFOV_vert; }

// rendering 관련
	void Flip();
	bool IsReadyToRender();
	bool ResetDevice();
/*	bool AttachMap(char*,ASSSchemeManager*);
	bool DetachMap();
*/
	bool RenderScene();
	bool RenderScene(int nDevice);		// 한 디바이스만 렌더.

// rendering primitives
	inline void Transform(rvertex *v) { TransformVertexASM(*v,RSViewProjectionViewport); }
	inline void DrawFace(rface *f) { RSDrawFace(f); }
	inline void SetTexture(int iT) { RSSetTexture(iT); }
	inline void DrawLine(rvertex *v1,rvertex *v2) { RSDrawLine(v1,v2); }
	void DrawLine3D(rvertex *v1,rvertex *v2);

// state and library
	void SetShadowState(RSSHADOWSTYLE a) { ShadowStyle=a; }
	void SetShadowTexture(int Handle){	RSShadowTexture=Handle;}
	void SetLightMapTexture(int Handle){ RSLightMapTexture=Handle;}
	void SetFogRange(float sz,float ez);
	void SetFogState(bool bEnable,DWORD color) { bUseFog=bEnable;m_FogColor=color;}
	bool GetFogEnable() { return bUseFog; }

	static void SetAlphaState( RSALPHASTYLE a ){ RSSetAlphaState( a ); }
	static void SetZBufferState( bool bEnable ){ RSSetZBufferState( bEnable, bEnable ); }

// 2d functions
	void Unlock();
	char* Lock(int *lPitch);
	void DrawRect( LPBYTE pTarget, int nLeft, int nTop, int nWidth, int nHeight, int lPitch, int nColor );
	void DrawHLine( LPBYTE pTarget, int nLeft, int nTop, int nLen, int lPitch, int nColor );
	void DrawVLine( LPBYTE pTarget, int nLeft, int nTop, int nLen, int lPitch, int nColor );
	void PutPixel( LPBYTE pTarget, int nX, int nY, int lPitch, int nColor );

// picking 관련
	RSVisualObject *PickObject(int x,int y);
	int PickObjects(int x1,int y1,int x2,int y2,int BufferSize,RSVisualObject** Buffer);
	int PickObjects(int x1,int y1,int BufferSize,RSVisualObject **Buffer);
	bool Pick(rvector &from,rvector &to,rvector *ret);

// map관련 없어져야할 것들...
	ASSMap* GetMap()	{	return Map;	}
	bool SetTargetMap(ASSMap *pTargetMap);

	void SetDrawOption(int m)	{ m_nDrawOption = m;}
	int  GetDrawOption()		{ return m_nDrawOption;}

	void SetRenderLevel(int lv) { m_nDbgRenderLevel = lv; }
	int	 GetRenderLevel()		{ return m_nDbgRenderLevel; }

	void SetEnvLevel(int lv)	{ m_nEnvLevel = lv; }
	int	 GetEnvLevel()			{ return m_nEnvLevel; }

	/*
	rface *PickMapFace(float x,float y);
	bool PickMap(int x,int y,rvector *ret);
	rvector GetSurfaceNormal(float x,float y) { return Map->GetNormal(x,y); }
	rvector GetInterpolatedSurfaceNormal(float x,float y) { return Map->GetInterpolatedNormal(x,y); }
	float GetMapHeight(float x,float y) { return Map?Map->GetHeight(x,y):0; }
*/
// help functions...
	void ShowConsole(bool b) {bShowConsole=b;};
	void LogCurrentStatus();
	bool GetViewFrustrumClippedFloorVertices(rvertex ***verbuff,int *nverbuff);
	bool IsMMX(){ /* MMX인가? Create(...)함수가 불린 이후에만 제대로 인식된다 */ return m_bMMX; }
	int	 GetRenderedObjectCount() { return m_nRenderedObjectCount; }
	
private:
	RSConsoleRenderer	*m_pConsoleRenderer;
	RSEnvironmentObject	*m_pEnvironmentObject;
	RSMaterialManager	*m_pDefaultRML;

	char RSDllName[256],RSDllAdvanceString[256];
	ASSMap	*Map;
	DWORD lasttime,thistime,elapsedtime,LastFrameCount,LastFrameTime;
	DWORD m_FogColor;
	bool bShowConsole,bShadowEnable,bUseFog;
	RSSHADOWSTYLE ShadowStyle;
	rvertex FloorVertices[4],*pFloorVertices[4];
	bool m_bMMX;
	int	m_nDrawOption;
	int m_nDbgRenderLevel;
	int	m_nEnvLevel;

private:
	CMLinkedList <RSObject> ObjList;
	CMLinkedList <RSBase3D> Base3DList;
	CMLinkedList <RSPointLight> PointLightList;
//	CMLinkedList <RSDecal> DecalList;

	float RScx,RScy,RSwx,RSwy;
	float m_fFOV_horiz,m_fFOV_vert,m_FarZClipDistance,m_NearZClipDistance;
	float m_FogStartZ,m_FogEndZ,m_FogDistance;

	int m_nRenderedObjectCount;

// dualmonitor properties
public:
	bool m_bDualMon;
	float m_fEyeDistance,m_fFocusDistance,m_fRotateAngle;			// 눈사이의 거리, 포커스 거리, 회전 각

};

#endif // of __REALSPACE_H
