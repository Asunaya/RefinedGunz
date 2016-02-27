#define D3D_OVERLOADS
#include <stdio.h>
#include <windows.h>
#include <d3d8.h>
#include "d3ddebug.h"
#include "RSD3D.h"
#include "rtexture.h"
#include "rsdebug.h"
#include "ddraw.h"

// 아~ 만들어보자..
rvector RSLightDirectionWorld,RSLightDirection,RSLightColor;

bool g_bHardwareTNL;
int						g_nDevices=0,g_nCurrentDevice=0;
D3DADAPTER_IDENTIFIER8	*g_DeviceIDs=NULL;
LPDIRECT3DDEVICE8		*g_pDevices=NULL;
D3DPRESENT_PARAMETERS	g_d3dpp; 

LPDIRECT3D8			g_pD3D=NULL;
LPDIRECT3DDEVICE8	g_pd3dDevice=NULL;
D3DVIEWPORT8		g_D3DViewport;

static TRANSLITVERTEX ver[3];

#pragma comment( lib, "ddraw.lib" )
#pragma comment( lib, "d3d8.lib" )

static HWND			g_hWnd;

HMODULE				g_hModule=NULL;
RSPIXELFORMAT		RSPixelFormat;
int					g_nScreenWidth=640,g_nScreenHeight=480,g_nPicmip=0;
static bool			g_bFoundGUID=false,g_bFullScreen=false;
bool				bUseMipmap=true,bZBuffer=true,bTrilinear=true,bTripleBuffer=true,bDualMon=false;

RFFUNCTION g_pFunctions[RF_ENDOFRFUNCTIONTYPE];

void*				g_pCallBackRestoreParam = NULL;


bool RSGetBFullScreen() { return g_bFullScreen; }
void RSSetBFullScreen(bool bFullSCreen) { g_bFullScreen=bFullSCreen; }
RSPIXELFORMAT RSGetPixelFormat() { return RSPixelFormat; }
int RSGetScreenWidth() { return g_nScreenWidth; }
int RSGetScreenHeight() { return g_nScreenHeight; }

void RSSetScreenWH(int w,int h)
{
	g_nScreenWidth  = w;
	g_nScreenHeight = h;
}

void RSSetPicmip(int n) {
	g_nPicmip = n;
}

int RSSetPicmip() {
	return g_nPicmip;
}

DWORD	g_dwFogColor=0;
char	sDDrawName[256];

int nT=0,nTBytes=0,nMaxBytes=0;
typedef struct _D3DTextureType
{
	LPDIRECT3DTEXTURE8		*ITextures;
	int	iPosition;
	int	nBytes;
#ifdef _DEBUG
	char name[256];
#endif
} D3DTextureType;


D3DTextureType		D3DTextures[MAX_TEXTURE_COUNT];
int					EmptyHandleList[MAX_TEXTURE_COUNT];

RGB16f_t*	RS_RGB	= NULL;
GetXf_t*	RS_GetR	= NULL;
GetXf_t*	RS_GetG	= NULL;
GetXf_t*	RS_GetB	= NULL;

extern DWORD GetDXVersion();		// getdxver.cpp 에 있음.

bool CheckDXVersion()
{

    DWORD dwDXVersion=GetDXVersion();
	if(dwDXVersion<0x600)
		return false;
	return true;
}

WORD GetNumberOfBits( DWORD dwMask )		// dwMask 에서 1의 갯수를 리턴.
{
    WORD wBits = 0;
    while( dwMask )
    {
        dwMask = dwMask & ( dwMask - 1 );  
        wBits++;
    }
    return wBits;
}

bool bPowerOf2(int x)						// x 가 2^n 꼴인가.
{
	int c=x,count=0;
	while(c!=0) {count+=c&1;c>>=1;}
	return (count==1)? true : false;
}

int nSurfaceCount;

HRESULT CheckError(HRESULT hr)
{
#ifdef _DEBUG
	if(hr!=DD_OK)
		rslog("%x %s\n",hr,HR2STR(hr));
#endif
	return hr;
}

int RSGetTextureCount()
{
#ifdef _DEBUG
	for(int i=1;i<=nT;i++)
	{
		rslog("%s\n",D3DTextures[EmptyHandleList[i]].name);
	}
#endif
	return nT;
}

int RSGetDeviceCount()
{
	return g_nDevices;
}

int RSGetCurrentDevice()
{
	return g_nCurrentDevice;
}

// 여기서부터는 렌더링 관련 함수들.
///////////////////////////////////////////////////////////////////////////////////////////

