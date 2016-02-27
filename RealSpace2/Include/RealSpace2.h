#ifndef __REALSPACE2_H
#define __REALSPACE2_H

#include "RTypes.h"
#include "RError.h"
#include "MZFileSystem.h"

#pragma comment(lib,"d3dx9.lib")

#include "RNameSpace.h"
_NAMESPACE_REALSPACE2_BEGIN

class RParticleSystem;

// 바깥에서 참조할만한 것들.
bool	RIsActive();
bool	RIsQuery();
void	RSetQuery(bool b);//테스트용..
bool	RIsFullScreen();
bool	RIsHardwareTNL();
bool	RIsSupportVS();
bool	RIsAvailUserClipPlane();
bool	RIsTrilinear();
int		RGetApproxVMem();
int		RGetScreenWidth();
int		RGetScreenHeight();
int		RGetPicmip();
RPIXELFORMAT RGetPixelFormat();
D3DADAPTER_IDENTIFIER9*	RGetAdapterID();
void SetClearColor(DWORD c);
int		RGetVideoMemory();
void	RSetWBuffer(bool bEnable);
bool	RIsStencilBuffer();


int RGetAdapterModeCount( D3DFORMAT Format,UINT Adapter = D3DADAPTER_DEFAULT );
bool REnumAdapterMode( UINT Adapter, D3DFORMAT Format ,UINT Mode, D3DDISPLAYMODE* pMode );


// 파티클 시스템
RParticleSystem *RGetParticleSystem();


extern int g_nFrameCount,g_nLastFrameCount;
extern float g_fFPS;
extern HWND	g_hWnd;
extern MZFileSystem *g_pFileSystem;

// 초기화 & 종료
bool RInitDisplay(HWND hWnd,const RMODEPARAMS *params);
bool RCloseDisplay();
void RSetFileSystem(MZFileSystem *pFileSystem);
void RAdjustWindow(const RMODEPARAMS *pModeParams);		// 윈도우 크기를 맞춰줌

// 디바이스 & 뷰포트 
LPDIRECT3DDEVICE9	RGetDevice();

// 모드전환 & 플리핑관련
void RResetDevice(const RMODEPARAMS *params);
RRESULT RIsReadyToRender();
void RFlip();
bool REndScene();
bool RBeginScene();

// RS의 렌더와 관련된 옵션들 - 더 추가할 것 있으면 추가하도록 하자.
enum RRENDER_FLAGS
{
	RRENDER_CLEAR_BACKBUFFER		= 0x000001,
};

void RSetRenderFlags(unsigned long nFlags);
unsigned long RGetRenderFlags();

//Fog
void RSetFog(bool bFog, float fNear = 0, float fFar=0, DWORD dwColor=0xFFFFFFFF );
bool RGetFog();
float RGetFogNear();
float RGetFogFar();
DWORD RGetFogColor();

// 카메라 관련	( RCamera.cpp )
extern rvector RCameraPosition,RCameraDirection,RCameraUp;
extern rmatrix RView,RProjection,RViewProjection,RViewport,RViewProjectionViewport;

void RSetCamera(const rvector &from, const rvector &at, const rvector &up);
void RUpdateCamera();		// rsetcamera 를 사용하거나 직접 고친후 update를 해줘야합니다.

void RSetProjection(float fFov,float fNearZ,float fFarZ);
void RSetProjection(float fFov,float fAspect,float fNearZ,float fFarZ);

inline rplane *RGetViewFrustum();
void RSetViewport(int x1,int y1,int x2,int y2);
D3DVIEWPORT9		*RGetViewport();

void RResetTransform();		// 밖에서 쓸 일은 없는 평션.. 디바이스 로스트 되었을때 불림.


///////////////////////// 도움을 주는 펑션들.

// 화면좌표계 x,y 를 3차원의 line으로 변환
bool RGetScreenLine(int sx,int sy,rvector *pos,rvector *dir);

// 화면좌표계의 x,y 와 임의의 평면과의 교점을 구한다.
rvector RGetIntersection(int x,int y,rplane &plane);
bool RGetIntersection( rvector& a, rvector& b, rplane &plane, rvector* pIntersection );

// 3d 좌표를 화면좌표로 transform 한다.
rvector RGetTransformCoord(rvector &coord);

// 그리는것과 관련된 펑션..
void RDrawLine(rvector &v1,rvector &v2,DWORD dwColor);
void RDrawCylinder(rvector origin,float fRadius,float fHeight,int nSegment);
void RDrawCorn(rvector center,rvector pole,float fRadius,int nSegment);
void RDrawSphere(rvector origin,float fRadius,int nSegment);
void RDrawAxis(rvector origin,float fSize);
void RDrawCircle(rvector origin,float fRadius, int nSegment);
void RDrawArc(rvector origin, float fRadius, float fAngle1, float fAngle2, int nSegment, DWORD color);

// bmp를 저장한다 data 는 argb 각각 1byte로 되어있다.
bool RSaveAsBmp(int x,int y,void *data,const char *szFilename);

// 스크린샷을 저장한다 data 는 argb 각각 1byte로 되어있다.
bool RScreenShot(int x,int y,void *data,const char *szFilename);

LPDIRECT3DSURFACE9 RCreateImageSurface(const char *filename);

// 감마값 설정
void RSetGammaRamp(unsigned short nGammaValue = 255);
// wbuffer enable
void RSetWBuffer(bool bEnable);

// winmain 어시스트.

enum RFUNCTIONTYPE {
	RF_CREATE  =0,
	RF_DESTROY,
	RF_RENDER,
	RF_UPDATE,
	RF_INVALIDATE,
	RF_RESTORE,
	RF_ACTIVATE,
	RF_DEACTIVATE,
	RF_ERROR,

	RF_ENDOFRFUNCTIONTYPE		// 이건 실제 쓰는 펑션이 아니고.. 숫자를 세기위한것임.
};

typedef RRESULT (*RFFUNCTION)(void *Params);

void RSetFunction(RFUNCTIONTYPE ft,RFFUNCTION pfunc);
int RMain(const char *AppName, HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow, RMODEPARAMS *pModeParams, WNDPROC winproc=NULL, WORD nIconResID=NULL );

//////Do Not Use This Method --> Danger...
//void FixedResolutionRenderStart();
//void FixedResolutionRenderEnd();
//void FixedResolutionRenderInvalidate();
////// Use This Method
//void FixedResolutionRenderFlip();
//bool IsFixedResolution();
//void SetFixedResolution( bool b );


////// inline functions
extern rplane RViewFrustum[6];
inline rplane *RGetViewFrustum()
{
	return RViewFrustum;
}


_NAMESPACE_REALSPACE2_END

#endif