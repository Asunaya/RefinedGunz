#ifndef __RSD3D_H
#define __RSD3D_H

#include "d3d8.h"
#include "rstypes.h"

#define	MAX_TEXTURE_COUNT	2048
#define MAX_TEXTURE_SIZE	256
#define MAXIMUM_LIGHT	4


#define RSFVF	( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 )
#define RSLFVF	( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1 )
#define RSTLFVF	( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1 )

typedef struct _VERTEX {
    float x, y, z;		// world position
	float nx,ny,nz;		// normal
    float tu1, tv1;		// texture coordinates
} VERTEX, *LPVERTEX; 

typedef struct _LITVERTEX {
    float x, y, z;		// world position
    DWORD Diffuse;   // diffuse color    
    DWORD Specular;  // specular color    
    float tu1, tv1;  // texture coordinates
} LITVERTEX, *LPLITVERTEX; 

typedef struct _TRANSLITVERTEX {
    float x, y;      // screen position    
    float z;         // Z-buffer depth    
    float rhw;       // reciprocal homogeneous W    
    DWORD Diffuse;   // diffuse color    
    DWORD Specular;  // specular color    
    float tu1, tv1;  // texture coordinates
} TRANSLITVERTEX, *LPTRANSLITVERTEX; 

// WORD RGB16( BYTE r, BYTE g, BYTE b )를 위한 함수 정의
typedef WORD (RGB16f_t)(BYTE r, BYTE g, BYTE b);
// BYTE GetX16( WORD rgb )를 위한 함수 정의
typedef BYTE (GetXf_t)(WORD rgb);

extern RGB16f_t		*RS_RGB;
extern GetXf_t		*RS_GetR,*RS_GetG,*RS_GetB;

extern RSPIXELFORMAT			RSPixelFormat;
extern LPDIRECT3DDEVICE8		g_pd3dDevice;
extern LPDIRECT3DDEVICE8*		g_pDevices;
extern LPDIRECT3DTEXTURE8		g_pShadowTexture;
extern LPDIRECT3D8				g_pD3D;
extern D3DPRESENT_PARAMETERS	g_d3dpp;

extern rvector	RSLightDirection,RSLightColor;
extern int g_nPicmip;
extern bool g_bHardwareTNL;

RSPIXELFORMAT RSGetPixelFormat();

int RSGetScreenWidth();
int RSGetScreenHeight();
void RSSetScreenWH(int w,int h);

void RSSetPicmip(int n); 
int	 RSSetPicmip(); 

bool RSGetBFullScreen();
void RSSetBFullScreen(bool bFullSCreen);

int RSGetDeviceCount();
int RSGetCurrentDevice();
int RSGetTextureCount();
void RSDrawFace(rface *f);
void RSDrawFaceArray(rface *f,int nFace);
void RSDrawLine(rvertex *v1,rvertex *v2);
void RSDrawTriangle(rvertex *v1,rvertex *v2,rvertex *v3);
int RSCreateTexture(int x,int y,char *memory,char *alphamap,bool bForceNotMipmap,char *name,bool bUsePicMip = false );
bool RSDeleteTexture(int iT);
void RSSetTexture(int iT);
void RSSetTexture(int iStage,int iT);
char* RSLock(int *lPitch);
void RSUnlock();
char* RSLockZ(int *lPitch);
void RSUnlockZ();
void RSSetRenderRect(int x1,int y1,int x2,int y2);
void RSSetAlphaState(RSALPHASTYLE a);
void RSSetZBufferState(bool r,bool w); // read z, write z
void RSSetFogRange(float sz,float ez);
void RSSetFogState( bool bFog , DWORD dwColor);
void RSSetTextureWrapState(bool bWrap);
void RSDrawStrip(rstrip *pStrip);
void RSDrawLineStrip(rstrip *pStrip);
void RSSetCullState(RSCULLSTYPE cs);
bool RSInitDisplay(HWND hWnd,int nWidth,int nHeight,bool bFullScreen,bool bDualMon);
bool RSCloseDisplay();
bool RSFlip();
void RSSelectDevice(int nDevice);
void RSSetDirectionalLight(rvector &dir,rvector &color);
void RSResetDevice();
LPDIRECT3DSURFACE8 RSGetBackBuffer();
LPDIRECT3DDEVICE8	RSGetD3DDevice();

// Restore CallBack Function
enum RFUNCTIONTYPE {
	RF_CREATE  =0,
	RF_DESTROY,
	RF_RENDER,
	RF_INVALIDATE,
	RF_RESTORE,

	RF_ENDOFRFUNCTIONTYPE		// 이건 실제 쓰는 펑션이 아니고.. 숫자를 세기위한것임.
};

typedef int (*RFFUNCTION)(void *Params);

void RSetFunction(RFUNCTIONTYPE ft,RFFUNCTION pfunc);

/*
void RSDrawBegin();
void RSDrawPoly(LITVERTEX* v,int mtrl_id);
void RSDrawEnd();
*/

#endif