void RSDrawFace(rface *f)
{
	memcpy(ver,&f->v[0]->sx,sizeof(TRANSLITVERTEX)-8);
	memcpy(ver+1,&f->v[1]->sx,sizeof(TRANSLITVERTEX)-8);
	memcpy(ver+2,&f->v[2]->sx,sizeof(TRANSLITVERTEX)-8);
	ver[0].tu1=f->s[0];ver[0].tv1=f->t[0];
	ver[1].tu1=f->s[1];ver[1].tv1=f->t[1];
	ver[2].tu1=f->s[2];ver[2].tv1=f->t[2];
	HRESULT result=g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST,1,ver,sizeof(TRANSLITVERTEX));
	_ASSERT(result==D3D_OK);
}

void RSDrawLine(rvertex *v1,rvertex *v2)
{
	memcpy(ver,&v1->sx,sizeof(TRANSLITVERTEX)-8);
	memcpy(ver+1,&v2->sx,sizeof(TRANSLITVERTEX)-8);
	HRESULT result=g_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST,1,ver,sizeof(TRANSLITVERTEX));
	_ASSERT(result==D3D_OK);
}

void RSDrawTriangle(rvertex *v1,rvertex *v2,rvertex *v3)
{
	/*
	g_v[0].rhw=v1->pw;g_v[1].rhw=v2->pw;g_v[2].rhw=v3->pw;

	g_v[0].sx=v1->sx;g_v[0].sy=v1->sy;g_v[0].sz=v1->sz;
	g_v[0].color=v1->color;g_v[0].specular=v1->specular;
	g_v[0].tu=v1->u;g_v[0].tv=v1->v;

	g_v[1].sx=v2->sx;g_v[1].sy=v2->sy;g_v[1].sz=v2->sz;
	g_v[1].color=v2->color;g_v[1].specular=v2->specular;
	g_v[1].tu=v2->u;g_v[1].tv=v2->v;

	g_v[2].sx=v3->sx;g_v[2].sy=v3->sy;g_v[2].sz=v3->sz;
	g_v[2].color=v3->color;g_v[2].specular=v3->specular;
	g_v[2].tu=v3->u;g_v[2].tv=v3->v;

	lpDevice->DrawPrimitive(D3DPT_TRIANGLELIST,D3DFVF_TLVERTEX,g_v,3,D3DDP_WAIT);
	*/
	memcpy(ver,&v1->sx,sizeof(TRANSLITVERTEX));
	memcpy(ver+1,&v2->sx,sizeof(TRANSLITVERTEX));
	memcpy(ver+2,&v3->sx,sizeof(TRANSLITVERTEX));
	HRESULT result=g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST,1,ver,sizeof(TRANSLITVERTEX));
	_ASSERT(result==D3D_OK);
}

