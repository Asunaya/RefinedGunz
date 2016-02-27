// RSEnvironmentObject.cpp: implementation of the RSEnvironmentObject class.
//////////////////////////////////////////////////////////////////////

#include "RSEnvironmentObject.h"
#include "RealSpace.h"
#include "RSObject.h"
#include "RSDebug.h"
#include "RSD3D.h"
#include "RSMaterialList.h"
#include "RSVisualObject.h"

#define DEFAULT_SCALE	20.f //(REALSPACE_DEFAULT_FARZCLIP_DISTANCE-5.f)

// 코드에 모델 박아넣기 필살기..

LITVERTEX SkyVertices[71]={
{-0.273f,0.000f,0.962f,0,},	{-0.084f,0.260f,0.962f,0,0,-0.028f,0.086f},	
{0.000f,0.000f,1.000f,0,0,0.000f,0.000f},	{-0.526f,0.000f,0.851f,0,0,-0.190f,0.000f},	
{-0.362f,0.263f,0.894f,0,0,-0.126f,0.091f},	{-0.162f,0.500f,0.851f,0,0,-0.059f,0.180f},	
{0.221f,0.161f,0.962f,0,0,0.073f,0.053f},	{0.138f,0.425f,0.894f,0,0,0.048f,0.148f},	
{0.425f,0.309f,0.851f,0,0,0.153f,0.112f},	{0.221f,-0.161f,0.962f,0,0,0.073f,-0.053f},	
{0.447f,-0.000f,0.894f,0,0,0.156f,-0.000f},	{0.425f,-0.309f,0.851f,0,0,0.153f,-0.112f},	
{-0.084f,-0.260f,0.962f,0,0,-0.028f,-0.086f},	{0.138f,-0.425f,0.894f,0,0,0.048f,-0.148f},	
{-0.162f,-0.500f,0.851f,0,0,-0.059f,-0.180f},	{-0.362f,-0.263f,0.894f,0,0,-0.126f,-0.091f},	
{-0.447f,0.526f,0.724f,0,0,-0.181f,0.213f},	{-0.638f,0.263f,0.724f,0,0,-0.258f,0.106f},	
{-0.688f,0.500f,0.526f,0,0,-0.343f,0.249f},	{0.362f,0.588f,0.724f,0,0,0.146f,0.238f},	
{0.053f,0.688f,0.724f,0,0,0.021f,0.278f},	{0.263f,0.809f,0.526f,0,0,0.131f,0.403f},	
{0.671f,-0.162f,0.724f,0,0,0.271f,-0.066f},	{0.671f,0.162f,0.724f,0,0,0.271f,0.066f},	
{0.851f,-0.000f,0.526f,0,0,0.423f,-0.000f},	{0.053f,-0.688f,0.724f,0,0,0.021f,-0.278f},	
{0.362f,-0.588f,0.724f,0,0,0.146f,-0.238f},	{0.263f,-0.809f,0.526f,0,0,0.131f,-0.403f},	
{-0.638f,-0.263f,0.724f,0,0,-0.258f,-0.106f},	{-0.447f,-0.526f,0.724f,0,0,-0.181f,-0.213f},	
{-0.688f,-0.500f,0.526f,0,0,-0.343f,-0.249f},	{-0.738f,0.000f,0.675f,0,0,-0.313f,0.000f},	
{-0.823f,-0.260f,0.506f,0,0,-0.419f,-0.132f},	{-0.894f,0.000f,0.447f,0,0,-0.490f,0.000f},	
{-0.823f,0.260f,0.506f,0,0,-0.419f,0.132f},	{-0.228f,0.702f,0.675f,0,0,-0.097f,0.298f},	
{-0.501f,0.702f,0.506f,0,0,-0.255f,0.358f},	{-0.276f,0.851f,0.447f,0,0,-0.151f,0.466f},	
{-0.007f,0.863f,0.506f,0,0,-0.004f,0.440f},	{0.597f,0.434f,0.675f,0,0,0.253f,0.184f},	
{0.513f,0.694f,0.506f,0,0,0.261f,0.353f},	{0.724f,0.526f,0.447f,0,0,0.396f,0.288f},	
{0.818f,0.273f,0.506f,0,0,0.417f,0.139f},	{0.597f,-0.434f,0.675f,0,0,0.253f,-0.184f},	
{0.818f,-0.273f,0.506f,0,0,0.417f,-0.139f},	{0.724f,-0.526f,0.447f,0,0,0.396f,-0.288f},	
{0.513f,-0.694f,0.506f,0,0,0.261f,-0.353f},	{-0.228f,-0.702f,0.675f,0,0,-0.097f,-0.298f},	
{-0.007f,-0.863f,0.506f,0,0,-0.004f,-0.440f},	{-0.276f,-0.851f,0.447f,0,0,-0.151f,-0.466f},	
{-0.501f,-0.702f,0.506f,0,0,-0.255f,-0.358f},	{-0.862f,0.425f,0.276f,0,0,-0.601f,0.297f},	
{-0.959f,0.161f,0.232f,0,0,-0.719f,0.120f},	{-0.449f,0.863f,0.232f,0,0,-0.337f,0.647f},	
{-0.671f,0.688f,0.276f,0,0,-0.468f,0.480f},	{0.138f,0.951f,0.276f,0,0,0.096f,0.663f},	
{-0.144f,0.962f,0.232f,0,0,-0.108f,0.721f},	{0.682f,0.694f,0.232f,0,0,0.511f,0.520f},	
{0.447f,0.851f,0.276f,0,0,0.312f,0.593f},	{0.947f,0.162f,0.276f,0,0,0.660f,0.113f},	
{0.870f,0.434f,0.232f,0,0,0.652f,0.325f},	{0.870f,-0.434f,0.232f,0,0,0.652f,-0.325f},	
{0.947f,-0.162f,0.276f,0,0,0.660f,-0.113f},	{0.447f,-0.851f,0.276f,0,0,0.312f,-0.593f},	
{0.682f,-0.694f,0.232f,0,0,0.511f,-0.520f},	{-0.144f,-0.962f,0.232f,0,0,-0.108f,-0.721f},	
{0.138f,-0.951f,0.276f,0,0,0.096f,-0.663f},	{-0.671f,-0.688f,0.276f,0,0,-0.468f,-0.480f},	
{-0.449f,-0.863f,0.232f,0,0,-0.337f,-0.647f},	{-0.959f,-0.161f,0.232f,0,0,-0.719f,-0.120f},	
{-0.862f,-0.425f,0.276f,0,0,-0.601f,-0.297f}};

