//////////////////////////////////////////////////////////////////////////
//  [10/27/2003]
//	Water - reflection / refraction / wave
//	Magicbell	
//	 
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "d3d8.h"
#include "d3dx8.h"
#include "RMeshUtil.h"
#include "vector"

using namespace std;
//
struct WATER_VERTEX 
{
	float x, y, z;
	float nx, ny, nz;
	float tu1, tv1;
	float tu2, tv2;
//	float tu3, tv3;
};

#define WATER_VERTEX_TYPE (D3DFVF_XYZ   |D3DFVF_NORMAL | D3DFVF_TEX2 )

enum WAVE_TYPE
{
	PERTURBATION,
	RIPPLE,
};

struct sWave {
	bool bRefresh;

	int iTimer;
	float fRadius;
	float fShift;

	float fSpeed;

	int iType;
	float fAmplitude;
	float fFrequency;
	int iLimitTime;

	rvector vPos;

};

#define  MAX_WATER_SURFACE_VERTEX 2000

class ZWater
{
protected:
public:
	LPD3DXRENDERTOSURFACE	mpRenderToSurfaceForReflection;		//	REFLECTION
	LPD3DXRENDERTOSURFACE	mpRenderToSurfaceForRefraction;		// REFRACTION
	LPDIRECT3DSURFACE8		mpWaterReflectionSurface;
	LPDIRECT3DSURFACE8		mpWaterRefractionSurface;
	LPDIRECT3DTEXTURE8		mpWaterReflectionTexture;
	LPDIRECT3DTEXTURE8		mpWaterRefractionTexture;
	LPDIRECT3DVERTEXBUFFER8	mpWaterVB;				// Water
	LPDIRECT3DINDEXBUFFER8	mpWaterIB;

	rmatrix		mProjHolder;
	rmatrix		mViewHolder;


	int		mWidth;
	int		mHeight;

	int			mNumVertices;
	rvector*	mVertexList;
	int			mNumIndices;
	WORD*		mIndexList;

	rplane		mClipPlane;
	rvector		mCentre;

	rmatrix		mMirrorMatrix;
	float		mPosZ;

	static RVertex		msCopyTemp[MAX_WATER_SURFACE_VERTEX];
	//static WATER_VERTEX		msCopyTemp[MAX_WATER_SURFACE_VERTEX];
	vector<sWave*> mWaveList;

	LPDIRECT3DSURFACE8	mpHoldBackBuffer;
	LPDIRECT3DSURFACE8	mpHoldStencil;

	rvector	mCameraPosition;
	rvector	mCameraDirection;
	rvector	mCameraUp;
     
public:

	bool	Create();
	bool	Release();
	bool	CreateBuffer();		// device create

	bool	Initialize( int width_, int heiht_, rvector& pos_, float diff_ = 1.0f );
	bool	SetBuffer();
	bool	Update();

	bool	ReflectionBegin();
	bool	ReflectionEnd();
	
	bool	RefractionBegin();
	bool	RefractionEnd();

	void	CopyVertices();
	void	Perturbation( sWave* pWave_ );
	void	Ripple( sWave* pWave_ );
	void	UpdateNormal();

	void	addWave( sWave* wave )
	{
		mWaveList.push_back( wave );
	}

	void	Draw();
	static void	ss();

public:
	ZWater(void);
	~ZWater(void);
};