int RSCreateTexture(int x,int y,char *memory,char *alphamap,bool bForceNotMipmap,char *Name,bool bUsePicMip)
// Name for debug
{
	int i;

	if(nT>=MAX_TEXTURE_COUNT)
	{
		rslog("Not Enough Texture Space.");
		return 0;
	}

	int nBytes=0;

	rtexture *source=new rtexture;
	source->Create(x,y,memory,alphamap);

	if(!bPowerOf2(x)||!bPowerOf2(y)) 
	{
		rslog("warning : texture size is not power of 2 : %d %d %s\n",x,y,Name);
		int newx=1,newy=1;
		while(newx<x) newx<<=1;
		newx=min(MAX_TEXTURE_SIZE,newx);
		while(newy<y) newy<<=1;
		newy=min(MAX_TEXTURE_SIZE,newy);
		rtexture *newtex=new rtexture;
		newtex->New(newx,newy,source->GetFormat());
		newtex->Fill(0,0,0,source);
		delete source;
		source=newtex;
	}

	if(source->GetWidth()>MAX_TEXTURE_SIZE || source->GetHeight()>MAX_TEXTURE_SIZE)
		rslog("warning : Over MAX_TEXTURE_SIZE %d %d %s\n",source->GetWidth(),source->GetHeight(),Name);
	while(source->GetWidth()>MAX_TEXTURE_SIZE || source->GetHeight()>MAX_TEXTURE_SIZE)
	{
		rtexture *rtnew=new rtexture;
		rtnew->CreateAsHalf(source);
		delete source;
		source=rtnew;
	}


	if(bUsePicMip)
	{
		for(i=0;i<g_nPicmip;i++)
		{
			rtexture *rtnew=new rtexture;
			rtnew->CreateAsHalf(source);
			delete source;
			source=rtnew;
		}
	}


	LPDIRECT3DTEXTURE8 *tex=NULL;
	tex=new LPDIRECT3DTEXTURE8[g_nDevices];


	for(i=0;i<g_nDevices;i++)
	{
		tex[i]=NULL;
		g_pDevices[i]->CreateTexture(source->GetWidth(),source->GetHeight(), bForceNotMipmap?1:0,NULL, 
			alphamap?D3DFMT_A4R4G4B4:D3DFMT_R5G6B5, D3DPOOL_MANAGED , &tex[i]);
		
		if(!tex[i]) {
			delete tex;
			return 0;
		}

		int level,n=tex[i]->GetLevelCount();
		rtexture *rt=new rtexture;
		for(level=0;level<n;level++)
		{
			if(level==0)
				rt->Create(source);
			else
			{
				rtexture *newrt=new rtexture;
				if(!newrt->CreateAsHalf(rt))
				{
					delete newrt;
					break;
				}
				delete rt;
				rt=newrt;
			}
			
			nBytes+=2*rt->GetHeight()*rt->GetWidth();

			D3DLOCKED_RECT	LRECT;
			if(tex[i]->LockRect(level,&LRECT,NULL,0)==DD_OK)
			{
				int j;
				WORD *p=(WORD*)LRECT.pBits;
				if(rt->GetFormat()==RTEXTUREFORMAT_32)
				{
					BYTE *a=(BYTE*)rt->GetData(),*b=a+1,*g=a+2,*r=a+3;
					for(j=0;j<rt->GetWidth()*rt->GetHeight();j++)
					{
						*p= 
							((unsigned short int(*a)&0xf0)<<8) |
							((unsigned short int(*r)&0xf0)<<4) | 
							(unsigned short int(*g)&0xf0) | 
							(unsigned short int(*b)>>4) ;
						a+=4;r+=4;g+=4;b+=4;
						p++;
					}
				}
				else
				{
					BYTE *b=(BYTE*)rt->GetData(),*g=b+1,*r=b+2;
					for(j=0;j<rt->GetWidth()*rt->GetHeight();j++)
					{
						*p=RS_RGB(*r,*g,*b);
						r+=3;g+=3;b+=3;
						p++;
					}
				}
				tex[i]->UnlockRect(level);
			}
			else
				_ASSERT(0);
		}
		delete rt;
/*
		texture[i]=NULL;
		g_pDevices[i]->CreateTexture(x,y, bForceNotMipmap?1:0,NULL, 
			alphamap?D3DFMT_A4R4G4B4:D3DFMT_R5G6B5, D3DPOOL_MANAGED , &texture[i]);
		if(!texture[i]) 
		{
			delete texture;
			delete systex;
			return 0;
		}
		g_pDevices[i]->UpdateTexture(systex[i],texture[i]);
*/
	}
	delete source;

	nT++;
	int rethandle=EmptyHandleList[nT];
	D3DTextures[rethandle].iPosition=nT;
	D3DTextures[rethandle].ITextures=tex;
	D3DTextures[rethandle].nBytes=nBytes;
	nTBytes+=nBytes;
	nMaxBytes=max(nTBytes,nMaxBytes);

#ifdef _DEBUG
	strcpy(D3DTextures[rethandle].name,Name);
#endif

	return rethandle;
}

bool RSDeleteTexture(int iT)
{
	if(!g_pd3dDevice) return false;
	if(D3DTextures[iT].ITextures)
	{
		for(int i=0;i<g_nDevices;i++)
		{
			g_pDevices[i]->EndScene();
			D3DTextures[iT].ITextures[i]->Release();
			D3DTextures[iT].ITextures[i]=NULL;
			g_pDevices[i]->BeginScene();
		}
		delete D3DTextures[iT].ITextures;
		D3DTextures[iT].ITextures=NULL;
		nTBytes-=D3DTextures[iT].nBytes;

		int toswap=EmptyHandleList[nT];
		i=D3DTextures[toswap].iPosition;
		D3DTextures[toswap].iPosition=D3DTextures[iT].iPosition;
		D3DTextures[iT].iPosition=i;

		i=EmptyHandleList[D3DTextures[toswap].iPosition];
		EmptyHandleList[D3DTextures[toswap].iPosition]=EmptyHandleList[nT];
		EmptyHandleList[nT]=i;
		
		nT--;
		
	}
	return true;
}

static int currenttexture[2][8]={0,};

void RSSetTexture(int iT)
{
	if(iT!=currenttexture[g_nCurrentDevice][0])
	{
		HRESULT hr=g_pd3dDevice->SetTexture(0,D3DTextures[iT].ITextures[g_nCurrentDevice]);
		_ASSERT(hr==DD_OK);
		currenttexture[g_nCurrentDevice][0]=iT;
	}
}