WORD SkyIndicies[360]={
0,1,2,3,4,0,4,1,0,4,5,1,1,6,2,5,7,1,7,6,
1,7,8,6,6,9,2,8,10,6,10,9,6,10,11,9,9,12,2,11,
13,9,13,12,9,13,14,12,12,0,2,14,15,12,15,0,12,15,3,0,
16,17,18,5,4,16,4,17,16,4,3,17,19,20,21,8,7,19,7,20,
19,7,5,20,22,23,24,11,10,22,10,23,22,10,8,23,25,26,27,14,
13,25,13,26,25,13,11,26,28,29,30,3,15,28,15,29,28,15,14,29,
28,31,3,30,32,28,32,31,28,32,33,31,31,17,3,33,34,31,34,17,
31,34,18,17,16,35,5,18,36,16,36,35,16,36,37,35,35,20,5,37,
38,35,38,20,35,38,21,20,19,39,8,21,40,19,40,39,19,40,41,39,
39,23,8,41,42,39,42,23,39,42,24,23,22,43,11,24,44,22,44,43,
22,44,45,43,43,26,11,45,46,43,46,26,43,46,27,26,25,47,14,27,
48,25,48,47,25,48,49,47,47,29,14,49,50,47,50,29,47,50,30,29,
18,34,51,34,52,51,34,33,52,37,36,53,36,54,53,36,18,54,21,38,
55,38,56,55,38,37,56,41,40,57,40,58,57,40,21,58,24,42,59,42,
60,59,42,41,60,45,44,61,44,62,61,44,24,62,27,46,63,46,64,63,
46,45,64,49,48,65,48,66,65,48,27,66,30,50,67,50,68,67,50,49,
68,33,32,69,32,70,69,32,30,70,69,52,33,51,54,18,53,56,37,55,
58,21,57,60,41,59,62,24,61,64,45,63,66,27,65,68,49,67,70,30};

RSEnvironmentObject::RSEnvironmentObject()
{
	m_TextureHandle=NULL;
	m_fou=0;
	m_fov=0;
	pVertexBuffer=NULL;
	pIndexBuffer=NULL;
	uvs=NULL;
	m_pVO=NULL;
	m_nDrawLevel = 2;
}

RSEnvironmentObject::~RSEnvironmentObject()
{
	SAFE_DELETE(uvs);

	int i;
	if(pVertexBuffer)
	{
		for(i=0;i<RSGetDeviceCount();i++)
			pVertexBuffer[i]->Release();
		delete pVertexBuffer;
	}
	if(pIndexBuffer)
	{
		for(i=0;i<RSGetDeviceCount();i++)
			pIndexBuffer[i]->Release();
		delete pIndexBuffer;
	}
	SAFE_DELETE(m_pVO);
}

#define TILING_CONSTANT 1.f