void RSSetTexture(int iStage,int iT)
{
	if(iT!=currenttexture[g_nCurrentDevice][iStage])
	{
		HRESULT hr=g_pd3dDevice->SetTexture(iStage,D3DTextures[iT].ITextures[g_nCurrentDevice]);
		_ASSERT(hr==DD_OK);
		currenttexture[g_nCurrentDevice][iStage]=iT;
	}
}

DDSURFACEDESC2 ddsd;

char* RSLock(int *lPitch)
{
	LPDIRECT3DSURFACE8 backbuffer;
	RSSetTexture(0);
	g_pd3dDevice->EndScene();
	g_pd3dDevice->GetRenderTarget(&backbuffer);
	
	D3DLOCKED_RECT LRECT;
	if(backbuffer->LockRect(&LRECT,NULL,NULL)==DD_OK)
	{
		*lPitch=LRECT.Pitch;
		backbuffer->Release();
		return (char*)LRECT.pBits;
	}

	backbuffer->Release();
	return NULL;
}

void RSUnlock()
{
	LPDIRECT3DSURFACE8 backbuffer;
	g_pd3dDevice->GetRenderTarget(&backbuffer);
	backbuffer->UnlockRect();
	backbuffer->Release();
	g_pd3dDevice->BeginScene();
}

LPDIRECT3DSURFACE8 RSGetBackBuffer()
{
	LPDIRECT3DSURFACE8 backbuffer;

	if(g_pd3dDevice == NULL) return NULL;
	g_pd3dDevice->GetRenderTarget(&backbuffer);

	return backbuffer;
}

LPDIRECT3DDEVICE8	RSGetD3DDevice()
{
	return g_pd3dDevice;
}

void RSSetRenderRect(int x1,int y1,int x2,int y2)
{
	g_D3DViewport.X=x1;
	g_D3DViewport.Y=y1;
	g_D3DViewport.Width=x2-x1;
	g_D3DViewport.Height=y2-y1;

	g_D3DViewport.MinZ=0;
	g_D3DViewport.MaxZ=1;
	g_pd3dDevice->SetViewport(&g_D3DViewport);

}

void RSSetAlphaState_Safe(bool bAlphaBlend,D3DBLEND srcblend,D3DBLEND destblend,bool bAlphaTest)
{
	static bool curAlpha=false,curAlphaTest=false;
	static D3DBLEND cursrc=D3DBLEND_ONE,curdest=D3DBLEND_ZERO;

	if(curAlpha!=bAlphaBlend) { g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, bAlphaBlend);curAlpha=bAlphaBlend; }
	if(cursrc!=srcblend) { g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, srcblend);cursrc=srcblend; }
	if(curdest!=destblend) { g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, destblend);curdest=destblend; }
	if(curAlphaTest!=bAlphaTest) { g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, bAlphaTest);curAlphaTest=bAlphaTest; }
}

RSALPHASTYLE RSAlphaState=RS_ALPHASTYLE_NONE;

void RSSetAlphaState(RSALPHASTYLE a)
{
	switch(a)
	{
		case RS_ALPHASTYLE_ALPHAMAP: 
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2 , D3DTA_DIFFUSE );
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_MODULATE );
			break;
		default : 
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_DIFFUSE );
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_SELECTARG1 );
	}

	if(a==RSAlphaState) return; else RSAlphaState=a;

	switch(a)
	{
		case RS_ALPHASTYLE_NONE: RSSetAlphaState_Safe(false,D3DBLEND_ONE,D3DBLEND_ZERO,false);break;
		case RS_ALPHASTYLE_ADD: RSSetAlphaState_Safe(true,D3DBLEND_SRCALPHA,D3DBLEND_ONE,false);break;
		case RS_ALPHASTYLE_ALPHA: RSSetAlphaState_Safe(true,D3DBLEND_SRCALPHA,D3DBLEND_INVSRCALPHA,false);break;
		case RS_ALPHASTYLE_LIGHTMAP: RSSetAlphaState_Safe(true,D3DBLEND_ONE,D3DBLEND_INVSRCCOLOR,false);break;
		case RS_ALPHASTYLE_ALPHAMAP: RSSetAlphaState_Safe(true,D3DBLEND_SRCALPHA,D3DBLEND_INVSRCALPHA,true);break;
		case RS_ALPHASTYLE_SHADOW: RSSetAlphaState_Safe(true,D3DBLEND_ZERO,D3DBLEND_SRCCOLOR,false);break;
		case RS_ALPHASTYLE_MARKS: RSSetAlphaState_Safe(true,D3DBLEND_ZERO,D3DBLEND_INVSRCCOLOR,false);
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP  , D3DTOP_DISABLE );
			break;
	}
}

void RSSetZBufferState(bool r,bool w) // read z, write z
{
	static bool rz=true,wz=true;
	if(rz!=r) { g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, r);rz=r; }
	if(wz!=w) { g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, w);wz=w; }
}

void RSSetFogState( bool bFog , DWORD dwColor)
{
	static bool bcurrent=false;
	static DWORD color=0;

	if(bcurrent!=bFog) { g_pd3dDevice->SetRenderState( D3DRS_FOGENABLE, bFog );bcurrent=bFog;}
	if(color!=dwColor) { g_pd3dDevice->SetRenderState( D3DRS_FOGCOLOR, dwColor );color=dwColor;}
	/*
	if(!dwColor)
		g_dwFogColor=_RGB16( unsigned char (dwColor & 0xff0000 >> 16) ,
								unsigned char (dwColor & 0xff00 >> 8) ,
								unsigned char (dwColor & 0xff) );
	*/
}

void RSSetFogRange(float sz,float ez)
{
	g_pd3dDevice->SetRenderState( D3DRS_FOGSTART , *((DWORD*) (&sz)) );
	g_pd3dDevice->SetRenderState( D3DRS_FOGEND	, *((DWORD*) (&ez)) );
}

void RSSetTextureWrapState(bool bWrap)
{
	g_pd3dDevice->SetTextureStageState(0,D3DTSS_ADDRESSU,bWrap?D3DTADDRESS_WRAP:D3DTADDRESS_CLAMP );
	g_pd3dDevice->SetTextureStageState(0,D3DTSS_ADDRESSV,bWrap?D3DTADDRESS_WRAP:D3DTADDRESS_CLAMP );
}

void RSDrawStrip(rstrip *pStrip)
{
	static TRANSLITVERTEX ver[20];
	for(int i=0;i<pStrip->nVertex;i++)
	{
		memcpy(&ver[i],&pStrip->v[i]->sx,sizeof(TRANSLITVERTEX)-8);
		ver[i].tu1=pStrip->s[i];
		ver[i].tv1=pStrip->t[i];
	}
	g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,pStrip->nVertex-2,ver,sizeof(TRANSLITVERTEX));
}

void RSDrawLineStrip(rstrip *pStrip)
{
	/*
	static D3DTLVERTEX v[20];
	for(int i=0;i<pStrip->nVertex;i++)
	{
		v[i].sx=pStrip->v[i]->sx;
		v[i].sy=pStrip->v[i]->sy;
		v[i].sz=pStrip->v[i]->sz;
		v[i].rhw=pStrip->v[i]->pw;
		v[i].color=pStrip->v[i]->color;
		v[i].specular=pStrip->v[i]->specular;
//		v[i].tu=pStrip->s[i];
//		v[i].tv=pStrip->t[i];
	}
	lpDevice->DrawPrimitive(D3DPT_LINESTRIP,D3DFVF_TLVERTEX,v,pStrip->nVertex,D3DDP_WAIT);
	*/
}

void RSSetCullState(RSCULLSTYPE cs)
{
	switch(cs)
	{
	case RS_CULLSTYLE_NONE	: g_pd3dDevice->SetRenderState(D3DRS_CULLMODE  , D3DCULL_NONE );break;
	case RS_CULLSTYLE_CW	: g_pd3dDevice->SetRenderState(D3DRS_CULLMODE  , D3DCULL_CW);break;
	case RS_CULLSTYLE_CCW	: g_pd3dDevice->SetRenderState(D3DRS_CULLMODE  , D3DCULL_CCW);break;
	}
}

void RSSetDirectionalLight(rvector &dir,rvector &color)
{
	D3DMATERIAL8 mtrl;
	ZeroMemory( &mtrl, sizeof(D3DMATERIAL8) );
	mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
	mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
	mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
	mtrl.Specular.a = 0.5f;
	g_pd3dDevice->SetMaterial( &mtrl );

	D3DLIGHT8 light;
	ZeroMemory( &light, sizeof(D3DLIGHT8) );
	light.Type       = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r  = color.x;
	light.Diffuse.g  = color.y;
	light.Diffuse.b  = color.z;
	light.Range       = 1000.0f;

	dir=Normalize(dir);
	light.Direction.x=dir.x;
	light.Direction.y=dir.y;
	light.Direction.z=dir.z;
	g_pd3dDevice->SetLight( 0, &light );
	g_pd3dDevice->LightEnable( 0, TRUE);

	RSLightDirection=dir;
	RSLightColor=color;
}