bool RSEnvironmentObject::Create(RSObject *pObject,int hTexture,bool bAlpha)
{
	m_bAlpha=bAlpha;

	int i,k;
	nv=sizeof(SkyVertices)/sizeof(LITVERTEX);
	ni=sizeof(SkyIndicies)/sizeof(WORD);
	uvs=new float[nv*2];

	float maxangle=0;
	for(i=0;i<nv;i++)
	{
		rvector *pp=(rvector*)&SkyVertices[i].x;
		float angle=(float)acos(DotProduct(*pp,rvector(0,0,1)));
		maxangle=max(maxangle,angle);
	}
	for(i=0;i<nv;i++)
	{
		rvector *pp=(rvector*)&SkyVertices[i].x;
		float angle=(float)acos(DotProduct(*pp,rvector(0,0,1)));
// calc opacity
		float opac=1.0f-angle/maxangle;
		SkyVertices[i].Diffuse=0xffffff | DWORD ( unsigned char(opac*255.f) ) << 24;
// calc uv
		rvector dir=InterpolatedVector(rvector(0,0,1),Normalize(*pp),0.95f);
		float pl=TILING_CONSTANT*1/dir.z;
		uvs[i*2]=SkyVertices[i].tu1=dir.x*pl;
		uvs[i*2+1]=SkyVertices[i].tv1=dir.y*pl;
	}


	m_TextureHandle=hTexture;
	pVertexBuffer=new LPDIRECT3DVERTEXBUFFER8[RSGetDeviceCount()];
	pIndexBuffer=new LPDIRECT3DINDEXBUFFER8[RSGetDeviceCount()];

	for(k=0;k<RSGetDeviceCount();k++)
	{
		pVertexBuffer[k]=NULL;
		g_pDevices[k]->EndScene();

		// create vertex buffer
		if( FAILED( g_pDevices[k]->CreateVertexBuffer( sizeof(LITVERTEX)*nv, 0 , RSLFVF,
										  D3DPOOL_MANAGED, &pVertexBuffer[k] ) ) )

		{
			rslog("create vertex buffer failed.\n");
			delete pVertexBuffer;
			delete pIndexBuffer;
			return false;
		}

		VOID* pVertices;
		pVertexBuffer[k]->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ;
		memcpy( pVertices, SkyVertices, sizeof(LITVERTEX)*nv);
		pVertexBuffer[k]->Unlock();

		// create index buffer
		if( FAILED( g_pDevices[k]->CreateIndexBuffer( sizeof(WORD)*ni, 0 , D3DFMT_INDEX16 ,
										  D3DPOOL_MANAGED, &pIndexBuffer[k] ) ) )

		{
			rslog("create index buffer failed.\n");
			delete pVertexBuffer;
			delete pIndexBuffer;
			return false;
		}

		WORD* pIndicies;
		pIndexBuffer[k]->Lock( 0, sizeof(WORD)*ni, (BYTE**)&pIndicies, 0 ) ;
		memcpy( pIndicies,SkyIndicies,sizeof(WORD)*ni);
		pIndexBuffer[k]->Unlock();

		g_pDevices[k]->BeginScene();
		//*/
	}

	if(pObject)
	{
		m_pVO=new RSVisualObject;
		m_pVO->Create(pObject);
		m_pVO->SetAnimation(0);
		m_pVO->SetAnimationStyle(RS_ANIMATIONSTYLE_ROUND);
	}
	return true;
}

void RSEnvironmentObject::Move(float fTime)
{
	m_fou=(float)fmod(m_fou+fTime,1.0f);
	m_fov=(float)fmod(m_fov+fTime,1.0f);
	if(m_pVO && m_pVO->GetCurrentAnimation())
		m_pVO->MoveAnimation(fTime*100000.f);
}

void RSEnvironmentObject::Draw()
{
	if(m_nDrawLevel == 0) return;

	g_pd3dDevice->SetRenderState( D3DRS_RANGEFOGENABLE , TRUE );
	g_pd3dDevice->SetRenderState( D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);

	float fFogDistance=1.2f*DEFAULT_SCALE;
	float fFogEndDistance=1.7f*DEFAULT_SCALE;
	RSSetFogRange(fFogDistance,fFogEndDistance);
//	RSSetCullState(RS_CULLSTYLE_NONE);
	
	if(m_pVO)
	{
		m_pVO->SetPosition(RSCameraPosition);
		m_pVO->SetScale(DEFAULT_SCALE);
		m_pVO->SetZBufferState(false,false);
		m_pVO->DrawMono(0xffffffff);
	}

	g_pd3dDevice->SetRenderState( D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
	RSSetFogState(false,0);

	RSSetTextureWrapState(true);
	RSSetAlphaState(m_bAlpha ? RS_ALPHASTYLE_ALPHAMAP : RS_ALPHASTYLE_ADD);
	RSSetTexture(m_TextureHandle);

	SetPosition(RSCameraPosition);
	m_Transform=ScaleMatrix44(DEFAULT_SCALE)*GetMatrix();
	g_pd3dDevice->SetTransform(D3DTS_VIEW, (_D3DMATRIX*)&RSView);
	g_pd3dDevice->SetTransform(D3DTS_WORLD, (_D3DMATRIX*)&m_Transform);

	rmatrix44 tcm=IdentityMatrix44();
	tcm._31=m_fou;tcm._32=m_fov;
	g_pd3dDevice->SetTransform( D3DTS_TEXTURE0 , (_D3DMATRIX*)&tcm );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS,   D3DTTFF_COUNT2 );

	g_pd3dDevice->SetVertexShader(RSLFVF);
	g_pd3dDevice->SetStreamSource( 0, pVertexBuffer[RSGetCurrentDevice()], sizeof(LITVERTEX) );
	g_pd3dDevice->SetIndices(pIndexBuffer[RSGetCurrentDevice()],0);
	g_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,0,nv, 0, ni/3);
//*/

	g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE  , true );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS,  D3DTTFF_DISABLE );
}