LPDIRECT3DTEXTURE8 g_pShadowTexture=NULL;

void InitDevice()
{
	g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x00202020 );

	g_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE  , D3DCULL_NONE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER , D3DTEXF_LINEAR);
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER , D3DTEXF_LINEAR);
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER , bTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE );
	g_pd3dDevice->SetTextureStageState( 1, D3DTSS_MAGFILTER , D3DTEXF_LINEAR);
	g_pd3dDevice->SetTextureStageState( 1, D3DTSS_MINFILTER , D3DTEXF_LINEAR);
	g_pd3dDevice->SetTextureStageState( 1, D3DTSS_MIPFILTER , bTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE );

	g_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x00000000);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL );

//	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE ,D3DZB_USEW  );		// for w-buffering
	g_pd3dDevice->SetRenderState(D3DRS_ZFUNC ,D3DCMP_LESSEQUAL);

	D3DCLIPSTATUS8 cs;

	g_pd3dDevice->GetClipStatus(&cs);
	g_pd3dDevice->SetClipStatus(&cs);

	float fMaxBias = -1.0f;
	g_pd3dDevice->SetTextureStageState( 0 ,D3DTSS_MIPMAPLODBIAS , *(unsigned long*)&fMaxBias);
	g_pd3dDevice->SetTextureStageState( 1 ,D3DTSS_MIPMAPLODBIAS , *(unsigned long*)&fMaxBias);

	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );
	g_pd3dDevice->SetVertexShader(RSTLFVF);
	g_pd3dDevice->Clear(0,NULL,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER ,0x000000,1.0f ,0);

	g_pd3dDevice->CreateTexture(256,256, 1, D3DUSAGE_RENDERTARGET , D3DFMT_R5G6B5, D3DPOOL_DEFAULT , &g_pShadowTexture);

	g_pd3dDevice->BeginScene();
}

bool RSInitDisplay(HWND hWnd,int nWidth,int nHeight,bool bFullScreen,bool bDualMon)
{
	int i;

	if( NULL == ( g_pD3D = Direct3DCreate8( D3D_SDK_VERSION ) ) )
		return false;
	rslog("D3D Created.\n");

	// get device's informations
	g_nDevices=(g_pD3D->GetAdapterCount() >=2 && bDualMon ) ? 2 : 1;

	g_DeviceIDs=new D3DADAPTER_IDENTIFIER8[g_nDevices];
	g_pDevices=new LPDIRECT3DDEVICE8[g_nDevices];
	for(i=0;i<g_nDevices;i++)
	{
		g_pD3D->GetAdapterIdentifier(i,0,&g_DeviceIDs[i]);
		rslog("device : %s , version : %d , %d \n",g_DeviceIDs[i].Description,
			g_DeviceIDs[i].DriverVersion.HighPart,g_DeviceIDs[i].DriverVersion.LowPart);
		g_pDevices[i]=NULL;
	}

	D3DCAPS8 d3dcaps;
	g_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT , D3DDEVTYPE_HAL , &d3dcaps );
	g_bHardwareTNL = (d3dcaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) != 0;
	if(d3dcaps.RasterCaps & D3DPRASTERCAPS_WFOG) rslog("WFog Enabled Device.\n");
	// get screen information
	D3DDISPLAYMODE d3ddm;
	HRESULT hr;
	hr=g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );
	
	g_bFullScreen=bFullScreen;
	g_nScreenWidth=nWidth;
	g_nScreenHeight=nHeight;

	ZeroMemory( &g_d3dpp, sizeof(g_d3dpp) );
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
	g_d3dpp.BackBufferWidth = nWidth;
	g_d3dpp.BackBufferHeight = nHeight;
    g_d3dpp.hDeviceWindow    = hWnd;
	g_d3dpp.Windowed   = !bFullScreen;
	g_d3dpp.BackBufferFormat = bFullScreen ? D3DFMT_R5G6B5 : d3ddm.Format;
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.Flags=D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	DWORD BehaviorFlags=D3DCREATE_FPU_PRESERVE | 
		(g_bHardwareTNL ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING);
#ifdef _MT
	rslog("multithread.\n");
	BehaviorFlags|=D3DCREATE_MULTITHREADED;
#endif

	if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,BehaviorFlags,&g_d3dpp,&g_pDevices[0]) ) )
		return false;
	g_pd3dDevice=g_pDevices[0];

	if(bDualMon)
	{
		if( FAILED( g_pD3D->CreateDevice( 1, D3DDEVTYPE_HAL, hWnd,BehaviorFlags,&g_d3dpp, &g_pDevices[1] ) ) )
		{
			rslog("secondary device initialize failed.\n");
			g_pd3dDevice->Release();
			g_pD3D->Release();
			delete g_DeviceIDs;
			delete g_pDevices;
			return false;
		}
	}

	rslog("device created.\n");

	for(i=0;i<g_nDevices;i++)
	{
		if(g_pDevices[i])
		{
			RSSelectDevice(i);
			InitDevice();
		}
	}

	if( FAILED( g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )
		return false;
	if( g_hModule ) FreeLibrary( g_hModule );
	g_hModule=NULL;

	hr=g_pd3dDevice->GetDisplayMode( &d3ddm );
	switch(d3ddm.Format)
	{
	case D3DFMT_X8R8G8B8 : 
		RSPixelFormat=RS_PIXELFORMAT_X888;
		g_hModule = LoadLibrary("drv16.dll"); 
		break;
	case D3DFMT_R5G6B5 : 
		RSPixelFormat=RS_PIXELFORMAT_565;
		g_hModule = LoadLibrary("drv16.dll"); 
		break;
	case D3DFMT_X1R5G5B5 : 
		RSPixelFormat=RS_PIXELFORMAT_555;
		g_hModule = LoadLibrary("drv15.dll"); 
		break;
	}
	if(!g_hModule)
	{
		rslog("failed to load drvXX.dll or Unknown Pixel Format \n");
		for(i=0;i<g_nDevices;i++)
		{
			if(g_pDevices[i])
			{
				g_pDevices[i]->Release();
				g_pDevices[i]=NULL;
			}
		}
		SAFE_DELETE(g_pDevices);
		SAFE_DELETE(g_DeviceIDs);
		SAFE_RELEASE(g_pD3D);
		return false;
	}

	RS_RGB	= (RGB16f_t *) GetProcAddress( g_hModule, "RGB16" );
	RS_GetR = (GetXf_t*) GetProcAddress( g_hModule, "GetR16" );
	RS_GetG = (GetXf_t*) GetProcAddress( g_hModule, "GetG16" );
	RS_GetB = (GetXf_t*) GetProcAddress( g_hModule, "GetB16" );

	g_nScreenWidth=nWidth;
	g_nScreenHeight=nHeight;

	for(i=0;i<MAX_TEXTURE_COUNT;i++)
	{
		D3DTextures[i].ITextures=NULL;
		D3DTextures[i].iPosition=i;
		EmptyHandleList[i]=i;
	}

	D3DTextures[0].ITextures=new LPDIRECT3DTEXTURE8[g_nDevices];
	for(i=0;i<g_nDevices;i++) D3DTextures[0].ITextures[i]=NULL;

	return true;
}

bool RSCloseDisplay()
{
	if(!g_pd3dDevice) return false;
	
	g_pd3dDevice->EndScene();
	rslog("Close Textures... ( texture memory peak : %d bytes )\n",nMaxBytes);
	SAFE_RELEASE(g_pShadowTexture);
	for(int i=1;i<=nT;i++)
	{
		for(int j=0;j<g_nDevices;j++)
		{
			D3DTextures[EmptyHandleList[i]].ITextures[j]->Release();
		}
		delete D3DTextures[EmptyHandleList[i]].ITextures;
	}
	SAFE_DELETE(D3DTextures[0].ITextures);

	if(g_pDevices)
	{
		for(i=0;i<g_nDevices;i++)
			SAFE_RELEASE(g_pDevices[i]);
	}
	
	SAFE_DELETE(g_pDevices);
	SAFE_DELETE(g_DeviceIDs);
	SAFE_RELEASE(g_pD3D);
	g_pd3dDevice=NULL;

	if(g_hModule)
		FreeLibrary(g_hModule);
	return true;
}

bool RSFlip()
{
	for(int i=0;i<g_nDevices;i++)
	{
		RSSelectDevice(i);
		RSSetTexture(0);
		g_pd3dDevice->EndScene();
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
		g_pd3dDevice->Clear(0,NULL,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER ,0x000000,1.0f ,0);
		g_pd3dDevice->BeginScene();
	}
	return true;
}

void RSSelectDevice(int nDevice)
{
	g_pd3dDevice=g_pDevices[nDevice];
	g_nCurrentDevice=nDevice;
}


void RSResetDevice()
{
	if(g_pFunctions[RF_INVALIDATE]!=NULL)
		g_pFunctions[RF_INVALIDATE](NULL);
	if(g_pShadowTexture)
		g_pShadowTexture->Release();

	g_d3dpp.Windowed			= !g_bFullScreen;

	if((g_d3dpp.BackBufferWidth != (UINT)g_nScreenWidth)
		||(g_d3dpp.BackBufferHeight != (UINT)g_nScreenHeight))
	{
		g_d3dpp.BackBufferWidth		= g_nScreenWidth;
		g_d3dpp.BackBufferHeight	= g_nScreenHeight;
	}

	HRESULT hr=g_pd3dDevice->Reset(&g_d3dpp);
	_ASSERT(hr==D3D_OK);
	InitDevice();

	if(g_pFunctions[RF_RESTORE]!=NULL)
		g_pFunctions[RF_RESTORE](NULL);
}

bool RSReadyToRender()
{
	HRESULT hr;
    if( FAILED( hr = g_pd3dDevice->TestCooperativeLevel() ) )
    {
        // If the device was lost, do not render until we get it back
        if( D3DERR_DEVICELOST == hr )
            return false;

        // Check if the device needs to be resized.
        if( D3DERR_DEVICENOTRESET == hr )
		{
			int i;

			for(i=0;i<g_nDevices;i++)
			{
				if(g_pDevices[i])
				{
					RSSelectDevice(i);
					RSResetDevice();
				}
			}
			
			rslog("All devices Restored. \n");
			return true;
		}
	}
	return true;
}

void RSetFunction(RFUNCTIONTYPE ft,RFFUNCTION pfunc)
{
	g_pFunctions[ft]=pfunc;
}

/*
//////////////////////////////////////// 임시 테스트 코드
//<--------------------
// 우선은 decal 용

int	g_draw_buffer_cnt;
int	g_draw_buffer_pos;

//효과 face 1만개 정도로 잡음

LITVERTEX	g_vert_render[20000];
LITVERTEX	g_vert_temp[30000];
int			g_vert_temp_mtrl[10000];

int			g_mtrl_index[1000];
int			g_mtrl_index_cnt;

/////////////////////////////////////
// 알파 거리에 대한 고려 없음

void RSDrawBegin()
{
	g_draw_buffer_cnt	= 0;
	g_draw_buffer_pos	= 0;

	g_mtrl_index_cnt	= 0;//머터리얼 종류의 개수 
}

void RSDrawPoly(LITVERTEX* v,int mtrl_id)
{
	if(g_draw_buffer_pos > 29990) 
	{
		rslog("ase 단일 오브젝트 버텍스 갯수가 29990개가 넘습니다");
		return;
	}

	g_vert_temp[g_draw_buffer_pos  ] = v[0];
	g_vert_temp[g_draw_buffer_pos+1] = v[1];
	g_vert_temp[g_draw_buffer_pos+2] = v[2];

	g_vert_temp_mtrl[g_draw_buffer_cnt] = mtrl_id;// 이번 버퍼의 mtrl id

//	rslog("mtrl_id  : %d \n",mtrl_id);

	if(!g_mtrl_index_cnt)
	{
		g_mtrl_index[g_mtrl_index_cnt] = mtrl_id;
		g_mtrl_index_cnt++;
	}
	else
	{
		bool add = true;

		for(int t=0;t<g_mtrl_index_cnt;t++)
		{
			if(g_mtrl_index[t]==mtrl_id)
			{
				add = false;
				break;
			}
		}

		if(add)
		{
			g_mtrl_index[g_mtrl_index_cnt] = mtrl_id;
			g_mtrl_index_cnt++;
		}
	}

	g_draw_buffer_cnt++;
	g_draw_buffer_pos += 3;
}

void RSDrawEnd()
{
	int i,j,cnt;

	// mtrl 종류가 무지 많다면 손해봄..
	for(i=0;i<g_mtrl_index_cnt;i++)
	{
		cnt = 0;
		
		for(j=0;j<g_draw_buffer_cnt;j++)
		{
			if(g_vert_temp_mtrl[j] == g_mtrl_index[i])
			{
				memcpy(&g_vert_render[cnt],&g_vert_temp[j*3],sizeof(LITVERTEX)*3);//어떻게 인덱스로 해결할 방법은??
				cnt+=3;
			}
		}

		RSSetTexture(g_mtrl_index[i]);
		g_pd3dDevice->SetVertexShader(RSLFVF);
		g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, cnt/3, (LPVOID) g_vert_render, sizeof(LITVERTEX));
	}

//	rslog("g_mtrl_index_cnt : %d g_draw_buffer_cnt : %d\n",g_mtrl_index_cnt,g_draw_buffer_cnt);

	g_draw_buffer_cnt = 0;
	g_draw_buffer_pos = 0;
	g_mtrl_index_cnt  = 0;
}

//-------------------->
*/